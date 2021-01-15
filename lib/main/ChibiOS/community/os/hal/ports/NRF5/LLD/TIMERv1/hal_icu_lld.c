/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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

/*
 * Hardware Abstraction Layer for Extended Input Capture Unit
 */
#include "hal.h"

#if (HAL_USE_ICU == TRUE) || defined(__DOXYGEN__)

/**
 * @brief   Returns the compare value of the latest cycle.
 *
 * @param[in] chp       Pointer to channel structure that fired the interrupt.
 * @return              The number of ticks.
 *
 * @notapi
 */
//#define icu_lld_get_compare(chp)     (*((chp)->ccp) + 1)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ICUD1 driver identifier.
 * @note    The driver ICUD1 allocates the complex timer TIMER0 when enabled.
 */
#if NRF5_ICU_USE_TIMER0 && !defined(__DOXYGEN__)
ICUDriver ICUD1;
#endif

/**
 * @brief   ICUD2 driver identifier.
 * @note    The driver ICUD2 allocates the timer TIMER1 when enabled.
 */
#if NRF5_ICU_USE_TIMER1 && !defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD3 driver identifier.
 * @note    The driver ICUD3 allocates the timer TIMER2 when enabled.
 */
#if NRF5_ICU_USE_TIMER2 && !defined(__DOXYGEN__)
ICUDriver ICUD3;
#endif

/**
 * @brief   ICUD4 driver identifier.
 * @note    The driver ICUD4 allocates the timer TIMER3 when enabled.
 */
#if NRF5_ICU_USE_TIMER3 && !defined(__DOXYGEN__)
ICUDriver ICUD4;
#endif

/**
 * @brief   ICUD5 driver identifier.
 * @note    The driver ICUD5 allocates the timer TIMER4 when enabled.
 */
#if NRF5_ICU_USE_TIMER4 && !defined(__DOXYGEN__)
ICUDriver ICUD5;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief     Returns pulse width.
 * @details   The time is defined as number of ticks.
 *
 * @param[in] icup      Pointer to the ICUDriver object.
 * @param[in] channel   The timer channel that fired the interrupt.
 * @param[in] compare   Content of the CC register.
 * @return              The number of ticks.
 *
 * @notapi
 */
static icucnt_t get_time_width(const ICUDriver *icup,
                                uint8_t channel,
                                icucnt_t compare) {

  const ICUChannel *chp = &icup->channel[channel];

  /* Note! there is no overflow check because it handles under the hood of
     unsigned subtraction math.*/
  return compare - chp->last_idle;
}

/**
 * @brief     Returns pulse period.
 * @details   The time is defined as number of ticks.
 *
 * @param[in] icup      Pointer to the ICUDriver object.
 * @param[in] channel   The timer channel that fired the interrupt.
 * @param[in] compare   Content of the CC register.
 * @return              The number of ticks.
 *
 * @notapi
 */
static icucnt_t get_time_period(const ICUDriver *icup,
                                 uint8_t channel,
                                 icucnt_t compare) {

  const ICUChannel *chp = &icup->channel[channel];

  /* Note! there is no overflow check because it handles under the hood of
     unsigned subtraction math.*/
  return compare - chp->last_idle;
}

/**
 * @brief   ICU width event.
 *
 * @param[in] icup      Pointer to the @p ICUDriver object
 * @param[in] channel   The timer channel that fired the interrupt.
 *
 * @notapi
 */
static void _isr_invoke_width_cb(ICUDriver *icup, uint8_t channel) {
  ICUChannel *chp = &icup->channel[channel];
  icucnt_t compare = icup->timer->CC[channel+2];
  chp->last_active = compare;
  if (ICU_CH_ACTIVE == chp->state) {
    icup->result.width = get_time_width(icup, channel, compare);
    if ((icup->state == ICU_ACTIVE) && (icup->config->width_cb != NULL))
      icup->config->width_cb(icup);
    chp->state = ICU_CH_IDLE;
  }
}

/**
 * @brief   ICU period detect event.
 *
 * @param[in] icup      Pointer to the @p ICUDriver object
 * @param[in] channel   The timer channel that fired the interrupt.
 *
 * @notapi
 */
static void _isr_invoke_period_cb(ICUDriver *icup, uint8_t channel) {
  ICUChannel *chp = &icup->channel[channel];
  icucnt_t compare = (uint32_t)icup->timer->CC[channel];
  icup->result.period = get_time_period(icup, channel, compare);
  chp->last_idle = compare;
  chp->state = ICU_CH_ACTIVE;
  if ((icup->state == ICU_ACTIVE) && (icup->config->period_cb != NULL))
      icup->config->period_cb(icup);
  icup->state = ICU_ACTIVE;
  /* Set overflow timeout */
  icup->timer->CC[channel] = compare + ICU_WAIT_TIMEOUT;
}

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] icup     Pointer to the @p ICUDriver object
 */
void icu_lld_serve_gpiote_interrupt(ICUDriver *icup) {
  uint8_t ch;
  for (ch=0; ch<ICU_CHANNELS; ch++) {
	const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[ch];
	const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;

	/* Period event */
	if (NRF_GPIOTE->INTENSET & (1 << gpiote_channel[0]) && NRF_GPIOTE->EVENTS_IN[gpiote_channel[0]]) {
	  _isr_invoke_period_cb(icup, ch);
	  NRF_GPIOTE->EVENTS_IN[gpiote_channel[0]] = 0;
	  (void) NRF_GPIOTE->EVENTS_IN[gpiote_channel[0]];
	}
	/* Width event */
	if (NRF_GPIOTE->INTENSET & (1 << gpiote_channel[1]) && NRF_GPIOTE->EVENTS_IN[gpiote_channel[1]]) {
	  _isr_invoke_width_cb(icup, ch);
	  NRF_GPIOTE->EVENTS_IN[gpiote_channel[1]] = 0;
	  (void) NRF_GPIOTE->EVENTS_IN[gpiote_channel[1]];
	}
  }
}

/**
 * @brief   Overflow IRQ handler.
 *
 * @param[in] icup     Pointer to the @p ICUDriver object
 */
void icu_lld_serve_interrupt(ICUDriver *icup) {
  uint8_t ch;
  for (ch=0; ch<ICU_CHANNELS; ch++) {
	/* Clear overflow events  */
	if (icup->timer->INTENSET & (1 << (TIMER_INTENSET_COMPARE0_Pos + ch)) &&
		icup->timer->EVENTS_COMPARE[ch]) {
	  icup->timer->EVENTS_COMPARE[ch] = 0;
	  (void) icup->timer->EVENTS_COMPARE[ch];
	  /* Set next overlow compare */
	  icup->timer->CC[ch] = icup->timer->CC[ch] + ICU_WAIT_TIMEOUT;
	 }
  }
  if (icup->config->overflow_cb != NULL)
    icup->config->overflow_cb(icup);
  icup->state = ICU_WAITING;
}

/**
 * @brief   Starts every channel.
 *
 * @param[in]   icup     Pointer to the @p ICUDriver object
 *
 * @note        GPIO Line[0] -> GPIOTE channel[0] will detect start edge.
 * @note        GPIO Line[1] -> GPIOTE channel[1] will detect end edge.
 */
static void start_channels(ICUDriver *icup) {

  /* Set each input channel that is used as: a normal input capture channel. */
#if NRF5_ICU_USE_GPIOTE_PPI
  uint8_t channel;
  for (channel = 0; channel<ICU_CHANNELS; channel++) {
	const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	if (cfg_channel->mode == ICU_INPUT_DISABLED) continue;

	const uint32_t gpio_pin0 = PAL_PAD(cfg_channel->ioline[0]);
	const uint32_t gpio_pin1 = PAL_PAD(cfg_channel->ioline[1]);
	osalDbgAssert((gpio_pin0 < 32) &&
	              (gpio_pin1 < 32) &&
				  (gpio_pin0 != gpio_pin1),
	               "invalid Line configuration");

	/* NRF52832 GPIOTE channels 0..7 */
	const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;
	osalDbgAssert((gpiote_channel[0] < 8) &&
	              (gpiote_channel[1] < 8) &&
				  (gpiote_channel[0] != gpiote_channel[1]),
	               "invalid GPIOTE configuration");

	/* NRF52832 PPI channels 0..19 */
	const uint8_t *ppi_channel    = cfg_channel->ppi_channel;
	osalDbgAssert((gpiote_channel[0] < 20) &&
	              (gpiote_channel[1] < 20) &&
				  (gpiote_channel[0] != gpiote_channel[1]),
	               "invalid PPI configuration");

	/* Program PPI events for period */
	NRF_PPI->CH[ppi_channel[0]].EEP = (uint32_t) &NRF_GPIOTE->EVENTS_IN[gpiote_channel[0]];
	NRF_PPI->CH[ppi_channel[0]].TEP = (uint32_t) &icup->timer->TASKS_CAPTURE[channel];

	/* Program PPI events for width */
	NRF_PPI->CH[ppi_channel[1]].EEP = (uint32_t) &NRF_GPIOTE->EVENTS_IN[gpiote_channel[1]];
	NRF_PPI->CH[ppi_channel[1]].TEP = (uint32_t) &icup->timer->TASKS_CAPTURE[channel+2];

	/* Disable GPIOTE interrupts */
	NRF_GPIOTE->INTENCLR = (GPIOTE_INTENCLR_PORT_Clear << GPIOTE_INTENCLR_PORT_Pos) |
							(1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);
	NRF_GPIOTE->EVENTS_PORT = 1;

	/* Clear GPIOTE channels */
	NRF_GPIOTE->CONFIG[gpiote_channel[0]] &= ~(GPIOTE_CONFIG_PSEL_Msk | GPIOTE_CONFIG_POLARITY_Msk);
	NRF_GPIOTE->CONFIG[gpiote_channel[1]] &= ~(GPIOTE_CONFIG_PSEL_Msk | GPIOTE_CONFIG_POLARITY_Msk);

	/* Set GPIOTE channels */
	if (cfg_channel->mode == ICU_INPUT_ACTIVE_HIGH) {
	  NRF_GPIOTE->CONFIG[gpiote_channel[0]] =
    	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
	    ((gpio_pin0 << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PSEL_Msk) |
	    ((GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos) & GPIOTE_CONFIG_POLARITY_Msk);
	  NRF_GPIOTE->CONFIG[gpiote_channel[1]] =
    	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
	    ((gpio_pin1 << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PSEL_Msk) |
	    ((GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos) & GPIOTE_CONFIG_POLARITY_Msk);
	} else {
	  NRF_GPIOTE->CONFIG[gpiote_channel[0]] =
    	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
	    ((gpio_pin0 << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PSEL_Msk) |
	    ((GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos)
	     & GPIOTE_CONFIG_POLARITY_Msk);
	  NRF_GPIOTE->CONFIG[gpiote_channel[1]] =
    	(GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
	    ((gpio_pin1 << GPIOTE_CONFIG_PSEL_Pos) & GPIOTE_CONFIG_PSEL_Msk) |
	    ((GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos)
	     & GPIOTE_CONFIG_POLARITY_Msk);
	}
  }
#endif

}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_ICU_USE_TIMER0
/**
 * @brief   TIMER0 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector60) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_ICU_USE_TIMER0 */

#if NRF5_ICU_USE_TIMER1
/**
 * @brief   TIMER1 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector64) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_ICU_USE_TIMER1 */

#if NRF5_ICU_USE_TIMER2
/**
 * @brief   TIMER2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector68) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_ICU_USE_TIMER2 */

#if NRF5_ICU_USE_TIMER3
/**
 * @brief   TIMER3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorA8) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD4);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_ICU_USE_TIMER3 */

#if NRF5_ICU_USE_TIMER4
/**
 * @brief   TIMER4 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(VectorAC) {

  OSAL_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD5);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_ICU_USE_TIMER4 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ICU driver initialization.
 *
 * @notapi
 */
void icu_lld_init(void) {
#if NRF5_ICU_USE_TIMER0
  /* Driver initialization.*/
  icuObjectInit(&ICUD1);
  ICUD1.timer = NRF_TIMER0;
#endif

#if NRF5_ICU_USE_TIMER1
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.timer = NRF_TIMER1;
#endif

#if NRF5_ICU_USE_TIMER2
  /* Driver initialization.*/
  icuObjectInit(&ICUD3);
  ICUD3.timer = NRF_TIMER2;
#endif

#if NRF5_ICU_USE_TIMER3
  /* Driver initialization.*/
  icuObjectInit(&ICUD4);
  ICUD4.timer = NRF_TIMER3;
#endif

#if NRF5_ICU_USE_TIMER4
  /* Driver initialization.*/
  icuObjectInit(&ICUD5);
  ICUD5.timer = NRF_TIMER4;
#endif
}

/**
 * @brief   Configures and activates the ICU peripheral.
 *
 * @param[in] icup     Pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start(ICUDriver *icup) {
  size_t ch;
  osalDbgAssert((&icup->config->iccfgp[0] != NULL) ||
                (&icup->config->iccfgp[1] != NULL),
                 "invalid input configuration");
  /* Prescaler value calculation: ftimer = 16MHz / 2^PRESCALER */
  uint16_t psc_ratio = 16000000 / icup->config->frequency;
  /* Prescaler ratio must be between 1 and 512, and a power of two. */
  osalDbgAssert(psc_ratio <= 512 && !(psc_ratio & (psc_ratio - 1)),
                "invalid frequency");
  /* Prescaler value as a power of 2, must be 0..9 */
  uint32_t psc_value;
  for (psc_value = 0; psc_value < 10; psc_value++)
    if (psc_ratio == (unsigned)(1 << psc_value))
      break;

  /* Configure as 32bits timer */
  icup->timer->BITMODE = TIMER_BITMODE_BITMODE_32Bit;

  /* Set timer mode */
  icup->timer->MODE = TIMER_MODE_MODE_Timer;

  /* Set prescaler */
  icup->timer->PRESCALER = psc_value;

  /* With clear shortcuts. */
  icup->timer->SHORTS = 0;

  /* Clear Timer */
  icup->timer->TASKS_CLEAR = 1;

  /* Disable and reset interrupts for compare events */
  icup->timer->INTENCLR = (TIMER_INTENCLR_COMPARE0_Msk |
                           TIMER_INTENCLR_COMPARE1_Msk |
                           TIMER_INTENCLR_COMPARE2_Msk |
                           TIMER_INTENCLR_COMPARE3_Msk );

  icup->timer->EVENTS_COMPARE[0] = 0;
  icup->timer->EVENTS_COMPARE[1] = 0;
  icup->timer->EVENTS_COMPARE[2] = 0;
  icup->timer->EVENTS_COMPARE[3] = 0;
  (void) icup->timer->EVENTS_COMPARE[0];
  (void) icup->timer->EVENTS_COMPARE[1];
  (void) icup->timer->EVENTS_COMPARE[2];
  (void) icup->timer->EVENTS_COMPARE[3];

    /* Enable GPIOTE and TIMER interrupt vectors.*/
#if NRF5_ICU_USE_GPIOTE_PPI
    nvicEnableVector(GPIOTE_IRQn, NRF5_ICU_GPIOTE_IRQ_PRIORITY);
#endif
#if NRF5_ICU_USE_TIMER0
    if (&ICUD1 == icup) {
      nvicEnableVector(TIMER0_IRQn, NRF5_ICU_TIMER0_IRQ_PRIORITY);
    }
#endif
#if NRF5_ICU_USE_TIMER1
    if (&ICUD2 == icup) {
      nvicEnableVector(TIMER1_IRQn, NRF5_ICU_TIMER1_IRQ_PRIORITY);
    }
#endif
#if NRF5_ICU_USE_TIMER2
    if (&ICUD3 == icup) {
      nvicEnableVector(TIMER2_IRQn, NRF5_ICU_TIMER2_IRQ_PRIORITY);
    }
#endif
#if NRF5_ICU_USE_TIMER3
    if (&ICUD4 == icup) {
      nvicEnableVector(TIMER3_IRQn, NRF5_ICU_TIMER3_IRQ_PRIORITY);
    }
#endif
#if NRF5_ICU_USE_TIMER4
    if (&ICUD5 == icup) {
      nvicEnableVector(TIMER4_IRQn, NRF5_ICU_TIMER4_IRQ_PRIORITY);
    }
#endif

  /* clean channel structures and set pointers to channel configs */
  for (ch=0; ch<ICU_CHANNELS; ch++) {
    icup->channel[ch].last_active = 0;
    icup->channel[ch].last_idle = 0;
    icup->channel[ch].state = ICU_CH_IDLE;
  }
  /* Set GPIOTE & PPI channels */
  start_channels(icup);
}

/**
 * @brief   Deactivates the ICU peripheral.
 *
 * @param[in] icup     Pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop(ICUDriver *icup) {

  if (icup->state == ICU_READY) {
    /* Timer stop.*/
	icup->timer->TASKS_STOP = 1;

#if NRF5_ICU_USE_GPIOTE_PPI
	uint8_t channel;
	for (channel = 0; channel<ICU_CHANNELS; channel++) {
	  const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	  if (cfg_channel == NULL) continue;

	  const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;
	  const uint8_t *ppi_channel = cfg_channel->ppi_channel;

	  /* Disable Timer interrupt */
	  icup->timer->INTENCLR = 1 << (TIMER_INTENCLR_COMPARE0_Pos + channel);

	  /* Disable GPIOTE interrupts */
	  NRF_GPIOTE->INTENCLR = (1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);

	  /* Disable PPI channels */
	  NRF_PPI->CHENCLR = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));

	  /* Clear GPIOTE channels */
	  NRF_GPIOTE->CONFIG[gpiote_channel[0]] &= ~(GPIOTE_CONFIG_PSEL_Msk | GPIOTE_CONFIG_POLARITY_Msk);
	  NRF_GPIOTE->CONFIG[gpiote_channel[1]] &= ~(GPIOTE_CONFIG_PSEL_Msk | GPIOTE_CONFIG_POLARITY_Msk);
	}
#endif

#if NRF5_ICU_USE_GPIOTE_PPI
    nvicDisableVector(GPIOTE_IRQn);
#endif
#if NRF5_ICU_USE_TIMER0
    if (&ICUD1 == icup) {
      nvicDisableVector(TIMER0_IRQn);
    }
#endif
#if NRF5_ICU_USE_TIMER1
    if (&ICUD2 == icup) {
      nvicDisableVector(TIMER1_IRQn);
    }
#endif
#if NRF5_ICU_USE_TIMER2
    if (&ICUD3 == icup) {
      nvicDisableVector(TIMER2_IRQn);
    }
#endif
#if NRF5_ICU_USE_TIMER3
    if (&ICUD4 == icup) {
      nvicDisableVector(TIMER3_IRQn);
    }
#endif
#if NRF5_ICU_USE_TIMER4
    if (&ICUD5 == icup) {
      nvicDisableVector(TIMER4_IRQn);
    }
#endif
  }
}

/**
 * @brief   Starts the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start_capture(ICUDriver *icup) {
  /* Clear and start Timer */
  icup->timer->TASKS_CLEAR = 1;
  icup->timer->TASKS_START = 1;

#if NRF5_ICU_USE_GPIOTE_PPI
  uint8_t channel;
  for (channel = 0; channel<ICU_CHANNELS; channel++) {
	const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	if (cfg_channel == NULL) continue;

	const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;
	const uint8_t *ppi_channel = cfg_channel->ppi_channel;

	/* Enable interrupt for overflow events */
	icup->timer->CC[channel] = ICU_WAIT_TIMEOUT;
	icup->timer->INTENSET = 1 << (TIMER_INTENSET_COMPARE0_Pos + channel);

	/* Enable PPI channels */
	NRF_PPI->CHENSET = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));

	/* Enable GPIOTE interrupt */
	NRF_GPIOTE->INTENSET = (1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);
  }
#endif
}

/**
 * @brief   Waits for a completed capture.
 * @note    The operation is performed in polled mode.
 * @note    In order to use this function notifications must be disabled.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The capture status.
 * @retval false        if the capture is successful.
 * @retval true         if a timer overflow occurred.
 *
 * @notapi
 */
bool icu_lld_wait_capture(ICUDriver *icup) {

  /* If the driver is still in the ICU_WAITING state then we need to wait
     for the first activation edge.*/
  if (icup->state == ICU_WAITING)
    if (icu_lld_wait_edge(icup))
      return true;

  /* This edge marks the availability of a capture result.*/
  return icu_lld_wait_edge(icup);
}

/**
 * @brief   Stops the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop_capture(ICUDriver *icup) {
  /* Timer stopped.*/
  icup->timer->TASKS_STOP = 1;

#if NRF5_ICU_USE_GPIOTE_PPI
	uint8_t channel;
	for (channel = 0; channel<ICU_CHANNELS; channel++) {
	  const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	  if (cfg_channel == NULL) continue;

	  const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;
	  const uint8_t *ppi_channel = cfg_channel->ppi_channel;

	  /* Disable Timer interrupt for overflow events */
	  icup->timer->INTENCLR = 1 << (TIMER_INTENCLR_COMPARE0_Pos + channel);

	  /* Disable GPIOTE interrupt */
	  NRF_GPIOTE->INTENCLR = (1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);

	  /* Disable PPI channels */
	  NRF_PPI->CHENCLR = ((1 << ppi_channel[0]) | (1 << ppi_channel[1]));
	}
#endif
}

/**
 * @brief   Enables notifications.
 * @pre     The ICU unit must have been activated using @p icuStart() and the
 *          capture started using @p icuStartCapture().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_enable_notifications(ICUDriver *icup) {
#if NRF5_ICU_USE_GPIOTE_PPI
	uint8_t channel;
	for (channel = 0; channel<ICU_CHANNELS; channel++) {
	  const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	  if (cfg_channel == NULL) continue;

	  const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;

	  /* Enable Timer interrupt */
	  icup->timer->INTENSET = 1 << (TIMER_INTENSET_COMPARE0_Pos + channel);

	  /* Enable GPIOTE interrupt */
	  NRF_GPIOTE->INTENSET = (1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);
	}
#endif
}

/**
 * @brief   Disables notifications.
 * @pre     The ICU unit must have been activated using @p icuStart() and the
 *          capture started using @p icuStartCapture().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_disable_notifications(ICUDriver *icup) {
  /* All interrupts disabled.*/
#if NRF5_ICU_USE_GPIOTE_PPI
	uint8_t channel;
	for (channel = 0; channel<ICU_CHANNELS; channel++) {
	  const ICUChannelConfig *cfg_channel = &icup->config->iccfgp[channel];
	  if (cfg_channel == NULL) continue;

	  const uint8_t *gpiote_channel = cfg_channel->gpiote_channel;

	  /* Disable Timer interrupt for overflow events */
	  icup->timer->INTENCLR = 1 << (TIMER_INTENCLR_COMPARE0_Pos + channel);

	  /* Disable GPIOTE interrupt */
	  NRF_GPIOTE->INTENCLR = (1 << gpiote_channel[0]) | (1 << gpiote_channel[1]);
	}
#endif

}

#endif /* HAL_USE_ICU */
