// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_json.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>
#include <azure/iot/az_iot_common.h>
#include <azure/iot/az_iot_provisioning_client.h>

#include <azure/core/_az_cfg.h>

static const az_span str_put_iotdps_register
    = AZ_SPAN_LITERAL_FROM_STR("PUT/iotdps-register/?$rid=1");
static const az_span str_get_iotdps_get_operationstatus
    = AZ_SPAN_LITERAL_FROM_STR("GET/iotdps-get-operationstatus/?$rid=1&operationId=");

// From the protocol described in
// https://docs.microsoft.com/azure/iot-dps/iot-dps-mqtt-support#registering-a-device
static const az_span prov_registration_id_label = AZ_SPAN_LITERAL_FROM_STR("registrationId");
static const az_span prov_payload_label = AZ_SPAN_LITERAL_FROM_STR("payload");

// $dps/registrations/res/
AZ_INLINE az_span _az_iot_provisioning_get_dps_registrations_res()
{
  az_span sub_topic = AZ_SPAN_LITERAL_FROM_STR(AZ_IOT_PROVISIONING_CLIENT_REGISTER_SUBSCRIBE_TOPIC);

  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  return az_span_slice(sub_topic, 0, 23);
}

// /registrations/
AZ_INLINE az_span _az_iot_provisioning_get_str_registrations()
{
  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  return az_span_slice(_az_iot_provisioning_get_dps_registrations_res(), 4, 19);
}

// $dps/registrations/
AZ_INLINE az_span _az_iot_provisioning_get_str_dps_registrations()
{
  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  return az_span_slice(_az_iot_provisioning_get_dps_registrations_res(), 0, 19);
}

AZ_NODISCARD az_iot_provisioning_client_options az_iot_provisioning_client_options_default()
{
  return (az_iot_provisioning_client_options){ .user_agent = AZ_SPAN_EMPTY };
}

AZ_NODISCARD az_result az_iot_provisioning_client_init(
    az_iot_provisioning_client* client,
    az_span global_device_hostname,
    az_span id_scope,
    az_span registration_id,
    az_iot_provisioning_client_options const* options)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(global_device_hostname, 1, false);
  _az_PRECONDITION_VALID_SPAN(id_scope, 1, false);
  _az_PRECONDITION_VALID_SPAN(registration_id, 1, false);

  client->_internal.global_device_endpoint = global_device_hostname;
  client->_internal.id_scope = id_scope;
  client->_internal.registration_id = registration_id;

  client->_internal.options
      = options == NULL ? az_iot_provisioning_client_options_default() : *options;

  return AZ_OK;
}

// <id_scope>/registrations/<registration_id>/api-version=<service_version>
AZ_NODISCARD az_result az_iot_provisioning_client_get_user_name(
    az_iot_provisioning_client const* client,
    char* mqtt_user_name,
    size_t mqtt_user_name_size,
    size_t* out_mqtt_user_name_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(mqtt_user_name);
  _az_PRECONDITION(mqtt_user_name_size > 0);

  az_span provisioning_service_api_version
      = AZ_SPAN_LITERAL_FROM_STR("/api-version=" AZ_IOT_PROVISIONING_SERVICE_VERSION);
  az_span user_agent_version_prefix = AZ_SPAN_LITERAL_FROM_STR("&ClientVersion=");

  az_span mqtt_user_name_span
      = az_span_create((uint8_t*)mqtt_user_name, (int32_t)mqtt_user_name_size);

  const az_span* const user_agent = &(client->_internal.options.user_agent);
  az_span str_registrations = _az_iot_provisioning_get_str_registrations();

  int32_t required_length = az_span_size(client->_internal.id_scope)
      + az_span_size(str_registrations) + az_span_size(client->_internal.registration_id)
      + az_span_size(provisioning_service_api_version);
  if (az_span_size(*user_agent) > 0)
  {
    required_length += az_span_size(user_agent_version_prefix) + az_span_size(*user_agent);
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_user_name_span, required_length + (int32_t)sizeof((uint8_t)'\0'));

  az_span remainder = az_span_copy(mqtt_user_name_span, client->_internal.id_scope);
  remainder = az_span_copy(remainder, str_registrations);
  remainder = az_span_copy(remainder, client->_internal.registration_id);
  remainder = az_span_copy(remainder, provisioning_service_api_version);

  if (az_span_size(*user_agent) > 0)
  {
    remainder = az_span_copy(remainder, user_agent_version_prefix);
    remainder = az_span_copy(remainder, *user_agent);
  }

  az_span_copy_u8(remainder, '\0');

  if (out_mqtt_user_name_length)
  {
    *out_mqtt_user_name_length = (size_t)required_length;
  }

  return AZ_OK;
}

// <registration_id>
AZ_NODISCARD az_result az_iot_provisioning_client_get_client_id(
    az_iot_provisioning_client const* client,
    char* mqtt_client_id,
    size_t mqtt_client_id_size,
    size_t* out_mqtt_client_id_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(mqtt_client_id);
  _az_PRECONDITION(mqtt_client_id_size > 0);

  az_span mqtt_client_id_span
      = az_span_create((uint8_t*)mqtt_client_id, (int32_t)mqtt_client_id_size);

  int32_t required_length = az_span_size(client->_internal.registration_id);

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_client_id_span, required_length + (int32_t)sizeof((uint8_t)'\0'));

  az_span remainder = az_span_copy(mqtt_client_id_span, client->_internal.registration_id);
  az_span_copy_u8(remainder, '\0');

  if (out_mqtt_client_id_length)
  {
    *out_mqtt_client_id_length = (size_t)required_length;
  }

  return AZ_OK;
}

// $dps/registrations/PUT/iotdps-register/?$rid=%s
AZ_NODISCARD az_result az_iot_provisioning_client_register_get_publish_topic(
    az_iot_provisioning_client const* client,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  (void)client;

  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.global_device_endpoint, 1, false);
  _az_PRECONDITION_NOT_NULL(mqtt_topic);
  _az_PRECONDITION(mqtt_topic_size > 0);

  az_span mqtt_topic_span = az_span_create((uint8_t*)mqtt_topic, (int32_t)mqtt_topic_size);
  az_span str_dps_registrations = _az_iot_provisioning_get_str_dps_registrations();

  int32_t required_length
      = az_span_size(str_dps_registrations) + az_span_size(str_put_iotdps_register);

  _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_topic_span, required_length + (int32_t)sizeof((uint8_t)'\0'));

  az_span remainder = az_span_copy(mqtt_topic_span, str_dps_registrations);
  remainder = az_span_copy(remainder, str_put_iotdps_register);
  az_span_copy_u8(remainder, '\0');

  if (out_mqtt_topic_length)
  {
    *out_mqtt_topic_length = (size_t)required_length;
  }

  return AZ_OK;
}

// Topic: $dps/registrations/GET/iotdps-get-operationstatus/?$rid=%s&operationId=%s
AZ_NODISCARD az_result az_iot_provisioning_client_query_status_get_publish_topic(
    az_iot_provisioning_client const* client,
    az_span operation_id,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  (void)client;

  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.global_device_endpoint, 1, false);
  _az_PRECONDITION_NOT_NULL(mqtt_topic);
  _az_PRECONDITION(mqtt_topic_size > 0);

  _az_PRECONDITION_VALID_SPAN(operation_id, 1, false);

  az_span mqtt_topic_span = az_span_create((uint8_t*)mqtt_topic, (int32_t)mqtt_topic_size);
  az_span str_dps_registrations = _az_iot_provisioning_get_str_dps_registrations();

  int32_t required_length = az_span_size(str_dps_registrations)
      + az_span_size(str_get_iotdps_get_operationstatus) + az_span_size(operation_id);

  _az_RETURN_IF_NOT_ENOUGH_SIZE(mqtt_topic_span, required_length + (int32_t)sizeof((uint8_t)'\0'));

  az_span remainder = az_span_copy(mqtt_topic_span, str_dps_registrations);
  remainder = az_span_copy(remainder, str_get_iotdps_get_operationstatus);
  remainder = az_span_copy(remainder, operation_id);
  az_span_copy_u8(remainder, '\0');

  if (out_mqtt_topic_length)
  {
    *out_mqtt_topic_length = (size_t)required_length;
  }

  return AZ_OK;
}

AZ_INLINE az_iot_provisioning_client_registration_state
_az_iot_provisioning_registration_state_default()
{
  return (az_iot_provisioning_client_registration_state){ .assigned_hub_hostname = AZ_SPAN_EMPTY,
                                                          .device_id = AZ_SPAN_EMPTY,
                                                          .error_code = AZ_IOT_STATUS_UNKNOWN,
                                                          .extended_error_code = 0,
                                                          .error_message = AZ_SPAN_EMPTY,
                                                          .error_tracking_id = AZ_SPAN_EMPTY,
                                                          .error_timestamp = AZ_SPAN_EMPTY };
}

AZ_INLINE az_iot_status _az_iot_status_from_extended_status(uint32_t extended_status)
{
  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  return (az_iot_status)(extended_status / 1000);
}

/*
Documented at
https://docs.microsoft.com/rest/api/iot-dps/device/runtime-registration/register-device#deviceregistrationresult
  "registrationState":{
    "x509":{},
    "registrationId":"paho-sample-device1",
    "createdDateTimeUtc":"2020-04-10T03:11:13.0276997Z",
    "assignedHub":"contoso.azure-devices.net",
    "deviceId":"paho-sample-device1",
    "status":"assigned",
    "substatus":"initialAssignment",
    "lastUpdatedDateTimeUtc":"2020-04-10T03:11:13.2096201Z",
    "etag":"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI="}}
*/
AZ_INLINE az_result _az_iot_provisioning_client_parse_payload_error_code(
    az_json_reader* jr,
    az_iot_provisioning_client_registration_state* out_state)
{
  if (az_json_token_is_text_equal(&jr->token, AZ_SPAN_FROM_STR("errorCode")))
  {
    _az_RETURN_IF_FAILED(az_json_reader_next_token(jr));
    _az_RETURN_IF_FAILED(az_json_token_get_uint32(&jr->token, &out_state->extended_error_code));
    out_state->error_code = _az_iot_status_from_extended_status(out_state->extended_error_code);

    return AZ_OK;
  }

  return AZ_ERROR_ITEM_NOT_FOUND;
}

AZ_INLINE az_result _az_iot_provisioning_client_payload_registration_state_parse(
    az_json_reader* jr,
    az_iot_provisioning_client_registration_state* out_state)
{
  if (jr->token.kind != AZ_JSON_TOKEN_BEGIN_OBJECT)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  bool found_assigned_hub = false;
  bool found_device_id = false;

  while ((!(found_device_id && found_assigned_hub))
         && az_result_succeeded(az_json_reader_next_token(jr))
         && jr->token.kind != AZ_JSON_TOKEN_END_OBJECT)
  {
    if (az_json_token_is_text_equal(&jr->token, AZ_SPAN_FROM_STR("assignedHub")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(jr));
      if (jr->token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_state->assigned_hub_hostname = jr->token.slice;
      found_assigned_hub = true;
    }
    else if (az_json_token_is_text_equal(&jr->token, AZ_SPAN_FROM_STR("deviceId")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(jr));
      if (jr->token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_state->device_id = jr->token.slice;
      found_device_id = true;
    }
    else if (az_json_token_is_text_equal(&jr->token, AZ_SPAN_FROM_STR("errorMessage")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(jr));
      if (jr->token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_state->error_message = jr->token.slice;
    }
    else if (az_json_token_is_text_equal(&jr->token, AZ_SPAN_FROM_STR("lastUpdatedDateTimeUtc")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(jr));
      if (jr->token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_state->error_timestamp = jr->token.slice;
    }
    else if (az_result_succeeded(
                 _az_iot_provisioning_client_parse_payload_error_code(jr, out_state)))
    {
      // Do nothing
    }
    else
    {
      // ignore other tokens
      _az_RETURN_IF_FAILED(az_json_reader_skip_children(jr));
    }
  }

  if (found_assigned_hub != found_device_id)
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  return AZ_OK;
}

AZ_NODISCARD static az_result _az_iot_provisioning_client_parse_operation_status(
    az_span response_operation_status,
    az_iot_provisioning_client_operation_status* out_operation_status)
{
  _az_PRECONDITION_VALID_SPAN(response_operation_status, 0, false);
  _az_PRECONDITION_NOT_NULL(out_operation_status);

  if (az_span_is_content_equal(response_operation_status, AZ_SPAN_FROM_STR("assigning")))
  {
    *out_operation_status = AZ_IOT_PROVISIONING_STATUS_ASSIGNING;
  }
  else if (az_span_is_content_equal(response_operation_status, AZ_SPAN_FROM_STR("assigned")))
  {
    *out_operation_status = AZ_IOT_PROVISIONING_STATUS_ASSIGNED;
  }
  else if (az_span_is_content_equal(response_operation_status, AZ_SPAN_FROM_STR("failed")))
  {
    *out_operation_status = AZ_IOT_PROVISIONING_STATUS_FAILED;
  }
  else if (az_span_is_content_equal(response_operation_status, AZ_SPAN_FROM_STR("unassigned")))
  {
    *out_operation_status = AZ_IOT_PROVISIONING_STATUS_UNASSIGNED;
  }
  else if (az_span_is_content_equal(response_operation_status, AZ_SPAN_FROM_STR("disabled")))
  {
    *out_operation_status = AZ_IOT_PROVISIONING_STATUS_DISABLED;
  }
  else
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  return AZ_OK;
}

AZ_INLINE az_result az_iot_provisioning_client_parse_payload(
    az_span received_payload,
    az_iot_provisioning_client_register_response* out_response)
{
  // Parse the payload:
  az_json_reader jr;
  _az_RETURN_IF_FAILED(az_json_reader_init(&jr, received_payload, NULL));

  _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
  if (jr.token.kind != AZ_JSON_TOKEN_BEGIN_OBJECT)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  out_response->registration_state = _az_iot_provisioning_registration_state_default();

  bool found_operation_id = false;
  bool found_operation_status = false;
  bool found_error = false;

  while (az_result_succeeded(az_json_reader_next_token(&jr))
         && jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
  {
    if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("operationId")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_response->operation_id = jr.token.slice;
      found_operation_id = true;
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("status")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      _az_RETURN_IF_FAILED(_az_iot_provisioning_client_parse_operation_status(
          jr.token.slice, &out_response->operation_status));

      found_operation_status = true;
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("registrationState")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      _az_RETURN_IF_FAILED(_az_iot_provisioning_client_payload_registration_state_parse(
          &jr, &out_response->registration_state));
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("trackingId")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_response->registration_state.error_tracking_id = jr.token.slice;
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("message")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_response->registration_state.error_message = jr.token.slice;
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("timestampUtc")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      out_response->registration_state.error_timestamp = jr.token.slice;
    }
    else if (az_result_succeeded(_az_iot_provisioning_client_parse_payload_error_code(
                 &jr, &out_response->registration_state)))
    {
      found_error = true;
    }
    else
    {
      // ignore other tokens
      _az_RETURN_IF_FAILED(az_json_reader_skip_children(&jr));
    }
  }

  if (!(found_operation_status && found_operation_id))
  {
    out_response->operation_id = AZ_SPAN_EMPTY;
    out_response->operation_status = AZ_IOT_PROVISIONING_STATUS_FAILED;

    if (!found_error)
    {
      return AZ_ERROR_ITEM_NOT_FOUND;
    }
  }

  return AZ_OK;
}

/*
Example flow:

Stage 1:
 topic: $dps/registrations/res/202/?$rid=1&retry-after=3
 payload:
  {"operationId":"4.d0a671905ea5b2c8.e7173b7b-0e54-4aa0-9d20-aeb1b89e6c7d","status":"assigning"}

Stage 2:
  {"operationId":"4.d0a671905ea5b2c8.e7173b7b-0e54-4aa0-9d20-aeb1b89e6c7d","status":"assigning",
  "registrationState":{"registrationId":"paho-sample-device1","status":"assigning"}}

Stage 3:
 topic: $dps/registrations/res/200/?$rid=1
 payload:
  {"operationId":"4.d0a671905ea5b2c8.e7173b7b-0e54-4aa0-9d20-aeb1b89e6c7d","status":"assigned",
  "registrationState":{ ... }}

 Error:
 topic: $dps/registrations/res/401/?$rid=1
 payload:
 {"errorCode":401002,"trackingId":"8ad0463c-6427-4479-9dfa-3e8bb7003e9b","message":"Invalid
  certificate.","timestampUtc":"2020-04-10T05:24:22.4718526Z"}
*/
AZ_NODISCARD az_result az_iot_provisioning_client_parse_received_topic_and_payload(
    az_iot_provisioning_client const* client,
    az_span received_topic,
    az_span received_payload,
    az_iot_provisioning_client_register_response* out_response)
{
  (void)client;

  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.global_device_endpoint, 1, false);
  _az_PRECONDITION_VALID_SPAN(received_topic, 1, false);
  _az_PRECONDITION_VALID_SPAN(received_payload, 1, false);
  _az_PRECONDITION_NOT_NULL(out_response);

  az_span str_dps_registrations_res = _az_iot_provisioning_get_dps_registrations_res();
  int32_t idx = az_span_find(received_topic, str_dps_registrations_res);
  if (idx != 0)
  {
    return AZ_ERROR_IOT_TOPIC_NO_MATCH;
  }

  _az_LOG_WRITE(AZ_LOG_MQTT_RECEIVED_TOPIC, received_topic);
  _az_LOG_WRITE(AZ_LOG_MQTT_RECEIVED_PAYLOAD, received_payload);

  // Parse the status.
  az_span remainder = az_span_slice_to_end(received_topic, az_span_size(str_dps_registrations_res));

  int32_t index = 0;
  az_span int_slice = _az_span_token(remainder, AZ_SPAN_FROM_STR("/"), &remainder, &index);
  _az_RETURN_IF_FAILED(az_span_atou32(int_slice, (uint32_t*)(&out_response->status)));

  // Parse the optional retry-after= field.
  az_span retry_after = AZ_SPAN_FROM_STR("retry-after=");
  idx = az_span_find(remainder, retry_after);
  if (idx != -1)
  {
    remainder = az_span_slice_to_end(remainder, idx + az_span_size(retry_after));
    int_slice = _az_span_token(remainder, AZ_SPAN_FROM_STR("&"), &remainder, &index);

    _az_RETURN_IF_FAILED(az_span_atou32(int_slice, &out_response->retry_after_seconds));
  }
  else
  {
    out_response->retry_after_seconds = 0;
  }

  _az_RETURN_IF_FAILED(az_iot_provisioning_client_parse_payload(received_payload, out_response));

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_provisioning_client_get_request_payload(
    az_iot_provisioning_client const* client,
    az_span custom_payload_property,
    az_iot_provisioning_client_payload_options const* options,
    uint8_t* mqtt_payload,
    size_t mqtt_payload_size,
    size_t* out_mqtt_payload_length)
{
  (void)options;

  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_IS_NULL(options);
  _az_PRECONDITION_NOT_NULL(mqtt_payload);
  _az_PRECONDITION(mqtt_payload_size > 0);
  _az_PRECONDITION_NOT_NULL(out_mqtt_payload_length);

  az_json_writer json_writer;
  az_span payload_buffer = az_span_create(mqtt_payload, (int32_t)mqtt_payload_size);

  _az_RETURN_IF_FAILED(az_json_writer_init(&json_writer, payload_buffer, NULL));
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(&json_writer));
  _az_RETURN_IF_FAILED(
      az_json_writer_append_property_name(&json_writer, prov_registration_id_label));
  _az_RETURN_IF_FAILED(
      az_json_writer_append_string(&json_writer, client->_internal.registration_id));

  if (az_span_size(custom_payload_property) > 0)
  {
    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(&json_writer, prov_payload_label));
    _az_RETURN_IF_FAILED(az_json_writer_append_json_text(&json_writer, custom_payload_property));
  }

  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(&json_writer));
  *out_mqtt_payload_length
      = (size_t)az_span_size(az_json_writer_get_bytes_used_in_destination(&json_writer));
  ;

  return AZ_OK;
}
