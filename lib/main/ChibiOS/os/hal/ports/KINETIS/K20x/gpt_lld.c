/*
    ChibiOS - Copyright (C) 2014 Derek Mulcahy

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
 * @file    KINETIS/gpt_lld.c
 * @brief   KINETIS GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if HAL_USE_GPT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define KINETIS_PIT0_HANDLER    VectorB8
#define KINETIS_PIT1_HANDLER    VectorBC
#define KINETIS_PIT2_HANDLER    VectorC0
#define KINETIS_PIT3_HANDLER    VectorC4

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTD1 driver identifier.
 * @note    The driver GPTD1 allocates the complex timer PIT0 when enabled.
 */
#if KINETIS_GPT_USE_PIT0 || defined(__DOXYGEN__)
GPTDriver GPTD1;
#endif

/**
 * @brief   GPTD2 driver identifier.
 * @note    The driver GPTD2 allocates the timer PIT1 when enabled.
 */
#if KINETIS_GPT_USE_PIT1 || defined(__DOXYGEN__)
GPTDriver GPTD2;
#endif

/**
 * @brief   GPTD3 driver identifier.
 * @note    The driver GPTD3 allocates the timer PIT2 when enabled.
 */
#if KINETIS_GPT_USE_PIT2 || defined(__DOXYGEN__)
GPTDriver GPTD3;
#endif

/**
 * @brief   GPTD4 driver identifier.
 * @note    The driver GPTD4 allocates the timer PIT3 when enabled.
 */
#if KINETIS_GPT_USE_PIT3 || defined(__DOXYGEN__)
GPTDriver GPTD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] gptp      pointer to a @p GPTDriver object
 */
static void gpt_lld_serve_interrupt(GPTDriver *gptp) {

  /* Clear the interrupt */
  gptp->channel->TFLG |= PIT_TCTRL_TIE;

  if (gptp->state == GPT_ONESHOT) {
    gptp->state = GPT_READY;                /* Back in GPT_READY state.     */
    gpt_lld_stop_timer(gptp);               /* Timer automatically stopped. */
  }
  gptp->config->callback(gptp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_GPT_USE_PIT0
#if !defined(KINETIS_PIT0_HANDLER)
#error "KINETIS_PIT0_HANDLER not defined"
#endif
/**
 * @brief   PIT1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_PIT0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD1);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_GPT_USE_PIT0 */

#if KINETIS_GPT_USE_PIT1
#if !defined(KINETIS_PIT1_HANDLER)
#error "KINETIS_PIT1_HANDLER not defined"
#endif
/**
 * @brief   PIT1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_PIT1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD2);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_GPT_USE_PIT1 */

#if KINETIS_GPT_USE_PIT2
#if !defined(KINETIS_PIT2_HANDLER)
#error "KINETIS_PIT2_HANDLER not defined"
#endif
/**
 * @brief   PIT2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_PIT2_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD3);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_GPT_USE_PIT2 */

#if KINETIS_GPT_USE_PIT3
#if !defined(KINETIS_PIT3_HANDLER)
#error "KINETIS_PIT3_HANDLER not defined"
#endif
/**
 * @brief   PIT3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_PIT3_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD4);

  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_GPT_USE_PIT3 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void) {

#if KINETIS_GPT_USE_PIT0
  /* Driver initialization.*/
  GPTD1.channel = &PIT->CHANNEL[0];
  gptObjectInit(&GPTD1);
#endif

#if KINETIS_GPT_USE_PIT1
  /* Driver initialization.*/
  GPTD2.channel = &PIT->CHANNEL[1];
  gptObjectInit(&GPTD2);
#endif

#if KINETIS_GPT_USE_PIT2
  /* Driver initialization.*/
  GPTD3.channel =  &PIT->CHANNEL[2];
  gptObjectInit(&GPTD3);
#endif

#if KINETIS_GPT_USE_PIT3
  /* Driver initialization.*/
  GPTD4.channel =  &PIT->CHANNEL[3];
  gptObjectInit(&GPTD4);
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
  uint16_t psc;

  if (gptp->state == GPT_STOP) {
    /* Clock activation.*/
    SIM->SCGC6 |= SIM_SCGC6_PIT;
    gptp->clock = KINETIS_SYSCLK_FREQUENCY;

#if KINETIS_GPT_USE_PIT0
    if (&GPTD1 == gptp) {
      nvicEnableVector(PITChannel0_IRQn, KINETIS_GPT_PIT0_IRQ_PRIORITY);
    }
#endif
#if KINETIS_GPT_USE_PIT1
    if (&GPTD2 == gptp) {
      nvicEnableVector(PITChannel1_IRQn, KINETIS_GPT_PIT1_IRQ_PRIORITY);
    }
#endif
#if KINETIS_GPT_USE_PIT2
    if (&GPTD3 == gptp) {
      nvicEnableVector(PITChannel2_IRQn, KINETIS_GPT_PIT2_IRQ_PRIORITY);
    }
#endif
#if KINETIS_GPT_USE_PIT3
    if (&GPTD4 == gptp) {
      nvicEnableVector(PITChannel3_IRQn, KINETIS_GPT_PIT3_IRQ_PRIORITY);
    }
#endif

  }

  /* Prescaler value calculation.*/
  psc = (uint16_t)((gptp->clock / gptp->config->frequency) - 1);
  osalDbgAssert(((uint32_t)(psc + 1) * gptp->config->frequency) == gptp->clock,
                "invalid frequency");

  /* Enable the PIT */
  PIT->MCR = 0;
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
    SIM->SCGC6 &= ~SIM_SCGC6_PIT;

    /* Disable the channel */
    gptp->channel->TCTRL = 0;

    /* Clear pending interrupts */
    gptp->channel->TFLG |= PIT_TFLG_TIF;

#if KINETIS_GPT_USE_PIT0
    if (&GPTD1 == gptp) {
      nvicDisableVector(PITChannel0_IRQn);
    }
#endif
#if KINETIS_GPT_USE_PIT1
    if (&GPTD2 == gptp) {
      nvicDisableVector(PITChannel1_IRQn);
    }
#endif
#if KINETIS_GPT_USE_PIT2
    if (&GPTD3 == gptp) {
      nvicDisableVector(PITChannel2_IRQn);
    }
#endif
#if KINETIS_GPT_USE_PIT3
    if (&GPTD4 == gptp) {
      nvicDisableVector(PITChannel3_IRQn);
    }
#endif
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

  /* Clear pending interrupts */
  gptp->channel->TFLG |= PIT_TFLG_TIF;

  /* Set the interval */
  gptp->channel->LDVAL = (gptp->clock / gptp->config->frequency) * interval;

  /* Start the timer */
  gptp->channel->TCTRL |= PIT_TCTRL_TIE | PIT_TCTRL_TEN;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp) {

  /* Stop the timer */
  gptp->channel->TCTRL = 0;
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
  struct PIT_CHANNEL *channel = gptp->channel;

  /* Disable timer and disable interrupts */
  channel->TCTRL = 0;

  /* Clear the interrupt flag */
  channel->TFLG |= PIT_TFLG_TIF;

  /* Set the interval */
  channel->LDVAL = (gptp->clock / gptp->config->frequency) * interval;

  /* Enable Timer but keep interrupts disabled */
  channel->TCTRL = PIT_TCTRL_TEN;

  /* Wait for the interrupt flag to be set */
  while (!(channel->TFLG & PIT_TFLG_TIF))
    ;

  /* Disable timer and disable interrupts */
  channel->TCTRL = 0;
}

#endif /* HAL_USE_GPT */

/** @} */
