// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/az_version.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>
#include <azure/iot/az_iot_hub_client.h>
#include <azure/iot/internal/az_iot_common_internal.h>

#include <azure/core/_az_cfg.h>

static const uint8_t null_terminator = '\0';
static const uint8_t hub_client_forward_slash = '/';
static const az_span hub_client_param_separator_span = AZ_SPAN_LITERAL_FROM_STR("&");
static const az_span hub_client_param_equals_span = AZ_SPAN_LITERAL_FROM_STR("=");

static const az_span hub_digital_twin_model_id = AZ_SPAN_LITERAL_FROM_STR("model-id");
static const az_span hub_service_api_version = AZ_SPAN_LITERAL_FROM_STR("/?api-version=2020-09-30");
static const az_span client_sdk_device_client_type_name
    = AZ_SPAN_LITERAL_FROM_STR("DeviceClientType");
static const az_span client_sdk_version_default_value
    = AZ_SPAN_LITERAL_FROM_STR("azsdk-c%2F" AZ_SDK_VERSION_STRING);

AZ_NODISCARD az_iot_hub_client_options az_iot_hub_client_options_default()
{
  return (az_iot_hub_client_options){ .module_id = AZ_SPAN_EMPTY,
                                      .user_agent = client_sdk_version_default_value,
                                      .model_id = AZ_SPAN_EMPTY,
                                      .component_names = NULL,
                                      .component_names_length = 0 };
}

AZ_NODISCARD az_result az_iot_hub_client_init(
    az_iot_hub_client* client,
    az_span iot_hub_hostname,
    az_span device_id,
    az_iot_hub_client_options const* options)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(iot_hub_hostname, 1, false);
  _az_PRECONDITION_VALID_SPAN(device_id, 1, false);

  client->_internal.iot_hub_hostname = iot_hub_hostname;
  client->_internal.device_id = device_id;
  client->_internal.options = options == NULL ? az_iot_hub_client_options_default() : *options;

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_hub_client_get_user_name(
    az_iot_hub_client const* client,
    char* mqtt_user_name,
    size_t mqtt_user_name_size,
    size_t* out_mqtt_user_name_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(mqtt_user_name);
  _az_PRECONDITION(mqtt_user_name_size > 0);

  const az_span* const module_id = &(client->_internal.options.module_id);
  const az_span* const user_agent = &(client->_internal.options.user_agent);
  const az_span* const model_id = &(client->_internal.options.model_id);

  az_span mqtt_user_name_span
      = az_span_create((uint8_t*)mqtt_user_name, (int32_t)mqtt_user_name_size);

  int32_t required_length = az_span_size(client->_internal.iot_hub_hostname)
      + az_span_size(client->_internal.device_id) + (int32_t)sizeof(hub_client_forward_slash)
      + az_span_size(hub_service_api_version);
  if (az_span_size(*module_id) > 0)
  {
    required_length += az_span_size(*module_id) + (int32_t)sizeof(hub_client_forward_slash);
  }
  if (az_span_size(*user_agent) > 0)
  {
    required_length += az_span_size(hub_client_param_separator_span)
        + az_span_size(client_sdk_device_client_type_name)
        + az_span_size(hub_client_param_equals_span) + az_span_size(*user_agent);
  }
  // Note we skip the length of the model id since we have to url encode it. Bound checking is done
  // later.
  if (az_span_size(*model_id) > 0)
  {
    required_length += az_span_size(hub_client_param_separator_span)
        + az_span_size(hub_client_param_equals_span);
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_user_name_span, required_length + (int32_t)sizeof(null_terminator));

  az_span remainder = az_span_copy(mqtt_user_name_span, client->_internal.iot_hub_hostname);
  remainder = az_span_copy_u8(remainder, hub_client_forward_slash);
  remainder = az_span_copy(remainder, client->_internal.device_id);

  if (az_span_size(*module_id) > 0)
  {
    remainder = az_span_copy_u8(remainder, hub_client_forward_slash);
    remainder = az_span_copy(remainder, *module_id);
  }

  remainder = az_span_copy(remainder, hub_service_api_version);

  if (az_span_size(*user_agent) > 0)
  {
    remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_separator_span));
    remainder = az_span_copy(remainder, client_sdk_device_client_type_name);
    remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_equals_span));
    remainder = az_span_copy(remainder, *user_agent);
  }

  if (az_span_size(*model_id) > 0)
  {
    remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_separator_span));
    remainder = az_span_copy(remainder, hub_digital_twin_model_id);
    remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_equals_span));

    _az_RETURN_IF_FAILED(_az_span_copy_url_encode(remainder, *model_id, &remainder));
  }
  if (az_span_size(remainder) > 0)
  {
    remainder = az_span_copy_u8(remainder, null_terminator);
  }
  else
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  if (out_mqtt_user_name_length)
  {
    *out_mqtt_user_name_length
        = mqtt_user_name_size - (size_t)az_span_size(remainder) - sizeof(null_terminator);
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_hub_client_get_client_id(
    az_iot_hub_client const* client,
    char* mqtt_client_id,
    size_t mqtt_client_id_size,
    size_t* out_mqtt_client_id_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(mqtt_client_id);
  _az_PRECONDITION(mqtt_client_id_size > 0);

  az_span mqtt_client_id_span
      = az_span_create((uint8_t*)mqtt_client_id, (int32_t)mqtt_client_id_size);
  const az_span* const module_id = &(client->_internal.options.module_id);

  int32_t required_length = az_span_size(client->_internal.device_id);
  if (az_span_size(*module_id) > 0)
  {
    required_length += az_span_size(*module_id) + (int32_t)sizeof(hub_client_forward_slash);
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_client_id_span, required_length + (int32_t)sizeof(null_terminator));

  az_span remainder = az_span_copy(mqtt_client_id_span, client->_internal.device_id);

  if (az_span_size(*module_id) > 0)
  {
    remainder = az_span_copy_u8(remainder, hub_client_forward_slash);
    remainder = az_span_copy(remainder, *module_id);
  }

  az_span_copy_u8(remainder, null_terminator);

  if (out_mqtt_client_id_length)
  {
    *out_mqtt_client_id_length = (size_t)required_length;
  }

  return AZ_OK;
}
