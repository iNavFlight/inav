// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_iot_provisioning_client.h
 *
 * @brief Definition for the Azure Device Provisioning client.
 * @remark The Device Provisioning MQTT protocol is described at
 * https://docs.microsoft.com/azure/iot-dps/iot-dps-mqtt-support
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_IOT_PROVISIONING_CLIENT_H
#define _az_IOT_PROVISIONING_CLIENT_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_common.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief The client is fixed to a specific version of the Azure IoT Provisioning service.
 */
#define AZ_IOT_PROVISIONING_SERVICE_VERSION "2019-03-31"

/**
 * @brief Azure IoT Provisioning Client options.
 *
 */
typedef struct
{
  /**
   * The user-agent is a formatted string that will be used for Azure IoT usage statistics.
   */
  az_span user_agent;
} az_iot_provisioning_client_options;

/**
 * @brief Azure IoT Provisioning Client.
 *
 */
typedef struct
{
  struct
  {
    az_span global_device_endpoint;
    az_span id_scope;
    az_span registration_id;
    az_iot_provisioning_client_options options;
  } _internal;
} az_iot_provisioning_client;

/**
 * @brief Gets the default Azure IoT Provisioning Client options.
 *
 * Call this to obtain an initialized #az_iot_provisioning_client_options structure that
 * can be afterwards modified and passed to az_iot_provisioning_client_init().
 *
 * @return #az_iot_provisioning_client_options.
 */
AZ_NODISCARD az_iot_provisioning_client_options az_iot_provisioning_client_options_default();

/**
 * @brief Initializes an Azure IoT Provisioning Client.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] global_device_hostname The device provisioning services global host name.
 * @param[in] id_scope The ID Scope.
 * @param[in] registration_id The Registration ID. This must match the client certificate name (CN
 * part of the certificate subject). Must conform to the limitations listed in the link below:
 * https://docs.microsoft.com/azure/iot-dps/concepts-service#registration-id
 * @param[in] options __[nullable]__ A reference to an #az_iot_provisioning_client_options
 * structure. Can be `NULL` for default options.
 * @pre \p client must not be `NULL`.
 * @pre \p global_device_hostname must be a valid span of size greater than 0.
 * @pre \p id_scope must be a valid span of size greater than 0.
 * @pre \p registration_id must be a valid span of size greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The client was initialized successfully.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_init(
    az_iot_provisioning_client* client,
    az_span global_device_hostname,
    az_span id_scope,
    az_span registration_id,
    az_iot_provisioning_client_options const* options);

/**
 * @brief Gets the MQTT user name.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[out] mqtt_user_name A buffer with sufficient capacity to hold the MQTT user name. If
 * successful, contains a null-terminated string with the user name that needs to be passed to the
 * MQTT client.
 * @param[in] mqtt_user_name_size The size, in bytes of \p mqtt_user_name.
 * @param[out] out_mqtt_user_name_length __[nullable]__ Contains the string length, in bytes, of \p
 * mqtt_user_name. Can be `NULL`.
 * @pre \p client must not be `NULL`.
 * @pre \p mqtt_user_name must not be `NULL`.
 * @pre \p mqtt_user_name_size must be greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The user name was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_get_user_name(
    az_iot_provisioning_client const* client,
    char* mqtt_user_name,
    size_t mqtt_user_name_size,
    size_t* out_mqtt_user_name_length);

/**
 * @brief Gets the MQTT client id.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[out] mqtt_client_id A buffer with sufficient capacity to hold the MQTT client id. If
 * successful, contains a null-terminated string with the client id that needs to be passed to the
 * MQTT client.
 * @param[in] mqtt_client_id_size The size, in bytes of \p mqtt_client_id.
 * @param[out] out_mqtt_client_id_length __[nullable]__ Contains the string length, in bytes, of of
 * \p mqtt_client_id. Can be `NULL`.
 * @pre \p client must not be `NULL`.
 * @pre \p mqtt_client_id must not be `NULL`.
 * @pre \p mqtt_client_id_size must be greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The client id was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_get_client_id(
    az_iot_provisioning_client const* client,
    char* mqtt_client_id,
    size_t mqtt_client_id_size,
    size_t* out_mqtt_client_id_length);

/*
 *
 * SAS Token APIs
 *
 *   Use the following APIs when the Shared Access Key is available to the application or stored
 *   within a Hardware Security Module. The APIs are not necessary if X509 Client Certificate
 *   Authentication is used.
 *
 *   The TPM Asymmetric Device Provisioning protocol is not supported on the MQTT protocol. TPMs can
 *   still be used to securely store and perform HMAC-SHA256 operations for SAS tokens.
 */

/**
 * @brief Gets the Shared Access clear-text signature.
 *
 * The application must obtain a valid clear-text signature using this API, sign it using
 * HMAC-SHA256 using the Shared Access Key as password then Base64 encode the result.
 *
 * @remark More information available at
 * https://docs.microsoft.com/azure/iot-dps/concepts-symmetric-key-attestation#detailed-attestation-process
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] token_expiration_epoch_time The time, in seconds, from 1/1/1970.
 * @param[in] signature An empty #az_span with sufficient capacity to hold the SAS signature.
 * @param[out] out_signature The output #az_span containing the SAS signature.
 * @pre \p client must not be `NULL`.
 * @pre \p token_expiration_epoch_time must be greater than 0.
 * @pre \p signature must be a valid span of size greater than 0.
 * @pre \p out_signature must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The signature was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_sas_get_signature(
    az_iot_provisioning_client const* client,
    uint64_t token_expiration_epoch_time,
    az_span signature,
    az_span* out_signature);

/**
 * @brief Gets the MQTT password.
 * @remark The MQTT password must be an empty string if X509 Client certificates are used. Use this
 * API only when authenticating with SAS tokens.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] base64_hmac_sha256_signature The Base64 encoded value of the HMAC-SHA256(signature,
 * SharedAccessKey). The signature is obtained by using
 * #az_iot_provisioning_client_sas_get_signature.
 * @param[in] token_expiration_epoch_time The time, in seconds, from 1/1/1970.
 * @param[in] key_name The Shared Access Key Name (Policy Name). This is optional. For security
 * reasons we recommend using one key per device instead of using a global policy key.
 * @param[out] mqtt_password A buffer with sufficient capacity to hold the MQTT password. If
 * successful, contains a null-terminated string with the password that needs to be passed to the
 * MQTT client.
 * @param[in] mqtt_password_size The size, in bytes of \p mqtt_password.
 * @param[out] out_mqtt_password_length __[nullable]__ Contains the string length, in bytes, of \p
 * mqtt_password. Can be `NULL`.
 * @pre \p client must not be `NULL`.
 * @pre \p base64_hmac_sha256_signature must be a valid span of size greater than 0.
 * @pre \p token_expiration_epoch_time must be greater than 0.
 * @pre \p mqtt_password must not be `NULL`.
 * @pre \p mqtt_password_size must be greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The password was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_sas_get_password(
    az_iot_provisioning_client const* client,
    az_span base64_hmac_sha256_signature,
    uint64_t token_expiration_epoch_time,
    az_span key_name,
    char* mqtt_password,
    size_t mqtt_password_size,
    size_t* out_mqtt_password_length);

/*
 *
 * Register APIs
 *
 *   Use the following APIs when the Shared Access Key is available to the application or stored
 *   within a Hardware Security Module. The APIs are not necessary if X509 Client Certificate
 *   Authentication is used.
 */

/**
 * @brief The MQTT topic filter to subscribe to register responses.
 * @remark Register MQTT Publish messages will have QoS At most once (0).
 */
#define AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC "$dps/registrations/res/#"

/**
 * @brief The registration operation state.
 * @remark This is returned only when the operation completed.
 *
 */
typedef struct
{
  /**
   * Assigned Azure IoT Hub hostname.
   * @remark This is only available if `error_code` is success.
   */
  az_span assigned_hub_hostname;

  /**
   * Assigned device ID.
   */
  az_span device_id;

  /**
   * The error code.
   */
  az_iot_status error_code;

  /**
   * The extended, 6 digit error code.
   */
  uint32_t extended_error_code;

  /**
   * Error description.
   */
  az_span error_message;

  /**
   * Submit this ID when asking for Azure IoT service-desk help.
   */
  az_span error_tracking_id;

  /**
   * Submit this timestamp when asking for Azure IoT service-desk help.
   */
  az_span error_timestamp;
} az_iot_provisioning_client_registration_state;

/**
 * @brief Azure IoT Provisioning Service operation status.
 *
 */
typedef enum
{
  /**
   * Starting state (not assigned).
   */
  AZ_IOT_PROVISIONING_STATUS_UNASSIGNED,
  /**
   * Assigning in progress.
   */
  AZ_IOT_PROVISIONING_STATUS_ASSIGNING,

  // Device assignment operation complete.
  /**
   * Device was assigned successfully.
   */
  AZ_IOT_PROVISIONING_STATUS_ASSIGNED,

  /**
   * The provisioning for the device failed.
   */
  AZ_IOT_PROVISIONING_STATUS_FAILED,

  /**
   * The provisioning for this device was disabled.
   */
  AZ_IOT_PROVISIONING_STATUS_DISABLED,
} az_iot_provisioning_client_operation_status;

/**
 * @brief Register or query operation response.
 *
 */
typedef struct
{
  /**
   * The id of the register operation.
   */
  az_span operation_id;

  // Avoid using enum as the first field within structs, to allow for { 0 } initialization.
  // This is a workaround for IAR compiler warning [Pe188]: enumerated type mixed with another type.

  /**
   * The current request status.
   * @remark The authoritative response for the device registration operation (which may require
   * several requests) is available only through #operation_status.
   */
  az_iot_status status;

  /**
   * The status of the register operation.
   */
  az_iot_provisioning_client_operation_status operation_status;

  /**
   * Recommended timeout before sending the next MQTT publish.
   */
  uint32_t retry_after_seconds;

  /**
   * If the operation is complete (success or error), the registration state will contain the hub
   * and device id in case of success.
   */
  az_iot_provisioning_client_registration_state registration_state;
} az_iot_provisioning_client_register_response;

/**
 * @brief Attempts to parse a received message's topic.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] received_topic An #az_span containing the received MQTT topic.
 * @param[in] received_payload An #az_span containing the received MQTT payload.
 * @param[out] out_response If the message is register-operation related, this will contain the
 * #az_iot_provisioning_client_register_response.
 * @pre \p client must not be `NULL`.
 * @pre \p received_topic must be a valid span of size greater than or equal to 0.
 * @pre \p received_payload must be a valid span of size greater than or equal to 0.
 * @pre \p out_response must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The topic and payload were parsed successfully.
 * @retval #AZ_ERROR_IOT_TOPIC_NO_MATCH If the topic is not matching the expected format.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_parse_received_topic_and_payload(
    az_iot_provisioning_client const* client,
    az_span received_topic,
    az_span received_payload,
    az_iot_provisioning_client_register_response* out_response);

/**
 * @brief Checks if the status indicates that the service has an authoritative result of the
 * register operation. The operation may have completed in either success or error. Completed
 * states are:
 *
 * - #AZ_IOT_PROVISIONING_STATUS_ASSIGNED
 * - #AZ_IOT_PROVISIONING_STATUS_FAILED
 * - #AZ_IOT_PROVISIONING_STATUS_DISABLED
 *
 * @param[in] operation_status The status used to check if the operation completed.
 * @return `true` if the operation completed. `false` otherwise.
 */
AZ_INLINE bool az_iot_provisioning_client_operation_complete(
    az_iot_provisioning_client_operation_status operation_status)
{
  return (operation_status > AZ_IOT_PROVISIONING_STATUS_ASSIGNING);
}

/**
 * @brief Gets the MQTT topic that must be used to submit a Register request.
 * @remark The payload of the MQTT publish message may contain a JSON document formatted according
 * to the [Provisioning Service's Device Registration document]
 * (https://docs.microsoft.com/rest/api/iot-dps/device/runtime-registration/register-device#deviceregistration)
 * specification.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[out] mqtt_topic A buffer with sufficient capacity to hold the MQTT topic filter. If
 * successful, contains a null-terminated string with the topic filter that needs to be passed to
 * the MQTT client.
 * @param[in] mqtt_topic_size The size, in bytes of \p mqtt_topic.
 * @param[out] out_mqtt_topic_length __[nullable]__ Contains the string length, in bytes, of \p
 * mqtt_topic. Can be `NULL`.
 * @pre \p client must not be `NULL`.
 * @pre \p mqtt_topic must not be `NULL`.
 * @pre \p mqtt_topic_size must be greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The topic was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_register_get_publish_topic(
    az_iot_provisioning_client const* client,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length);

/**
 * @brief Gets the MQTT topic that must be used to submit a Register Status request.
 * @remark The payload of the MQTT publish message should be empty.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] operation_id The received operation_id from the
 * #az_iot_provisioning_client_register_response response.
 * @param[out] mqtt_topic A buffer with sufficient capacity to hold the MQTT topic filter. If
 * successful, contains a null-terminated string with the topic filter that needs to be passed to
 * the MQTT client.
 * @param[in] mqtt_topic_size The size, in bytes of \p mqtt_topic.
 * @param[out] out_mqtt_topic_length __[nullable]__ Contains the string length, in bytes, of \p
 * mqtt_topic. Can be `NULL`.
 * @pre \p client must not be `NULL`.
 * @pre \p operation_id must be a valid span of size greater than 0.
 * @pre \p mqtt_topic must not be `NULL`.
 * @pre \p mqtt_topic_size must be greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The topic was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_query_status_get_publish_topic(
    az_iot_provisioning_client const* client,
    az_span operation_id,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length);

/**
 * @brief Azure IoT Provisioning Client options for
 * az_iot_provisioning_client_get_request_payload(). Not currently used.  Reserved for future use.
 *
 */
typedef struct
{
  struct
  {
    /// Currently, this is unused, but needed as a placeholder since we can't have an empty struct.
    bool unused;
  } _internal;
} az_iot_provisioning_client_payload_options;

/**
 * @brief Builds the optional payload for a provisioning request.
 * @remark Use this API to build an MQTT payload during registration.
 *         This call is optional for most scenarios. Some service
 *         applications may require `custom_payload_property` specified during
 *         registration to take additional decisions during provisioning time.
 *         For example, if you need to register an IoT Plug and Play device you must
 *         specify its model_id with this API via the `custom_payload_property`
 *         `{"modelId":"your_model_id"}`.
 *
 * @param[in] client The #az_iot_provisioning_client to use for this call.
 * @param[in] custom_payload_property __[nullable]__ Custom JSON to be added to this payload.
 * Can be `NULL`.
 * @param[in] options __[nullable]__ Reserved field for future options to this function.  Must be
 * `NULL`.
 * @param[out] mqtt_payload A buffer with sufficient capacity to hold the MQTT payload.
 * @param[in] mqtt_payload_size The size, in bytes of \p mqtt_payload.
 * @param[out] out_mqtt_payload_length Contains the length, in bytes, written to \p mqtt_payload on
 * success.
 * @pre \p client must not be `NULL`.
 * @pre \p options must be `NULL`.
 * @pre \p mqtt_payload must not be `NULL`.
 * @pre \p mqtt_payload_size must be greater than 0.
 * @pre \p out_mqtt_payload_length must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The payload was created successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_iot_provisioning_client_get_request_payload(
    az_iot_provisioning_client const* client,
    az_span custom_payload_property,
    az_iot_provisioning_client_payload_options const* options,
    uint8_t* mqtt_payload,
    size_t mqtt_payload_size,
    size_t* out_mqtt_payload_length);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_IOT_PROVISIONING_CLIENT_H
