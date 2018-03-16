/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    templates/nilcore.c
 * @brief   Port code.
 *
 * @addtogroup NIL_CORE
 * @{
 */

#include "nil.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

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
 */
#if !defined(__DOXYGEN__)
__attribute__((naked))
#endif
void port_dummy1(void) {

  asm (".global _port_switch");
  asm ("_port_switch:");
  asm ("subi        %sp, %sp, 80");     /* Size of the intctx structure.    */
  asm ("mflr        %r0");
  asm ("stw         %r0, 84(%sp)");     /* LR into the caller frame.        */
  asm ("mfcr        %r0");
  asm ("stw         %r0, 0(%sp)");      /* CR.                              */
  asm ("stmw        %r14, 4(%sp)");     /* GPR14...GPR31.                   */

  asm ("stw         %sp, 0(%r4)");      /* Store swapped-out stack.         */
  asm ("lwz         %sp, 0(%r3)");      /* Load swapped-in stack.           */

  asm ("lmw         %r14, 4(%sp)");     /* GPR14...GPR31.                   */
  asm ("lwz         %r0, 0(%sp)");      /* CR.                              */
  asm ("mtcr        %r0");
  asm ("lwz         %r0, 84(%sp)");     /* LR from the caller frame.        */
  asm ("mtlr        %r0");
  asm ("addi        %sp, %sp, 80");     /* Size of the intctx structure.    */
  asm ("blr");
}

/**
 * @brief   Start a thread by invoking its work function.
 * @details If the work function returns @p chThdExit() is automatically
 *          invoked.
 */
#if !defined(__DOXYGEN__)
__attribute__((naked))
#endif
void port_dummy2(void) {

  asm (".global _port_thread_start");
  asm ("_port_thread_start:");
  chSysUnlock();
  asm ("mr          %r3, %r31");        /* Thread parameter.                */
  asm ("mtctr       %r30");
  asm ("bctrl");                        /* Invoke thread function.          */
  asm ("li          %r0, 0");
  asm ("bl          chSysHalt");        /* Thread termination on exit.      */
}

/** @} */
