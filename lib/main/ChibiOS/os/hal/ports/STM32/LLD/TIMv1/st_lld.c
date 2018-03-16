/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    STM32/st_lld.c
 * @brief   ST Driver subsystem low level driver code.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)


#define NVIC_PRIORITY_GROUPING NVIC_PriorityGroup_2
#define NVIC_PRIO_CHIBIOS      NVIC_BUILD_PRIORITY(3, 0)
#define NVIC_BUILD_PRIORITY(base,sub) (((((base)<<(4-(7-(NVIC_PRIORITY_GROUPING>>8))))|((sub)&(0x0f>>(7-(NVIC_PRIORITY_GROUPING>>8)))))<<4)&0xf0)
#define NVIC_PRIORITY_BASE(prio) (((prio)>>(4-(7-(NVIC_PRIORITY_GROUPING>>8))))>>4)
#define NVIC_PRIORITY_SUB(prio) (((prio)&(0x0f>>(7-(NVIC_PRIORITY_GROUPING>>8))))>>4)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING

#if (OSAL_ST_RESOLUTION == 32)
#define ST_ARR_INIT                         0xFFFFFFFF
#else
#define ST_ARR_INIT                         0x0000FFFF
#endif

#if STM32_ST_USE_TIMER == 2
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM2_IS_32BITS
#error "TIM2 is not a 32bits timer"
#endif

#if defined(STM32_TIM2_IS_USED)
#error "ST requires TIM2 but the timer is already used"
#else
#define STM32_TIM2_IS_USED
#endif

#define ST_HANDLER                          STM32_TIM2_HANDLER
#define ST_NUMBER                           STM32_TIM2_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK1
#define ST_ENABLE_CLOCK()                   rccEnableTIM2(FALSE)
#if defined(STM32F1XX)
#define ST_ENABLE_STOP()                    DBGMCU->CR |= DBGMCU_CR_DBG_TIM2_STOP
#elif defined(STM32L4XX)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM2_STOP
#else
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM2_STOP
#endif

#elif STM32_ST_USE_TIMER == 3
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM3_IS_32BITS
#error "TIM3 is not a 32bits timer"
#endif

#if defined(STM32_TIM3_IS_USED)
#error "ST requires TIM3 but the timer is already used"
#else
#define STM32_TIM3_IS_USED
#endif

#define ST_HANDLER                          TIM3_IRQHandler
#define ST_IRQ                              TIM3_IRQn
#define ST_NUMBER                           STM32_TIM3_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK1
#define ST_ENABLE_CLOCK()                   rccEnableTIM3(FALSE)
#if defined(STM32F1XX)
#define ST_ENABLE_STOP()                    DBGMCU->CR |= DBGMCU_CR_DBG_TIM3_STOP
#elif defined(STM32L4XX)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM3_STOP
#else
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP
#endif

#elif STM32_ST_USE_TIMER == 4
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM4_IS_32BITS
#error "TIM4 is not a 32bits timer"
#endif

#if defined(STM32_TIM4_IS_USED)
#error "ST requires TIM4 but the timer is already used"
#else
#define STM32_TIM4_IS_USED
#endif

#define ST_HANDLER                          TIM4_IRQHandler
#define ST_IRQ                              TIM4_IRQn
#define ST_NUMBER                           STM32_TIM4_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK1
#define ST_ENABLE_CLOCK()                   rccEnableTIM4(FALSE)
#if defined(STM32F1XX)
#define ST_ENABLE_STOP()                    DBGMCU->CR |= DBGMCU_CR_DBG_TIM4_STOP
#elif defined(STM32L4XX)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM4_STOP
#else
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM4_STOP
#endif

#elif STM32_ST_USE_TIMER == 5
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM5_IS_32BITS
#error "TIM5 is not a 32bits timer"
#endif

#if defined(STM32_TIM5_IS_USED)
#error "ST requires TIM5 but the timer is already used"
#else
#define STM32_TIM5_IS_USED
#endif

#define ST_HANDLER                          STM32_TIM5_HANDLER
#define ST_NUMBER                           STM32_TIM5_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK1
#define ST_ENABLE_CLOCK()                   rccEnableTIM5(FALSE)
#if defined(STM32F1XX)
#define ST_ENABLE_STOP()                    DBGMCU->CR |= DBGMCU_CR_DBG_TIM5_STOP
#elif defined(STM32L4XX)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM5_STOP
#else
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM5_STOP
#endif

#elif STM32_ST_USE_TIMER == 21
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM21_IS_32BITS
#error "TIM21 is not a 32bits timer"
#endif

#if defined(STM32_TIM21_IS_USED)
#error "ST requires TIM21 but the timer is already used"
#else
#define STM32_TIM21_IS_USED
#endif

#define ST_HANDLER                          STM32_TIM21_HANDLER
#define ST_NUMBER                           STM32_TIM21_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK2
#define ST_ENABLE_CLOCK()                   rccEnableTIM21(FALSE)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB2_FZ_DBG_TIM21_STOP

#elif STM32_ST_USE_TIMER == 22
#if (OSAL_ST_RESOLUTION == 32) && !STM32_TIM22_IS_32BITS
#error "TIM21 is not a 32bits timer"
#endif

#if defined(STM32_TIM22_IS_USED)
#error "ST requires TIM22 but the timer is already used"
#else
#define STM32_TIM22_IS_USED
#endif

#define ST_HANDLER                          STM32_TIM22_HANDLER
#define ST_NUMBER                           STM32_TIM22_NUMBER
#define ST_CLOCK_SRC                        STM32_TIMCLK2
#define ST_ENABLE_CLOCK()                   rccEnableTIM22(FALSE)
#define ST_ENABLE_STOP()                    DBGMCU->APB1FZ |= DBGMCU_APB2_FZ_DBG_TIM21_STOP

#else
#error "STM32_ST_USE_TIMER specifies an unsupported timer"
#endif

#if ST_CLOCK_SRC % OSAL_ST_FREQUENCY != 0
#error "the selected ST frequency is not obtainable because integer rounding"
#endif

#if (ST_CLOCK_SRC / OSAL_ST_FREQUENCY) - 1 > 0xFFFF
#error "the selected ST frequency is not obtainable because TIM timer prescaler limits"
#endif

#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC

#if STM32_HCLK % OSAL_ST_FREQUENCY != 0
#error "the selected ST frequency is not obtainable because integer rounding"
#endif

#if (STM32_HCLK / OSAL_ST_FREQUENCY) - 1 > 0xFFFFFF
#error "the selected ST frequency is not obtainable because SysTick timer counter limits"
#endif

#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */

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
OSAL_IRQ_HANDLER(SysTick_Handler) {

  OSAL_IRQ_PROLOGUE();

  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  OSAL_IRQ_EPILOGUE();
}
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined(__DOXYGEN__)
/**
 * @brief   TIM2 interrupt handler.
 * @details This interrupt is used for system tick in free running mode.
 *
 * @isr
 */
//OSAL_IRQ_HANDLER(ST_HANDLER) {
void ST_HANDLER(void) {
  OSAL_IRQ_PROLOGUE();

  /* Note, under rare circumstances an interrupt can remain latched even if
     the timer SR register has been cleared, in those cases the interrupt
     is simply ignored.*/
  if ((STM32_ST_TIM->SR & TIM_SR_CC1IF) != 0U) {
    STM32_ST_TIM->SR = 0U;

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
void st_lld_init(void) {

#if OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING
  /* Free running counter mode.*/

  /* Enabling timer clock.*/
  ST_ENABLE_CLOCK();

  /* Enabling the stop mode during debug for this timer.*/
  ST_ENABLE_STOP();

  /* Initializing the counter in free running mode.*/
  STM32_ST_TIM->PSC    = (ST_CLOCK_SRC / OSAL_ST_FREQUENCY) - 1;
  STM32_ST_TIM->ARR    = ST_ARR_INIT;
  STM32_ST_TIM->CCMR1  = 0;
  STM32_ST_TIM->CCR[0] = 0;
  STM32_ST_TIM->DIER   = 0;
  STM32_ST_TIM->CR2    = 0;
  STM32_ST_TIM->EGR    = TIM_EGR_UG;
  STM32_ST_TIM->CR1    = TIM_CR1_CEN;

  /* IRQ enabled.*/
  //nvicEnableVector(ST_NUMBER, STM32_ST_IRQ_PRIORITY);
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = ST_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PRIORITY_BASE(NVIC_PRIO_CHIBIOS);
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_PRIORITY_SUB(NVIC_PRIO_CHIBIOS);
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC
  /* Periodic systick mode, the Cortex-Mx internal systick timer is used
     in this mode.*/
  SysTick->LOAD = (STM32_HCLK / OSAL_ST_FREQUENCY) - 1;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                  SysTick_CTRL_ENABLE_Msk |
                  SysTick_CTRL_TICKINT_Msk;

  /* IRQ enabled.*/
  nvicSetSystemHandlerPriority(HANDLER_SYSTICK, STM32_ST_IRQ_PRIORITY);
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
