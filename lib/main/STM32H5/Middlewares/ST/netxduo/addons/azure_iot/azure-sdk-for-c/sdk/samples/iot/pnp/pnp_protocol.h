// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// DEPRECATED: This sample helper file has been deprecated.
// This file has *sample* helper functions for building and
// parsing IoT Plug and Play payloads.  Applications instead should use
// the officially supported APIs (see az_iot_hub_client.h and
// az_iot_hub_client_properties.h).

#ifndef PNP_PROTOCOL_H
#define PNP_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <azure/az_core.h>
#include <azure/az_iot.h>

/**
 * @brief Callback which is invoked for each property found by the
 * #pnp_process_device_twin_message() API.
 */
typedef void (*pnp_property_callback)(
    az_span component_name,
    az_json_token const* property_name,
    az_json_reader property_value_as_json,
    int32_t version,
    void* user_context_callback);

/**
 * @brief Callback which is invoked to append user properties to a payload.
 */
typedef az_result (*pnp_append_property_callback)(az_json_writer* jw, void* context);

/**
 * @brief Gets the MQTT topic that must be used for device to cloud telemetry messages.
 * @remark Telemetry MQTT Publish messages must have QoS At least once (1).
 * @remark This topic can also be used to set the MQTT Will message in the Connect message.
 *
 * @param[in] client The #az_iot_hub_client to use for this call.
 * @param[in] properties An optional #az_iot_message_properties object (can be NULL).
 * @param[in] component_name An optional component name if the telemetry is being sent from a
 * sub-component.
 * @param[out] mqtt_topic A buffer with sufficient capacity to hold the MQTT topic. If
 * successful, contains a null-terminated string with the topic that needs to be passed to the MQTT
 * client.
 * @param[in] mqtt_topic_size The size of \p out_mqtt_topic in bytes.
 * @param[out] out_mqtt_topic_length __[nullable]__ Contains the string length, in bytes, of \p
 * mqtt_topic . Can be `NULL`.
 */
void pnp_telemetry_get_publish_topic(
    az_iot_hub_client const* client,
    az_iot_message_properties* properties,
    az_span component_name,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length);

/**
 * @brief Parse a JSON payload for a PnP component command.
 *
 * @param[in] component_command Input JSON payload containing the details for the component command.
 * @param[out] out_component_name The parsed component name (if it exists).
 * @param[out] out_command_name The parsed command name.
 */
void pnp_parse_command_name(
    az_span component_command,
    az_span* out_component_name,
    az_span* out_command_name);

/**
 * @brief Build a reported property.
 *
 * @param[in] json_buffer An #az_span with sufficient capacity to hold the json payload.
 * @param[in] component_name The name of the component for the reported property.
 * @param[in] property_name The name of the property for which to send an update.
 * @param[in] append_callback The user callback to invoke to add the property value.
 * @param[in] context The user context which is passed to the callback.
 * @param[out] out_span A pointer to the #az_span containing the output json payload.
 */
void pnp_build_reported_property(
    az_span json_buffer,
    az_span component_name,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* context,
    az_span* out_span);

/**
 * @brief Build a reported property with the ack status.
 *
 * @param[in] json_buffer An #az_span with sufficient capacity to hold the json payload.
 * @param[in] component_name The name of the component for the reported property.
 * @param[in] property_name The name of the property for which to send an update.
 * @param[in] append_callback The user callback to invoke to add the property value.
 * @param[in] context The user context which is passed to the callback.
 * @param[in] code The return status for the reported property.
 * @param[in] version The ack version for the reported property.
 * @param[in] description The optional description for the reported property.
 * @param[out] out_span A pointer to the #az_span containing the output json payload.
 */
void pnp_build_reported_property_with_status(
    az_span json_buffer,
    az_span component_name,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* context,
    int32_t code,
    int32_t version,
    az_span description,
    az_span* out_span);

/**
 * @brief Build a simple telemetry message using one property name and one value.
 *
 * @param[in] json_buffer An #az_span with sufficient capacity to hold the json payload.
 * @param[in] property_name The name of the property for which to send telemetry.
 * @param[in] append_callback The user callback to invoke to add the property value.
 * @param[in] property_value The property value which is passed to the callback to be appended.
 * @param[out] out_span A pointer to the #az_span containing the output json payload.
 */
void pnp_build_telemetry_message(
    az_span json_buffer,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* property_value,
    az_span* out_span);

/**
 * @brief Process the twin properties and invoke user callback for each property.
 *
 * @param[in] twin_message_span The #az_span of the received message to process.
 * @param[in] is_partial Boolean stating whether the JSON document is partial or not.
 * @param[in] components_ptr A pointer to a set of `#az_span` pointers containing all the names for
 * components.
 * @param[in] components_num Number of components in the set pointed to by \p components_ptr .
 * @param[in] property_callback The callback which is called on each twin property.
 * @param[in] context_ptr Pointer to user context.
 */
void pnp_process_device_twin_message(
    az_span twin_message_span,
    bool is_partial,
    az_span const** components_ptr,
    int32_t components_num,
    pnp_property_callback property_callback,
    void* context_ptr);

#endif // PNP_PROTOCOL_H
