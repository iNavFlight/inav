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
 * @file    GPTM/hal_st_lld.c
 * @brief   ST Driver subsystem low level driver code.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING

#if (TIVA_ST_USE_WIDE_TIMER == TRUE)

#if TIVA_ST_TIMER_NUMBER == 0
#define ST_HANDLER                          TIVA_WGPT0A_HANDLER
#define ST_NUMBER                           TIVA_WGPT0A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 0))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 0)))

#elif TIVA_ST_TIMER_NUMBER == 1
#define ST_HANDLER                          TIVA_WGPT1A_HANDLER
#define ST_NUMBER                           TIVA_WGPT1A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 1))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 1)))

#elif TIVA_ST_TIMER_NUMBER == 2
#define ST_HANDLER                          TIVA_WGPT2A_HANDLER
#define ST_NUMBER                           TIVA_WGPT2A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 2))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 2)))

#elif TIVA_ST_TIMER_NUMBER == 3
#define ST_HANDLER                          TIVA_WGPT3A_HANDLER
#define ST_NUMBER                           TIVA_WGPT3A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 3))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 3)))

#elif TIVA_ST_TIMER_NUMBER == 4
#define ST_HANDLER                          TIVA_WGPT4A_HANDLER
#define ST_NUMBER                           TIVA_WGPT4A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 4))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 4)))

#elif TIVA_ST_TIMER_NUMBER == 5
#define ST_HANDLER                          TIVA_WGPT5A_HANDLER
#define ST_NUMBER                           TIVA_WGPT5A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCWTIMER) |= (1 << 5))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRWTIMER) & (1 << 5)))

#else
#error "TIVA_ST_USE_TIMER specifies an unsupported timer"
#endif

#if (TIVA_SYSCLK / OSAL_ST_FREQUENCY) - 1 > 0xFFFF
#error "the selected ST frequency is not obtainable because TIM timer prescaler limits"
#endif

#elif (TIVA_ST_USE_WIDE_TIMER == FALSE)

#if TIVA_ST_TIMER_NUMBER == 0
#define ST_HANDLER                          TIVA_GPT0A_HANDLER
#define ST_NUMBER                           TIVA_GPT0A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 0))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 0)))

#elif TIVA_ST_TIMER_NUMBER == 1
#define ST_HANDLER                          TIVA_GPT1A_HANDLER
#define ST_NUMBER                           TIVA_GPT1A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 1))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 1)))

#elif TIVA_ST_TIMER_NUMBER == 2
#define ST_HANDLER                          TIVA_GPT2A_HANDLER
#define ST_NUMBER                           TIVA_GPT2A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 2))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 2)))

#elif TIVA_ST_TIMER_NUMBER == 3
#define ST_HANDLER                          TIVA_GPT3A_HANDLER
#define ST_NUMBER                           TIVA_GPT3A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 3))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 3)))

#elif TIVA_ST_TIMER_NUMBER == 4
#define ST_HANDLER                          TIVA_GPT4A_HANDLER
#define ST_NUMBER                           TIVA_GPT4A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 4))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 4)))

#elif TIVA_ST_TIMER_NUMBER == 5
#define ST_HANDLER                          TIVA_GPT5A_HANDLER
#define ST_NUMBER                           TIVA_GPT5A_NUMBER
#define ST_ENABLE_CLOCK()                   (HWREG(SYSCTL_RCGCTIMER) |= (1 << 5))
#define ST_WAIT_CLOCK()                     while (!(HWREG(SYSCTL_PRTIMER) & (1 << 5)))

#else
#error "TIVA_ST_USE_TIMER specifies an unsupported timer"
#endif

#if (TIVA_SYSCLK / OSAL_ST_FREQUENCY) - 1 > 0xFF
#error "the selected ST frequency is not obtainable because TIM timer prescaler limits"
#endif

#endif

#if TIVA_SYSCLK % OSAL_ST_FREQUENCY != 0
#error "the selected ST frequency is not obtainable because integer rounding"
#endif

#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if (OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC) || defined(__DOXYGEN__)
/**
 * @brief   System Timer vector.
 * @details This interrupt is used for system tick in periodic mode.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SysTick_Handler)
{
  OSAL_IRQ_PROLOGUE();

  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  OSAL_IRQ_EPILOGUE();
}
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined(__DOXYGEN__)
/**
 * @brief   GPT interrupt handler.
 * @details This interrupt is used for system tick in free running mode.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(ST_HANDLER)
{
  uint32_t mis;

  OSAL_IRQ_PROLOGUE();

  mis = HWREG(TIVA_ST_TIM + TIMER_O_MIS);
  HWREG(TIVA_ST_TIM + TIMER_O_ICR) = mis;

  if (mis & TIMER_IMR_TAMIM) {
    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();
  }

  OSAL_IRQ_EPILOGUE();
}
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ST driver initialization.
 *
 * @notapi
 */
void st_lld_init(void)
{
#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
  /* Free running counter mode.*/

  /* Enabling timer clock.*/
  ST_ENABLE_CLOCK();

  /* Wait until timer peripheral is ready */
  ST_WAIT_CLOCK();

  /* Initializing the counter in free running down mode.*/
  HWREG(TIVA_ST_TIM + TIMER_O_CTL)  = 0;
  HWREG(TIVA_ST_TIM + TIMER_O_CFG)  = TIMER_CFG_16_BIT; /* Timer split mode */
  HWREG(TIVA_ST_TIM + TIMER_O_TAMR) = (
      TIMER_TAMR_TAMR_PERIOD |  /* Periodic mode */
      TIMER_TAMR_TAMIE |        /* Match interrupt enable */
      TIMER_TAMR_TASNAPS);      /* Snapshot mode */

  HWREG(TIVA_ST_TIM + TIMER_O_TAPR) = (TIVA_SYSCLK / OSAL_ST_FREQUENCY) - 1;
  HWREG(TIVA_ST_TIM + TIMER_O_CTL)  = (
      TIMER_CTL_TAEN |          /* Timer A enable */
      TIMER_CTL_TASTALL);       /* Timer A stall when paused */

  /* IRQ enabled.*/
  nvicEnableVector(ST_NUMBER, TIVA_ST_IRQ_PRIORITY);
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC
  /* Periodic systick mode, the Cortex-Mx internal systick timer is used
     in this mode.*/
  SysTick->LOAD = (TIVA_SYSCLK / OSAL_ST_FREQUENCY) - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_ENABLE_Msk |
                  SysTick_CTRL_TICKINT_Msk;

  /* IRQ enabled.*/
  nvicSetSystemHandlerPriority(HANDLER_SYSTICK, TIVA_ST_IRQ_PRIORITY);
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/**
 * @}
 */
