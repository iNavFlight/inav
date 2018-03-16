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
 * @file    chqueues.h
 * @brief   I/O Queues macros and structures.
 *
 * @addtogroup io_queues
 * @{
 */

#ifndef _CHQUEUES_H_
#define _CHQUEUES_H_

#if (CH_CFG_USE_QUEUES == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Queue functions returned status value
 * @{
 */
#define Q_OK            MSG_OK      /**< @brief Operation successful.       */
#define Q_TIMEOUT       MSG_TIMEOUT /**< @brief Timeout condition.          */
#define Q_RESET         MSG_RESET   /**< @brief Queue has been reset.       */
#define Q_EMPTY         (msg_t)-3   /**< @brief Queue empty.                */
#define Q_FULL          (msg_t)-4   /**< @brief Queue full,                 */
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a generic I/O queue structure.
 */
typedef struct io_queue io_queue_t;

/** @brief Queue notification callback type.*/
typedef void (*qnotify_t)(io_queue_t *qp);

/**
 * @brief   Generic I/O queue structure.
 * @details This structure represents a generic Input or Output asymmetrical
 *          queue. The queue is asymmetrical because one end is meant to be
 *          accessed from a thread context, and thus can be blocking, the other
 *          end is accessible from interrupt handlers or from within a kernel
 *          lock zone (see <b>I-Locked</b> and <b>S-Locked</b> states in
 *          @ref system_states) and is non-blocking.
 */
struct io_queue {
  threads_queue_t       q_waiting;  /**< @brief Queue of waiting threads.   */
  volatile size_t       q_counter;  /**< @brief Resources counter.          */
  uint8_t               *q_buffer;  /**< @brief Pointer to the queue buffer.*/
  uint8_t               *q_top;     /**< @brief Pointer to the first location
                                                after the buffer.           */
  uint8_t               *q_wrptr;   /**< @brief Write pointer.              */
  uint8_t               *q_rdptr;   /**< @brief Read pointer.               */
  qnotify_t             q_notify;   /**< @brief Data notification callback. */
  void                  *q_link;    /**< @brief Application defined field.  */
};

/**
 * @extends io_queue_t
 *
 * @brief   Type of an input queue structure.
 * @details This structure represents a generic asymmetrical input queue.
 *          Writing to the queue is non-blocking and can be performed from
 *          interrupt handlers or from within a kernel lock zone (see
 *          <b>I-Locked</b> and <b>S-Locked</b> states in @ref system_states).
 *          Reading the queue can be a blocking operation and is supposed to
 *          be performed by a system thread.
 */
typedef io_queue_t input_queue_t;

/**
 * @extends io_queue_t
 *
 * @brief   Type of an output queue structure.
 * @details This structure represents a generic asymmetrical output queue.
 *          Reading from the queue is non-blocking and can be performed from
 *          interrupt handlers or from within a kernel lock zone (see
 *          <b>I-Locked</b> and <b>S-Locked</b> states in @ref system_states).
 *          Writing the queue can be a blocking operation and is supposed to
 *          be performed by a system thread.
 */
typedef io_queue_t output_queue_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Data part of a static input queue initializer.
 * @details This macro should be used when statically initializing an
 *          input queue that is part of a bigger structure.
 *
 * @param[in] name      the name of the input queue variable
 * @param[in] buffer    pointer to the queue buffer area
 * @param[in] size      size of the queue buffer area
 * @param[in] inotify   input notification callback pointer
 * @param[in] link      application defined pointer
 */
#define _INPUTQUEUE_DATA(name, buffer, size, inotify, link) {               \
  _THREADS_QUEUE_DATA(name),                                                \
  0U,                                                                       \
  (uint8_t *)(buffer),                                                      \
  (uint8_t *)(buffer) + (size),                                             \
  (uint8_t *)(buffer),                                                      \
  (uint8_t *)(buffer),                                                      \
  (inotify),                                                                \
  (link)                                                                    \
}

/**
 * @brief   Static input queue initializer.
 * @details Statically initialized input queues require no explicit
 *          initialization using @p chIQInit().
 *
 * @param[in] name      the name of the input queue variable
 * @param[in] buffer    pointer to the queue buffer area
 * @param[in] size      size of the queue buffer area
 * @param[in] inotify   input notification callback pointer
 * @param[in] link      application defined pointer
 */
#define INPUTQUEUE_DECL(name, buffer, size, inotify, link)                  \
  input_queue_t name = _INPUTQUEUE_DATA(name, buffer, size, inotify, link)

/**
 * @brief   Data part of a static output queue initializer.
 * @details This macro should be used when statically initializing an
 *          output queue that is part of a bigger structure.
 *
 * @param[in] name      the name of the output queue variable
 * @param[in] buffer    pointer to the queue buffer area
 * @param[in] size      size of the queue buffer area
 * @param[in] onotify   output notification callback pointer
 * @param[in] link      application defined pointer
 */
#define _OUTPUTQUEUE_DATA(name, buffer, size, onotify, link) {              \
  _THREADS_QUEUE_DATA(name),                                                \
  (size),                                                                   \
  (uint8_t *)(buffer),                                                      \
  (uint8_t *)(buffer) + (size),                                             \
  (uint8_t *)(buffer),                                                      \
  (uint8_t *)(buffer),                                                      \
  (onotify),                                                                \
  (link)                                                                    \
}

/**
 * @brief   Static output queue initializer.
 * @details Statically initialized output queues require no explicit
 *          initialization using @p chOQInit().
 *
 * @param[in] name      the name of the output queue variable
 * @param[in] buffer    pointer to the queue buffer area
 * @param[in] size      size of the queue buffer area
 * @param[in] onotify   output notification callback pointer
 * @param[in] link      application defined pointer
 */
#define OUTPUTQUEUE_DECL(name, buffer, size, onotify, link)                 \
  output_queue_t name = _OUTPUTQUEUE_DATA(name, buffer, size, onotify, link)

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Returns the queue's buffer size.
 *
 * @param[in] qp        pointer to a @p io_queue_t structure
 * @return              The buffer size.
 *
 * @xclass
 */
#define chQSizeX(qp)                                                        \
  /*lint -save -e9033 [10.8] The cast is safe.*/                            \
  ((size_t)((qp)->q_top - (qp)->q_buffer))                                  \
  /*lint -restore*/

/**
 * @brief   Queue space.
 * @details Returns the used space if used on an input queue or the empty
 *          space if used on an output queue.
 *
 * @param[in] qp        pointer to a @p io_queue_t structure
 * @return              The buffer space.
 *
 * @iclass
 */
#define chQSpaceI(qp) ((qp)->q_counter)

/**
 * @brief   Returns the queue application-defined link.
 *
 * @param[in] qp        pointer to a @p io_queue_t structure
 * @return              The application-defined link.
 *
 * @xclass
 */
#define chQGetLinkX(qp) ((qp)->q_link)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void chIQObjectInit(input_queue_t *iqp, uint8_t *bp, size_t size,
                      qnotify_t infy, void *link);
  void chIQResetI(input_queue_t *iqp);
  msg_t chIQPutI(input_queue_t *iqp, uint8_t b);
  msg_t chIQGetTimeout(input_queue_t *iqp, systime_t timeout);
  size_t chIQReadTimeout(input_queue_t *iqp, uint8_t *bp,
                         size_t n, systime_t timeout);

  void chOQObjectInit(output_queue_t *oqp, uint8_t *bp, size_t size,
                      qnotify_t onfy, void *link);
  void chOQResetI(output_queue_t *oqp);
  msg_t chOQPutTimeout(output_queue_t *oqp, uint8_t b, systime_t timeout);
  msg_t chOQGetI(output_queue_t *oqp);
  size_t chOQWriteTimeout(output_queue_t *oqp, const uint8_t *bp,
                          size_t n, systime_t timeout);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @brief   Returns the filled space into an input queue.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @return              The number of full bytes in the queue.
 * @retval 0            if the queue is empty.
 *
 * @iclass
 */
static inline size_t chIQGetFullI(input_queue_t *iqp) {

  chDbgCheckClassI();

  return (size_t)chQSpaceI(iqp);
}

/**
 * @brief   Returns the empty space into an input queue.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @return              The number of empty bytes in the queue.
 * @retval 0            if the queue is full.
 *
 * @iclass
 */
static inline size_t chIQGetEmptyI(input_queue_t *iqp) {

  chDbgCheckClassI();

  return (size_t)(chQSizeX(iqp) - chQSpaceI(iqp));
}

/**
 * @brief   Evaluates to @p true if the specified input queue is empty.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not empty.
 * @retval true         if the queue is empty.
 *
 * @iclass
 */
static inline bool chIQIsEmptyI(input_queue_t *iqp) {

  chDbgCheckClassI();

  return (bool)(chQSpaceI(iqp) == 0U);
}

/**
 * @brief   Evaluates to @p true if the specified input queue is full.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not full.
 * @retval true         if the queue is full.
 *
 * @iclass
 */
static inline bool chIQIsFullI(input_queue_t *iqp) {

  chDbgCheckClassI();

  /*lint -save -e9007 [13.5] No side effects.*/
  return (bool)((iqp->q_wrptr == iqp->q_rdptr) && (iqp->q_counter != 0U));
  /*lint -restore*/
}

/**
 * @brief   Input queue read.
 * @details This function reads a byte value from an input queue. If the queue
 *          is empty then the calling thread is suspended until a byte arrives
 *          in the queue.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @return              A byte value from the queue.
 * @retval Q_RESET      if the queue has been reset.
 *
 * @api
 */
static inline msg_t chIQGet(input_queue_t *iqp) {

  return chIQGetTimeout(iqp, TIME_INFINITE);
}

/**
 * @brief   Returns the filled space into an output queue.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @return              The number of full bytes in the queue.
 * @retval 0            if the queue is empty.
 *
 * @iclass
 */
static inline size_t chOQGetFullI(output_queue_t *oqp) {

  chDbgCheckClassI();

  return (size_t)(chQSizeX(oqp) - chQSpaceI(oqp));
}

/**
 * @brief   Returns the empty space into an output queue.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @return              The number of empty bytes in the queue.
 * @retval 0            if the queue is full.
 *
 * @iclass
 */
static inline size_t chOQGetEmptyI(output_queue_t *oqp) {

  chDbgCheckClassI();

  return (size_t)chQSpaceI(oqp);
}

/**
 * @brief   Evaluates to @p true if the specified output queue is empty.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not empty.
 * @retval true         if the queue is empty.
 *
 * @iclass
 */
static inline bool chOQIsEmptyI(output_queue_t *oqp) {

  chDbgCheckClassI();

  /*lint -save -e9007 [13.5] No side effects.*/
  return (bool)((oqp->q_wrptr == oqp->q_rdptr) && (oqp->q_counter != 0U));
  /*lint -restore*/
}

/**
 * @brief   Evaluates to @p true if the specified output queue is full.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not full.
 * @retval true         if the queue is full.
 *
 * @iclass
 */
static inline bool chOQIsFullI(output_queue_t *oqp) {

  chDbgCheckClassI();

  return (bool)(chQSpaceI(oqp) == 0U);
}

/**
 * @brief   Output queue write.
 * @details This function writes a byte value to an output queue. If the queue
 *          is full then the calling thread is suspended until there is space
 *          in the queue.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @param[in] b         the byte value to be written in the queue
 * @return              The operation status.
 * @retval Q_OK         if the operation succeeded.
 * @retval Q_RESET      if the queue has been reset.
 *
 * @api
 */
static inline msg_t chOQPut(output_queue_t *oqp, uint8_t b) {

  return chOQPutTimeout(oqp, b, TIME_INFINITE);
}

#endif /* CH_CFG_USE_QUEUES == TRUE */

#endif /* _CHQUEUES_H_ */

/** @} */
