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
 * @file    e200/compilers/GCC/chcoreasm.s
 * @brief   Power Architecture port low level code.
 *
 * @addtogroup PPC_GCC_CORE
 * @{
 */

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

#if !defined(FALSE) || defined(__DOXYGEN__)
#define FALSE                               0
#endif

#if !defined(TRUE) || defined(__DOXYGEN__)
#define TRUE                                1
#endif

/*===========================================================================*/
/* Code section.                                                             */
/*===========================================================================*/

#define _FROM_ASM_
#include "chconf.h"
#include "chcore.h"

#if !defined(__DOXYGEN__)

#if PPC_USE_VLE == TRUE
        .section    .text_vle, "ax"
#else
        .section    .text, "ax"
#endif

        .align		2
        .globl      _port_switch
        .type       _port_switch, @function
_port_switch:
        subi        %sp, %sp, 80
        mflr        %r0
        stw         %r0, 84(%sp)
        mfcr        %r0
        stw         %r0, 0(%sp)
        stmw        %r14, 4(%sp)

        stw         %sp, 12(%r4)
        lwz         %sp, 12(%r3)

        lmw         %r14, 4(%sp)
        lwz         %r0, 0(%sp)
        mtcr        %r0
        lwz         %r0, 84(%sp)
        mtlr        %r0
        addi        %sp, %sp, 80
        blr

        .align      2
        .globl      _port_thread_start
        .type       _port_thread_start, @function
_port_thread_start:
#if CH_DBG_SYSTEM_STATE_CHECK
        bl          _dbg_check_unlock
#endif
#if CH_DBG_STATISTICS
        bl          _stats_stop_measure_crit_thd
#endif
        wrteei      1
        mr          %r3, %r31
        mtctr       %r30
        bctrl
        li          %r0, 0
        bl          chThdExit

#endif /* !defined(__DOXYGEN__) */

/** @} */
