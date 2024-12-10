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

/* Version: 6.1 */

/**
 * @file nx_azure_iot.h
 *
 */

#ifndef NX_AZURE_IOT_H
#define NX_AZURE_IOT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "azure/core/az_log.h"
#include "azure/iot/az_iot_common.h"
#include "nx_api.h"
#include "nx_cloud.h"
#include "nxd_dns.h"
#include "nxd_mqtt_client.h"

#ifndef NXD_MQTT_CLOUD_ENABLE
#error "NXD_MQTT_CLOUD_ENABLE must be defined"
#endif /* NXD_MQTT_CLOUD_ENABLE */

#ifndef NX_SECURE_ENABLE
#error "NX_SECURE_ENABLE must be defined"
#endif /* NX_SECURE_ENABLE */

/* Defined, certificate date is not validated. By default, it is enabled. */
/*
#define NX_AZURE_IOT_DISABLE_CERTIFICATE_DATE
*/

/* Define the LOG LEVEL.  */
#ifndef NX_AZURE_IOT_LOG_LEVEL
#define NX_AZURE_IOT_LOG_LEVEL    2
#endif /* NX_AZURE_IOT_LOG_LEVEL */

/* Define maximum trusted certificates count.  */
#ifndef NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS
#define NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS 3
#endif /* NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS */

/* Define maximum device certificates count.  */
#ifndef NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS
#define NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS 2
#endif /* NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS */

#define NX_AZURE_IOT_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Define the az iot log function. */
#define LogError(...)
#define LogInfo(...)
#define LogDebug(...)
#define LogLiteralArgs(s) (UCHAR *)s, sizeof(s) - 1

/**
* Supports logging of AZ_LOG_IOT_AZURERTOS classification message. In the logged
* message, at-most one format specifier can be specifed at the end of message.
* Currently only two format specifier are supported:
*   - %d for integer
*   - %s for string, where first argument is length and second is pointer to string    
*/
UINT nx_azure_iot_log(UCHAR *type_ptr, UINT type_len, UCHAR *msg_ptr, UINT msg_len, ...);

#if NX_AZURE_IOT_LOG_LEVEL > 0
#undef LogError
#define LogError(...) nx_azure_iot_log(LogLiteralArgs("[ERROR] "), __VA_ARGS__)
#endif /* NX_AZURE_IOT_LOG_LEVEL > 0 */
#if NX_AZURE_IOT_LOG_LEVEL > 1
#undef LogInfo
#define LogInfo(...) nx_azure_iot_log(LogLiteralArgs("[INFO] "), __VA_ARGS__)
#endif /* NX_AZURE_IOT_LOG_LEVEL > 1 */
#if NX_AZURE_IOT_LOG_LEVEL > 2
#undef LogDebug
#define LogDebug(...) nx_azure_iot_log(LogLiteralArgs("[DEBUG] "), __VA_ARGS__)
#endif /* NX_AZURE_IOT_LOG_LEVEL > 2 */

#define NX_AZURE_IOT_MQTT_QOS_0                           0
#define NX_AZURE_IOT_MQTT_QOS_1                           1

/* Define AZ IoT SDK event flags. These events are processed by the Cloud thread.  */

/* Provisioning Client Connect event */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_CONNECT_EVENT    ((ULONG)0x00000001)

/* Provisioning Client Subscribe event */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_SUBSCRIBE_EVENT  ((ULONG)0x00000002)

/* Provisioning Client Request event */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_REQUEST_EVENT    ((ULONG)0x00000004)

/* Provisioning Client Response event */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_RESPONSE_EVENT   ((ULONG)0x00000008)

/* Provisioning Client Disconnect event */
#define NX_AZURE_IOT_PROVISIONING_CLIENT_DISCONNECT_EVENT ((ULONG)0x00000010)

/* API return values.  */
/**< The operation was successful. */
#define NX_AZURE_IOT_SUCCESS                              0x0

/**< The operation was unsuccessful. */
#define NX_AZURE_IOT_FAILURE                              0x20000
#define NX_AZURE_IOT_SDK_CORE_ERROR                       0x20001
#define NX_AZURE_IOT_INVALID_PARAMETER                    0x20002
#define NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE            0x20003
#define NX_AZURE_IOT_INVALID_PACKET                       0x20004
#define NX_AZURE_IOT_NO_PACKET                            0x20005

/**< If the item requested was not found */
#define NX_AZURE_IOT_NOT_FOUND                            0x20006
#define NX_AZURE_IOT_NOT_ENABLED                          0x20007
#define NX_AZURE_IOT_NOT_INITIALIZED                      0x20008
#define NX_AZURE_IOT_NOT_SUPPORTED                        0x20009
#define NX_AZURE_IOT_ALREADY_CONNECTED                    0x2000A
#define NX_AZURE_IOT_CONNECTING                           0x2000B
#define NX_AZURE_IOT_DISCONNECTED                         0x2000C

/**< The operation is pending. */
#define NX_AZURE_IOT_PENDING                              0x2000D
#define NX_AZURE_IOT_SERVER_RESPONSE_ERROR                0x2000E
#define NX_AZURE_IOT_TOPIC_TOO_LONG                       0x2000F
#define NX_AZURE_IOT_MESSAGE_TOO_LONG                     0x20010
#define NX_AZURE_IOT_NO_AVAILABLE_CIPHER                  0x20011
#define NX_AZURE_IOT_WRONG_STATE                          0x20012
#define NX_AZURE_IOT_DELETE_ERROR                         0x20013
#define NX_AZURE_IOT_NO_SUBSCRIBE_ACK                     0x20014
#define NX_AZURE_IOT_THROTTLED                            0x20015

#define NX_AZURE_IOT_EMPTY_JSON                           0x20016
#define NX_AZURE_IOT_SAS_TOKEN_EXPIRED                    0x20017
#define NX_AZURE_IOT_NO_MORE_ENTRIES                      0x20018

/* Resource type managed by AZ_IOT.  */
#define NX_AZURE_IOT_RESOURCE_IOT_HUB                     0x1
#define NX_AZURE_IOT_RESOURCE_IOT_PROVISIONING            0x2

/* Define the packet buffer for THREADX TLS.  */
#ifndef NX_AZURE_IOT_TLS_PACKET_BUFFER_SIZE
#define NX_AZURE_IOT_TLS_PACKET_BUFFER_SIZE               (1024 * 7)
#endif /* NX_AZURE_IOT_TLS_PACKET_BUFFER_SIZE  */

/* Define MQTT keep alive in seconds. 0 means the keep alive is disabled.
   By default, keep alive is 4 minutes. */
#ifndef NX_AZURE_IOT_MQTT_KEEP_ALIVE
#define NX_AZURE_IOT_MQTT_KEEP_ALIVE                      (60 * 4)
#endif /* NX_AZURE_IOT_MQTT_KEEP_ALIVE */

/* MQTT Subscribe topic offset.  */
#define NX_AZURE_IOT_MQTT_SUBSCRIBE_TOPIC_OFFSET          6

/* MQTT Publish offset.  */
#define NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET          7

/**
 * @brief Resource struct
 *
 */
typedef struct NX_AZURE_IOT_RESOURCE_STRUCT
{
    UINT                                   resource_type;
    VOID                                  *resource_data_ptr;
    NXD_MQTT_CLIENT                        resource_mqtt;
    UCHAR                                 *resource_mqtt_client_id;
    UINT                                   resource_mqtt_client_id_length;
    UCHAR                                 *resource_mqtt_user_name;
    UINT                                   resource_mqtt_user_name_length;
    UCHAR                                 *resource_mqtt_sas_token;
    UINT                                   resource_mqtt_sas_token_length;
    VOID                                  *resource_mqtt_buffer_context;
    UINT                                   resource_mqtt_buffer_size;
    UCHAR                                  resource_tls_packet_buffer[NX_AZURE_IOT_TLS_PACKET_BUFFER_SIZE];
    const NX_CRYPTO_METHOD               **resource_crypto_array;
    UINT                                   resource_crypto_array_size;
    const NX_CRYPTO_CIPHERSUITE          **resource_cipher_map;
    UINT                                   resource_cipher_map_size;
    UCHAR                                 *resource_metadata_ptr;
    UINT                                   resource_metadata_size;
    NX_SECURE_X509_CERT                   *resource_trusted_certificates[NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS];
    NX_SECURE_X509_CERT                   *resource_device_certificates[NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS];
    const UCHAR                           *resource_hostname;
    UINT                                   resource_hostname_length;
    struct NX_AZURE_IOT_RESOURCE_STRUCT   *resource_next;

} NX_AZURE_IOT_RESOURCE;

/**
 * @brief Azure IoT Struct
 *
 */
typedef struct NX_AZURE_IOT_STRUCT
{
    const UCHAR                           *nx_azure_iot_name;
    NX_IP                                 *nx_azure_iot_ip_ptr;
    NX_PACKET_POOL                        *nx_azure_iot_pool_ptr;
    NX_DNS                                *nx_azure_iot_dns_ptr;
    NX_CLOUD                               nx_azure_iot_cloud;
    NX_CLOUD_MODULE                        nx_azure_iot_cloud_module;
    TX_MUTEX                              *nx_azure_iot_mutex_ptr;
    VOID                                 (*nx_azure_iot_provisioning_client_event_process)(
                                          struct NX_AZURE_IOT_STRUCT *nx_azure_iot_ptr,
                                          ULONG common_events, ULONG module_own_events);
    struct NX_AZURE_IOT_RESOURCE_STRUCT   *nx_azure_iot_resource_list_header;
    UINT                                 (*nx_azure_iot_unix_time_get)(ULONG *unix_time);
} NX_AZURE_IOT;

typedef struct NX_AZURE_IOT_THREAD_STRUCT
{
    TX_THREAD                           *thread_ptr;
    struct NX_AZURE_IOT_THREAD_STRUCT   *thread_next;
    UINT                                 thread_message_type;
    UINT                                 thread_expected_id;     /* Used by device twin. */
    NX_PACKET                           *thread_received_message;
} NX_AZURE_IOT_THREAD;

/**
 * @brief Create the Azure IoT subsystem
 *
 * @details This routine creates the Azure IoT subsystem. An internal thread is created to
 *          manage activities related to Azure IoT services. Only one NX_AZURE_IOT instance
 *          is needed to manage instances for Azure IoT hub, IoT Central, Device Provisioning
 *          Services (DPS), and Azure Security Center (ASC).
 * 
 * @remarks This routine enables ASC by default. ASC provides a comprehensive security solution for
 *          Azure RTOS devices in which it collects network information and send it to the IoTHub.
 *          More details can be found https://docs.microsoft.com/en-us/azure/defender-for-iot/iot-security-azure-rtos.
 *          To disable ASC from your application, we provide both compile time and runtime option:
 *              - compile-time : NX_AZURE_DISABLE_IOT_SECURITY_MODULE in NetXDuo header file such as nx_port.h
 *                               when building the middleware.
 *              - runtime : Call UINT nx_azure_iot_security_module_disable(NX_AZURE_IOT *nx_azure_iot_ptr)
 *                          in your application code.
 * 
 * @param[in] nx_azure_iot_ptr A pointer to a #NX_AZURE_IOT
 * @param[in] name_ptr A pointer to a NULL-terminated string indicating the name of the Azure IoT instance.
 * @param[in] ip_ptr A pointer to a `NX_IP`, which is the IP stack used to connect to Azure IoT Services.
 * @param[in] pool_ptr A pointer to a `NX_PACKET_POOL`, which is the packet pool used by Azure IoT Services.
 * @param[in] dns_ptr A pointer to a `NX_DNS`.
 * @param[in] stack_memory_ptr A pointer to memory to be used as a stack space for the internal thread.
 * @param[in] stack_memory_size Size of stack memory area.
 * @param[in] priority Priority of the internal thread.
 * @param[in] unix_time_callback Callback to fetch unix time from platform.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully created the Azure IoT instance.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to create the Azure IoT instance due to invalid parameter.
 *   @retval NX_OPTION_ERROR Fail to create the Azure IoT instance due to invalid priority.
 *   @retval NX_PTR_ERROR Fail to create the Azure IoT instance due to invalid parameter.
 *   @retval NX_SIZE_ERROR Fail to create the Azure IoT instance due to insufficient size of stack memory.
 */
UINT nx_azure_iot_create(NX_AZURE_IOT *nx_azure_iot_ptr, const UCHAR *name_ptr,
                         NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr,
                         VOID *stack_memory_ptr, UINT stack_memory_size,
                         UINT priority, UINT (*unix_time_callback)(ULONG *unix_time));

/**
 * @brief Shutdown and cleanup the Azure IoT subsystem.
 * @details This routine stops all Azure services managed by this instance, and cleans up internal resources.
 *
 * @param[in] nx_azure_iot_ptr A pointer to a #NX_AZURE_IOT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully stopped Azure IoT services and cleaned up internal
 *                                    resources instance.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to delete the Azure IoT instance due to invalid parameter.
 *   @retval #NX_AZURE_IOT_DELETE_ERROR Fail to delete the Azure IoT instance due to resource is still in use.
 */
UINT nx_azure_iot_delete(NX_AZURE_IOT *nx_azure_iot_ptr);

/**
 * @brief Get unixtime
 *
 * @param[in] nx_azure_iot_ptr A pointer to a #NX_AZURE_IOT.
 * @param[out] unix_time Pointer to `ULONG` where unixtime is returned.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully return unix time.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to get unix time due to invalid parameter.
 */
UINT nx_azure_iot_unix_time_get(NX_AZURE_IOT *nx_azure_iot_ptr, ULONG *unix_time);

/**
 * @brief Initialize logging
 *
 * @details This routine initialized the logging with customer specific callback to output the logs for different
 *          classifications:
 *          - AZ_LOG_IOT_AZURERTOS,
 *          - AZ_LOG_MQTT_RECEIVED_TOPIC,
 *          - AZ_LOG_MQTT_RECEIVED_PAYLOAD,
 *          - AZ_LOG_IOT_RETRY,
 *          - AZ_LOG_IOT_SAS_TOKEN
 *
 * @param[in] log_callback A pointer to a callback.
 *
 */
VOID nx_azure_iot_log_init(VOID(*log_callback)(az_log_classification classification, UCHAR *msg, UINT msg_len));

/* Internal APIs. */
UINT nx_azure_iot_buffer_allocate(NX_AZURE_IOT *nx_azure_iot_ptr, UCHAR **buffer_pptr,
                                  UINT *buffer_size, VOID **buffer_context);
UINT nx_azure_iot_buffer_free(VOID *buffer_context);
UINT nx_azure_iot_resource_add(NX_AZURE_IOT *nx_azure_iot_ptr, NX_AZURE_IOT_RESOURCE *resource);
UINT nx_azure_iot_resource_remove(NX_AZURE_IOT *nx_azure_iot_ptr, NX_AZURE_IOT_RESOURCE *resource);
NX_AZURE_IOT_RESOURCE *nx_azure_iot_resource_search(NXD_MQTT_CLIENT *client_ptr);
UINT nx_azure_iot_publish_mqtt_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr,
                                      UINT topic_len, UCHAR *packet_id, UINT qos, UINT wait_option);
UINT nx_azure_iot_publish_packet_get(NX_AZURE_IOT *nx_azure_iot_ptr, NXD_MQTT_CLIENT *client_ptr,
                                     NX_PACKET **packet_pptr, UINT wait_option);
UINT nx_azure_iot_mqtt_packet_id_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *packet_id);
VOID nx_azure_iot_mqtt_packet_adjust(NX_PACKET *packet_ptr);
UINT nx_azure_iot_mqtt_tls_setup(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                 NX_SECURE_X509_CERT *certificate,
                                 NX_SECURE_X509_CERT *trusted_certificate);
UINT nx_azure_iot_base64_hmac_sha256_calculate(NX_AZURE_IOT_RESOURCE *resource_ptr,
                                               const UCHAR *key_ptr, UINT key_size,
                                               const UCHAR *message_ptr, UINT message_size,
                                               UCHAR *buffer_ptr, UINT buffer_len,
                                               UCHAR **output_ptr, UINT *output_len_ptr);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_H */
