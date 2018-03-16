/*
    ChibiOS - Copyright (C) 2014 Adam J. Porter

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
 * @file    KL2x/pwm_lld.c
 * @brief   KINETIS PWM subsystem low level driver source.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define KINETIS_TPM0_CHANNELS 6
#define KINETIS_TPM1_CHANNELS 2
#define KINETIS_TPM2_CHANNELS 2

#define KINETIS_TPM0_HANDLER  Vector84
#define KINETIS_TPM1_HANDLER  Vector88
#define KINETIS_TPM2_HANDLER  Vector8C

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 allocates the timer TPM0 when enabled.
 */
#if KINETIS_PWM_USE_TPM0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the timer TPM1 when enabled.
 */
#if KINETIS_PWM_USE_TPM1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the timer TPM2 when enabled.
 */
#if KINETIS_PWM_USE_TPM2 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {
  uint32_t sr;

  sr = pwmp->tpm->STATUS;
  pwmp->tpm->STATUS = 0xFFFFFFFF;

  if (((sr & TPM_SC_TOF) != 0) &&
      (pwmp->config->callback != NULL))
    pwmp->config->callback(pwmp);
  if (((sr & TPM_STATUS_CH0F) != 0) &&
      (pwmp->config->channels[0].callback != NULL))
    pwmp->config->channels[0].callback(pwmp);
  if (((sr & TPM_STATUS_CH1F) != 0) &&
      (pwmp->config->channels[1].callback != NULL))
    pwmp->config->channels[1].callback(pwmp);
  if (((sr & TPM_STATUS_CH2F) != 0) &&
      (pwmp->config->channels[2].callback != NULL))
    pwmp->config->channels[2].callback(pwmp);
  if (((sr & TPM_STATUS_CH3F) != 0) &&
      (pwmp->config->channels[3].callback != NULL))
    pwmp->config->channels[3].callback(pwmp);
  if (((sr & TPM_STATUS_CH4F) != 0) &&
      (pwmp->config->channels[4].callback != NULL))
    pwmp->config->channels[4].callback(pwmp);
  if (((sr & TPM_STATUS_CH5F) != 0) &&
      (pwmp->config->channels[5].callback != NULL))
    pwmp->config->channels[5].callback(pwmp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_PWM_USE_TPM0
/**
 * @brief   TPM0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_TPM0_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_TPM0 */

#if KINETIS_PWM_USE_TPM1
/**
 * @brief   TPM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_TPM1_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_TPM1 */

#if KINETIS_PWM_USE_TPM2
/**
 * @brief   TPM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_TPM2_HANDLER) {

  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_TPM2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if KINETIS_PWM_USE_TPM0
  pwmObjectInit(&PWMD1);
  PWMD1.channels = KINETIS_TPM0_CHANNELS;
  PWMD1.tpm = TPM0;
#endif

#if KINETIS_PWM_USE_TPM1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = KINETIS_TPM1_CHANNELS;
  PWMD2.tpm = TPM1;
#endif

#if KINETIS_PWM_USE_TPM2
  pwmObjectInit(&PWMD3);
  PWMD3.channels = KINETIS_TPM2_CHANNELS;
  PWMD3.tpm = TPM2;
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
void pwm_lld_start(PWMDriver *pwmp) {
  uint32_t psc;
  int i;

  if (pwmp->state == PWM_STOP) {
    /* Clock activation and timer reset.*/
#if KINETIS_PWM_USE_TPM0
    if (&PWMD1 == pwmp) {
      SIM->SCGC6 |= SIM_SCGC6_TPM0;
      nvicEnableVector(TPM0_IRQn, KINETIS_PWM_TPM0_IRQ_PRIORITY);
    }
#endif

#if KINETIS_PWM_USE_TPM1
    if (&PWMD2 == pwmp) {
      SIM->SCGC6 |= SIM_SCGC6_TPM1;
      nvicEnableVector(TPM1_IRQn, KINETIS_PWM_TPM1_IRQ_PRIORITY);
    }
#endif

#if KINETIS_PWM_USE_TPM2
    if (&PWMD3 == pwmp) {
      SIM->SCGC6 |= SIM_SCGC6_TPM2;
      nvicEnableVector(TPM2_IRQn, KINETIS_PWM_TPM2_IRQ_PRIORITY);
    }
#endif
  }

  /* Disable LPTPM counter.*/
  pwmp->tpm->SC = 0;
  /* Clear count register.*/
  pwmp->tpm->CNT = 0;

  /* Prescaler value calculation.*/
  psc = (KINETIS_SYSCLK_FREQUENCY / pwmp->config->frequency);
  /* Prescaler must be power of two between 1 and 128.*/
  osalDbgAssert(psc <= 128 && !(psc & (psc - 1)), "invalid frequency");
  /* Prescaler register value determination.
     Prescaler register value conveniently corresponds to bit position,
     i.e., register value for prescaler CLK/64 is 6 ((1 << 6) == 64).*/
  for (i = 0; i < 8; i++) {
    if (psc == (1UL << i)) {
      break;
    }
  }
  /* Set prescaler and clock mode.
     This also sets the following:
          CPWM up-counting mode
          Timer overflow interrupt disabled
          DMA disabled.*/
  pwmp->tpm->SC = TPM_SC_CMOD_LPTPM_CLK | i;
  /* Configure period.*/
  pwmp->tpm->MOD = pwmp->period - 1;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

  /* If in ready state then disables the PWM clock.*/
  if (pwmp->state == PWM_READY) {
#if KINETIS_PWM_USE_TPM0
    if (&PWMD1 == pwmp) {
      SIM->SCGC6 &= ~SIM_SCGC6_TPM0;
      nvicDisableVector(TPM0_IRQn);
    }
#endif

#if KINETIS_PWM_USE_TPM1
    if (&PWMD2 == pwmp) {
      SIM->SCGC6 &= ~SIM_SCGC6_TPM1;
      nvicDisableVector(TPM1_IRQn);
    }
#endif

#if KINETIS_PWM_USE_TPM2
    if (&PWMD3 == pwmp) {
      SIM->SCGC6 &= ~SIM_SCGC6_TPM2;
      nvicDisableVector(TPM2_IRQn);
    }
#endif
    /* Disable LPTPM counter.*/
    pwmp->tpm->SC = 0;
    pwmp->tpm->MOD = 0;
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
                            pwmcnt_t width) {
  uint32_t mode = TPM_CnSC_MSB; /* Edge-aligned PWM mode.*/

  switch (pwmp->config->channels[channel].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_HIGH:
    mode |= TPM_CnSC_ELSB;
    break;
  case PWM_OUTPUT_ACTIVE_LOW:
    mode |= TPM_CnSC_ELSA;
    break;
  }

  if (pwmp->tpm->C[channel].SC & TPM_CnSC_CHIE)
    mode |= TPM_CnSC_CHIE;

  pwmp->tpm->C[channel].SC = mode;
  pwmp->tpm->C[channel].V = width;
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
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {

  pwmp->tpm->C[channel].SC = 0;
  pwmp->tpm->C[channel].V = 0;
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
void pwm_lld_enable_periodic_notification(PWMDriver *pwmp) {

  pwmp->tpm->SC |= TPM_SC_TOIE;
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
void pwm_lld_disable_periodic_notification(PWMDriver *pwmp) {

  pwmp->tpm->SC &= ~TPM_SC_TOIE;
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
                                         pwmchannel_t channel) {

  pwmp->tpm->C[channel].SC |= TPM_CnSC_CHIE;
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
                                          pwmchannel_t channel) {

  pwmp->tpm->C[channel].SC &= ~TPM_CnSC_CHIE;
}

#endif /* HAL_USE_PWM */

/** @} */
