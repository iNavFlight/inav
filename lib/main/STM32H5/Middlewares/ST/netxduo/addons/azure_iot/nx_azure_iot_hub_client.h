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
 * @file nx_azure_iot_hub_client.h
 *
 * @brief Definition for the Azure IoT Hub client.
 * @remark The IoT Hub MQTT protocol is described at
 * https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support.
 *
 */

#ifndef NX_AZURE_IOT_HUB_CLIENT_H
#define NX_AZURE_IOT_HUB_CLIENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "nx_azure_iot.h"
#include "azure/iot/az_iot_hub_client.h"

/**< Value denoting a message is of "None" type */
#define NX_AZURE_IOT_HUB_NONE                                       0x00000000

/**< Value denoting a message is of "all" type */
#define NX_AZURE_IOT_HUB_ALL_MESSAGE                                0xFFFFFFFF

/**< Value denoting a message is a cloud-to-device message */
#define NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE                    0x00000001

/**< Value denoting a message is a command message */
#define NX_AZURE_IOT_HUB_COMMAND                                    0x00000002

/**< Value denoting a message is a properties message */
#define NX_AZURE_IOT_HUB_PROPERTIES                                 0x00000004

/**< Value denoting a message is a writable properties message */
#define NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES                        0x00000008

/**< Value denoting a message is a reported properties response */
#define NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE               0x00000010

/* Map the message type.  */
#define NX_AZURE_IOT_HUB_DIRECT_METHOD                              NX_AZURE_IOT_HUB_COMMAND
#define NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES                     NX_AZURE_IOT_HUB_PROPERTIES
#define NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES             NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES
#define NX_AZURE_IOT_HUB_DEVICE_TWIN_REPORTED_PROPERTIES_RESPONSE   NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE

/* Set the default timeout for DNS query.  */
#ifndef NX_AZURE_IOT_HUB_CLIENT_DNS_TIMEOUT
#define NX_AZURE_IOT_HUB_CLIENT_DNS_TIMEOUT                         (5 * NX_IP_PERIODIC_RATE)
#endif /* NX_AZURE_IOT_HUB_CLIENT_DNS_TIMEOUT */

/* Set the default timeout for connection with SAS token authentication in secs.  */
#ifndef NX_AZURE_IOT_HUB_CLIENT_TOKEN_CONNECTION_TIMEOUT
#ifdef NX_AZURE_IOT_HUB_CLIENT_TOKEN_EXPIRY
#define NX_AZURE_IOT_HUB_CLIENT_TOKEN_CONNECTION_TIMEOUT            NX_AZURE_IOT_HUB_CLIENT_TOKEN_EXPIRY
#else
#define NX_AZURE_IOT_HUB_CLIENT_TOKEN_CONNECTION_TIMEOUT            (3600)
#endif /* NX_AZURE_IOT_HUB_CLIENT_TOKEN_EXPIRY */
#endif /* NX_AZURE_IOT_HUB_CLIENT_SAS_CONNECTION_TIMEOUT */

#ifndef NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC
#define NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC                  (10 * 60)
#endif /* NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC */

#ifndef NX_AZURE_IOT_HUB_CLIENT_INITIAL_BACKOFF_IN_SEC
#define NX_AZURE_IOT_HUB_CLIENT_INITIAL_BACKOFF_IN_SEC              (3)
#endif /* NX_AZURE_IOT_HUB_CLIENT_INITIAL_BACKOFF_IN_SEC */

#ifndef NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_JITTER_PERCENT
#define NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_JITTER_PERCENT          (60)
#endif /* NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_JITTER_PERCENT */

/* Define Azure IoT Hub Client state.  */
/**< The client is not connected */
#define NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED                0

/**< The client is connecting */
#define NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTING                   1

/**< The client is connected */
#define NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED                    2

/* Default TELEMETRY QoS is QoS1 */
#ifndef NX_AZURE_IOT_HUB_CLIENT_TELEMETRY_QOS
#define NX_AZURE_IOT_HUB_CLIENT_TELEMETRY_QOS                       NX_AZURE_IOT_MQTT_QOS_1
#endif /* NX_AZURE_IOT_HUB_CLIENT_TELEMETRY_QOS */

#ifndef NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST
#define NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST                  (4)
#endif /* NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST */

/* Forward declration*/
struct NX_AZURE_IOT_HUB_CLIENT_STRUCT;

typedef struct NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE_STRUCT
{
    NX_PACKET    *message_head;
    NX_PACKET    *message_tail;
    VOID        (*message_callback)(struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr, VOID *args);
    VOID         *message_callback_args;
    UINT        (*message_process)(struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr,
                                   NX_PACKET *packet_ptr, ULONG topic_offset, USHORT topic_length);
    UINT        (*message_enable)(struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr);
} NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE;

/**
 * @brief Azure IoT Hub Client struct
 *
 */
typedef struct NX_AZURE_IOT_HUB_CLIENT_STRUCT
{
    NX_AZURE_IOT                           *nx_azure_iot_ptr;

    UINT                                    nx_azure_iot_hub_client_state;
    NX_AZURE_IOT_THREAD                    *nx_azure_iot_hub_client_thread_suspended;
    NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE nx_azure_iot_hub_client_c2d_message;
    NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE nx_azure_iot_hub_client_command_message;
    NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE nx_azure_iot_hub_client_properties_message;
    NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE nx_azure_iot_hub_client_writable_properties_message;
    VOID                                  (*nx_azure_iot_hub_client_report_properties_response_callback)(
                                           struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr,
                                           UINT request_id, UINT response_status, ULONG version, VOID *args);
    VOID                                   *nx_azure_iot_hub_client_report_properties_response_callback_args;

    VOID                                  (*nx_azure_iot_hub_client_connection_status_callback)(
                                           struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr,
                                           UINT status);
    UINT                                  (*nx_azure_iot_hub_client_token_refresh)(
                                           struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr,
                                           ULONG expiry_time_secs, const UCHAR *key, UINT key_len,
                                           UCHAR *sas_buffer, UINT sas_buffer_len, UINT *sas_length);

    UINT                                    nx_azure_iot_hub_client_request_id;
    const UCHAR                            *nx_azure_iot_hub_client_symmetric_key;
    UINT                                    nx_azure_iot_hub_client_symmetric_key_length;
    NX_AZURE_IOT_RESOURCE                   nx_azure_iot_hub_client_resource;

    volatile UCHAR                          nx_azure_iot_hub_client_properties_subscribe_ack;
    UCHAR                                   reserved[3];

    az_iot_hub_client                       iot_hub_client_core;
    UINT                                    nx_azure_iot_hub_client_throttle_count;
    ULONG                                   nx_azure_iot_hub_client_throttle_end_time;
    ULONG                                   nx_azure_iot_hub_client_sas_token_expiry_time;
    az_span                                 nx_azure_iot_hub_client_component_list[NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST];
    UINT                                  (*nx_azure_iot_hub_client_component_callback[NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST])
                                                                                      (VOID *json_reader_ptr,
                                                                                       ULONG version,
                                                                                       VOID *args);
    VOID                                   *nx_azure_iot_hub_client_component_callback_args[NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST];
    VOID                                  (*nx_azure_iot_hub_client_component_properties_process)(struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *hub_client_ptr,
                                                                                                  NX_PACKET *packet_ptr,
                                                                                                  UINT message_type);
} NX_AZURE_IOT_HUB_CLIENT;


/**
 * @brief Initialize Azure IoT hub instance
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] nx_azure_iot_ptr A pointer to a #NX_AZURE_IOT.
 * @param[in] host_name A `UCHAR` pointer to IoTHub hostname. Must be `NULL` terminated.
 * @param[in] host_name_length Length of `host_name`. Does not include the `NULL` terminator.
 * @param[in] device_id A `UCHAR` pointer to the device ID.
 * @param[in] device_id_length Length of the `device_id`. Does not include the `NULL` terminator.
 * @param[in] module_id A `UCHAR` pointer to the module ID.
 * @param[in] module_id_length Length of the `module_id`. Does not include the `NULL` terminator.
 * @param[in] crypto_array A pointer to an array of `NX_CRYPTO_METHOD`.
 * @param[in] crypto_array_size Size of `crypto_array`.
 * @param[in] cipher_map A pointer to an array of `NX_CRYPTO_CIPHERSUITE`.
 * @param[in] cipher_map_size Size of `cipher_map`.
 * @param[in] metadata_memory A `UCHAR` pointer to metadata memory buffer.
 * @param[in] memory_size Size of `metadata_memory`.
 * @param[in] trusted_certificate A pointer to `NX_SECURE_X509_CERT`, which are the server side certs.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully initialized the Azure IoT hub client.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to initialize the Azure IoT hub client due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to initialize the Azure IoT hub client due to SDK core error.
 */
UINT nx_azure_iot_hub_client_initialize(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                        NX_AZURE_IOT *nx_azure_iot_ptr,
                                        const UCHAR *host_name, UINT host_name_length,
                                        const UCHAR *device_id, UINT device_id_length,
                                        const UCHAR *module_id, UINT module_id_length,
                                        const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                        const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                        UCHAR *metadata_memory, UINT memory_size,
                                        NX_SECURE_X509_CERT *trusted_certificate);

/**
 * @brief Deinitialize the Azure IoT Hub instance.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully de-initialized the Azure IoT hub client.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to deinitialize the Azure IoT hub client due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_deinitialize(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Add more trusted certificate in the IoT Hub client if needed. It can be called multiple times to set multiple trusted certificates.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] trusted_certificate A pointer to a `NX_SECURE_X509_CERT`, which is the trusted certificate.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully add trusted certificate to Azure IoT Hub Instance.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to add trusted certificate to Azure IoT Hub Instance due to invalid parameter.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add trusted certificate due to NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS is too small.
 */
UINT nx_azure_iot_hub_client_trusted_cert_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                              NX_SECURE_X509_CERT *trusted_certificate);

/**
 * @brief Set the client certificate in the IoT Hub client. It can be called multiple times to set certificate chain.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] device_certificate A pointer to a `NX_SECURE_X509_CERT`, which is the device certificate.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully set device certificate to Azure IoT Hub Instance.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set device certificate to Azure IoT Hub Instance due to invalid parameter.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to set device certificate due to NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS is too small.
 */
UINT nx_azure_iot_hub_client_device_cert_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                             NX_SECURE_X509_CERT *device_certificate);

/**
 * @brief Set symmetric key in the IoT Hub client.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] symmetric_key A pointer to a symmetric key.
 * @param[in] symmetric_key_length Length of `symmetric_key`.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully set symmetric key to IoTHub client.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set symmetric key to IoTHub client due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_symmetric_key_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                               const UCHAR *symmetric_key, UINT symmetric_key_length);

/**
 * @brief Set model id in the IoT Hub client to enable PnP.
 * @note To enable pnp, this routine should be called immediately after nx_azure_iot_hub_client_initialize(),
         if model id is not set, only normal iothub APIs can be used, if it is set, only pnp APIs can be used.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] model_id_ptr A pointer to a model id.
 * @param[in] model_id_length Length of `model id`.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully set model id to IoTHub client.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set model id to IoTHub client due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_model_id_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                          const UCHAR *model_id_ptr, UINT model_id_length);

/**
 * @brief Add component name to IoT Hub client.
 * @note This routine should be called for all the component in the PnP model.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] component_name_ptr A pointer to component, that is part of PnP model.
 * @param[in] component_name_length Length of the `component_name_ptr`.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successfully add the component name to the PnP client.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add the component name due to out of memory.
 */
UINT nx_azure_iot_hub_client_component_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                           const UCHAR *component_name_ptr,
                                           USHORT component_name_length);

#ifdef NXD_MQTT_OVER_WEBSOCKET
/**
 * @brief Enable using MQTT over WebSocket to connect to IoTHub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if MQTT over WebSocket is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable C2D message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable C2D message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable C2D message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable C2D message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_websocket_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
#endif /* NXD_MQTT_OVER_WEBSOCKET */

/**
 * @brief Connect to IoT Hub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] clean_session Can be set to `0` to re-use current session, or `1` to start new session
 * @param[in] wait_option Number of ticks to wait for internal resources to be available.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if connected to Azure IoT Hub.
 *   @retval #NX_AZURE_IOT_CONNECTING Successfully started connection but not yet completed.
 *   @retval #NX_AZURE_IOT_ALREADY_CONNECTED Already connected to Azure IoT Hub.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to connect to Azure IoT Hub due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to connect to Azure IoT Hub due to SDK core error.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to connect to Azure IoT Hub due to buffer size is too small.
 *   @retval NX_DNS_QUERY_FAILED Fail to connect to Azure IoT Hub due to hostname can not be resolved.
 *   @retval NX_NO_PACKET Fail to connect to Azure IoT Hub due to no available packet in pool.
 *   @retval NX_INVALID_PARAMETERS Fail to connect to Azure IoT Hub due to invalid parameters.
 *   @retval NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE Fail to connect to Azure IoT Hub due to insufficient metadata space.
 *   @retval NX_SECURE_TLS_UNSUPPORTED_CIPHER Fail to connect to Azure IoT Hub due to unsupported cipher.
 *   @retval NXD_MQTT_ALREADY_CONNECTED Fail to connect to Azure IoT Hub due to MQTT session is not disconnected.
 *   @retval NXD_MQTT_CONNECT_FAILURE Fail to connect to Azure IoT Hub due to TCP/TLS connect error.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to connect to Azure IoT Hub due to MQTT connect error.
 *   @retval NXD_MQTT_ERROR_SERVER_UNAVAILABLE Fail to connect to Azure IoT Hub due to server unavailable.
 *   @retval NXD_MQTT_ERROR_NOT_AUTHORIZED Fail to connect to Azure IoT Hub due to authentication error.
 */
UINT nx_azure_iot_hub_client_connect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                     UINT clean_session, UINT wait_option);

/**
 * @brief Disconnect from IoT Hub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if client disconnects.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disconnect due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_disconnect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Sets connection status callback function
 * @details This routine sets the connection status callback. This callback function is
 *          invoked when IoT Hub status is changed, such as when the client is connected to IoT Hub.
 *          The different statuses include:
 *
 *          - #NX_AZURE_IOT_SUCCESS
 *          - NX_SECURE_TLS_ALERT_RECEIVED
 *          - NX_SECURE_TLS_NO_SUPPORTED_CIPHERS
 *          - NX_SECURE_X509_CHAIN_VERIFY_FAILURE
 *          - NXD_MQTT_CONNECT_FAILURE
 *          - NXD_MQTT_ERROR_SERVER_UNAVAILABLE
 *          - NXD_MQTT_ERROR_NOT_AUTHORIZED
 *          - NX_AZURE_IOT_DISCONNECTED
 *
 *          Setting the callback function to `NULL` disables the callback function.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] connection_status_cb Pointer to a callback function invoked on connection status is changed.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if connection status callback is set.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set connection status callback due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_connection_status_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            VOID (*connection_status_cb)(
                                                                  struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *client_ptr,
                                                                  UINT status));

/**
 * @brief Sets receive callback function
 * @details This routine sets the IoT Hub receive callback function. This callback
 *          function is invoked when a message is received from Azure IoT hub. Setting the
 *          callback function to `NULL` disables the callback function. Message types can be:
 *
 *          - #NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE
 *          - #NX_AZURE_IOT_HUB_COMMAND / NX_AZURE_IOT_HUB_DIRECT_METHOD
 *          - #NX_AZURE_IOT_HUB_PROPERTIES / NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES
 *          - #NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES / NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] message_type Message type of callback function.
 * @param[in] callback_ptr Pointer to a callback function invoked if the specified message type is received.
 * @param[in] callback_args Pointer to an argument passed to callback function.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if callback function is set.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set receive callback due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_SUPPORTED Fail to set receive callback due to message_type not supported.
 */
UINT nx_azure_iot_hub_client_receive_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  UINT message_type,
                                                  VOID (*callback_ptr)(
                                                        NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        VOID *args),
                                                  VOID *callback_args);

/**
 * @brief Creates telemetry message.
 * @details This routine prepares a packet for sending telemetry data. After the packet is properly created,
 *          application owns the `NX_PACKET` and can add additional user-defined properties before sending out.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr Returned allocated `NX_PACKET` on success. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait if no packet is available.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if a packet is allocated.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to allocate telemetry message due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to allocate telemetry message due to SDK core error.
 *   @retval NX_NO_PACKET Fail to allocate telemetry message due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_telemetry_message_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET **packet_pptr,
                                                      UINT wait_option);

/**
 * @brief Deletes telemetry message
 *
 * @param[in] packet_ptr The `NX_PACKET` to release.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if a packet is deallocated.
 */
UINT nx_azure_iot_hub_client_telemetry_message_delete(NX_PACKET *packet_ptr);

/**
 * @brief Set component to telemetry message.
 * @details This routine allows an application to set a component name to a telemetry message
 *          before it is being sent. The component is stored in the sequence which the routine is being called.
 *
 * @param[in] packet_ptr A pointer to telemetry property packet.
 * @param[in] component_name_ptr A pointer to a component name.
 * @param[in] component_name_length Length of `component_name_ptr`. Does not include the `NULL` terminator.
 * @param[in] wait_option Ticks to wait if no packet is available.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if component is set.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set component due to invalid parameter.
 *   @retval NX_NO_PACKET Fail to set component due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_telemetry_component_set(NX_PACKET *packet_pptr,
                                                     const UCHAR *component_name_ptr,
                                                     USHORT component_name_length,
                                                     UINT wait_option);

/**
 * @brief Add property to telemetry message
 * @details This routine allows an application to add user-defined properties to a telemetry message
 *          before it is being sent. This routine can be called multiple times to add all the properties to
 *          the message. The properties are stored in the sequence which the routine is being called.
 *          The property must be added after a telemetry packet is created, and before the telemetry
 *          message is being sent.
 *
 * @param[in] packet_ptr A pointer to telemetry property packet.
 * @param[in] property_name Pointer to property name.
 * @param[in] property_name_length Length of property name.
 * @param[in] property_value Pointer to property value.
 * @param[in] property_value_length Length of property value.
 * @param[in] wait_option Ticks to wait if packet needs to be expanded.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if property is added.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to add property due to invalid parameter.
 *   @retval NX_NO_PACKET Fail to add property due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_telemetry_property_add(NX_PACKET *packet_ptr,
                                                    const UCHAR *property_name, USHORT property_name_length,
                                                    const UCHAR *property_value, USHORT property_value_length,
                                                    UINT wait_option);

/**
 * @brief Sends telemetry message to IoTHub.
 * @details This routine sends telemetry to IoTHub, with `packet_ptr` containing all the properties.
 *          On successful return of this function, ownership of `NX_PACKET` is released.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] packet_ptr A pointer to telemetry property packet.
 * @param[in] telemetry_data Pointer to telemetry data.
 * @param[in] data_size Size of telemetry data.
 * @param[in] wait_option Ticks to wait for message to be sent.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if telemetry message is sent out.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send telemetry message due to invalid parameter.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to send telemetry message due to packet is invalid.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to send telemetry message due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to send telemetry message due to TCP/TLS error.
 *   @retval NX_NO_PACKET Fail to send telemetry message due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_telemetry_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, NX_PACKET *packet_ptr,
                                            const UCHAR *telemetry_data, UINT data_size, UINT wait_option);

/**
 * @brief Enable receiving C2D message from IoTHub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if C2D message receiving is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable C2D message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable C2D message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable C2D message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable C2D message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_cloud_message_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Disables receiving C2D message from IoTHub
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if C2D message receiving is disabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disable C2D message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to disable C2D message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to disable C2D message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to disable C2D message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_cloud_message_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Receives C2D message from IoTHub
 * @details This routine receives C2D message from IoT Hub. If there are no messages in the receive
 *          queue, this routine can block.The amount of time it waits for a message is determined
 *          by the `wait_option` parameter.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr Return a `NX_PACKET` pointer with C2D message on success. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait for message to arrive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if C2D message is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive C2D message due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive C2D message due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive C2D message due to timeout.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive C2D message due to disconnection.
 */
UINT nx_azure_iot_hub_client_cloud_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Retrieve the property with given property name in the C2D message.
 *
 * @param[in] hub_client_ptr A pointer to a NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] packet_ptr Pointer to NX_PACKET containing C2D message.
 * @param[in] property_name A `UCHAR` pointer to property name.
 * @param[in] property_name_length Length of `property_name`.
 * @param[out] property_value Pointer to `UCHAR` array that contains property values.
 * @param[out] property_value_length A `USHORT` pointer to size of `property_value`.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if property is found and copied successfully into user buffer.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to find the property due to invalid parameter.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to find the property due to the packet is invalid.
 *   @retval #NX_AZURE_IOT_NOT_FOUND Property is not found.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to find the property due to parsing error.
 */
UINT nx_azure_iot_hub_client_cloud_message_property_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET *packet_ptr, const UCHAR *property_name,
                                                        USHORT property_name_length, const UCHAR **property_value,
                                                        USHORT *property_value_length);

/**
 * @brief Enables receiving direct method messages from IoTHub
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if direct method message receiving is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable direct method message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable direct method message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable direct method message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable direct method message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_direct_method_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Disables receiving direct method messages from IoTHub
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if direct method message receiving is disabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disable direct method message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to disable direct method message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to disable direct method message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to disable direct method message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_direct_method_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Receives direct method message from IoTHub
 * @details This routine receives direct method message from IoT Hub. If there are no
 *          messages in the receive queue, this routine can block. The amount of time it waits for a
 *          message is determined by the `wait_option` parameter.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] method_name_pptr Return a pointer to method name on success.
 * @param[out] method_name_length_ptr Return length of `method_name_pptr` on success.
 * @param[out] context_pptr Return a pointer to the context pointer on success.
 * @param[out] context_length_ptr Return length of `context` on success.
 * @param[out] packet_pptr Return `NX_PACKET` containing the method payload on success. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait for message to arrive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if direct method message is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive direct method message due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive direct method message due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive direct method message due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive direct method message due to invalid packet.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive direct method message due to SDK core error.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive direct method message due to disconnect.
 */
UINT nx_azure_iot_hub_client_direct_method_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                           const UCHAR **method_name_pptr, USHORT *method_name_length_ptr,
                                                           VOID **context_pptr, USHORT *context_length_ptr,
                                                           NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Return response to direct method message from IoTHub
 * @details This routine returns response to the direct method message from IoT Hub.
 * @note request_id ties the correlation between direct method receive and response.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] status_code Status code for direct method.
 * @param[in] context_ptr Pointer to context return from nx_azure_iot_hub_client_direct_method_message_receive().
 * @param[in] context_length Length of context.
 * @param[in] payload  Pointer to `UCHAR` containing the payload for the direct method response. Payload is in JSON format.
 * @param[in] payload_length Length of `payload`
 * @param[in] wait_option Ticks to wait for message to send.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if direct method response is send.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send direct method response due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to send direct method response due to SDK core error.
 *   @retval NX_NO_PACKET Fail send direct method response due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_direct_method_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT status_code, VOID *context_ptr,
                                                            USHORT context_length, const UCHAR *payload,
                                                            UINT payload_length, UINT wait_option);

/**
 * @brief Enables receiving command messages from IoTHub
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if command message receiving is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable command message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable command message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable command message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable command message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_command_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Disables receiving command messages from IoTHub
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if command message receiving is disabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disable command message receiving due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to disable command message receiving due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to disable command message receiving due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to disable command message receiving due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_command_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Receives PnP command message from IoTHub
 * @details This routine receives command message from IoT Hub. If there are no
 *          messages in the receive queue, this routine can block. The amount of time it waits for a
 *          message is determined by the `wait_option` parameter.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] component_name_pptr Return a pointer to PnP component name on success.
 * @param[out] component_name_length_ptr Return length of `*component_name_pptr` on success.
 * @param[out] command_name_pptr Return a pointer to command name on success.
 * @param[out] command_name_length_ptr Return length of `command_name_pptr` on success.
 * @param[out] context_pptr Return a pointer to the context pointer on success.
 * @param[out] context_length_ptr Return length of `context` on success.
 * @param[out] packet_pptr Return `NX_PACKET` containing the command payload on success. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait for message to arrive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if command message is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive command message due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive command message due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive command message due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive command message due to invalid packet.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive command message due to SDK core error.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive command message due to disconnect.
 */
UINT nx_azure_iot_hub_client_command_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                     const UCHAR **component_name_pptr, USHORT *component_name_length_ptr,
                                                     const UCHAR **command_name_pptr, USHORT *command_name_length_ptr,
                                                     VOID **context_pptr, USHORT *context_length_ptr,
                                                     NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Return response to PnP command message from IoTHub
 * @details This routine returns response to the command message from IoT Hub.
 * @note request_id ties the correlation between command receive and response.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] status_code Status code for command.
 * @param[in] context_ptr Pointer to context return from nx_azure_iot_hub_client_command_message_receive().
 * @param[in] context_length Length of context.
 * @param[in] payload  Pointer to `UCHAR` containing the payload for the command response. Payload is in JSON format.
 * @param[in] payload_length Length of `payload`
 * @param[in] wait_option Ticks to wait for message to send.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if command response is send.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send command response due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to send command response due to SDK core error.
 *   @retval NX_NO_PACKET Fail send command response due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_command_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      UINT status_code, VOID *context_ptr,
                                                      USHORT context_length, const UCHAR *payload,
                                                      UINT payload_length, UINT wait_option);

/**
 * @brief Enables device twin feature
 * @details This routine enables device twin feature.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if device twin feature is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable device twin feature due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable device twin feature due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable device twin feature due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable device twin feature due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_device_twin_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Disables device twin feature
 * @details This routine disables device twin feature.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if device twin feature is disabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disable device twin feature due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to disable device twin feature due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to disable device twin feature due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to disable device twin feature due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_device_twin_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Sets reported properties response callback function
 * @details This routine sets the reponse receive callback function for reported properties. This callback
 *          function is invoked when a response is received from Azure IoT hub for reported properties and no
 *          thread is waiting for response. Setting the callback function to `NULL` disables the callback
 *          function.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] callback_ptr Pointer to a callback function invoked.
 * @param[in] callback_args Pointer to an argument passed to callback function.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if callback function is set.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set callback due to invalid parameter.
 */
UINT nx_azure_iot_hub_client_reported_properties_response_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                       VOID (*callback_ptr)(
                                                                             NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                             UINT request_id,
                                                                             UINT response_status,
                                                                             ULONG version,
                                                                             VOID *args),
                                                                       VOID *callback_args);

/* Map old API to new API.  */
#define nx_azure_iot_hub_client_report_properties_response_callback_set nx_azure_iot_hub_client_reported_properties_response_callback_set

/**
 * @brief Send device twin reported properties to IoT Hub
 * @details This routine sends device twin reported properties to IoT Hub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] message_buffer JSON document containing the reported properties.
 * @param[in] message_length Length of JSON document.
 * @param[out] request_id_ptr Request Id assigned to the request.
 * @param[out] response_status_ptr Status return for successful send of reported properties.
 * @param[out] version_ptr Version return for successful send of reported properties.
 * @param[in] wait_option Ticks to wait for message to send.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if device twin reported properties is sent.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send reported properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to send reported properties due to device twin is not enabled.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to send reported properties due to SDK core error.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send reported properties due to buffer size is too small.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to send reported properties due to no packet available.
 *   @retval NX_NO_PACKET Fail to send reported properties due to no packet available.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to send reported properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_device_twin_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                  const UCHAR *message_buffer, UINT message_length,
                                                                  UINT *request_id_ptr, UINT *response_status_ptr,
                                                                  ULONG *version_ptr, UINT wait_option);

/**
 * @brief Request complete device twin properties
 * @details This routine requests complete device twin properties.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @param[in] wait_option Ticks to wait for sending request.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if device twin properties is requested.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send device twin request due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NO_SUBSCRIBE_ACK Fail to send device twin request due to no subscribe ack.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to send device twin request due to SDK core error.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send device twin request due to buffer size is too small.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to send device twin request due to no packet available.
 *   @retval NX_NO_PACKET Fail to send device twin request due to no packet available.
 */
UINT nx_azure_iot_hub_client_device_twin_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT wait_option);

/**
 * @brief Receive complete device twin properties
 * @details This routine receives complete device twin properties.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @param[out] packet_pptr Pointer to #NX_PACKET* that contains complete twin document. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait for message to receive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if device twin properties is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive device twin properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive device twin properties due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive device twin properties due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive device twin properties due to invalid packet.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive device twin properties due to SDK core error.
 *   @retval #NX_AZURE_IOT_SERVER_RESPONSE_ERROR Response code from server is not 2xx.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive device twin properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_device_twin_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Receive desired properties form IoTHub
 * @details This routine receives desired properties from IoTHub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr Pointer to #NX_PACKET* that contains complete twin document. Caller owns the `NX_PACKET` memory.
 * @param[in] wait_option Ticks to wait for message to receive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if desired properties is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive desired properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive desired properties due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive desired properties due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive desired properties due to invalid packet.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive desired properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_device_twin_desired_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                    NX_PACKET **packet_pptr, UINT wait_option);

/**
 * @brief Enables properties feature
 * @details This routine enables property feature.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if property feature is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable property feature due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to enable property feature due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to enable property feature due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to enable property feature due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_properties_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Disables properties feature
 * @details This routine disables property feature.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if property feature is disabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to disable property feature due to invalid parameter.
 *   @retval NXD_MQTT_NOT_CONNECTED Fail to disable property feature due to MQTT not connected.
 *   @retval NXD_MQTT_PACKET_POOL_FAILURE Fail to disable property feature due to no available packet in pool.
 *   @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to disable property feature due to TCP/TLS error.
 */
UINT nx_azure_iot_hub_client_properties_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

/**
 * @brief Creates reported properties message.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr Return `NX_PACKET` containing the reported properties payload on success.
 * @param[in] wait_option Ticks to wait for writer creation
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if a message writer is created.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to create message writer due to invalid parameter.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to create message writer due to SDK core error.
 *   @retval NX_NO_PACKET Fail to create message writer due to no available packet in pool.
 */
UINT nx_azure_iot_hub_client_reported_properties_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET **packet_pptr,
                                                        UINT wait_option);

/**
 * @brief Sends reported properties message to IoTHub.
 * @note The return status of the API indicates if the reported properties is sent out successfully or not,
 * the response status is used to track if the reported properties is accepted or not by IoT Hub, and the
 * reponse status is available only when the return status is NX_AZURE_IOT_SUCCESS.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] packet_ptr A pointer to a #NX_PACKET
 * @param[out] request_id_ptr Request Id assigned to the request.
 * @param[out] response_status_ptr Status return for successful send of reported properties.
 * @param[out] version_ptr Version return for successful send of reported properties.
 * @param[in] wait_option Ticks to wait for message to send.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if reported properties is sent.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to send reported properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to send reported properties due to property is not enabled.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to send reported properties due to SDK core error.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send reported properties due to buffer size is too small.
 *   @retval NX_NO_PACKET Fail to send reported properties due to no packet available.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to send reported properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET *packet_ptr,
                                                      UINT *request_id_ptr, UINT *response_status_ptr,
                                                      ULONG *version_ptr, UINT wait_option);

/**
 * @brief Request complete properties
 * @details This routine requests complete properties.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] wait_option Ticks to wait for request to send.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if request get all properties is sent.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to request get all properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NO_SUBSCRIBE_ACK Fail to request get all properties due to no subscribe ack.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to request get all properties due to SDK core error.
 *   @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to request get all properties due to buffer size is too small.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to request get all properties due to no packet available.
 *   @retval NX_NO_PACKET Fail to request get all properties due to no packet available.
 */
UINT nx_azure_iot_hub_client_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                UINT wait_option);

/**
 * @brief Receive all the properties
 * @details This routine receives all the properties.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr Return `NX_PACKET` containing properties payload on success.
 * @param[in] wait_option Ticks to wait for message to receive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if all properties is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive all properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive all properties due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive all properties due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive all properties due to invalid packet.
 *   @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive all properties due to SDK core error.
 *   @retval #NX_AZURE_IOT_SERVER_RESPONSE_ERROR Response code from server is not 2xx.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive all properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_PACKET **packet_pptr,
                                                UINT wait_option);

/**
 * @brief Receive writable properties form IoTHub
 * @details This routine receives writable properties from IoTHub.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[out] packet_pptr A pointer to a #NX_PACKET containing writable properties on success.
 * @param[in] wait_option Ticks to wait for message to receive.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if writable properties is received.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to receive writable properties due to invalid parameter.
 *   @retval #NX_AZURE_IOT_NOT_ENABLED Fail to receive writable properties due to it is not enabled.
 *   @retval #NX_AZURE_IOT_NO_PACKET Fail to receive writable properties due to timeout.
 *   @retval #NX_AZURE_IOT_INVALID_PACKET Fail to receive writable properties due to invalid packet.
 *   @retval #NX_AZURE_IOT_DISCONNECTED Fail to receive writable properties due to disconnect.
 */
UINT nx_azure_iot_hub_client_writable_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                         NX_PACKET **packet_pptr,
                                                         UINT wait_option);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_HUB_CLIENT_H */
