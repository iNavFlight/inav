/*
    ChibiOS - Copyright (C) 2016 Jonathan Struebel

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
 * @file    hal_usb_hid.c
 * @brief   USB HID Driver code.
 *
 * @addtogroup USB_HID
 * @{
 */

#include "hal.h"

#if (HAL_USE_USB_HID == TRUE) || defined(__DOXYGEN__)

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

static uint16_t get_hword(uint8_t *p) {
  uint16_t hw;

  hw  = (uint16_t)*p++;
  hw |= (uint16_t)*p << 8U;
  return hw;
}

/*
 * Interface implementation.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return obqWriteTimeout(&((USBHIDDriver *)ip)->obqueue, bp,
                         n, TIME_INFINITE);
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((USBHIDDriver *)ip)->ibqueue, bp,
                        n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return obqPutTimeout(&((USBHIDDriver *)ip)->obqueue, b, TIME_INFINITE);
}

static msg_t get(void *ip) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((USBHIDDriver *)ip)->ibqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return obqPutTimeout(&((USBHIDDriver *)ip)->obqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((USBHIDDriver *)ip)->ibqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return obqWriteTimeout(&((USBHIDDriver *)ip)->obqueue, bp, n, timeout);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((USBHIDDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((USBHIDDriver *)ip)->ibqueue, bp, n, timeout);
}

static msg_t ctl(void *ip, unsigned int operation, void *arg) {
  (void)ip;
  (void)operation;
  (void)arg;
  return MSG_OK;
}

static void flush(void *ip) {

  obqFlush(&((USBHIDDriver *)ip)->obqueue);
}

static const struct USBHIDDriverVMT vmt = {
  (size_t)0,
  write, read, put, get,
  putt, gett, writet, readt,
  ctl, flush
};

/**
 * @brief   Notification of empty buffer released into the input buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void ibnotify(io_buffers_queue_t *bqp) {
  USBHIDDriver *uhdp = bqGetLinkX(bqp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(uhdp->config->usbp) != USB_ACTIVE) ||
      (uhdp->state != HID_READY)) {
    return;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (!usbGetReceiveStatusI(uhdp->config->usbp, uhdp->config->int_out)) {
    /* Trying to get a free buffer.*/
    uint8_t *buf = ibqGetEmptyBufferI(&uhdp->ibqueue);
    if (buf != NULL) {
      /* Buffer found, starting a new transaction.*/
      usbStartReceiveI(uhdp->config->usbp, uhdp->config->int_out,
                       buf, SERIAL_USB_BUFFERS_SIZE);
    }
  }
}

/**
 * @brief   Notification of filled buffer inserted into the output buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void obnotify(io_buffers_queue_t *bqp) {
  size_t n;
  USBHIDDriver *uhdp = bqGetLinkX(bqp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(uhdp->config->usbp) != USB_ACTIVE) ||
      (uhdp->state != HID_READY)) {
    return;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (!usbGetTransmitStatusI(uhdp->config->usbp, uhdp->config->int_in)) {
    /* Trying to get a full buffer.*/
    uint8_t *buf = obqGetFullBufferI(&uhdp->obqueue, &n);
    if (buf != NULL) {
      /* Buffer found, starting a new transaction.*/
      usbStartTransmitI(uhdp->config->usbp, uhdp->config->int_in, buf, n);
    }
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   USB HID Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void hidInit(void) {
}

/**
 * @brief   Initializes a generic full duplex USB HID driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] uhdp     pointer to a @p USBHIDDriver structure
 *
 * @init
 */
void hidObjectInit(USBHIDDriver *uhdp) {

  uhdp->vmt = &vmt;
  osalEventObjectInit(&uhdp->event);
  uhdp->state = HID_STOP;
  ibqObjectInit(&uhdp->ibqueue, true, uhdp->ib,
                USB_HID_BUFFERS_SIZE, USB_HID_BUFFERS_NUMBER,
                ibnotify, uhdp);
  obqObjectInit(&uhdp->obqueue, true, uhdp->ob,
                USB_HID_BUFFERS_SIZE, USB_HID_BUFFERS_NUMBER,
                obnotify, uhdp);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] uhdp      pointer to a @p USBHIDDriver object
 * @param[in] config    the USB HID driver configuration
 *
 * @api
 */
void hidStart(USBHIDDriver *uhdp, const USBHIDConfig *config) {
  USBDriver *usbp = config->usbp;

  osalDbgCheck(uhdp != NULL);

  osalSysLock();
  osalDbgAssert((uhdp->state == HID_STOP) || (uhdp->state == HID_READY),
                "invalid state");
  usbp->in_params[config->int_in - 1U]   = uhdp;
  usbp->out_params[config->int_out - 1U] = uhdp;
  uhdp->config = config;
  uhdp->state = HID_READY;
  osalSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p MSG_RESET.
 *
 * @param[in] uhdp      pointer to a @p USBHIDDriver object
 *
 * @api
 */
void hidStop(USBHIDDriver *uhdp) {
  USBDriver *usbp = uhdp->config->usbp;

  osalDbgCheck(uhdp != NULL);

  osalSysLock();
  osalDbgAssert((uhdp->state == HID_STOP) || (uhdp->state == HID_READY),
                "invalid state");

  /* Driver in stopped state.*/
  usbp->in_params[uhdp->config->int_in - 1U]   = NULL;
  usbp->out_params[uhdp->config->int_out - 1U] = NULL;
  uhdp->state = HID_STOP;

  /* Enforces a disconnection.*/
  hidDisconnectI(uhdp);
  osalOsRescheduleS();
  osalSysUnlock();
}

/**
 * @brief   USB device disconnection handler.
 * @note    If this function is not called from an ISR then an explicit call
 *          to @p osalOsRescheduleS() in necessary afterward.
 *
 * @param[in] uhdp      pointer to a @p USBHIDDriver object
 *
 * @iclass
 */
void hidDisconnectI(USBHIDDriver *uhdp) {

  /* Queues reset in order to signal the driver stop to the application.*/
  chnAddFlagsI(uhdp, CHN_DISCONNECTED);
  ibqResetI(&uhdp->ibqueue);
  obqResetI(&uhdp->obqueue);
}

/**
 * @brief   USB device configured handler.
 *
 * @param[in] uhdp      pointer to a @p USBHIDDriver object
 *
 * @iclass
 */
void hidConfigureHookI(USBHIDDriver *uhdp) {
  uint8_t *buf;

  ibqResetI(&uhdp->ibqueue);
  obqResetI(&uhdp->obqueue);
  chnAddFlagsI(uhdp, CHN_CONNECTED);

  /* Starts the first OUT transaction immediately.*/
  buf = ibqGetEmptyBufferI(&uhdp->ibqueue);

  osalDbgAssert(buf != NULL, "no free buffer");

  usbStartReceiveI(uhdp->config->usbp, uhdp->config->int_out,
                   buf, USB_HID_BUFFERS_SIZE);
}

/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the USB HID driver can use
 *          this function at the end of the application specific
 *          requests hook. The HID_* requests handled here do not
 *          transfer any data to the application.
 *          The following requests are handled:
 *          - HID_GET_IDLE.
 *          - HID_GET_PROTOCOL.
 *          - HID_SET_REPORT.
 *          - HID_SET_IDLE.
 *          - HID_SET_PROTOCOL.
 *          - USB_REQ_GET_DESCRIPTOR.
 *          .
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The hook status.
 * @retval true         Message handled internally.
 * @retval false        Message not handled.
 */
bool hidRequestsHook(USBDriver *usbp) {
  const USBDescriptor *dp;

  if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) {
    switch (usbp->setup[1]) {
    case HID_GET_IDLE:
      usbSetupTransfer(usbp, NULL, 0, NULL);
      return true;
    case HID_GET_PROTOCOL:
      return true;
    case HID_SET_REPORT:
      usbSetupTransfer(usbp, NULL, 0, NULL);
      return true;
    case HID_SET_IDLE:
      usbSetupTransfer(usbp, NULL, 0, NULL);
      return true;
    case HID_SET_PROTOCOL:
      return true;
    default:
      return false;
    }
  }

  /* GET_DESCRIPTOR from interface not handled by default so handle it here */
  if (((usbp->setup[0] & USB_RTYPE_DIR_MASK) == USB_RTYPE_DIR_DEV2HOST) &&
      ((usbp->setup[0] & USB_RTYPE_RECIPIENT_MASK) == USB_RTYPE_RECIPIENT_INTERFACE)) {
    switch (usbp->setup[1]) {
    case USB_REQ_GET_DESCRIPTOR:
      dp = usbp->config->get_descriptor_cb(usbp, usbp->setup[3], usbp->setup[2],
                                           get_hword(&usbp->setup[4]));
      if (dp == NULL)
        return false;

      usbSetupTransfer(usbp, (uint8_t *)dp->ud_string, dp->ud_size, NULL);
      return true;
    default:
      return false;
    }
  }
  return false;
}

/**
 * @brief   Default data transmitted callback.
 * @details The application must use this function as callback for the IN
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        IN endpoint number
 */
void hidDataTransmitted(USBDriver *usbp, usbep_t ep) {
  uint8_t *buf;
  size_t n;
  USBHIDDriver *uhdp = usbp->in_params[ep - 1U];

  if (uhdp == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that space is available in the output queue.*/
  chnAddFlagsI(uhdp, CHN_OUTPUT_EMPTY);

  /* Freeing the buffer just transmitted, if it was not a zero size packet.*/
  if (usbp->epc[ep]->in_state->txsize > 0U) {
    obqReleaseEmptyBufferI(&uhdp->obqueue);
  }

  /* Checking if there is a buffer ready for transmission.*/
  buf = obqGetFullBufferI(&uhdp->obqueue, &n);

  if (buf != NULL) {
    /* The endpoint cannot be busy, we are in the context of the callback,
       so it is safe to transmit without a check.*/
    usbStartTransmitI(usbp, ep, buf, n);
  }
  else if ((usbp->epc[ep]->in_state->txsize > 0U) &&
           ((usbp->epc[ep]->in_state->txsize &
            ((size_t)usbp->epc[ep]->in_maxsize - 1U)) == 0U)) {
    /* Transmit zero sized packet in case the last one has maximum allowed
       size. Otherwise the recipient may expect more data coming soon and
       not return buffered data to app. See section 5.8.3 Bulk Transfer
       Packet Size Constraints of the USB Specification document.*/
    usbStartTransmitI(usbp, ep, usbp->setup, 0);

  }
  else {
    /* Nothing to transmit.*/
  }

  osalSysUnlockFromISR();
}

/**
 * @brief   Default data received callback.
 * @details The application must use this function as callback for the OUT
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        OUT endpoint number
 */
void hidDataReceived(USBDriver *usbp, usbep_t ep) {
  uint8_t *buf;
  USBHIDDriver *uhdp = usbp->out_params[ep - 1U];

  if (uhdp == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that data is available in the input queue.*/
  chnAddFlagsI(uhdp, CHN_INPUT_AVAILABLE);

  /* Posting the filled buffer in the queue.*/
  ibqPostFullBufferI(&uhdp->ibqueue,
                     usbGetReceiveTransactionSizeX(uhdp->config->usbp, ep));

  /* The endpoint cannot be busy, we are in the context of the callback,
     so a packet is in the buffer for sure. Trying to get a free buffer
     for the next transaction.*/
  buf = ibqGetEmptyBufferI(&uhdp->ibqueue);
  if (buf != NULL) {
    /* Buffer found, starting a new transaction.*/
    usbStartReceiveI(uhdp->config->usbp, ep, buf, USB_HID_BUFFERS_SIZE);
  }

  osalSysUnlockFromISR();
}

/**
 * @brief   Write HID Report
 * @details The function writes data from a buffer to an output queue. The
 *          operation completes when the specified amount of data has been
 *          transferred or if the queue has been reset.
 *
 * @param[in] uhdp      pointer to the @p USBHIDDriver object
 * @param[in] bp        pointer to the report data buffer
 * @param[in] n         the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @return              The number of bytes effectively transferred.
 * @retval 0            if a timeout occurred.
 *
 * @api
 */
size_t hidWriteReport(USBHIDDriver *uhdp, uint8_t *bp, size_t n) {
  size_t val;

  val = uhdp->vmt->write(uhdp, bp, n);

  if (val > 0)
    uhdp->vmt->flush(uhdp);

  return val;
}

/**
 * @brief   Write HID report with timeout
 * @details The function writes data from a buffer to an output queue. The
 *          operation completes when the specified amount of data has been
 *          transferred or after the specified timeout or if the queue has
 *          been reset.
 *
 * @param[in] uhdp      pointer to the @p USBHIDDriver object
 * @param[in] bp        pointer to the report data buffer
 * @param[in] n         the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The number of bytes effectively transferred.
 * @retval 0            if a timeout occurred.
 *
 * @api
 */
size_t hidWriteReportt(USBHIDDriver *uhdp, uint8_t *bp, size_t n, systime_t timeout) {
  size_t val;

  val = uhdp->vmt->writet(uhdp, bp, n, timeout);

  if (val > 0)
    uhdp->vmt->flush(uhdp);

  return val;
}

/**
 * @brief   Read HID report
 * @details The function reads data from an input queue into a buffer.
 *          The operation completes when the specified amount of data has been
 *          transferred or if the queue has been reset.
 *
 * @param[in]  uhdp     pointer to the @p input_buffers_queue_t object
 * @param[out] bp       pointer to the data buffer
 * @param[in]  n        the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @return              The number of bytes effectively transferred.
 * @retval 0            if a timeout occurred.
 *
 * @api
 */
size_t hidReadReport(USBHIDDriver *uhdp, uint8_t *bp, size_t n) {

  return uhdp->vmt->read(uhdp, bp, n);
}

/**
 * @brief   Read HID report with timeout
 * @details The function reads data from an input queue into a buffer.
 *          The operation completes when the specified amount of data has been
 *          transferred or after the specified timeout or if the queue has
 *          been reset.
 *
 * @param[in]  uhdp     pointer to the @p input_buffers_queue_t object
 * @param[out] bp       pointer to the data buffer
 * @param[in]  n        the maximum amount of data to be transferred, the
 *                      value 0 is reserved
 * @param[in]  timeout  the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The number of bytes effectively transferred.
 * @retval 0            if a timeout occurred.
 *
 * @api
 */
size_t hidReadReportt(USBHIDDriver *uhdp, uint8_t *bp, size_t n, systime_t timeout) {

  return uhdp->vmt->readt(uhdp, bp, n, timeout);
}

#endif /* HAL_USE_USB_HID == TRUE */

/** @} */
