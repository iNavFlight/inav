// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/*
 * The Device Info component is a sample that sends (simulated) device information.
 * It implements the model defined by dtmi:azure:DeviceManagement:DeviceInformation;1.
 * See the readme for more information about this model.
 *
 * This sample does not setup its own connection to Azure IoT Hub.  It is invoked by
 * ../paho_iot_pnp_component_sample.c which handles connection management.
 */

#include <stddef.h>

#include <azure/az_core.h>

#include <azure/iot/az_iot_hub_client_properties.h>
#include <iot_sample_common.h>

#include "pnp_device_info_component.h"

#define DOUBLE_DECIMAL_PLACE_DIGITS 2

static az_span deviceInformation_1_name = AZ_SPAN_LITERAL_FROM_STR("deviceInformation");

// Reported property keys and values
static az_span const software_version_property_name = AZ_SPAN_LITERAL_FROM_STR("swVersion");
static az_span const software_version_property_value = AZ_SPAN_LITERAL_FROM_STR("1.0.0.0");
static az_span const manufacturer_property_name = AZ_SPAN_LITERAL_FROM_STR("manufacturer");
static az_span const manufacturer_property_value = AZ_SPAN_LITERAL_FROM_STR("Sample-Manufacturer");
static az_span const model_property_name = AZ_SPAN_LITERAL_FROM_STR("model");
static az_span const model_property_value = AZ_SPAN_LITERAL_FROM_STR("pnp-sample-Model-123");
static az_span const os_name_property_name = AZ_SPAN_LITERAL_FROM_STR("osName");
static az_span const os_name_property_value = AZ_SPAN_LITERAL_FROM_STR("Contoso");
static az_span const processor_architecture_property_name
    = AZ_SPAN_LITERAL_FROM_STR("processorArchitecture");
static az_span const processor_architecture_property_value
    = AZ_SPAN_LITERAL_FROM_STR("Contoso-Arch-64bit");
static az_span const processor_manufacturer_property_name
    = AZ_SPAN_LITERAL_FROM_STR("processorManufacturer");
static az_span const processor_manufacturer_property_value
    = AZ_SPAN_LITERAL_FROM_STR("Processor Manufacturer(TM)");
static az_span const total_storage_property_name = AZ_SPAN_LITERAL_FROM_STR("totalStorage");
static double const total_storage_property_value = 1024.0;
static az_span const total_memory_property_name = AZ_SPAN_LITERAL_FROM_STR("totalMemory");
static double const total_memory_property_value = 128;

// pnp_device_info_write_property_payload writes the JSON payload that contains simulated
// device information.
static void pnp_device_info_write_property_payload(
    az_iot_hub_client* hub_client,
    az_span payload,
    az_span* out_payload)
{
  char const* const log_message = "Failed to write reported property payload for device info";

  az_json_writer jw;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, payload, NULL), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log_message);

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_iot_hub_client_properties_writer_begin_component(
          hub_client, &jw, deviceInformation_1_name),
      log_message);

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, manufacturer_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, manufacturer_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, model_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, model_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, software_version_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, software_version_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, os_name_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, os_name_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, processor_architecture_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, processor_architecture_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, processor_manufacturer_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_string(&jw, processor_manufacturer_property_value), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, total_storage_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_double(&jw, total_storage_property_value, DOUBLE_DECIMAL_PLACE_DIGITS),
      log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, total_memory_property_name), log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_double(&jw, total_memory_property_value, DOUBLE_DECIMAL_PLACE_DIGITS),
      log_message);

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_iot_hub_client_properties_writer_end_component((az_iot_hub_client const*)0x1, &jw),
      log_message);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log_message);

  *out_payload = az_json_writer_get_bytes_used_in_destination(&jw);
}

void pnp_device_info_send_reported_properties(az_iot_hub_client* hub_client, MQTTClient mqtt_client)
{
  pnp_mqtt_message publish_message;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      pnp_mqtt_message_init(&publish_message), "Failed to initialize pnp_mqtt_message");

  // Get the property topic to send a reported property update.
  az_result rc = az_iot_hub_client_properties_get_reported_publish_topic(
      hub_client,
      pnp_mqtt_get_request_id(),
      publish_message.topic,
      publish_message.topic_length,
      NULL);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(rc, "Failed to get the property update topic");

  // Get the payload to send
  pnp_device_info_write_property_payload(
      hub_client, publish_message.payload, &publish_message.out_payload);

  // Publish the device info reported property update.
  publish_mqtt_message(
      mqtt_client, publish_message.topic, publish_message.payload, IOT_SAMPLE_MQTT_PUBLISH_QOS);
  IOT_SAMPLE_LOG_SUCCESS("Client sent reported property message for device info.");
  IOT_SAMPLE_LOG_AZ_SPAN("Payload:", publish_message.payload);
  IOT_SAMPLE_LOG(" "); // Formatting
}
