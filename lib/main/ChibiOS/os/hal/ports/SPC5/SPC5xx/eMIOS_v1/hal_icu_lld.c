/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    eMIOS_v1/hal_icu_lld.c
 * @brief   SPC5xx low level ICU driver code.
 *
 * @addtogroup ICU
 * @{
 */

#include "hal.h"

#if HAL_USE_ICU || defined(__DOXYGEN__)

#include "spc5_emios.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ICUD1 driver identifier.
 * @note    The driver ICUD1 allocates the unified channel eMIOS0_CH0
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH0 || defined(__DOXYGEN__)
ICUDriver ICUD1;
#endif

/**
 * @brief   ICUD2 driver identifier.
 * @note    The driver ICUD2 allocates the unified channel eMIOS0_CH1
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH1 || defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD3 driver identifier.
 * @note    The driver ICUD3 allocates the unified channel eMIOS0_CH2
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH2 || defined(__DOXYGEN__)
ICUDriver ICUD3;
#endif

/**
 * @brief   ICUD4 driver identifier.
 * @note    The driver ICUD4 allocates the unified channel eMIOS0_CH3
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH3 || defined(__DOXYGEN__)
ICUDriver ICUD4;
#endif

/**
 * @brief   ICUD5 driver identifier.
 * @note    The driver ICUD5 allocates the unified channel eMIOS0_CH4
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH4 || defined(__DOXYGEN__)
ICUDriver ICUD5;
#endif

/**
 * @brief   ICUD6 driver identifier.
 * @note    The driver ICUD6 allocates the unified channel eMIOS0_CH5
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH5 || defined(__DOXYGEN__)
ICUDriver ICUD6;
#endif

/**
 * @brief   ICUD7 driver identifier.
 * @note    The driver ICUD7 allocates the unified channel eMIOS0_CH6
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH6 || defined(__DOXYGEN__)
ICUDriver ICUD7;
#endif

/**
 * @brief   ICUD8 driver identifier.
 * @note    The driver ICUD8 allocates the unified channel eMIOS0_CH7
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH7 || defined(__DOXYGEN__)
ICUDriver ICUD8;
#endif

/**
 * @brief   ICUD9 driver identifier.
 * @note    The driver ICUD9 allocates the unified channel eMIOS0_CH24
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS0_CH24 || defined(__DOXYGEN__)
ICUDriver ICUD9;
#endif

/**
 * @brief   ICUD10 driver identifier.
 * @note    The driver ICUD10 allocates the unified channel eMIOS1_CH24
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS1_CH24 || defined(__DOXYGEN__)
ICUDriver ICUD10;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Width and Period registers.
 */
int16_t width;
int16_t period;

/**
 * @brief   A2 temp registers.
 */
uint16_t A2_1, A2_2, A2_3;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief               ICU IRQ handler.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 */
static void icu_lld_serve_interrupt(ICUDriver *icup) {
  uint32_t gfr = icup->emiosp->GFR.R;

  if (gfr & (1 << icup->ch_number)) {
    uint32_t sr = icup->emiosp->CH[icup->ch_number].CSR.R;

    if((sr & EMIOSS_OVFL) && (icup->config->overflow_cb != NULL)) {
      icup->emiosp->CH[icup->ch_number].CSR.R |= EMIOSS_OVFLC;
      _icu_isr_invoke_overflow_cb(icup);
    }
    if (sr & EMIOSS_FLAG) {
      icup->emiosp->CH[icup->ch_number].CSR.R |= EMIOSS_FLAGC;
      if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH) {
        if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 1U) &&       \
            (icup->config->period_cb != NULL)) {
          A2_3 = icup->emiosp->CH[icup->ch_number].CADR.R;
          period = A2_3 - A2_1;
          _icu_isr_invoke_period_cb(icup);
          A2_1 = A2_3;
        } else if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 0) && \
            (icup->config->width_cb != NULL)) {
          A2_2 = icup->emiosp->CH[icup->ch_number].CADR.R;
          width = A2_2 - A2_1;
          _icu_isr_invoke_width_cb(icup);
        }
      } else if (icup->config->mode == ICU_INPUT_ACTIVE_LOW) {
        if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 1U) &&       \
            (icup->config->width_cb != NULL)) {
          A2_2 = icup->emiosp->CH[icup->ch_number].CADR.R;
          width = A2_2 - A2_1;
          _icu_isr_invoke_width_cb(icup);
        } else if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 0) && \
            (icup->config->period_cb != NULL)) {
          A2_3 = icup->emiosp->CH[icup->ch_number].CADR.R;
          period = A2_3 - A2_1;
          _icu_isr_invoke_period_cb(icup);
          A2_1 = A2_3;
        }
      }
    }
    if (sr & EMIOSS_OVR) {
      icup->emiosp->CH[icup->ch_number].CSR.R |= EMIOSS_OVRC;
    }

  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_ICU_USE_EMIOS0_CH0 || SPC5_ICU_USE_EMIOS0_CH1
#if !defined(SPC5_EMIOS0_GFR_F0F1_HANDLER)
#error "SPC5_EMIOS0_GFR_F0F1_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 0 and 1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F0F1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS0_CH0
  icu_lld_serve_interrupt(&ICUD1);
#endif

#if SPC5_ICU_USE_EMIOS0_CH1
  icu_lld_serve_interrupt(&ICUD2);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS0_CH0 || SPC5_ICU_USE_EMIOS0_CH1 */

#if SPC5_ICU_USE_EMIOS0_CH2 || SPC5_ICU_USE_EMIOS0_CH3
#if !defined(SPC5_EMIOS0_GFR_F2F3_HANDLER)
#error "SPC5_EMIOS0_GFR_F2F3_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 2 and 3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F2F3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS0_CH2
  icu_lld_serve_interrupt(&ICUD3);
#endif

#if SPC5_ICU_USE_EMIOS0_CH3
  icu_lld_serve_interrupt(&ICUD4);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS0_CH2 || SPC5_ICU_USE_EMIOS0_CH3 */

#if SPC5_ICU_USE_EMIOS0_CH4 || SPC5_ICU_USE_EMIOS0_CH5
#if !defined(SPC5_EMIOS0_GFR_F4F5_HANDLER)
#error "SPC5_EMIOS0_GFR_F4F5_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 4 and 5 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F4F5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS0_CH4
  icu_lld_serve_interrupt(&ICUD5);
#endif

#if SPC5_ICU_USE_EMIOS0_CH5
  icu_lld_serve_interrupt(&ICUD6);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS0_CH4 || SPC5_ICU_USE_EMIOS0_CH5 */

#if SPC5_ICU_USE_EMIOS0_CH6 || SPC5_ICU_USE_EMIOS0_CH7

#if !defined(SPC5_EMIOS0_GFR_F6F7_HANDLER)
#error "SPC5_EMIOS0_GFR_F6F7_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 6 and 7 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F6F7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS0_CH6
  icu_lld_serve_interrupt(&ICUD7);
#endif

#if SPC5_ICU_USE_EMIOS0_CH7
  icu_lld_serve_interrupt(&ICUD8);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS0_CH6 || SPC5_ICU_USE_EMIOS0_CH7 */

#if SPC5_ICU_USE_EMIOS0_CH24
#if !defined(SPC5_EMIOS0_GFR_F24F25_HANDLER)
#error "SPC5_EMIOS0_GFR_F24F25_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 24 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F24F25_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS0_CH24
  icu_lld_serve_interrupt(&ICUD9);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS0_CH24 */

#if SPC5_ICU_USE_EMIOS1_CH24
#if !defined(SPC5_EMIOS1_GFR_F24F25_HANDLER)
#error "SPC5_EMIOS1_GFR_F24F25_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 24 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F24F25_HANDLER) {

  OSAL_IRQ_PROLOGUE();

#if SPC5_ICU_USE_EMIOS1_CH24
  icu_lld_serve_interrupt(&ICUD10);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS1_CH24 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ICU driver initialization.
 *
 * @notapi
 */
void icu_lld_init(void) {

  /* Initialize A2 temp registers.*/
  A2_1 = 0U;
  A2_2 = 0U;
  A2_3 = 0U;

  /* eMIOSx channels initially all not in use.*/
#if SPC5_HAS_EMIOS0
  reset_emios0_active_channels();
#endif
#if SPC5_HAS_EMIOS1
  reset_emios1_active_channels();
#endif

#if SPC5_ICU_USE_EMIOS0_CH0
  /* Driver initialization.*/
  icuObjectInit(&ICUD1);
  ICUD1.emiosp = &EMIOS_0;
  ICUD1.ch_number = 0U;
  ICUD1.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH0 */

#if SPC5_ICU_USE_EMIOS0_CH1
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.emiosp = &EMIOS_0;
  ICUD2.ch_number = 1U;
  ICUD2.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH1 */

#if SPC5_ICU_USE_EMIOS0_CH2
  /* Driver initialization.*/
  icuObjectInit(&ICUD3);
  ICUD3.emiosp = &EMIOS_0;
  ICUD3.ch_number = 2U;
  ICUD3.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH2 */

#if SPC5_ICU_USE_EMIOS0_CH3
  /* Driver initialization.*/
  icuObjectInit(&ICUD4);
  ICUD4.emiosp = &EMIOS_0;
  ICUD4.ch_number = 3U;
  ICUD4.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH3 */

#if SPC5_ICU_USE_EMIOS0_CH4
  /* Driver initialization.*/
  icuObjectInit(&ICUD5);
  ICUD5.emiosp = &EMIOS_0;
  ICUD5.ch_number = 4U;
  ICUD5.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH4 */

#if SPC5_ICU_USE_EMIOS0_CH5
  /* Driver initialization.*/
  icuObjectInit(&ICUD6);
  ICUD6.emiosp = &EMIOS_0;
  ICUD6.ch_number = 5U;
  ICUD6.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH5 */

#if SPC5_ICU_USE_EMIOS0_CH6
  /* Driver initialization.*/
  icuObjectInit(&ICUD7);
  ICUD7.emiosp = &EMIOS_0;
  ICUD7.ch_number = 6U;
  ICUD7.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH6 */

#if SPC5_ICU_USE_EMIOS0_CH7
  /* Driver initialization.*/
  icuObjectInit(&ICUD8);
  ICUD8.emiosp = &EMIOS_0;
  ICUD8.ch_number = 7U;
  ICUD8.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH7 */

#if SPC5_ICU_USE_EMIOS0_CH24
  /* Driver initialization.*/
  icuObjectInit(&ICUD9);
  ICUD9.emiosp = &EMIOS_0;
  ICUD9.ch_number = 24U;
  ICUD9.clock = SPC5_EMIOS0_CLK;
#endif /* SPC5_ICU_USE_EMIOS0_CH24 */

#if SPC5_ICU_USE_EMIOS1_CH24
  /* Driver initialization.*/
  icuObjectInit(&ICUD10);
  ICUD10.emiosp = &EMIOS_1;
  ICUD10.ch_number = 24U;
  ICUD10.clock = SPC5_EMIOS1_CLK;
#endif /* SPC5_ICU_USE_EMIOS1_CH24 */

#if SPC5_ICU_USE_EMIOS0

  INTC.PSR[SPC5_EMIOS0_GFR_F0F1_NUMBER].R = SPC5_EMIOS0_GFR_F0F1_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F2F3_NUMBER].R = SPC5_EMIOS0_GFR_F2F3_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F4F5_NUMBER].R = SPC5_EMIOS0_GFR_F4F5_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F6F7_NUMBER].R = SPC5_EMIOS0_GFR_F6F7_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F24F25_NUMBER].R = SPC5_EMIOS0_GFR_F24F25_PRIORITY;

#endif

#if SPC5_ICU_USE_EMIOS1

  INTC.PSR[SPC5_EMIOS1_GFR_F24F25_NUMBER].R = SPC5_EMIOS1_GFR_F24F25_PRIORITY;

#endif
}

/**
 * @brief   Configures and activates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start(ICUDriver *icup) {

#if SPC5_HAS_EMIOS0
  osalDbgAssert(get_emios0_active_channels() < 25, "too many channels");
#endif

#if SPC5_HAS_EMIOS1
  osalDbgAssert(get_emios1_active_channels() < 25, "too many channels");
#endif

  if (icup->state == ICU_STOP) {
    /* Enables the peripheral.*/
#if SPC5_ICU_USE_EMIOS0_CH0
    if (&ICUD1 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH0 */
#if SPC5_ICU_USE_EMIOS0_CH1
    if (&ICUD2 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH1 */
#if SPC5_ICU_USE_EMIOS0_CH2
    if (&ICUD3 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH2 */
#if SPC5_ICU_USE_EMIOS0_CH3
    if (&ICUD4 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH3 */
#if SPC5_ICU_USE_EMIOS0_CH4
    if (&ICUD5 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH4 */
#if SPC5_ICU_USE_EMIOS0_CH5
    if (&ICUD6 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH5 */
#if SPC5_ICU_USE_EMIOS0_CH6
    if (&ICUD7 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH6 */
#if SPC5_ICU_USE_EMIOS0_CH7
    if (&ICUD8 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH7 */
#if SPC5_ICU_USE_EMIOS0_CH24
    if (&ICUD9 == icup)
      increase_emios0_active_channels();
#endif /* SPC5_ICU_USE_EMIOS0_CH24 */
#if SPC5_ICU_USE_EMIOS1_CH24
    if (&ICUD10 == icup)
      increase_emios1_active_channels();
#endif /* SPC5_ICU_USE_EMIOS1_CH24 */

    /* Set eMIOS0 Clock.*/
#if SPC5_ICU_USE_EMIOS0
    icu_active_emios0_clock(icup);
#endif

    /* Set eMIOS1 Clock.*/
#if SPC5_ICU_USE_EMIOS1
    icu_active_emios1_clock(icup);
#endif

  }
  /* Configures the peripheral.*/

  /* Channel enables.*/
  icup->emiosp->UCDIS.R &= ~(1 << icup->ch_number);

  /* Clear pending IRQs (if any).*/
  icup->emiosp->CH[icup->ch_number].CSR.R = EMIOSS_OVRC |
      EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Set clock prescaler and control register.*/
  uint32_t psc = (icup->clock / icup->config->frequency);
  osalDbgAssert((psc <= 0xFFFF) &&
              (((psc) * icup->config->frequency) == icup->clock) &&
              ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
              "invalid frequency");

  icup->emiosp->CH[icup->ch_number].CCR.B.UCPEN = 0;
  icup->emiosp->CH[icup->ch_number].CCR.R |=
      EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER) |
      EMIOSC_EDSEL | EMIOS_CCR_MODE_SAIC;
  icup->emiosp->CH[icup->ch_number].CCR.B.UCPRE = psc - 1;
  icup->emiosp->CH[icup->ch_number].CCR.R |= EMIOSC_UCPREN;

  /* Set source polarity.*/
  if(icup->config->mode == ICU_INPUT_ACTIVE_HIGH){
    icup->emiosp->CH[icup->ch_number].CCR.R |= EMIOSC_EDPOL;
  } else {
    icup->emiosp->CH[icup->ch_number].CCR.R &= ~EMIOSC_EDPOL;
  }

  /* Direct pointers to the period and width registers in order to make
     reading data faster from within callbacks.*/
  icup->pccrp = &period;
  icup->wccrp = &width;

  /* Channel disables.*/
  icup->emiosp->UCDIS.R |= (1 << icup->ch_number);

}

/**
 * @brief   Deactivates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop(ICUDriver *icup) {

#if SPC5_HAS_EMIOS0
  osalDbgAssert(get_emios0_active_channels() < 25, "too many channels");
#endif
#if SPC5_HAS_EMIOS1
  osalDbgAssert(get_emios1_active_channels() < 25, "too many channels");
#endif

  if (icup->state == ICU_READY) {

    /* Disables the peripheral.*/
#if SPC5_ICU_USE_EMIOS0_CH0
    if (&ICUD1 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH0 */
#if SPC5_ICU_USE_EMIOS0_CH1
    if (&ICUD2 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH1 */
#if SPC5_ICU_USE_EMIOS0_CH2
    if (&ICUD3 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH2 */
#if SPC5_ICU_USE_EMIOS0_CH3
    if (&ICUD4 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH3 */
#if SPC5_ICU_USE_EMIOS0_CH4
    if (&ICUD5 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH4 */
#if SPC5_ICU_USE_EMIOS0_CH5
    if (&ICUD6 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH5 */
#if SPC5_ICU_USE_EMIOS0_CH6
    if (&ICUD7 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH6 */
#if SPC5_ICU_USE_EMIOS0_CH7
    if (&ICUD8 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH7 */
#if SPC5_ICU_USE_EMIOS0_CH24
    if (&ICUD9 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios0_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS0_CH24 */
#if SPC5_ICU_USE_EMIOS1_CH24
    if (&ICUD10 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios1_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS1_CH24 */

    /* eMIOS0 clock deactivation.*/
#if SPC5_ICU_USE_EMIOS0
    icu_deactive_emios0_clock(icup);
#endif

    /* eMIOS1 clock deactivation.*/
#if SPC5_ICU_USE_EMIOS1
    icu_deactive_emios1_clock(icup);
#endif
  }
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_enable(ICUDriver *icup) {

  /* Channel enables.*/
  /*
  if (!(icup->emiosp->UCDIS.R && (1 << icup->ch_number))) {

    icup->emiosp->UCDIS.R &= ~(1 << icup->ch_number);
  }
  */

  /* Channel enables.*/
  icup->emiosp->UCDIS.R &= ~(1 << icup->ch_number);

  /* Clear pending IRQs (if any).*/
  icup->emiosp->CH[icup->ch_number].CSR.R = EMIOSS_OVRC |
      EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Active interrupts.*/
  if (icup->config->period_cb != NULL || icup->config->width_cb != NULL ||  \
      icup->config->overflow_cb != NULL) {
    icup->emiosp->CH[icup->ch_number].CCR.B.FEN = 1U;
  }

}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_disable(ICUDriver *icup) {

  /* Clear pending IRQs (if any).*/
  icup->emiosp->CH[icup->ch_number].CSR.R = EMIOSS_OVRC |
        EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Disable interrupts.*/
  icup->emiosp->CH[icup->ch_number].CCR.B.FEN = 0;

  /* Channel disables.*/
  icup->emiosp->UCDIS.R |= (1 << icup->ch_number);

}

#endif /* HAL_USE_ICU */

/** @} */
