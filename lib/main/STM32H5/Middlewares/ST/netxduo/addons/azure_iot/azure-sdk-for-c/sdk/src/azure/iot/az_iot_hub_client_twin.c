// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include <azure/core/az_precondition.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>
#include <azure/iot/az_iot_hub_client.h>

#include <azure/core/_az_cfg.h>

static const uint8_t null_terminator = '\0';
static const uint8_t az_iot_hub_client_twin_question = '?';
static const uint8_t az_iot_hub_client_twin_equals = '=';
static const az_span az_iot_hub_client_request_id_span = AZ_SPAN_LITERAL_FROM_STR("$rid");
static const az_span az_iot_hub_twin_topic_prefix = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/");
static const az_span az_iot_hub_twin_response_sub_topic = AZ_SPAN_LITERAL_FROM_STR("res/");
static const az_span az_iot_hub_twin_get_pub_topic = AZ_SPAN_LITERAL_FROM_STR("GET/");
static const az_span az_iot_hub_twin_version_prop = AZ_SPAN_LITERAL_FROM_STR("$version");
static const az_span az_iot_hub_twin_patch_pub_topic
    = AZ_SPAN_LITERAL_FROM_STR("PATCH/properties/reported/");
static const az_span az_iot_hub_twin_patch_sub_topic
    = AZ_SPAN_LITERAL_FROM_STR("PATCH/properties/desired/");

AZ_NODISCARD az_result az_iot_hub_client_twin_document_get_publish_topic(
    az_iot_hub_client const* client,
    az_span request_id,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.iot_hub_hostname, 1, false);
  _az_PRECONDITION_VALID_SPAN(request_id, 1, false);
  _az_PRECONDITION_NOT_NULL(mqtt_topic);
  _az_PRECONDITION(mqtt_topic_size > 0);
  (void)client;

  az_span mqtt_topic_span = az_span_create((uint8_t*)mqtt_topic, (int32_t)mqtt_topic_size);
  int32_t required_length = az_span_size(az_iot_hub_twin_topic_prefix)
      + az_span_size(az_iot_hub_twin_get_pub_topic)
      + (int32_t)sizeof(az_iot_hub_client_twin_question)
      + az_span_size(az_iot_hub_client_request_id_span)
      + (int32_t)sizeof(az_iot_hub_client_twin_equals) + az_span_size(request_id);

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_topic_span, required_length + (int32_t)sizeof(null_terminator));

  az_span remainder = az_span_copy(mqtt_topic_span, az_iot_hub_twin_topic_prefix);
  remainder = az_span_copy(remainder, az_iot_hub_twin_get_pub_topic);
  remainder = az_span_copy_u8(remainder, az_iot_hub_client_twin_question);
  remainder = az_span_copy(remainder, az_iot_hub_client_request_id_span);
  remainder = az_span_copy_u8(remainder, az_iot_hub_client_twin_equals);
  remainder = az_span_copy(remainder, request_id);
  az_span_copy_u8(remainder, null_terminator);

  if (out_mqtt_topic_length)
  {
    *out_mqtt_topic_length = (size_t)required_length;
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_hub_client_twin_patch_get_publish_topic(
    az_iot_hub_client const* client,
    az_span request_id,
    char* mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.iot_hub_hostname, 1, false);
  _az_PRECONDITION_VALID_SPAN(request_id, 1, false);
  _az_PRECONDITION_NOT_NULL(mqtt_topic);
  _az_PRECONDITION(mqtt_topic_size > 0);
  (void)client;

  az_span mqtt_topic_span = az_span_create((uint8_t*)mqtt_topic, (int32_t)mqtt_topic_size);
  int32_t required_length = az_span_size(az_iot_hub_twin_topic_prefix)
      + az_span_size(az_iot_hub_twin_patch_pub_topic)
      + (int32_t)sizeof(az_iot_hub_client_twin_question)
      + az_span_size(az_iot_hub_client_request_id_span)
      + (int32_t)sizeof(az_iot_hub_client_twin_equals) + az_span_size(request_id);

  _az_RETURN_IF_NOT_ENOUGH_SIZE(
      mqtt_topic_span, required_length + (int32_t)sizeof(null_terminator));

  az_span remainder = az_span_copy(mqtt_topic_span, az_iot_hub_twin_topic_prefix);
  remainder = az_span_copy(remainder, az_iot_hub_twin_patch_pub_topic);
  remainder = az_span_copy_u8(remainder, az_iot_hub_client_twin_question);
  remainder = az_span_copy(remainder, az_iot_hub_client_request_id_span);
  remainder = az_span_copy_u8(remainder, az_iot_hub_client_twin_equals);
  remainder = az_span_copy(remainder, request_id);
  az_span_copy_u8(remainder, null_terminator);

  if (out_mqtt_topic_length)
  {
    *out_mqtt_topic_length = (size_t)required_length;
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_hub_client_twin_parse_received_topic(
    az_iot_hub_client const* client,
    az_span received_topic,
    az_iot_hub_client_twin_response* out_response)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_VALID_SPAN(client->_internal.iot_hub_hostname, 1, false);
  _az_PRECONDITION_VALID_SPAN(received_topic, 1, false);
  _az_PRECONDITION_NOT_NULL(out_response);
  (void)client;

  az_result result = AZ_OK;

  int32_t twin_index = az_span_find(received_topic, az_iot_hub_twin_topic_prefix);
  // Check if is related to twin or not
  if (twin_index >= 0)
  {
    _az_LOG_WRITE(AZ_LOG_MQTT_RECEIVED_TOPIC, received_topic);

    int32_t twin_feature_index = -1;
    az_span twin_feature_span
        = az_span_slice(received_topic, twin_index, az_span_size(received_topic));

    if ((twin_feature_index = az_span_find(twin_feature_span, az_iot_hub_twin_response_sub_topic))
        >= 0)
    {
      // Is a res case
      int32_t index = 0;
      az_span remainder;
      az_span status_str = _az_span_token(
          az_span_slice(
              received_topic,
              twin_feature_index + az_span_size(az_iot_hub_twin_response_sub_topic),
              az_span_size(received_topic)),
          AZ_SPAN_FROM_STR("/"),
          &remainder,
          &index);

      // Get status and convert to enum
      uint32_t status_int = 0;
      _az_RETURN_IF_FAILED(az_span_atou32(status_str, &status_int));
      out_response->status = (az_iot_status)status_int;

      if (index == -1)
      {
        return AZ_ERROR_UNEXPECTED_END;
      }

      // Get request id prop value
      az_iot_message_properties props;
      az_span prop_span = az_span_slice(remainder, 1, az_span_size(remainder));
      _az_RETURN_IF_FAILED(
          az_iot_message_properties_init(&props, prop_span, az_span_size(prop_span)));
      _az_RETURN_IF_FAILED(az_iot_message_properties_find(
          &props, az_iot_hub_client_request_id_span, &out_response->request_id));

      if (out_response->status >= AZ_IOT_STATUS_BAD_REQUEST) // 400+
      {
        // Is an error response
        out_response->response_type = AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REQUEST_ERROR;
        out_response->version = AZ_SPAN_EMPTY;
      }
      else if (out_response->status == AZ_IOT_STATUS_NO_CONTENT) // 204
      {
        // Is a reported prop response
        out_response->response_type = AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES;

        result = az_iot_message_properties_find(
            &props, az_iot_hub_twin_version_prop, &out_response->version);
        if (result == AZ_ERROR_ITEM_NOT_FOUND)
        {
          out_response->version = AZ_SPAN_EMPTY;
        }
        else
        {
          _az_RETURN_IF_FAILED(result);
        }
      }
      else // 200 or 202
      {
        // Is a twin GET response
        out_response->response_type = AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_GET;
        out_response->version = AZ_SPAN_EMPTY;
      }

      result = AZ_OK;
    }
    else if (
        (twin_feature_index = az_span_find(twin_feature_span, az_iot_hub_twin_patch_sub_topic))
        >= 0)
    {
      // Is a /PATCH case (desired props)
      az_iot_message_properties props;
      az_span prop_span = az_span_slice(
          received_topic,
          twin_feature_index + az_span_size(az_iot_hub_twin_patch_sub_topic)
              + (int32_t)sizeof(az_iot_hub_client_twin_question),
          az_span_size(received_topic));
      _az_RETURN_IF_FAILED(
          az_iot_message_properties_init(&props, prop_span, az_span_size(prop_span)));
      _az_RETURN_IF_FAILED(az_iot_message_properties_find(
          &props, az_iot_hub_twin_version_prop, &out_response->version));

      out_response->response_type = AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES;
      out_response->request_id = AZ_SPAN_EMPTY;
      out_response->status = AZ_IOT_STATUS_OK;

      result = AZ_OK;
    }
    else
    {
      result = AZ_ERROR_IOT_TOPIC_NO_MATCH;
    }
  }
  else
  {
    result = AZ_ERROR_IOT_TOPIC_NO_MATCH;
  }

  return result;
}
