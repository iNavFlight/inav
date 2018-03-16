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
 * @file    SPC5xx/eMIOS200_v1/hal_icu_lld.c
 * @brief   SPC5xx low level icu driver code.
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
 * @note    The driver ICUD1 allocates the unified channel eMIOS_CH0
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH0 || defined(__DOXYGEN__)
ICUDriver ICUD1;
#endif

/**
 * @brief   ICUD2 driver identifier.
 * @note    The driver ICUD2 allocates the unified channel eMIOS_CH1
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH1 || defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD3 driver identifier.
 * @note    The driver ICUD3 allocates the unified channel eMIOS_CH2
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH2 || defined(__DOXYGEN__)
ICUDriver ICUD3;
#endif


/**
 * @brief   ICUD4 driver identifier.
 * @note    The driver ICUD4 allocates the unified channel eMIOS_CH3
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH3 || defined(__DOXYGEN__)
ICUDriver ICUD4;
#endif

/**
 * @brief   ICUD5 driver identifier.
 * @note    The driver ICUD5 allocates the unified channel eMIOS_CH4
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH4 || defined(__DOXYGEN__)
ICUDriver ICUD5;
#endif

/**
 * @brief   ICUD6 driver identifier.
 * @note    The driver ICUD6 allocates the unified channel eMIOS_CH5
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH5 || defined(__DOXYGEN__)
ICUDriver ICUD6;
#endif

/**
 * @brief   ICUD7 driver identifier.
 * @note    The driver ICUD7 allocates the unified channel eMIOS_CH6
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH6 || defined(__DOXYGEN__)
ICUDriver ICUD7;
#endif

/**
 * @brief   ICUD8 driver identifier.
 * @note    The driver ICUD8 allocates the unified channel eMIOS_CH7
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH7 || defined(__DOXYGEN__)
ICUDriver ICUD8;
#endif

/**
 * @brief   ICUD9 driver identifier.
 * @note    The driver ICUD9 allocates the unified channel eMIOS_CH8
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH8 || defined(__DOXYGEN__)
ICUDriver ICUD9;
#endif

/**
 * @brief   ICUD10 driver identifier.
 * @note    The driver ICUD10 allocates the unified channel eMIOS_CH9
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH9 || defined(__DOXYGEN__)
ICUDriver ICUD10;
#endif

/**
 * @brief   ICUD11 driver identifier.
 * @note    The driver ICUD11 allocates the unified channel eMIOS_CH10
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH10 || defined(__DOXYGEN__)
ICUDriver ICUD11;
#endif

/**
 * @brief   ICUD12 driver identifier.
 * @note    The driver ICUD12 allocates the unified channel eMIOS_CH11
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH11 || defined(__DOXYGEN__)
ICUDriver ICUD12;
#endif

/**
 * @brief   ICUD13 driver identifier.
 * @note    The driver ICUD13 allocates the unified channel eMIOS_CH12
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH12 || defined(__DOXYGEN__)
ICUDriver ICUD13;
#endif

/**
 * @brief   ICUD14 driver identifier.
 * @note    The driver ICUD14 allocates the unified channel eMIOS_CH13
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH13 || defined(__DOXYGEN__)
ICUDriver ICUD14;
#endif

/**
 * @brief   ICUD15 driver identifier.
 * @note    The driver ICUD15 allocates the unified channel eMIOS_CH14
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH14 || defined(__DOXYGEN__)
ICUDriver ICUD15;
#endif

/**
 * @brief   ICUD16 driver identifier.
 * @note    The driver ICUD16 allocates the unified channel eMIOS_CH15
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH15 || defined(__DOXYGEN__)
ICUDriver ICUD16;
#endif

/**
 * @brief   ICUD17 driver identifier.
 * @note    The driver ICUD17 allocates the unified channel eMIOS_CH16
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH16 || defined(__DOXYGEN__)
ICUDriver ICUD17;
#endif

/**
 * @brief   ICUD18 driver identifier.
 * @note    The driver ICUD18 allocates the unified channel eMIOS_CH17
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH17 || defined(__DOXYGEN__)
ICUDriver ICUD18;
#endif

/**
 * @brief   ICUD19 driver identifier.
 * @note    The driver ICUD19 allocates the unified channel eMIOS_CH18
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH18 || defined(__DOXYGEN__)
ICUDriver ICUD19;
#endif

/**
 * @brief   ICUD20 driver identifier.
 * @note    The driver ICUD20 allocates the unified channel eMIOS_CH19
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH19 || defined(__DOXYGEN__)
ICUDriver ICUD20;
#endif

/**
 * @brief   ICUD21 driver identifier.
 * @note    The driver ICUD21 allocates the unified channel eMIOS_CH20
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH20 || defined(__DOXYGEN__)
ICUDriver ICUD21;
#endif

/**
 * @brief   ICUD22 driver identifier.
 * @note    The driver ICUD22 allocates the unified channel eMIOS_CH21
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH21 || defined(__DOXYGEN__)
ICUDriver ICUD22;
#endif

/**
 * @brief   ICUD23 driver identifier.
 * @note    The driver ICUD23 allocates the unified channel eMIOS_CH22
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH22 || defined(__DOXYGEN__)
ICUDriver ICUD23;
#endif

/**
 * @brief   ICUD24 driver identifier.
 * @note    The driver ICUD24 allocates the unified channel eMIOS_CH23
 *          when enabled.
 */
#if SPC5_ICU_USE_EMIOS_CH23 || defined(__DOXYGEN__)
ICUDriver ICUD24;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/**
 * @brief   Width and Period registers.
 */
uint32_t width;
uint32_t period;

/**
 * @brief   A2 temp registers.
 */
uint32_t A2_1, A2_2, A2_3;

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief               ICU IRQ handler.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 */
static void icu_lld_serve_interrupt(ICUDriver *icup) {

  uint32_t sr = icup->emiosp->CH[icup->ch_number].CSR.R;

  if ((sr & EMIOSS_OVFL) && (icup->config->overflow_cb != NULL)) {
    icup->emiosp->CH[icup->ch_number].CSR.R |= EMIOSS_OVFLC;
    _icu_isr_invoke_overflow_cb(icup);
  }
  if (sr & EMIOSS_FLAG) {
    icup->emiosp->CH[icup->ch_number].CSR.R |= EMIOSS_FLAGC;
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH) {
      if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 1U) &&           \
          (icup->config->period_cb != NULL)) {
        A2_3 = icup->emiosp->CH[icup->ch_number].CADR.R;
        period = A2_3 - A2_1;
        _icu_isr_invoke_period_cb(icup);
        A2_1 = A2_3;
      } else if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 0) &&     \
          (icup->config->width_cb != NULL)) {
        A2_2 = icup->emiosp->CH[icup->ch_number].CADR.R;
        width = A2_2 - A2_1;
        _icu_isr_invoke_width_cb(icup);
      }
    } else if (icup->config->mode == ICU_INPUT_ACTIVE_LOW) {
      if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 1U) &&           \
          (icup->config->width_cb != NULL)) {
        A2_2 = icup->emiosp->CH[icup->ch_number].CADR.R;
        width = A2_2 - A2_1;
        _icu_isr_invoke_width_cb(icup);
      } else if ((icup->emiosp->CH[icup->ch_number].CSR.B.UCIN == 0) &&     \
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

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_ICU_USE_EMIOS_CH0
#if !defined(SPC5_EMIOS_FLAG_F0_HANDLER)
#error "SPC5_EMIOS_FLAG_F0_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 0 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH0 */

#if SPC5_ICU_USE_EMIOS_CH1
#if !defined(SPC5_EMIOS_FLAG_F1_HANDLER)
#error "SPC5_EMIOS_FLAG_F1_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH1 */

#if SPC5_ICU_USE_EMIOS_CH2
#if !defined(SPC5_EMIOS_FLAG_F2_HANDLER)
#error "SPC5_EMIOS_FLAG_F2_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH2 */

#if SPC5_ICU_USE_EMIOS_CH3
#if !defined(SPC5_EMIOS_FLAG_F3_HANDLER)
#error "SPC5_EMIOS_FLAG_F3_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD4);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH3 */

#if SPC5_ICU_USE_EMIOS_CH4
#if !defined(SPC5_EMIOS_FLAG_F4_HANDLER)
#error "SPC5_EMIOS_FLAG_F4_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 4 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD5);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH4 */

#if SPC5_ICU_USE_EMIOS_CH5
#if !defined(SPC5_EMIOS_FLAG_F5_HANDLER)
#error "SPC5_EMIOS_FLAG_F5_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 5 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD6);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH5 */

#if SPC5_ICU_USE_EMIOS_CH6
#if !defined(SPC5_EMIOS_FLAG_F6_HANDLER)
#error "SPC5_EMIOS_FLAG_F6_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 6 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F6_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD7);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH6 */

#if SPC5_ICU_USE_EMIOS_CH7
#if !defined(SPC5_EMIOS_FLAG_F7_HANDLER)
#error "SPC5_EMIOS_FLAG_F7_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 7 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD8);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH7 */

#if SPC5_ICU_USE_EMIOS_CH8
#if !defined(SPC5_EMIOS_FLAG_F8_HANDLER)
#error "SPC5_EMIOS_FLAG_F8_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 8 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F8_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD9);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH8 */

#if SPC5_ICU_USE_EMIOS_CH9
#if !defined(SPC5_EMIOS_FLAG_F9_HANDLER)
#error "SPC5_EMIOS_FLAG_F9_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 9 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F9_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD10);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH9 */

#if SPC5_ICU_USE_EMIOS_CH10
#if !defined(SPC5_EMIOS_FLAG_F10_HANDLER)
#error "SPC5_EMIOS_FLAG_F10_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 10 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F10_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD11);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH10 */

#if SPC5_ICU_USE_EMIOS_CH11
#if !defined(SPC5_EMIOS_FLAG_F11_HANDLER)
#error "SPC5_EMIOS_FLAG_F11_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 11 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F11_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD12);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH11 */

#if SPC5_ICU_USE_EMIOS_CH12
#if !defined(SPC5_EMIOS_FLAG_F12_HANDLER)
#error "SPC5_EMIOS_FLAG_F12_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 12 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F12_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD13);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH12 */

#if SPC5_ICU_USE_EMIOS_CH13
#if !defined(SPC5_EMIOS_FLAG_F13_HANDLER)
#error "SPC5_EMIOS_FLAG_F13_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 13 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F13_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD14);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH13 */

#if SPC5_ICU_USE_EMIOS_CH14
#if !defined(SPC5_EMIOS_FLAG_F14_HANDLER)
#error "SPC5_EMIOS_FLAG_F14_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 14 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F14_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD15);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH14 */

#if SPC5_ICU_USE_EMIOS_CH15
#if !defined(SPC5_EMIOS_FLAG_F15_HANDLER)
#error "SPC5_EMIOS_FLAG_F15_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 15 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD16);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH15 */

#if SPC5_ICU_USE_EMIOS_CH16
#if !defined(SPC5_EMIOS_FLAG_F16_HANDLER)
#error "SPC5_EMIOS_FLAG_F16_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 16 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F16_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD17);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH16 */

#if SPC5_ICU_USE_EMIOS_CH17
#if !defined(SPC5_EMIOS_FLAG_F17_HANDLER)
#error "SPC5_EMIOS_FLAG_F17_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 17 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F17_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD18);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH17 */

#if SPC5_ICU_USE_EMIOS_CH18
#if !defined(SPC5_EMIOS_FLAG_F18_HANDLER)
#error "SPC5_EMIOS_FLAG_F18_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 18 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F18_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD19);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH18 */

#if SPC5_ICU_USE_EMIOS_CH19
#if !defined(SPC5_EMIOS_FLAG_F19_HANDLER)
#error "SPC5_EMIOS_FLAG_F19_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 19 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F19_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD20);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH19 */

#if SPC5_ICU_USE_EMIOS_CH20
#if !defined(SPC5_EMIOS_FLAG_F20_HANDLER)
#error "SPC5_EMIOS_FLAG_F20_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 20 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F20_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD21);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH20 */

#if SPC5_ICU_USE_EMIOS_CH21
#if !defined(SPC5_EMIOS_FLAG_F21_HANDLER)
#error "SPC5_EMIOS_FLAG_F21_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 21 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F21_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD22);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH21 */

#if SPC5_ICU_USE_EMIOS_CH22
#if !defined(SPC5_EMIOS_FLAG_F22_HANDLER)
#error "SPC5_EMIOS_FLAG_F22_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 22 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F22_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD23);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH22 */

#if SPC5_ICU_USE_EMIOS_CH23
#if !defined(SPC5_EMIOS_FLAG_F23_HANDLER)
#error "SPC5_EMIOS_FLAG_F23_HANDLER not defined"
#endif
/**
 * @brief   eMIOS Channel 23 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F23_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD24);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_ICU_USE_EMIOS_CH23 */

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
  reset_emios_active_channels();

#if SPC5_ICU_USE_EMIOS_CH0
  /* Driver initialization.*/
  icuObjectInit(&ICUD1);
  ICUD1.emiosp = &EMIOS;
  ICUD1.ch_number = 0U;
  ICUD1.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH0 */

#if SPC5_ICU_USE_EMIOS_CH1
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.emiosp = &EMIOS;
  ICUD2.ch_number = 1U;
  ICUD2.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH1 */

#if SPC5_ICU_USE_EMIOS_CH2
  /* Driver initialization.*/
  icuObjectInit(&ICUD3);
  ICUD3.emiosp = &EMIOS;
  ICUD3.ch_number = 2U;
  ICUD3.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH2 */

#if SPC5_ICU_USE_EMIOS_CH3
  /* Driver initialization.*/
  icuObjectInit(&ICUD4);
  ICUD4.emiosp = &EMIOS;
  ICUD4.ch_number = 3U;
  ICUD4.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH3 */

#if SPC5_ICU_USE_EMIOS_CH4
  /* Driver initialization.*/
  icuObjectInit(&ICUD5);
  ICUD5.emiosp = &EMIOS;
  ICUD5.ch_number = 4U;
  ICUD5.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH4 */

#if SPC5_ICU_USE_EMIOS_CH5
  /* Driver initialization.*/
  icuObjectInit(&ICUD6);
  ICUD6.emiosp = &EMIOS;
  ICUD6.ch_number = 5U;
  ICUD6.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH5 */

#if SPC5_ICU_USE_EMIOS_CH6
  /* Driver initialization.*/
  icuObjectInit(&ICUD7);
  ICUD7.emiosp = &EMIOS;
  ICUD7.ch_number = 6U;
  ICUD7.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH6 */

#if SPC5_ICU_USE_EMIOS_CH7
  /* Driver initialization.*/
  icuObjectInit(&ICUD8);
  ICUD8.emiosp = &EMIOS;
  ICUD8.ch_number = 7U;
  ICUD8.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH7 */

#if SPC5_ICU_USE_EMIOS_CH8
  /* Driver initialization.*/
  icuObjectInit(&ICUD9);
  ICUD9.emiosp = &EMIOS;
  ICUD9.ch_number = 8U;
  ICUD9.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH8 */

#if SPC5_ICU_USE_EMIOS_CH9
  /* Driver initialization.*/
  icuObjectInit(&ICUD10);
  ICUD10.emiosp = &EMIOS;
  ICUD10.ch_number = 9U;
  ICUD10.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH9 */

#if SPC5_ICU_USE_EMIOS_CH10
  /* Driver initialization.*/
  icuObjectInit(&ICUD11);
  ICUD11.emiosp = &EMIOS;
  ICUD11.ch_number = 10U;
  ICUD11.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH10 */

#if SPC5_ICU_USE_EMIOS_CH11
  /* Driver initialization.*/
  icuObjectInit(&ICUD12);
  ICUD12.emiosp = &EMIOS;
  ICUD12.ch_number = 11U;
  ICUD12.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH11 */

#if SPC5_ICU_USE_EMIOS_CH12
  /* Driver initialization.*/
  icuObjectInit(&ICUD13);
  ICUD13.emiosp = &EMIOS;
  ICUD13.ch_number = 12U;
  ICUD13.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH12 */

#if SPC5_ICU_USE_EMIOS_CH13
  /* Driver initialization.*/
  icuObjectInit(&ICUD14);
  ICUD14.emiosp = &EMIOS;
  ICUD14.ch_number = 13U;
  ICUD14.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH13 */

#if SPC5_ICU_USE_EMIOS_CH14
  /* Driver initialization.*/
  icuObjectInit(&ICUD15);
  ICUD15.emiosp = &EMIOS;
  ICUD15.ch_number = 14U;
  ICUD15.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH14 */

#if SPC5_ICU_USE_EMIOS_CH15
  /* Driver initialization.*/
  icuObjectInit(&ICUD16);
  ICUD16.emiosp = &EMIOS;
  ICUD16.ch_number = 15U;
  ICUD16.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH15 */

#if SPC5_ICU_USE_EMIOS_CH16
  /* Driver initialization.*/
  icuObjectInit(&ICUD17);
  ICUD17.emiosp = &EMIOS;
  ICUD17.ch_number = 16U;
  ICUD17.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH16 */

#if SPC5_ICU_USE_EMIOS_CH17
  /* Driver initialization.*/
  icuObjectInit(&ICUD18);
  ICUD18.emiosp = &EMIOS;
  ICUD18.ch_number = 17U;
  ICUD18.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH17 */

#if SPC5_ICU_USE_EMIOS_CH18
  /* Driver initialization.*/
  icuObjectInit(&ICUD19);
  ICUD19.emiosp = &EMIOS;
  ICUD19.ch_number = 18U;
  ICUD19.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH18 */

#if SPC5_ICU_USE_EMIOS_CH19
  /* Driver initialization.*/
  icuObjectInit(&ICUD20);
  ICUD20.emiosp = &EMIOS;
  ICUD20.ch_number = 19U;
  ICUD20.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH19 */

#if SPC5_ICU_USE_EMIOS_CH20
  /* Driver initialization.*/
  icuObjectInit(&ICUD21);
  ICUD21.emiosp = &EMIOS;
  ICUD21.ch_number = 20U;
  ICUD21.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH20 */

#if SPC5_ICU_USE_EMIOS_CH21
  /* Driver initialization.*/
  icuObjectInit(&ICUD22);
  ICUD22.emiosp = &EMIOS;
  ICUD22.ch_number = 21U;
  ICUD22.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH21 */

#if SPC5_ICU_USE_EMIOS_CH22
  /* Driver initialization.*/
  icuObjectInit(&ICUD23);
  ICUD23.emiosp = &EMIOS;
  ICUD23.ch_number = 22U;
  ICUD23.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH22 */

#if SPC5_ICU_USE_EMIOS_CH23
  /* Driver initialization.*/
  icuObjectInit(&ICUD24);
  ICUD24.emiosp = &EMIOS;
  ICUD24.ch_number = 23U;
  ICUD24.clock = SPC5_EMIOS_CLK;
#endif /* SPC5_ICU_USE_EMIOS_CH23 */

#if SPC5_ICU_USE_EMIOS

#if SPC5_ICU_USE_EMIOS_CH0
  INTC.PSR[SPC5_EMIOS_FLAG_F0_NUMBER].R = SPC5_EMIOS_FLAG_F0_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH0 */

#if SPC5_ICU_USE_EMIOS_CH1
  INTC.PSR[SPC5_EMIOS_FLAG_F1_NUMBER].R = SPC5_EMIOS_FLAG_F1_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH1 */

#if SPC5_ICU_USE_EMIOS_CH2
  INTC.PSR[SPC5_EMIOS_FLAG_F2_NUMBER].R = SPC5_EMIOS_FLAG_F2_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH2 */

#if SPC5_ICU_USE_EMIOS_CH3
  INTC.PSR[SPC5_EMIOS_FLAG_F3_NUMBER].R = SPC5_EMIOS_FLAG_F3_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH3 */

#if SPC5_ICU_USE_EMIOS_CH4
  INTC.PSR[SPC5_EMIOS_FLAG_F4_NUMBER].R = SPC5_EMIOS_FLAG_F4_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH4 */

#if SPC5_ICU_USE_EMIOS_CH5
  INTC.PSR[SPC5_EMIOS_FLAG_F5_NUMBER].R = SPC5_EMIOS_FLAG_F5_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH5 */

#if SPC5_ICU_USE_EMIOS_CH6
  INTC.PSR[SPC5_EMIOS_FLAG_F6_NUMBER].R = SPC5_EMIOS_FLAG_F6_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH6 */

#if SPC5_ICU_USE_EMIOS_CH7
  INTC.PSR[SPC5_EMIOS_FLAG_F7_NUMBER].R = SPC5_EMIOS_FLAG_F7_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH7 */

#if SPC5_ICU_USE_EMIOS_CH8
  INTC.PSR[SPC5_EMIOS_FLAG_F8_NUMBER].R = SPC5_EMIOS_FLAG_F8_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH8 */

#if SPC5_ICU_USE_EMIOS_CH9
  INTC.PSR[SPC5_EMIOS_FLAG_F9_NUMBER].R = SPC5_EMIOS_FLAG_F9_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH9 */

#if SPC5_ICU_USE_EMIOS_CH10
  INTC.PSR[SPC5_EMIOS_FLAG_F10_NUMBER].R = SPC5_EMIOS_FLAG_F10_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH10 */

#if SPC5_ICU_USE_EMIOS_CH11
  INTC.PSR[SPC5_EMIOS_FLAG_F11_NUMBER].R = SPC5_EMIOS_FLAG_F11_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH11 */

#if SPC5_ICU_USE_EMIOS_CH12
  INTC.PSR[SPC5_EMIOS_FLAG_F12_NUMBER].R = SPC5_EMIOS_FLAG_F12_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH12 */

#if SPC5_ICU_USE_EMIOS_CH13
  INTC.PSR[SPC5_EMIOS_FLAG_F13_NUMBER].R = SPC5_EMIOS_FLAG_F13_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH13 */

#if SPC5_ICU_USE_EMIOS_CH14
  INTC.PSR[SPC5_EMIOS_FLAG_F14_NUMBER].R = SPC5_EMIOS_FLAG_F14_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH14 */

#if SPC5_ICU_USE_EMIOS_CH15
  INTC.PSR[SPC5_EMIOS_FLAG_F15_NUMBER].R = SPC5_EMIOS_FLAG_F15_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH15 */

#if SPC5_ICU_USE_EMIOS_CH16
  INTC.PSR[SPC5_EMIOS_FLAG_F16_NUMBER].R = SPC5_EMIOS_FLAG_F16_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH16 */

#if SPC5_ICU_USE_EMIOS_CH17
  INTC.PSR[SPC5_EMIOS_FLAG_F17_NUMBER].R = SPC5_EMIOS_FLAG_F17_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH17 */

#if SPC5_ICU_USE_EMIOS_CH18
  INTC.PSR[SPC5_EMIOS_FLAG_F18_NUMBER].R = SPC5_EMIOS_FLAG_F18_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH18 */

#if SPC5_ICU_USE_EMIOS_CH19
  INTC.PSR[SPC5_EMIOS_FLAG_F19_NUMBER].R = SPC5_EMIOS_FLAG_F19_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH19 */

#if SPC5_ICU_USE_EMIOS_CH20
  INTC.PSR[SPC5_EMIOS_FLAG_F20_NUMBER].R = SPC5_EMIOS_FLAG_F20_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH20 */

#if SPC5_ICU_USE_EMIOS_CH21
  INTC.PSR[SPC5_EMIOS_FLAG_F21_NUMBER].R = SPC5_EMIOS_FLAG_F21_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH21 */

#if SPC5_ICU_USE_EMIOS_CH22
  INTC.PSR[SPC5_EMIOS_FLAG_F22_NUMBER].R = SPC5_EMIOS_FLAG_F22_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH22 */

#if SPC5_ICU_USE_EMIOS_CH23
  INTC.PSR[SPC5_EMIOS_FLAG_F23_NUMBER].R = SPC5_EMIOS_FLAG_F23_PRIORITY;
#endif /* SPC5_ICU_USE_EMIOS_CH23 */

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

  osalDbgAssert(get_emios_active_channels() < SPC5_EMIOS_NUM_CHANNELS,
              "too many channels");

  if (icup->state == ICU_STOP) {
    /* Enables the peripheral.*/
#if SPC5_ICU_USE_EMIOS_CH0
    if (&ICUD1 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH0 */

#if SPC5_ICU_USE_EMIOS_CH1
    if (&ICUD2 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH1 */

#if SPC5_ICU_USE_EMIOS_CH2
    if (&ICUD3 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH2 */

#if SPC5_ICU_USE_EMIOS_CH3
    if (&ICUD4 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH3 */

#if SPC5_ICU_USE_EMIOS_CH4
    if (&ICUD5 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH4 */

#if SPC5_ICU_USE_EMIOS_CH5
    if (&ICUD6 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH5 */

#if SPC5_ICU_USE_EMIOS_CH6
    if (&ICUD7 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH6 */

#if SPC5_ICU_USE_EMIOS_CH7
    if (&ICUD8 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH7 */

#if SPC5_ICU_USE_EMIOS_CH8
    if (&ICUD9 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH8 */

#if SPC5_ICU_USE_EMIOS_CH9
    if (&ICUD10 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH9 */

#if SPC5_ICU_USE_EMIOS_CH10
    if (&ICUD11 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH10 */

#if SPC5_ICU_USE_EMIOS_CH11
    if (&ICUD12 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH11 */

#if SPC5_ICU_USE_EMIOS_CH12
    if (&ICUD13 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH12 */

#if SPC5_ICU_USE_EMIOS_CH13
    if (&ICUD14 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH13 */

#if SPC5_ICU_USE_EMIOS_CH14
    if (&ICUD15 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH14 */

#if SPC5_ICU_USE_EMIOS_CH15
    if (&ICUD16 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH15 */

#if SPC5_ICU_USE_EMIOS_CH16
    if (&ICUD17 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH16 */

#if SPC5_ICU_USE_EMIOS_CH17
    if (&ICUD18 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH17 */

#if SPC5_ICU_USE_EMIOS_CH18
    if (&ICUD19 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH18 */

#if SPC5_ICU_USE_EMIOS_CH19
    if (&ICUD20 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH19 */

#if SPC5_ICU_USE_EMIOS_CH20
    if (&ICUD21 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH20 */

#if SPC5_ICU_USE_EMIOS_CH21
    if (&ICUD22 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH21 */

#if SPC5_ICU_USE_EMIOS_CH22
    if (&ICUD23 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH22 */

#if SPC5_ICU_USE_EMIOS_CH23
    if (&ICUD24 == icup)
      increase_emios_active_channels();
#endif /* SPC5_ICU_USE_EMIOS_CH23 */

    /* Set eMIOS Clock.*/
#if SPC5_ICU_USE_EMIOS
    icu_active_emios_clock(icup);
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
  osalDbgAssert((psc <= 4) &&
              ((psc * icup->config->frequency) == icup->clock) &&
              ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
              "invalid frequency");

  icup->emiosp->CH[icup->ch_number].CCR.B.UCPREN = 0;
  icup->emiosp->CH[icup->ch_number].CCR.R |=
      EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER) |
      EMIOSC_EDSEL | EMIOS_CCR_MODE_SAIC;
  icup->emiosp->CH[icup->ch_number].CCR.B.UCPRE = psc - 1;
  icup->emiosp->CH[icup->ch_number].CCR.R |= EMIOSC_UCPREN;

  /* Set source polarity.*/
  if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH) {
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

  osalDbgAssert(get_emios_active_channels() < SPC5_EMIOS_NUM_CHANNELS,
                "too many channels");

  if (icup->state == ICU_READY) {

    /* Disables the peripheral.*/
#if SPC5_ICU_USE_EMIOS_CH0
    if (&ICUD1 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH0 */

#if SPC5_ICU_USE_EMIOS_CH1
    if (&ICUD2 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH1 */

#if SPC5_ICU_USE_EMIOS_CH2
    if (&ICUD3 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH2 */

#if SPC5_ICU_USE_EMIOS_CH3
    if (&ICUD4 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH3 */

#if SPC5_ICU_USE_EMIOS_CH4
    if (&ICUD5 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH4 */

#if SPC5_ICU_USE_EMIOS_CH5
    if (&ICUD6 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH5 */

#if SPC5_ICU_USE_EMIOS_CH6
    if (&ICUD7 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH6 */

#if SPC5_ICU_USE_EMIOS_CH7
    if (&ICUD8 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH7 */

#if SPC5_ICU_USE_EMIOS_CH8
    if (&ICUD9 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH8 */

#if SPC5_ICU_USE_EMIOS_CH9
    if (&ICUD10 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH9 */

#if SPC5_ICU_USE_EMIOS_CH10
    if (&ICUD11 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH10 */

#if SPC5_ICU_USE_EMIOS_CH11
    if (&ICUD12 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH11 */

#if SPC5_ICU_USE_EMIOS_CH12
    if (&ICUD13 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH12 */

#if SPC5_ICU_USE_EMIOS_CH13
    if (&ICUD14 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH13 */

#if SPC5_ICU_USE_EMIOS_CH14
    if (&ICUD15 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH14 */

#if SPC5_ICU_USE_EMIOS_CH15
    if (&ICUD16 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH15 */

#if SPC5_ICU_USE_EMIOS_CH16
    if (&ICUD17 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH16 */

#if SPC5_ICU_USE_EMIOS_CH17
    if (&ICUD18 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH17 */

#if SPC5_ICU_USE_EMIOS_CH18
    if (&ICUD19 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH18 */

#if SPC5_ICU_USE_EMIOS_CH19
    if (&ICUD20 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH19 */

#if SPC5_ICU_USE_EMIOS_CH20
    if (&ICUD21 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH20 */

#if SPC5_ICU_USE_EMIOS_CH21
    if (&ICUD22 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH21 */

#if SPC5_ICU_USE_EMIOS_CH22
    if (&ICUD23 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH22 */

#if SPC5_ICU_USE_EMIOS_CH23
    if (&ICUD24 == icup) {
      /* Reset UC Control Register.*/
      icup->emiosp->CH[icup->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_ICU_USE_EMIOS_CH23 */

    /* eMIOS clock deactivation.*/
#if SPC5_ICU_USE_EMIOS
    deactive_emios_clock();
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

  /* Clear pending IRQs (if any).*/
  icup->emiosp->CH[icup->ch_number].CSR.R = EMIOSS_OVRC |
      EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Active interrupts.*/
  if (icup->config->period_cb != NULL || icup->config->width_cb != NULL ||  \
      icup->config->overflow_cb != NULL) {
    icup->emiosp->CH[icup->ch_number].CCR.B.FEN = 1U;
  }

  /* Channel enables.*/
  icup->emiosp->UCDIS.R &= ~(1 << icup->ch_number);
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
