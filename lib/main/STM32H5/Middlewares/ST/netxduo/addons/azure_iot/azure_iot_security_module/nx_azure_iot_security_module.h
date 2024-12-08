/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#ifndef NX_AZURE_IOT_SECURITY_MODULE_H
#define NX_AZURE_IOT_SECURITY_MODULE_H

#include <asc_config.h>

#ifdef __ASC_CONFIG_EXCLUDE_PORT__H__
#error Platform includes error
#endif

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_api.h"
#include "nx_azure_iot.h"

/* Ensure enable ip packet filter is defined. */
#ifndef NX_ENABLE_IP_PACKET_FILTER
#error "Azure IoT Security Module requires NX_ENABLE_IP_PACKET_FILTER to be defined, enable NX_ENABLE_IP_PACKET_FILTER in `nx_user.h`."
#endif /* NX_ENABLE_IP_PACKET_FILTER */

#include "asc_security_core/core.h"
#include "asc_security_core/version.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/utils/ievent_loop.h"

/* Define AZ IoT ASC event flags. These events are processed by the Cloud thread.  */
#define AZ_IOT_SECURITY_MODULE_SEND_EVENT   ((ULONG)0x00000001)       /* Security send event. */

/**
 * @brief Azure IoT Security Module state enum
 *
 * @details State diagram:
 *
 *       +--------------+           +--------------+               +--------------+                +------+-------+
 *       |              |           |              +--------+      |              +--------+       |              |
 *       |              |           |              |        |(2)   |              |        |(4)    |              |
 *       |     NOT      |    (1)    |    PENDING   <--------+      |    ACTIVE    <--------+       |  SUSPENDED   |
 *       | INITIALIZED  +----------->              |               |              |                |              |
 *       |              |           |              +--------------->              <----------------|              |
 *       |              |           |              |      (3)      |              |       (7)      |              |
 *       +--------------+           +-+----+-------+               +------+-------+                +------+-------+
 *                                    |    ^                              |                               ^
 *                                    |    |        (5)                   |                               |
 *                                    |    +------------------------------+                  (6)          |
 *                                    +-------------------------------------------------------------------+
 *
 * (1) After registering to cloud thread.
 * (2) Collect event and waiting for established connection.
 * (3) Connection established.
 * (4) Collect and send security messages.
 * (5) No connections.
 * (6) Pending time interval exceeded.
 * (7) Connection has been established.
 *
 */
typedef enum security_module_state
{
    /* Security Module initial state. */
    SECURITY_MODULE_STATE_NOT_INITIALIZED = 0,

    /* Security Module collects security events, but skips sending to IoT Hub. */
    SECURITY_MODULE_STATE_PENDING = 1,

    /* Security Module is collecting security messages and sending them to IoT Hub. */
    SECURITY_MODULE_STATE_ACTIVE = 2,

    /* Security Module is idle, waiting for a healthy IoT Hub connection. */
    SECURITY_MODULE_STATE_SUSPENDED = 3
} security_module_state_t;

/**
 * @struct NX_AZURE_IOT_SECURITY_MODULE
 *
 * @details Azure Security Center for IoT security module provides a comprehensive security solution for Azure RTOS devices.
 *
 * Azure Security Center for IoT security module with Azure RTOS support offers the following features:
 *  - Detection of malicious network activities: Every device inbound and outbound network activity is
 *    monitored (supported protocols: TCP, UDP, ICMP on IPv4 and IPv6). Azure Security Center for IoT inspects each of
 *    these network activities against the Microsoft Threat Intelligence feed. The feed gets updated in real-time with
 *    millions of unique threat indicators collected worldwide.
 *  - Device behavior baselining based on custom alerts: Allows for clustering of devices into security groups and
 *    defining the expected behavior of each group. As IoT devices are typically designed to operate in well-defined
 *    and limited scenarios, it is easy to create a baseline that defines their expected behavior using a set of
 *    parameters. Any deviation from the baseline, triggers an alert.
 *  - Improve the security hygiene of the device: By leveraging the recommended infrastructure Azure Security Center
 *    for IoT provides, gain knowledge and insights about issues in your environment that impact and damage the
 *    security posture of your devices. Poor IoT device security posture can allow potential attacks to succeed if left
 *    unchanged, as security is always measured by the weakest link within any organization.
 */
typedef struct NX_AZURE_IOT_SECURITY_MODULE_STRUCT
{
    security_module_state_t state;
    notifier_t message_ready;
    unsigned long state_timestamp;
    event_loop_timer_handler h_state_machine;

    NX_AZURE_IOT *nx_azure_iot_ptr;

    NX_CLOUD_MODULE nx_azure_iot_security_module_cloud;
} NX_AZURE_IOT_SECURITY_MODULE;

/**
 * @brief Enable Azure IoT Security Module
 *
 * @details This routine enables the Azure IoT Security Module subsystem. An internal state machine
 *          manage security events collection and sends them to Azure IoT Hub. Only one NX_AZURE_IOT_SECURITY_MODULE
 *          instance exists and needed to manage data collection.
 *
 * @param[in] nx_azure_iot_ptr    A pointer to a #NX_AZURE_IOT
 *
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS               Successfully enabled Azure IoT Security Module.
 *   @retval #NX_AZURE_IOT_FAILURE               Fail to enable Azure IoT Security Module due to internal error.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER     Security module requires valid #NX_AZURE_IOT instance.
 */
UINT nx_azure_iot_security_module_enable(NX_AZURE_IOT *nx_azure_iot_ptr);


/**
 * @brief Disable Azure IoT Security Module
 *
 * @details This routine disables the Azure IoT Security Module subsystem.
 *
 * @param[in] nx_azure_iot_ptr    A pointer to a #NX_AZURE_IOT, if NULL the singleton instance will be disabled.
 *
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS               Successful if Azure IoT Security Module has been disabled successfully.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER     Azure IoT Hub instance differ than the singleton composite instance.
 *   @retval #NX_AZURE_IOT_FAILURE               Fail to disable Azure IoT Security Module due to internal error.
 */
UINT nx_azure_iot_security_module_disable(NX_AZURE_IOT *nx_azure_iot_ptr);


#ifdef __cplusplus
}
#endif

#endif /* NX_AZURE_IOT_SECURITY_MODULE_H */