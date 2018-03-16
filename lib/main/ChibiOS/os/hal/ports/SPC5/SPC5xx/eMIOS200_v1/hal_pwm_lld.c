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
 * @file    SPC5xx/eMIOS200_v1/hal_pwm_lld.c
 * @brief   SPC5xx low level pwm driver code.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

#include "spc5_emios.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 allocates the unified channel EMIOS_CH0
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the unified channel EMIOS_CH1
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the unified channel EMIOS_CH2
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH2 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/**
 * @brief   PWMD4 driver identifier.
 * @note    The driver PWMD4 allocates the unified channel EMIOS_CH3
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH3 || defined(__DOXYGEN__)
PWMDriver PWMD4;
#endif

/**
 * @brief   PWMD5 driver identifier.
 * @note    The driver PWMD5 allocates the unified channel EMIOS_CH4
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH4 || defined(__DOXYGEN__)
PWMDriver PWMD5;
#endif

/**
 * @brief   PWMD6 driver identifier.
 * @note    The driver PWMD6 allocates the unified channel EMIOS_CH5
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH5 || defined(__DOXYGEN__)
PWMDriver PWMD6;
#endif

/**
 * @brief   PWMD7 driver identifier.
 * @note    The driver PWMD7 allocates the unified channel EMIOS_CH6
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH6 || defined(__DOXYGEN__)
PWMDriver PWMD7;
#endif

/**
 * @brief   PWMD8 driver identifier.
 * @note    The driver PWMD8 allocates the unified channel EMIOS_CH7
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH7 || defined(__DOXYGEN__)
PWMDriver PWMD8;
#endif

/**
 * @brief   PWMD9 driver identifier.
 * @note    The driver PWMD9 allocates the unified channel EMIOS_CH8
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH8 || defined(__DOXYGEN__)
PWMDriver PWMD9;
#endif

/**
 * @brief   PWMD10 driver identifier.
 * @note    The driver PWMD10 allocates the unified channel EMIOS_CH9
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH9 || defined(__DOXYGEN__)
PWMDriver PWMD10;
#endif

/**
 * @brief   PWMD11 driver identifier.
 * @note    The driver PWMD11 allocates the unified channel EMIOS_CH10
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH10 || defined(__DOXYGEN__)
PWMDriver PWMD11;
#endif

/**
 * @brief   PWMD12 driver identifier.
 * @note    The driver PWMD12 allocates the unified channel EMIOS_CH11
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH11 || defined(__DOXYGEN__)
PWMDriver PWMD12;
#endif

/**
 * @brief   PWMD13 driver identifier.
 * @note    The driver PWMD13 allocates the unified channel EMIOS_CH12
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH12 || defined(__DOXYGEN__)
PWMDriver PWMD13;
#endif

/**
 * @brief   PWMD14 driver identifier.
 * @note    The driver PWMD14 allocates the unified channel EMIOS_CH13
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH13 || defined(__DOXYGEN__)
PWMDriver PWMD14;
#endif

/**
 * @brief   PWMD15 driver identifier.
 * @note    The driver PWMD15 allocates the unified channel EMIOS_CH14
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH14 || defined(__DOXYGEN__)
PWMDriver PWMD15;
#endif

/**
 * @brief   PWMD16 driver identifier.
 * @note    The driver PWMD16 allocates the unified channel EMIOS_CH15
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH15 || defined(__DOXYGEN__)
PWMDriver PWMD16;
#endif

/**
 * @brief   PWMD17 driver identifier.
 * @note    The driver PWMD17 allocates the unified channel EMIOS_CH16
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH16 || defined(__DOXYGEN__)
PWMDriver PWMD17;
#endif

/**
 * @brief   PWMD18 driver identifier.
 * @note    The driver PWMD18 allocates the unified channel EMIOS_CH17
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH17 || defined(__DOXYGEN__)
PWMDriver PWMD18;
#endif

/**
 * @brief   PWMD19 driver identifier.
 * @note    The driver PWMD19 allocates the unified channel EMIOS_CH18
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH18 || defined(__DOXYGEN__)
PWMDriver PWMD19;
#endif

/**
 * @brief   PWMD20 driver identifier.
 * @note    The driver PWMD20 allocates the unified channel EMIOS_CH19
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH19 || defined(__DOXYGEN__)
PWMDriver PWMD20;
#endif

/**
 * @brief   PWMD21 driver identifier.
 * @note    The driver PWMD21 allocates the unified channel EMIOS_CH20
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH20 || defined(__DOXYGEN__)
PWMDriver PWMD21;
#endif

/**
 * @brief   PWMD22 driver identifier.
 * @note    The driver PWMD22 allocates the unified channel EMIOS_CH21
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH21 || defined(__DOXYGEN__)
PWMDriver PWMD22;
#endif

/**
 * @brief   PWMD23 driver identifier.
 * @note    The driver PWMD23 allocates the unified channel EMIOS_CH22
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH22 || defined(__DOXYGEN__)
PWMDriver PWMD23;
#endif

/**
 * @brief   PWMD24 driver identifier.
 * @note    The driver PWMD24 allocates the unified channel EMIOS_CH23
 *          when enabled.
 */
#if SPC5_PWM_USE_EMIOS_CH23 || defined(__DOXYGEN__)
PWMDriver PWMD24;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief               PWM IRQ handler.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 */
static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {

  uint32_t sr = pwmp->emiosp->CH[pwmp->ch_number].CSR.R;

  if (sr & EMIOSS_OVFL) {
    pwmp->emiosp->CH[pwmp->ch_number].CSR.R |= EMIOSS_OVFLC;
  }
  if (sr & EMIOSS_OVR) {
    pwmp->emiosp->CH[pwmp->ch_number].CSR.R |= EMIOSS_OVRC;
  }
  if (sr & EMIOSS_FLAG) {
    pwmp->emiosp->CH[pwmp->ch_number].CSR.R |= EMIOSS_FLAGC;

    if (pwmp->config->channels[0].mode == PWM_OUTPUT_ACTIVE_HIGH) {
      if ((pwmp->emiosp->CH[pwmp->ch_number].CSR.B.UCOUT == 1U) &&          \
          (pwmp->config->callback != NULL)) {
        pwmp->config->callback(pwmp);
      } else if ((pwmp->emiosp->CH[pwmp->ch_number].CSR.B.UCOUT == 0) &&    \
          (pwmp->config->channels[0].callback != NULL)) {
        pwmp->config->channels[0].callback(pwmp);
      }
    } else if (pwmp->config->channels[0].mode == PWM_OUTPUT_ACTIVE_LOW) {
      if ((pwmp->emiosp->CH[pwmp->ch_number].CSR.B.UCOUT == 0) &&           \
          (pwmp->config->callback != NULL)) {
        pwmp->config->callback(pwmp);
      } else if ((pwmp->emiosp->CH[pwmp->ch_number].CSR.B.UCOUT == 1U) &&   \
          (pwmp->config->channels[0].callback != NULL)) {
        pwmp->config->channels[0].callback(pwmp);
      }
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_PWM_USE_EMIOS_CH0
#if !defined(SPC5_EMIOS_FLAG_F0_HANDLER)
#error "SPC5_EMIOS_FLAG_F0_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 0 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH0 */

#if SPC5_PWM_USE_EMIOS_CH1
#if !defined(SPC5_EMIOS_FLAG_F1_HANDLER)
#error "SPC5_EMIOS_FLAG_F1_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH1 */

#if SPC5_PWM_USE_EMIOS_CH2
#if !defined(SPC5_EMIOS_FLAG_F2_HANDLER)
#error "SPC5_EMIOS_FLAG_F2_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH2 */

#if SPC5_PWM_USE_EMIOS_CH3
#if !defined(SPC5_EMIOS_FLAG_F3_HANDLER)
#error "SPC5_EMIOS_FLAG_F3_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD4);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH3 */

#if SPC5_PWM_USE_EMIOS_CH4
#if !defined(SPC5_EMIOS_FLAG_F4_HANDLER)
#error "SPC5_EMIOS_FLAG_F4_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 4 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F4_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD5);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH4 */

#if SPC5_PWM_USE_EMIOS_CH5
#if !defined(SPC5_EMIOS_FLAG_F5_HANDLER)
#error "SPC5_EMIOS_FLAG_F5_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 5 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD6);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH5 */

#if SPC5_PWM_USE_EMIOS_CH6
#if !defined(SPC5_EMIOS_FLAG_F6_HANDLER)
#error "SPC5_EMIOS_FLAG_F6_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 6 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F6_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD7);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH6 */

#if SPC5_PWM_USE_EMIOS_CH7
#if !defined(SPC5_EMIOS_FLAG_F7_HANDLER)
#error "SPC5_EMIOS_FLAG_F7_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 7 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD8);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH7 */

#if SPC5_PWM_USE_EMIOS_CH8
#if !defined(SPC5_EMIOS_FLAG_F8_HANDLER)
#error "SPC5_EMIOS_FLAG_F8_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 8 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F8_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD9);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH8 */

#if SPC5_PWM_USE_EMIOS_CH9
#if !defined(SPC5_EMIOS_FLAG_F9_HANDLER)
#error "SPC5_EMIOS_FLAG_F9_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 9 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F9_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD10);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH9 */

#if SPC5_PWM_USE_EMIOS_CH10
#if !defined(SPC5_EMIOS_FLAG_F10_HANDLER)
#error "SPC5_EMIOS_FLAG_F10_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 10 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F10_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD11);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH10 */

#if SPC5_PWM_USE_EMIOS_CH11
#if !defined(SPC5_EMIOS_FLAG_F11_HANDLER)
#error "SPC5_EMIOS_FLAG_F11_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 11 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F11_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD12);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH11 */

#if SPC5_PWM_USE_EMIOS_CH12
#if !defined(SPC5_EMIOS_FLAG_F12_HANDLER)
#error "SPC5_EMIOS_FLAG_F12_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 12 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F12_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD13);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH12 */

#if SPC5_PWM_USE_EMIOS_CH13
#if !defined(SPC5_EMIOS_FLAG_F13_HANDLER)
#error "SPC5_EMIOS_FLAG_F13_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 13 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F13_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD14);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH13 */

#if SPC5_PWM_USE_EMIOS_CH14
#if !defined(SPC5_EMIOS_FLAG_F14_HANDLER)
#error "SPC5_EMIOS_FLAG_F14_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 14 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F14_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD15);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH14 */

#if SPC5_PWM_USE_EMIOS_CH15
#if !defined(SPC5_EMIOS_FLAG_F15_HANDLER)
#error "SPC5_EMIOS_FLAG_F15_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 15 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD16);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH15 */

#if SPC5_PWM_USE_EMIOS_CH16
#if !defined(SPC5_EMIOS_FLAG_F16_HANDLER)
#error "SPC5_EMIOS_FLAG_F16_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 16 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F16_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD17);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH16 */

#if SPC5_PWM_USE_EMIOS_CH17
#if !defined(SPC5_EMIOS_FLAG_F17_HANDLER)
#error "SPC5_EMIOS_FLAG_F17_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 17 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F17_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD18);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH17 */

#if SPC5_PWM_USE_EMIOS_CH18
#if !defined(SPC5_EMIOS_FLAG_F18_HANDLER)
#error "SPC5_EMIOS_FLAG_F18_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 18 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F18_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD19);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH18 */

#if SPC5_PWM_USE_EMIOS_CH19
#if !defined(SPC5_EMIOS_FLAG_F19_HANDLER)
#error "SPC5_EMIOS_FLAG_F19_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 19 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F19_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD20);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH19 */

#if SPC5_PWM_USE_EMIOS_CH20
#if !defined(SPC5_EMIOS_FLAG_F20_HANDLER)
#error "SPC5_EMIOS_FLAG_F20_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 20 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F20_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD21);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH20 */

#if SPC5_PWM_USE_EMIOS_CH21
#if !defined(SPC5_EMIOS_FLAG_F21_HANDLER)
#error "SPC5_EMIOS_FLAG_F21_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 21 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F21_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD22);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH21 */

#if SPC5_PWM_USE_EMIOS_CH22
#if !defined(SPC5_EMIOS_FLAG_F22_HANDLER)
#error "SPC5_EMIOS_FLAG_F22_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 22 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F22_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD23);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH22 */

#if SPC5_PWM_USE_EMIOS_CH23
#if !defined(SPC5_EMIOS_FLAG_F23_HANDLER)
#error "SPC5_EMIOS_FLAG_F23_HANDLER not defined"
#endif
/**
 * @brief   EMIOS Channel 23 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS_FLAG_F23_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_interrupt(&PWMD24);

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS_CH23 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {
  /* eMIOSx channels initially all not in use.*/
  reset_emios_active_channels();

#if SPC5_PWM_USE_EMIOS_CH0
  /* Driver initialization.*/
  pwmObjectInit(&PWMD1);
  PWMD1.emiosp = &EMIOS;
  PWMD1.ch_number = 0U;
#endif /* SPC5_PWM_USE_EMIOS_CH0 */

#if SPC5_PWM_USE_EMIOS_CH1
  /* Driver initialization.*/
  pwmObjectInit(&PWMD2);
  PWMD2.emiosp = &EMIOS;
  PWMD2.ch_number = 1U;
#endif /* SPC5_PWM_USE_EMIOS_CH1 */

#if SPC5_PWM_USE_EMIOS_CH2
  /* Driver initialization.*/
  pwmObjectInit(&PWMD3);
  PWMD3.emiosp = &EMIOS;
  PWMD3.ch_number = 2U;
#endif /* SPC5_PWM_USE_EMIOS_CH2 */

#if SPC5_PWM_USE_EMIOS_CH3
  /* Driver initialization.*/
  pwmObjectInit(&PWMD4);
  PWMD4.emiosp = &EMIOS;
  PWMD4.ch_number = 3U;
#endif /* SPC5_PWM_USE_EMIOS_CH3 */

#if SPC5_PWM_USE_EMIOS_CH4
  /* Driver initialization.*/
  pwmObjectInit(&PWMD5);
  PWMD5.emiosp = &EMIOS;
  PWMD5.ch_number = 4U;
#endif /* SPC5_PWM_USE_EMIOS_CH4 */

#if SPC5_PWM_USE_EMIOS_CH5
  /* Driver initialization.*/
  pwmObjectInit(&PWMD6);
  PWMD6.emiosp = &EMIOS;
  PWMD6.ch_number = 5U;
#endif /* SPC5_PWM_USE_EMIOS_CH5 */

#if SPC5_PWM_USE_EMIOS_CH6
  /* Driver initialization.*/
  pwmObjectInit(&PWMD7);
  PWMD7.emiosp = &EMIOS;
  PWMD7.ch_number = 6U;
#endif /* SPC5_PWM_USE_EMIOS_CH6 */

#if SPC5_PWM_USE_EMIOS_CH7
  /* Driver initialization.*/
  pwmObjectInit(&PWMD8);
  PWMD8.emiosp = &EMIOS;
  PWMD8.ch_number = 7U;
#endif /* SPC5_PWM_USE_EMIOS_CH7 */

#if SPC5_PWM_USE_EMIOS_CH8
  /* Driver initialization.*/
  pwmObjectInit(&PWMD9);
  PWMD9.emiosp = &EMIOS;
  PWMD9.ch_number = 8U;
#endif /* SPC5_PWM_USE_EMIOS_CH8 */

#if SPC5_PWM_USE_EMIOS_CH9
  /* Driver initialization.*/
  pwmObjectInit(&PWMD10);
  PWMD10.emiosp = &EMIOS;
  PWMD10.ch_number = 9U;
#endif /* SPC5_PWM_USE_EMIOS_CH9 */

#if SPC5_PWM_USE_EMIOS_CH10
  /* Driver initialization.*/
  pwmObjectInit(&PWMD11);
  PWMD11.emiosp = &EMIOS;
  PWMD11.ch_number = 10U;
#endif /* SPC5_PWM_USE_EMIOS_CH10 */

#if SPC5_PWM_USE_EMIOS_CH11
  /* Driver initialization.*/
  pwmObjectInit(&PWMD12);
  PWMD12.emiosp = &EMIOS;
  PWMD12.ch_number = 11U;
#endif /* SPC5_PWM_USE_EMIOS_CH11 */

#if SPC5_PWM_USE_EMIOS_CH12
  /* Driver initialization.*/
  pwmObjectInit(&PWMD13);
  PWMD13.emiosp = &EMIOS;
  PWMD13.ch_number = 12U;
#endif /* SPC5_PWM_USE_EMIOS_CH12 */

#if SPC5_PWM_USE_EMIOS_CH13
  /* Driver initialization.*/
  pwmObjectInit(&PWMD14);
  PWMD14.emiosp = &EMIOS;
  PWMD14.ch_number = 13U;
#endif /* SPC5_PWM_USE_EMIOS_CH13 */

#if SPC5_PWM_USE_EMIOS_CH14
  /* Driver initialization.*/
  pwmObjectInit(&PWMD15);
  PWMD15.emiosp = &EMIOS;
  PWMD15.ch_number = 14U;
#endif /* SPC5_PWM_USE_EMIOS_CH14 */

#if SPC5_PWM_USE_EMIOS_CH15
  /* Driver initialization.*/
  pwmObjectInit(&PWMD16);
  PWMD16.emiosp = &EMIOS;
  PWMD16.ch_number = 15U;
#endif /* SPC5_PWM_USE_EMIOS_CH15 */

#if SPC5_PWM_USE_EMIOS_CH16
  /* Driver initialization.*/
  pwmObjectInit(&PWMD17);
  PWMD17.emiosp = &EMIOS;
  PWMD17.ch_number = 16U;
#endif /* SPC5_PWM_USE_EMIOS_CH16 */

#if SPC5_PWM_USE_EMIOS_CH17
  /* Driver initialization.*/
  pwmObjectInit(&PWMD18);
  PWMD18.emiosp = &EMIOS;
  PWMD18.ch_number = 17U;
#endif /* SPC5_PWM_USE_EMIOS_CH17 */

#if SPC5_PWM_USE_EMIOS_CH18
  /* Driver initialization.*/
  pwmObjectInit(&PWMD19);
  PWMD19.emiosp = &EMIOS;
  PWMD19.ch_number = 18U;
#endif /* SPC5_PWM_USE_EMIOS_CH18 */

#if SPC5_PWM_USE_EMIOS_CH19
  /* Driver initialization.*/
  pwmObjectInit(&PWMD20);
  PWMD20.emiosp = &EMIOS;
  PWMD20.ch_number = 19U;
#endif /* SPC5_PWM_USE_EMIOS_CH19 */

#if SPC5_PWM_USE_EMIOS_CH20
  /* Driver initialization.*/
  pwmObjectInit(&PWMD21);
  PWMD21.emiosp = &EMIOS;
  PWMD21.ch_number = 20U;
#endif /* SPC5_PWM_USE_EMIOS_CH20 */

#if SPC5_PWM_USE_EMIOS_CH21
  /* Driver initialization.*/
  pwmObjectInit(&PWMD22);
  PWMD22.emiosp = &EMIOS;
  PWMD22.ch_number = 21U;
#endif /* SPC5_PWM_USE_EMIOS_CH21 */

#if SPC5_PWM_USE_EMIOS_CH22
  /* Driver initialization.*/
  pwmObjectInit(&PWMD23);
  PWMD23.emiosp = &EMIOS;
  PWMD23.ch_number = 22U;
#endif /* SPC5_PWM_USE_EMIOS_CH22 */

#if SPC5_PWM_USE_EMIOS_CH23
  /* Driver initialization.*/
  pwmObjectInit(&PWMD24);
  PWMD24.emiosp = &EMIOS;
  PWMD24.ch_number = 23U;
#endif /* SPC5_PWM_USE_EMIOS_CH23 */

#if SPC5_PWM_USE_EMIOS

#if SPC5_PWM_USE_EMIOS_CH0
  INTC.PSR[SPC5_EMIOS_FLAG_F0_NUMBER].R = SPC5_EMIOS_FLAG_F0_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH0 */

#if SPC5_PWM_USE_EMIOS_CH1
  INTC.PSR[SPC5_EMIOS_FLAG_F1_NUMBER].R = SPC5_EMIOS_FLAG_F1_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH1 */

#if SPC5_PWM_USE_EMIOS_CH2
  INTC.PSR[SPC5_EMIOS_FLAG_F2_NUMBER].R = SPC5_EMIOS_FLAG_F2_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH2 */

#if SPC5_PWM_USE_EMIOS_CH3
  INTC.PSR[SPC5_EMIOS_FLAG_F3_NUMBER].R = SPC5_EMIOS_FLAG_F3_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH3 */

#if SPC5_PWM_USE_EMIOS_CH4
  INTC.PSR[SPC5_EMIOS_FLAG_F4_NUMBER].R = SPC5_EMIOS_FLAG_F4_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH4 */

#if SPC5_PWM_USE_EMIOS_CH5
  INTC.PSR[SPC5_EMIOS_FLAG_F5_NUMBER].R = SPC5_EMIOS_FLAG_F5_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH5 */

#if SPC5_PWM_USE_EMIOS_CH6
  INTC.PSR[SPC5_EMIOS_FLAG_F6_NUMBER].R = SPC5_EMIOS_FLAG_F6_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH6 */

#if SPC5_PWM_USE_EMIOS_CH7
  INTC.PSR[SPC5_EMIOS_FLAG_F7_NUMBER].R = SPC5_EMIOS_FLAG_F7_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH7 */

#if SPC5_PWM_USE_EMIOS_CH8
  INTC.PSR[SPC5_EMIOS_FLAG_F8_NUMBER].R = SPC5_EMIOS_FLAG_F8_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH8 */

#if SPC5_PWM_USE_EMIOS_CH9
  INTC.PSR[SPC5_EMIOS_FLAG_F9_NUMBER].R = SPC5_EMIOS_FLAG_F9_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH9 */

#if SPC5_PWM_USE_EMIOS_CH10
  INTC.PSR[SPC5_EMIOS_FLAG_F10_NUMBER].R = SPC5_EMIOS_FLAG_F10_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH10 */

#if SPC5_PWM_USE_EMIOS_CH11
  INTC.PSR[SPC5_EMIOS_FLAG_F11_NUMBER].R = SPC5_EMIOS_FLAG_F11_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH11 */

#if SPC5_PWM_USE_EMIOS_CH12
  INTC.PSR[SPC5_EMIOS_FLAG_F12_NUMBER].R = SPC5_EMIOS_FLAG_F12_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH12 */

#if SPC5_PWM_USE_EMIOS_CH13
  INTC.PSR[SPC5_EMIOS_FLAG_F13_NUMBER].R = SPC5_EMIOS_FLAG_F13_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH13 */

#if SPC5_PWM_USE_EMIOS_CH14
  INTC.PSR[SPC5_EMIOS_FLAG_F14_NUMBER].R = SPC5_EMIOS_FLAG_F14_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH14 */

#if SPC5_PWM_USE_EMIOS_CH15
  INTC.PSR[SPC5_EMIOS_FLAG_F15_NUMBER].R = SPC5_EMIOS_FLAG_F15_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH15 */

#if SPC5_PWM_USE_EMIOS_CH16
  INTC.PSR[SPC5_EMIOS_FLAG_F16_NUMBER].R = SPC5_EMIOS_FLAG_F16_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH16 */

#if SPC5_PWM_USE_EMIOS_CH17
  INTC.PSR[SPC5_EMIOS_FLAG_F17_NUMBER].R = SPC5_EMIOS_FLAG_F17_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH17 */

#if SPC5_PWM_USE_EMIOS_CH18
  INTC.PSR[SPC5_EMIOS_FLAG_F18_NUMBER].R = SPC5_EMIOS_FLAG_F18_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH18 */

#if SPC5_PWM_USE_EMIOS_CH19
  INTC.PSR[SPC5_EMIOS_FLAG_F19_NUMBER].R = SPC5_EMIOS_FLAG_F19_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH19 */

#if SPC5_PWM_USE_EMIOS_CH20
  INTC.PSR[SPC5_EMIOS_FLAG_F20_NUMBER].R = SPC5_EMIOS_FLAG_F20_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH20 */

#if SPC5_PWM_USE_EMIOS_CH21
  INTC.PSR[SPC5_EMIOS_FLAG_F21_NUMBER].R = SPC5_EMIOS_FLAG_F21_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH21 */

#if SPC5_PWM_USE_EMIOS_CH22
  INTC.PSR[SPC5_EMIOS_FLAG_F22_NUMBER].R = SPC5_EMIOS_FLAG_F22_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH22 */

#if SPC5_PWM_USE_EMIOS_CH23
  INTC.PSR[SPC5_EMIOS_FLAG_F23_NUMBER].R = SPC5_EMIOS_FLAG_F23_PRIORITY;
#endif /* SPC5_PWM_USE_EMIOS_CH23 */

#endif
}

/**
 * @brief   Configures and activates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp) {

  uint32_t psc = 0;

  osalDbgAssert(get_emios_active_channels() < SPC5_EMIOS_NUM_CHANNELS,
              "too many channels");

  if (pwmp->state == PWM_STOP) {
#if SPC5_PWM_USE_EMIOS_CH0
    if (&PWMD1 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH0 */

#if SPC5_PWM_USE_EMIOS_CH1
    if (&PWMD2 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH1 */

#if SPC5_PWM_USE_EMIOS_CH2
    if (&PWMD3 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH2 */

#if SPC5_PWM_USE_EMIOS_CH3
    if (&PWMD4 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH3 */

#if SPC5_PWM_USE_EMIOS_CH4
    if (&PWMD5 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH4 */

#if SPC5_PWM_USE_EMIOS_CH5
    if (&PWMD6 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH5 */

#if SPC5_PWM_USE_EMIOS_CH6
    if (&PWMD7 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH6 */

#if SPC5_PWM_USE_EMIOS_CH7
    if (&PWMD8== pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH7 */

#if SPC5_PWM_USE_EMIOS_CH8
    if (&PWMD9 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH8 */

#if SPC5_PWM_USE_EMIOS_CH9
    if (&PWMD10 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH9 */

#if SPC5_PWM_USE_EMIOS_CH10
    if (&PWMD11 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH10 */

#if SPC5_PWM_USE_EMIOS_CH11
    if (&PWMD12 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH11 */

#if SPC5_PWM_USE_EMIOS_CH12
    if (&PWMD13 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH12 */

#if SPC5_PWM_USE_EMIOS_CH13
    if (&PWMD14 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH13 */

#if SPC5_PWM_USE_EMIOS_CH14
    if (&PWMD15 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH14 */

#if SPC5_PWM_USE_EMIOS_CH15
    if (&PWMD16 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH15 */

#if SPC5_PWM_USE_EMIOS_CH16
    if (&PWMD17 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH16 */

#if SPC5_PWM_USE_EMIOS_CH17
    if (&PWMD18 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH17 */

#if SPC5_PWM_USE_EMIOS_CH18
    if (&PWMD19 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH18 */

#if SPC5_PWM_USE_EMIOS_CH19
    if (&PWMD20 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH19 */

#if SPC5_PWM_USE_EMIOS_CH20
    if (&PWMD21 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH20 */

#if SPC5_PWM_USE_EMIOS_CH21
    if (&PWMD22 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH21 */

#if SPC5_PWM_USE_EMIOS_CH22
    if (&PWMD23 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH22 */

#if SPC5_PWM_USE_EMIOS_CH23
    if (&PWMD24 == pwmp) {
      increase_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH23 */

    /* Set eMIOS Clock.*/
#if SPC5_PWM_USE_EMIOS
    pwm_active_emios_clock(pwmp);
#endif
  }
  /* Configures the peripheral.*/

  /* Channel enables.*/
  pwmp->emiosp->UCDIS.R &= ~(1 << pwmp->ch_number);

  /* Clear pending IRQs (if any).*/
  pwmp->emiosp->CH[pwmp->ch_number].CSR.R = EMIOSS_OVRC |
      EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Set clock prescaler and control register.*/
  psc = (SPC5_EMIOS_CLK / pwmp->config->frequency);
  osalDbgAssert((psc <= 0xFFFF) &&
              (((psc) * pwmp->config->frequency) == SPC5_EMIOS_CLK) &&
              ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
              "invalid frequency");

  if (pwmp->config->mode == PWM_ALIGN_EDGE) {
    pwmp->emiosp->CH[pwmp->ch_number].CCR.B.UCPREN = 0;
    pwmp->emiosp->CH[pwmp->ch_number].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[pwmp->ch_number].CCR.B.UCPREN = 1U;
    pwmp->emiosp->CH[pwmp->ch_number].CCNTR.R = 1U;
    pwmp->emiosp->CH[pwmp->ch_number].CADR.R = 0U;
    pwmp->emiosp->CH[pwmp->ch_number].CBDR.R = pwmp->config->period;
    pwmp->emiosp->CH[pwmp->ch_number].CCR.R |=
        EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER) | EMIOS_CCR_MODE_OPWFMB | 2U;
    pwmp->emiosp->CH[pwmp->ch_number].CCR.R |= EMIOSC_UCPREN;

    /* Set output polarity.*/
    if (pwmp->config->channels[0].mode == PWM_OUTPUT_ACTIVE_LOW) {
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R |= EMIOSC_EDPOL;
    } else if (pwmp->config->channels[0].mode == PWM_OUTPUT_ACTIVE_HIGH) {
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R &= ~EMIOSC_EDPOL;
    }

    /* Channel disable.*/
    pwmp->emiosp->UCDIS.R |= (1 << pwmp->ch_number);
  } else if (pwmp->config->mode == PWM_ALIGN_CENTER) {
    /* Not implemented.*/
  }
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

  osalDbgAssert(get_emios_active_channels() < SPC5_EMIOS_NUM_CHANNELS,
                "too many channels");

  if (pwmp->state == PWM_READY) {

    /* Disables the peripheral.*/
#if SPC5_PWM_USE_EMIOS_CH0
    if (&PWMD1 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH0 */

#if SPC5_PWM_USE_EMIOS_CH1
    if (&PWMD2 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH1 */

#if SPC5_PWM_USE_EMIOS_CH2
    if (&PWMD3 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH2 */

#if SPC5_PWM_USE_EMIOS_CH3
    if (&PWMD4 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH3 */

#if SPC5_PWM_USE_EMIOS_CH4
    if (&PWMD5 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH4 */

#if SPC5_PWM_USE_EMIOS_CH5
    if (&PWMD6 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH5 */

#if SPC5_PWM_USE_EMIOS_CH6
    if (&PWMD7 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH6 */

#if SPC5_PWM_USE_EMIOS_CH7
    if (&PWMD8 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH7 */

#if SPC5_PWM_USE_EMIOS_CH8
    if (&PWMD9 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH8 */

#if SPC5_PWM_USE_EMIOS_CH9
    if (&PWMD10 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH9 */

#if SPC5_PWM_USE_EMIOS_CH10
    if (&PWMD11 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH10 */

#if SPC5_PWM_USE_EMIOS_CH11
    if (&PWMD12 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH11 */

#if SPC5_PWM_USE_EMIOS_CH12
    if (&PWMD13 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH12 */

#if SPC5_PWM_USE_EMIOS_CH13
    if (&PWMD14 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH13 */

#if SPC5_PWM_USE_EMIOS_CH14
    if (&PWMD15 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH14 */

#if SPC5_PWM_USE_EMIOS_CH15
    if (&PWMD16 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH15 */

#if SPC5_PWM_USE_EMIOS_CH16
    if (&PWMD17 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH16 */

#if SPC5_PWM_USE_EMIOS_CH17
    if (&PWMD18 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH17 */

#if SPC5_PWM_USE_EMIOS_CH18
    if (&PWMD19 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH18 */

#if SPC5_PWM_USE_EMIOS_CH19
    if (&PWMD20 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH19 */

#if SPC5_PWM_USE_EMIOS_CH20
    if (&PWMD21 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH20 */

#if SPC5_PWM_USE_EMIOS_CH21
    if (&PWMD22 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH21 */

#if SPC5_PWM_USE_EMIOS_CH22
    if (&PWMD23 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH22 */

#if SPC5_PWM_USE_EMIOS_CH23
    if (&PWMD24 == pwmp) {
      /* Reset UC Control Register.*/
      pwmp->emiosp->CH[pwmp->ch_number].CCR.R = 0;

      decrease_emios_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS_CH23 */

    /* eMIOS clock deactivation.*/
#if SPC5_PWM_USE_EMIOS
    deactive_emios_clock();
#endif

  }
}

/**
 * @brief   Changes the period the PWM peripheral.
 * @details This function changes the period of a PWM unit that has already
 *          been activated using @p pwmStart().
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The PWM unit period is changed to the new value.
 * @note    The function has effect at the next cycle start.
 * @note    If a period is specified that is shorter than the pulse width
 *          programmed in one of the channels then the behavior is not
 *          guaranteed.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] period    new cycle time in ticks
 *
 * @notapi
 */
void pwm_lld_change_period(PWMDriver *pwmp, pwmcnt_t period) {

  pwmp->period = period;
  pwmp->emiosp->CH[pwmp->ch_number].CBDR.R = period;
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width) {

  (void)channel;

  /* Clear pending IRQs (if any).*/
  pwmp->emiosp->CH[pwmp->ch_number].CSR.R = EMIOSS_OVRC |
      EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Set pwm width.*/
  pwmp->emiosp->CH[pwmp->ch_number].CADR.R = width;

  /* Active interrupts.*/
  if (pwmp->config->callback != NULL ||                                   \
      pwmp->config->channels[0].callback != NULL) {
    pwmp->emiosp->CH[pwmp->ch_number].CCR.B.FEN = 1U;
  }

  /* Channel enables.*/
  pwmp->emiosp->UCDIS.R &= ~(1 << pwmp->ch_number);
}

/**
 * @brief   Disables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {

  (void)channel;
  /* Clear pending IRQs (if any).*/
  pwmp->emiosp->CH[pwmp->ch_number].CSR.R = EMIOSS_OVRC |
        EMIOSS_OVFLC | EMIOSS_FLAGC;

  /* Disable interrupts.*/
  pwmp->emiosp->CH[pwmp->ch_number].CCR.B.FEN = 0;

  /* Channel disables.*/
  pwmp->emiosp->UCDIS.R |= (1 << pwmp->ch_number);
}

#endif /* HAL_USE_PWM */

/** @} */
