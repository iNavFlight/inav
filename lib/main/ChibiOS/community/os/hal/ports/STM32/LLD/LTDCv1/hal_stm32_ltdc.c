/*
    Copyright (C) 2013-2015 Andrea Zoppi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    hal_stm32_ltdc.c
 * @brief   LCD-TFT Controller Driver.
 */

#include "hal.h"

#include "hal_stm32_ltdc.h"

#if (TRUE == STM32_LTDC_USE_LTDC) || defined(__DOXYGEN__)

/* TODO: Check preconditions (e.g., LTDC is ready).*/

/* Ignore annoying warning messages for actually safe code.*/
#if defined(__GNUC__) && !defined(__DOXYGEN__)
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * @addtogroup ltdc
 * @{
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if !defined(LTDC_LxBFCR_BF) && !defined(__DOXYGEN__)
#define LTDC_LxBFCR_BF  (LTDC_LxBFCR_BF1 | LTDC_LxBFCR_BF2)
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   LTDC1 driver identifier.
 */
LTDCDriver LTDCD1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Bits per pixel lookup table.
 */
static const uint8_t ltdc_bpp[LTDC_MAX_PIXFMT_ID + 1] = {
  32,  /* LTDC_FMT_ARGB8888 */
  24,  /* LTDC_FMT_RGB888 */
  16,  /* LTDC_FMT_RGB565 */
  16,  /* LTDC_FMT_ARGB1555 */
  16,  /* LTDC_FMT_ARGB4444 */
   8,  /* LTDC_FMT_L8 */
   8,  /* LTDC_FMT_AL44 */
  16   /* LTDC_FMT_AL88 */
};

/**
 * @brief   Invalid frame.
 */
static const ltdc_frame_t ltdc_invalid_frame = {
  NULL,
  1,
  1,
  1,
  LTDC_FMT_L8
};

/**
 * @brief   Invalid window.
 * @details Pixel size, located at the origin of the screen.
 */
static const ltdc_window_t ltdc_invalid_window = {
  0,
  1,
  0,
  1
};

/**
 * @brief   Default layer specifications.
 */
static const ltdc_laycfg_t ltdc_default_laycfg = {
  &ltdc_invalid_frame,
  &ltdc_invalid_window,
  LTDC_COLOR_BLACK,
  0x00,
  LTDC_COLOR_BLACK,
  NULL,
  0,
  LTDC_BLEND_FIX1_FIX2,
  0
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Forces LTDC register reload.
 * @details Blocking function.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @sclass
 * @notapi
 */
static void ltdc_force_reload_s(LTDCDriver *ltdcp) {

  osalDbgCheckClassS();
  osalDbgCheck(ltdcp == &LTDCD1);

  LTDC->SRCR |= LTDC_SRCR_IMR;
  while (LTDC->SRCR & (LTDC_SRCR_IMR | LTDC_SRCR_VBR))
    chSchDoYieldS();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @name    LTDC interrupt handlers
 * @{
 */

/**
 * @brief   LTDC event interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_LTDC_EV_HANDLER) {

  LTDCDriver *const ltdcp = &LTDCD1;
  thread_t *tp = NULL;

  OSAL_IRQ_PROLOGUE();

  /* Handle Line Interrupt ISR.*/
  if ((LTDC->ISR & LTDC_ISR_LIF) && (LTDC->IER & LTDC_IER_LIE)) {
    osalDbgAssert(ltdcp->config->line_isr != NULL, "invalid state");
    ltdcp->config->line_isr(ltdcp);
    LTDC->ICR |= LTDC_ICR_CLIF;
  }

  /* Handle Register Reload ISR.*/
  if ((LTDC->ISR & LTDC_ISR_RRIF) && (LTDC->IER & LTDC_IER_RRIE)) {
    if (ltdcp->config->rr_isr != NULL)
      ltdcp->config->rr_isr(ltdcp);

    osalSysLockFromISR();
    osalDbgAssert(ltdcp->state == LTDC_ACTIVE, "invalid state");
#if (TRUE == LTDC_USE_WAIT)
    /* Wake the waiting thread up.*/
    if (ltdcp->thread != NULL) {
      tp = ltdcp->thread;
      ltdcp->thread = NULL;
      tp->u.rdymsg = MSG_OK;
      chSchReadyI(tp);
    }
#endif  /* LTDC_USE_WAIT */
    ltdcp->state = LTDC_READY;
    osalSysUnlockFromISR();

    LTDC->ICR |= LTDC_ICR_CRRIF;
  }

  OSAL_IRQ_EPILOGUE();
}

/**
 * @brief   LTDC error interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_LTDC_ER_HANDLER) {

  static LTDCDriver *const ltdcp = &LTDCD1;

  OSAL_IRQ_PROLOGUE();

  /* Handle FIFO Underrun ISR.*/
  if ((LTDC->ISR & LTDC_ISR_FUIF) && (LTDC->IER & LTDC_IER_FUIE)) {
    osalDbgAssert(ltdcp->config->fuerr_isr != NULL, "invalid state");
    ltdcp->config->fuerr_isr(ltdcp);
    LTDC->ICR |= LTDC_ICR_CFUIF;
  }

  /* Handle Transfer Error ISR.*/
  if ((LTDC->ISR & LTDC_ISR_TERRIF) && (LTDC->IER & LTDC_IER_TERRIE)) {
    osalDbgAssert(ltdcp->config->terr_isr != NULL, "invalid state");
    ltdcp->config->terr_isr(ltdcp);
    LTDC->ICR |= LTDC_ICR_CTERRIF;
  }

  OSAL_IRQ_EPILOGUE();
}

/** @} */

/**
 * @name    LTDC driver-specific methods
 * @{
 */

/**
 * @brief   LTDC Driver initialization.
 * @details Initializes the LTDC subsystem and chosen drivers. Should be
 *          called at board initialization.
 *
 * @init
 */
void ltdcInit(void) {

  /* Reset the LTDC hardware module.*/
  rccResetLTDC();

  /* Enable the LTDC clock.*/
  RCC->DCKCFGR = (RCC->DCKCFGR & ~RCC_DCKCFGR_PLLSAIDIVR) | (2 << 16); /* /8 */
  rccEnableLTDC(false);

  /* Driver struct initialization.*/
  ltdcObjectInit(&LTDCD1);
  LTDCD1.state = LTDC_STOP;
}

/**
 * @brief   Initializes the standard part of a @p LTDCDriver structure.
 *
 * @param[out] ltdcp    pointer to the @p LTDCDriver object
 *
 * @init
 */
void ltdcObjectInit(LTDCDriver *ltdcp) {

  osalDbgCheck(ltdcp == &LTDCD1);

  ltdcp->state = LTDC_UNINIT;
  ltdcp->config = NULL;
  ltdcp->active_window = ltdc_invalid_window;
#if (TRUE == LTDC_USE_WAIT)
  ltdcp->thread = NULL;
#endif  /* LTDC_USE_WAIT */
#if (TRUE == LTDC_USE_MUTUAL_EXCLUSION)
#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxObjectInit(&ltdcp->lock);
#else
  chSemObjectInit(&ltdcp->lock, 1);
#endif
#endif  /* LTDC_USE_MUTUAL_EXCLUSION */
}

/**
 * @brief   Get the driver state.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @retun               driver state
 *
 * @iclass
 */
ltdc_state_t ltdcGetStateI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);

  return ltdcp->state;
}

/**
 * @brief   Get the driver state.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @retun               driver state
 *
 * @api
 */
ltdc_state_t ltdcGetState(LTDCDriver *ltdcp) {

  ltdc_state_t state;
  osalSysLock();
  state = ltdcGetStateI(ltdcp);
  osalSysUnlock();
  return state;
}

/**
 * @brief   Configures and activates the LTDC peripheral.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] configp   pointer to the @p LTDCConfig object
 *
 * @api
 */
void ltdcStart(LTDCDriver *ltdcp, const LTDCConfig *configp) {

  uint32_t hacc, vacc, flags;

  osalSysLock();

  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(configp != NULL);
  osalDbgAssert(ltdcp->state == LTDC_STOP, "invalid state");

  ltdcp->config = configp;

  /* Turn off the controller and its interrupts.*/
  LTDC->GCR = 0;
  LTDC->IER = 0;
  ltdc_force_reload_s(ltdcp);

  /* Set synchronization params.*/
  osalDbgAssert(configp->hsync_width >= LTDC_MIN_HSYNC_WIDTH, "bounds");
  osalDbgAssert(configp->hsync_width <= LTDC_MAX_HSYNC_WIDTH, "bounds");
  osalDbgAssert(configp->vsync_height >= LTDC_MIN_VSYNC_HEIGHT, "bounds");
  osalDbgAssert(configp->vsync_height <= LTDC_MAX_VSYNC_HEIGHT, "bounds");

  hacc = configp->hsync_width - 1;
  vacc = configp->vsync_height - 1;

  LTDC->SSCR = (((hacc << 16) & LTDC_SSCR_HSW) |
                ((vacc <<  0) & LTDC_SSCR_VSH));

  /* Set accumulated back porch params.*/
  osalDbgAssert(configp->hbp_width >= LTDC_MIN_HBP_WIDTH, "bounds");
  osalDbgAssert(configp->hbp_width <= LTDC_MAX_HBP_WIDTH, "bounds");
  osalDbgAssert(configp->vbp_height >= LTDC_MIN_VBP_HEIGHT, "bounds");
  osalDbgAssert(configp->vbp_height <= LTDC_MAX_VBP_HEIGHT, "bounds");

  hacc += configp->hbp_width;
  vacc += configp->vbp_height;

  osalDbgAssert(hacc + 1 >= LTDC_MIN_ACC_HBP_WIDTH, "bounds");
  osalDbgAssert(hacc + 1 <= LTDC_MAX_ACC_HBP_WIDTH, "bounds");
  osalDbgAssert(vacc + 1 >= LTDC_MIN_ACC_VBP_HEIGHT, "bounds");
  osalDbgAssert(vacc + 1 <= LTDC_MAX_ACC_VBP_HEIGHT, "bounds");

  LTDC->BPCR = (((hacc << 16) & LTDC_BPCR_AHBP) |
                ((vacc <<  0) & LTDC_BPCR_AVBP));

  ltdcp->active_window.hstart = hacc + 1;
  ltdcp->active_window.vstart = vacc + 1;

  /* Set accumulated active params.*/
  osalDbgAssert(configp->screen_width >= LTDC_MIN_SCREEN_WIDTH, "bounds");
  osalDbgAssert(configp->screen_width <= LTDC_MAX_SCREEN_WIDTH, "bounds");
  osalDbgAssert(configp->screen_height >= LTDC_MIN_SCREEN_HEIGHT, "bounds");
  osalDbgAssert(configp->screen_height <= LTDC_MAX_SCREEN_HEIGHT, "bounds");

  hacc += configp->screen_width;
  vacc += configp->screen_height;

  osalDbgAssert(hacc + 1 >= LTDC_MIN_ACC_ACTIVE_WIDTH, "bounds");
  osalDbgAssert(hacc + 1 <= LTDC_MAX_ACC_ACTIVE_WIDTH, "bounds");
  osalDbgAssert(vacc + 1 >= LTDC_MIN_ACC_ACTIVE_HEIGHT, "bounds");
  osalDbgAssert(vacc + 1 <= LTDC_MAX_ACC_ACTIVE_HEIGHT, "bounds");

  LTDC->AWCR = (((hacc << 16) & LTDC_AWCR_AAW) |
                ((vacc <<  0) & LTDC_AWCR_AAH));

  ltdcp->active_window.hstop = hacc;
  ltdcp->active_window.vstop = vacc;

  /* Set accumulated total params.*/
  osalDbgAssert(configp->hfp_width >= LTDC_MIN_HFP_WIDTH, "bounds");
  osalDbgAssert(configp->hfp_width <= LTDC_MAX_HFP_WIDTH, "bounds");
  osalDbgAssert(configp->vfp_height >= LTDC_MIN_VFP_HEIGHT, "bounds");
  osalDbgAssert(configp->vfp_height <= LTDC_MAX_VFP_HEIGHT, "bounds");

  hacc += configp->hfp_width;
  vacc += configp->vfp_height;

  osalDbgAssert(hacc + 1 >= LTDC_MIN_ACC_TOTAL_WIDTH, "bounds");
  osalDbgAssert(hacc + 1 <= LTDC_MAX_ACC_TOTAL_WIDTH, "bounds");
  osalDbgAssert(vacc + 1 >= LTDC_MIN_ACC_TOTAL_HEIGHT, "bounds");
  osalDbgAssert(vacc + 1 <= LTDC_MAX_ACC_TOTAL_HEIGHT, "bounds");

  LTDC->TWCR = (((hacc << 16) & LTDC_TWCR_TOTALW) |
                ((vacc <<  0) & LTDC_TWCR_TOTALH));

  /* Set signal polarities and other flags.*/
  ltdcSetEnableFlagsI(ltdcp, configp->flags & ~LTDC_EF_ENABLE);

  /* Color settings.*/
  ltdcSetClearColorI(ltdcp, configp->clear_color);

  /* Load layer configurations.*/
  ltdcBgSetConfigI(ltdcp, configp->bg_laycfg);
  ltdcFgSetConfigI(ltdcp, configp->fg_laycfg);

  /* Enable only the assigned interrupt service routines.*/
  nvicEnableVector(STM32_LTDC_EV_NUMBER, STM32_LTDC_EV_IRQ_PRIORITY);
  nvicEnableVector(STM32_LTDC_ER_NUMBER, STM32_LTDC_ER_IRQ_PRIORITY);

  flags = LTDC_IER_RRIE;
  if (configp->line_isr != NULL)
    flags |= LTDC_IER_LIE;
  if (configp->fuerr_isr != NULL)
    flags |= LTDC_IER_FUIE;
  if (configp->terr_isr != NULL)
    flags |= LTDC_IER_TERRIE;
  LTDC->IER = flags;

  /* Apply settings.*/
  ltdc_force_reload_s(ltdcp);

  /* Turn on the controller.*/
  LTDC->GCR |= LTDC_GCR_LTDCEN;
  ltdc_force_reload_s(ltdcp);

  ltdcp->state = LTDC_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the LTDC peripheral.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcStop(LTDCDriver *ltdcp) {

  osalDbgCheck(ltdcp == &LTDCD1);

  osalSysLock();
  osalDbgAssert(ltdcp->state == LTDC_READY, "invalid state");

  /* Turn off the controller and its interrupts.*/
  LTDC->GCR &= ~LTDC_GCR_LTDCEN;
  LTDC->IER = 0;
#if (TRUE == LTDC_USE_WAIT)
  ltdcReloadS(ltdcp, true);
#else
  ltdcStartReloadI(ltdcp, true);
  while (ltdcIsReloadingI(ltdcp))
    chSchDoYieldS();
#endif  /* LTDC_USE_WAIT */

  ltdcp->state = LTDC_STOP;
  osalSysUnlock();
}

#if (TRUE == LTDC_USE_MUTUAL_EXCLUSION)

/**
 * @brief   Gains exclusive access to the LTDC module.
 * @details This function tries to gain ownership to the LTDC module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p LTDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @sclass
 */
void ltdcAcquireBusS(LTDCDriver *ltdcp) {

  osalDbgCheckClassS();
  osalDbgCheck(ltdcp == &LTDCD1);

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxLockS(&ltdcp->lock);
#else
  chSemWaitS(&ltdcp->lock);
#endif
}

/**
 * @brief   Gains exclusive access to the LTDC module.
 * @details This function tries to gain ownership to the LTDC module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p LTDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcAcquireBus(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcAcquireBusS(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Releases exclusive access to the LTDC module.
 * @pre     In order to use this function the option
 *          @p LTDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @sclass
 */
void ltdcReleaseBusS(LTDCDriver *ltdcp) {

  osalDbgCheckClassS();
  osalDbgCheck(ltdcp == &LTDCD1);

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxUnlockS(&ltdcp->lock);
#else
  chSemSignalI(&ltdcp->lock);
#endif
}

/**
 * @brief   Releases exclusive access to the LTDC module.
 * @pre     In order to use this function the option
 *          @p LTDC_USE_MUTUAL_EXCLUSION must be enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcReleaseBus(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcReleaseBusS(ltdcp);
  osalSysUnlock();
}

#endif  /* LTDC_USE_MUTUAL_EXCLUSION */

/** @} */

/**
 * @name    LTDC global methods
 * @{
 */

/**
 * @brief   Get enabled flags.
 * @details Returns all the flags of the <tt>LTDC_EF_*</tt> group at once.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @iclass
 */
ltdc_flags_t ltdcGetEnableFlagsI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return LTDC->GCR & LTDC_EF_MASK;
}

/**
 * @brief   Get enabled flags.
 * @details Returns all the flags of the <tt>LTDC_EF_*</tt> group at once.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @api
 */
ltdc_flags_t ltdcGetEnableFlags(LTDCDriver *ltdcp) {

  ltdc_flags_t flags;
  osalSysLock();
  flags = ltdcGetEnableFlagsI(ltdcp);
  osalSysUnlock();
  return flags;
}

/**
 * @brief   Set enabled flags.
 * @details Sets all the flags of the <tt>LTDC_EF_*</tt> group at once.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @iclass
 */
void ltdcSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->GCR = flags & LTDC_EF_MASK;
}

/**
 * @brief   Set enabled flags.
 * @details Sets all the flags of the <tt>LTDC_EF_*</tt> group at once.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @api
 */
void ltdcSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalSysLock();
  ltdcSetEnableFlagsI(ltdcp, flags);
  osalSysUnlock();
}

/**
 * @brief   Reloading shadow registers.
 * @details Tells whether the LTDC is reloading shadow registers.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              reloading
 *
 * @iclass
 */
bool ltdcIsReloadingI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC->SRCR & (LTDC_SRCR_IMR | LTDC_SRCR_VBR)) != 0;
}

/**
 * @brief   Reloading shadow registers.
 * @details Tells whether the LTDC is reloading shadow registers.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              reloading
 *
 * @api
 */
bool ltdcIsReloading(LTDCDriver *ltdcp) {

  bool reloading;
  osalSysLock();
  reloading = ltdcIsReloadingI(ltdcp);
  osalSysUnlock();
  return reloading;
}

/**
 * @brief   Reload shadow registers.
 * @details Starts reloading LTDC shadow registers, upon vsync or immediately.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] ltdcp         pointer to the @p LTDCDriver object
 * @param[in] immediately   reload immediately, not upon vsync
 *
 * @iclass
 */
void ltdcStartReloadI(LTDCDriver *ltdcp, bool immediately) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgAssert(ltdcp->state == LTDC_READY, "not ready");
  (void)ltdcp;

  ltdcp->state = LTDC_ACTIVE;
  if (immediately)
    LTDC->SRCR |= LTDC_SRCR_IMR;
  else
    LTDC->SRCR |= LTDC_SRCR_VBR;
}

/**
 * @brief   Reload shadow registers.
 * @details Starts reloading LTDC shadow registers, upon vsync or immediately.
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] ltdcp         pointer to the @p LTDCDriver object
 * @param[in] immediately   reload immediately, not upon vsync
 *
 * @api
 */
void ltdcStartReload(LTDCDriver *ltdcp, bool immediately) {

  osalSysLock();
  ltdcStartReloadI(ltdcp, immediately);
  osalSysUnlock();
}

/**
 * @brief   Reload shadow registers.
 * @details Reloads LTDC shadow registers, upon vsync or immediately.
 *
 * @param[in] ltdcp         pointer to the @p LTDCDriver object
 * @param[in] immediately   reload immediately, not upon vsync
 *
 * @sclass
 */
void ltdcReloadS(LTDCDriver *ltdcp, bool immediately) {

  osalDbgCheckClassS();
  osalDbgCheck(ltdcp == &LTDCD1);

  ltdcStartReloadI(ltdcp, immediately);

#if (TRUE == LTDC_USE_WAIT)
  osalDbgAssert(ltdcp->thread == NULL, "already waiting");

  if (immediately) {
    while (LTDC->SRCR & LTDC_SRCR_IMR)
      chSchDoYieldS();
    ltdcp->state = LTDC_READY;
  } else {
    ltdcp->thread = chThdGetSelfX();
    chSchGoSleepS(CH_STATE_SUSPENDED);
  }
#else
  while (LTDC->SRCR & LTDC_SRCR_IMR)
    chSchDoYieldS();
  ltdcp->state = LTDC_READY;
#endif
}

/**
 * @brief   Reload shadow registers.
 * @details Reloads LTDC shadow registers, upon vsync or immediately.
 *
 * @param[in] ltdcp         pointer to the @p LTDCDriver object
 * @param[in] immediately   reload immediately, not upon vsync
 *
 * @api
 */
void ltdcReload(LTDCDriver *ltdcp, bool immediately) {

  osalSysLock();
  ltdcReloadS(ltdcp, immediately);
  osalSysUnlock();
}

/**
 * @brief   Dithering enabled.
 * @details Tells whether the dithering is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcIsDitheringEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC->GCR & LTDC_GCR_DTEN) != 0;
}

/**
 * @brief   Dithering enabled.
 * @details Tells whether the dithering is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcIsDitheringEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcIsDitheringEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable dithering.
 * @details Enables dithering capabilities for pixel formats with less than
 *          8 bits per channel.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcEnableDitheringI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->GCR |= LTDC_GCR_DTEN;
}

/**
 * @brief   Enable dithering.
 * @details Enables dithering capabilities for pixel formats with less than
 *          8 bits per channel.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcEnableDithering(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcEnableDitheringI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable dithering.
 * @details Disables dithering capabilities for pixel formats with less than
 *          8 bits per channel.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcDisableDitheringI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->GCR &= ~LTDC_GCR_DTEN;
}

/**
 * @brief   Disable dithering.
 * @details Disables dithering capabilities for pixel formats with less than
 *          8 bits per channel.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcDisableDithering(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcDisableDitheringI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Get clear screen color.
 * @details Gets the clear screen (actual background) color.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              clear screen color, RGB-888
 *
 * @iclass
 */
ltdc_color_t ltdcGetClearColorI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_color_t)(LTDC->BCCR & 0x00FFFFFF);
}

/**
 * @brief   Get clear screen color.
 * @details Gets the clear screen (actual background) color.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              clear screen color, RGB-888
 *
 * @api
 */
ltdc_color_t ltdcGetClearColor(LTDCDriver *ltdcp) {

  ltdc_color_t color;
  osalSysLock();
  color = ltdcGetClearColorI(ltdcp);
  osalSysUnlock();
  return color;
}

/**
 * @brief   Set clear screen color.
 * @details Sets the clear screen (actual background) color.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         clear screen color, RGB-888
 *
 * @iclass
 */
void ltdcSetClearColorI(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->BCCR = (LTDC->BCCR & ~0x00FFFFFF) | (c & 0x00FFFFFF);
}

/**
 * @brief   Set clear screen color.
 * @details Sets the clear screen (actual background) color.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         clear screen color, RGB-888
 *
 * @api
 */
void ltdcSetClearColor(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalSysLock();
  ltdcSetClearColorI(ltdcp, c);
  osalSysUnlock();
}

/**
 * @brief   Get line interrupt position.
 * @details Gets the line interrupt position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              line interrupt position
 *
 * @iclass
 */
uint16_t ltdcGetLineInterruptPosI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (uint16_t)(LTDC->LIPCR & LTDC_LIPCR_LIPOS);
}

/**
 * @brief   Get line interrupt position.
 * @details Gets the line interrupt position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              line interrupt position
 *
 * @api
 */
uint16_t ltdcGetLineInterruptPos(LTDCDriver *ltdcp) {

  uint16_t line;
  osalSysLock();
  line = ltdcGetLineInterruptPosI(ltdcp);
  osalSysUnlock();
  return line;
}

/**
 * @brief   Set line interrupt position.
 * @details Sets the line interrupt position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcSetLineInterruptPosI(LTDCDriver *ltdcp, uint16_t line) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->LIPCR = ((LTDC->LIPCR & ~LTDC_LIPCR_LIPOS) |
                 ((uint32_t)line & LTDC_LIPCR_LIPOS));
}

/**
 * @brief   Set line interrupt position.
 * @details Sets the line interrupt position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcSetLineInterruptPos(LTDCDriver *ltdcp, uint16_t line) {

  osalSysLock();
  ltdcSetLineInterruptPosI(ltdcp, line);
  osalSysUnlock();
}

/**
 * @brief   Line interrupt enabled.
 * @details Tells whether the line interrupt is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcIsLineInterruptEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC->IER & LTDC_IER_LIE) != 0;
}

/**
 * @brief   Line interrupt enabled.
 * @details Tells whether the line interrupt is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcIsLineInterruptEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcIsLineInterruptEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable line interrupt.
 * @details Enables line interrupt.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcEnableLineInterruptI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->IER |= LTDC_IER_LIE;
}

/**
 * @brief   Enable line interrupt.
 * @details Enables line interrupt.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcEnableLineInterrupt(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcEnableLineInterruptI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable line interrupt.
 * @details Disables line interrupt.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcDisableLineInterruptI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC->IER &= ~LTDC_IER_LIE;
}

/**
 * @brief   Disable line interrupt.
 * @details Disables line interrupt.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcDisableLineInterrupt(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcDisableLineInterruptI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Get current position.
 * @details Gets the current position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] xp       pointer to the destination horizontal coordinate
 * @param[out] yp       pointer to the destination vertical coordinate
 *
 * @iclass
 */
void ltdcGetCurrentPosI(LTDCDriver *ltdcp, uint16_t *xp, uint16_t *yp) {

  const uint32_t r = LTDC->CPSR;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  *xp = (uint16_t)((r & LTDC_CPSR_CXPOS) >> 16);
  *yp = (uint16_t)((r & LTDC_CPSR_CYPOS) >>  0);
}

/**
 * @brief   Get current position.
 * @details Gets the current position.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] xp       pointer to the destination horizontal coordinate
 * @param[out] yp       pointer to the destination vertical coordinate
 *
 * @api
 */
void ltdcGetCurrentPos(LTDCDriver *ltdcp, uint16_t *xp, uint16_t *yp) {

  osalSysLock();
  ltdcGetCurrentPosI(ltdcp, xp, yp);
  osalSysUnlock();
}

/** @} */

/**
 * @name    LTDC background layer (layer 1) methods
 * @{
 */

/**
 * @brief   Get background layer enabled flags.
 * @details Returns all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @iclass
 */
ltdc_flags_t ltdcBgGetEnableFlagsI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return LTDC_Layer1->CR & LTDC_LEF_MASK;
}

/**
 * @brief   Get background layer enabled flags.
 * @details Returns all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @api
 */
ltdc_flags_t ltdcBgGetEnableFlags(LTDCDriver *ltdcp) {

  ltdc_flags_t flags;
  osalSysLock();
  flags = ltdcBgGetEnableFlagsI(ltdcp);
  osalSysUnlock();
  return flags;
}

/**
 * @brief   Set background layer enabled flags.
 * @details Sets all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @iclass
 */
void ltdcBgSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR = ((LTDC_Layer1->CR & ~LTDC_LEF_MASK) |
                     ((uint32_t)flags & LTDC_LEF_MASK));
}

/**
 * @brief   Set background layer enabled flags.
 * @details Sets all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @api
 */
void ltdcBgSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalSysLock();
  ltdcBgSetEnableFlagsI(ltdcp, flags);
  osalSysUnlock();
}

/**
 * @brief   Background layer enabled.
 * @details Tells whether the background layer (layer 1) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcBgIsEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer1->CR & ~LTDC_LxCR_LEN) != 0;
}

/**
 * @brief   Background layer enabled.
 * @details Tells whether the background layer (layer 1) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcBgIsEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcBgIsEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Background layer enable.
 * @details Enables the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgEnableI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR |= LTDC_LxCR_LEN;
}

/**
 * @brief   Background layer enable.
 * @details Enables the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgEnable(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgEnableI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Background layer disable.
 * @details Disables the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgDisableI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR &= ~LTDC_LxCR_LEN;
}

/**
 * @brief   Background layer disable.
 * @details Disables the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgDisable(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgDisableI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Background layer palette enabled.
 * @details Tells whether the background layer (layer 1) palette (color lookup
 *          table) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcBgIsPaletteEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer1->CR & ~LTDC_LxCR_CLUTEN) != 0;
}

/**
 * @brief   Background layer palette enabled.
 * @details Tells whether the background layer (layer 1) palette (color lookup
 *          table) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcBgIsPaletteEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcBgIsPaletteEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable background layer palette.
 * @details Enables the palette (color lookup table) of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgEnablePaletteI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR |= LTDC_LxCR_CLUTEN;
}

/**
 * @brief   Enable background layer palette.
 * @details Enables the palette (color lookup table) of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgEnablePalette(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgEnablePaletteI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable background layer palette.
 * @details Disables the palette (color lookup table) of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgDisablePaletteI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR &= ~LTDC_LxCR_CLUTEN;
}

/**
 * @brief   Disable background layer palette.
 * @details Disables the palette (color lookup table) of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgDisablePalette(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgDisablePaletteI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Set background layer palette color.
 * @details Sets the color of a palette (color lookup table) slot to the
 *          background layer (layer 1).
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] slot      palette slot
 * @param[in] c         color, RGB-888
 *
 * @iclass
 */
void ltdcBgSetPaletteColorI(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgAssert(!ltdcBgIsEnabledI(ltdcp), "invalid state");
  (void)ltdcp;

  LTDC_Layer1->CLUTWR = (((uint32_t)slot << 24) | (c & 0x00FFFFFF));
}

/**
 * @brief   Set background layer palette color.
 * @details Sets the color of a palette (color lookup table) slot to the
 *          background layer (layer 1).
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] slot      palette slot
 * @param[in] c         color, RGB-888
 *
 * @api
 */
void ltdcBgSetPaletteColor(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c) {

  osalSysLock();
  ltdcBgSetPaletteColorI(ltdcp, slot, c);
  osalSysUnlock();
}

/**
 * @brief   Set background layer palette.
 * @details Sets the entire palette color (color lookup table) slot.
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] colors    array of palette colors, RGB-888
 * @param[in] length    number of palette colors
 *
 * @iclass
 */
void ltdcBgSetPaletteI(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                       uint16_t length) {

  uint16_t i;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck((colors == NULL) == (length == 0));
  osalDbgAssert(length <= LTDC_MAX_PALETTE_LENGTH, "bounds");
  osalDbgAssert(!ltdcBgIsEnabledI(ltdcp), "invalid state");
  (void)ltdcp;

  for (i = 0; i < length; ++i)
    LTDC_Layer1->CLUTWR = (((uint32_t)i << 24) | (colors[i] & 0x00FFFFFF));
}

/**
 * @brief   Set background layer palette.
 * @details Sets the entire palette color (color lookup table) slot.
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] colors    array of palette colors, RGB-888
 * @param[in] length    number of palette colors
 *
 * @api
 */
void ltdcBgSetPalette(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                      uint16_t length) {

  osalSysLock();
  ltdcBgSetPaletteI(ltdcp, colors, length);
  osalSysUnlock();
}

/**
 * @brief   Get background layer pixel format.
 * @details Gets the pixel format of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              pixel format
 *
 * @iclass
 */
ltdc_pixfmt_t ltdcBgGetPixelFormatI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_pixfmt_t)(LTDC_Layer1->PFCR & LTDC_LxPFCR_PF);
}

/**
 * @brief   Get background layer pixel format.
 * @details Gets the pixel format of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              pixel format
 *
 * @api
 */
ltdc_pixfmt_t ltdcBgGetPixelFormat(LTDCDriver *ltdcp) {

  ltdc_pixfmt_t fmt;
  osalSysLock();
  fmt = ltdcBgGetPixelFormatI(ltdcp);
  osalSysUnlock();
  return fmt;
}

/**
 * @brief   Set background layer pixel format.
 * @details Sets the pixel format of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] fmt       pixel format
 *
 * @iclass
 */
void ltdcBgSetPixelFormatI(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgAssert(fmt >= LTDC_MIN_PIXFMT_ID, "bounds");
  osalDbgAssert(fmt <= LTDC_MAX_PIXFMT_ID, "bounds");
  (void)ltdcp;

  LTDC_Layer1->PFCR = ((LTDC_Layer1->PFCR & ~LTDC_LxPFCR_PF) |
                       ((uint32_t)fmt & LTDC_LxPFCR_PF));
}

/**
 * @brief   Set background layer pixel format.
 * @details Sets the pixel format of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] fmt       pixel format
 *
 * @api
 */
void ltdcBgSetPixelFormat(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt) {

  osalSysLock();
  ltdcBgSetPixelFormatI(ltdcp, fmt);
  osalSysUnlock();
}

/**
 * @brief   Background layer color keying enabled.
 * @details Tells whether the background layer (layer 1) has color keying
 *          enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcBgIsKeyingEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer1->CR & ~LTDC_LxCR_COLKEN) != 0;
}

/**
 * @brief   Background layer color keying enabled.
 * @details Tells whether the background layer (layer 1) has color keying
 *          enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcBgIsKeyingEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcBgIsKeyingEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable background layer color keying.
 * @details Enables color keying capabilities of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgEnableKeyingI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR |= LTDC_LxCR_COLKEN;
}

/**
 * @brief   Enable background layer color keying.
 * @details Enables color keying capabilities of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgEnableKeying(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgEnableKeyingI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable background layer color keying.
 * @details Disables color keying capabilities of the background layer (layer
 *          1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgDisableKeyingI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CR &= ~LTDC_LxCR_COLKEN;
}

/**
 * @brief   Disable background layer color keying.
 * @details Disables color keying capabilities of the background layer (layer
 *          1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgDisableKeying(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgDisableKeyingI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Get background layer color key.
 * @details Gets the color key of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              color key, RGB-888
 *
 * @iclass
 */
ltdc_color_t ltdcBgGetKeyingColorI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_color_t)(LTDC_Layer1->CKCR & 0x00FFFFFF);
}

/**
 * @brief   Get background layer color key.
 * @details Gets the color key of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              color key, RGB-888
 *
 * @api
 */
ltdc_color_t ltdcBgGetKeyingColor(LTDCDriver *ltdcp) {

  ltdc_color_t color;
  osalSysLock();
  color = ltdcBgGetKeyingColorI(ltdcp);
  osalSysUnlock();
  return color;
}

/**
 * @brief   Set background layer color key.
 * @details Sets the color key of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         color key, RGB-888
 *
 * @iclass
 */
void ltdcBgSetKeyingColorI(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CKCR = ((LTDC_Layer1->CKCR & ~0x00FFFFFF) |
                       ((uint32_t)c & 0x00FFFFFF));
}

/**
 * @brief   Set background layer color key.
 * @details Sets the color key of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         color key, RGB-888
 *
 * @api
 */
void ltdcBgSetKeyingColor(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalSysLock();
  ltdcBgSetKeyingColorI(ltdcp, c);
  osalSysUnlock();
}

/**
 * @brief   Get background layer constant alpha.
 * @details Gets the constant alpha component of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @iclass
 */
uint8_t ltdcBgGetConstantAlphaI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (uint8_t)(LTDC_Layer1->CACR & LTDC_LxCACR_CONSTA);
}

/**
 * @brief   Get background layer constant alpha.
 * @details Gets the constant alpha component of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @api
 */
uint8_t ltdcBgGetConstantAlpha(LTDCDriver *ltdcp) {

  uint8_t a;
  osalSysLock();
  a = ltdcBgGetConstantAlphaI(ltdcp);
  osalSysUnlock();
  return a;
}

/**
 * @brief   Set background layer constant alpha.
 * @details Sets the constant alpha component of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @iclass
 */
void ltdcBgSetConstantAlphaI(LTDCDriver *ltdcp, uint8_t a) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CACR = ((LTDC_Layer1->CACR & ~LTDC_LxCACR_CONSTA) |
                       ((uint32_t)a & LTDC_LxCACR_CONSTA));
}

/**
 * @brief   Set background layer constant alpha.
 * @details Sets the constant alpha component of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @api
 */
void ltdcBgSetConstantAlpha(LTDCDriver *ltdcp, uint8_t a) {

  osalSysLock();
  ltdcBgSetConstantAlphaI(ltdcp, a);
  osalSysUnlock();
}

/**
 * @brief   Get background layer default color.
 * @details Gets the default color of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              default color, RGB-888
 *
 * @iclass
 */
ltdc_color_t ltdcBgGetDefaultColorI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_color_t)LTDC_Layer1->DCCR;
}

/**
 * @brief   Get background layer default color.
 * @details Gets the default color of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              default color, RGB-888
 *
 * @api
 */
ltdc_color_t ltdcBgGetDefaultColor(LTDCDriver *ltdcp) {

  ltdc_color_t color;
  osalSysLock();
  color = ltdcBgGetDefaultColorI(ltdcp);
  osalSysUnlock();
  return color;
}

/**
 * @brief   Set background layer default color.
 * @details Sets the default color of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         default color, RGB-888
 *
 * @iclass
 */
void ltdcBgSetDefaultColorI(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->DCCR = (uint32_t)c;
}

/**
 * @brief   Set background layer default color.
 * @details Sets the default color of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         default color, RGB-888
 *
 * @api
 */
void ltdcBgSetDefaultColor(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalSysLock();
  ltdcBgSetDefaultColorI(ltdcp, c);
  osalSysUnlock();
}

/**
 * @brief   Get background layer blending factors.
 * @details Gets the blending factors of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              blending factors
 *
 * @iclass
 */
ltdc_blendf_t ltdcBgGetBlendingFactorsI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_blendf_t)(LTDC_Layer1->BFCR & LTDC_LxBFCR_BF);
}

/**
 * @brief   Get background layer blending factors.
 * @details Gets the blending factors of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              blending factors
 *
 * @api
 */
ltdc_blendf_t ltdcBgGetBlendingFactors(LTDCDriver *ltdcp) {

  ltdc_blendf_t bf;
  osalSysLock();
  bf = ltdcBgGetBlendingFactorsI(ltdcp);
  osalSysUnlock();
  return bf;
}

/**
 * @brief   Set background layer blending factors.
 * @details Sets the blending factors of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] factors   blending factors
 *
 * @iclass
 */
void ltdcBgSetBlendingFactorsI(LTDCDriver *ltdcp, ltdc_blendf_t bf) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->BFCR = ((LTDC_Layer1->BFCR & ~LTDC_LxBFCR_BF) |
                       ((uint32_t)bf & LTDC_LxBFCR_BF));
}

/**
 * @brief   Set background layer blending factors.
 * @details Sets the blending factors of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] factors   blending factors
 *
 * @api
 */
void ltdcBgSetBlendingFactors(LTDCDriver *ltdcp, ltdc_blendf_t bf) {

  osalSysLock();
  ltdcBgSetBlendingFactorsI(ltdcp, bf);
  osalSysUnlock();
}

/**
 * @brief   Get background layer window specs.
 * @details Gets the window specifications of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] windowp  pointer to the window specifications
 *
 * @iclass
 */
void ltdcBgGetWindowI(LTDCDriver *ltdcp, ltdc_window_t *windowp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(windowp != NULL);
  (void)ltdcp;

  windowp->hstart =
    (uint16_t)((LTDC_Layer1->WHPCR & LTDC_LxWHPCR_WHSTPOS) >>  0);
  windowp->hstop =
    (uint16_t)((LTDC_Layer1->WHPCR & LTDC_LxWHPCR_WHSPPOS) >> 16);
  windowp->vstart =
    (uint16_t)((LTDC_Layer1->WVPCR & LTDC_LxWVPCR_WVSTPOS) >>  0);
  windowp->vstop =
    (uint16_t)((LTDC_Layer1->WVPCR & LTDC_LxWVPCR_WVSPPOS) >> 16);
}

/**
 * @brief   Get background layer window specs.
 * @details Gets the window specifications of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] windowp  pointer to the window specifications
 *
 * @api
 */
void ltdcBgGetWindow(LTDCDriver *ltdcp, ltdc_window_t *windowp) {

  osalSysLock();
  ltdcBgGetWindowI(ltdcp, windowp);
  osalSysUnlock();
}

/**
 * @brief   Set background layer window specs.
 * @details Sets the window specifications of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] windowp   pointer to the window specifications
 *
 * @iclass
 */
void ltdcBgSetWindowI(LTDCDriver *ltdcp, const ltdc_window_t *windowp) {

  uint32_t start, stop;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(windowp != NULL);
  (void)ltdcp;

  osalDbgAssert(windowp->hstop < ltdcp->config->screen_width, "bounds");
  osalDbgAssert(windowp->vstop < ltdcp->config->screen_height, "bounds");

  /* Horizontal boundaries.*/
  start = (uint32_t)windowp->hstart + ltdcp->active_window.hstart;
  stop  = (uint32_t)windowp->hstop  + ltdcp->active_window.hstart;

  osalDbgAssert(start >= ltdcp->active_window.hstart, "bounds");
  osalDbgAssert(stop <= ltdcp->active_window.hstop, "bounds");

  LTDC_Layer1->WHPCR = (((start <<  0) & LTDC_LxWHPCR_WHSTPOS) |
                        ((stop  << 16) & LTDC_LxWHPCR_WHSPPOS));

  /* Vertical boundaries.*/
  start = (uint32_t)windowp->vstart + ltdcp->active_window.vstart;
  stop  = (uint32_t)windowp->vstop  + ltdcp->active_window.vstart;

  osalDbgAssert(start >= ltdcp->active_window.vstart, "bounds");
  osalDbgAssert(stop <= ltdcp->active_window.vstop, "bounds");

  LTDC_Layer1->WVPCR = (((start <<  0) & LTDC_LxWVPCR_WVSTPOS) |
                        ((stop  << 16) & LTDC_LxWVPCR_WVSPPOS));
}

/**
 * @brief   Set background layer window specs.
 * @details Sets the window specifications of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] windowp   pointer to the window specifications
 *
 * @api
 */
void ltdcBgSetWindow(LTDCDriver *ltdcp, const ltdc_window_t *windowp) {

  osalSysLock();
  ltdcBgSetWindowI(ltdcp, windowp);
  osalSysUnlock();
}

/**
 * @brief   Set background layer window as invalid.
 * @details Sets the window specifications of the background layer (layer 1)
 *          so that the window is pixel sized at the screen origin.
 * @note    Useful before reconfiguring the frame specifications of the layer,
 *          to avoid errors.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcBgSetInvalidWindowI(LTDCDriver *ltdcp) {

  ltdcBgSetWindowI(ltdcp, &ltdc_invalid_window);
}

/**
 * @brief   Set background layer window as invalid.
 * @details Sets the window specifications of the background layer (layer 1)
 *          so that the window is pixel sized at the screen origin.
 * @note    Useful before reconfiguring the frame specifications of the layer,
 *          to avoid errors.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcBgSetInvalidWindow(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcBgSetWindowI(ltdcp, &ltdc_invalid_window);
  osalSysUnlock();
}

/**
 * @brief   Get background layer frame buffer specs.
 * @details Gets the frame buffer specifications of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] framep   pointer to the frame buffer specifications
 *
 * @iclass
 */
void ltdcBgGetFrameI(LTDCDriver *ltdcp, ltdc_frame_t *framep) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(framep != NULL);

  framep->bufferp = (void *)(LTDC_Layer1->CFBAR & LTDC_LxCFBAR_CFBADD);
  framep->pitch   = (size_t)((LTDC_Layer1->CFBLR & LTDC_LxCFBLR_CFBP) >> 16);
  framep->width   = (uint16_t)(((LTDC_Layer1->CFBLR & LTDC_LxCFBLR_CFBLL) - 3) /
                    ltdcBytesPerPixel(ltdcBgGetPixelFormatI(ltdcp)));
  framep->height  = (uint16_t)(LTDC_Layer1->CFBLNR & LTDC_LxCFBLNR_CFBLNBR);
}

/**
 * @brief   Get background layer frame buffer specs.
 * @details Gets the frame buffer specifications of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] framep   pointer to the frame buffer specifications
 *
 * @api
 */
void ltdcBgGetFrame(LTDCDriver *ltdcp, ltdc_frame_t *framep) {

  osalSysLock();
  ltdcBgGetFrameI(ltdcp, framep);
  osalSysUnlock();
}

/**
 * @brief   Set background layer frame buffer specs.
 * @details Sets the frame buffer specifications of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] framep    pointer to the frame buffer specifications
 *
 * @iclass
 */
void ltdcBgSetFrameI(LTDCDriver *ltdcp, const ltdc_frame_t *framep) {

  size_t linesize;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(framep != NULL);

  ltdcBgSetPixelFormatI(ltdcp, framep->fmt);

  linesize = ltdcBytesPerPixel(framep->fmt) * framep->width;

  osalDbgAssert(framep->width  <= ltdcp->config->screen_width, "bounds");
  osalDbgAssert(framep->height <= ltdcp->config->screen_height, "bounds");
  osalDbgAssert(linesize >= LTDC_MIN_FRAME_WIDTH_BYTES, "bounds");
  osalDbgAssert(linesize <= LTDC_MAX_FRAME_WIDTH_BYTES, "bounds");
  osalDbgAssert(framep->height >= LTDC_MIN_FRAME_HEIGHT_LINES, "bounds");
  osalDbgAssert(framep->height <= LTDC_MAX_FRAME_HEIGHT_LINES, "bounds");
  osalDbgAssert(framep->pitch  >= linesize, "bounds");

  LTDC_Layer1->CFBAR = (uint32_t)framep->bufferp & LTDC_LxCFBAR_CFBADD;
  LTDC_Layer1->CFBLR = ((((uint32_t)framep->pitch << 16) & LTDC_LxCFBLR_CFBP) |
                        ((linesize + 3) & LTDC_LxCFBLR_CFBLL));
  LTDC_Layer1->CFBLNR = (uint32_t)framep->height & LTDC_LxCFBLNR_CFBLNBR;
}

/**
 * @brief   Set background layer frame buffer specs.
 * @details Sets the frame buffer specifications of the background layer
 *          (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] framep    pointer to the frame buffer specifications
 *
 * @api
 */
void ltdcBgSetFrame(LTDCDriver *ltdcp, const ltdc_frame_t *framep) {

  osalSysLock();
  ltdcBgSetFrameI(ltdcp, framep);
  osalSysUnlock();
}

/**
 * @brief   Get background layer frame buffer address.
 * @details Gets the frame buffer address of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              frame buffer address
 *
 * @iclass
 */
void *ltdcBgGetFrameAddressI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (void *)LTDC_Layer1->CFBAR;
}

/**
 * @brief   Get background layer frame buffer address.
 * @details Gets the frame buffer address of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              frame buffer address
 *
 * @api
 */
void *ltdcBgGetFrameAddress(LTDCDriver *ltdcp) {

  void *bufferp;
  osalSysLock();
  bufferp = ltdcBgGetFrameAddressI(ltdcp);
  osalSysUnlock();
  return bufferp;
}

/**
 * @brief   Set background layer frame buffer address.
 * @details Sets the frame buffer address of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] bufferp   frame buffer address
 *
 * @iclass
 */
void ltdcBgSetFrameAddressI(LTDCDriver *ltdcp, void *bufferp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer1->CFBAR = (uint32_t)bufferp;
}

/**
 * @brief   Set background layer frame buffer address.
 * @details Sets the frame buffer address of the background layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] bufferp   frame buffer address
 *
 * @api
 */
void ltdcBgSetFrameAddress(LTDCDriver *ltdcp, void *bufferp) {

  osalSysLock();
  ltdcBgSetFrameAddressI(ltdcp, bufferp);
  osalSysUnlock();
}

/**
 * @brief   Get background layer specifications.
 * @details Gets the background layer (layer 1) specifications at once.
 * @note    If palette specifications cannot be retrieved, they are set to
 *          @p NULL. This is not an error.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @iclass
 */
void ltdcBgGetLayerI(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(cfgp != NULL);

  ltdcBgGetFrameI(ltdcp, (ltdc_frame_t *)cfgp->frame);
  ltdcBgGetWindowI(ltdcp, (ltdc_window_t *)cfgp->window);
  cfgp->def_color = ltdcBgGetDefaultColorI(ltdcp);
  cfgp->key_color = ltdcBgGetKeyingColorI(ltdcp);
  cfgp->const_alpha = ltdcBgGetConstantAlphaI(ltdcp);
  cfgp->blending = ltdcBgGetBlendingFactorsI(ltdcp);

  cfgp->pal_colors = NULL;
  cfgp->pal_length = 0;

  cfgp->flags = ltdcBgGetEnableFlagsI(ltdcp);
}

/**
 * @brief   Get background layer specifications.
 * @details Gets the background layer (layer 1) specifications at once.
 * @note    If palette specifications cannot be retrieved, they are set to
 *          @p NULL. This is not an error.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @api
 */
void ltdcBgGetLayer(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp) {

  osalSysLock();
  ltdcBgGetLayerI(ltdcp, cfgp);
  osalSysUnlock();
}

/**
 * @brief   Set background layer specifications.
 * @details Sets the background layer (layer 1) specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @iclass
 */
void ltdcBgSetConfigI(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);

  if (cfgp == NULL)
    cfgp = &ltdc_default_laycfg;

  osalDbgCheck((cfgp->pal_colors == NULL) == (cfgp->pal_length == 0));

  ltdcBgSetFrameI(ltdcp, cfgp->frame);
  ltdcBgSetWindowI(ltdcp, cfgp->window);
  ltdcBgSetDefaultColorI(ltdcp, cfgp->def_color);
  ltdcBgSetKeyingColorI(ltdcp, cfgp->key_color);
  ltdcBgSetConstantAlphaI(ltdcp, cfgp->const_alpha);
  ltdcBgSetBlendingFactorsI(ltdcp, cfgp->blending);

  if (cfgp->pal_length > 0)
    ltdcBgSetPaletteI(ltdcp, cfgp->pal_colors, cfgp->pal_length);

  ltdcBgSetEnableFlagsI(ltdcp, cfgp->flags);
}

/**
 * @brief   Set background layer specifications.
 * @details Sets the background layer (layer 1) specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @api
 */
void ltdcBgSetConfig(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp) {

  osalSysLock();
  ltdcBgSetConfigI(ltdcp, cfgp);
  osalSysUnlock();
}

/** @} */

/**
 * @name    LTDC foreground layer (layer 2) methods
 * @{
 */

/**
 * @brief   Get foreground layer enabled flags.
 * @details Returns all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @iclass
 */
ltdc_flags_t ltdcFgGetEnableFlagsI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return LTDC_Layer2->CR & LTDC_LEF_MASK;
}

/**
 * @brief   Get foreground layer enabled flags.
 * @details Returns all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled flags
 *
 * @api
 */
ltdc_flags_t ltdcFgGetEnableFlags(LTDCDriver *ltdcp) {

  ltdc_flags_t flags;
  osalSysLock();
  flags = ltdcFgGetEnableFlagsI(ltdcp);
  osalSysUnlock();
  return flags;
}

/**
 * @brief   Set foreground layer enabled flags.
 * @details Sets all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @iclass
 */
void ltdcFgSetEnableFlagsI(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR = ((LTDC_Layer2->CR & ~LTDC_LEF_MASK) |
                     ((uint32_t)flags & LTDC_LEF_MASK));
}

/**
 * @brief   Set foreground layer enabled flags.
 * @details Sets all the flags of the <tt>LTDC_LEF_*</tt> group at once.
 *          Targeting the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] flags     enabled flags
 *
 * @api
 */
void ltdcFgSetEnableFlags(LTDCDriver *ltdcp, ltdc_flags_t flags) {

  osalSysLock();
  ltdcFgSetEnableFlagsI(ltdcp, flags);
  osalSysUnlock();
}

/**
 * @brief   Foreground layer enabled.
 * @details Tells whether the foreground layer (layer 2) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcFgIsEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer2->CR & ~LTDC_LxCR_LEN) != 0;
}

/**
 * @brief   Foreground layer enabled.
 * @details Tells whether the foreground layer (layer 2) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcFgIsEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcFgIsEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Foreground layer enable.
 * @details Enables the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgEnableI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR |= LTDC_LxCR_LEN;
}

/**
 * @brief   Foreground layer enable.
 * @details Enables the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgEnable(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgEnableI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Foreground layer disable.
 * @details Disables the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgDisableI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR &= ~LTDC_LxCR_LEN;
}

/**
 * @brief   Foreground layer disable.
 * @details Disables the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgDisable(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgDisableI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Foreground layer palette enabled.
 * @details Tells whether the foreground layer (layer 2) palette (color lookup
 *          table) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcFgIsPaletteEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer2->CR & ~LTDC_LxCR_CLUTEN) != 0;
}

/**
 * @brief   Foreground layer palette enabled.
 * @details Tells whether the foreground layer (layer 2) palette (color lookup
 *          table) is enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcFgIsPaletteEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcFgIsPaletteEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable foreground layer palette.
 * @details Enables the palette (color lookup table) of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgEnablePaletteI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR |= LTDC_LxCR_CLUTEN;
}

/**
 * @brief   Enable foreground layer palette.
 * @details Enables the palette (color lookup table) of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgEnablePalette(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgEnablePaletteI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable foreground layer palette.
 * @details Disables the palette (color lookup table) of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgDisablePaletteI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR &= ~LTDC_LxCR_CLUTEN;
}

/**
 * @brief   Disable foreground layer palette.
 * @details Disables the palette (color lookup table) of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgDisablePalette(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgDisablePaletteI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer palette color.
 * @details Sets the color of a palette (color lookup table) slot to the
 *          foreground layer (layer 2).
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] slot      palette slot
 * @param[in] c         color, RGB-888
 *
 * @iclass
 */
void ltdcFgSetPaletteColorI(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgAssert(!ltdcFgIsEnabledI(ltdcp), "invalid state");
  (void)ltdcp;

  LTDC_Layer2->CLUTWR = (((uint32_t)slot << 24) | (c & 0x00FFFFFF));
}

/**
 * @brief   Set foreground layer palette color.
 * @details Sets the color of a palette (color lookup table) slot to the
 *          foreground layer (layer 2).
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] slot      palette slot
 * @param[in] c         color, RGB-888
 *
 * @api
 */
void ltdcFgSetPaletteColor(LTDCDriver *ltdcp, uint8_t slot, ltdc_color_t c) {

  osalSysLock();
  ltdcFgSetPaletteColorI(ltdcp, slot, c);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer palette.
 * @details Sets the entire palette color (color lookup table) slot.
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] colors    array of palette colors, RGB-888
 * @param[in] length    number of palette colors
 *
 * @iclass
 */
void ltdcFgSetPaletteI(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                       uint16_t length) {

  uint16_t i;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck((colors == NULL) == (length == 0));
  osalDbgAssert(length <= LTDC_MAX_PALETTE_LENGTH, "bounds");
  osalDbgAssert(!ltdcFgIsEnabledI(ltdcp), "invalid state");
  (void)ltdcp;

  for (i = 0; i < length; ++i)
    LTDC_Layer2->CLUTWR = (((uint32_t)i << 24) | (colors[i] & 0x00FFFFFF));
}

/**
 * @brief   Set foreground layer palette.
 * @details Sets the entire palette color (color lookup table) slot.
 * @pre     The layer must be disabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] colors    array of palette colors, RGB-888
 * @param[in] length    number of palette colors
 *
 * @api
 */
void ltdcFgSetPalette(LTDCDriver *ltdcp, const ltdc_color_t colors[],
                      uint16_t length) {

  osalSysLock();
  ltdcFgSetPaletteI(ltdcp, colors, length);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer pixel format.
 * @details Gets the pixel format of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              pixel format
 *
 * @iclass
 */
ltdc_pixfmt_t ltdcFgGetPixelFormatI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_pixfmt_t)(LTDC_Layer2->PFCR & LTDC_LxPFCR_PF);
}

/**
 * @brief   Get foreground layer pixel format.
 * @details Gets the pixel format of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              pixel format
 *
 * @api
 */
ltdc_pixfmt_t ltdcFgGetPixelFormat(LTDCDriver *ltdcp) {

  ltdc_pixfmt_t fmt;
  osalSysLock();
  fmt = ltdcFgGetPixelFormatI(ltdcp);
  osalSysUnlock();
  return fmt;
}

/**
 * @brief   Set foreground layer pixel format.
 * @details Sets the pixel format of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] fmt       pixel format
 *
 * @iclass
 */
void ltdcFgSetPixelFormatI(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgAssert(fmt >= LTDC_MIN_PIXFMT_ID, "bounds");
  osalDbgAssert(fmt <= LTDC_MAX_PIXFMT_ID, "bounds");
  (void)ltdcp;

  LTDC_Layer2->PFCR = ((LTDC_Layer2->PFCR & ~LTDC_LxPFCR_PF) |
                       ((uint32_t)fmt & LTDC_LxPFCR_PF));
}

/**
 * @brief   Set foreground layer pixel format.
 * @details Sets the pixel format of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] fmt       pixel format
 *
 * @api
 */
void ltdcFgSetPixelFormat(LTDCDriver *ltdcp, ltdc_pixfmt_t fmt) {

  osalSysLock();
  ltdcFgSetPixelFormatI(ltdcp, fmt);
  osalSysUnlock();
}

/**
 * @brief   Foreground layer color keying enabled.
 * @details Tells whether the foreground layer (layer 2) has color keying
 *          enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool ltdcFgIsKeyingEnabledI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (LTDC_Layer2->CR & ~LTDC_LxCR_COLKEN) != 0;
}

/**
 * @brief   Foreground layer color keying enabled.
 * @details Tells whether the foreground layer (layer 2) has color keying
 *          enabled.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool ltdcFgIsKeyingEnabled(LTDCDriver *ltdcp) {

  bool enabled;
  osalSysLock();
  enabled = ltdcFgIsKeyingEnabledI(ltdcp);
  osalSysUnlock();
  return enabled;
}

/**
 * @brief   Enable foreground layer color keying.
 * @details Enables color keying capabilities of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgEnableKeyingI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR |= LTDC_LxCR_COLKEN;
}

/**
 * @brief   Enable foreground layer color keying.
 * @details Enables color keying capabilities of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgEnableKeying(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgEnableKeyingI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Disable foreground layer color keying.
 * @details Disables color keying capabilities of the foreground layer (layer
 *          2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgDisableKeyingI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CR &= ~LTDC_LxCR_COLKEN;
}

/**
 * @brief   Disable foreground layer color keying.
 * @details Disables color keying capabilities of the foreground layer (layer
 *          2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgDisableKeying(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgDisableKeyingI(ltdcp);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer color key.
 * @details Gets the color key of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              color key, RGB-888
 *
 * @iclass
 */
ltdc_color_t ltdcFgGetKeyingColorI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_color_t)(LTDC_Layer2->CKCR & 0x00FFFFFF);
}

/**
 * @brief   Get foreground layer color key.
 * @details Gets the color key of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              color key, RGB-888
 *
 * @api
 */
ltdc_color_t ltdcFgGetKeyingColor(LTDCDriver *ltdcp) {

  ltdc_color_t color;
  osalSysLock();
  color = ltdcFgGetKeyingColorI(ltdcp);
  osalSysUnlock();
  return color;
}

/**
 * @brief   Set foreground layer color key.
 * @details Sets the color key of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         color key, RGB-888
 *
 * @iclass
 */
void ltdcFgSetKeyingColorI(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CKCR = ((LTDC_Layer2->CKCR & ~0x00FFFFFF) |
                       ((uint32_t)c & 0x00FFFFFF));
}

/**
 * @brief   Set foreground layer color key.
 * @details Sets the color key of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         color key, RGB-888
 *
 * @api
 */
void ltdcFgSetKeyingColor(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalSysLock();
  ltdcFgSetKeyingColorI(ltdcp, c);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer constant alpha.
 * @details Gets the constant alpha component of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @iclass
 */
uint8_t ltdcFgGetConstantAlphaI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (uint8_t)(LTDC_Layer2->CACR & LTDC_LxCACR_CONSTA);
}

/**
 * @brief   Get foreground layer constant alpha.
 * @details Gets the constant alpha component of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @api
 */
uint8_t ltdcFgGetConstantAlpha(LTDCDriver *ltdcp) {

  uint8_t a;
  osalSysLock();
  a = ltdcFgGetConstantAlphaI(ltdcp);
  osalSysUnlock();
  return a;
}

/**
 * @brief   Set foreground layer constant alpha.
 * @details Sets the constant alpha component of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @iclass
 */
void ltdcFgSetConstantAlphaI(LTDCDriver *ltdcp, uint8_t a) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CACR = ((LTDC_Layer2->CACR & ~LTDC_LxCACR_CONSTA) |
                       ((uint32_t)a & LTDC_LxCACR_CONSTA));
}

/**
 * @brief   Set foreground layer constant alpha.
 * @details Sets the constant alpha component of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @api
 */
void ltdcFgSetConstantAlpha(LTDCDriver *ltdcp, uint8_t a) {

  osalSysLock();
  ltdcFgSetConstantAlphaI(ltdcp, a);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer default color.
 * @details Gets the default color of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              default color, RGB-888
 *
 * @iclass
 */
ltdc_color_t ltdcFgGetDefaultColorI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_color_t)LTDC_Layer2->DCCR;
}

/**
 * @brief   Get foreground layer default color.
 * @details Gets the default color of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              default color, RGB-888
 *
 * @api
 */
ltdc_color_t ltdcFgGetDefaultColor(LTDCDriver *ltdcp) {

  ltdc_color_t color;
  osalSysLock();
  color = ltdcFgGetDefaultColorI(ltdcp);
  osalSysUnlock();
  return color;
}

/**
 * @brief   Set foreground layer default color.
 * @details Sets the default color of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         default color, RGB-888
 *
 * @iclass
 */
void ltdcFgSetDefaultColorI(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->DCCR = (uint32_t)c;
}

/**
 * @brief   Set foreground layer default color.
 * @details Sets the default color of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] c         default color, RGB-888
 *
 * @api
 */
void ltdcFgSetDefaultColor(LTDCDriver *ltdcp, ltdc_color_t c) {

  osalSysLock();
  ltdcFgSetDefaultColorI(ltdcp, c);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer blending factors.
 * @details Gets the blending factors of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              blending factors
 *
 * @iclass
 */
ltdc_blendf_t ltdcFgGetBlendingFactorsI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (ltdc_blendf_t)(LTDC_Layer2->BFCR & LTDC_LxBFCR_BF);
}

/**
 * @brief   Get foreground layer blending factors.
 * @details Gets the blending factors of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              blending factors
 *
 * @api
 */
ltdc_blendf_t ltdcFgGetBlendingFactors(LTDCDriver *ltdcp) {

  ltdc_blendf_t bf;
  osalSysLock();
  bf = ltdcFgGetBlendingFactorsI(ltdcp);
  osalSysUnlock();
  return bf;
}

/**
 * @brief   Set foreground layer blending factors.
 * @details Sets the blending factors of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] factors   blending factors
 *
 * @iclass
 */
void ltdcFgSetBlendingFactorsI(LTDCDriver *ltdcp, ltdc_blendf_t bf) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->BFCR = ((LTDC_Layer2->BFCR & ~LTDC_LxBFCR_BF) |
                       ((uint32_t)bf & LTDC_LxBFCR_BF));
}

/**
 * @brief   Set foreground layer blending factors.
 * @details Sets the blending factors of the foreground layer (layer 1).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] factors   blending factors
 *
 * @api
 */
void ltdcFgSetBlendingFactors(LTDCDriver *ltdcp, ltdc_blendf_t bf) {

  osalSysLock();
  ltdcFgSetBlendingFactorsI(ltdcp, bf);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer window specs.
 * @details Gets the window specifications of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] windowp  pointer to the window specifications
 *
 * @iclass
 */
void ltdcFgGetWindowI(LTDCDriver *ltdcp, ltdc_window_t *windowp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(windowp != NULL);
  (void)ltdcp;

  windowp->hstart =
    (uint16_t)((LTDC_Layer2->WHPCR & LTDC_LxWHPCR_WHSTPOS) >>  0);
  windowp->hstop =
    (uint16_t)((LTDC_Layer2->WHPCR & LTDC_LxWHPCR_WHSPPOS) >> 16);
  windowp->vstart =
    (uint16_t)((LTDC_Layer2->WVPCR & LTDC_LxWVPCR_WVSTPOS) >>  0);
  windowp->vstop =
    (uint16_t)((LTDC_Layer2->WVPCR & LTDC_LxWVPCR_WVSPPOS) >> 16);
}

/**
 * @brief   Get foreground layer window specs.
 * @details Gets the window specifications of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] windowp  pointer to the window specifications
 *
 * @api
 */
void ltdcFgGetWindow(LTDCDriver *ltdcp, ltdc_window_t *windowp) {

  osalSysLock();
  ltdcFgGetWindowI(ltdcp, windowp);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer window specs.
 * @details Sets the window specifications of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] windowp   pointer to the window specifications
 *
 * @iclass
 */
void ltdcFgSetWindowI(LTDCDriver *ltdcp, const ltdc_window_t *windowp) {

  uint32_t start, stop;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(windowp != NULL);
  (void)ltdcp;

  osalDbgAssert(windowp->hstop < ltdcp->config->screen_width, "bounds");
  osalDbgAssert(windowp->vstop < ltdcp->config->screen_height, "bounds");

  /* Horizontal boundaries.*/
  start = (uint32_t)windowp->hstart + ltdcp->active_window.hstart;
  stop  = (uint32_t)windowp->hstop  + ltdcp->active_window.hstart;

  osalDbgAssert(start >= ltdcp->active_window.hstart, "bounds");
  osalDbgAssert(stop <= ltdcp->active_window.hstop, "bounds");

  LTDC_Layer2->WHPCR = (((start <<  0) & LTDC_LxWHPCR_WHSTPOS) |
                        ((stop  << 16) & LTDC_LxWHPCR_WHSPPOS));

  /* Vertical boundaries.*/
  start = (uint32_t)windowp->vstart + ltdcp->active_window.vstart;
  stop  = (uint32_t)windowp->vstop  + ltdcp->active_window.vstart;

  osalDbgAssert(start >= ltdcp->active_window.vstart, "bounds");
  osalDbgAssert(stop <= ltdcp->active_window.vstop, "bounds");

  LTDC_Layer2->WVPCR = (((start <<  0) & LTDC_LxWVPCR_WVSTPOS) |
                        ((stop  << 16) & LTDC_LxWVPCR_WVSPPOS));
}

/**
 * @brief   Set foreground layer window specs.
 * @details Sets the window specifications of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] windowp   pointer to the window specifications
 *
 * @api
 */
void ltdcFgSetWindow(LTDCDriver *ltdcp, const ltdc_window_t *windowp) {

  osalSysLock();
  ltdcFgSetWindowI(ltdcp, windowp);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer window as invalid.
 * @details Sets the window specifications of the foreground layer (layer 2)
 *          so that the window is pixel sized at the screen origin.
 * @note    Useful before reconfiguring the frame specifications of the layer,
 *          to avoid errors.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @iclass
 */
void ltdcFgSetInvalidWindowI(LTDCDriver *ltdcp) {

  ltdcFgSetWindowI(ltdcp, &ltdc_invalid_window);
}

/**
 * @brief   Set foreground layer window as invalid.
 * @details Sets the window specifications of the foreground layer (layer 2)
 *          so that the window is pixel sized at the screen origin.
 * @note    Useful before reconfiguring the frame specifications of the layer,
 *          to avoid errors.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @api
 */
void ltdcFgSetInvalidWindow(LTDCDriver *ltdcp) {

  osalSysLock();
  ltdcFgSetWindowI(ltdcp, &ltdc_invalid_window);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer frame buffer specs.
 * @details Gets the frame buffer specifications of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] framep   pointer to the frame buffer specifications
 *
 * @iclass
 */
void ltdcFgGetFrameI(LTDCDriver *ltdcp, ltdc_frame_t *framep) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(framep != NULL);

  framep->bufferp = (void *)(LTDC_Layer2->CFBAR & LTDC_LxCFBAR_CFBADD);
  framep->pitch   = (size_t)((LTDC_Layer2->CFBLR & LTDC_LxCFBLR_CFBP) >> 16);
  framep->width   = (uint16_t)(((LTDC_Layer2->CFBLR & LTDC_LxCFBLR_CFBLL) - 3) /
                    ltdcBytesPerPixel(ltdcFgGetPixelFormatI(ltdcp)));
  framep->height  = (uint16_t)(LTDC_Layer2->CFBLNR & LTDC_LxCFBLNR_CFBLNBR);
}

/**
 * @brief   Get foreground layer frame buffer specs.
 * @details Gets the frame buffer specifications of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] framep   pointer to the frame buffer specifications
 *
 * @api
 */
void ltdcFgGetFrame(LTDCDriver *ltdcp, ltdc_frame_t *framep) {

  osalSysLock();
  ltdcFgGetFrameI(ltdcp, framep);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer frame buffer specs.
 * @details Sets the frame buffer specifications of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] framep    pointer to the frame buffer specifications
 *
 * @iclass
 */
void ltdcFgSetFrameI(LTDCDriver *ltdcp, const ltdc_frame_t *framep) {

  size_t linesize;

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(framep != NULL);

  ltdcFgSetPixelFormatI(ltdcp, framep->fmt);

  linesize = ltdcBytesPerPixel(framep->fmt) * framep->width;

  osalDbgAssert(framep->width  <= ltdcp->config->screen_width, "bounds");
  osalDbgAssert(framep->height <= ltdcp->config->screen_height, "bounds");
  osalDbgAssert(linesize >= LTDC_MIN_FRAME_WIDTH_BYTES, "bounds");
  osalDbgAssert(linesize <= LTDC_MAX_FRAME_WIDTH_BYTES, "bounds");
  osalDbgAssert(framep->height >= LTDC_MIN_FRAME_HEIGHT_LINES, "bounds");
  osalDbgAssert(framep->height <= LTDC_MAX_FRAME_HEIGHT_LINES, "bounds");
  osalDbgAssert(framep->pitch  >= linesize, "bounds");

  LTDC_Layer2->CFBAR = (uint32_t)framep->bufferp & LTDC_LxCFBAR_CFBADD;
  LTDC_Layer2->CFBLR = ((((uint32_t)framep->pitch << 16) & LTDC_LxCFBLR_CFBP) |
                        ((linesize + 3) & LTDC_LxCFBLR_CFBLL));
  LTDC_Layer2->CFBLNR = (uint32_t)framep->height & LTDC_LxCFBLNR_CFBLNBR;
}

/**
 * @brief   Set foreground layer frame buffer specs.
 * @details Sets the frame buffer specifications of the foreground layer
 *          (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] framep    pointer to the frame buffer specifications
 *
 * @api
 */
void ltdcFgSetFrame(LTDCDriver *ltdcp, const ltdc_frame_t *framep) {

  osalSysLock();
  ltdcFgSetFrameI(ltdcp, framep);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer frame buffer address.
 * @details Gets the frame buffer address of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              frame buffer address
 *
 * @iclass
 */
void *ltdcFgGetFrameAddressI(LTDCDriver *ltdcp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  return (void *)LTDC_Layer2->CFBAR;
}

/**
 * @brief   Get foreground layer frame buffer address.
 * @details Gets the frame buffer address of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 *
 * @return              frame buffer address
 *
 * @api
 */
void *ltdcFgGetFrameAddress(LTDCDriver *ltdcp) {

  void *bufferp;
  osalSysLock();
  bufferp = ltdcFgGetFrameAddressI(ltdcp);
  osalSysUnlock();
  return bufferp;
}

/**
 * @brief   Set foreground layer frame buffer address.
 * @details Sets the frame buffer address of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] bufferp   frame buffer address
 *
 * @iclass
 */
void ltdcFgSetFrameAddressI(LTDCDriver *ltdcp, void *bufferp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  (void)ltdcp;

  LTDC_Layer2->CFBAR = (uint32_t)bufferp;
}

/**
 * @brief   Set foreground layer frame buffer address.
 * @details Sets the frame buffer address of the foreground layer (layer 2).
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] bufferp   frame buffer address
 *
 * @api
 */
void ltdcFgSetFrameAddress(LTDCDriver *ltdcp, void *bufferp) {

  osalSysLock();
  ltdcFgSetFrameAddressI(ltdcp, bufferp);
  osalSysUnlock();
}

/**
 * @brief   Get foreground layer specifications.
 * @details Gets the foreground layer (layer 2) specifications at once.
 * @note    If palette specifications cannot be retrieved, they are set to
 *          @p NULL. This is not an error.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @iclass
 */
void ltdcFgGetLayerI(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);
  osalDbgCheck(cfgp != NULL);

  ltdcFgGetFrameI(ltdcp, (ltdc_frame_t *)cfgp->frame);
  ltdcFgGetWindowI(ltdcp, (ltdc_window_t *)cfgp->window);
  cfgp->def_color = ltdcFgGetDefaultColorI(ltdcp);
  cfgp->key_color = ltdcFgGetKeyingColorI(ltdcp);
  cfgp->const_alpha = ltdcFgGetConstantAlphaI(ltdcp);
  cfgp->blending = ltdcFgGetBlendingFactorsI(ltdcp);

  cfgp->pal_colors = NULL;
  cfgp->pal_length = 0;

  cfgp->flags = ltdcFgGetEnableFlagsI(ltdcp);
}

/**
 * @brief   Get foreground layer specifications.
 * @details Gets the foreground layer (layer 2) specifications at once.
 * @note    If palette specifications cannot be retrieved, they are set to
 *          @p NULL. This is not an error.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @api
 */
void ltdcFgGetLayer(LTDCDriver *ltdcp, ltdc_laycfg_t *cfgp) {

  osalSysLock();
  ltdcFgGetLayerI(ltdcp, cfgp);
  osalSysUnlock();
}

/**
 * @brief   Set foreground layer specifications.
 * @details Sets the foreground layer (layer 2) specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @iclass
 */
void ltdcFgSetConfigI(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(ltdcp == &LTDCD1);

  if (cfgp == NULL)
    cfgp = &ltdc_default_laycfg;

  osalDbgCheck((cfgp->pal_colors == NULL) == (cfgp->pal_length == 0));

  ltdcFgSetFrameI(ltdcp, cfgp->frame);
  ltdcFgSetWindowI(ltdcp, cfgp->window);
  ltdcFgSetDefaultColorI(ltdcp, cfgp->def_color);
  ltdcFgSetKeyingColorI(ltdcp, cfgp->key_color);
  ltdcFgSetConstantAlphaI(ltdcp, cfgp->const_alpha);
  ltdcFgSetBlendingFactorsI(ltdcp, cfgp->blending);

  if (cfgp->pal_length > 0)
    ltdcFgSetPaletteI(ltdcp, cfgp->pal_colors, cfgp->pal_length);

  ltdcFgSetEnableFlagsI(ltdcp, cfgp->flags);
}

/**
 * @brief   Set foreground layer specifications.
 * @details Sets the foreground layer (layer 2) specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 *
 * @param[in] ltdcp     pointer to the @p LTDCDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @api
 */
void ltdcFgSetConfig(LTDCDriver *ltdcp, const ltdc_laycfg_t *cfgp) {

  osalSysLock();
  ltdcFgSetConfigI(ltdcp, cfgp);
  osalSysUnlock();
}

/** @} */

/**
 * @name    LTDC helper functions
 */

/**
 * @brief   Compute bits per pixel.
 * @details Computes the bits per pixel for the specified pixel format.
 *
 * @param[in] fmt       pixel format
 *
 * @retuen              bits per pixel
 *
 * @api
 */
size_t ltdcBitsPerPixel(ltdc_pixfmt_t fmt) {

  osalDbgAssert(fmt < LTDC_MAX_PIXFMT_ID, "invalid format");

  return (size_t)ltdc_bpp[(unsigned)fmt];
}

#if (TRUE == LTDC_USE_SOFTWARE_CONVERSIONS) || defined(__DOXYGEN__)

/**
 * @brief   Convert from ARGB-8888.
 * @details Converts an ARGB-8888 color to the specified pixel format.
 *
 * @param[in] c         color, ARGB-8888
 * @param[in] fmt       target pixel format
 *
 * @return              raw color value for the target pixel format, left
 *                      padded with zeros.
 *
 * @api
 */
ltdc_color_t ltdcFromARGB8888(ltdc_color_t c, ltdc_pixfmt_t fmt) {

  switch (fmt) {
  case LTDC_FMT_ARGB8888: {
    return c;
  }
  case LTDC_FMT_RGB888: {
    return (c & 0x00FFFFFF);
  }
  case LTDC_FMT_RGB565: {
    return (((c & 0x000000F8) >> ( 8 -  5)) |
            ((c & 0x0000FC00) >> (16 - 11)) |
            ((c & 0x00F80000) >> (24 - 16)));
  }
  case LTDC_FMT_ARGB1555: {
    return (((c & 0x000000F8) >> ( 8 -  5)) |
            ((c & 0x0000F800) >> (16 - 10)) |
            ((c & 0x00F80000) >> (24 - 15)) |
            ((c & 0x80000000) >> (32 - 16)));
  }
  case LTDC_FMT_ARGB4444: {
    return (((c & 0x000000F0) >> ( 8 -  4)) |
            ((c & 0x0000F000) >> (16 -  8)) |
            ((c & 0x00F00000) >> (24 - 12)) |
            ((c & 0xF0000000) >> (32 - 16)));
  }
  case LTDC_FMT_L8: {
    return (c & 0x000000FF);
  }
  case LTDC_FMT_AL44: {
    return (((c & 0x000000F0) >> ( 8 - 4)) |
            ((c & 0xF0000000) >> (32 - 8)));
  }
  case LTDC_FMT_AL88: {
    return (((c & 0x000000FF) >> ( 8 -  8)) |
            ((c & 0xFF000000) >> (32 - 16)));
  }
  default:
    osalDbgAssert(false, "invalid format");
    return 0;
  }
}

/**
 * @brief   Convert to ARGB-8888.
 * @details Converts color of the specified pixel format to an ARGB-8888 color.
 *
 * @param[in] c         color for the source pixel format, left padded with
 *                      zeros.
 * @param[in] fmt       source pixel format
 *
 * @return              color in ARGB-8888 format
 *
 * @api
 */
ltdc_color_t ltdcToARGB8888(ltdc_color_t c, ltdc_pixfmt_t fmt) {

  switch (fmt) {
  case LTDC_FMT_ARGB8888: {
    return c;
  }
  case LTDC_FMT_RGB888: {
    return ((c & 0x00FFFFFF) | 0xFF000000);
  }
  case LTDC_FMT_RGB565: {
    register ltdc_color_t output = 0xFF000000;
    if (c & 0x001F) output |= (((c & 0x001F) << ( 8 -  5)) | 0x00000007);
    if (c & 0x07E0) output |= (((c & 0x07E0) << (16 - 11)) | 0x00000300);
    if (c & 0xF800) output |= (((c & 0xF800) << (24 - 16)) | 0x00070000);
    return output;
  }
  case LTDC_FMT_ARGB1555: {
    register ltdc_color_t output = 0x00000000;
    if (c & 0x001F) output |= (((c & 0x001F) << ( 8 -  5)) | 0x00000007);
    if (c & 0x03E0) output |= (((c & 0x03E0) << (16 - 10)) | 0x00000700);
    if (c & 0x7C00) output |= (((c & 0x7C00) << (24 - 15)) | 0x00070000);
    if (c & 0x8000) output |= 0xFF000000;
    return output;
  }
  case LTDC_FMT_ARGB4444: {
    register ltdc_color_t output = 0x00000000;
    if (c & 0x000F) output |= (((c & 0x000F) << ( 8 -  4)) | 0x0000000F);
    if (c & 0x00F0) output |= (((c & 0x00F0) << (16 -  8)) | 0x00000F00);
    if (c & 0x0F00) output |= (((c & 0x0F00) << (24 - 12)) | 0x000F0000);
    if (c & 0xF000) output |= (((c & 0xF000) << (32 - 16)) | 0x0F000000);
    return output;
  }
  case LTDC_FMT_L8: {
    return ((c & 0xFF) | 0xFF000000);
  }
  case LTDC_FMT_AL44: {
    register ltdc_color_t output = 0x00000000;
    if (c & 0x0F) output |= (((c & 0x0F) << ( 8 - 4)) | 0x0000000F);
    if (c & 0xF0) output |= (((c & 0xF0) << (32 - 8)) | 0x0F000000);
    return output;
  }
  case LTDC_FMT_AL88: {
    return (((c & 0x00FF) << ( 8 -  8)) |
            ((c & 0xFF00) << (32 - 16)));
  }
  default:
    osalDbgAssert(false, "invalid format");
    return 0;
  }
}

#endif  /* LTDC_USE_SOFTWARE_CONVERSIONS */

/** @} */

/** @} */

#endif  /* STM32_LTDC_USE_LTDC */
