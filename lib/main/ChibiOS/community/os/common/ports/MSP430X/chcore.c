/*
    ChibiOS/HAL - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/nilcore.c
 * @brief   MSP430X port code.
 *
 * @addtogroup MSP430X_CORE
 * @{
 */

#include "ch.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

bool __msp430x_in_isr;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Performs a context switch between two threads.
 * @details This is the most critical code in any port, this function
 *          is responsible for the context switch between 2 threads.
 * @note    The implementation of this code affects <b>directly</b> the context
 *          switch performance so optimize here as much as you can.
 *
 * @param[in] ntp       the thread to be switched in
 * @param[in] otp       the thread to be switched out
 */
#if !(__GNUC__ < 6 && __GNUC_MINOR__ < 4) || defined(__OPTIMIZE__)
__attribute__((naked))
#endif
void _port_switch(thread_t *ntp, thread_t *otp) {
#if (__GNUC__ < 6 && __GNUC_MINOR__ < 4) && !defined(__OPTIMIZE__)
  asm volatile ("add #4, r1");
#endif
  (void)(ntp);
  (void)(otp);
#if defined(__MSP430X_LARGE__)
  asm volatile ("pushm.a #7, R10");
  asm volatile ("mova r1, @R13");
  asm volatile ("mova @R12, r1");
  asm volatile ("popm.a #7, R10");
  asm volatile ("reta");
#else
  asm volatile ("pushm.w #7, R10");
  asm volatile ("mov r1, @R13");
  asm volatile ("mov @R12, r1");
  asm volatile ("popm.w #7, R10");
  asm volatile ("ret");
#endif
}

/**
 * @brief   Start a thread by invoking its work function.
 * @details If the work function returns @p chThdExit() is automatically
 *          invoked.
 */
void _port_thread_start(void) {
  
  /* See PORT_SETUP_CONTEXT in nilcore.h */
  chSysUnlock();
#if defined(__MSP430X_LARGE__)
  asm volatile ("mova R5, R12");
  asm volatile ("calla R4");
#else
  asm volatile ("mov R5, R12");
  asm volatile ("call R4");
#endif
#if defined(_CHIBIOS_RT_CONF_)
  chThdExit(MSG_OK);
#endif
#if defined(_CHIBIOS_NIL_CONF_)
  chSysHalt(0);
#endif
}
/** @} */
