/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    TIVA/LLD/pwm_lld.c
 * @brief   TM4C123x/TM4C129x PWM subsystem low level driver.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define PWM_INT_CMPBD       (1 << 5)
#define PWM_INT_CMPBU       (1 << 4)
#define PWM_INT_CMPAD       (1 << 3)
#define PWM_INT_CMPAU       (1 << 2)
#define PWM_INT_CNTLOAD     (1 << 1)
#define PWM_INT_CNTZERO     (1 << 0)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 */
#if TIVA_PWM_USE_PWM0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 */
#if TIVA_PWM_USE_PWM1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Common PWM Generator IRQ handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] i         pwm generator number
 */
static void pwm_lld_serve_generator_interrupt (PWMDriver *pwmp, uint8_t i)
{
  uint32_t isc;

  isc = pwmp->pwm->PWM[i].ISC;
  pwmp->pwm->PWM[i].ISC = isc;

  if (((isc & PWM_INT_CMPAD) != 0) &&
      (pwmp->config->channels[i * 2 + 0].callback != NULL)) {
    pwmp->config->channels[i * 2 + 0].callback(pwmp);
  }

  if (((isc & PWM_INT_CMPAU) != 0) &&
      (pwmp->config->channels[i * 2 + 0].callback != NULL)) {
    pwmp->config->channels[i * 2 + 0].callback(pwmp);
  }

  if (((isc & PWM_INT_CMPBD) != 0) &&
      (pwmp->config->channels[i * 2 + 1].callback != NULL)) {
    pwmp->config->channels[i * 2 + 1].callback(pwmp);
  }

  if (((isc & PWM_INT_CMPBU) != 0) &&
      (pwmp->config->channels[i * 2 + 1].callback != NULL)) {
    pwmp->config->channels[i * 2 + 1].callback(pwmp);
  }

  if (((isc & PWM_INT_CNTLOAD) != 0) && (pwmp->config->callback != NULL)) {
    pwmp->config->callback(pwmp);
  }

  if (((isc & PWM_INT_CNTZERO) != 0) && (pwmp->config->callback != NULL)) {
    pwmp->config->callback(pwmp);
  }
}

/**
 * @brief   Common PWM fault IRQ handler.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 */
static void pwm_lld_serve_fault_interrupt (PWMDriver *pwmp)
{
  (void) pwmp;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_PWM_USE_PWM0
#if !defined(TIVA_PWM0FAULT_HANDLER)
#error "TIVA_PWM0FAULT_HANDLER not defined"
#endif
/*
 * @brief PWM0 Fault handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM0FAULT_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_fault_interrupt(&PWMD1);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM0GEN0_HANDLER)
#error "TIVA_PWM0GEN0_HANDLER not defined"
#endif
/*
 * @brief PWM0 Generator 0 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM0GEN0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD1, 0);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM0GEN1_HANDLER)
#error "TIVA_PWM0GEN1_HANDLER not defined"
#endif
/*
 * @brief PWM0 Generator 1 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM0GEN1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD1, 1);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM0GEN2_HANDLER)
#error "TIVA_PWM0GEN2_HANDLER not defined"
#endif
/*
 * @brief PWM0 Generator 2 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM0GEN2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD1, 2);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM0GEN3_HANDLER)
#error "TIVA_PWM0GEN3_HANDLER not defined"
#endif
/*
 * @brief PWM0 Generator 3 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM0GEN3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD1, 3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_PWM_USE_PWM1
#if !defined(TIVA_PWM1FAULT_HANDLER)
#error "TIVA_PWM1FAULT_HANDLER not defined"
#endif
/*
 * @brief PWM1 Fault handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM1FAULT_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_fault_interrupt(&PWMD2);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM1GEN0_HANDLER)
#error "TIVA_PWM1GEN0_HANDLER not defined"
#endif
/*
 * @brief PWM1 Generator 0 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM1GEN0_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD2, 0);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM1GEN1_HANDLER)
#error "TIVA_PWM1GEN1_HANDLER not defined"
#endif
/*
 * @brief PWM1 Generator 1 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM1GEN1_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD2, 1);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM1GEN2_HANDLER)
#error "TIVA_PWM1GEN2_HANDLER not defined"
#endif
/*
 * @brief PWM1 Generator 2 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM1GEN2_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD2, 2);

  OSAL_IRQ_EPILOGUE();
}

#if !defined(TIVA_PWM1GEN3_HANDLER)
#error "TIVA_PWM1GEN3_HANDLER not defined"
#endif
/*
 * @brief PWM1 Generator 3 handler
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_PWM1GEN3_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  pwm_lld_serve_generator_interrupt(&PWMD2, 3);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void)
{
  /* Driver initialization.*/
#if TIVA_PWM_USE_PWM0
  pwmObjectInit(&PWMD1);
  PWMD1.channels = PWM_CHANNELS;
  PWMD1.pwm = PWM0;
#endif

#if TIVA_PWM_USE_PWM1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = PWM_CHANNELS;
  PWMD2.pwm = PWM1;
#endif
}

/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp)
{
  uint8_t i;
  uint32_t invert = 0;
  uint32_t enable = 0;

  if (pwmp->state == PWM_STOP) {
    /* Clock activation.*/
#if TIVA_PWM_USE_PWM0
    if (&PWMD1 == pwmp) {
      SYSCTL->RCGCPWM |= (1 << 0);
      nvicEnableVector(TIVA_PWM0FAULT_NUMBER,
                       TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM0GEN0_NUMBER, TIVA_PWM_PWM0_0_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM0GEN1_NUMBER, TIVA_PWM_PWM0_1_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM0GEN2_NUMBER, TIVA_PWM_PWM0_2_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM0GEN3_NUMBER, TIVA_PWM_PWM0_3_IRQ_PRIORITY);
    }
#endif

#if TIVA_PWM_USE_PWM1
    if (&PWMD2 == pwmp) {
      SYSCTL->RCGCPWM |= (1 << 1);
      nvicEnableVector(TIVA_PWM1FAULT_NUMBER,
                       TIVA_PWM_PWM1_FAULT_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM1GEN0_NUMBER, TIVA_PWM_PWM1_0_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM1GEN1_NUMBER, TIVA_PWM_PWM1_1_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM1GEN2_NUMBER, TIVA_PWM_PWM1_2_IRQ_PRIORITY);
      nvicEnableVector(TIVA_PWM1GEN3_NUMBER, TIVA_PWM_PWM1_3_IRQ_PRIORITY);
    }
#endif
  }
  else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    pwmp->pwm->PWM[0].CTL = 0;
    pwmp->pwm->PWM[1].CTL = 0;
    pwmp->pwm->PWM[2].CTL = 0;
    pwmp->pwm->PWM[3].CTL = 0;
  }

  /* Timer configuration.*/
  for (i = 0; i  < (PWM_CHANNELS >> 1); i++) {
    pwmp->pwm->PWM[i].CTL = 0;
    pwmp->pwm->PWM[i].GEN[0] = 0x08C;
    pwmp->pwm->PWM[i].GEN[1] = 0x80C;
    pwmp->pwm->PWM[i].LOAD = (uint16_t)(pwmp->config->frequency - 1);
    pwmp->pwm->PWM[i].CMP[0] = (uint16_t)(pwmp->period - 1);
    pwmp->pwm->PWM[i].CMP[1] = (uint16_t)(pwmp->period - 1);
  }

  /* Output enables and polarities setup.*/
  for (i = 0; i < PWM_CHANNELS; i++) {
    switch (pwmp->config->channels[i].mode & PWM_OUTPUT_MASK) {
    case PWM_OUTPUT_DISABLED:
      enable &= ~(1 << i);
      break;
    case PWM_OUTPUT_ACTIVE_LOW:
      invert |= (1 << i);
      enable |= (1 << i);
      break;
    case PWM_OUTPUT_ACTIVE_HIGH:
      invert &= ~(1 << i);
      enable |= (1 << i);
      break;
    default:
      ;
    }
  }

  pwmp->pwm->INVERT = invert;
  pwmp->pwm->ENABLE = enable;
  pwmp->pwm->ISC = 0xFFFFFFFF;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp)
{
  /* If in ready state then disables the PWM clock.*/
  if (pwmp->state == PWM_READY) {
    pwmp->pwm->PWM[0].CTL = 0;
    pwmp->pwm->PWM[1].CTL = 0;
    pwmp->pwm->PWM[2].CTL = 0;
    pwmp->pwm->PWM[3].CTL = 0;

#if TIVA_PWM_USE_PWM0
    if (&PWMD1 == pwmp) {
      nvicDisableVector(TIVA_PWM0FAULT_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN0_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN1_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN2_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN3_NUMBER);
      SYSCTL->RCGCPWM &= ~(1 << 0);
    }
#endif

#if TIVA_PWM_USE_PWM1
    if (&PWMD2 == pwmp) {
      nvicDisableVector(TIVA_PWM1FAULT_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN0_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN1_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN2_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN3_NUMBER);
      SYSCTL->RCGCPWM &= ~(1 << 1);
    }
#endif
  }
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    The function has effect at the next cycle start.
 * @note    Channel notification is not enabled.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width)
{
  /* Changing channel duty cycle on the fly.*/
  pwmp->pwm->PWM[channel >> 1].CMP[channel & 1] = width;
  pwmp->pwm->PWM[channel >> 1].CTL |= (1 << 0);
}

/**
 * @brief   Disables a PWM channel and its notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel)
{
  pwmp->pwm->PWM[channel >> 1].CMP[channel & 1] = 0;
  pwmp->pwm->PWM[channel >> 1].CTL &= ~(1 << 0);
}

/**
 * @brief   Enables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_enable_periodic_notification(PWMDriver *pwmp)
{
  uint32_t inten;
  uint8_t i;

  /* If the IRQ is not already enabled care must be taken to clear it,
     it is probably already pending because the timer is running.*/
  for(i = 0; i < (PWM_CHANNELS >> 1); i++) {
    inten = pwmp->pwm->PWM[i].INTEN;
    if ((inten & 0x03) == 0) {
      pwmp->pwm->PWM[i].INTEN |= 0x03;
      pwmp->pwm->PWM[i].ISC = 0x03;
    }
  }

  pwmp->pwm->INTEN = 0x3f;
}

/**
 * @brief   Disables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_disable_periodic_notification(PWMDriver *pwmp)
{
  pwmp->pwm->PWM[0].INTEN &= ~(0x03);
  pwmp->pwm->PWM[1].INTEN &= ~(0x03);
  pwmp->pwm->PWM[2].INTEN &= ~(0x03);
  pwmp->pwm->PWM[3].INTEN &= ~(0x03);
  pwmp->pwm->INTEN &= ~(0x3F);
}

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_enable_channel_notification(PWMDriver *pwmp,
                                         pwmchannel_t channel)
{
  uint32_t inten = pwmp->pwm->PWM[channel >> 1].INTEN;

  /* If the IRQ is not already enabled care must be taken to clear it,
     it is probably already pending because the timer is running.*/
  if ((inten & (0x03 << (((channel & 1) * 2) + 2))) == 0) {
    pwmp->pwm->PWM[channel >> 1].INTEN |= (0x03 << (((channel & 1) * 2) + 2));
    pwmp->pwm->PWM[channel >> 1].ISC = (0x03 << (((channel & 1) * 2) + 2));
  }
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel_notification(PWMDriver *pwmp,
                                          pwmchannel_t channel)
{
  pwmp->pwm->PWM[channel >> 1].INTEN &= ~(0x03 << (((channel & 1) * 2) + 2));
}

#endif /* HAL_USE_PWM */

/**
 * @}
 */
