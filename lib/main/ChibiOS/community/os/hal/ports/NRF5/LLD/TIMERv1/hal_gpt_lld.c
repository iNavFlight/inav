/*
    ChibiOS - 2015 Stephen Caudle

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
 * @file    TIMERv1/hal_gpt_lld.c
 * @brief   NRF5 GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if HAL_USE_GPT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define NRF5_TIMER_PRESCALER_NUM 10
#define NRF5_TIMER_COMPARE_NUM   4

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTD1 driver identifier.
 * @note    The driver GPTD1 allocates the complex timer TIM1 when enabled.
 */
#if NRF5_GPT_USE_TIMER0 || defined(__DOXYGEN__)
GPTDriver GPTD1;
#endif

/**
 * @brief   GPTD2 driver identifier.
 * @note    The driver GPTD2 allocates the timer TIM2 when enabled.
 */
#if NRF5_GPT_USE_TIMER1 || defined(__DOXYGEN__)
GPTDriver GPTD2;
#endif

/**
 * @brief   GPTD3 driver identifier.
 * @note    The driver GPTD3 allocates the timer TIM3 when enabled.
 */
#if NRF5_GPT_USE_TIMER2 || defined(__DOXYGEN__)
GPTDriver GPTD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static uint8_t prescaler(uint16_t freq)
{
  uint8_t i;
  static const gptfreq_t frequencies[] = {
    NRF5_GPT_FREQ_16MHZ,
    NRF5_GPT_FREQ_8MHZ,
    NRF5_GPT_FREQ_4MHZ,
    NRF5_GPT_FREQ_2MHZ,
    NRF5_GPT_FREQ_1MHZ,
    NRF5_GPT_FREQ_500KHZ,
    NRF5_GPT_FREQ_250KHZ,
    NRF5_GPT_FREQ_125KHZ,
    NRF5_GPT_FREQ_62500HZ,
    NRF5_GPT_FREQ_31250HZ,
  };

  for (i = 0; i < NRF5_TIMER_PRESCALER_NUM; i++)
    if (freq == frequencies[i])
      return i;

  osalDbgAssert(FALSE, "invalid timer frequency");

  return 0;
}

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 */
static void gpt_lld_serve_interrupt(GPTDriver *gptp) {

  gptp->tim->EVENTS_COMPARE[gptp->cc_int] = 0;
#if CORTEX_MODEL >= 4
  (void)gptp->tim->EVENTS_COMPARE[gptp->cc_int];
#endif
  if (gptp->state == GPT_ONESHOT)
    gptp->state = GPT_READY;                 /* Back in GPT_READY state.     */
  gptp->config->callback(gptp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_GPT_USE_TIMER0
/**
 * @brief   TIMER0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector60) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_GPT_USE_TIMER0 */

#if NRF5_GPT_USE_TIMER1
/**
 * @brief   TIMER1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector64) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_GPT_USE_TIMER1 */

#if NRF5_GPT_USE_TIMER2
/**
 * @brief   TIMER2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector68) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* NRF5_GPT_USE_TIMER2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if NRF5_GPT_USE_TIMER0
  /* Driver initialization.*/
  GPTD1.tim = NRF_TIMER0;
  gptObjectInit(&GPTD1);
#endif

#if NRF5_GPT_USE_TIMER1
  /* Driver initialization.*/
  GPTD2.tim = NRF_TIMER1;
  gptObjectInit(&GPTD2);
#endif

#if NRF5_GPT_USE_TIMER2
  /* Driver initialization.*/
  GPTD3.tim = NRF_TIMER2;
  gptObjectInit(&GPTD3);
#endif
}

/**
 * @brief   Configures and activates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_start(GPTDriver *gptp) {

  NRF_TIMER_Type *tim = gptp->tim;

  if (gptp->state == GPT_STOP) {
    osalDbgAssert(gptp->cc_int < NRF5_TIMER_COMPARE_NUM,
        "invalid capture/compare index");

    tim->INTENSET = TIMER_INTENSET_COMPARE0_Msk << gptp->cc_int;
#if NRF5_GPT_USE_TIMER0
    if (&GPTD1 == gptp)
      nvicEnableVector(TIMER0_IRQn, NRF5_GPT_TIMER0_IRQ_PRIORITY);
#endif
#if NRF5_GPT_USE_TIMER1
    if (&GPTD2 == gptp)
      nvicEnableVector(TIMER1_IRQn, NRF5_GPT_TIMER1_IRQ_PRIORITY);
#endif
#if NRF5_GPT_USE_TIMER2
    if (&GPTD3 == gptp)
      nvicEnableVector(TIMER2_IRQn, NRF5_GPT_TIMER2_IRQ_PRIORITY);
#endif
  }

  /* Prescaler value calculation.*/
  tim->PRESCALER = prescaler(gptp->config->frequency);

  /* Timer configuration.*/
  tim->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;

  switch (gptp->config->resolution) {

    case 8:
      tim->BITMODE = TIMER_BITMODE_BITMODE_08Bit << TIMER_BITMODE_BITMODE_Pos;
      break;

    case 16:
      tim->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
      break;

#if NRF5_GPT_USE_TIMER0
    case 24:
      tim->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
      break;

    case 32:
      tim->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
      break;
#endif

    default:
      osalDbgAssert(FALSE, "invalid timer resolution");
      break;
  };
}

/**
 * @brief   Deactivates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop(GPTDriver *gptp) {

  if (gptp->state == GPT_READY) {
    gptp->tim->TASKS_SHUTDOWN = 1;

#if NRF5_GPT_USE_TIMER0
    if (&GPTD1 == gptp)
      nvicDisableVector(TIMER0_IRQn);
#endif
#if NRF5_GPT_USE_TIMER1
    if (&GPTD2 == gptp)
      nvicDisableVector(TIMER1_IRQn);
#endif
#if NRF5_GPT_USE_TIMER2
    if (&GPTD3 == gptp)
      nvicDisableVector(TIMER2_IRQn);
#endif
    gptp->tim->INTENCLR = TIMER_INTENSET_COMPARE0_Msk << gptp->cc_int;
  }
}

/**
 * @brief   Starts the timer in continuous mode.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  period in ticks
 *
 * @notapi
 */
void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval) {

  NRF_TIMER_Type *tim = gptp->tim;

  tim->TASKS_CLEAR = 1;
  tim->CC[gptp->cc_int] = (uint32_t)(interval - 1);  /* Time constant.        */
  if (gptp->state == GPT_ONESHOT)
    gptp->tim->SHORTS = TIMER_SHORTS_COMPARE0_STOP_Msk << gptp->cc_int;
  else if (gptp->state == GPT_CONTINUOUS)
    gptp->tim->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk << gptp->cc_int;
  tim->TASKS_START = 1;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {

  gptp->tim->TASKS_STOP = 1;
}

/**
 * @brief   Starts the timer in one shot mode and waits for completion.
 * @details This function specifically polls the timer waiting for completion
 *          in order to not have extra delays caused by interrupt servicing,
 *          this function is only recommended for short delays.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 * @param[in] interval  time interval in ticks
 *
 * @notapi
 */
void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval) {

  NRF_TIMER_Type *tim = gptp->tim;

  tim->INTENCLR = (1UL << gptp->cc_int) << TIMER_INTENSET_COMPARE0_Pos;
  tim->TASKS_CLEAR = 1;
  tim->CC[gptp->cc_int] = (uint32_t)(interval - 1);  /* Time constant.        */
  tim->TASKS_START = 1;
  while (!(tim->INTENSET & (TIMER_INTENSET_COMPARE0_Msk << gptp->cc_int)))
    ;
  tim->INTENSET = TIMER_INTENSET_COMPARE0_Msk << gptp->cc_int;
}

/**
 * @brief   Returns the counter value of GPT peripheral.
 * @pre     The GPT unit must be running in continuous mode.
 * @note    The nature of the counter is not defined, it may count upward
 *          or downward, it could be continuously running or not.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 * @return              The current counter value.
 *
 * @notapi
 */
gptcnt_t gpt_lld_get_counter(GPTDriver *gptp) {

  gptp->tim->TASKS_CAPTURE[gptp->cc_get] = 1;
  return gptp->tim->CC[gptp->cc_get];
}

#endif /* HAL_USE_GPT */

/** @} */
