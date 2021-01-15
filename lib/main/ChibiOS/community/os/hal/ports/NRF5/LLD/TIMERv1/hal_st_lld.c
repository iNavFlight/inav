/*
    ChibiOS - Copyright (C) 2015 Fabio Utzig
                            2016 Stephane D'Alu

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
 * @file    TIMERv1/hal_st_lld.c
 * @brief   NRF5 ST subsystem low level driver source.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

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
#if NRF5_ST_USE_RTC0 == TRUE
/**
 * @brief   System Timer vector (RTC0)
 * @details This interrupt is used for system tick in periodic mode
 *          if selected with NRF5_ST_USE_RTC0
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector6C) {

  OSAL_IRQ_PROLOGUE();

  NRF_RTC0->EVENTS_TICK = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC0->EVENTS_TICK;
#endif
  
  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  OSAL_IRQ_EPILOGUE();
}
#endif

#if NRF5_ST_USE_RTC1 == TRUE
/**
 * @brief   System Timer vector (RTC1)
 * @details This interrupt is used for system tick in periodic mode
 *          if selected with NRF5_ST_USE_RTC1
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector84) {

  OSAL_IRQ_PROLOGUE();

  NRF_RTC1->EVENTS_TICK = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC1->EVENTS_TICK;
#endif
  
  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  OSAL_IRQ_EPILOGUE();
}
#endif

#if NRF5_ST_USE_TIMER0 == TRUE
/**
 * @brief   System Timer vector. (TIMER0)
 * @details This interrupt is used for system tick in periodic mode
 *          if selected with NRF5_ST_USE_TIMER0
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector60) {

  OSAL_IRQ_PROLOGUE();

  /* Clear timer compare event */
  if (NRF_TIMER0->EVENTS_COMPARE[0] != 0) {
    NRF_TIMER0->EVENTS_COMPARE[0] = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_TIMER0->EVENTS_COMPARE[0];
#endif
    
    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();
  }

  OSAL_IRQ_EPILOGUE();
}
#endif
#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined(__DOXYGEN__)
#if NRF5_ST_USE_RTC0 == TRUE
/**
 * @brief   System Timer vector (RTC0)
 * @details This interrupt is used for freerunning mode (tick-less)
 *          if selected with NRF5_ST_USE_RTC0 
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector6C) {

  OSAL_IRQ_PROLOGUE();

  if (NRF_RTC0->EVENTS_COMPARE[0]) {
    NRF_RTC0->EVENTS_COMPARE[0] = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_RTC0->EVENTS_COMPARE[0];
#endif
    
    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();
  }

#if OSAL_ST_RESOLUTION == 16
  if (NRF_RTC0->EVENTS_COMPARE[1]) {
    NRF_RTC0->EVENTS_COMPARE[1] = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_RTC0->EVENTS_COMPARE[1];
#endif
    NRF_RTC0->TASKS_CLEAR = 1;
  }
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif

#if NRF5_ST_USE_RTC1 == TRUE
/**
 * @brief   System Timer vector (RTC1)
 * @details This interrupt is used for freerunning mode (tick-less)
 *          if selected with NRF5_ST_USE_RTC1
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector84) {

  OSAL_IRQ_PROLOGUE();

  if (NRF_RTC1->EVENTS_COMPARE[0]) {
    NRF_RTC1->EVENTS_COMPARE[0] = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_RTC1->EVENTS_COMPARE[0];
#endif
    
    osalSysLockFromISR();
    osalOsTimerHandlerI();
    osalSysUnlockFromISR();
  }

#if OSAL_ST_RESOLUTION == 16
  if (NRF_RTC1->EVENTS_COMPARE[1]) {
    NRF_RTC1->EVENTS_COMPARE[1] = 0;
#if CORTEX_MODEL >= 4
    (void)NRF_RTC1->EVENTS_COMPARE[1];
#endif
    NRF_RTC1->TASKS_CLEAR = 1;
  }
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif
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

#if NRF5_ST_USE_RTC0 == TRUE
  /* Using RTC with prescaler */
  NRF_RTC0->TASKS_STOP  = 1;
  NRF_RTC0->PRESCALER   = (NRF5_LFCLK_FREQUENCY / OSAL_ST_FREQUENCY) - 1; 
  NRF_RTC0->EVTENCLR    = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC0->EVENTS_COMPARE[0] = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC0->EVENTS_COMPARE[0];
#endif
  NRF_RTC0->INTENSET    = RTC_INTENSET_COMPARE0_Msk;
#if OSAL_ST_RESOLUTION == 16
  NRF_RTC0->CC[1]       = 0x10000; /* 2^16 */
  NRF_RTC0->EVENTS_COMPARE[1] = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC0->EVENTS_COMPARE[1];
#endif
  NRF_RTC0->EVTENSET    = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC0->INTENSET    = RTC_INTENSET_COMPARE1_Msk;
#endif
  NRF_RTC0->TASKS_CLEAR  = 1;

    /* Start timer */
  nvicEnableVector(RTC0_IRQn, NRF5_ST_PRIORITY);
  NRF_RTC0->TASKS_START = 1;
#endif /* NRF5_ST_USE_RTC0 == TRUE */

#if NRF5_ST_USE_RTC1 == TRUE
  /* Using RTC with prescaler */
  NRF_RTC1->TASKS_STOP  = 1;
  NRF_RTC1->PRESCALER   = (NRF5_LFCLK_FREQUENCY / OSAL_ST_FREQUENCY) - 1; 
  NRF_RTC1->EVTENCLR    = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC1->EVENTS_COMPARE[0] = 0;
#if CORTEX_MODEL >= 4
  (void)NRF_RTC1->EVENTS_COMPARE[0];
#endif
  NRF_RTC1->INTENSET    = RTC_INTENSET_COMPARE0_Msk;
#if OSAL_ST_RESOLUTION == 16
  NRF_RTC1->CC[1]       = 0x10000; /* 2^16 */
  NRF_RTC1->EVENTS_COMPARE[1] = 0;
#if CORTEX_MODEL >= 4
  NRF_RTC1->EVENTS_COMPARE[1];
#endif
  NRF_RTC1->EVTENSET    = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC1->INTENSET    = RTC_INTENSET_COMPARE1_Msk;
#endif
  NRF_RTC1->TASKS_CLEAR  = 1;

  /* Start timer */
  nvicEnableVector(RTC1_IRQn, NRF5_ST_PRIORITY);
  NRF_RTC1->TASKS_START = 1;
#endif /* NRF5_ST_USE_RTC1 == TRUE */

#endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */

#if OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC

#if NRF5_ST_USE_RTC0 == TRUE
  /* Using RTC with prescaler */
  NRF_RTC0->TASKS_STOP  = 1;
  NRF_RTC0->PRESCALER   = (NRF5_LFCLK_FREQUENCY / OSAL_ST_FREQUENCY) - 1; 
  NRF_RTC0->INTENSET    = RTC_INTENSET_TICK_Msk;

  /* Start timer */
  nvicEnableVector(RTC0_IRQn, NRF5_ST_PRIORITY);
  NRF_RTC0->TASKS_START = 1;
#endif /* NRF5_ST_USE_RTC0 == TRUE */

#if NRF5_ST_USE_RTC1 == TRUE
  /* Using RTC with prescaler */
  NRF_RTC1->TASKS_STOP  = 1;
  NRF_RTC1->PRESCALER   = (NRF5_LFCLK_FREQUENCY / OSAL_ST_FREQUENCY) - 1; 
  NRF_RTC1->INTENSET    = RTC_INTENSET_TICK_Msk;

  /* Start timer */
  nvicEnableVector(RTC1_IRQn, NRF5_ST_PRIORITY);
  NRF_RTC1->TASKS_START = 1;
#endif /* NRF5_ST_USE_RTC1 == TRUE */

#if NRF5_ST_USE_TIMER0 == TRUE
  NRF_TIMER0->TASKS_CLEAR = 1;

  /*
   * Using 32-bit mode with prescaler 1/16 configures this
   * timer with a 1MHz clock, reducing power consumption.
   */
  NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit;
  NRF_TIMER0->PRESCALER = 4;

  /*
   * Configure timer 0 compare capture 0 to generate interrupt
   * and clear timer value when event is generated.
   */
  NRF_TIMER0->CC[0] = (1000000 / OSAL_ST_FREQUENCY) - 1;
  NRF_TIMER0->SHORTS = 1;
  NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

  /* Start timer */
  nvicEnableVector(TIMER0_IRQn, NRF5_ST_PRIORITY);
  NRF_TIMER0->TASKS_START = 1;
#endif /* NRF5_ST_USE_TIMER0 == TRUE */

#endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
