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
 * @file    nilcore_v6m.c
 * @brief   ARMv6-M architecture port code.
 *
 * @addtogroup ARMCMx_V6M_CORE
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
/* Module interrupt handlers.                                                */
/*===========================================================================*/

#if (CORTEX_ALTERNATE_SWITCH == FALSE) || defined(__DOXYGEN__)
/**
 * @brief   NMI vector.
 * @details The NMI vector is used for exception mode re-entering after a
 *          context switch.
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void NMI_Handler(void) {
/*lint -restore*/

  /* The port_extctx structure is pointed by the PSP register.*/
  struct port_extctx *ctxp = (struct port_extctx *)__get_PSP();

  /* Discarding the current exception context and positioning the stack to
     point to the real one.*/
  ctxp++;

  /* Writing back the modified PSP value.*/
  __set_PSP((uint32_t)ctxp);

  /* Restoring the normal interrupts status.*/
  port_unlock_from_isr();
}
#endif /* !CORTEX_ALTERNATE_SWITCH */

#if (CORTEX_ALTERNATE_SWITCH == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   PendSV vector.
 * @details The PendSV vector is used for exception mode re-entering after a
 *          context switch.
 */
/*lint -save -e9075 [8.4] All symbols are invoked from asm context.*/
void PendSV_Handler(void) {
/*lint -restore*/

  /* The port_extctx structure is pointed by the PSP register.*/
  struct port_extctx *ctxp = (struct port_extctx *)__get_PSP();

  /* Discarding the current exception context and positioning the stack to
     point to the real one.*/
  ctxp++;

  /* Writing back the modified PSP value.*/
  __set_PSP((uint32_t)ctxp);
}
#endif /* CORTEX_ALTERNATE_SWITCH */

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   IRQ epilogue code.
 *
 * @param[in] lr        value of the @p LR register on ISR entry
 */
void _port_irq_epilogue(regarm_t lr) {

  if (lr != (regarm_t)0xFFFFFFF1U) {
    struct port_extctx *ctxp;

    port_lock_from_isr();

    /* The extctx structure is pointed by the PSP register.*/
    ctxp = (struct port_extctx *)__get_PSP();

    /* Adding an artificial exception return context, there is no need to
       populate it fully.*/
    ctxp--;

    /* Writing back the modified PSP value.*/
    __set_PSP((uint32_t)ctxp);

    /* Setting up a fake XPSR register value.*/
    ctxp->xpsr = (regarm_t)0x01000000;

    /* The exit sequence is different depending on if a preemption is
       required or not.*/
    if (chSchIsRescRequiredI()) {
      /* Preemption is required we need to enforce a context switch.*/
      ctxp->pc = (regarm_t)_port_switch_from_isr;
    }
    else {
      /* Preemption not required, we just need to exit the exception
         atomically.*/
      ctxp->pc = (regarm_t)_port_exit_from_isr;
    }

    /* Note, returning without unlocking is intentional, this is done in
       order to keep the rest of the context switch atomic.*/
  }
}

/** @} */
