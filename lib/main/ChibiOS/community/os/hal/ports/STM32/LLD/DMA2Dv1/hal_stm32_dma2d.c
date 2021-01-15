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
 * @file    hal_stm32_dma2d.c
 * @brief   DMA2D/Chrom-ART driver.
 */

#include "hal.h"

#include "hal_stm32_dma2d.h"

#if STM32_DMA2D_USE_DMA2D || defined(__DOXYGEN__)

/* Ignore annoying warning messages for actually safe code.*/
#if defined(__GNUC__) && !defined(__DOXYGEN__)
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * @addtogroup dma2d
 * @{
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief DMA2DD1 driver identifier.*/
DMA2DDriver DMA2DD1;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Bits per pixel lookup table.
 */
static const uint8_t dma2d_bpp[DMA2D_MAX_PIXFMT_ID + 1] = {
  32,  /* DMA2D_FMT_ARGB8888 */
  24,  /* DMA2D_FMT_RGB888 */
  16,  /* DMA2D_FMT_RGB565 */
  16,  /* DMA2D_FMT_ARGB1555 */
  16,  /* DMA2D_FMT_ARGB4444 */
   8,  /* DMA2D_FMT_L8 */
   8,  /* DMA2D_FMT_AL44 */
  16,  /* DMA2D_FMT_AL88 */
   4,  /* DMA2D_FMT_L4 */
   8,  /* DMA2D_FMT_A8 */
   4   /* DMA2D_FMT_A4 */
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @name    DMA2D interrupt handlers
 * @{
 */

/**
 * @brief   DMA2D global interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(STM32_DMA2D_HANDLER) {

  DMA2DDriver *const dma2dp = &DMA2DD1;
  bool job_done = false;
  thread_t *tp = NULL;

  OSAL_IRQ_PROLOGUE();

  /* Handle Configuration Error ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_CEIF) && (DMA2D->CR & DMA2D_CR_CEIE)) {
    if (dma2dp->config->cfgerr_isr != NULL)
      dma2dp->config->cfgerr_isr(dma2dp);
    job_done = true;
    DMA2D->IFCR |= DMA2D_IFSR_CCEIF;
  }

  /* Handle CLUT (Palette) Transfer Complete ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_CTCIF) && (DMA2D->CR & DMA2D_CR_CTCIE)) {
    if (dma2dp->config->paltrfdone_isr != NULL)
      dma2dp->config->paltrfdone_isr(dma2dp);
    job_done = true;
    DMA2D->IFCR |= DMA2D_IFSR_CCTCIF;
  }

  /* Handle CLUT (Palette) Access Error ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_CAEIF) && (DMA2D->CR & DMA2D_CR_CAEIE)) {
    if (dma2dp->config->palacserr_isr != NULL)
      dma2dp->config->palacserr_isr(dma2dp);
    job_done = true;
    DMA2D->IFCR |= DMA2D_IFSR_CCAEIF;
  }

  /* Handle Transfer Watermark ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_TWIF) && (DMA2D->CR & DMA2D_CR_TWIE)) {
    if (dma2dp->config->trfwmark_isr != NULL)
      dma2dp->config->trfwmark_isr(dma2dp);
    DMA2D->IFCR |= DMA2D_IFSR_CTWIF;
  }

  /* Handle Transfer Complete ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_TCIF) && (DMA2D->CR & DMA2D_CR_TCIE)) {
    if (dma2dp->config->trfdone_isr != NULL)
      dma2dp->config->trfdone_isr(dma2dp);
    job_done = true;
    DMA2D->IFCR |= DMA2D_IFSR_CTCIF;
  }

  /* Handle Transfer Error ISR.*/
  if ((DMA2D->ISR & DMA2D_ISR_TEIF) && (DMA2D->CR & DMA2D_CR_TEIE)) {
    if (dma2dp->config->trferr_isr != NULL)
      dma2dp->config->trferr_isr(dma2dp);
    job_done = true;
    DMA2D->IFCR |= DMA2D_IFSR_CTEIF;
  }

  if (job_done) {
    osalSysLockFromISR();
    osalDbgAssert(dma2dp->state == DMA2D_ACTIVE, "invalid state");

  #if DMA2D_USE_WAIT
    /* Wake the waiting thread up.*/
    if (dma2dp->thread != NULL) {
      tp = dma2dp->thread;
      dma2dp->thread = NULL;
      tp->u.rdymsg = MSG_OK;
      chSchReadyI(tp);
    }
  #endif  /* DMA2D_USE_WAIT */

    dma2dp->state = DMA2D_READY;
    osalSysUnlockFromISR();
  }

  OSAL_IRQ_EPILOGUE();
}

/** @} */

/**
 * @name    DMA2D driver-specific methods
 * @{
 */

/**
 * @brief   DMA2D Driver initialization.
 * @details Initializes the DMA2D subsystem and chosen drivers. Should be
 *          called at board initialization.
 *
 * @init
 */
void dma2dInit(void) {

  /* Reset the DMA2D hardware module.*/
  rccResetDMA2D();

  /* Enable the DMA2D clock.*/
  rccEnableDMA2D(false);

  /* Driver struct initialization.*/
  dma2dObjectInit(&DMA2DD1);
  DMA2DD1.state = DMA2D_STOP;
}

/**
 * @brief   Initializes the standard part of a @p DMA2DDriver structure.
 *
 * @param[out] dma2dp   pointer to the @p DMA2DDriver object
 *
 * @init
 */
void dma2dObjectInit(DMA2DDriver *dma2dp) {

  osalDbgCheck(dma2dp == &DMA2DD1);

  dma2dp->state = DMA2D_UNINIT;
  dma2dp->config = NULL;
#if DMA2D_USE_WAIT
  dma2dp->thread = NULL;
#endif  /* DMA2D_USE_WAIT */
#if (TRUE == DMA2D_USE_MUTUAL_EXCLUSION)
#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxObjectInit(&dma2dp->lock);
#else
  chSemObjectInit(&dma2dp->lock, 1);
#endif
#endif  /* (TRUE == DMA2D_USE_MUTUAL_EXCLUSION) */
}

/**
 * @brief   Get the driver state.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @retun               driver state
 *
 * @iclass
 */
dma2d_state_t dma2dGetStateI(DMA2DDriver *dma2dp) {

  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheckClassI();

  return dma2dp->state;
}

/**
 * @brief   Get the driver state.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @retun               driver state
 *
 * @api
 */
dma2d_state_t dma2dGetState(DMA2DDriver *dma2dp) {

  dma2d_state_t state;
  chSysLock();
  state = dma2dGetStateI(dma2dp);
  chSysUnlock();
  return state;
}

/**
 * @brief   Configures and activates the DMA2D peripheral.
 * @pre     DMA2D is stopped.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] configp   pointer to the @p DMA2DConfig object
 *
 * @api
 */
void dma2dStart(DMA2DDriver *dma2dp, const DMA2DConfig *configp) {

  chSysLock();

  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(configp != NULL);
  osalDbgAssert(dma2dp->state == DMA2D_STOP, "invalid state");

  dma2dp->config = configp;

  /* Turn off the controller and its interrupts.*/
  DMA2D->CR = 0;

  /* Enable interrupts, except Line Watermark.*/
  nvicEnableVector(STM32_DMA2D_NUMBER, STM32_DMA2D_IRQ_PRIORITY);

  DMA2D->CR = (DMA2D_CR_CEIE | DMA2D_CR_CTCIE | DMA2D_CR_CAEIE |
               DMA2D_CR_TCIE | DMA2D_CR_TEIE);

  dma2dp->state = DMA2D_READY;
  chSysUnlock();
}

/**
 * @brief   Deactivates the DMA2D peripheral.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dStop(DMA2DDriver *dma2dp) {

  chSysLock();

  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "invalid state");
#if DMA2D_USE_WAIT
  osalDbgAssert(dma2dp->thread == NULL, "still waiting");
#endif  /* DMA2D_USE_WAIT */

  dma2dp->state = DMA2D_STOP;
  chSysUnlock();
}

#if DMA2D_USE_MUTUAL_EXCLUSION

/**
 * @brief   Gains exclusive access to the DMA2D module.
 * @details This function tries to gain ownership to the DMA2D module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p DMA2D_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @sclass
 */
void dma2dAcquireBusS(DMA2DDriver *dma2dp) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxLockS(&dma2dp->lock);
#else
  chSemWaitS(&dma2dp->lock);
#endif
}

/**
 * @brief   Gains exclusive access to the DMA2D module.
 * @details This function tries to gain ownership to the DMA2D module, if the
 *          module is already being used then the invoking thread is queued.
 * @pre     In order to use this function the option
 *          @p DMA2D_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dAcquireBus(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dAcquireBusS(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Releases exclusive access to the DMA2D module.
 * @pre     In order to use this function the option
 *          @p DMA2D_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @sclass
 */
void dma2dReleaseBusS(DMA2DDriver *dma2dp) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");

#if (TRUE == CH_CFG_USE_MUTEXES)
  chMtxUnlockS(&dma2dp->lock);
#else
  chSemSignalI(&dma2dp->lock);
#endif
}

/**
 * @brief   Releases exclusive access to the DMA2D module.
 * @pre     In order to use this function the option
 *          @p DMA2D_USE_MUTUAL_EXCLUSION must be enabled.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dReleaseBus(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dReleaseBusS(dma2dp);
  chSysUnlock();
}

#endif  /* DMA2D_USE_MUTUAL_EXCLUSION */

/** @} */

/**
 * @name    DMA2D global methods
 * @{
 */

/**
 * @brief   Get watermark position.
 * @details Gets the watermark line position.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              watermark line position
 *
 * @iclass
 */
uint16_t dma2dGetWatermarkPosI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (uint16_t)(DMA2D->LWR & DMA2D_LWR_LW);
}

/**
 * @brief   Get watermark position.
 * @details Gets the watermark line position.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              watermark line position
 *
 * @api
 */
uint16_t dma2dGetWatermarkPos(DMA2DDriver *dma2dp) {

  uint16_t line;
  chSysLock();
  line = dma2dGetWatermarkPosI(dma2dp);
  chSysUnlock();
  return line;
}

/**
 * @brief   Set watermark position.
 * @details Sets the watermark line position.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] line      watermark line position
 *
 * @iclass
 */
void dma2dSetWatermarkPosI(DMA2DDriver *dma2dp, uint16_t line) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->LWR = ((DMA2D->LWR & ~DMA2D_LWR_LW) |
                ((uint32_t)line & DMA2D_LWR_LW));
}

/**
 * @brief   Set watermark position.
 * @details Sets the watermark line position.
 * @note    The interrupt is invoked after the last pixel of the watermark line
 *          is written.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] line      watermark line position
 *
 * @iclass
 */
void dma2dSetWatermarkPos(DMA2DDriver *dma2dp, uint16_t line) {

  chSysLock();
  dma2dSetWatermarkPosI(dma2dp, line);
  chSysUnlock();
}

/**
 * @brief   Watermark interrupt enabled.
 * @details Tells whether the watermark interrupt is enabled.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool dma2dIsWatermarkEnabledI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (DMA2D->CR & DMA2D_CR_TWIE) != 0;
}

/**
 * @brief   Watermark interrupt enabled.
 * @details Tells whether the watermark interrupt is enabled.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool dma2dIsWatermarkEnabled(DMA2DDriver *dma2dp) {

  bool enabled;
  chSysLock();
  enabled = dma2dIsWatermarkEnabledI(dma2dp);
  chSysUnlock();
  return enabled;
}

/**
 * @brief   Enable watermark interrupt.
 * @details Enables the watermark interrupt. The interrupt is invoked after the
 *          last pixel of the watermark line is written to the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dEnableWatermarkI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  DMA2D->CR |= DMA2D_CR_TWIE;
}

/**
 * @brief   Enable watermark interrupt.
 * @details Enables the watermark interrupt. The interrupt is invoked after the
 *          last pixel of the watermark line is written to the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dEnableWatermark(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dEnableWatermarkI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Disable watermark interrupt.
 * @details Disables the watermark interrupt.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dDisableWatermarkI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  DMA2D->CR &= ~DMA2D_CR_TWIE;
}

/**
 * @brief   Disable watermark interrupt.
 * @details Disables the watermark interrupt.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dDisableWatermark(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dDisableWatermarkI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Get dead time cycles.
 * @details Gets the minimum dead time DMA2D clock cycles between DMA2D
 *          transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              dead time, in DMA2D clock cycles
 *
 * @iclass
 */
uint32_t dma2dGetDeadTimeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (DMA2D->AMTCR & DMA2D_AMTCR_DT) >> 8;
}

/**
 * @brief   Get dead time cycles.
 * @details Gets the minimum dead time DMA2D clock cycles between DMA2D
 *          transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              dead time, in DMA2D clock cycles
 *
 * @api
 */
uint32_t dma2dGetDeadTime(DMA2DDriver *dma2dp) {

  uint32_t cycles;
  chSysLock();
  cycles = dma2dGetDeadTimeI(dma2dp);
  chSysUnlock();
  return cycles;
}

/**
 * @brief   Set dead time cycles.
 * @details Sets the minimum dead time DMA2D clock cycles between DMA2D
 *          transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cycles    dead time, in DMA2D clock cycles
 *
 * @iclass
 */
void dma2dSetDeadTimeI(DMA2DDriver *dma2dp, uint32_t cycles) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(cycles <= DMA2D_MAX_DEADTIME_CYCLES, "bounds");
  (void)dma2dp;

  DMA2D->AMTCR = ((DMA2D->AMTCR & ~DMA2D_AMTCR_DT) |
                  ((cycles << 8) & DMA2D_AMTCR_DT));
}

/**
 * @brief   Set dead time cycles.
 * @details Sets the minimum dead time DMA2D clock cycles between DMA2D
 *          transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cycles    dead time, in DMA2D clock cycles
 *
 * @api
 */
void dma2dSetDeadTime(DMA2DDriver *dma2dp, uint32_t cycles) {

  chSysLock();
  dma2dSetDeadTimeI(dma2dp, cycles);
  chSysUnlock();
}

/**
 * @brief   Dead time enabled.
 * @details Tells whether the dead time between DMA2D transactions is enabled.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              enabled
 *
 * @iclass
 */
bool dma2dIsDeadTimeEnabledI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (DMA2D->AMTCR & DMA2D_AMTCR_EN) != 0;
}

/**
 * @brief   Dead time enabled.
 * @details Tells whether the dead time between DMA2D transactions is enabled.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              enabled
 *
 * @api
 */
bool dma2dIsDeadTimeEnabled(DMA2DDriver *dma2dp) {

  bool enabled;
  chSysLock();
  enabled = dma2dIsDeadTimeEnabledI(dma2dp);
  chSysUnlock();
  return enabled;
}

/**
 * @brief   Enable dead time.
 * @details Enables the dead time between DMA2D transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dEnableDeadTimeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  DMA2D->AMTCR |= DMA2D_AMTCR_EN;
}

/**
 * @brief   Enable dead time.
 * @details Enables the dead time between DMA2D transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dEnableDeadTime(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dEnableDeadTimeI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Disable dead time.
 * @details Disables the dead time between DMA2D transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dDisableDeadTimeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  DMA2D->AMTCR &= ~DMA2D_AMTCR_EN;
}

/**
 * @brief   Disable dead time.
 * @details Disables the dead time between DMA2D transactions.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dDisableDeadTime(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dDisableDeadTimeI(dma2dp);
  chSysUnlock();
}

/** @} */

/**
 * @name    DMA2D job (transaction) methods
 * @{
 */

/**
 * @brief   Get job mode.
 * @details Gets the job mode.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              job mode
 *
 * @iclass
 */
dma2d_jobmode_t dma2dJobGetModeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_jobmode_t)(DMA2D->CR & DMA2D_CR_MODE);
}

/**
 * @brief   Get job mode.
 * @details Gets the job mode.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              job mode
 *
 * @api
 */
dma2d_jobmode_t dma2dJobGetMode(DMA2DDriver *dma2dp) {

  dma2d_jobmode_t mode;
  chSysLock();
  mode = dma2dJobGetModeI(dma2dp);
  chSysUnlock();
  return mode;
}

/**
 * @brief   Set job mode.
 * @details Sets the job mode.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      job mode
 *
 * @iclass
 */
void dma2dJobSetModeI(DMA2DDriver *dma2dp, dma2d_jobmode_t mode) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert((mode & ~DMA2D_CR_MODE) == 0, "bounds");
  (void)dma2dp;

  DMA2D->CR = ((DMA2D->CR & ~DMA2D_CR_MODE) |
               ((uint32_t)mode & DMA2D_CR_MODE));
}

/**
 * @brief   Set job mode.
 * @details Sets the job mode.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      job mode
 *
 * @api
 */
void dma2dJobSetMode(DMA2DDriver *dma2dp, dma2d_jobmode_t mode) {

  chSysLock();
  dma2dJobSetModeI(dma2dp, mode);
  chSysUnlock();
}

/**
 * @brief   Get job size.
 * @details Gets the job size.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] widthp   pointer to the job width, in pixels
 * @param[out] heightp  pointer to the job height, in pixels
 *
 * @iclass
 */
void dma2dJobGetSizeI(DMA2DDriver *dma2dp,
                      uint16_t *widthp, uint16_t *heightp) {

  uint32_t r;

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(widthp != NULL);
  osalDbgCheck(heightp != NULL);
  (void)dma2dp;

  r = DMA2D->NLR;
  *widthp  = (uint16_t)((r & DMA2D_NLR_PL) >> 16);
  *heightp = (uint16_t)((r & DMA2D_NLR_NL) >>  0);
}

/**
 * @brief   Get job size.
 * @details Gets the job size.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] widthp   pointer to the job width, in pixels
 * @param[out] heightp  pointer to the job height, in pixels
 *
 * @api
 */
void dma2dJobGetSize(DMA2DDriver *dma2dp,
                     uint16_t *widthp, uint16_t *heightp) {

  chSysLock();
  dma2dJobGetSizeI(dma2dp, widthp, heightp);
  chSysUnlock();
}

/**
 * @brief   Set job size.
 * @details Sets the job size.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] widthp    job width, in pixels
 * @param[in] heightp   job height, in pixels
 *
 * @iclass
 */
void dma2dJobSetSizeI(DMA2DDriver *dma2dp, uint16_t width, uint16_t height) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(width <= DMA2D_MAX_WIDTH, "bounds");
  osalDbgAssert(height <= DMA2D_MAX_HEIGHT, "bounds");
  (void)dma2dp;

  DMA2D->NLR = ((((uint32_t)width  << 16) & DMA2D_NLR_PL) |
                (((uint32_t)height <<  0) & DMA2D_NLR_NL));
}

/**
 * @brief   Set job size.
 * @details Sets the job size.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] widthp    job width, in pixels
 * @param[in] heightp   job height, in pixels
 *
 * @api
 */
void dma2dJobSetSize(DMA2DDriver *dma2dp, uint16_t width, uint16_t height) {

  chSysLock();
  dma2dJobSetSizeI(dma2dp, width, height);
  chSysUnlock();
}

/**
 * @brief   Job executing.
 * @details Tells whether a job (transaction) is active or paused.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              executing
 *
 * @iclass
 */
bool dma2dJobIsExecutingI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);

  return dma2dp->state > DMA2D_READY;
}

/**
 * @brief   Job executing.
 * @details Tells whether a job (transaction) is active or paused.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              executing
 *
 * @api
 */
bool dma2dJobIsExecuting(DMA2DDriver *dma2dp) {

  bool executing;
  chSysLock();
  executing = dma2dJobIsExecutingI(dma2dp);
  chSysUnlock();
  return executing;
}

/**
 * @brief   Start job.
 * @details The job is started, and the DMA2D is set to active.
 * @note    Should there be invalid parameters, the appropriate interrupt
 *          handler will be invoked, and the DMA2D set back to ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dJobStartI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");

  dma2dp->state = DMA2D_ACTIVE;
  DMA2D->CR |= DMA2D_CR_START;
}

/**
 * @brief   Start job.
 * @details The job is started, and the DMA2D is set to active.
 * @note    Should there be invalid parameters, the appropriate interrupt
 *          handler will be invoked, and the DMA2D set back to ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dJobStart(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dJobStartI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Execute job.
 * @details Starts the job and waits for its completion, synchronously.
 * @note    Should there be invalid parameters, the appropriate interrupt
 *          handler will be invoked, and the DMA2D set back to ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @sclass
 */
void dma2dJobExecuteS(DMA2DDriver *dma2dp) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);

  dma2dJobStartI(dma2dp);
#if DMA2D_USE_WAIT
  dma2dp->thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
#else
  while (DMA2D->CR & DMA2D_CR_START)
    chSchDoYieldS();
#endif
}

/**
 * @brief   Execute job.
 * @details Starts the job and waits for its completion, synchronously.
 * @note    Should there be invalid parameters, the appropriate interrupt
 *          handler will be invoked, and the DMA2D set back to ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dJobExecute(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dJobExecuteS(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Suspend current job.
 * @details Suspends the current job. The driver is set to a paused state.
 * @pre     There is an active job.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dJobSuspendI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck((DMA2D->CR & DMA2D_CR_SUSP) == 0);
  osalDbgAssert(dma2dp->state == DMA2D_ACTIVE, "invalid state");

  dma2dp->state = DMA2D_PAUSED;
  DMA2D->CR |= DMA2D_CR_SUSP;
}

/**
 * @brief   Suspend current job.
 * @details Suspends the current job. The driver is set to a paused state.
 * @pre     There is an active job.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dJobSuspend(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dJobSuspendI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Resume current job.
 * @details Resumes the current job.
 * @pre     There is a paused job.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dJobResumeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck((DMA2D->CR & DMA2D_CR_SUSP) != 0);
  osalDbgAssert(dma2dp->state == DMA2D_PAUSED, "invalid state");

  dma2dp->state = DMA2D_ACTIVE;
  DMA2D->CR &= ~DMA2D_CR_SUSP;
}

/**
 * @brief   Resume current job.
 * @details Resumes the current job.
 * @pre     There is a paused job.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dJobResume(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dJobResumeI(dma2dp);
  chSysUnlock();
}

/**
 * @brief   Abort current job.
 * @details Abots the current job (if any), and the driver becomes ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @iclass
 */
void dma2dJobAbortI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck((DMA2D->CR & DMA2D_CR_SUSP) == 0);
  osalDbgAssert(dma2dp->state >= DMA2D_READY, "invalid state");

  dma2dp->state = DMA2D_READY;
  DMA2D->CR |= DMA2D_CR_ABORT;
}

/**
 * @brief   Abort current job.
 * @details Abots the current job (if any), and the driver becomes ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @api
 */
void dma2dJobAbort(DMA2DDriver *dma2dp) {

  chSysLock();
  dma2dJobAbortI(dma2dp);
  chSysUnlock();
}

/** @} */

/**
 * @name    DMA2D background layer methods
 * @{
 */

/**
 * @brief   Get background layer buffer address.
 * @details Gets the buffer address of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @iclass
 */
void *dma2dBgGetAddressI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (void *)DMA2D->BGMAR;
}

/**
 * @brief   Get background layer buffer address.
 * @details Gets the buffer address of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @api
 */
void *dma2dBgGetAddress(DMA2DDriver *dma2dp) {

  void *bufferp;
  chSysLock();
  bufferp = dma2dBgGetAddressI(dma2dp);
  chSysUnlock();
  return bufferp;
}

/**
 * @brief   Set background layer buffer address.
 * @details Sets the buffer address of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @iclass
 */
void dma2dBgSetAddressI(DMA2DDriver *dma2dp, void *bufferp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(dma2dIsAligned(bufferp, dma2dBgGetPixelFormatI(dma2dp)));
  (void)dma2dp;

  DMA2D->BGMAR = (uint32_t)bufferp;
}

/**
 * @brief   Set background layer buffer address.
 * @details Sets the buffer address of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @api
 */
void dma2dBgSetAddress(DMA2DDriver *dma2dp, void *bufferp) {

  chSysLock();
  dma2dBgSetAddressI(dma2dp, bufferp);
  chSysUnlock();
}

/**
 * @brief   Get background layer wrap offset.
 * @details Gets the buffer line wrap offset of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @iclass
 */
size_t dma2dBgGetWrapOffsetI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (size_t)(DMA2D->BGOR & DMA2D_BGOR_LO);
}

/**
 * @brief   Get background layer wrap offset.
 * @details Gets the buffer line wrap offset of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @api
 */
size_t dma2dBgGetWrapOffset(DMA2DDriver *dma2dp) {

  size_t offset;
  chSysLock();
  offset = dma2dBgGetWrapOffsetI(dma2dp);
  chSysUnlock();
  return offset;
}

/**
 * @brief   Set background layer wrap offset.
 * @details Sets the buffer line wrap offset of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @iclass
 */
void dma2dBgSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(offset <= DMA2D_MAX_OFFSET, "bounds");
  (void)dma2dp;

  DMA2D->BGOR = ((DMA2D->BGOR & ~DMA2D_BGOR_LO) |
                 ((uint32_t)offset & DMA2D_BGOR_LO));
}

/**
 * @brief   Set background layer wrap offset.
 * @details Sets the buffer line wrap offset of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @api
 */
void dma2dBgSetWrapOffset(DMA2DDriver *dma2dp, size_t offset) {

  chSysLock();
  dma2dBgSetWrapOffsetI(dma2dp, offset);
  chSysUnlock();
}

/**
 * @brief   Get background layer constant alpha.
 * @details Gets the constant alpha component of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @iclass
 */
uint8_t dma2dBgGetConstantAlphaI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (uint8_t)((DMA2D->BGPFCCR & DMA2D_BGPFCCR_ALPHA) >> 24);
}

/**
 * @brief   Get background layer constant alpha.
 * @details Gets the constant alpha component of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @api
 */
uint8_t dma2dBgGetConstantAlpha(DMA2DDriver *dma2dp) {

  uint8_t a;
  chSysLock();
  a = dma2dBgGetConstantAlphaI(dma2dp);
  chSysUnlock();
  return a;
}

/**
 * @brief   Set background layer constant alpha.
 * @details Sets the constant alpha component of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @iclass
 */
void dma2dBgSetConstantAlphaI(DMA2DDriver *dma2dp, uint8_t a) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->BGPFCCR = ((DMA2D->BGPFCCR & ~DMA2D_BGPFCCR_ALPHA) |
                    (((uint32_t)a << 24) & DMA2D_BGPFCCR_ALPHA));
}

/**
 * @brief   Set background layer constant alpha.
 * @details Sets the constant alpha component of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @api
 */
void dma2dBgSetConstantAlpha(DMA2DDriver *dma2dp, uint8_t a) {

  chSysLock();
  dma2dBgSetConstantAlphaI(dma2dp, a);
  chSysUnlock();
}

/**
 * @brief   Get background layer alpha mode.
 * @details Gets the alpha mode of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              alpha mode
 *
 * @iclass
 */
dma2d_amode_t dma2dBgGetAlphaModeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_amode_t)(DMA2D->BGPFCCR & DMA2D_BGPFCCR_AM);
}

/**
 * @brief   Get background layer alpha mode.
 * @details Gets the alpha mode of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              alpha mode
 *
 * @api
 */
dma2d_amode_t dma2dBgGetAlphaMode(DMA2DDriver *dma2dp) {

  dma2d_amode_t mode;
  chSysLock();
  mode = dma2dBgGetAlphaModeI(dma2dp);
  chSysUnlock();
  return mode;
}

/**
 * @brief   Set background layer alpha mode.
 * @details Sets the alpha mode of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      alpha mode
 *
 * @iclass
 */
void dma2dBgSetAlphaModeI(DMA2DDriver *dma2dp, dma2d_amode_t mode) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert((mode & ~DMA2D_BGPFCCR_AM) == 0, "bounds");
  osalDbgAssert((mode & DMA2D_BGPFCCR_AM) != DMA2D_BGPFCCR_AM, "bounds");
  (void)dma2dp;

  DMA2D->BGPFCCR = ((DMA2D->BGPFCCR & ~DMA2D_BGPFCCR_AM) |
                    ((uint32_t)mode & DMA2D_BGPFCCR_AM));
}

/**
 * @brief   Set background layer alpha mode.
 * @details Sets the alpha mode of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      alpha mode
 *
 * @api
 */
void dma2dBgSetAlphaMode(DMA2DDriver *dma2dp, dma2d_amode_t mode) {

  chSysLock();
  dma2dBgSetAlphaModeI(dma2dp, mode);
  chSysUnlock();
}

/**
 * @brief   Get background layer pixel format.
 * @details Gets the pixel format of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @iclass
 */
dma2d_pixfmt_t dma2dBgGetPixelFormatI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_pixfmt_t)(DMA2D->BGPFCCR & DMA2D_BGPFCCR_CM);
}

/**
 * @brief   Get background layer pixel format.
 * @details Gets the pixel format of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @api
 */
dma2d_pixfmt_t dma2dBgGetPixelFormat(DMA2DDriver *dma2dp) {

  dma2d_pixfmt_t fmt;
  chSysLock();
  fmt = dma2dBgGetPixelFormatI(dma2dp);
  chSysUnlock();
  return fmt;
}

/**
 * @brief   Set background layer pixel format.
 * @details Sets the pixel format of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @iclass
 */
void dma2dBgSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(fmt <= DMA2D_MAX_PIXFMT_ID, "bounds");
  (void)dma2dp;

  DMA2D->BGPFCCR = ((DMA2D->BGPFCCR & ~DMA2D_BGPFCCR_CM) |
                    ((uint32_t)fmt & DMA2D_BGPFCCR_CM));
}

/**
 * @brief   Set background layer pixel format.
 * @details Sets the pixel format of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @api
 */
void dma2dBgSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  chSysLock();
  dma2dBgSetPixelFormatI(dma2dp, fmt);
  chSysUnlock();
}

/**
 * @brief   Get background layer default color.
 * @details Gets the default color of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, RGB-888
 *
 * @iclass
 */
dma2d_color_t dma2dBgGetDefaultColorI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_color_t)(DMA2D->BGCOLR & 0x00FFFFFF);
}

/**
 * @brief   Get background layer default color.
 * @details Gets the default color of the background layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, RGB-888
 *
 * @api
 */
dma2d_color_t dma2dBgGetDefaultColor(DMA2DDriver *dma2dp) {

  dma2d_color_t c;
  chSysLock();
  c = dma2dBgGetDefaultColorI(dma2dp);
  chSysUnlock();
  return c;
}

/**
 * @brief   Set background layer default color.
 * @details Sets the default color of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, RGB-888
 *
 * @iclass
 */
void dma2dBgSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->BGCOLR = (uint32_t)c & 0x00FFFFFF;
}

/**
 * @brief   Set background layer default color.
 * @details Sets the default color of the background layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, RGB-888
 *
 * @api
 */
void dma2dBgSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c) {

  chSysLock();
  dma2dBgSetDefaultColorI(dma2dp, c);
  chSysUnlock();
}

/**
 * @brief   Get background layer palette specifications.
 * @details Gets the palette specifications of the background layer.
 * @note    The palette colors pointer is actually addressed to a @p volatile
 *          memory zone.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] palettep pointer to the palette specifications
 *
 * @iclass
 */
void dma2dBgGetPaletteI(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep) {

  uint32_t r;

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(palettep != NULL);
  (void)dma2dp;

  r = DMA2D->BGPFCCR;
  palettep->colorsp = (const void *)DMA2D->BGCLUT;
  palettep->length = (uint16_t)((r & DMA2D_BGPFCCR_CS) >> 8) + 1;
  palettep->fmt = (dma2d_pixfmt_t)((r & DMA2D_BGPFCCR_CCM) >> 4);
}

/**
 * @brief   Get background layer palette specifications.
 * @details Gets the palette specifications of the background layer.
 * @note    The palette colors pointer is actually addressed to a @p volatile
 *          memory zone.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] palettep pointer to the palette specifications
 *
 * @api
 */
void dma2dBgGetPalette(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep) {

  chSysLock();
  dma2dBgGetPaletteI(dma2dp, palettep);
  chSysUnlock();
}

/**
 * @brief   Set background layer palette specifications.
 * @details Sets the palette specifications of the background layer.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] palettep  pointer to the palette specifications
 *
 * @sclass
 */
void dma2dBgSetPaletteS(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(palettep != NULL);
  osalDbgCheck(palettep->colorsp != NULL);
  osalDbgAssert(palettep->length > 0, "bounds");
  osalDbgAssert(palettep->length <= DMA2D_MAX_PALETTE_LENGTH, "bounds");
  osalDbgAssert(((palettep->fmt == DMA2D_FMT_ARGB8888) ||
                 (palettep->fmt == DMA2D_FMT_RGB888)), "invalid format");

  DMA2D->BGCMAR = (uint32_t)palettep->colorsp;
  DMA2D->BGPFCCR = (
    (DMA2D->BGPFCCR & ~(DMA2D_BGPFCCR_CS | DMA2D_BGPFCCR_CCM)) |
    ((((uint32_t)palettep->length - 1) << 8) & DMA2D_BGPFCCR_CS) |
    ((uint32_t)palettep->fmt << 4)
  );

  dma2dp->state = DMA2D_ACTIVE;
  DMA2D->BGPFCCR |= DMA2D_BGPFCCR_START;

#if DMA2D_USE_WAIT
  dma2dp->thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
#else
  while (DMA2D->BGPFCCR & DMA2D_BGPFCCR_START)
    chSchDoYieldS();
#endif  /* DMA2D_USE_WAIT */
}

/**
 * @brief   Set background layer palette specifications.
 * @details Sets the palette specifications of the background layer.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] palettep  pointer to the palette specifications
 *
 * @api
 */
void dma2dBgSetPalette(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep) {

  chSysLock();
  dma2dBgSetPaletteS(dma2dp, palettep);
  chSysUnlock();
}

/**
 * @brief   Get background layer specifications.
 * @details Gets the background layer specifications at once.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @iclass
 */
void dma2dBgGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(cfgp != NULL);

  cfgp->bufferp = dma2dBgGetAddressI(dma2dp);
  cfgp->wrap_offset = dma2dBgGetWrapOffsetI(dma2dp);
  cfgp->fmt = dma2dBgGetPixelFormatI(dma2dp);
  cfgp->def_color = dma2dBgGetDefaultColorI(dma2dp);
  cfgp->const_alpha = dma2dBgGetConstantAlphaI(dma2dp);
  if (cfgp->palettep != NULL)
    dma2dBgGetPaletteI(dma2dp, (dma2d_palcfg_t *)cfgp->palettep);
}

/**
 * @brief   Get background layer specifications.
 * @details Gets the background layer specifications at once.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @api
 */
void dma2dBgGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dBgGetLayerI(dma2dp, cfgp);
  chSysUnlock();
}

/**
 * @brief   Set background layer specifications.
 * @details Sets the background layer specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @sclass
 */
void dma2dBgSetConfigS(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(cfgp != NULL);

  dma2dBgSetAddressI(dma2dp, cfgp->bufferp);
  dma2dBgSetWrapOffsetI(dma2dp, cfgp->wrap_offset);
  dma2dBgSetPixelFormatI(dma2dp, cfgp->fmt);
  dma2dBgSetDefaultColorI(dma2dp, cfgp->def_color);
  dma2dBgSetConstantAlphaI(dma2dp, cfgp->const_alpha);
  if (cfgp->palettep != NULL)
    dma2dBgSetPaletteS(dma2dp, cfgp->palettep);
}

/**
 * @brief   Set background layer specifications.
 * @details Sets the background layer specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @api
 */
void dma2dBgSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dBgSetConfigS(dma2dp, cfgp);
  chSysUnlock();
}

/** @} */

/**
 * @name    DMA2D foreground layer methods
 * @{
 */

/**
 * @brief   Get foreground layer buffer address.
 * @details Gets the buffer address of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @iclass
 */
void *dma2dFgGetAddressI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (void *)DMA2D->FGMAR;
}

/**
 * @brief   Get foreground layer buffer address.
 * @details Gets the buffer address of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @api
 */
void *dma2dFgGetAddress(DMA2DDriver *dma2dp) {

  void *bufferp;
  chSysLock();
  bufferp = dma2dFgGetAddressI(dma2dp);
  chSysUnlock();
  return bufferp;
}

/**
 * @brief   Set foreground layer buffer address.
 * @details Sets the buffer address of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @iclass
 */
void dma2dFgSetAddressI(DMA2DDriver *dma2dp, void *bufferp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(dma2dIsAligned(bufferp, dma2dFgGetPixelFormatI(dma2dp)));
  (void)dma2dp;

  DMA2D->FGMAR = (uint32_t)bufferp;
}

/**
 * @brief   Set foreground layer buffer address.
 * @details Sets the buffer address of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @api
 */
void dma2dFgSetAddress(DMA2DDriver *dma2dp, void *bufferp) {

  chSysLock();
  dma2dFgSetAddressI(dma2dp, bufferp);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer wrap offset.
 * @details Gets the buffer line wrap offset of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @iclass
 */
size_t dma2dFgGetWrapOffsetI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (size_t)(DMA2D->FGOR & DMA2D_FGOR_LO);
}

/**
 * @brief   Get foreground layer wrap offset.
 * @details Gets the buffer line wrap offset of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @api
 */
size_t dma2dFgGetWrapOffset(DMA2DDriver *dma2dp) {

  size_t offset;
  chSysLock();
  offset = dma2dFgGetWrapOffsetI(dma2dp);
  chSysUnlock();
  return offset;
}

/**
 * @brief   Set foreground layer wrap offset.
 * @details Sets the buffer line wrap offset of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @iclass
 */
void dma2dFgSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(offset <= DMA2D_MAX_OFFSET, "bounds");
  (void)dma2dp;

  DMA2D->FGOR = ((DMA2D->FGOR & ~DMA2D_FGOR_LO) |
                 ((uint32_t)offset & DMA2D_FGOR_LO));
}

/**
 * @brief   Set foreground layer wrap offset.
 * @details Sets the buffer line wrap offset of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @api
 */
void dma2dFgSetWrapOffset(DMA2DDriver *dma2dp, size_t offset) {

  chSysLock();
  dma2dFgSetWrapOffsetI(dma2dp, offset);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer constant alpha.
 * @details Gets the constant alpha component of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @iclass
 */
uint8_t dma2dFgGetConstantAlphaI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (uint8_t)((DMA2D->FGPFCCR & DMA2D_FGPFCCR_ALPHA) >> 24);
}

/**
 * @brief   Get foreground layer constant alpha.
 * @details Gets the constant alpha component of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              constant alpha component, A-8
 *
 * @api
 */
uint8_t dma2dFgGetConstantAlpha(DMA2DDriver *dma2dp) {

  uint8_t a;
  chSysLock();
  a = dma2dFgGetConstantAlphaI(dma2dp);
  chSysUnlock();
  return a;
}

/**
 * @brief   Set foreground layer constant alpha.
 * @details Sets the constant alpha component of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @iclass
 */
void dma2dFgSetConstantAlphaI(DMA2DDriver *dma2dp, uint8_t a) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->FGPFCCR = ((DMA2D->FGPFCCR & ~DMA2D_FGPFCCR_ALPHA) |
                    (((uint32_t)a << 24) & DMA2D_FGPFCCR_ALPHA));
}

/**
 * @brief   Set foreground layer constant alpha.
 * @details Sets the constant alpha component of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] a         constant alpha component, A-8
 *
 * @api
 */
void dma2dFgSetConstantAlpha(DMA2DDriver *dma2dp, uint8_t a) {

  chSysLock();
  dma2dFgSetConstantAlphaI(dma2dp, a);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer alpha mode.
 * @details Gets the alpha mode of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              alpha mode
 *
 * @iclass
 */
dma2d_amode_t dma2dFgGetAlphaModeI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_amode_t)(DMA2D->FGPFCCR & DMA2D_FGPFCCR_AM);
}

/**
 * @brief   Get foreground layer alpha mode.
 * @details Gets the alpha mode of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              alpha mode
 *
 * @api
 */
dma2d_amode_t dma2dFgGetAlphaMode(DMA2DDriver *dma2dp) {

  dma2d_amode_t mode;
  chSysLock();
  mode = dma2dFgGetAlphaModeI(dma2dp);
  chSysUnlock();
  return mode;
}

/**
 * @brief   Set foreground layer alpha mode.
 * @details Sets the alpha mode of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      alpha mode
 *
 * @iclass
 */
void dma2dFgSetAlphaModeI(DMA2DDriver *dma2dp, dma2d_amode_t mode) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert((mode & ~DMA2D_FGPFCCR_AM) == 0, "bounds");
  osalDbgAssert((mode & DMA2D_FGPFCCR_AM) != DMA2D_FGPFCCR_AM, "bounds");
  (void)dma2dp;

  DMA2D->FGPFCCR = ((DMA2D->FGPFCCR & ~DMA2D_FGPFCCR_AM) |
                    ((uint32_t)mode & DMA2D_FGPFCCR_AM));
}

/**
 * @brief   Set foreground layer alpha mode.
 * @details Sets the alpha mode of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] mode      alpha mode
 *
 * @api
 */
void dma2dFgSetAlphaMode(DMA2DDriver *dma2dp, dma2d_amode_t mode) {

  chSysLock();
  dma2dFgSetAlphaModeI(dma2dp, mode);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer pixel format.
 * @details Gets the pixel format of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @iclass
 */
dma2d_pixfmt_t dma2dFgGetPixelFormatI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_pixfmt_t)(DMA2D->FGPFCCR & DMA2D_FGPFCCR_CM);
}

/**
 * @brief   Get foreground layer pixel format.
 * @details Gets the pixel format of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @api
 */
dma2d_pixfmt_t dma2dFgGetPixelFormat(DMA2DDriver *dma2dp) {

  dma2d_pixfmt_t fmt;
  chSysLock();
  fmt = dma2dFgGetPixelFormatI(dma2dp);
  chSysUnlock();
  return fmt;
}

/**
 * @brief   Set foreground layer pixel format.
 * @details Sets the pixel format of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @iclass
 */
void dma2dFgSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(fmt <= DMA2D_MAX_PIXFMT_ID, "bounds");
  (void)dma2dp;

  DMA2D->FGPFCCR = ((DMA2D->FGPFCCR & ~DMA2D_FGPFCCR_CM) |
                    ((uint32_t)fmt & DMA2D_FGPFCCR_CM));
}

/**
 * @brief   Set foreground layer pixel format.
 * @details Sets the pixel format of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @api
 */
void dma2dFgSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  chSysLock();
  dma2dFgSetPixelFormatI(dma2dp, fmt);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer default color.
 * @details Gets the default color of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, RGB-888
 *
 * @iclass
 */
dma2d_color_t dma2dFgGetDefaultColorI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_color_t)(DMA2D->FGCOLR & 0x00FFFFFF);
}

/**
 * @brief   Get foreground layer default color.
 * @details Gets the default color of the foreground layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, RGB-888
 *
 * @api
 */
dma2d_color_t dma2dFgGetDefaultColor(DMA2DDriver *dma2dp) {

  dma2d_color_t c;
  chSysLock();
  c = dma2dFgGetDefaultColorI(dma2dp);
  chSysUnlock();
  return c;
}

/**
 * @brief   Set foreground layer default color.
 * @details Sets the default color of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, RGB-888
 *
 * @iclass
 */
void dma2dFgSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->FGCOLR = (uint32_t)c & 0x00FFFFFF;
}

/**
 * @brief   Set foreground layer default color.
 * @details Sets the default color of the foreground layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, RGB-888
 *
 * @api
 */
void dma2dFgSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c) {

  chSysLock();
  dma2dFgSetDefaultColorI(dma2dp, c);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer palette specifications.
 * @details Gets the palette specifications of the foreground layer.
 * @note    The palette colors pointer is actually addressed to a @p volatile
 *          memory zone.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] palettep pointer to the palette specifications
 *
 * @iclass
 */
void dma2dFgGetPaletteI(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep) {

  uint32_t r;

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(palettep != NULL);
  (void)dma2dp;

  r = DMA2D->FGPFCCR;
  palettep->colorsp = (const void *)DMA2D->FGCLUT;
  palettep->length = (uint16_t)((r & DMA2D_FGPFCCR_CS) >> 8) + 1;
  palettep->fmt = (dma2d_pixfmt_t)((r & DMA2D_FGPFCCR_CCM) >> 4);
}

/**
 * @brief   Get foreground layer palette specifications.
 * @details Gets the palette specifications of the foreground layer.
 * @note    The palette colors pointer is actually addressed to a @p volatile
 *          memory zone.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] palettep pointer to the palette specifications
 *
 * @api
 */
void dma2dFgGetPalette(DMA2DDriver *dma2dp, dma2d_palcfg_t *palettep) {

  chSysLock();
  dma2dFgGetPaletteI(dma2dp, palettep);
  chSysUnlock();
}

/**
 * @brief   Set foreground layer palette specifications.
 * @details Sets the palette specifications of the foreground layer.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] palettep  pointer to the palette specifications
 *
 * @sclass
 */
void dma2dFgSetPaletteS(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(palettep != NULL);
  osalDbgCheck(palettep->colorsp != NULL);
  osalDbgAssert(palettep->length > 0, "bounds");
  osalDbgAssert(palettep->length <= DMA2D_MAX_PALETTE_LENGTH, "bounds");
  osalDbgAssert(((palettep->fmt == DMA2D_FMT_ARGB8888) ||
                 (palettep->fmt == DMA2D_FMT_RGB888)), "invalid format");

  DMA2D->FGCMAR = (uint32_t)palettep->colorsp;
  DMA2D->FGPFCCR = (
    (DMA2D->FGPFCCR & ~(DMA2D_FGPFCCR_CS | DMA2D_FGPFCCR_CCM)) |
    ((((uint32_t)palettep->length - 1) << 8) & DMA2D_FGPFCCR_CS) |
    ((uint32_t)palettep->fmt << 4)
  );

  dma2dp->state = DMA2D_ACTIVE;
  DMA2D->FGPFCCR |= DMA2D_FGPFCCR_START;

#if DMA2D_USE_WAIT
  dma2dp->thread = chThdGetSelfX();
  chSchGoSleepS(CH_STATE_SUSPENDED);
#else
  while (DMA2D->FGPFCCR & DMA2D_FGPFCCR_START)
    chSchDoYieldS();
#endif  /* DMA2D_USE_WAIT */
}

/**
 * @brief   Set foreground layer palette specifications.
 * @details Sets the palette specifications of the foreground layer.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] palettep  pointer to the palette specifications
 *
 * @api
 */
void dma2dFgSetPalette(DMA2DDriver *dma2dp, const dma2d_palcfg_t *palettep) {

  chSysLock();
  dma2dFgSetPaletteS(dma2dp, palettep);
  chSysUnlock();
}

/**
 * @brief   Get foreground layer specifications.
 * @details Gets the foreground layer specifications at once.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @iclass
 */
void dma2dFgGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(cfgp != NULL);

  cfgp->bufferp = dma2dFgGetAddressI(dma2dp);
  cfgp->wrap_offset = dma2dFgGetWrapOffsetI(dma2dp);
  cfgp->fmt = dma2dFgGetPixelFormatI(dma2dp);
  cfgp->def_color = dma2dFgGetDefaultColorI(dma2dp);
  cfgp->const_alpha = dma2dFgGetConstantAlphaI(dma2dp);
  if (cfgp->palettep != NULL)
    dma2dFgGetPaletteI(dma2dp, (dma2d_palcfg_t *)cfgp->palettep);
}

/**
 * @brief   Get foreground layer specifications.
 * @details Gets the foreground layer specifications at once.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @api
 */
void dma2dFgGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dFgGetLayerI(dma2dp, cfgp);
  chSysUnlock();
}

/**
 * @brief   Set foreground layer specifications.
 * @details Sets the foreground layer specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @sclass
 */
void dma2dFgSetConfigS(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassS();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(cfgp != NULL);

  dma2dFgSetAddressI(dma2dp, cfgp->bufferp);
  dma2dFgSetWrapOffsetI(dma2dp, cfgp->wrap_offset);
  dma2dFgSetPixelFormatI(dma2dp, cfgp->fmt);
  dma2dFgSetDefaultColorI(dma2dp, cfgp->def_color);
  dma2dFgSetConstantAlphaI(dma2dp, cfgp->const_alpha);
  if (cfgp->palettep != NULL)
    dma2dFgSetPaletteS(dma2dp, cfgp->palettep);
}

/**
 * @brief   Set foreground layer specifications.
 * @details Sets the foreground layer specifications at once.
 * @note    If the palette is unspecified, the layer palette is unmodified.
 * @note    This function should not be called while the DMA2D is already
 *          executing a job, otherwise the appropriate error interrupt might be
 *          invoked.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @api
 */
void dma2dFgSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dFgSetConfigS(dma2dp, cfgp);
  chSysUnlock();
}

/** @} */

/**
 * @name    DMA2D output layer methods
 * @{
 */

/**
 * @brief   Get output layer buffer address.
 * @details Gets the buffer address of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @iclass
 */
void *dma2dOutGetAddressI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (void *)DMA2D->OMAR;
}

/**
 * @brief   Get output layer buffer address.
 * @details Gets the buffer address of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              buffer address
 *
 * @api
 */
void *dma2dOutGetAddress(DMA2DDriver *dma2dp) {

  void *bufferp;
  chSysLock();
  bufferp = dma2dOutGetAddressI(dma2dp);
  chSysUnlock();
  return bufferp;
}

/**
 * @brief   Set output layer buffer address.
 * @details Sets the buffer address of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @iclass
 */
void dma2dOutSetAddressI(DMA2DDriver *dma2dp, void *bufferp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(dma2dIsAligned(bufferp, dma2dOutGetPixelFormatI(dma2dp)));
  (void)dma2dp;

  DMA2D->OMAR = (uint32_t)bufferp;
}

/**
 * @brief   Set output layer buffer address.
 * @details Sets the buffer address of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] bufferp   buffer address
 *
 * @api
 */
void dma2dOutSetAddress(DMA2DDriver *dma2dp, void *bufferp) {

  chSysLock();
  dma2dOutSetAddressI(dma2dp, bufferp);
  chSysUnlock();
}

/**
 * @brief   Get output layer wrap offset.
 * @details Gets the buffer line wrap offset of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @iclass
 */
size_t dma2dOutGetWrapOffsetI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (size_t)(DMA2D->OOR & DMA2D_OOR_LO);
}

/**
 * @brief   Get output layer wrap offset.
 * @details Gets the buffer line wrap offset of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              wrap offset, in pixels
 *
 * @api
 */
size_t dma2dOutGetWrapOffset(DMA2DDriver *dma2dp) {

  size_t offset;
  chSysLock();
  offset = dma2dOutGetWrapOffsetI(dma2dp);
  chSysUnlock();
  return offset;
}

/**
 * @brief   Set output layer wrap offset.
 * @details Sets the buffer line wrap offset of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @iclass
 */
void dma2dOutSetWrapOffsetI(DMA2DDriver *dma2dp, size_t offset) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(offset <= DMA2D_MAX_OFFSET, "bounds");
  (void)dma2dp;

  DMA2D->OOR = ((DMA2D->OOR & ~DMA2D_OOR_LO) |
                ((uint32_t)offset & DMA2D_OOR_LO));
}

/**
 * @brief   Set output layer wrap offset.
 * @details Sets the buffer line wrap offset of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] offset    wrap offset, in pixels
 *
 * @api
 */
void dma2dOutSetWrapOffset(DMA2DDriver *dma2dp, size_t offset) {

  chSysLock();
  dma2dOutSetWrapOffsetI(dma2dp, offset);
  chSysUnlock();
}

/**
 * @brief   Get output layer pixel format.
 * @details Gets the pixel format of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @iclass
 */
dma2d_pixfmt_t dma2dOutGetPixelFormatI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_pixfmt_t)(DMA2D->OPFCCR & DMA2D_OPFCCR_CM);
}

/**
 * @brief   Get output layer pixel format.
 * @details Gets the pixel format of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              pixel format
 *
 * @api
 */
dma2d_pixfmt_t dma2dOutGetPixelFormat(DMA2DDriver *dma2dp) {

  dma2d_pixfmt_t fmt;
  chSysLock();
  fmt = dma2dOutGetPixelFormatI(dma2dp);
  chSysUnlock();
  return fmt;
}

/**
 * @brief   Set output layer pixel format.
 * @details Sets the pixel format of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @iclass
 */
void dma2dOutSetPixelFormatI(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgAssert(fmt <= DMA2D_MAX_OUTPIXFMT_ID, "bounds");
  (void)dma2dp;

  DMA2D->OPFCCR = ((DMA2D->OPFCCR & ~DMA2D_OPFCCR_CM) |
                   ((uint32_t)fmt & DMA2D_OPFCCR_CM));
}

/**
 * @brief   Set output layer pixel format.
 * @details Sets the pixel format of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] fmt       pixel format
 *
 * @api
 */
void dma2dOutSetPixelFormat(DMA2DDriver *dma2dp, dma2d_pixfmt_t fmt) {

  chSysLock();
  dma2dOutSetPixelFormatI(dma2dp, fmt);
  chSysUnlock();
}

/**
 * @brief   Get output layer default color.
 * @details Gets the default color of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, chosen output format
 *
 * @iclass
 */
dma2d_color_t dma2dOutGetDefaultColorI(DMA2DDriver *dma2dp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  (void)dma2dp;

  return (dma2d_color_t)(DMA2D->OCOLR & 0x00FFFFFF);
}

/**
 * @brief   Get output layer default color.
 * @details Gets the default color of the output layer.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 *
 * @return              default color, chosen output format
 *
 * @api
 */
dma2d_color_t dma2dOutGetDefaultColor(DMA2DDriver *dma2dp) {

  dma2d_color_t c;
  chSysLock();
  c = dma2dOutGetDefaultColorI(dma2dp);
  chSysUnlock();
  return c;
}

/**
 * @brief   Set output layer default color.
 * @details Sets the default color of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, chosen output format
 *
 * @iclass
 */
void dma2dOutSetDefaultColorI(DMA2DDriver *dma2dp, dma2d_color_t c) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  (void)dma2dp;

  DMA2D->OCOLR = (uint32_t)c & 0x00FFFFFF;
}

/**
 * @brief   Set output layer default color.
 * @details Sets the default color of the output layer.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] c         default color, chosen output format
 *
 * @api
 */
void dma2dOutSetDefaultColor(DMA2DDriver *dma2dp, dma2d_color_t c) {

  chSysLock();
  dma2dOutSetDefaultColorI(dma2dp, c);
  chSysUnlock();
}

/**
 * @brief   Get output layer specifications.
 * @details Gets the output layer specifications at once.
 * @note    Constant alpha and palette specifications are ignored.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @iclass
 */
void dma2dOutGetLayerI(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgCheck(cfgp != NULL);

  cfgp->bufferp = dma2dOutGetAddressI(dma2dp);
  cfgp->wrap_offset = dma2dOutGetWrapOffsetI(dma2dp);
  cfgp->fmt = dma2dOutGetPixelFormatI(dma2dp);
  cfgp->def_color = dma2dOutGetDefaultColorI(dma2dp);
}

/**
 * @brief   Get output layer specifications.
 * @details Gets the output layer specifications at once.
 * @note    Constant alpha and palette specifications are ignored.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[out] cfgp     pointer to the layer specifications
 *
 * @api
 */
void dma2dOutGetLayer(DMA2DDriver *dma2dp, dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dOutGetLayerI(dma2dp, cfgp);
  chSysUnlock();
}

/**
 * @brief   Set output layer specifications.
 * @details Sets the output layer specifications at once.
 * @note    Constant alpha and palette specifications are ignored.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @iclass
 */
void dma2dOutSetConfigI(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  osalDbgCheckClassI();
  osalDbgCheck(dma2dp == &DMA2DD1);
  osalDbgAssert(dma2dp->state == DMA2D_READY, "not ready");
  osalDbgCheck(cfgp != NULL);

  dma2dOutSetAddressI(dma2dp, cfgp->bufferp);
  dma2dOutSetWrapOffsetI(dma2dp, cfgp->wrap_offset);
  dma2dOutSetPixelFormatI(dma2dp, cfgp->fmt);
  dma2dOutSetDefaultColorI(dma2dp, cfgp->def_color);
}

/**
 * @brief   Set output layer specifications.
 * @details Sets the output layer specifications at once.
 * @note    Constant alpha and palette specifications are ignored.
 * @pre     DMA2D is ready.
 *
 * @param[in] dma2dp    pointer to the @p DMA2DDriver object
 * @param[in] cfgp      pointer to the layer specifications
 *
 * @api
 */
void dma2dOutSetConfig(DMA2DDriver *dma2dp, const dma2d_laycfg_t *cfgp) {

  chSysLock();
  dma2dOutSetConfigI(dma2dp, cfgp);
  chSysUnlock();
}

/** @} */

/**
 * @name    DMA2D helper functions
 * @{
 */

/**
 * @brief   Compute pixel address.
 * @details Computes the buffer address of a pixel, given the buffer
 *          specifications.
 *
 * @param[in] originp   buffer origin address
 * @param[in] pitch     buffer pitch, in bytes
 * @param[in] fmt       buffer pixel format
 * @param[in] x         horizontal pixel coordinate
 * @param[in] y         vertical pixel coordinate
 *
 * @return              pixel address, constant data
 *
 * @api
 */
const void *dma2dComputeAddressConst(const void *originp, size_t pitch,
                                     dma2d_pixfmt_t fmt,
                                     uint16_t x, uint16_t y) {

  osalDbgCheck(pitch > 0);

  switch (fmt) {
  case DMA2D_FMT_ARGB8888:
    return (const void *)((uintptr_t)originp +
                          (uintptr_t)y * pitch + (uintptr_t)x * 4);
  case DMA2D_FMT_RGB888:
    return (const void *)((uintptr_t)originp +
                          (uintptr_t)y * pitch + (uintptr_t)x * 3);
  case DMA2D_FMT_RGB565:
  case DMA2D_FMT_ARGB1555:
  case DMA2D_FMT_ARGB4444:
  case DMA2D_FMT_AL88:
    return (const void *)((uintptr_t)originp +
                          (uintptr_t)y * pitch + (uintptr_t)x * 2);
  case DMA2D_FMT_L8:
  case DMA2D_FMT_AL44:
  case DMA2D_FMT_A8:
    return (const void *)((uintptr_t)originp +
                          (uintptr_t)y * pitch + (uintptr_t)x);
  case DMA2D_FMT_L4:
  case DMA2D_FMT_A4:
    osalDbgAssert((x & 1) == 0, "not aligned");
    return (const void *)((uintptr_t)originp +
                          (uintptr_t)y * pitch + (uintptr_t)x / 2);
  default:
    osalDbgAssert(false, "invalid format");
    return NULL;
  }
}

/**
 * @brief   Address is aligned.
 * @details Tells whether the address is aligned with the provided pixel format.
 *
 * @param[in] bufferp   address
 * @param[in] fmt       pixel format
 *
 * @return              address is aligned
 *
 * @api
 */
bool dma2dIsAligned(const void *bufferp, dma2d_pixfmt_t fmt) {

  switch (fmt) {
  case DMA2D_FMT_ARGB8888:
  case DMA2D_FMT_RGB888:
    return ((uintptr_t)bufferp & 3) == 0;   /* 32-bit alignment.*/
  case DMA2D_FMT_RGB565:
  case DMA2D_FMT_ARGB1555:
  case DMA2D_FMT_ARGB4444:
  case DMA2D_FMT_AL88:
    return ((uintptr_t)bufferp & 1) == 0;   /* 16-bit alignment.*/
  case DMA2D_FMT_L8:
  case DMA2D_FMT_AL44:
  case DMA2D_FMT_L4:
  case DMA2D_FMT_A8:
  case DMA2D_FMT_A4:
    return true;                            /* 8-bit alignment.*/
  default:
    osalDbgAssert(false, "invalid format");
    return false;
  }
}

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
size_t dma2dBitsPerPixel(dma2d_pixfmt_t fmt) {

  osalDbgAssert(fmt < DMA2D_MAX_PIXFMT_ID, "invalid format");

  return (size_t)dma2d_bpp[(unsigned)fmt];
}

#if DMA2D_USE_SOFTWARE_CONVERSIONS || defined(__DOXYGEN__)

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
dma2d_color_t dma2dFromARGB8888(dma2d_color_t c, dma2d_pixfmt_t fmt) {

  switch (fmt) {
  case DMA2D_FMT_ARGB8888: {
    return c;
  }
  case DMA2D_FMT_RGB888: {
    return (c & 0x00FFFFFF);
  }
  case DMA2D_FMT_RGB565: {
    return (((c & 0x000000F8) >> ( 8 -  5)) |
            ((c & 0x0000FC00) >> (16 - 11)) |
            ((c & 0x00F80000) >> (24 - 16)));
  }
  case DMA2D_FMT_ARGB1555: {
    return (((c & 0x000000F8) >> ( 8 -  5)) |
            ((c & 0x0000F800) >> (16 - 10)) |
            ((c & 0x00F80000) >> (24 - 15)) |
            ((c & 0x80000000) >> (32 - 16)));
  }
  case DMA2D_FMT_ARGB4444: {
    return (((c & 0x000000F0) >> ( 8 -  4)) |
            ((c & 0x0000F000) >> (16 -  8)) |
            ((c & 0x00F00000) >> (24 - 12)) |
            ((c & 0xF0000000) >> (32 - 16)));
  }
  case DMA2D_FMT_L8: {
    return (c & 0x000000FF);
  }
  case DMA2D_FMT_AL44: {
    return (((c & 0x000000F0) >> ( 8 - 4)) |
            ((c & 0xF0000000) >> (32 - 8)));
  }
  case DMA2D_FMT_AL88: {
    return (((c & 0x000000FF) >> ( 8 -  8)) |
            ((c & 0xFF000000) >> (32 - 16)));
  }
  case DMA2D_FMT_L4: {
    return (c & 0x0000000F);
  }
  case DMA2D_FMT_A8: {
    return ((c & 0xFF000000) >> (32 - 8));
  }
  case DMA2D_FMT_A4: {
    return ((c & 0xF0000000) >> (32 - 4));
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
dma2d_color_t dma2dToARGB8888(dma2d_color_t c, dma2d_pixfmt_t fmt) {

  switch (fmt) {
  case DMA2D_FMT_ARGB8888: {
    return c;
  }
  case DMA2D_FMT_RGB888: {
    return ((c & 0x00FFFFFF) | 0xFF000000);
  }
  case DMA2D_FMT_RGB565: {
    register dma2d_color_t output = 0xFF000000;
    if (c & 0x001F) output |= (((c & 0x001F) << ( 8 -  5)) | 0x00000007);
    if (c & 0x07E0) output |= (((c & 0x07E0) << (16 - 11)) | 0x00000300);
    if (c & 0xF800) output |= (((c & 0xF800) << (24 - 16)) | 0x00070000);
    return output;
  }
  case DMA2D_FMT_ARGB1555: {
    register dma2d_color_t output = 0x00000000;
    if (c & 0x001F) output |= (((c & 0x001F) << ( 8 -  5)) | 0x00000007);
    if (c & 0x03E0) output |= (((c & 0x03E0) << (16 - 10)) | 0x00000700);
    if (c & 0x7C00) output |= (((c & 0x7C00) << (24 - 15)) | 0x00070000);
    if (c & 0x8000) output |= 0xFF000000;
    return output;
  }
  case DMA2D_FMT_ARGB4444: {
    register dma2d_color_t output = 0x00000000;
    if (c & 0x000F) output |= (((c & 0x000F) << ( 8 -  4)) | 0x0000000F);
    if (c & 0x00F0) output |= (((c & 0x00F0) << (16 -  8)) | 0x00000F00);
    if (c & 0x0F00) output |= (((c & 0x0F00) << (24 - 12)) | 0x000F0000);
    if (c & 0xF000) output |= (((c & 0xF000) << (32 - 16)) | 0x0F000000);
    return output;
  }
  case DMA2D_FMT_L8: {
    return (c & 0xFF) | 0xFF000000;
  }
  case DMA2D_FMT_AL44: {
    register dma2d_color_t output = 0x00000000;
    if (c & 0x0F) output |= (((c & 0x0F) << ( 8 - 4)) | 0x0000000F);
    if (c & 0xF0) output |= (((c & 0xF0) << (32 - 8)) | 0x0F000000);
    return output;
  }
  case DMA2D_FMT_AL88: {
    return (((c & 0x00FF) << ( 8 -  8)) |
            ((c & 0xFF00) << (32 - 16)));
  }
  case DMA2D_FMT_L4: {
    return ((c & 0x0F) | 0xFF000000);
  }
  case DMA2D_FMT_A8: {
    return ((c & 0xFF) << (32 - 8));
  }
  case DMA2D_FMT_A4: {
    return ((c & 0x0F) << (32 - 4));
  }
  default:
    osalDbgAssert(false, "invalid format");
    return 0;
  }
}

#endif  /* DMA2D_NEED_CONVERSIONS */

/** @} */

/** @} */

#endif  /* STM32_DMA2D_USE_DMA2D */
