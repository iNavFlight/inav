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
 * @file    serial.c
 * @brief   Serial Driver code.
 *
 * @addtogroup SERIAL
 * @{
 */

#include "hal.h"

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*
 * Interface implementation, the following functions just invoke the equivalent
 * queue-level function or macro.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {

  return oqWriteTimeout(&((SerialDriver *)ip)->oqueue, bp,
                        n, TIME_INFINITE);
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  return iqReadTimeout(&((SerialDriver *)ip)->iqueue, bp,
                       n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {

  return oqPutTimeout(&((SerialDriver *)ip)->oqueue, b, TIME_INFINITE);
}

static msg_t get(void *ip) {

  return iqGetTimeout(&((SerialDriver *)ip)->iqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  return oqPutTimeout(&((SerialDriver *)ip)->oqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  return iqGetTimeout(&((SerialDriver *)ip)->iqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t timeout) {

  return oqWriteTimeout(&((SerialDriver *)ip)->oqueue, bp, n, timeout);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t timeout) {

  return iqReadTimeout(&((SerialDriver *)ip)->iqueue, bp, n, timeout);
}

static const struct SerialDriverVMT vmt = {
  write, read, put, get,
  putt, gett, writet, readt
};

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Serial Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sdInit(void) {

  sd_lld_init();
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] sdp      pointer to a @p SerialDriver structure
 * @param[in] inotify   pointer to a callback function that is invoked when
 *                      some data is read from the Queue. The value can be
 *                      @p NULL.
 * @param[in] onotify   pointer to a callback function that is invoked when
 *                      some data is written in the Queue. The value can be
 *                      @p NULL.
 *
 * @init
 */
void sdObjectInit(SerialDriver *sdp, qnotify_t inotify, qnotify_t onotify) {

  sdp->vmt = &vmt;
  osalEventObjectInit(&sdp->event);
  sdp->state = SD_STOP;
  iqObjectInit(&sdp->iqueue, sdp->ib, SERIAL_BUFFERS_SIZE, inotify, sdp);
  oqObjectInit(&sdp->oqueue, sdp->ob, SERIAL_BUFFERS_SIZE, onotify, sdp);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 * @param[in] config    the architecture-dependent serial driver configuration.
 *                      If this parameter is set to @p NULL then a default
 *                      configuration is used.
 *
 * @api
 */
void sdStart(SerialDriver *sdp, const SerialConfig *config) {

  osalDbgCheck(sdp != NULL);

  osalSysLock();
  osalDbgAssert((sdp->state == SD_STOP) || (sdp->state == SD_READY),
                "invalid state");
  sd_lld_start(sdp, config);
  sdp->state = SD_READY;
  osalSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] sdp       pointer to a @p SerialDriver object
 *
 * @api
 */
void sdStop(SerialDriver *sdp) {

  osalDbgCheck(sdp != NULL);

  osalSysLock();
  osalDbgAssert((sdp->state == SD_STOP) || (sdp->state == SD_READY),
                "invalid state");
  sd_lld_stop(sdp);
  sdp->state = SD_STOP;
  oqResetI(&sdp->oqueue);
  iqResetI(&sdp->iqueue);
  osalOsRescheduleS();
  osalSysUnlock();
}

/**
 * @brief   Handles incoming data.
 * @details This function must be called from the input interrupt service
 *          routine in order to enqueue incoming data and generate the
 *          related events.
 * @note    The incoming data event is only generated when the input queue
 *          becomes non-empty.
 * @note    In order to gain some performance it is suggested to not use
 *          this function directly but copy this code directly into the
 *          interrupt service routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver structure
 * @param[in] b         the byte to be written in the driver's Input Queue
 *
 * @iclass
 */
void sdIncomingDataI(SerialDriver *sdp, uint8_t b) {

  osalDbgCheckClassI();
  osalDbgCheck(sdp != NULL);

  if (iqIsEmptyI(&sdp->iqueue))
    chnAddFlagsI(sdp, CHN_INPUT_AVAILABLE);
  if (iqPutI(&sdp->iqueue, b) < Q_OK)
    chnAddFlagsI(sdp, SD_OVERRUN_ERROR);
}

/**
 * @brief   Handles outgoing data.
 * @details Must be called from the output interrupt service routine in order
 *          to get the next byte to be transmitted.
 * @note    In order to gain some performance it is suggested to not use
 *          this function directly but copy this code directly into the
 *          interrupt service routine.
 *
 * @param[in] sdp       pointer to a @p SerialDriver structure
 * @return              The byte value read from the driver's output queue.
 * @retval Q_EMPTY      if the queue is empty (the lower driver usually
 *                      disables the interrupt source when this happens).
 *
 * @iclass
 */
msg_t sdRequestDataI(SerialDriver *sdp) {
  msg_t  b;

  osalDbgCheckClassI();
  osalDbgCheck(sdp != NULL);

  b = oqGetI(&sdp->oqueue);
  if (b < Q_OK)
    chnAddFlagsI(sdp, CHN_OUTPUT_EMPTY);
  return b;
}

/**
 * @brief   Direct output check on a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          checks directly the output queue. This is faster but cannot
 *          be used to check different channels implementations.
 *
 * @param[in] sdp       pointer to a @p SerialDriver structure
 * @return              The queue status.
 * @retval false        if the next write operation would not block.
 * @retval true         if the next write operation would block.
 *
 * @deprecated
 *
 * @api
 */
bool sdPutWouldBlock(SerialDriver *sdp) {
  bool b;

  osalSysLock();
  b = oqIsFullI(&sdp->oqueue);
  osalSysUnlock();

  return b;
}

/**
 * @brief   Direct input check on a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          checks directly the input queue. This is faster but cannot
 *          be used to check different channels implementations.
 *
 * @param[in] sdp       pointer to a @p SerialDriver structure
 * @return              The queue status.
 * @retval false        if the next write operation would not block.
 * @retval true         if the next write operation would block.
 *
 * @deprecated
 *
 * @api
 */
bool sdGetWouldBlock(SerialDriver *sdp) {
  bool b;

  osalSysLock();
  b = iqIsEmptyI(&sdp->iqueue);
  osalSysUnlock();

  return b;
}

#endif /* HAL_USE_SERIAL == TRUE */

/** @} */
