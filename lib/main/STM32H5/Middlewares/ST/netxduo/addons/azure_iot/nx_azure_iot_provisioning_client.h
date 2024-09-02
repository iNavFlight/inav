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
 * @file nx_azure_iot_provisioning_client.h
 *
 * @brief Definition for the Azure Device Provisioning client.
 * @remark The Device Provisioning MQTT protocol is described at
 * https://docs.microsoft.com/en-us/azure/iot-dps/iot-dps-mqtt-support.
 *
 */

#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_H
#define NX_AZURE_IOT_PROVISIONING_CLIENT_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "azure/iot/az_iot_provisioning_client.h"
#include "nx_azure_iot.h"
#include "nx_api.h"
#include "nxd_mqtt_client.h"

/* Define the MAX status size. */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_STATUS_ID_SIZE
#define NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_STATUS_ID_SIZE        30
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_STATUS_ID_SIZE */

/* Define the MAX deviceID size. */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_ID_BUFFER_SIZE
#define NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_ID_BUFFER_SIZE        100
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_ID_BUFFER_SIZE */

/* Define the MAX IoT Hub Endpoint size. */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_HUB_SIZE
#define NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_HUB_SIZE              100
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_HUB_SIZE */

/* Define the MAX operation Id of provisioning service. */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_REQ_OP_ID_SIZE
#define NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_REQ_OP_ID_SIZE        100
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_MAX_REQ_OP_ID_SIZE */

/* Set the default token expiry in secs.  */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_TOKEN_EXPIRY
#define NX_AZURE_IOT_PROVISIONING_CLIENT_TOKEN_EXPIRY              (3600)
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_TOKEN_EXPIRY */

/* Set the default timeout for DNS query.  */
#ifndef NX_AZURE_IOT_PROVISIONING_CLIENT_DNS_TIMEOUT
#define NX_AZURE_IOT_PROVISIONING_CLIENT_DNS_TIMEOUT               (5 * NX_IP_PERIODIC_RATE)
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_DNS_TIMEOUT */

typedef struct NX_AZURE_IOT_PROVISIONING_DEVICE_RESPONSE_STRUCT
{
    az_iot_provisioning_client_register_response    register_response;
    NX_PACKET                                      *packet_ptr;
} NX_AZURE_IOT_PROVISIONING_RESPONSE;

typedef struct NX_AZURE_IOT_PROVISIONING_THREAD_STRUCT
{
    TX_THREAD                                      *thread_ptr;
    struct NX_AZURE_IOT_PROVISIONING_THREAD_STRUCT *thread_next;
} NX_AZURE_IOT_PROVISIONING_THREAD;

/**
 * @brief Azure IoT Provisining Client struct
 *
 */
typedef struct NX_AZURE_IOT_PROVISIONING_CLIENT_STRUCT
{
    NX_AZURE_IOT                           *nx_azure_iot_ptr;

    UINT                                    nx_azure_iot_provisioning_client_state;
    NX_AZURE_IOT_PROVISIONING_THREAD       *nx_azure_iot_provisioning_client_thread_suspended;

    UINT                                    nx_azure_iot_provisioning_client_req_timeout;
    NX_PACKET                              *nx_azure_iot_provisioning_client_last_response;
    UINT                                    nx_azure_iot_provisioning_client_request_id;
    UINT                                    nx_azure_iot_provisioning_client_result;
    NX_AZURE_IOT_PROVISIONING_RESPONSE      nx_azure_iot_provisioning_client_response;
    VOID                                  (*nx_azure_iot_provisioning_client_on_complete_callback)(
                                           struct NX_AZURE_IOT_PROVISIONING_CLIENT_STRUCT *prov_client_ptr,
                                           UINT status);

    const UCHAR                            *nx_azure_iot_provisioning_client_endpoint;
    UINT                                    nx_azure_iot_provisioning_client_endpoint_length;
    const UCHAR                            *nx_azure_iot_provisioning_client_id_scope;
    UINT                                    nx_azure_iot_provisioning_client_id_scope_length;
    const UCHAR                            *nx_azure_iot_provisioning_client_registration_payload;
    UINT                                    nx_azure_iot_provisioning_client_registration_payload_length;
    const UCHAR                            *nx_azure_iot_provisioning_client_registration_id;
    UINT                                    nx_azure_iot_provisioning_client_registration_id_length;
    const UCHAR                            *nx_azure_iot_provisioning_client_symmetric_key;
    UINT                                    nx_azure_iot_provisioning_client_symmetric_key_length;
    UCHAR                                  *nx_azure_iot_provisioning_client_sas_token;
    UINT                                    nx_azure_iot_provisioning_client_sas_token_buff_size;

    NX_AZURE_IOT_RESOURCE                   nx_azure_iot_provisioning_client_resource;
    az_iot_provisioning_client              nx_azure_iot_provisioning_client_core;
} NX_AZURE_IOT_PROVISIONING_CLIENT;


/**
 * @brief Initialize Azure IoT Provisioning instance
 * @details This routine initializes the device to the IoT provisioning service.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] nx_azure_iot_ptr A pointer to a #NX_AZURE_IOT.
 * @param[in] endpoint A `UCHAR` pointer to IoT Provisioning endpoint. Must be `NULL` terminated.
 * @param[in] endpoint_length Length of `endpoint`. Does not include the `NULL` terminator.
 * @param[in] id_scope A `UCHAR` pointer to ID Scope.
 * @param[in] id_scope_length Length of the `id_scope`. Does not include the `NULL` terminator.
 * @param[in] registration_id A `UCHAR` pointer to registration ID.
 * @param[in] registration_id_length Length of `registration_id`. Does not include the `NULL` terminator.
 * @param[in] crypto_array A pointer to `NX_CRYPTO_METHOD`.
 * @param[in] crypto_array_size Size of `crypto_array`.
 * @param[in] cipher_map A pointer to `NX_CRYPTO_CIPHERSUITE`.
 * @param[in] cipher_map_size Size of `cipher_map`.
 * @param[in] metadata_memory A `UCHAR` pointer to metadata memory buffer.
 * @param[in] memory_size Size of metadata buffer.
 * @param[in] trusted_certificate A pointer to `NX_SECURE_X509_CERT`, which are the server side certs.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully initialized to Azure IoT Provisioning Client.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to initialize the Azure IoT Provisioning Client due to invalid parameter.
 *  @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to initialize the Azure IoT Provisioning Client due to SDK core error.
 *  @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to initialize the Azure IoT Provisioning Client due to buffer size is too small.
 */
UINT nx_azure_iot_provisioning_client_initialize(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                 NX_AZURE_IOT *nx_azure_iot_ptr,
                                                 const UCHAR *endpoint, UINT endpoint_length,
                                                 const UCHAR *id_scope, UINT id_scope_length,
                                                 const UCHAR *registration_id, UINT registration_id_length,
                                                 const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                                 const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                                 UCHAR *metadata_memory, UINT memory_size,
                                                 NX_SECURE_X509_CERT *trusted_certificate);

/**
 * @brief Cleanup the Azure IoT Provisioning Client.
 * @details This routine de-initializes the Azure IoT Provisioning Client.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully cleaned up AZ IoT Provisioning Client Instance.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to deinitialize the AZ IoT Provisioning Client Instance due to invalid parameter.
 *  @retval #NX_AZURE_IOT_NOT_FOUND Fail to deinitialize the AZ IoT Provisioning Client Instance due to resource not found.
 */
UINT nx_azure_iot_provisioning_client_deinitialize(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr);

/**
 * @brief Add more trusted certificate in the IoT Provisioning client if needed. It can be called multiple times to set certificate chain.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] trusted_certificate A pointer to a `NX_SECURE_X509_CERT`, which is the trusted certificate.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully add trusted certificate to AZ IoT Provisioning Client Instance.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to add trusted certificate to AZ IoT Provisioning Client Instance due to invalid parameter.
 *  @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add trusted certificate due to NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS is too small.
 */
UINT nx_azure_iot_provisioning_client_trusted_cert_add(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                       NX_SECURE_X509_CERT *trusted_certificate);

/**
 * @brief Set client certificate.
 * @details This routine sets the device certificate. It can be called multiple times to set certificate chain.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] x509_cert A pointer to a `NX_SECURE_X509_CERT` client cert.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully set device certs to AZ IoT Provisioning Client Instance.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set device certs to AZ IoT Provisioning Client Instance due to invalid parameter.
 *  @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to set device certificate due to NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS is too small.
 */
UINT nx_azure_iot_provisioning_client_device_cert_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                      NX_SECURE_X509_CERT *x509_cert);

/**
 * @brief Set symmetric key
 * @details This routine sets symmetric key.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] symmetric_key A UCHAR pointer to a symmetric key.
 * @param[in] symmetric_key_length Length of symmetric key.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully set symmetric key to the IoT Provisioning client.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set symmetric key to AZ IoT Provisioning Client Instance due to invalid parameter.
 */
UINT nx_azure_iot_provisioning_client_symmetric_key_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                        const UCHAR *symmetric_key, UINT symmetric_key_length);

#ifdef NXD_MQTT_OVER_WEBSOCKET
/**
 * @brief Enable using MQTT over WebSocket to register device to Azure IoT Provisioning service.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if MQTT over WebSocket is enabled.
 *   @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to enable MQTT over WebSocket due to invalid parameter.
 */
UINT nx_azure_iot_provisioning_client_websocket_enable(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr);
#endif /* NXD_MQTT_OVER_WEBSOCKET */

/**
 * @brief Register device to Azure IoT Provisioning service.
 * @details This routine registers device to Azure IoT Provisioning service.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] wait_option Number of ticks to block for device registration.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully register device to AZ IoT Provisioning.
 *  @retval #NX_AZURE_IOT_PENDING Successfully started registration of device but not yet completed.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to register device to AZ IoT Provisioning due to invalid parameter.
 *  @retval #NX_AZURE_IOT_SDK_CORE_ERROR Fail to register device to AZ IoT Provisioning due to SDK core error.
 *  @retval #NX_AZURE_IOT_SERVER_RESPONSE_ERROR Fail to register device to AZ IoT Provisioning due to error response from server.
 *  @retval #NX_AZURE_IOT_DISCONNECTED Fail to register device to AZ IoT Provisioning due to disconnection.
 *  @retval NX_DNS_QUERY_FAILED Fail to register device to AZ IoT Provisioning due to hostname can not be resolved.
 *  @retval NX_NO_PACKET Fail to register device to AZ IoT Provisioning due to no available packet in pool.
 *  @retval NX_INVALID_PARAMETERS Fail to register device to AZ IoT Provisioning due to invalid parameters.
 *  @retval NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE Fail to register device to AZ IoT Provisioning due to insufficient metadata space.
 *  @retval NX_SECURE_TLS_UNSUPPORTED_CIPHER Fail to register device to AZ IoT Provisioning due to unsupported cipher.
 *  @retval NXD_MQTT_ALREADY_CONNECTED Fail to register device to AZ IoT Provisioning due to MQTT session is not disconnected.
 *  @retval NXD_MQTT_CONNECT_FAILURE Fail to register device to AZ IoT Provisioning due to TCP/TLS connect error.
 *  @retval NXD_MQTT_COMMUNICATION_FAILURE Fail to register device to AZ IoT Provisioning due to MQTT connect error.
 *  @retval NXD_MQTT_ERROR_SERVER_UNAVAILABLE Fail to register device to AZ IoT Provisioning due to server unavailable.
 *  @retval NXD_MQTT_ERROR_NOT_AUTHORIZED Fail to register device to AZ IoT Provisioning due to authentication error.
 */
UINT nx_azure_iot_provisioning_client_register(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr, UINT wait_option);

/**
 * @brief Set registration completion callback
 * @details This routine sets the callback for registration completion.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] on_complete_callback Registration completion callback.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successful register completion callback.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to register completion callback due to invalid parameter.
 */
UINT nx_azure_iot_provisioning_client_completion_callback_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                              VOID (*on_complete_callback)(
                                                                    struct NX_AZURE_IOT_PROVISIONING_CLIENT_STRUCT *client_ptr,
                                                                    UINT status));

/**
 * @brief Set registration payload
 * @details This routine sets registration payload, which is JSON object.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[in] payload_ptr A pointer to registration payload.
 * @param[in] payload_length Length of `payload`. Does not include the `NULL` terminator.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS Successfully set registration payload to Azure IoT Provisioning Client.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to set registration payload Azure IoT Provisioning Client due to invalid parameter.
 */
UINT nx_azure_iot_provisioning_client_registration_payload_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                               const UCHAR *payload_ptr, UINT payload_length);

/**
 * @brief Get IoTHub device info into user supplied buffer.
 * @details This routine gets the device id and puts it into a user supplied buffer.
 *
 * @param[in] prov_client_ptr A pointer to a #NX_AZURE_IOT_PROVISIONING_CLIENT.
 * @param[out] iothub_hostname Buffer pointer that will contain IoTHub hostname.
 * @param[in/out] iothub_hostname_len Pointer to UINT that contains size of buffer supplied. On successful return,
 *               it contains bytes copied to the buffer.
 * @param[out] device_id Buffer pointer that will contain IoTHub deviceId.
 * @param[in/out] device_id_len Pointer to UINT that contains size of buffer supplied, once successfully return it contains bytes copied to buffer.
 * @return A `UINT` with the result of the API.
 *  @retval #NX_AZURE_IOT_SUCCESS The device info is successfully retrieved to user supplied buffers.
 *  @retval #NX_AZURE_IOT_INVALID_PARAMETER Fail to retrieve device info due to invalid parameter.
 *  @retval #NX_AZURE_IOT_WRONG_STATE Fail to retrieve device info due to wrong state.
 *  @retval #NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to retrieve device info due to buffer size is too small.
 */
UINT nx_azure_iot_provisioning_client_iothub_device_info_get(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                             UCHAR *iothub_hostname, UINT *iothub_hostname_len,
                                                             UCHAR *device_id, UINT *device_id_len);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_PROVISIONING_CLIENT_H */
