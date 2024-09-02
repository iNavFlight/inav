// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_iot_common.h
 *
 * @brief Azure IoT common definitions.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_IOT_CORE_H
#define _az_IOT_CORE_H

#include <azure/core/az_log.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief The type represents the various #az_result success and error conditions specific to the
 * IoT clients within the SDK.
 */
enum az_result_iot
{
  // === IoT error codes ===
  /// The IoT topic is not matching the expected format.
  AZ_ERROR_IOT_TOPIC_NO_MATCH = _az_RESULT_MAKE_ERROR(_az_FACILITY_IOT, 1),

  /// While iterating, there are no more properties to return.
  AZ_ERROR_IOT_END_OF_PROPERTIES = _az_RESULT_MAKE_ERROR(_az_FACILITY_IOT, 2),
};

/**
 * @brief Identifies the #az_log_classification produced specifically by the IoT clients within the
 * SDK.
 */
enum az_log_classification_iot
{
  AZ_LOG_MQTT_RECEIVED_TOPIC
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT_MQTT, 1), ///< Accepted MQTT topic received.

  AZ_LOG_MQTT_RECEIVED_PAYLOAD
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT_MQTT, 2), ///< Accepted MQTT payload received.

  AZ_LOG_IOT_RETRY = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT, 1), ///< IoT Client retry.

  AZ_LOG_IOT_SAS_TOKEN
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT, 2), ///< IoT Client generated new SAS token.

  AZ_LOG_IOT_AZURERTOS
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT, 3), ///< Azure IoT classification for Azure RTOS.

  AZ_LOG_IOT_ADU
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_IOT, 4), ///< Azure IoT classification for ADU APIs.
};

enum
{
  AZ_IOT_DEFAULT_MQTT_CONNECT_PORT = 8883,
  AZ_IOT_DEFAULT_MQTT_CONNECT_KEEPALIVE_SECONDS = 240
};

/**
 * @brief Azure IoT service status codes.
 *
 * @note https://docs.microsoft.com/azure/iot-central/core/troubleshoot-connection#error-codes
 *
 */
typedef enum
{
  // Default, unset value
  AZ_IOT_STATUS_UNKNOWN = 0,

  // Service success codes
  AZ_IOT_STATUS_OK = 200,
  AZ_IOT_STATUS_ACCEPTED = 202,
  AZ_IOT_STATUS_NO_CONTENT = 204,

  // Service error codes
  AZ_IOT_STATUS_BAD_REQUEST = 400,
  AZ_IOT_STATUS_UNAUTHORIZED = 401,
  AZ_IOT_STATUS_FORBIDDEN = 403,
  AZ_IOT_STATUS_NOT_FOUND = 404,
  AZ_IOT_STATUS_NOT_ALLOWED = 405,
  AZ_IOT_STATUS_NOT_CONFLICT = 409,
  AZ_IOT_STATUS_PRECONDITION_FAILED = 412,
  AZ_IOT_STATUS_REQUEST_TOO_LARGE = 413,
  AZ_IOT_STATUS_UNSUPPORTED_TYPE = 415,
  AZ_IOT_STATUS_THROTTLED = 429,
  AZ_IOT_STATUS_CLIENT_CLOSED = 499,
  AZ_IOT_STATUS_SERVER_ERROR = 500,
  AZ_IOT_STATUS_BAD_GATEWAY = 502,
  AZ_IOT_STATUS_SERVICE_UNAVAILABLE = 503,
  AZ_IOT_STATUS_TIMEOUT = 504,
} az_iot_status;

/*
 *
 * Properties APIs
 *
 *   IoT message properties are used for Device-to-Cloud (D2C) as well as Cloud-to-Device (C2D).
 *   Properties are always appended to the MQTT topic of the published or received message and
 *   must contain percent-encoded names and values.
 */

/// Add unique identification to a message.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_MESSAGE_ID "%24.mid"

/// Used in distributed tracing.
/// @note More information here:
/// https://docs.microsoft.com/azure/iot-hub/iot-hub-distributed-tracing.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_CORRELATION_ID "%24.cid"

/// URL encoded and of the form `text%2Fplain` or `application%2Fjson`, etc.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_CONTENT_TYPE "%24.ct"

/// UTF-8, UTF-16, etc.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_CONTENT_ENCODING "%24.ce"

/// User ID field.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_USER_ID "%24.uid"

/// Creation time of the message.
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_PROPERTIES_CREATION_TIME "%24.ctime"

/// Name of the component
/// @note It can be used with IoT message property APIs by wrapping the macro in a
/// #AZ_SPAN_FROM_STR macro as a parameter, where needed.
#define AZ_IOT_MESSAGE_COMPONENT_NAME "%24.sub"

/**
 * @brief Telemetry or C2D properties.
 *
 */
typedef struct
{
  struct
  {
    az_span properties_buffer;
    int32_t properties_written;
    uint32_t current_property_index;
  } _internal;
} az_iot_message_properties;

/**
 * @brief Initializes the Telemetry or C2D properties.
 *
 * @note The properties must adhere to the character restrictions listed in the below link.
 * https://docs.microsoft.com/azure/iot-hub/iot-hub-devguide-messages-construct
 *
 * @param[in] properties The #az_iot_message_properties to initialize.
 * @param[in] buffer Can either be an unfilled (but properly sized) #az_span or an #az_span
 * containing properly formatted (with above mentioned characters encoded if applicable) properties
 * with the following format: {name}={value}&{name}={value}.
 * @param[in] written_length The length of the properly formatted properties already initialized
 * within the buffer. If the \p buffer is unfilled (uninitialized), this should be 0.
 * @pre \p properties must not be `NULL`.
 * @pre \p buffer must be a valid span of size greater than 0.
 * @pre \p written_length must be greater than or equal to 0.
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_iot_message_properties_init(
    az_iot_message_properties* properties,
    az_span buffer,
    int32_t written_length);

/**
 * @brief Appends a name-value property to the list of properties.
 *
 * @note The properties must adhere to the character restrictions listed in the below link.
 * https://docs.microsoft.com/azure/iot-hub/iot-hub-devguide-messages-construct
 *
 * @param[in] properties The #az_iot_message_properties to use for this call.
 * @param[in] name The name of the property. Must be a valid, non-empty span.
 * @param[in] value The value of the property. Must be a valid, non-empty span.
 * @pre \p properties must not be `NULL`.
 * @pre \p name must be a valid span of size greater than 0.
 * @pre \p value must be a valid span of size greater than 0.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The operation was performed successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE There was not enough space to append the property.
 */
AZ_NODISCARD az_result az_iot_message_properties_append(
    az_iot_message_properties* properties,
    az_span name,
    az_span value);

/**
 * @brief Finds the value of a property.
 * @remark This will return the first value of the property with the given name if multiple
 * properties with the same name exist.
 *
 * @param[in] properties The #az_iot_message_properties to use for this call.
 * @param[in] name The name of the property to search for.
 * @param[out] out_value An #az_span containing the value of the found property.
 * @pre \p properties must not be `NULL`.
 * @pre \p name must be a valid span of size greater than 0.
 * @pre \p out_value must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The property was successfully found.
 * @retval #AZ_ERROR_ITEM_NOT_FOUND The property could not be found.
 */
AZ_NODISCARD az_result az_iot_message_properties_find(
    az_iot_message_properties* properties,
    az_span name,
    az_span* out_value);

/**
 * @brief Iterates over the list of properties.
 *
 * @param[in] properties The #az_iot_message_properties to use for this call.
 * @param[out] out_name A pointer to an #az_span containing the name of the next property.
 * @param[out] out_value A pointer to an #az_span containing the value of the next property.
 * @pre \p properties must not be `NULL`.
 * @pre \p out_name must not be `NULL`.
 * @pre \p out_value must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK A property was retrieved successfully.
 * @retval #AZ_ERROR_IOT_END_OF_PROPERTIES The API reached the end of the properties to retrieve.
 */
AZ_NODISCARD az_result az_iot_message_properties_next(
    az_iot_message_properties* properties,
    az_span* out_name,
    az_span* out_value);

/**
 * @brief Checks if the status indicates a successful operation.
 *
 * @param[in] status The #az_iot_status to verify.
 * @return `true` if the status indicates success. `false` otherwise.
 */
AZ_NODISCARD AZ_INLINE bool az_iot_status_succeeded(az_iot_status status)
{
  return status < AZ_IOT_STATUS_BAD_REQUEST;
}

/**
 * @brief Checks if the status indicates a retriable error occurred during the
 *        operation.
 *
 * @param[in] status The #az_iot_status to verify.
 * @return `true` if the operation should be retried. `false` otherwise.
 */
AZ_NODISCARD AZ_INLINE bool az_iot_status_retriable(az_iot_status status)
{
  return ((status == AZ_IOT_STATUS_THROTTLED) || (status == AZ_IOT_STATUS_SERVER_ERROR));
}

/**
 * @brief Calculates the recommended delay before retrying an operation that failed.
 *
 * @param[in] operation_msec The time it took, in milliseconds, to perform the operation that
 *                           failed.
 * @param[in] attempt The number of failed retry attempts.
 * @param[in] min_retry_delay_msec The minimum time, in milliseconds, to wait before a retry.
 * @param[in] max_retry_delay_msec The maximum time, in milliseconds, to wait before a retry.
 * @param[in] random_jitter_msec A random value between 0 and the maximum allowed jitter, in
 * milliseconds.
 * @pre \p operation_msec must be between 0 and INT32_MAX - 1.
 * @pre \p attempt must be between 0 and INT16_MAX - 1.
 * @pre \p min_retry_delay_msec must be between 0 and INT32_MAX - 1.
 * @pre \p max_retry_delay_msec must be between 0 and INT32_MAX - 1.
 * @pre \p random_jitter_msec must be between 0 and INT32_MAX - 1.
 * @return The recommended delay in milliseconds.
 */
AZ_NODISCARD int32_t az_iot_calculate_retry_delay(
    int32_t operation_msec,
    int16_t attempt,
    int32_t min_retry_delay_msec,
    int32_t max_retry_delay_msec,
    int32_t random_jitter_msec);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_IOT_CORE_H
