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
 * @file    eMIOS_v1/hal_pwm_lld.c
 * @brief   SPC5xx low level PWM driver code.
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
 * @note    The driver PWMD1 allocates the unified channels eMIOS0_CH8 -
 *          eMIOS0_CH15 when enabled.
 */
#if SPC5_PWM_USE_EMIOS0_GROUP0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the unified channels eMIOS0_CH16 -
 *          eMIOS0_CH23 when enabled.
 */
#if SPC5_PWM_USE_EMIOS0_GROUP1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the unified channels eMIOS1_CH0 -
 *          eMIOS1_CH7 when enabled.
 */
#if SPC5_PWM_USE_EMIOS1_GROUP0 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/**
 * @brief   PWMD4 driver identifier.
 * @note    The driver PWMD4 allocates the unified channels eMIOS1_CH8 -
 *          eMIOS1_CH15 when enabled.
 */
#if SPC5_PWM_USE_EMIOS1_GROUP1 || defined(__DOXYGEN__)
PWMDriver PWMD4;
#endif

/**
 * @brief   PWMD5 driver identifier.
 * @note    The driver PWMD5 allocates the unified channels eMIOS1_CH16 -
 *          eMIOS1_CH23 when enabled.
 */
#if SPC5_PWM_USE_EMIOS1_GROUP2 || defined(__DOXYGEN__)
PWMDriver PWMD5;
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
static void pwm_lld_serve_interrupt1(PWMDriver *pwmp, uint32_t index) {

  uint32_t sr = pwmp->emiosp->CH[index].CSR.R;
  if (sr & EMIOSS_OVFL) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_OVFLC;
  }
  if (sr & EMIOSS_OVR) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_OVRC;
  }
  if (sr & EMIOSS_FLAG) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_FLAGC;
    if (pwmp->config->callback != NULL) {
      pwmp->config->callback(pwmp);
    }
  }

}

static void pwm_lld_serve_interrupt2(PWMDriver *pwmp, uint32_t index) {

  uint32_t sr = pwmp->emiosp->CH[index].CSR.R;
  if (sr & EMIOSS_OVFL) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_OVFLC;
  }
  if (sr & EMIOSS_OVR) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_OVRC;
  }
  if (sr & EMIOSS_FLAG) {
    pwmp->emiosp->CH[index].CSR.R |= EMIOSS_FLAGC;
    if (pwmp->config->channels[index%8U - 1].callback != NULL) {
      pwmp->config->channels[index%8U - 1].callback(pwmp);
    }
  }

}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SPC5_PWM_USE_EMIOS0_GROUP0
#if !defined(SPC5_EMIOS0_GFR_F8F9_HANDLER)
#error "SPC5_EMIOS0_GFR_F8F9_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 8 and 9 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F8F9_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD1.emiosp->GFR.R;

  if (gfr & (1U << 8U)) {
    pwm_lld_serve_interrupt1(&PWMD1, 8U);
  }
  if (gfr & (1U << 9U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 9U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F10F11_HANDLER)
#error "SPC5_EMIOS0_GFR_F10F11_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 10 and 11 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F10F11_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD1.emiosp->GFR.R;

  if (gfr & (1U << 10U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 10U);
  }
  if (gfr & (1U << 11U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 11U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F12F13_HANDLER)
#error "SPC5_EMIOS0_GFR_F12F13_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 12 and 13 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F12F13_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD1.emiosp->GFR.R;

  if (gfr & (1U << 12U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 12U);
  }
  if (gfr & (1U << 13U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 13U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F14F15_HANDLER)
#error "SPC5_EMIOS0_GFR_F14F15_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 14 and 15 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F14F15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD1.emiosp->GFR.R;

  if (gfr & (1U << 14U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 14U);
  }
  if (gfr & (1U << 15U)) {
    pwm_lld_serve_interrupt2(&PWMD1, 15U);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS0_GROUP0 */

#if SPC5_PWM_USE_EMIOS0_GROUP1
#if !defined(SPC5_EMIOS0_GFR_F16F17_HANDLER)
#error "SPC5_EMIOS0_GFR_F16F17_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 16 and 17 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F16F17_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD2.emiosp->GFR.R;

  if (gfr & (1U << 16U)) {
    pwm_lld_serve_interrupt1(&PWMD2, 16U);
  }
  if (gfr & (1U << 17U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 17U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F18F19_HANDLER)
#error "SPC5_EMIOS0_GFR_F18F19_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 18 and 19 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F18F19_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD2.emiosp->GFR.R;

  if (gfr & (1U << 18U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 18U);
  }
  if (gfr & (1U << 19U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 19U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F20F21_HANDLER)
#error "SPC5_EMIOS0_GFR_F20F21_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 20 and 21 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F20F21_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD2.emiosp->GFR.R;

  if (gfr & (1U << 20U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 20U);
  }
  if (gfr & (1U << 21U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 21U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS0_GFR_F22F23_HANDLER)
#error "SPC5_EMIOS0_GFR_F22F23_HANDLER not defined"
#endif
/**
 * @brief   eMIOS0 Channels 22 and 23 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS0_GFR_F22F23_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD2.emiosp->GFR.R;

  if (gfr & (1U << 22U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 22U);
  }
  if (gfr & (1U << 23U)) {
    pwm_lld_serve_interrupt2(&PWMD2, 23U);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS0_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP0
#if !defined(SPC5_EMIOS1_GFR_F0F1_HANDLER)
#error "SPC5_EMIOS1_GFR_F0F1_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 0 and 1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F0F1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD3.emiosp->GFR.R;

  if (gfr & (1U << 0)) {
    pwm_lld_serve_interrupt1(&PWMD3, 0);
  }
  if (gfr & (1U << 1U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 1U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F2F3_HANDLER)
#error "SPC5_EMIOS1_GFR_F2F3_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 2 and 3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F2F3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD3.emiosp->GFR.R;

  if (gfr & (1U << 2U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 2U);
  }
  if (gfr & (1U << 3U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 3U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F4F5_HANDLER)
#error "SPC5_EMIOS1_GFR_F4F5_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 4 and 5 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F4F5_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD3.emiosp->GFR.R;

  if (gfr & (1U << 4U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 4U);
  }
  if (gfr & (1U << 5U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 5U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F6F7_HANDLER)
#error "SPC5_EMIOS1_GFR_F6F7_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 6 and 7 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F6F7_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD3.emiosp->GFR.R;

  if (gfr & (1U << 6U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 6U);
  }
  if (gfr & (1U << 7U)) {
    pwm_lld_serve_interrupt2(&PWMD3, 7U);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS1_GROUP0 */

#if SPC5_PWM_USE_EMIOS1_GROUP1
#if !defined(SPC5_EMIOS1_GFR_F8F9_HANDLER)
#error "SPC5_EMIOS1_GFR_F8F9_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 8 and 9 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F8F9_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD4.emiosp->GFR.R;

  if (gfr & (1U << 8U)) {
    pwm_lld_serve_interrupt1(&PWMD4, 8U);
  }
  if (gfr & (1U << 9U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 9U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F10F11_HANDLER)
#error "SPC5_EMIOS1_GFR_F10F11_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 10 and 11 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F10F11_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD4.emiosp->GFR.R;

  if (gfr & (1U << 10U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 10U);
  }
  if (gfr & (1U << 11U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 11U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F12F13_HANDLER)
#error "SPC5_EMIOS1_GFR_F12F13_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 12 and 13 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F12F13_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD4.emiosp->GFR.R;

  if (gfr & (1U << 12U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 12U);
  }
  if (gfr & (1U << 13U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 13U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F14F15_HANDLER)
#error "SPC5_EMIOS1_GFR_F14F15_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 14 and 15 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F14F15_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD4.emiosp->GFR.R;

  if (gfr & (1U << 14U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 14U);
  }
  if (gfr & (1U << 15U)) {
    pwm_lld_serve_interrupt2(&PWMD4, 15U);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS1_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP2
#if !defined(SPC5_EMIOS1_GFR_F16F17_HANDLER)
#error "SPC5_EMIOS1_GFR_F16F17_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 16 and 17 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F16F17_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD5.emiosp->GFR.R;

  if (gfr & (1U << 16U)) {
    pwm_lld_serve_interrupt1(&PWMD5, 16U);
  }
  if (gfr & (1U << 17U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 17U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F18F19_HANDLER)
#error "SPC5_EMIOS1_GFR_F18F19_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 18 and 19 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F18F19_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD5.emiosp->GFR.R;

  if (gfr & (1U << 18U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 18U);
  }
  if (gfr & (1U << 19U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 19U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F20F21_HANDLER)
#error "SPC5_EMIOS1_GFR_F20F21_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 20 and 21 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F20F21_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD5.emiosp->GFR.R;

  if (gfr & (1U << 20U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 20U);
  }
  if (gfr & (1U << 21U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 21U);
  }

  OSAL_IRQ_EPILOGUE();
}

#if !defined(SPC5_EMIOS1_GFR_F22F23_HANDLER)
#error "SPC5_EMIOS1_GFR_F22F23_HANDLER not defined"
#endif
/**
 * @brief   eMIOS1 Channels 22 and 23 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SPC5_EMIOS1_GFR_F22F23_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t gfr = PWMD5.emiosp->GFR.R;

  if (gfr & (1U << 22U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 22U);
  }
  if (gfr & (1U << 23U)) {
    pwm_lld_serve_interrupt2(&PWMD5, 23U);
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* SPC5_PWM_USE_EMIOS1_GROUP2 */

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
#if SPC5_HAS_EMIOS0
  reset_emios0_active_channels();
#endif
#if SPC5_HAS_EMIOS1
  reset_emios1_active_channels();
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP0
  /* Driver initialization.*/
  pwmObjectInit(&PWMD1);
  PWMD1.emiosp = &EMIOS_0;
#endif /* SPC5_PWM_USE_EMIOS0_GROUP0 */

#if SPC5_PWM_USE_EMIOS0_GROUP1
  /* Driver initialization.*/
  pwmObjectInit(&PWMD2);
  PWMD2.emiosp = &EMIOS_0;
#endif /* SPC5_PWM_USE_EMIOS0_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP0
  /* Driver initialization.*/
  pwmObjectInit(&PWMD3);
  PWMD3.emiosp = &EMIOS_1;
#endif /* SPC5_PWM_USE_EMIOS1_GROUP0 */

#if SPC5_PWM_USE_EMIOS1_GROUP1
  /* Driver initialization.*/
  pwmObjectInit(&PWMD4);
  PWMD4.emiosp = &EMIOS_1;
#endif /* SPC5_PWM_USE_EMIOS1_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP2
  /* Driver initialization.*/
  pwmObjectInit(&PWMD5);
  PWMD5.emiosp = &EMIOS_1;
#endif /* SPC5_PWM_USE_EMIOS1_GROUP2 */

#if SPC5_PWM_USE_EMIOS0

  INTC.PSR[SPC5_EMIOS0_GFR_F8F9_NUMBER].R = SPC5_EMIOS0_GFR_F8F9_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F10F11_NUMBER].R = SPC5_EMIOS0_GFR_F10F11_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F12F13_NUMBER].R = SPC5_EMIOS0_GFR_F12F13_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F14F15_NUMBER].R = SPC5_EMIOS0_GFR_F14F15_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F16F17_NUMBER].R = SPC5_EMIOS0_GFR_F16F17_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F18F19_NUMBER].R = SPC5_EMIOS0_GFR_F18F19_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F20F21_NUMBER].R = SPC5_EMIOS0_GFR_F20F21_PRIORITY;
  INTC.PSR[SPC5_EMIOS0_GFR_F22F23_NUMBER].R = SPC5_EMIOS0_GFR_F22F23_PRIORITY;

#endif

#if SPC5_PWM_USE_EMIOS1

  INTC.PSR[SPC5_EMIOS1_GFR_F0F1_NUMBER].R = SPC5_EMIOS1_GFR_F0F1_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F2F3_NUMBER].R = SPC5_EMIOS1_GFR_F2F3_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F4F5_NUMBER].R = SPC5_EMIOS1_GFR_F4F5_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F6F7_NUMBER].R = SPC5_EMIOS1_GFR_F6F7_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F8F9_NUMBER].R = SPC5_EMIOS1_GFR_F8F9_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F10F11_NUMBER].R = SPC5_EMIOS1_GFR_F10F11_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F12F13_NUMBER].R = SPC5_EMIOS1_GFR_F12F13_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F14F15_NUMBER].R = SPC5_EMIOS1_GFR_F14F15_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F16F17_NUMBER].R = SPC5_EMIOS1_GFR_F16F17_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F18F19_NUMBER].R = SPC5_EMIOS1_GFR_F18F19_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F20F21_NUMBER].R = SPC5_EMIOS1_GFR_F20F21_PRIORITY;
  INTC.PSR[SPC5_EMIOS1_GFR_F22F23_NUMBER].R = SPC5_EMIOS1_GFR_F22F23_PRIORITY;

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

  uint32_t psc = 0, i = 0;

#if SPC5_HAS_EMIOS0
  osalDbgAssert(get_emios0_active_channels() < 25, "too many channels");
#endif
#if SPC5_HAS_EMIOS1
  osalDbgAssert(get_emios1_active_channels() < 25, "too many channels");
#endif

  if (pwmp->state == PWM_STOP) {
#if SPC5_PWM_USE_EMIOS0_GROUP0
    if (&PWMD1 == pwmp) {
      increase_emios0_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS0_GROUP0 */

#if SPC5_PWM_USE_EMIOS0_GROUP1
    if (&PWMD2 == pwmp) {
      increase_emios0_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS0_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP0
    if (&PWMD3 == pwmp) {
      increase_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP0 */

#if SPC5_PWM_USE_EMIOS1_GROUP1
    if (&PWMD4 == pwmp) {
      increase_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP2
    if (&PWMD5 == pwmp) {
      increase_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP2 */

    /* Set eMIOS0 Clock.*/
#if SPC5_PWM_USE_EMIOS0
    pwm_active_emios0_clock(pwmp);
#endif

    /* Set eMIOS1 Clock.*/
#if SPC5_PWM_USE_EMIOS1
    pwm_active_emios1_clock(pwmp);
#endif

  }
  /* Configures the peripheral.*/

#if SPC5_PWM_USE_EMIOS0_GROUP0
  if (&PWMD1 == pwmp) {
    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << 8U);

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[8U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1
  if (&PWMD2 == pwmp) {
    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << 16U);

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[16U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0
  if (&PWMD3 == pwmp) {
    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~1U;

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[0].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1
  if (&PWMD4 == pwmp) {
    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << 8U);

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[8U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2
  if (&PWMD5 == pwmp) {
    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << 16U);

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[16U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

  }
#endif

  /* Set clock prescaler and control register.*/
#if SPC5_HAS_EMIOS0 && SPC5_HAS_EMIOS1
  if (pwmp->emiosp == &EMIOS_0) {
    psc = (SPC5_EMIOS0_CLK / pwmp->config->frequency);
    osalDbgAssert((psc <= 0xFFFF) &&
                (((psc) * pwmp->config->frequency) == SPC5_EMIOS0_CLK) &&
                ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
                 "invalid frequency");
  } else if (pwmp->emiosp == &EMIOS_1) {
    psc = (SPC5_EMIOS1_CLK / pwmp->config->frequency);
    osalDbgAssert((psc <= 0xFFFF) &&
                (((psc) * pwmp->config->frequency) == SPC5_EMIOS1_CLK) &&
                ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
                 "invalid frequency");
  }
#elif SPC5_HAS_EMIOS0
  if (pwmp->emiosp == &EMIOS_0) {
    psc = (SPC5_EMIOS0_CLK / pwmp->config->frequency);
    osalDbgAssert((psc <= 0xFFFF) &&
                (((psc) * pwmp->config->frequency) == SPC5_EMIOS0_CLK) &&
                ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
                "invalid frequency");
  }

#elif SPC5_HAS_EMIOS1
  if (pwmp->emiosp == &EMIOS_1) {
    psc = (SPC5_EMIOS1_CLK / pwmp->config->frequency);
    osalDbgAssert((psc <= 0xFFFF) &&
                (((psc) * pwmp->config->frequency) == SPC5_EMIOS1_CLK) &&
                ((psc == 1) || (psc == 2) || (psc == 3) || (psc == 4)),
                "invalid frequency");
  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP0
  if (&PWMD1 == pwmp) {

    pwmp->emiosp->CH[8U].CCR.B.UCPEN = 0;
    pwmp->emiosp->CH[8U].CCNTR.R = 1U;
    pwmp->emiosp->CH[8U].CADR.R = pwmp->config->period;
    pwmp->emiosp->CH[8U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER);
    pwmp->emiosp->CH[8U].CCR.R |= EMIOS_CCR_MODE_MCB_UP;
    pwmp->emiosp->CH[8U].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[8U].CCR.R |= EMIOSC_UCPREN;

    if (pwmp->config->mode == PWM_ALIGN_EDGE) {
      for (i = 0; i < PWM_CHANNELS; i++) {
        switch (pwmp->config->channels[i].mode) {
        case PWM_OUTPUT_DISABLED:
          break;
        case PWM_OUTPUT_ACTIVE_HIGH:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (9U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 9U].CADR.R = 0;
          pwmp->emiosp->CH[i + 9U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 9U));

          break;
        case PWM_OUTPUT_ACTIVE_LOW:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (9U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 9U].CADR.R = 1U;
          pwmp->emiosp->CH[i + 9U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 9U].CCR.R &= ~EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 9U));

          break;
        }
      }

      /* Channel disables.*/
      pwmp->emiosp->UCDIS.R |= (1U << 8U);

    }
  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1
  if (&PWMD2 == pwmp) {

    pwmp->emiosp->CH[16U].CCR.B.UCPEN = 0;
    pwmp->emiosp->CH[16U].CCNTR.R = 1U;
    pwmp->emiosp->CH[16U].CADR.R = pwmp->config->period;
    pwmp->emiosp->CH[16U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER);
    pwmp->emiosp->CH[16U].CCR.R |= EMIOS_CCR_MODE_MCB_UP;
    pwmp->emiosp->CH[16U].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[16U].CCR.R |= EMIOSC_UCPREN;

    if (pwmp->config->mode == PWM_ALIGN_EDGE) {
      for (i = 0; i < PWM_CHANNELS; i++) {
        switch (pwmp->config->channels[i].mode) {
        case PWM_OUTPUT_DISABLED:
          break;
        case PWM_OUTPUT_ACTIVE_HIGH:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (17U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 17U].CADR.R = 0;
          pwmp->emiosp->CH[i + 17U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 17U));

          break;
        case PWM_OUTPUT_ACTIVE_LOW:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (17U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 17U].CADR.R = 1U;
          pwmp->emiosp->CH[i + 17U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 17U].CCR.R &= ~EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 17U));

          break;
        }
      }

      /* Channel disables.*/
      pwmp->emiosp->UCDIS.R |= (1U << 16U);

    }
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0
  if (&PWMD3 == pwmp) {

    pwmp->emiosp->CH[0].CCR.B.UCPEN = 0;
    pwmp->emiosp->CH[0].CCNTR.R = 1U;
    pwmp->emiosp->CH[0].CADR.R = pwmp->config->period;
    pwmp->emiosp->CH[0].CCR.R |= EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER);
    pwmp->emiosp->CH[0].CCR.R |= EMIOS_CCR_MODE_MCB_UP;
    pwmp->emiosp->CH[0].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[0].CCR.R |= EMIOSC_UCPREN;

    if (pwmp->config->mode == PWM_ALIGN_EDGE) {
      for (i = 0; i < PWM_CHANNELS; i++) {
        switch (pwmp->config->channels[i].mode) {
        case PWM_OUTPUT_DISABLED:
          break;
        case PWM_OUTPUT_ACTIVE_HIGH:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (1U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 1U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 1U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 1U].CADR.R = 0;
          pwmp->emiosp->CH[i + 1U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 1U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 1U));

          break;
        case PWM_OUTPUT_ACTIVE_LOW:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (1U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 1U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 1U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 1U].CADR.R = 1U;
          pwmp->emiosp->CH[i + 1U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 1U].CCR.R &= ~EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 1U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 1U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 1U));

          break;
        }
      }

      /* Channel disables.*/
      pwmp->emiosp->UCDIS.R |= 1U;

    }
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1
  if (&PWMD4 == pwmp) {

    pwmp->emiosp->CH[8U].CCR.B.UCPEN = 0;
    pwmp->emiosp->CH[8U].CCNTR.R = 1U;
    pwmp->emiosp->CH[8U].CADR.R = pwmp->config->period;
    pwmp->emiosp->CH[8U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER);
    pwmp->emiosp->CH[8U].CCR.R |= EMIOS_CCR_MODE_MCB_UP;
    pwmp->emiosp->CH[8U].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[8U].CCR.R |= EMIOSC_UCPREN;

    if (pwmp->config->mode == PWM_ALIGN_EDGE) {
      for (i = 0; i < PWM_CHANNELS; i++) {
        switch (pwmp->config->channels[i].mode) {
        case PWM_OUTPUT_DISABLED:
          break;
        case PWM_OUTPUT_ACTIVE_HIGH:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (9U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 9U].CADR.R = 0;
          pwmp->emiosp->CH[i + 9U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 9U));

          break;
        case PWM_OUTPUT_ACTIVE_LOW:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (9U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 9U].CADR.R = 1U;
          pwmp->emiosp->CH[i + 9U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 9U].CCR.R &= ~EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 9U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 9U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 9U));

          break;
        }
      }

      /* Channel disables.*/
      pwmp->emiosp->UCDIS.R |= (1U << 8U);

    }
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2
  if (&PWMD5 == pwmp) {

    pwmp->emiosp->CH[16U].CCR.B.UCPEN = 0;
    pwmp->emiosp->CH[16U].CCNTR.R = 1U;
    pwmp->emiosp->CH[16U].CADR.R = pwmp->config->period;
    pwmp->emiosp->CH[16U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_INTERNAL_COUNTER);
    pwmp->emiosp->CH[16U].CCR.R |= EMIOS_CCR_MODE_MCB_UP;
    pwmp->emiosp->CH[16U].CCR.B.UCPRE = psc - 1U;
    pwmp->emiosp->CH[16U].CCR.R |= EMIOSC_UCPREN;

    if (pwmp->config->mode == PWM_ALIGN_EDGE) {
      for (i = 0; i < PWM_CHANNELS; i++) {
        switch (pwmp->config->channels[i].mode) {
        case PWM_OUTPUT_DISABLED:
          break;
        case PWM_OUTPUT_ACTIVE_HIGH:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (17U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 17U].CADR.R = 0;
          pwmp->emiosp->CH[i + 17U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 17U));

          break;
        case PWM_OUTPUT_ACTIVE_LOW:

          /* Channel enables.*/
          pwmp->emiosp->UCDIS.R &= ~(1U << (17U + i));

          /* Clear pending IRQs (if any).*/
          pwmp->emiosp->CH[i + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
              EMIOSS_FLAGC;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPEN = 0;
          pwmp->emiosp->CH[i + 17U].CADR.R = 1U;
          pwmp->emiosp->CH[i + 17U].CBDR.R = 0;

          /* Set output polarity.*/
          pwmp->emiosp->CH[i + 17U].CCR.R &= ~EMIOSC_EDPOL;

          /* Set unified channel mode.*/
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_BSL(EMIOS_BSL_COUNTER_BUS_2);
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOS_CCR_MODE_OPWMB;

          pwmp->emiosp->CH[i + 17U].CCR.B.UCPRE = psc - 1U;
          pwmp->emiosp->CH[i + 17U].CCR.R |= EMIOSC_UCPREN;

          /* Channel disables.*/
          pwmp->emiosp->UCDIS.R |= (1U << (i + 17U));

          break;
        }
      }

      /* Channel disables.*/
      pwmp->emiosp->UCDIS.R |= (1U << 16U);

    }
  }
#endif

}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

  uint32_t i = 0;

#if SPC5_HAS_EMIOS0
  osalDbgAssert(get_emios0_active_channels() < 25, "too many channels");
#endif
#if SPC5_HAS_EMIOS1
  osalDbgAssert(get_emios1_active_channels() < 25, "too many channels");
#endif

  if (pwmp->state == PWM_READY) {

    /* Disables the peripheral.*/
#if SPC5_PWM_USE_EMIOS0_GROUP0
    if (&PWMD1 == pwmp) {
      /* Reset UC Control Register of group channels.*/
      for (i = 0; i < 8; i++) {
        pwmp->emiosp->CH[i + 8U].CCR.R = 0;
      }
      decrease_emios0_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS0_GROUP0 */

#if SPC5_PWM_USE_EMIOS0_GROUP1
    if (&PWMD2 == pwmp) {
      /* Reset UC Control Register of group channels.*/
      for (i = 0; i < 8; i++) {
        pwmp->emiosp->CH[i + 16U].CCR.R = 0;
      }
      decrease_emios0_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS0_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP0
    if (&PWMD3 == pwmp) {
      /* Reset UC Control Register of group channels.*/
      for (i = 0; i < 8; i++) {
        pwmp->emiosp->CH[i].CCR.R = 0;
      }
      decrease_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP0 */

#if SPC5_PWM_USE_EMIOS1_GROUP1
    if (&PWMD4 == pwmp) {
      /* Reset UC Control Register of group channels.*/
      for (i = 0; i < 8; i++) {
        pwmp->emiosp->CH[i + 8U].CCR.R = 0;
      }
      decrease_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP1 */

#if SPC5_PWM_USE_EMIOS1_GROUP2
    if (&PWMD5 == pwmp) {
      /* Reset UC Control Register of group channels.*/
      for (i = 0; i < 8; i++) {
        pwmp->emiosp->CH[i + 16U].CCR.R = 0;
      }
      decrease_emios1_active_channels();
    }
#endif /* SPC5_PWM_USE_EMIOS1_GROUP2 */

    /* eMIOS0 clock deactivation.*/
#if SPC5_PWM_USE_EMIOS0
    pwm_deactive_emios0_clock(pwmp);
#endif

    /* eMIOS1 clock deactivation.*/
#if SPC5_PWM_USE_EMIOS1
    pwm_deactive_emios1_clock(pwmp);
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

#if SPC5_PWM_USE_EMIOS0_GROUP0
  if (&PWMD1 == pwmp) {
    pwmp->period = period;
    pwmp->emiosp->CH[8U].CADR.R = period;
  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1
  if (&PWMD2 == pwmp) {
    pwmp->period = period;
    pwmp->emiosp->CH[16U].CADR.R = period;
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0
  if (&PWMD3 == pwmp) {
    pwmp->period = period;
    pwmp->emiosp->CH[0].CADR.R = period;
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1
  if (&PWMD4 == pwmp) {
    pwmp->period = period;
    pwmp->emiosp->CH[8U].CADR.R = period;
  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2
  if (&PWMD5 == pwmp) {
    pwmp->period = period;
    pwmp->emiosp->CH[16U].CADR.R = period;
  }
#endif

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

#if SPC5_PWM_USE_EMIOS0_GROUP0
  if (&PWMD1 == pwmp) {

    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << (9U + channel));

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

    /* Set PWM width.*/
    pwmp->emiosp->CH[channel + 9U].CBDR.R = width;

    /* Active interrupts.*/
    if (pwmp->config->channels[channel].callback != NULL) {
      pwmp->emiosp->CH[channel + 9U].CCR.B.FEN = 1U;
    }

    /* Enables timer base channel if disable.*/
    if (pwmp->emiosp->UCDIS.R & (1U << 8U)) {
      /* Channel enables.*/
      pwmp->emiosp->UCDIS.R &= ~(1U << 8U);

      /* Active interrupts.*/
      if (pwmp->config->callback != NULL ) {
        pwmp->emiosp->CH[8U].CCR.B.FEN = 1U;
      }
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1
  if (&PWMD2 == pwmp) {

    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << (17U + channel));

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

    /* Set PWM width.*/
    pwmp->emiosp->CH[channel + 17U].CBDR.R = width;

    /* Active interrupts.*/
    if (pwmp->config->channels[channel].callback != NULL) {
      pwmp->emiosp->CH[channel + 17U].CCR.B.FEN = 1U;
    }

    /* Enables timer base channel if disable.*/
    if (pwmp->emiosp->UCDIS.R & (1U << 16U)) {
      /* Channel enables.*/
      pwmp->emiosp->UCDIS.R &= ~(1U << 16U);

      /* Active interrupts.*/
      if (pwmp->config->callback != NULL ) {
        pwmp->emiosp->CH[16U].CCR.B.FEN = 1U;
      }
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0
  if (&PWMD3 == pwmp) {

    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << (1U + channel));

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 1U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

    /* Set PWM width.*/
    pwmp->emiosp->CH[channel + 1U].CBDR.R = width;


    /* Active interrupts.*/
    if (pwmp->config->channels[channel].callback != NULL) {
      pwmp->emiosp->CH[channel + 1U].CCR.B.FEN = 1U;
    }

    /* Enables timer base channel if disable.*/
    if (pwmp->emiosp->UCDIS.R & 1U) {
      /* Channel enables.*/
      pwmp->emiosp->UCDIS.R &= ~1U;

      /* Active interrupts.*/
      if (pwmp->config->callback != NULL ) {
        pwmp->emiosp->CH[0].CCR.B.FEN = 1U;
      }
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1
  if (&PWMD4 == pwmp) {

    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << (9U + channel));

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 9U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

    /* Set PWM width.*/
    pwmp->emiosp->CH[channel + 9U].CBDR.R = width;

    /* Active interrupts.*/
    if (pwmp->config->channels[channel].callback != NULL) {
      pwmp->emiosp->CH[channel + 9U].CCR.B.FEN = 1U;
    }

    /* Enables timer base channel if disable.*/
    if (pwmp->emiosp->UCDIS.R & (1U << 8U)) {
      /* Channel enables.*/
      pwmp->emiosp->UCDIS.R &= ~(1U << 8U);

      /* Active interrupts.*/
      if (pwmp->config->callback != NULL ) {
        pwmp->emiosp->CH[8U].CCR.B.FEN = 1U;
      }
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2
  if (&PWMD5 == pwmp) {

    /* Channel enables.*/
    pwmp->emiosp->UCDIS.R &= ~(1U << (17U + channel));

    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 17U].CSR.R = EMIOSS_OVRC | EMIOSS_OVFLC |
        EMIOSS_FLAGC;

    /* Set PWM width.*/
    pwmp->emiosp->CH[channel + 17U].CBDR.R = width;

    /* Active interrupts.*/
    if (pwmp->config->channels[channel].callback != NULL) {
      pwmp->emiosp->CH[channel + 17U].CCR.B.FEN = 1U;
    }

    /* Enables timer base channel if disable.*/
    if (pwmp->emiosp->UCDIS.R & (1U << 16U)) {
      /* Channel enables.*/
      pwmp->emiosp->UCDIS.R &= ~(1U << 16U);

      /* Active interrupts.*/
      if (pwmp->config->callback != NULL ) {
        pwmp->emiosp->CH[16U].CCR.B.FEN = 1U;
      }
    }

  }
#endif

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

#if SPC5_PWM_USE_EMIOS0_GROUP0
  if (&PWMD1 == pwmp) {
    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 9U].CSR.R = EMIOSS_OVRC |
          EMIOSS_OVFLC | EMIOSS_FLAGC;

    /* Disable interrupts.*/
    pwmp->emiosp->CH[channel + 9U].CCR.B.FEN = 0;

    /* Disable channel.*/
    pwmp->emiosp->UCDIS.R |= (1U << (channel + 9U));

    /* Disable timer base channel if all PWM channels are disabled.*/
    if ((pwmp->emiosp->UCDIS.R & (0xFE << 8U)) == (0xFE << 8U)) {
      /* Deactive interrupts.*/
      pwmp->emiosp->CH[8U].CCR.B.FEN = 0;

      /* Disable channel.*/
      pwmp->emiosp->UCDIS.R |= (1U << 8U);
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1
  if (&PWMD2 == pwmp) {
    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 17U].CSR.R = EMIOSS_OVRC |
          EMIOSS_OVFLC | EMIOSS_FLAGC;

    /* Disable interrupts.*/
    pwmp->emiosp->CH[channel + 17U].CCR.B.FEN = 0;

    /* Disable channel.*/
    pwmp->emiosp->UCDIS.R |= (1U << (channel + 17));

    /* Disable timer base channel if all PWM channels are disabled.*/
    if ((pwmp->emiosp->UCDIS.R & (0xFE << 16U)) == (0xFE << 16U)) {
      /* Deactive interrupts.*/
      pwmp->emiosp->CH[16U].CCR.B.FEN = 0;

      /* Disable channel.*/
      pwmp->emiosp->UCDIS.R |= (1U << 16U);
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0
  if (&PWMD3 == pwmp) {
    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 1U].CSR.R = EMIOSS_OVRC |
          EMIOSS_OVFLC | EMIOSS_FLAGC;

    /* Disable interrupts.*/
    pwmp->emiosp->CH[channel + 1U].CCR.B.FEN = 0;

    /* Disable channel.*/
    pwmp->emiosp->UCDIS.R |= (1U << (channel + 1U));

    /* Disable timer base channel if all PWM channels are disabled.*/
    if ((pwmp->emiosp->UCDIS.R & 0xFE) == 0xFE) {
      /* Deactive interrupts.*/
      pwmp->emiosp->CH[0].CCR.B.FEN = 0;

      /* Disable channel.*/
      pwmp->emiosp->UCDIS.R |= 1U;
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1
  if (&PWMD4 == pwmp) {
    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 9U].CSR.R = EMIOSS_OVRC |
          EMIOSS_OVFLC | EMIOSS_FLAGC;

    /* Disable interrupts.*/
    pwmp->emiosp->CH[channel + 9U].CCR.B.FEN = 0;

    /* Disable channel.*/
    pwmp->emiosp->UCDIS.R |= (1U << (channel + 9U));

    /* Disable timer base channel if all PWM channels are disabled.*/
    if ((pwmp->emiosp->UCDIS.R & (0xFE << 8U)) == (0xFE << 8U)) {
      /* Deactive interrupts.*/
      pwmp->emiosp->CH[8U].CCR.B.FEN = 0;

      /* Disable channel.*/
      pwmp->emiosp->UCDIS.R |= (1U << 8U);
    }

  }
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2
  if (&PWMD5 == pwmp) {
    /* Clear pending IRQs (if any).*/
    pwmp->emiosp->CH[channel + 17U].CSR.R = EMIOSS_OVRC |
          EMIOSS_OVFLC | EMIOSS_FLAGC;

    /* Disable interrupts.*/
    pwmp->emiosp->CH[channel + 17U].CCR.B.FEN = 0;

    /* Disable channel.*/
    pwmp->emiosp->UCDIS.R |= (1U << (channel + 17U));

    /* Disable timer base channel if all PWM channels are disabled.*/
    if ((pwmp->emiosp->UCDIS.R & (0xFE << 16U)) == (0xFE << 16U)) {
      /* Deactive interrupts.*/
      pwmp->emiosp->CH[16U].CCR.B.FEN = 0;

      /* Disable channel.*/
      pwmp->emiosp->UCDIS.R |= (1U << 16U);
    }

  }
#endif

}

#endif /* HAL_USE_PWM */

/** @} */
