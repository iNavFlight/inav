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
 * @file    hal_queues.c
 * @brief   I/O Queues code.
 *
 * @addtogroup HAL_QUEUES
 * @details Queues are mostly used in serial-like device drivers.
 *          Serial device drivers are usually designed to have a lower side
 *          (lower driver, it is usually an interrupt service routine) and an
 *          upper side (upper driver, accessed by the application threads).<br>
 *          There are several kind of queues:<br>
 *          - <b>Input queue</b>, unidirectional queue where the writer is the
 *            lower side and the reader is the upper side.
 *          - <b>Output queue</b>, unidirectional queue where the writer is the
 *            upper side and the reader is the lower side.
 *          - <b>Full duplex queue</b>, bidirectional queue. Full duplex queues
 *            are implemented by pairing an input queue and an output queue
 *            together.
 *          .
 * @{
 */

#include "hal.h"

#if !defined(_CHIBIOS_RT_) || (CH_CFG_USE_QUEUES == FALSE) ||               \
    defined(__DOXYGEN__)

/**
 * @brief   Initializes an input queue.
 * @details A Semaphore is internally initialized and works as a counter of
 *          the bytes contained in the queue.
 * @note    The callback is invoked from within the S-Locked system state.
 *
 * @param[out] iqp      pointer to an @p input_queue_t structure
 * @param[in] bp        pointer to a memory area allocated as queue buffer
 * @param[in] size      size of the queue buffer
 * @param[in] infy      pointer to a callback function that is invoked when
 *                      data is read from the queue. The value can be @p NULL.
 * @param[in] link      application defined pointer
 *
 * @init
 */
void iqObjectInit(input_queue_t *iqp, uint8_t *bp, size_t size,
                  qnotify_t infy, void *link) {

  osalThreadQueueObjectInit(&iqp->q_waiting);
  iqp->q_counter = 0;
  iqp->q_buffer  = bp;
  iqp->q_rdptr   = bp;
  iqp->q_wrptr   = bp;
  iqp->q_top     = bp + size;
  iqp->q_notify  = infy;
  iqp->q_link    = link;
}

/**
 * @brief   Resets an input queue.
 * @details All the data in the input queue is erased and lost, any waiting
 *          thread is resumed with status @p Q_RESET.
 * @note    A reset operation can be used by a low level driver in order to
 *          obtain immediate attention from the high level layers.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 *
 * @iclass
 */
void iqResetI(input_queue_t *iqp) {

  osalDbgCheckClassI();

  iqp->q_rdptr = iqp->q_buffer;
  iqp->q_wrptr = iqp->q_buffer;
  iqp->q_counter = 0;
  osalThreadDequeueAllI(&iqp->q_waiting, Q_RESET);
}

/**
 * @brief   Input queue write.
 * @details A byte value is written into the low end of an input queue.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @param[in] b         the byte value to be written in the queue
 * @return              The operation status.
 * @retval Q_OK         if the operation has been completed with success.
 * @retval Q_FULL       if the queue is full and the operation cannot be
 *                      completed.
 *
 * @iclass
 */
msg_t iqPutI(input_queue_t *iqp, uint8_t b) {

  osalDbgCheckClassI();

  if (iqIsFullI(iqp)) {
    return Q_FULL;
  }

  iqp->q_counter++;
  *iqp->q_wrptr++ = b;
  if (iqp->q_wrptr >= iqp->q_top) {
    iqp->q_wrptr = iqp->q_buffer;
  }

  osalThreadDequeueNextI(&iqp->q_waiting, Q_OK);

  return Q_OK;
}

/**
 * @brief   Input queue read with timeout.
 * @details This function reads a byte value from an input queue. If the queue
 *          is empty then the calling thread is suspended until a byte arrives
 *          in the queue or a timeout occurs.
 * @note    The callback is invoked before reading the character from the
 *          buffer or before entering the state @p THD_STATE_WTQUEUE.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              A byte value from the queue.
 * @retval Q_TIMEOUT    if the specified time expired.
 * @retval Q_RESET      if the queue has been reset.
 *
 * @api
 */
msg_t iqGetTimeout(input_queue_t *iqp, systime_t timeout) {
  uint8_t b;

  osalSysLock();
  if (iqp->q_notify != NULL) {
    iqp->q_notify(iqp);
  }

  while (iqIsEmptyI(iqp)) {
    msg_t msg = osalThreadEnqueueTimeoutS(&iqp->q_waiting, timeout);
    if (msg < Q_OK) {
      osalSysUnlock();
      return msg;
    }
  }

  iqp->q_counter--;
  b = *iqp->q_rdptr++;
  if (iqp->q_rdptr >= iqp->q_top) {
    iqp->q_rdptr = iqp->q_buffer;
  }
  osalSysUnlock();

  return (msg_t)b;
}

/**
 * @brief   Input queue read with timeout.
 * @details The function reads data from an input queue into a buffer. The
 *          operation completes when the specified amount of data has been
 *          transferred or after the specified timeout or if the queue has
 *          been reset.
 * @note    The function is not atomic, if you need atomicity it is suggested
 *          to use a semaphore or a mutex for mutual exclusion.
 * @note    The callback is invoked before reading each character from the
 *          buffer or before entering the state @p THD_STATE_WTQUEUE.
 *
 * @param[in] iqp       pointer to an @p input_queue_t structure
 * @param[out] bp       pointer to the data buffer
 * @param[in] n         the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The number of bytes effectively transferred.
 *
 * @api
 */
size_t iqReadTimeout(input_queue_t *iqp, uint8_t *bp,
                     size_t n, systime_t timeout) {
  qnotify_t nfy = iqp->q_notify;
  size_t r = 0;

  osalDbgCheck(n > 0U);

  osalSysLock();
  while (true) {
    if (nfy != NULL) {
      nfy(iqp);
    }

    while (iqIsEmptyI(iqp)) {
      if (osalThreadEnqueueTimeoutS(&iqp->q_waiting, timeout) != Q_OK) {
        osalSysUnlock();
        return r;
      }
    }

    iqp->q_counter--;
    *bp++ = *iqp->q_rdptr++;
    if (iqp->q_rdptr >= iqp->q_top) {
      iqp->q_rdptr = iqp->q_buffer;
    }
    osalSysUnlock(); /* Gives a preemption chance in a controlled point.*/

    r++;
    if (--n == 0U) {
      return r;
    }

    osalSysLock();
  }
}

/**
 * @brief   Initializes an output queue.
 * @details A Semaphore is internally initialized and works as a counter of
 *          the free bytes in the queue.
 * @note    The callback is invoked from within the S-Locked system state.
 *
 * @param[out] oqp      pointer to an @p output_queue_t structure
 * @param[in] bp        pointer to a memory area allocated as queue buffer
 * @param[in] size      size of the queue buffer
 * @param[in] onfy      pointer to a callback function that is invoked when
 *                      data is written to the queue. The value can be @p NULL.
 * @param[in] link      application defined pointer
 *
 * @init
 */
void oqObjectInit(output_queue_t *oqp, uint8_t *bp, size_t size,
                  qnotify_t onfy, void *link) {

  osalThreadQueueObjectInit(&oqp->q_waiting);
  oqp->q_counter = size;
  oqp->q_buffer  = bp;
  oqp->q_rdptr   = bp;
  oqp->q_wrptr   = bp;
  oqp->q_top     = bp + size;
  oqp->q_notify  = onfy;
  oqp->q_link    = link;
}

/**
 * @brief   Resets an output queue.
 * @details All the data in the output queue is erased and lost, any waiting
 *          thread is resumed with status @p Q_RESET.
 * @note    A reset operation can be used by a low level driver in order to
 *          obtain immediate attention from the high level layers.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 *
 * @iclass
 */
void oqResetI(output_queue_t *oqp) {

  osalDbgCheckClassI();

  oqp->q_rdptr = oqp->q_buffer;
  oqp->q_wrptr = oqp->q_buffer;
  oqp->q_counter = qSizeX(oqp);
  osalThreadDequeueAllI(&oqp->q_waiting, Q_RESET);
}

/**
 * @brief   Output queue write with timeout.
 * @details This function writes a byte value to an output queue. If the queue
 *          is full then the calling thread is suspended until there is space
 *          in the queue or a timeout occurs.
 * @note    The callback is invoked after writing the character into the
 *          buffer.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @param[in] b         the byte value to be written in the queue
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval Q_OK         if the operation succeeded.
 * @retval Q_TIMEOUT    if the specified time expired.
 * @retval Q_RESET      if the queue has been reset.
 *
 * @api
 */
msg_t oqPutTimeout(output_queue_t *oqp, uint8_t b, systime_t timeout) {

  osalSysLock();
  while (oqIsFullI(oqp)) {
    msg_t msg = osalThreadEnqueueTimeoutS(&oqp->q_waiting, timeout);
    if (msg < Q_OK) {
      osalSysUnlock();
      return msg;
    }
  }

  oqp->q_counter--;
  *oqp->q_wrptr++ = b;
  if (oqp->q_wrptr >= oqp->q_top) {
    oqp->q_wrptr = oqp->q_buffer;
  }

  if (oqp->q_notify != NULL) {
    oqp->q_notify(oqp);
  }
  osalSysUnlock();

  return Q_OK;
}

/**
 * @brief   Output queue read.
 * @details A byte value is read from the low end of an output queue.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @return              The byte value from the queue.
 * @retval Q_EMPTY      if the queue is empty.
 *
 * @iclass
 */
msg_t oqGetI(output_queue_t *oqp) {
  uint8_t b;

  osalDbgCheckClassI();

  if (oqIsEmptyI(oqp)) {
    return Q_EMPTY;
  }

  oqp->q_counter++;
  b = *oqp->q_rdptr++;
  if (oqp->q_rdptr >= oqp->q_top) {
    oqp->q_rdptr = oqp->q_buffer;
  }

  osalThreadDequeueNextI(&oqp->q_waiting, Q_OK);

  return (msg_t)b;
}

/**
 * @brief   Output queue write with timeout.
 * @details The function writes data from a buffer to an output queue. The
 *          operation completes when the specified amount of data has been
 *          transferred or after the specified timeout or if the queue has
 *          been reset.
 * @note    The function is not atomic, if you need atomicity it is suggested
 *          to use a semaphore or a mutex for mutual exclusion.
 * @note    The callback is invoked after writing each character into the
 *          buffer.
 *
 * @param[in] oqp       pointer to an @p output_queue_t structure
 * @param[in] bp        pointer to the data buffer
 * @param[in] n         the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The number of bytes effectively transferred.
 *
 * @api
 */
size_t oqWriteTimeout(output_queue_t *oqp, const uint8_t *bp,
                      size_t n, systime_t timeout) {
  qnotify_t nfy = oqp->q_notify;
  size_t w = 0;

  osalDbgCheck(n > 0U);

  osalSysLock();
  while (true) {
    while (oqIsFullI(oqp)) {
      if (osalThreadEnqueueTimeoutS(&oqp->q_waiting, timeout) != Q_OK) {
        osalSysUnlock();
        return w;
      }
    }
    oqp->q_counter--;
    *oqp->q_wrptr++ = *bp++;
    if (oqp->q_wrptr >= oqp->q_top) {
      oqp->q_wrptr = oqp->q_buffer;
    }

    if (nfy != NULL) {
      nfy(oqp);
    }
    osalSysUnlock(); /* Gives a preemption chance in a controlled point.*/

    w++;
    if (--n == 0U) {
      return w;
    }

    osalSysLock();
  }
}

#endif /* !defined(_CHIBIOS_RT_) || (CH_USE_QUEUES == FALSE) */

/** @} */
