// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// DEPRECATED: This sample helper file has been deprecated.
// This file has *sample* helper functions for building and
// parsing IoT Plug and Play payloads.  Applications instead should use
// the officially supported APIs (see az_iot_hub_client.h and
// az_iot_hub_client_properties.h).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <azure/az_core.h>
#include <azure/az_iot.h>

#include <iot_sample_common.h>

#include "pnp_protocol.h"

// Property values
static char pnp_properties_buffer[64];

// Device twin values
static az_span const component_telemetry_prop_span = AZ_SPAN_LITERAL_FROM_STR("$.sub");
static az_span const desired_temp_response_value_name = AZ_SPAN_LITERAL_FROM_STR("value");
static az_span const desired_temp_ack_code_name = AZ_SPAN_LITERAL_FROM_STR("ac");
static az_span const desired_temp_ack_version_name = AZ_SPAN_LITERAL_FROM_STR("av");
static az_span const desired_temp_ack_description_name = AZ_SPAN_LITERAL_FROM_STR("ad");
static az_span const component_specifier_name = AZ_SPAN_LITERAL_FROM_STR("__t");
static az_span const component_specifier_value = AZ_SPAN_LITERAL_FROM_STR("c");
static az_span const command_separator = AZ_SPAN_LITERAL_FROM_STR("/");
static az_span const iot_hub_twin_desired_version = AZ_SPAN_LITERAL_FROM_STR("$version");
static az_span const iot_hub_twin_desired = AZ_SPAN_LITERAL_FROM_STR("desired");

// Visit each valid property for the component
static void visit_component_properties(
    az_span component_name,
    az_json_reader* jr,
    int32_t version,
    pnp_property_callback property_callback,
    void* context_ptr)
{
  char const* const log = "Failed to process device twin message";

  while (az_result_succeeded(az_json_reader_next_token(jr)))
  {
    if (jr->token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
    {
      if (az_json_token_is_text_equal(&(jr->token), component_specifier_name)
          || az_json_token_is_text_equal(&(jr->token), iot_hub_twin_desired_version))
      {
        IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(jr), log);
        continue;
      }

      // Found a property.
      az_json_token property_name = jr->token;
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(jr), log);

      property_callback(component_name, &property_name, *jr, version, context_ptr);
    }
    else if (jr->token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
    {
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_skip_children(jr), log);
    }
    else if (jr->token.kind == AZ_JSON_TOKEN_END_OBJECT)
    {
      break;
    }
  }
}

// Move reader to the value of property name.
static bool json_child_token_move(az_json_reader* jr, az_span property_name)
{
  char const* const log = "Failed to process device twin message";

  while (az_result_succeeded(az_json_reader_next_token(jr)))
  {
    if ((jr->token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
        && az_json_token_is_text_equal(&jr->token, property_name))
    {
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(jr), log);
      return true;
    }
    else if (jr->token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
    {
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_skip_children(jr), log);
    }
    else if (jr->token.kind == AZ_JSON_TOKEN_END_OBJECT)
    {
      return false;
    }
  }

  return false;
}

// Check if the component name is in the model
static bool is_component_in_model(
    az_span component_name,
    az_span const** components_ptr,
    int32_t components_num,
    int32_t* out_index)
{
  if (az_span_size(component_name) == 0)
  {
    return false;
  }

  int32_t index = 0;
  while (index < components_num)
  {
    if (az_span_is_content_equal(component_name, *components_ptr[index]))
    {
      *out_index = index;
      return true;
    }
    index++;
  }

  return false;
}

void pnp_telemetry_get_publish_topic(
    az_iot_hub_client const* client,
    az_iot_message_properties* properties,
    az_span component_name,
    char* out_mqtt_topic,
    size_t mqtt_topic_size,
    size_t* out_mqtt_topic_length)
{
  char const* const log = "Failed to get the Telemetry topic";

  az_iot_message_properties pnp_properties;

  if (az_span_size(component_name) != 0)
  {
    if (properties == NULL)
    {
      properties = &pnp_properties;

      IOT_SAMPLE_EXIT_IF_AZ_FAILED(
          az_iot_message_properties_init(properties, AZ_SPAN_FROM_BUFFER(pnp_properties_buffer), 0),
          log);
    }

    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_iot_message_properties_append(properties, component_telemetry_prop_span, component_name),
        log);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_iot_hub_client_telemetry_get_publish_topic(
          client,
          az_span_size(component_name) != 0 ? properties : NULL,
          out_mqtt_topic,
          mqtt_topic_size,
          out_mqtt_topic_length),
      log);
}

void pnp_parse_command_name(
    az_span component_command,
    az_span* out_component_name,
    az_span* out_command_name)
{
  int32_t index = az_span_find(component_command, command_separator);
  if (index > 0)
  {
    *out_component_name = az_span_slice(component_command, 0, index);
    *out_command_name
        = az_span_slice(component_command, index + 1, az_span_size(component_command));
  }
  else
  {
    *out_component_name = AZ_SPAN_EMPTY;
    *out_command_name = component_command;
  }
}

void pnp_build_reported_property(
    az_span json_buffer,
    az_span component_name,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* context,
    az_span* out_span)
{
  char const* const log = "Failed to build `%.*s` reported property";

  az_json_writer jw;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, json_buffer, NULL), log, property_name);

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log);

  if (az_span_size(component_name) != 0)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, component_name), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, component_specifier_name), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_string(&jw, component_specifier_value), log, property_name);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, property_name), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(append_callback(&jw, context), log, property_name);

  if (az_span_size(component_name) != 0)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log, property_name);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log, property_name);

  *out_span = az_json_writer_get_bytes_used_in_destination(&jw);
}

void pnp_build_reported_property_with_status(
    az_span json_buffer,
    az_span component_name,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* context,
    int32_t code,
    int32_t version,
    az_span description,
    az_span* out_span)
{
  char const* const log = "Failed to build `%.*s` reported property with status";

  az_json_writer jw;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, json_buffer, NULL), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log, property_name);

  if (az_span_size(component_name) != 0)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, component_name), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, component_specifier_name), log, property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_string(&jw, component_specifier_value), log, property_name);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, property_name), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, desired_temp_response_value_name),
      log,
      property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(append_callback(&jw, context), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, desired_temp_ack_code_name), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_int32(&jw, code), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(
      az_json_writer_append_property_name(&jw, desired_temp_ack_version_name), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_int32(&jw, version), log, property_name);

  if (az_span_size(description) != 0)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_property_name(&jw, desired_temp_ack_description_name),
        log,
        property_name);
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(
        az_json_writer_append_string(&jw, description), log, property_name);
  }

  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log, property_name);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log, property_name);

  if (az_span_size(component_name) != 0)
  {
    IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log, property_name);
  }

  *out_span = az_json_writer_get_bytes_used_in_destination(&jw);
}

void pnp_build_telemetry_message(
    az_span json_buffer,
    az_span property_name,
    pnp_append_property_callback append_callback,
    void* property_value,
    az_span* out_span)
{
  char const* const log = "Failed to build Telemetry message";

  az_json_writer jw;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_init(&jw, json_buffer, NULL), log);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_begin_object(&jw), log);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_property_name(&jw, property_name), log);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(append_callback(&jw, property_value), log);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_writer_append_end_object(&jw), log);

  *out_span = az_json_writer_get_bytes_used_in_destination(&jw);
}

void pnp_process_device_twin_message(
    az_span twin_message_span,
    bool is_partial,
    az_span const** components_ptr,
    int32_t components_num,
    pnp_property_callback property_callback,
    void* context_ptr)
{
  char const* const log = "Failed to process device twin message";

  int32_t version;

  // Parse twin_message_span.
  az_json_reader jr;
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_init(&jr, twin_message_span, NULL), log);
  IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log);

  // Parse to the `desired` wrapper if it exists.
  if (!is_partial && !json_child_token_move(&jr, iot_hub_twin_desired))
  {
    IOT_SAMPLE_LOG(
        "`%.*s` property object not found in device twin message.",
        az_span_size(iot_hub_twin_desired),
        az_span_ptr(iot_hub_twin_desired));
    return;
  }

  // Parse for `$version` if it exists.
  az_json_reader copy_jr = jr;
  if (!json_child_token_move(&copy_jr, iot_hub_twin_desired_version)
      || az_result_failed(az_json_token_get_int32(&(copy_jr.token), (int32_t*)&version)))
  {
    IOT_SAMPLE_LOG(
        "`%.*s` was not found in device twin message.",
        az_span_size(iot_hub_twin_desired_version),
        az_span_ptr(iot_hub_twin_desired_version));
    return;
  }

  // Parse the properties and call property_callback for each.
  az_json_token property_name;
  while (az_result_succeeded(az_json_reader_next_token(&jr)))
  {
    if (jr.token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
    {
      // Parse to the `desired` wrapper.
      if (az_json_token_is_text_equal(&jr.token, iot_hub_twin_desired_version))
      {
        IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log);
        continue;
      }

      // Found a property.
      property_name = jr.token;
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_next_token(&jr), log);

      int32_t index;
      if ((jr.token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT) && (components_ptr != NULL)
          && is_component_in_model(property_name.slice, components_ptr, components_num, &index))
      {
        visit_component_properties(
            *components_ptr[index], &jr, version, property_callback, context_ptr);
      }
      else
      {
        property_callback(AZ_SPAN_EMPTY, &property_name, jr, version, context_ptr);
      }
    }
    else if (jr.token.kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
    {
      IOT_SAMPLE_EXIT_IF_AZ_FAILED(az_json_reader_skip_children(&jr), log);
    }
    else if (jr.token.kind == AZ_JSON_TOKEN_END_OBJECT)
    {
      break;
    }
  }
}
