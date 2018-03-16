/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    hal_buffers.h
 * @brief   I/O Buffers macros and structures.
 *
 * @addtogroup HAL_BUFFERS
 * @{
 */

#ifndef _HAL_BUFFERS_H_
#define _HAL_BUFFERS_H_

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a generic queue of buffers.
 */
typedef struct io_buffers_queue io_buffers_queue_t;

/**
 * @brief   Double buffer notification callback type.
 *
 * @param[in] iodbp     the buffers queue pointer
 */
typedef void (*bqnotify_t)(io_buffers_queue_t *bqp);

/**
 * @brief   Structure of a generic buffers queue.
 */
struct io_buffers_queue {
  /**
   * @brief   Queue of waiting threads.
   */
  threads_queue_t       waiting;
  /**
   * @brief   Active buffers counter.
   */
  volatile size_t       bcounter;
  /**
   * @brief   Buffer write pointer.
   */
  uint8_t               *bwrptr;
  /**
   * @brief   Buffer read pointer.
   */
  uint8_t               *brdptr;
  /**
   * @brief   Pointer to the buffers boundary.
   */
  uint8_t               *btop;
  /**
   * @brief   Size of buffers.
   * @note    The buffer size must be not lower than <tt>sizeof(size_t) + 2</tt>
   *          because the first bytes are used to store the used size of the
   *          buffer.
   */
  size_t                bsize;
  /**
   * @brief   Number of buffers.
   */
  size_t                bn;
  /**
   * @brief   Queue of buffer objects.
   */
  uint8_t               *buffers;
  /**
   * @brief   Pointer for R/W sequential access.
   * @note    It is @p NULL if a new buffer must be fetched from the queue.
   */
  uint8_t               *ptr;
  /**
   * @brief   Boundary for R/W sequential access.
   */
  uint8_t               *top;
  /**
   * @brief   Data notification callback.
   */
  bqnotify_t            notify;
  /**
   * @brief   Application defined field.
   */
  void                  *link;
};

/**
 * @brief   Type of an input buffers queue.
 */
typedef io_buffers_queue_t input_buffers_queue_t;

/**
 * @brief   Type of an output buffers queue.
 */
typedef io_buffers_queue_t output_buffers_queue_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Computes the size of a buffers queue buffer size.
 *
 * @param[in] n         number of buffers in the queue
 * @param[in] size      size of the buffers
 */
#define BQ_BUFFER_SIZE(n, size)                                             \
  (((size_t)(size) + sizeof (size_t)) * (size_t)(n))

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Returns the queue's number of buffers.
 *
 * @param[in] bqp       pointer to an @p io_buffers_queue_t structure
 * @return              The number of buffers.
 *
 * @xclass
 */
#define bqSizeX(bqp) ((bqp)->bn)

/**
 * @brief   Return the ready buffers number.
 * @details Returns the number of filled buffers if used on an input queue
 *          or the number of empty buffers if used on an output queue.
 *
 * @param[in] bqp       pointer to an @p io_buffers_queue_t structure
 * @return              The number of ready buffers.
 *
 * @iclass
 */
#define bqSpaceI(bqp) ((bqp)->bcounter)

/**
 * @brief   Returns the queue application-defined link.
 *
 * @param[in] bqp       pointer to an @p io_buffers_queue_t structure
 * @return              The application-defined link.
 *
 * @special
 */
#define bqGetLinkX(bqp) ((bqp)->link)

/**
 * @brief   Evaluates to @p TRUE if the specified input buffers queue is empty.
 *
 * @param[in] ibqp      pointer to an @p input_buffers_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not empty.
 * @retval true         if the queue is empty.
 *
 * @iclass
 */
#define ibqIsEmptyI(ibqp) ((bool)(bqSpaceI(ibqp) == 0U))

/**
 * @brief   Evaluates to @p TRUE if the specified input buffers queue is full.
 *
 * @param[in] ibqp      pointer to an @p input_buffers_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not full.
 * @retval true         if the queue is full.
 *
 * @iclass
 */
#define ibqIsFullI(ibqp)                                                    \
  /*lint -save -e9007 [13.5] No side effects, a pointer is passed.*/        \
  ((bool)(((ibqp)->bwrptr == (ibqp)->brdptr) && ((ibqp)->bcounter != 0U)))  \
  /*lint -restore*/

/**
 * @brief   Evaluates to @p true if the specified output buffers queue is empty.
 *
 * @param[in] obqp      pointer to an @p output_buffers_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not empty.
 * @retval true         if the queue is empty.
 *
 * @iclass
 */
#define obqIsEmptyI(obqp)                                                   \
  /*lint -save -e9007 [13.5] No side effects, a pointer is passed.*/        \
  ((bool)(((obqp)->bwrptr == (obqp)->brdptr) && ((obqp)->bcounter != 0U)))  \
  /*lint -restore*/

/**
 * @brief   Evaluates to @p true if the specified output buffers queue is full.
 *
 * @param[in] obqp      pointer to an @p output_buffers_queue_t structure
 * @return              The queue status.
 * @retval false        if the queue is not full.
 * @retval true         if the queue is full.
 *
 * @iclass
 */
#define obqIsFullI(obqp) ((bool)(bqSpaceI(obqp) == 0U))
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void ibqObjectInit(input_buffers_queue_t *ibqp, uint8_t *bp,
                     size_t size, size_t n,
                     bqnotify_t infy, void *link);
  void ibqResetI(input_buffers_queue_t *ibqp);
  uint8_t *ibqGetEmptyBufferI(input_buffers_queue_t *ibqp);
  void ibqPostFullBufferI(input_buffers_queue_t *ibqp, size_t size);
  msg_t ibqGetFullBufferTimeout(input_buffers_queue_t *ibqp,
                                systime_t timeout);
  msg_t ibqGetFullBufferTimeoutS(input_buffers_queue_t *ibqp,
                                 systime_t timeout);
  void ibqReleaseEmptyBuffer(input_buffers_queue_t *ibqp);
  void ibqReleaseEmptyBufferS(input_buffers_queue_t *ibqp);
  msg_t ibqGetTimeout(input_buffers_queue_t *ibqp, systime_t timeout);
  size_t ibqReadTimeout(input_buffers_queue_t *ibqp, uint8_t *bp,
                        size_t n, systime_t timeout);
  void obqObjectInit(output_buffers_queue_t *obqp, uint8_t *bp,
                     size_t size, size_t n,
                     bqnotify_t onfy, void *link);
  void obqResetI(output_buffers_queue_t *obqp);
  uint8_t *obqGetFullBufferI(output_buffers_queue_t *obqp,
                             size_t *sizep);
  void obqReleaseEmptyBufferI(output_buffers_queue_t *obqp);
  msg_t obqGetEmptyBufferTimeout(output_buffers_queue_t *obqp,
                                 systime_t timeout);
  msg_t obqGetEmptyBufferTimeoutS(output_buffers_queue_t *obqp,
                                  systime_t timeout);
  void obqPostFullBuffer(output_buffers_queue_t *obqp, size_t size);
  void obqPostFullBufferS(output_buffers_queue_t *obqp, size_t size);
  msg_t obqPutTimeout(output_buffers_queue_t *obqp, uint8_t b,
                      systime_t timeout);
  size_t obqWriteTimeout(output_buffers_queue_t *obqp, const uint8_t *bp,
                         size_t n, systime_t timeout);
  bool obqTryFlushI(output_buffers_queue_t *obqp);
  void obqFlush(output_buffers_queue_t *obqp);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_BUFFERS_H_ */

/** @} */
