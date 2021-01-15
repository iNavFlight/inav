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
 * @file    GPTM/hal_gpt_lld.c
 * @brief   TM4C123x/TM4C129x GPT subsystem low level driver source.
 *
 * @addtogroup GPT
 * @{
 */

#include "hal.h"

#if HAL_USE_GPT || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   GPTD1 driver identifier.
 */
#if TIVA_GPT_USE_GPT0 || defined(__DOXYGEN__)
GPTDriver GPTD1;
#endif

/**
 * @brief   GPTD2 driver identifier.
 */
#if TIVA_GPT_USE_GPT1 || defined(__DOXYGEN__)
GPTDriver GPTD2;
#endif

/**
 * @brief   GPTD3 driver identifier.
 */
#if TIVA_GPT_USE_GPT2 || defined(__DOXYGEN__)
GPTDriver GPTD3;
#endif

/**
 * @brief   GPTD4 driver identifier.
 */
#if TIVA_GPT_USE_GPT3 || defined(__DOXYGEN__)
GPTDriver GPTD4;
#endif

/**
 * @brief   GPTD5 driver identifier.
 */
#if TIVA_GPT_USE_GPT4 || defined(__DOXYGEN__)
GPTDriver GPTD5;
#endif

/**
 * @brief   GPTD6 driver identifier.
 */
#if TIVA_GPT_USE_GPT5 || defined(__DOXYGEN__)
GPTDriver GPTD6;
#endif

/**
 * @brief   GPTD7 driver identifier.
 */
#if TIVA_GPT_USE_WGPT0 || defined(__DOXYGEN__)
GPTDriver GPTD7;
#endif

/**
 * @brief   GPTD8 driver identifier.
 */
#if TIVA_GPT_USE_WGPT1 || defined(__DOXYGEN__)
GPTDriver GPTD8;
#endif

/**
 * @brief   GPTD9 driver identifier.
 */
#if TIVA_GPT_USE_WGPT2 || defined(__DOXYGEN__)
GPTDriver GPTD9;
#endif

/**
 * @brief   GPTD10 driver identifier.
 */
#if TIVA_GPT_USE_WGPT3 || defined(__DOXYGEN__)
GPTDriver GPTD10;
#endif

/**
 * @brief   GPTD11 driver identifier.
 */
#if TIVA_GPT_USE_WGPT4 || defined(__DOXYGEN__)
GPTDriver GPTD11;
#endif

/**
 * @brief   GPTD12 driver identifier.
 */
#if TIVA_GPT_USE_WGPT5 || defined(__DOXYGEN__)
GPTDriver GPTD12;
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
 * @param[in] gptp      pointer to @p GPTDriver object
 */
static void gpt_lld_serve_interrupt(GPTDriver *gptp)
{
  HWREG(gptp->gpt + TIMER_O_ICR) = 0xffffffff;

  if (gptp->state == GPT_ONESHOT) {
    gptp->state = GPT_READY;
    gpt_lld_stop_timer(gptp);
  }

  gptp->config->callback(gptp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_GPT_USE_GPT0
#if !defined(TIVA_GPT0A_HANDLER)
#error "TIVA_GPT0A_HANDLER not defined"
#endif
/**
 * @brief   GPT0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT0A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD1);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_GPT1
#if !defined(TIVA_GPT1A_HANDLER)
#error "TIVA_GPT1A_HANDLER not defined"
#endif
/**
 * @brief   GPT1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT1A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD2);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_GPT2
#if !defined(TIVA_GPT2A_HANDLER)
#error "TIVA_GPT2A_HANDLER not defined"
#endif
/**
 * @brief   GPT2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT2A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD3);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_GPT3
#if !defined(TIVA_GPT3A_HANDLER)
#error "TIVA_GPT3A_HANDLER not defined"
#endif
/**
 * @brief   GPT3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT3A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD4);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_GPT4
#if !defined(TIVA_GPT4A_HANDLER)
#error "TIVA_GPT4A_HANDLER not defined"
#endif
/**
 * @brief   GPT4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT4A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD5);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_GPT5
#if !defined(TIVA_GPT5A_HANDLER)
#error "TIVA_GPT5A_HANDLER not defined"
#endif
/**
 * @brief   GPT5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_GPT5A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD6);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT0
#if !defined(TIVA_WGPT0A_HANDLER)
#error "TIVA_WGPT0A_HANDLER not defined"
#endif
/**
 * @brief   WGPT0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT0A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD7);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT1
#if !defined(TIVA_WGPT1A_HANDLER)
#error "TIVA_WGPT1A_HANDLER not defined"
#endif
/**
 * @brief   WGPT1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT1A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD8);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT2
#if !defined(TIVA_WGPT2A_HANDLER)
#error "TIVA_WGPT2A_HANDLER not defined"
#endif
/**
 * @brief   WGPT2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT2A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD9);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT3
#if !defined(TIVA_WGPT3A_HANDLER)
#error "TIVA_WGPT3A_HANDLER not defined"
#endif
/**
 * @brief   WGPT3 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT3A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD10);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT4
#if !defined(TIVA_WGPT4A_HANDLER)
#error "TIVA_WGPT4A_HANDLER not defined"
#endif
/**
 * @brief   WGPT4 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT4A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD11);

  OSAL_IRQ_EPILOGUE();
}
#endif

#if TIVA_GPT_USE_WGPT5
#if !defined(TIVA_WGPT5A_HANDLER)
#error "TIVA_WGPT5A_HANDLER not defined"
#endif
/**
 * @brief   WGPT5 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WGPT5A_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

  gpt_lld_serve_interrupt(&GPTD12);

  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level GPT driver initialization.
 *
 * @notapi
 */
void gpt_lld_init(void)
{
  /* Driver initialization.*/
#if TIVA_GPT_USE_GPT0
  GPTD1.gpt = TIMER0_BASE;
  gptObjectInit(&GPTD1);
#endif

#if TIVA_GPT_USE_GPT1
  GPTD2.gpt = TIMER1_BASE;
  gptObjectInit(&GPTD2);
#endif

#if TIVA_GPT_USE_GPT2
  GPTD3.gpt = TIMER2_BASE;
  gptObjectInit(&GPTD3);
#endif

#if TIVA_GPT_USE_GPT3
  GPTD4.gpt = TIMER3_BASE;
  gptObjectInit(&GPTD4);
#endif

#if TIVA_GPT_USE_GPT4
  GPTD5.gpt = TIMER4_BASE;
  gptObjectInit(&GPTD5);
#endif

#if TIVA_GPT_USE_GPT5
  GPTD6.gpt = TIMER5_BASE;
  gptObjectInit(&GPTD6);
#endif

#if TIVA_GPT_USE_WGPT0
  GPTD7.gpt = WTIMER0_BASE;
  gptObjectInit(&GPTD7);
#endif

#if TIVA_GPT_USE_WGPT1
  GPTD8.gpt = WTIMER1_BASE;
  gptObjectInit(&GPTD8);
#endif

#if TIVA_GPT_USE_WGPT2
  GPTD9.gpt = WTIMER2_BASE;
  gptObjectInit(&GPTD9);
#endif

#if TIVA_GPT_USE_WGPT3
  GPTD10.gpt = WTIMER3_BASE;
  gptObjectInit(&GPTD10);
#endif

#if TIVA_GPT_USE_WGPT4
  GPTD11.gpt = WTIMER4_BASE;
  gptObjectInit(&GPTD11);
#endif

#if TIVA_GPT_USE_WGPT5
  GPTD12.gpt = WTIMER5_BASE;
  gptObjectInit(&GPTD12);
#endif
}

/**
 * @brief   Configures and activates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_start(GPTDriver *gptp)
{
  if (gptp->state == GPT_STOP) {
    /* Clock activation.*/
#if TIVA_GPT_USE_GPT0
    if (&GPTD1 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 0)))
        ;

      nvicEnableVector(TIVA_GPT0A_NUMBER, TIVA_GPT_GPT0A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_GPT1
    if (&GPTD2 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 1)))
        ;

      nvicEnableVector(TIVA_GPT1A_NUMBER, TIVA_GPT_GPT1A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_GPT2
    if (&GPTD3 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 2);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 2)))
        ;

      nvicEnableVector(TIVA_GPT2A_NUMBER, TIVA_GPT_GPT2A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_GPT3
    if (&GPTD4 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 3);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 3)))
        ;

      nvicEnableVector(TIVA_GPT3A_NUMBER, TIVA_GPT_GPT3A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_GPT4
    if (&GPTD5 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 4);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 4)))
        ;

      nvicEnableVector(TIVA_GPT4A_NUMBER, TIVA_GPT_GPT4A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_GPT5
    if (&GPTD6 == gptp) {
      HWREG(SYSCTL_RCGCTIMER) |= (1 << 5);

      while (!(HWREG(SYSCTL_PRTIMER) & (1 << 5)))
        ;

      nvicEnableVector(TIVA_GPT5A_NUMBER, TIVA_GPT_GPT5A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT0
    if (&GPTD7 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 0);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 0)))
        ;

      nvicEnableVector(TIVA_WGPT0A_NUMBER, TIVA_GPT_WGPT0A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT1
    if (&GPTD8 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 1);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 1)))
        ;

      nvicEnableVector(TIVA_WGPT1A_NUMBER, TIVA_GPT_WGPT1A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT2
    if (&GPTD9 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 2);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 2)))
        ;

      nvicEnableVector(TIVA_WGPT2A_NUMBER, TIVA_GPT_WGPT2A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT3
    if (&GPTD10 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 3);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 3)))
        ;

      nvicEnableVector(TIVA_WGPT3A_NUMBER, TIVA_GPT_WGPT3A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT4
    if (&GPTD11 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 4);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 4)))
        ;

      nvicEnableVector(TIVA_WGPT4A_NUMBER, TIVA_GPT_WGPT4A_IRQ_PRIORITY);
    }
#endif

#if TIVA_GPT_USE_WGPT5
    if (&GPTD12 == gptp) {
      HWREG(SYSCTL_RCGCWTIMER) |= (1 << 5);

      while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 5)))
        ;

      nvicEnableVector(TIVA_WGPT5A_NUMBER, TIVA_GPT_WGPT5A_IRQ_PRIORITY);
    }
#endif
  }

  /* Timer configuration.*/
  HWREG(gptp->gpt + TIMER_O_CTL) = 0;
  HWREG(gptp->gpt + TIMER_O_CFG) = TIMER_CFG_16_BIT;
  HWREG(gptp->gpt + TIMER_O_TAPR) = ((TIVA_SYSCLK / gptp->config->frequency) - 1);
}

/**
 * @brief   Deactivates the GPT peripheral.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop(GPTDriver *gptp)
{
  if (gptp->state == GPT_READY) {
    HWREG(gptp->gpt + TIMER_O_IMR) = 0;
    HWREG(gptp->gpt + TIMER_O_TAILR) = 0;
    HWREG(gptp->gpt + TIMER_O_CTL) = 0;

#if TIVA_GPT_USE_GPT0
    if (&GPTD1 == gptp) {
      nvicDisableVector(TIVA_GPT0A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 0);
    }
#endif

#if TIVA_GPT_USE_GPT1
    if (&GPTD2 == gptp) {
      nvicDisableVector(TIVA_GPT1A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 1);
    }
#endif

#if TIVA_GPT_USE_GPT2
    if (&GPTD3 == gptp) {
      nvicDisableVector(TIVA_GPT2A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 2);
    }
#endif

#if TIVA_GPT_USE_GPT3
    if (&GPTD4 == gptp) {
      nvicDisableVector(TIVA_GPT3A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 3);
    }
#endif

#if TIVA_GPT_USE_GPT4
    if (&GPTD5 == gptp) {
      nvicDisableVector(TIVA_GPT4A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 4);
    }
#endif

#if TIVA_GPT_USE_GPT5
    if (&GPTD6 == gptp) {
      nvicDisableVector(TIVA_GPT5A_NUMBER);
      HWREG(SYSCTL_RCGCTIMER) &= ~(1 << 5);
    }
#endif

#if TIVA_GPT_USE_WGPT0
    if (&GPTD7 == gptp) {
      nvicDisableVector(TIVA_WGPT0A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 0);
    }
#endif

#if TIVA_GPT_USE_WGPT1
    if (&GPTD8 == gptp) {
      nvicDisableVector(TIVA_WGPT1A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 1);
    }
#endif

#if TIVA_GPT_USE_WGPT2
    if (&GPTD9 == gptp) {
      nvicDisableVector(TIVA_WGPT2A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 2);
    }
#endif

#if TIVA_GPT_USE_WGPT3
    if (&GPTD10 == gptp) {
      nvicDisableVector(TIVA_WGPT3A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 3);
    }
#endif

#if TIVA_GPT_USE_WGPT4
    if (&GPTD11 == gptp) {
      nvicDisableVector(TIVA_WGPT4A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 4);
    }
#endif

#if TIVA_GPT_USE_WGPT5
    if (&GPTD12 == gptp) {
      nvicDisableVector(TIVA_WGPT5A_NUMBER);
      HWREG(SYSCTL_RCGCWTIMER) &= ~(1 << 5);
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
void gpt_lld_start_timer(GPTDriver *gptp, gptcnt_t interval)
{
  HWREG(gptp->gpt + TIMER_O_TAILR) = interval - 1;
  HWREG(gptp->gpt + TIMER_O_ICR) = 0xfffffff;
  HWREG(gptp->gpt + TIMER_O_IMR) = TIMER_IMR_TATOIM;
  HWREG(gptp->gpt + TIMER_O_TAMR) = TIMER_TAMR_TAMR_PERIOD | TIMER_TAMR_TAILD | TIMER_TAMR_TASNAPS;
  HWREG(gptp->gpt + TIMER_O_CTL) = TIMER_CTL_TAEN | TIMER_CTL_TASTALL;
}

/**
 * @brief   Stops the timer.
 *
 * @param[in] gptp      pointer to the @p GPTDriver object
 *
 * @notapi
 */
void gpt_lld_stop_timer(GPTDriver *gptp)
{
  HWREG(gptp->gpt + TIMER_O_IMR) = 0;
  HWREG(gptp->gpt + TIMER_O_TAILR) = 0;
  HWREG(gptp->gpt + TIMER_O_CTL) &= ~TIMER_CTL_TAEN;
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
void gpt_lld_polled_delay(GPTDriver *gptp, gptcnt_t interval)
{
  HWREG(gptp->gpt + TIMER_O_TAMR) = TIMER_TAMR_TAMR_1_SHOT | TIMER_TAMR_TAILD | TIMER_TAMR_TASNAPS;
  HWREG(gptp->gpt + TIMER_O_TAILR) = interval - 1;
  HWREG(gptp->gpt + TIMER_O_ICR) = 0xffffffff;
  HWREG(gptp->gpt + TIMER_O_CTL) = TIMER_CTL_TAEN | TIMER_CTL_TASTALL;
  while (!(HWREG(gptp->gpt + TIMER_O_RIS) & TIMER_IMR_TATOIM))
    ;
  HWREG(gptp->gpt + TIMER_O_ICR) = 0xffffffff;
}

#endif /* HAL_USE_GPT */

/** @} */
