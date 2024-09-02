// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * The Temperature Controller component implements the properties, telemetry, and command declared
 * in the model defined by dtmi:com:example:TemperatureController;1.
 * See the readme for more information about this model.
 *
 * This sample does not setup its own connection to Azure IoT Hub.  It is invoked by
 * ../paho_iot_pnp_component_sample.c which handles connection management.
 */

#include <stddef.h>

#include <azure/az_core.h>

#include <azure/iot/az_iot_hub_client_properties.h>
#include <iot_sample_common.h>

#include "pnp_temperature_controller_component.h"

// Plug and Play command values
static az_span const command_reboot_name = AZ_SPAN_LITERAL_FROM_STR("reboot");
static az_span const command_empty_response_payload = AZ_SPAN_LITERAL_FROM_STR("{}");

// Plug and Play telemetry values
static az_span const telemetry_working_set_name = AZ_SPAN_LITERAL_FROM_STR("workingSet");

// Plug and Play property values
static az_span const reported_property_serial_number_name
    = AZ_SPAN_LITERAL_FROM_STR("serialNumber");
static az_span property_reported_serial_number_property_value = AZ_SPAN_LITERAL_FROM_STR("ABCDEFG");

void pnp_temperature_controller_process_command_request(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client,
    az_iot_hub_client_command_request const* command_request,
    az_span command_received_payload)
{
  (void)command_received_payload; // Not used

  pnp_mqtt_message publish_message;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      pnp_mqtt_message_init(&publish_message), "Failed to initialize pnp_mqtt_message");

  az_iot_status status;

  if (az_span_is_content_equal(command_reboot_name, command_request->command_name))
  {
    // Simulate invocation of command.
    IOT_SAMPLE_LOG("Client invoking reboot command on Temperature Controller.");
    publish_message.out_payload = command_empty_response_payload;
    status = AZ_IOT_STATUS_OK;
  }
  else
  {
    // Unsupported command
    IOT_SAMPLE_LOG_AZ_SPAN(
        "Command not supported on Temperature Controller:", command_request->command_name);
    publish_message.out_payload = command_empty_response_payload;
    status = AZ_IOT_STATUS_NOT_FOUND;
  }

  // Get the commands response topic to publish the command response.
  az_result rc = az_iot_hub_client_commands_response_get_publish_topic(
      hub_client,
      command_request->request_id,
      (uint16_t)status,
      publish_message.topic,
      publish_message.topic_length,
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the commands response topic");

  // Publish the command response.
  publish_mqtt_message(
      mqtt_client, publish_message.topic, publish_message.out_payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client published command response.");
  IOT_SAMPLE_LOG("Status: %d", status);
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", publish_message.out_payload);
}

// temp_controller_build_serial_number_property_payload builds the JSON payload for
// the serial number property.
static void temp_controller_build_serial_number_property_payload(
    az_span payload,
    az_span* out_payload)
{
  az_json_writer jw;

  const char* const log_message = "Failed to build reported property payload";

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, reported_property_serial_number_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, property_reported_serial_number_property_value),
      log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log_message);

  *out_payload = az_json_writer_get_bytes_used_in_destination(&jw);
}

void pnp_temperature_controller_send_serial_number(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client)
{
  pnp_mqtt_message publish_message;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      pnp_mqtt_message_init(&publish_message), "Failed to initialize pnp_mqtt_message");

  // Get the property update topic to send a reported property update.
  az_result rc = az_iot_hub_client_properties_get_reported_publish_topic(
      hub_client,
      pnp_mqtt_get_request_id(),
      publish_message.topic,
      publish_message.topic_length,
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the property update topic");

  // Build the serial number reported property message.
  temp_controller_build_serial_number_property_payload(
      publish_message.payload, &publish_message.out_payload);

  // Publish the serial number reported property update.
  publish_mqtt_message(
      mqtt_client, publish_message.topic, publish_message.out_payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS(
      "Client sent `%.*s` reported property message.",
      az_span_size(reported_property_serial_number_name),
      az_span_ptr(reported_property_serial_number_name));
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", publish_message.out_payload);
  IOT_SAMPLE_LOG(" "); // Formatting
}

// temp_controller_build_telemetry_message builds the JSON payload for the working set telemetry.
static void temp_controller_build_working_set_payload(az_span payload, az_span* out_payload)
{
  int32_t working_set_ram_in_kibibytes = rand() % 128;

  az_json_writer jr;

  const char* const log_message = "Failed to build telemetry payload";

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jr, payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jr), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jr, telemetry_working_set_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_int32(&jr, working_set_ram_in_kibibytes), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jr), log_message);

  *out_payload = az_json_writer_get_bytes_used_in_destination(&jr);
}

void pnp_temperature_controller_send_workingset(
    az_iot_hub_client const* hub_client,
    MQTTClient mqtt_client)
{
  pnp_mqtt_message publish_message;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      pnp_mqtt_message_init(&publish_message), "Failed to initialize pnp_mqtt_message");

  // Get the telemetry topic to publish the telemetry message.
  az_result rc = az_iot_hub_client_telemetry_get_publish_topic(
      hub_client, NULL, publish_message.topic, publish_message.topic_length, NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Unable to get the telemetry topic");

  // Build the telemetry message payload.
  temp_controller_build_working_set_payload(publish_message.payload, &publish_message.out_payload);

  // Publish the telemetry message.
  publish_mqtt_message(
      mqtt_client, publish_message.topic, publish_message.out_payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client published the telemetry message for Temperature Controller.");
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", publish_message.out_payload);
}
