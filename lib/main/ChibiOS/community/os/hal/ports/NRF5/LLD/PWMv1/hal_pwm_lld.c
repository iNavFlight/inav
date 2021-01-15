/*
    ChibiOS/HAL - Copyright (C) 2016 Stéphane D'Alu

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
 * @file    PWMv1/hal_pwm_lld.c
 * @brief   NRF51 PWM subsystem low level driver source.
 *
 * @note    Using the method described in nrf51-pwm-library to correctly
 *          handle toggling of the pin with GPIOTE when changing period.
 *          It means it is generally unsafe to use GPIOTE with a period
 *          less than (2 * PWM_GPIOTE_DECISION_TIME / 16MHz)
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define PWM_GPIOTE_PPI_CC 3
#define PWM_GPIOTE_DECISION_TIME 160

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 allocates the timer TIMER0 when enabled.
 */
#if NRF5_PWM_USE_TIMER0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the timer TIMER1 when enabled.
 */
#if NRF5_PWM_USE_TIMER1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the timer TIMER2 when enabled.
 */
#if NRF5_PWM_USE_TIMER2 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const uint8_t pwm_margin_by_prescaler[] = {
  (PWM_GPIOTE_DECISION_TIME +   0) >> 0,
  (PWM_GPIOTE_DECISION_TIME +   1) >> 1,
  (PWM_GPIOTE_DECISION_TIME +   3) >> 2,
  (PWM_GPIOTE_DECISION_TIME +   7) >> 3,
  (PWM_GPIOTE_DECISION_TIME +  15) >> 4,
  (PWM_GPIOTE_DECISION_TIME +  31) >> 5,
  (PWM_GPIOTE_DECISION_TIME +  63) >> 6,
  (PWM_GPIOTE_DECISION_TIME + 127) >> 7,
  (PWM_GPIOTE_DECISION_TIME + 255) >> 8,
  (PWM_GPIOTE_DECISION_TIME + 511) >> 9
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {
  uint8_t channel;
  /* Deal with PWM channels
   */
  for (channel = 0 ; channel < pwmp->channels ; channel++) {
    if (pwmp->timer->EVENTS_COMPARE[channel]) {
      pwmp->timer->EVENTS_COMPARE[channel] = 0;

      if (pwmp->config->channels[channel].callback != NULL) {
        pwmp->config->channels[channel].callback(pwmp);
      }
    }      
  }

  /* Deal with PWM period
   */
  if (pwmp->timer->EVENTS_COMPARE[pwmp->channels]) {
    pwmp->timer->EVENTS_COMPARE[pwmp->channels] = 0;

    if (pwmp->config->callback != NULL) {
      pwmp->config->callback(pwmp);
    }
  }
}

static inline
bool pwm_within_safe_margins(PWMDriver *pwmp, uint32_t timer, uint32_t width)  {
  const uint32_t margin = pwm_margin_by_prescaler[pwmp->timer->PRESCALER];
  return (width <= margin)
       ? ((width <= timer) && (timer < (pwmp->period + width - margin)))
       : ((width <= timer) || (timer < (width - margin)));
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_PWM_USE_TIMER0
/**
 * @brief   TIMER0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector60) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_TIMER0 */

#if NRF5_PWM_USE_TIMER1
/**
 * @brief   TIMER1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector64) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_TIMER1 */

#if NRF5_PWM_USE_TIMER2
/**
 * @brief   TIMER2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector68) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_PWM_USE_TIMER2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if NRF5_PWM_USE_TIMER0
  pwmObjectInit(&PWMD1);
  PWMD1.channels = PWM_CHANNELS;
  PWMD1.timer = NRF_TIMER0;
#endif

#if NRF5_PWM_USE_TIMER1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = PWM_CHANNELS;
  PWMD2.timer = NRF_TIMER1;
#endif

#if NRF5_PWM_USE_TIMER2
  pwmObjectInit(&PWMD3);
  PWMD3.channels = PWM_CHANNELS;
  PWMD3.timer = NRF_TIMER2;
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
  uint16_t psc_ratio = NRF5_HFCLK_FREQUENCY / pwmp->config->frequency;
  /* Prescaler ratio must be between 1 and 512, and a power of two. */
  osalDbgAssert(psc_ratio <= 512 && !(psc_ratio & (psc_ratio - 1)),
                "invalid frequency");
  /* Prescaler value as a power of 2, must be 0..9 */
  uint32_t psc_value;
  for (psc_value = 0; psc_value < 10; psc_value++)
    if (psc_ratio == (unsigned)(1 << psc_value))
      break;

  /* Configure as 16bits timer (only TIMER0 support 32bits) */
  pwmp->timer->BITMODE = TIMER_BITMODE_BITMODE_16Bit;
  pwmp->timer->MODE = TIMER_MODE_MODE_Timer;

  /* With clear shortcuts for period */
  pwmp->timer->SHORTS =
    0x1UL << (TIMER_SHORTS_COMPARE0_CLEAR_Pos + pwmp->channels);

  /* Disable and reset interrupts for compare events */
  pwmp->timer->INTENCLR = (TIMER_INTENCLR_COMPARE0_Msk |
                           TIMER_INTENCLR_COMPARE1_Msk |
                           TIMER_INTENCLR_COMPARE2_Msk |
                           TIMER_INTENCLR_COMPARE3_Msk );
  pwmp->timer->EVENTS_COMPARE[0] = 0;
  pwmp->timer->EVENTS_COMPARE[1] = 0;
  pwmp->timer->EVENTS_COMPARE[2] = 0;
  pwmp->timer->EVENTS_COMPARE[3] = 0;

  /* Set prescaler */
  pwmp->timer->PRESCALER = psc_value;

  /* Set period */
  pwmp->timer->CC[pwmp->channels] = pwmp->period; 

  /* Clear everything */
  pwmp->timer->TASKS_CLEAR = 1;

  /* Enable interrupt */
#if NRF5_PWM_USE_TIMER0
  if (&PWMD1 == pwmp) {
    nvicEnableVector(TIMER0_IRQn, NRF5_PWM_TIMER0_PRIORITY);
  }
#endif

#if NRF5_PWM_USE_TIMER1
  if (&PWMD2 == pwmp) {
    nvicEnableVector(TIMER1_IRQn, NRF5_PWM_TIMER1_PRIORITY);
  }
#endif

#if NRF5_PWM_USE_TIMER2
  if (&PWMD3 == pwmp) {
    nvicEnableVector(TIMER2_IRQn, NRF5_PWM_TIMER2_PRIORITY);
  }
#endif

  /* Start timer */
  pwmp->timer->TASKS_START = 1;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {
  pwmp->timer->TASKS_SHUTDOWN = 1;

#if NRF5_PWM_USE_TIMER0
  if (&PWMD1 == pwmp) {
    nvicDisableVector(TIMER0_IRQn);
  }
#endif

#if NRF5_PWM_USE_TIMER1
  if (&PWMD2 == pwmp) {
    nvicDisableVector(TIMER1_IRQn);
  }
#endif

#if NRF5_PWM_USE_TIMER2
  if (&PWMD3 == pwmp) {
    nvicDisableVector(TIMER2_IRQn);
  }
#endif
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
#if NRF5_PWM_USE_GPIOTE_PPI
  const PWMChannelConfig *cfg_channel = &pwmp->config->channels[channel];
  const uint8_t  gpiote_channel = cfg_channel->gpiote_channel;
  const uint8_t *ppi_channel    = cfg_channel->ppi_channel;
  
  uint32_t outinit;
  switch(cfg_channel->mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW : outinit = GPIOTE_CONFIG_OUTINIT_Low;  break;
  case PWM_OUTPUT_ACTIVE_HIGH: outinit = GPIOTE_CONFIG_OUTINIT_High; break;
  case PWM_OUTPUT_DISABLED   : /* fall-through */
  default                    : goto no_output_config;
  }

  /* Deal with corner case: 0% and 100% */
  if ((width <= 0) || (width >= pwmp->period)) {
      /* Disable GPIOTE/PPI task */
    NRF_GPIOTE->CONFIG[gpiote_channel] = GPIOTE_CONFIG_MODE_Disabled;
    NRF_PPI->CHENCLR = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));
    /* Set Line */
    palWriteLine(cfg_channel->ioline,
           ((width <= 0) ^
            ((cfg_channel->mode & PWM_OUTPUT_MASK) == PWM_OUTPUT_ACTIVE_HIGH)));

  /* Really doing PWM */
  } else {
    const uint32_t gpio_pin = PAL_PAD(cfg_channel->ioline);
    const uint32_t polarity = GPIOTE_CONFIG_POLARITY_Toggle;

    /* Program tasks (one for duty cycle, one for periode) */
    NRF_PPI->CH[ppi_channel[0]].EEP =
      (uint32_t)&pwmp->timer->EVENTS_COMPARE[channel];
    NRF_PPI->CH[ppi_channel[0]].TEP =
      (uint32_t)&NRF_GPIOTE->TASKS_OUT[gpiote_channel];
    NRF_PPI->CH[ppi_channel[1]].EEP =
      (uint32_t)&pwmp->timer->EVENTS_COMPARE[pwmp->channels];
    NRF_PPI->CH[ppi_channel[1]].TEP =
      (uint32_t)&NRF_GPIOTE->TASKS_OUT[gpiote_channel];
    NRF_PPI->CHENSET = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));

    /* Something Old, something New */
    const uint32_t old_width = pwmp->timer->CC[channel];
    const uint32_t new_width = width;

    /* Check GPIOTE state */
    const bool gpiote = (NRF_GPIOTE->CONFIG[gpiote_channel] &
                   GPIOTE_CONFIG_MODE_Msk) != GPIOTE_CONFIG_MODE_Disabled;

    /* GPIOTE is currently running */
    if (gpiote) {
      uint32_t current;
      while (true) {
        pwmp->timer->TASKS_CAPTURE[PWM_GPIOTE_PPI_CC] = 1;
        current = pwmp->timer->CC[PWM_GPIOTE_PPI_CC];

        if (pwm_within_safe_margins(pwmp, current, old_width) &&
            pwm_within_safe_margins(pwmp, current, new_width))
          break;
      }
      if (((old_width <= current) && (current < new_width)) ||
          ((new_width <= current) && (current < old_width))) {
        NRF_GPIOTE->TASKS_OUT[gpiote_channel] = 1;
      }

    /* GPIOTE need to be restarted */
    } else {
      /* Create GPIO Task */
      NRF_GPIOTE->CONFIG[gpiote_channel] =
        (GPIOTE_CONFIG_MODE_Task << GPIOTE_CONFIG_MODE_Pos) |
        ((gpio_pin << GPIOTE_CONFIG_PSEL_Pos    ) & GPIOTE_CONFIG_PSEL_Msk    )|
        ((polarity << GPIOTE_CONFIG_POLARITY_Pos) & GPIOTE_CONFIG_POLARITY_Msk)|
        ((outinit  << GPIOTE_CONFIG_OUTINIT_Pos ) & GPIOTE_CONFIG_OUTINIT_Msk );

      pwmp->timer->TASKS_CAPTURE[PWM_GPIOTE_PPI_CC] = 1;
      if (pwmp->timer->CC[PWM_GPIOTE_PPI_CC] > width)
        NRF_GPIOTE->TASKS_OUT[gpiote_channel] = 1;
    }
  }

 no_output_config:
#endif

  pwmp->timer->CC[channel] = width;
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
  pwmp->timer->CC[channel] = 0;
#if NRF5_PWM_USE_GPIOTE_PPI
  const PWMChannelConfig *cfg_channel = &pwmp->config->channels[channel];
  switch(cfg_channel->mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_LOW:
  case PWM_OUTPUT_ACTIVE_HIGH: {
    const uint8_t  gpiote_channel = cfg_channel->gpiote_channel;
    const uint8_t *ppi_channel    = cfg_channel->ppi_channel;
    NRF_PPI->CHENCLR = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));
    NRF_GPIOTE->CONFIG[gpiote_channel] = GPIOTE_CONFIG_MODE_Disabled;
    break;
  }
  case PWM_OUTPUT_DISABLED:
  default:
    break;
  }
#endif
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
  pwmp->timer->INTENSET =
    0x1UL << (TIMER_INTENSET_COMPARE0_Pos + pwmp->channels);
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
  pwmp->timer->INTENCLR =
    0x1UL << (TIMER_INTENCLR_COMPARE0_Pos + pwmp->channels);
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
  pwmp->timer->INTENSET =
    0x1UL << (TIMER_INTENSET_COMPARE0_Pos + channel);
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
  pwmp->timer->INTENCLR =
    0x1UL << (TIMER_INTENCLR_COMPARE0_Pos + channel);
}

#endif /* HAL_USE_PWM */

/** @} */
