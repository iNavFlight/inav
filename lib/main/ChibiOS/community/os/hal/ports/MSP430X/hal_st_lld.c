/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_st_lld.c
 * @brief   MSP430X ST subsystem low level driver source.
 *
 * @addtogroup ST
 * @{
 */

#include "hal.h"
#include <msp430.h>

#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined(__DOXYGEN__)
  #define MSP430X_ST_DIV_CALC(x) ((MSP430X_ST_CLK_FREQ / OSAL_ST_FREQUENCY) == x)
#endif


#if (OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC) || defined(__DOXYGEN__)
  #if ((MSP430X_ST_CLK_FREQ / OSAL_ST_FREQUENCY / 64) > MSP_TIMER_COUNTER_MAX)
    #error "Frequency too low for timer - please set OSAL_ST_FREQUENCY to a higher value"
  #endif
  
  #define MSP430X_ST_DIV_CALC(x) ((MSP430X_ST_CLK_FREQ / OSAL_ST_FREQUENCY / x) <= MSP_TIMER_COUNTER_MAX)
#endif

/* Find suitable prescaler setting */
#if MSP430X_ST_DIV_CALC(1)
  #define MSP430X_ST_DIV 1
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_0
#elif MSP430X_ST_DIV_CALC(2)
  #define MSP430X_ST_DIV 2
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_1
#elif MSP430X_ST_DIV_CALC(3)
  #define MSP430X_ST_DIV 3
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_2
#elif MSP430X_ST_DIV_CALC(4)
  #define MSP430X_ST_DIV 4
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_3
#elif MSP430X_ST_DIV_CALC(5)
  #define MSP430X_ST_DIV 5
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_4
#elif MSP430X_ST_DIV_CALC(6)
  #define MSP430X_ST_DIV 6
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_5
#elif MSP430X_ST_DIV_CALC(7)
  #define MSP430X_ST_DIV 7
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_6
#elif MSP430X_ST_DIV_CALC(8)
  #define MSP430X_ST_DIV 8
  #define MSP430X_ST_DIV_BITS ID__1
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_7
#elif MSP430X_ST_DIV_CALC(10)
  #define MSP430X_ST_DIV 10
  #define MSP430X_ST_DIV_BITS ID__2
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_4
#elif MSP430X_ST_DIV_CALC(12)
  #define MSP430X_ST_DIV 12
  #define MSP430X_ST_DIV_BITS ID__2
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_5
#elif MSP430X_ST_DIV_CALC(14)
  #define MSP430X_ST_DIV 14
  #define MSP430X_ST_DIV_BITS ID__2
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_6
#elif MSP430X_ST_DIV_CALC(16)
  #define MSP430X_ST_DIV 16
  #define MSP430X_ST_DIV_BITS ID__2
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_7
#elif MSP430X_ST_DIV_CALC(20)
  #define MSP430X_ST_DIV 20
  #define MSP430X_ST_DIV_BITS ID__4
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_4
#elif MSP430X_ST_DIV_CALC(24)
  #define MSP430X_ST_DIV 24
  #define MSP430X_ST_DIV_BITS ID__4
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_5
#elif MSP430X_ST_DIV_CALC(28)
  #define MSP430X_ST_DIV 28
  #define MSP430X_ST_DIV_BITS ID__4
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_6
#elif MSP430X_ST_DIV_CALC(32)
  #define MSP430X_ST_DIV 32
  #define MSP430X_ST_DIV_BITS ID__4
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_7
#elif MSP430X_ST_DIV_CALC(40)
  #define MSP430X_ST_DIV 40
  #define MSP430X_ST_DIV_BITS ID__8
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_4
#elif MSP430X_ST_DIV_CALC(48)
  #define MSP430X_ST_DIV 48
  #define MSP430X_ST_DIV_BITS ID__8
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_5
#elif MSP430X_ST_DIV_CALC(56)
  #define MSP430X_ST_DIV 56
  #define MSP430X_ST_DIV_BITS ID__8
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_6
#elif MSP430X_ST_DIV_CALC(64)
  #define MSP430X_ST_DIV 64
  #define MSP430X_ST_DIV_BITS ID__8
  #define MSP430X_ST_DIV_EX_BITS TAIDEX_7
#else
  #error "Error in calculating dividers - check OSAL_ST_FREQUENCY and frequency of input clock"
#endif
/* ugh never again*/
  
#if (OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC) || defined(__DOXYGEN__)
  #define MSP_TIMER_COUNTER (MSP430X_ST_CLK_FREQ / OSAL_ST_FREQUENCY / MSP430X_ST_DIV)
  #define MSP430X_ST_CLK_FREQ_ (MSP_TIMER_COUNTER * MSP430X_ST_DIV * OSAL_ST_FREQUENCY)
  #if (MSP430X_ST_CLK_FREQ != MSP430X_ST_CLK_FREQ_)
    #warning "OSAL_ST_FREQUENCY cannot be generated exactly using timer"
  #endif
  #undef MSP430X_ST_CLK_FREQ_
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

/**
 * @brief Timer handler for both modes
 */

PORT_IRQ_HANDLER( MSP430X_ST_ISR ) {
  
  OSAL_IRQ_PROLOGUE();
  
  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();
  
  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ST driver initialization.
 *
 * @notapi
 */
void st_lld_init(void) {
  #if (OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING) || defined (__DOXYGEN__)
  /* Start disabled */
  MSP430X_ST_CCR(MSP430X_ST_TIMER) = 0;
  MSP430X_ST_CCTL(MSP430X_ST_TIMER) = 0;
  MSP430X_ST_EX(MSP430X_ST_TIMER) = MSP430X_ST_DIV_EX_BITS;
  MSP430X_ST_CTL(MSP430X_ST_TIMER) = (TACLR | MC_2 | MSP430X_ST_DIV_BITS | MSP430X_ST_TASSEL);
  #endif /* OSAL_ST_MODE == OSAL_ST_MODE_FREERUNNING */
  
  #if (OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC) || defined (__DOXYGEN__)
  /* Start enabled */
  MSP430X_ST_CCR(MSP430X_ST_TIMER) = MSP_TIMER_COUNTER - 1;
  MSP430X_ST_CCTL(MSP430X_ST_TIMER) = CCIE;
  MSP430X_ST_EX(MSP430X_ST_TIMER) = MSP430X_ST_DIV_EX_BITS;
  MSP430X_ST_CTL(MSP430X_ST_TIMER) = (TACLR | MC_1 | MSP430X_ST_DIV_BITS | MSP430X_ST_TASSEL);
  #endif /* OSAL_ST_MODE == OSAL_ST_MODE_PERIODIC */
}

#endif /* OSAL_ST_MODE != OSAL_ST_MODE_NONE */

/** @} */
