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
 * @file    chmsg.h
 * @brief   Messages macros and structures.
 *
 * @addtogroup messages
 * @{
 */

#ifndef _CHMSG_H_
#define _CHMSG_H_

#if (CH_CFG_USE_MESSAGES == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  msg_t chMsgSend(thread_t *tp, msg_t msg);
  thread_t * chMsgWait(void);
  void chMsgRelease(thread_t *tp, msg_t msg);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief   Evaluates to @p true if the thread has pending messages.
 *
 * @param[in] tp        pointer to the thread
 * @return              The pending messages status.
 *
 * @iclass
 */
static inline bool chMsgIsPendingI(thread_t *tp) {

  chDbgCheckClassI();

  return (bool)(tp->p_msgqueue.p_next != (thread_t *)&tp->p_msgqueue);
}

/**
 * @brief   Returns the message carried by the specified thread.
 * @pre     This function must be invoked immediately after exiting a call
 *          to @p chMsgWait().
 *
 * @param[in] tp        pointer to the thread
 * @return              The message carried by the sender.
 *
 * @api
 */
static inline msg_t chMsgGet(thread_t *tp) {

  return tp->p_msg;
}

/**
 * @brief   Releases the thread waiting on top of the messages queue.
 * @pre     Invoke this function only after a message has been received
 *          using @p chMsgWait().
 *
 * @param[in] tp        pointer to the thread
 * @param[in] msg       message to be returned to the sender
 *
 * @sclass
 */
static inline void chMsgReleaseS(thread_t *tp, msg_t msg) {

  chDbgCheckClassS();

  chSchWakeupS(tp, msg);
}

#endif /* CH_CFG_USE_MESSAGES == TRUE */

#endif /* _CHMSG_H_ */

/** @} */
