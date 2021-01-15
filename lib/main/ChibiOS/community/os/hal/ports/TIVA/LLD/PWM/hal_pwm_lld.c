/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    PWM/hal_pwm_lld.c
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

static uint32_t pwm_generator_offsets[] = { PWM_GEN_0_OFFSET, PWM_GEN_1_OFFSET, PWM_GEN_2_OFFSET, PWM_GEN_3_OFFSET};

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
  uint32_t pwm = pwmp->pwm;

  isc = HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_ISC);
  HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_ISC) = isc;

  if (((isc & PWM_X_ISC_INTCMPAD) != 0) &&
      (pwmp->config->channels[i * 2 + 0].callback != NULL)) {
    pwmp->config->channels[i * 2 + 0].callback(pwmp);
  }

  if (((isc & PWM_X_ISC_INTCMPAU) != 0) &&
      (pwmp->config->channels[i * 2 + 0].callback != NULL)) {
    pwmp->config->channels[i * 2 + 0].callback(pwmp);
  }

  if (((isc & PWM_X_ISC_INTCMPBD) != 0) &&
      (pwmp->config->channels[i * 2 + 1].callback != NULL)) {
    pwmp->config->channels[i * 2 + 1].callback(pwmp);
  }

  if (((isc & PWM_X_ISC_INTCMPBU) != 0) &&
      (pwmp->config->channels[i * 2 + 1].callback != NULL)) {
    pwmp->config->channels[i * 2 + 1].callback(pwmp);
  }

  if (((isc & PWM_X_ISC_INTCNTLOAD) != 0) && (pwmp->config->callback != NULL)) {
    pwmp->config->callback(pwmp);
  }

  if (((isc & PWM_X_ISC_INTCNTZERO) != 0) && (pwmp->config->callback != NULL)) {
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
  PWMD1.pwm = PWM0_BASE;
#endif

#if TIVA_PWM_USE_PWM1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = PWM_CHANNELS;
  PWMD2.pwm = PWM1_BASE;
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
  uint32_t pwm = pwmp->pwm;

  if (pwmp->state == PWM_STOP) {
    /* Clock activation.*/
#if TIVA_PWM_USE_PWM0
    if (&PWMD1 == pwmp) {
      HWREG(SYSCTL_RCGCPWM) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRPWM) & (1 << 0)))
        ;

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
      HWREG(SYSCTL_RCGCPWM) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRPWM) & (1 << 1)))
        ;

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
    HWREG(pwm + PWM_O_0_CTL) = 0;
    HWREG(pwm + PWM_O_1_CTL) = 0;
    HWREG(pwm + PWM_O_2_CTL) = 0;
    HWREG(pwm + PWM_O_3_CTL) = 0;
  }

  /* Timer configuration.*/
  for (i = 0; i  < (PWM_CHANNELS >> 1); i++) {
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_CTL) = 0;
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_GENA) = 0x08C;
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_GENB) = 0x80C;
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_LOAD) = (uint16_t)(pwmp->config->frequency - 1);
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_CMPA) = (uint16_t)(pwmp->period - 1);
    HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_CMPB) = (uint16_t)(pwmp->period - 1);
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

  HWREG(pwm + PWM_O_INVERT) = invert;
  HWREG(pwm + PWM_O_ENABLE) = enable;
  HWREG(pwm + PWM_O_ISC) = 0xFFFFFFFF;
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
  uint32_t pwm = pwmp->pwm;

  /* If in ready state then disables the PWM clock.*/
  if (pwmp->state == PWM_READY) {
    HWREG(pwm + PWM_O_0_CTL) = 0;
    HWREG(pwm + PWM_O_1_CTL) = 0;
    HWREG(pwm + PWM_O_2_CTL) = 0;
    HWREG(pwm + PWM_O_3_CTL) = 0;

#if TIVA_PWM_USE_PWM0
    if (&PWMD1 == pwmp) {
      nvicDisableVector(TIVA_PWM0FAULT_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN0_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN1_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN2_NUMBER);
      nvicDisableVector(TIVA_PWM0GEN3_NUMBER);
      HWREG(SYSCTL_RCGCPWM) &= ~(1 << 0);
    }
#endif

#if TIVA_PWM_USE_PWM1
    if (&PWMD2 == pwmp) {
      nvicDisableVector(TIVA_PWM1FAULT_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN0_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN1_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN2_NUMBER);
      nvicDisableVector(TIVA_PWM1GEN3_NUMBER);
      HWREG(SYSCTL_RCGCPWM) &= ~(1 << 1);
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
  uint32_t pwm = pwmp->pwm;

  /* Changing channel duty cycle on the fly.*/
  if (channel & 1)
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CMPB) = width;
  else
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CMPA) = width;


  HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CTL) = (1 << 0);
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
  uint32_t pwm = pwmp->pwm;

  if (channel & 1)
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CMPB) = 0;
  else
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CMPA) = 0;

  HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_CTL) = (1 << 0);
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
  uint32_t pwm = pwmp->pwm;

  /* If the IRQ is not already enabled care must be taken to clear it,
     it is probably already pending because the timer is running.*/
  for(i = 0; i < (PWM_CHANNELS >> 1); i++) {
    inten = HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_INTEN);
    if ((inten & 0x03) == 0) {
      HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_INTEN) |= 0x03;
      HWREG(pwm + pwm_generator_offsets[i] + PWM_O_X_ISC) = 0x03;
    }
  }

  HWREG(pwm + PWM_O_INTEN) = 0x3f;
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
  uint32_t pwm = pwmp->pwm;

  HWREG(pwm + PWM_O_0_INTEN) = ~(0x03);
  HWREG(pwm + PWM_O_1_INTEN) = ~(0x03);
  HWREG(pwm + PWM_O_2_INTEN) = ~(0x03);
  HWREG(pwm + PWM_O_3_INTEN) = ~(0x03);

  HWREG(pwm + PWM_O_INTEN) &= ~(0x3F);
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
  uint32_t pwm = pwmp->pwm;
  uint32_t inten = HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_ISC);

  /* If the IRQ is not already enabled care must be taken to clear it,
     it is probably already pending because the timer is running.*/
  if ((inten & (0x03 << (((channel & 1) * 2) + 2))) == 0) {
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_INTEN) |= (0x03 << (((channel & 1) * 2) + 2));
    HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_ISC) = (0x03 << (((channel & 1) * 2) + 2));
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
  uint32_t pwm = pwmp->pwm;

  HWREG(pwm + pwm_generator_offsets[channel >> 1] + PWM_O_X_INTEN) = ~(0x03 << (((channel & 1) * 2) + 2));
}

#endif /* HAL_USE_PWM */

/**
 * @}
 */
