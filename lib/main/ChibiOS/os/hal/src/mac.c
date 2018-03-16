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
 * @file    mac.c
 * @brief   MAC Driver code.
 *
 * @addtogroup MAC
 * @{
 */

#include "hal.h"

#if (HAL_USE_MAC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if (MAC_USE_ZERO_COPY == TRUE) && (MAC_SUPPORTS_ZERO_COPY == FALSE)
#error "MAC_USE_ZERO_COPY not supported by this implementation"
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   MAC Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void macInit(void) {

  mac_lld_init();
}

/**
 * @brief   Initialize the standard part of a @p MACDriver structure.
 *
 * @param[out] macp     pointer to the @p MACDriver object
 *
 * @init
 */
void macObjectInit(MACDriver *macp) {

  macp->state  = MAC_STOP;
  macp->config = NULL;
  osalThreadQueueObjectInit(&macp->tdqueue);
  osalThreadQueueObjectInit(&macp->rdqueue);
#if MAC_USE_EVENTS == TRUE
  osalEventObjectInit(&macp->rdevent);
#endif
}

/**
 * @brief   Configures and activates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[in] config    pointer to the @p MACConfig object
 *
 * @api
 */
void macStart(MACDriver *macp, const MACConfig *config) {

  osalDbgCheck((macp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert(macp->state == MAC_STOP,
                "invalid state");
  macp->config = config;
  mac_lld_start(macp);
  macp->state = MAC_ACTIVE;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the MAC peripheral.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 *
 * @api
 */
void macStop(MACDriver *macp) {

  osalDbgCheck(macp != NULL);

  osalSysLock();
  osalDbgAssert((macp->state == MAC_STOP) || (macp->state == MAC_ACTIVE),
                "invalid state");
  mac_lld_stop(macp);
  macp->state = MAC_STOP;
  osalSysUnlock();
}

/**
 * @brief   Allocates a transmission descriptor.
 * @details One of the available transmission descriptors is locked and
 *          returned. If a descriptor is not currently available then the
 *          invoking thread is queued until one is freed.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[out] tdp      pointer to a @p MACTransmitDescriptor structure
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       the descriptor was obtained.
 * @retval MSG_TIMEOUT  the operation timed out, descriptor not initialized.
 *
 * @api
 */
msg_t macWaitTransmitDescriptor(MACDriver *macp,
                                MACTransmitDescriptor *tdp,
                                systime_t timeout) {
  msg_t msg;
  systime_t now;

  osalDbgCheck((macp != NULL) && (tdp != NULL));
  osalDbgAssert(macp->state == MAC_ACTIVE, "not active");

  while (((msg = mac_lld_get_transmit_descriptor(macp, tdp)) != MSG_OK) &&
         (timeout > (systime_t)0)) {
    osalSysLock();
    now = osalOsGetSystemTimeX();
    msg = osalThreadEnqueueTimeoutS(&macp->tdqueue, timeout);
    if (msg == MSG_TIMEOUT) {
      osalSysUnlock();
      break;
    }
    if (timeout != TIME_INFINITE) {
      timeout -= (osalOsGetSystemTimeX() - now);
    }
    osalSysUnlock();
  }
  return msg;
}

/**
 * @brief   Releases a transmit descriptor and starts the transmission of the
 *          enqueued data as a single frame.
 *
 * @param[in] tdp       the pointer to the @p MACTransmitDescriptor structure
 *
 * @api
 */
void macReleaseTransmitDescriptor(MACTransmitDescriptor *tdp) {

  osalDbgCheck(tdp != NULL);

  mac_lld_release_transmit_descriptor(tdp);
}

/**
 * @brief   Waits for a received frame.
 * @details Stops until a frame is received and buffered. If a frame is
 *          not immediately available then the invoking thread is queued
 *          until one is received.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @param[out] rdp      pointer to a @p MACReceiveDescriptor structure
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_IMMEDIATE immediate timeout.
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       the descriptor was obtained.
 * @retval MSG_TIMEOUT  the operation timed out, descriptor not initialized.
 *
 * @api
 */
msg_t macWaitReceiveDescriptor(MACDriver *macp,
                               MACReceiveDescriptor *rdp,
                               systime_t timeout) {
  msg_t msg;
  systime_t now;

  osalDbgCheck((macp != NULL) && (rdp != NULL));
  osalDbgAssert(macp->state == MAC_ACTIVE, "not active");

  while (((msg = mac_lld_get_receive_descriptor(macp, rdp)) != MSG_OK) &&
         (timeout > (systime_t)0)) {
    osalSysLock();
    now = osalOsGetSystemTimeX();
    msg = osalThreadEnqueueTimeoutS(&macp->rdqueue, timeout);
    if (msg == MSG_TIMEOUT) {
      osalSysUnlock();
      break;
    }
    if (timeout != TIME_INFINITE) {
      timeout -= (osalOsGetSystemTimeX() - now);
    }
    osalSysUnlock();
  }
  return msg;
}

/**
 * @brief   Releases a receive descriptor.
 * @details The descriptor and its buffer are made available for more incoming
 *          frames.
 *
 * @param[in] rdp       the pointer to the @p MACReceiveDescriptor structure
 *
 * @api
 */
void macReleaseReceiveDescriptor(MACReceiveDescriptor *rdp) {

  osalDbgCheck(rdp != NULL);

  mac_lld_release_receive_descriptor(rdp);
}

/**
 * @brief   Updates and returns the link status.
 *
 * @param[in] macp      pointer to the @p MACDriver object
 * @return              The link status.
 * @retval true         if the link is active.
 * @retval false        if the link is down.
 *
 * @api
 */
bool macPollLinkStatus(MACDriver *macp) {

  osalDbgCheck(macp != NULL);
  osalDbgAssert(macp->state == MAC_ACTIVE, "not active");

  return mac_lld_poll_link_status(macp);
}

#endif /* HAL_USE_MAC == TRUE */

/** @} */
