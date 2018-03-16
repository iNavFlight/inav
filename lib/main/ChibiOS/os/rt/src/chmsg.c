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
 * @file    chmsg.c
 * @brief   Messages code.
 *
 * @addtogroup messages
 * @details Synchronous inter-thread messages APIs and services.
 *          <h2>Operation Mode</h2>
 *          Synchronous messages are an easy to use and fast IPC mechanism,
 *          threads can both act as message servers and/or message clients,
 *          the mechanism allows data to be carried in both directions. Note
 *          that messages are not copied between the client and server threads
 *          but just a pointer passed so the exchange is very time
 *          efficient.<br>
 *          Messages are scalar data types of type @p msg_t that are guaranteed
 *          to be size compatible with data pointers. Note that on some
 *          architectures function pointers can be larger that @p msg_t.<br>
 *          Messages are usually processed in FIFO order but it is possible to
 *          process them in priority order by enabling the
 *          @p CH_CFG_USE_MESSAGES_PRIORITY option in @p chconf.h.<br>
 * @pre     In order to use the message APIs the @p CH_CFG_USE_MESSAGES option
 *          must be enabled in @p chconf.h.
 * @post    Enabling messages requires 6-12 (depending on the architecture)
 *          extra bytes in the @p thread_t structure.
 * @{
 */

#include "ch.h"

#if (CH_CFG_USE_MESSAGES == TRUE) || defined(__DOXYGEN__)

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

#if CH_CFG_USE_MESSAGES_PRIORITY == TRUE
#define msg_insert(tp, qp) queue_prio_insert(tp, qp)
#else
#define msg_insert(tp, qp) queue_insert(tp, qp)
#endif

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Sends a message to the specified thread.
 * @details The sender is stopped until the receiver executes a
 *          @p chMsgRelease()after receiving the message.
 *
 * @param[in] tp        the pointer to the thread
 * @param[in] msg       the message
 * @return              The answer message from @p chMsgRelease().
 *
 * @api
 */
msg_t chMsgSend(thread_t *tp, msg_t msg) {
  thread_t *ctp = currp;

  chDbgCheck(tp != NULL);

  chSysLock();
  ctp->p_msg = msg;
  ctp->p_u.wtobjp = &tp->p_msgqueue;
  msg_insert(ctp, &tp->p_msgqueue);
  if (tp->p_state == CH_STATE_WTMSG) {
    (void) chSchReadyI(tp);
  }
  chSchGoSleepS(CH_STATE_SNDMSGQ);
  msg = ctp->p_u.rdymsg;
  chSysUnlock();

  return msg;
}

/**
 * @brief   Suspends the thread and waits for an incoming message.
 * @post    After receiving a message the function @p chMsgGet() must be
 *          called in order to retrieve the message and then @p chMsgRelease()
 *          must be invoked in order to acknowledge the reception and send
 *          the answer.
 * @note    If the message is a pointer then you can assume that the data
 *          pointed by the message is stable until you invoke @p chMsgRelease()
 *          because the sending thread is suspended until then.
 *
 * @return              A reference to the thread carrying the message.
 *
 * @api
 */
thread_t *chMsgWait(void) {
  thread_t *tp;

  chSysLock();
  if (!chMsgIsPendingI(currp)) {
    chSchGoSleepS(CH_STATE_WTMSG);
  }
  tp = queue_fifo_remove(&currp->p_msgqueue);
  tp->p_state = CH_STATE_SNDMSG;
  chSysUnlock();

  return tp;
}

/**
 * @brief   Releases a sender thread specifying a response message.
 * @pre     Invoke this function only after a message has been received
 *          using @p chMsgWait().
 *
 * @param[in] tp        pointer to the thread
 * @param[in] msg       message to be returned to the sender
 *
 * @api
 */
void chMsgRelease(thread_t *tp, msg_t msg) {

  chSysLock();
  chDbgAssert(tp->p_state == CH_STATE_SNDMSG, "invalid state");
  chMsgReleaseS(tp, msg);
  chSysUnlock();
}

#endif /* CH_CFG_USE_MESSAGES == TRUE */

/** @} */
