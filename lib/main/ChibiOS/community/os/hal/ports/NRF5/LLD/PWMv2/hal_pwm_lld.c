/*
    ChibiOS/HAL - Copyright (C) 2018 Andru

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
 * @file    hal_pwm_lld.c
 * @brief   NRF52 PWM subsystem low level driver source.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
uint16_t pwm_seq[PWM_CHANNELS];

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 enabled.
 */
#if NRF5_PWM_USE_PWM0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 enabled.
 */
#if NRF5_PWM_USE_PWM1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3  enabled.
 */
#if NRF5_PWM_USE_PWM2 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {
  /* Deal with PWM period
   */
  if (pwmp->config->callback == NULL) {
	  return;
  }

  if ((pwmp->pwm->INTEN & PWM_INTEN_PWMPERIODEND_Msk) && (pwmp->pwm->EVENTS_PWMPERIODEND)) {
      pwmp->config->callback(pwmp);
	  pwmp->pwm->EVENTS_PWMPERIODEND = 0;
	  (void)pwmp->pwm->EVENTS_PWMPERIODEND;
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_PWM_USE_PWM0
/**
 * @brief   PWM0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorB0) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_PWM0 */

#if NRF5_PWM_USE_PWM1
/**
 * @brief   PWM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorC4) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_PWM1 */

#if NRF5_PWM_USE_PWM2
/**
 * @brief   PWM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorC8) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_PWM2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if NRF5_PWM_USE_PWM0
  pwmObjectInit(&PWMD1);
  PWMD1.channels = PWM_CHANNELS;
  PWMD1.pwm = NRF_PWM0;
#endif

#if NRF5_PWM_USE_PWM1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = PWM_CHANNELS;
  PWMD2.pwm = NRF_PWM1;
#endif

#if NRF5_PWM_USE_PWM2
  pwmObjectInit(&PWMD3);
  PWMD3.channels = PWM_CHANNELS;
  PWMD3.pwm = NRF_PWM2;
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
  /* Prescaler value calculation: ftimer = 16MHz / 2^PRESCALER */
  /* Prescaler value as a power of 2, must be 0..7 */
  uint8_t i, psc_value;
  for (psc_value = 0; psc_value < 8; psc_value++)
    if (pwmp->config->frequency == (uint32_t)(16000000 >> psc_value))
      break;

  /* Prescaler value must be between 0..7, and a power of two. */
  osalDbgAssert(psc_value <= 7, "invalid frequency");

  /* Set PWM output lines */
  for (i=0; i<PWM_CHANNELS; i++) {
	const PWMChannelConfig *cfg_channel = &pwmp->config->channels[i];
	uint32_t gpio_pin = PAL_PAD(cfg_channel->ioline);
	if (cfg_channel->mode == PWM_OUTPUT_DISABLED) {
		gpio_pin = PAL_NOLINE;
	}

	pwmp->pwm->PSEL.OUT[i] = gpio_pin << PWM_PSEL_OUT_PIN_Pos;
  }

  /* Enable PWM */
  pwmp->pwm->ENABLE = PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos;

  /* Set mode */
  pwmp->pwm->MODE = PWM_MODE_UPDOWN_Up & PWM_MODE_UPDOWN_Msk;

  /* Set prescaler */
  pwmp->pwm->PRESCALER = psc_value & PWM_PRESCALER_PRESCALER_Msk;

  /* Set period */
  pwmp->pwm->COUNTERTOP = pwmp->period & PWM_COUNTERTOP_COUNTERTOP_Msk;

  pwmp->pwm->LOOP      = PWM_LOOP_CNT_Disabled & PWM_LOOP_CNT_Msk;
  pwmp->pwm->DECODER   = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) |
                         (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

  pwmp->pwm->SEQ[0].PTR  = ((uint32_t)(pwm_seq) << PWM_SEQ_PTR_PTR_Pos);
  pwmp->pwm->SEQ[0].CNT  = ((sizeof(pwm_seq) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
  pwmp->pwm->SEQ[0].REFRESH = 0;
  pwmp->pwm->SEQ[0].ENDDELAY = 0;

  pwmp->pwm->SEQ[1].PTR  = ((uint32_t)(pwm_seq) << PWM_SEQ_PTR_PTR_Pos);
  pwmp->pwm->SEQ[1].CNT  = ((sizeof(pwm_seq) / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos);
  pwmp->pwm->SEQ[1].REFRESH = 0;
  pwmp->pwm->SEQ[1].ENDDELAY = 0;

  /* With clear shortcuts for period */
  pwmp->pwm->SHORTS = 0;

  /* Disable and reset interrupts */
  pwmp->pwm->INTEN = 0;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {
#if NRF5_PWM_USE_PWM0
  if (&PWMD1 == pwmp) {
    nvicDisableVector(PWM0_IRQn);
  }
#endif

#if NRF5_PWM_USE_PWM1
  if (&PWMD2 == pwmp) {
    nvicDisableVector(PWM1_IRQn);
  }
#endif

#if NRF5_PWM_USE_PWM2
  if (&PWMD3 == pwmp) {
    nvicDisableVector(PWM2_IRQn);
  }
#endif

  /* Stop PWM generation */
  pwmp->pwm->TASKS_STOP = 1;

  /* Disable PWM */
  pwmp->pwm->ENABLE = (PWM_ENABLE_ENABLE_Disabled << PWM_ENABLE_ENABLE_Pos);
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
  const PWMChannelConfig *cfg_channel = &pwmp->config->channels[channel];
  
  /* Deal with corner case: 0% and 100% */
  if ((width <= 0) || (width >= pwmp->period)) {
	pwm_seq[channel] = pwmp->period & PWM_COUNTERTOP_COUNTERTOP_Msk;
	if (cfg_channel->mode == PWM_OUTPUT_ACTIVE_LOW) pwm_seq[channel] |= 0x8000;
  /* Really doing PWM */
  } else {
    pwm_seq[channel] = width & PWM_COUNTERTOP_COUNTERTOP_Msk;
    if (cfg_channel->mode == PWM_OUTPUT_ACTIVE_HIGH) pwm_seq[channel] |= 0x8000;
  }

  pwmp->pwm->EVENTS_STOPPED = 0;
  (void)pwmp->pwm->EVENTS_STOPPED;

  pwmp->pwm->TASKS_SEQSTART[0] = 1;
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
  const PWMChannelConfig *cfg_channel = &pwmp->config->channels[channel];

  pwm_seq[channel] = pwmp->period & PWM_COUNTERTOP_COUNTERTOP_Msk;
  if (cfg_channel->mode == PWM_OUTPUT_ACTIVE_LOW) pwm_seq[channel] |= 0x8000;

  pwmp->pwm->EVENTS_STOPPED = 0;
  (void)pwmp->pwm->EVENTS_STOPPED;

  pwmp->pwm->TASKS_SEQSTART[0] = 1;
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

  /* Events clear */
  pwmp->pwm->EVENTS_LOOPSDONE = 0;
  pwmp->pwm->EVENTS_SEQEND[0] = 0;
  pwmp->pwm->EVENTS_SEQEND[1] = 0;
  pwmp->pwm->EVENTS_STOPPED = 0;
#if CORTEX_MODEL >= 4
  (void)pwmp->pwm->EVENTS_LOOPSDONE;
  (void)pwmp->pwm->EVENTS_SEQEND[0];
  (void)pwmp->pwm->EVENTS_SEQEND[1];
  (void)pwmp->pwm->EVENTS_STOPPED;
#endif

  pwmp->pwm->INTENSET = PWM_INTENSET_PWMPERIODEND_Msk;

  /* Enable interrupt */
#if NRF5_PWM_USE_PWM0
  if (&PWMD1 == pwmp) {
    nvicEnableVector(PWM0_IRQn, NRF5_PWM_PWM0_PRIORITY);
  }
#endif

#if NRF5_PWM_USE_PWM1
  if (&PWMD2 == pwmp) {
    nvicEnableVector(PWM1_IRQn, NRF5_PWM_PWM1_PRIORITY);
  }
#endif

#if NRF5_PWM_USE_PWM2
  if (&PWMD3 == pwmp) {
    nvicEnableVector(PWM2_IRQn, NRF5_PWM_PWM2_PRIORITY);
  }
#endif
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
  pwmp->pwm->INTENCLR = PWM_INTENCLR_PWMPERIODEND_Msk;
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
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp	pointer to a @p PWMDriver object
 * @param[in] channel	PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel_notification(PWMDriver *pwmp,
					  pwmchannel_t channel) {
}

#endif /* HAL_USE_PWM */

/** @} */
