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
 * @file    chmboxes.h
 * @brief   Mailboxes macros and structures.
 *
 * @addtogroup mailboxes
 * @{
 */

#ifndef _CHMBOXES_H_
#define _CHMBOXES_H_

#if (CH_CFG_USE_MAILBOXES == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if CH_CFG_USE_SEMAPHORES == FALSE
#error "CH_CFG_USE_MAILBOXES requires CH_CFG_USE_SEMAPHORES"
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Structure representing a mailbox object.
 */
typedef struct {
  msg_t                 *mb_buffer;     /**< @brief Pointer to the mailbox
                                                    buffer.                 */
  msg_t                 *mb_top;        /**< @brief Pointer to the location
                                                    after the buffer.       */
  msg_t                 *mb_wrptr;      /**< @brief Write pointer.          */
  msg_t                 *mb_rdptr;      /**< @brief Read pointer.           */
  semaphore_t           mb_fullsem;     /**< @brief Full counter
                                                    @p semaphore_t.         */
  semaphore_t           mb_emptysem;    /**< @brief Empty counter
                                                    @p semaphore_t.         */
} mailbox_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Data part of a static mailbox initializer.
 * @details This macro should be used when statically initializing a
 *          mailbox that is part of a bigger structure.
 *
 * @param[in] name      the name of the mailbox variable
 * @param[in] buffer    pointer to the mailbox buffer area
 * @param[in] size      size of the mailbox buffer area
 */
#define _MAILBOX_DATA(name, buffer, size) {                             \
  (msg_t *)(buffer),                                                    \
  (msg_t *)(buffer) + size,                                             \
  (msg_t *)(buffer),                                                    \
  (msg_t *)(buffer),                                                    \
  _SEMAPHORE_DATA(name.mb_fullsem, 0),                                  \
  _SEMAPHORE_DATA(name.mb_emptysem, size),                              \
}

/**
 * @brief   Static mailbox initializer.
 * @details Statically initialized mailboxes require no explicit
 *          initialization using @p chMBInit().
 *
 * @param[in] name      the name of the mailbox variable
 * @param[in] buffer    pointer to the mailbox buffer area
 * @param[in] size      size of the mailbox buffer area
 */
#define MAILBOX_DECL(name, buffer, size)                                \
  mailbox_t name = _MAILBOX_DATA(name, buffer, size)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void chMBObjectInit(mailbox_t *mbp, msg_t *buf, cnt_t n);
  void chMBReset(mailbox_t *mbp);
  void chMBResetI(mailbox_t *mbp);
  msg_t chMBPost(mailbox_t *mbp, msg_t msg, systime_t timeout);
  msg_t chMBPostS(mailbox_t *mbp, msg_t msg, systime_t timeout);
  msg_t chMBPostI(mailbox_t *mbp, msg_t msg);
  msg_t chMBPostAhead(mailbox_t *mbp, msg_t msg, systime_t timeout);
  msg_t chMBPostAheadS(mailbox_t *mbp, msg_t msg, systime_t timeout);
  msg_t chMBPostAheadI(mailbox_t *mbp, msg_t msg);
  msg_t chMBFetch(mailbox_t *mbp, msg_t *msgp, systime_t timeout);
  msg_t chMBFetchS(mailbox_t *mbp, msg_t *msgp, systime_t timeout);
  msg_t chMBFetchI(mailbox_t *mbp, msg_t *msgp);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Returns the mailbox buffer size.
 *
 * @param[in] mbp       the pointer to an initialized mailbox_t object
 * @return              The size of the mailbox.
 *
 * @iclass
 */
static inline size_t chMBGetSizeI(mailbox_t *mbp) {

  /*lint -save -e9033 [10.8] Perfectly safe pointers
    arithmetic.*/
  return (size_t)(mbp->mb_top - mbp->mb_buffer);
  /*lint -restore*/
}

/**
 * @brief   Returns the number of free message slots into a mailbox.
 * @note    Can be invoked in any system state but if invoked out of a locked
 *          state then the returned value may change after reading.
 * @note    The returned value can be less than zero when there are waiting
 *          threads on the internal semaphore.
 *
 * @param[in] mbp       the pointer to an initialized mailbox_t object
 * @return              The number of empty message slots.
 *
 * @iclass
 */
static inline cnt_t chMBGetFreeCountI(mailbox_t *mbp) {

  chDbgCheckClassI();

  return chSemGetCounterI(&mbp->mb_emptysem);
}

/**
 * @brief   Returns the number of used message slots into a mailbox.
 * @note    Can be invoked in any system state but if invoked out of a locked
 *          state then the returned value may change after reading.
 * @note    The returned value can be less than zero when there are waiting
 *          threads on the internal semaphore.
 *
 * @param[in] mbp       the pointer to an initialized mailbox_t object
 * @return              The number of queued messages.
 *
 * @iclass
 */
static inline cnt_t chMBGetUsedCountI(mailbox_t *mbp) {

  chDbgCheckClassI();

  return chSemGetCounterI(&mbp->mb_fullsem);
}

/**
 * @brief   Returns the next message in the queue without removing it.
 * @pre     A message must be waiting in the queue for this function to work
 *          or it would return garbage. The correct way to use this macro is
 *          to use @p chMBGetFullCountI() and then use this macro, all within
 *          a lock state.
 *
 * @param[in] mbp       the pointer to an initialized mailbox_t object
 * @return              The next message in queue.
 *
 * @iclass
 */
static inline msg_t chMBPeekI(mailbox_t *mbp) {

  chDbgCheckClassI();

  return *mbp->mb_rdptr;
}

#endif /* CH_CFG_USE_MAILBOXES == TRUE */

#endif /* _CHMBOXES_H_ */

/** @} */
