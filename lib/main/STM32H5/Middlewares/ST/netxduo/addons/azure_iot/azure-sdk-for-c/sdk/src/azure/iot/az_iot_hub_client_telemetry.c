// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_precondition.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/iot/az_iot_hub_client.h>

#include <stdint.h>

#include <azure/core/_az_cfg.h>

static const uint8_t null_terminator = '\0';
static const az_span telemetry_topic_prefix = AZ_SPAN_LITERAL_FROM_STR("devices/");
static const az_span telemetry_topic_modules_mid = AZ_SPAN_LITERAL_FROM_STR("/modules/");
static const az_span telemetry_topic_suffix = AZ_SPAN_LITERAL_FROM_STR("/messages/events/");

AZ_NODISCARD az_result az_iot_hub_client_telemetry_get_publish_topic(
    az_iot_hub_client const* client,
    az_iot_message_properties const* properties,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(mqtt_topic);
  _az_PRECONDITION(mqtt_topic_size > 0);

  const az_span* const module_id = &(client->_internal.options.module_id);

  az_span mqtt_topic_span = az_span_create((uint8_t*)mqtt_topic, (int32_t)mqtt_topic_size);
  int32_t required_length = az_span_size(telemetry_topic_prefix)
      + az_span_size(client->_internal.device_id) + az_span_size(telemetry_topic_suffix);
  int32_t module_id_length = az_span_size(*module_id);
  if (module_id_length > 0)
  {
    required_length += az_span_size(telemetry_topic_modules_mid) + module_id_length;
  }
  if (properties != NULL)
  {
    required_length += properties->_internal.properties_written;
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_topic_span, required_length + (int32_t)sizeof(null_terminator));

  az_span remainder = az_span_copy(mqtt_topic_span, telemetry_topic_prefix);
  remainder = az_span_copy(remainder, client->_internal.device_id);

  if (module_id_length > 0)
  {
    remainder = az_span_copy(remainder, telemetry_topic_modules_mid);
    remainder = az_span_copy(remainder, *module_id);
  }

  remainder = az_span_copy(remainder, telemetry_topic_suffix);

  if (properties != NULL)
  {
    remainder = az_span_copy(
        remainder,
        az_span_slice(
            properties->_internal.properties_buffer, 0, properties->_internal.properties_written));
  }

  az_span_copy_u8(remainder, null_terminator);

  if (out_mqtt_topic_length)
  {
    *out_mqtt_topic_length = (size_t)required_length;
  }

  return AZ_OK;
}
