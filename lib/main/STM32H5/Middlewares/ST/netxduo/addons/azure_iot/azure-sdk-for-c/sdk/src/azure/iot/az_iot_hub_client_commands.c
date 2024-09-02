// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include <azure/core/az_precondition.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/iot/az_iot_hub_client.h>

#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <azure/core/_az_cfg.h>

static const az_span command_separator = AZ_SPAN_LITERAL_FROM_STR("*");

AZ_NODISCARD az_result az_iot_hub_client_commands_response_get_publish_topic(
    az_iot_hub_client const* client,
    az_span request_id,
    uint16_t status,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  return az_iot_hub_client_methods_response_get_publish_topic(
      client, request_id, status, mqtt_topic, mqtt_topic_size, out_mqtt_topic_length);
}

AZ_NODISCARD az_result az_iot_hub_client_commands_parse_received_topic(
    az_iot_hub_client const* client,
    az_span received_topic,
    az_iot_hub_client_command_request* out_request)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(received_topic, 1, false);
  _az_PRECONDITION_NOT_NULL(out_request);

  az_iot_hub_client_method_request method_request;

  _az_RETURN_IF_FAILED(
      az_iot_hub_client_methods_parse_received_topic(client, received_topic, &method_request));

  out_request->request_id = method_request.request_id;

  int32_t command_separator_index = az_span_find(method_request.name, command_separator);
  if (command_separator_index > 0)
  {
    out_request->component_name = az_span_slice(method_request.name, 0, command_separator_index);
    out_request->command_name = az_span_slice(
        method_request.name, command_separator_index + 1, az_span_size(method_request.name));
  }
  else
  {
    out_request->component_name = AZ_SPAN_EMPTY;
    out_request->command_name
        = az_span_slice(method_request.name, 0, az_span_size(method_request.name));
  }

  return AZ_OK;
}
