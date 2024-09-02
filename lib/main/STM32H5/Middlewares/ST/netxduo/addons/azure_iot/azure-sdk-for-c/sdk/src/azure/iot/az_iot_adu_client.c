// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/iot/az_iot_adu_client.h>
#include <azure/iot/az_iot_hub_client_properties.h>

#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <stdio.h>

/* Define the ADU agent component name.  */
#define AZ_IOT_ADU_CLIENT_AGENT_COMPONENT_NAME "deviceUpdate"

#define AZ_IOT_ADU_CLIENT_AGENT_INTERFACE_ID "dtmi:azure:iot:deviceUpdate;1"

/* Define the ADU agent property name "agent" and sub property names.  */
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_AGENT "agent"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DEVICEPROPERTIES "deviceProperties"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MANUFACTURER "manufacturer"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MODEL "model"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INTERFACE_ID "interfaceId"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ADU_VERSION "aduVer"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DO_VERSION "doVer"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_COMPAT_PROPERTY_NAMES "compatPropertyNames"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTALLED_UPDATE_ID "installedUpdateId"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_PROVIDER "provider"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_NAME "name"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_VERSION "version"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_LAST_INSTALL_RESULT "lastInstallResult"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_CODE "resultCode"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE "extendedResultCode"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_DETAILS "resultDetails"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_STEP_RESULTS "stepResults"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_STATE "state"

/* Define the ADU agent property name "service" and sub property names.  */
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SERVICE "service"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_WORKFLOW "workflow"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ACTION "action"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ID "id"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP "retryTimestamp"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_MANIFEST "updateManifest"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_MANIFEST_SIGNATURE "updateManifestSignature"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILEURLS "fileUrls"

#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MANIFEST_VERSION "manifestVersion"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_ID "updateId"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_COMPATIBILITY "compatibility"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DEVICE_MANUFACTURER "deviceManufacturer"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DEVICE_MODEL "deviceModel"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_GROUP "group"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTRUCTIONS "instructions"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_STEPS "steps"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_TYPE "type"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HANDLER "handler"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HANDLER_PROPERTIES "handlerProperties"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILES "files"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DETACHED_MANIFEST_FILED "detachedManifestFileId"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTALLED_CRITERIA "installedCriteria"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILE_NAME "fileName"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SIZE_IN_BYTES "sizeInBytes"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HASHES "hashes"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SHA256 "sha256"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_CREATED_DATE_TIME "createdDateTime"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DOWNLOAD_HANDLER "downloadHandler"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RELATED_FILES "relatedFiles"
#define AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MIME_TYPE "mimeType"

#define NULL_TERM_CHAR_SIZE 1

#define RESULT_STEP_ID_PREFIX "step_"
#define MAX_UINT32_NUMBER_OF_DIGITS 10
#define RESULT_STEP_ID_MAX_SIZE (sizeof(RESULT_STEP_ID_PREFIX) - 1 + MAX_UINT32_NUMBER_OF_DIGITS)

#define RETURN_IF_JSON_TOKEN_NOT_TYPE(jr_ptr, json_token_type) \
  if (jr_ptr->token.kind != json_token_type)                   \
  {                                                            \
    return AZ_ERROR_JSON_INVALID_STATE;                        \
  }

#define RETURN_IF_JSON_TOKEN_NOT_TEXT(jr_ptr, literal_text)                         \
  if (!az_json_token_is_text_equal(&jr_ptr->token, AZ_SPAN_FROM_STR(literal_text))) \
  {                                                                                 \
    return AZ_ERROR_JSON_INVALID_STATE;                                             \
  }

const az_span default_compatibility_properties
    = AZ_SPAN_LITERAL_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_DEFAULT_COMPATIBILITY_PROPERTIES);

AZ_NODISCARD az_iot_adu_client_options az_iot_adu_client_options_default()
{
  return (az_iot_adu_client_options){ .device_compatibility_properties
                                      = default_compatibility_properties };
}

AZ_NODISCARD az_iot_adu_client_device_properties az_iot_adu_client_device_properties_default()
{
  return (az_iot_adu_client_device_properties){ .manufacturer = AZ_SPAN_LITERAL_EMPTY,
                                                .model = AZ_SPAN_LITERAL_EMPTY,
                                                .custom_properties = NULL,
                                                .adu_version = AZ_SPAN_LITERAL_EMPTY,
                                                .delivery_optimization_agent_version
                                                = AZ_SPAN_LITERAL_EMPTY,
                                                .update_id = AZ_SPAN_LITERAL_EMPTY };
}

AZ_NODISCARD az_result
az_iot_adu_client_init(az_iot_adu_client* client, az_iot_adu_client_options* options)
{
  _az_PRECONDITION_NOT_NULL(client);

  client->_internal.options = options == NULL ? az_iot_adu_client_options_default() : *options;

  return AZ_OK;
}

AZ_NODISCARD bool az_iot_adu_client_is_component_device_update(
    az_iot_adu_client* client,
    az_span component_name)
{
  _az_PRECONDITION_NOT_NULL(client);

  (void)client;

  return az_span_is_content_equal(
      AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_COMPONENT_NAME), component_name);
}

static az_result _generate_step_id(az_span buffer, uint32_t step_index, az_span* step_id)
{
  az_result result;
  *step_id = buffer;
  buffer = az_span_copy(buffer, AZ_SPAN_FROM_STR(RESULT_STEP_ID_PREFIX));

  result = az_span_u32toa(buffer, step_index, &buffer);
  _az_RETURN_IF_FAILED(result);

  *step_id = az_span_slice(*step_id, 0, az_span_size(*step_id) - az_span_size(buffer));

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_adu_client_get_agent_state_payload(
    az_iot_adu_client* client,
    az_iot_adu_client_device_properties* device_properties,
    az_iot_adu_client_agent_state agent_state,
    az_iot_adu_client_workflow* workflow,
    az_iot_adu_client_install_result* last_install_result,
    az_json_writer* ref_json_writer)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(device_properties);
  _az_PRECONDITION_VALID_SPAN(device_properties->manufacturer, 1, false);
  _az_PRECONDITION_VALID_SPAN(device_properties->model, 1, false);
  _az_PRECONDITION_VALID_SPAN(device_properties->update_id, 1, false);
  _az_PRECONDITION_VALID_SPAN(device_properties->adu_version, 1, false);
  _az_PRECONDITION_NOT_NULL(ref_json_writer);

  uint8_t step_id_scratch_buffer[7];

  /* Update reported property */
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

  /* Fill the ADU agent component name.  */
  _az_RETURN_IF_FAILED(az_iot_hub_client_properties_writer_begin_component(
      NULL, ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_COMPONENT_NAME)));

  /* Fill the agent property name.  */
  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_AGENT)));
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

  /* Fill the deviceProperties.  */
  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DEVICEPROPERTIES)));
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MANUFACTURER)));
  _az_RETURN_IF_FAILED(
      az_json_writer_append_string(ref_json_writer, device_properties->manufacturer));

  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MODEL)));
  _az_RETURN_IF_FAILED(az_json_writer_append_string(ref_json_writer, device_properties->model));

  if (device_properties->custom_properties != NULL)
  {
    for (int32_t custom_property_index = 0;
         custom_property_index < device_properties->custom_properties->count;
         custom_property_index++)
    {
      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
          ref_json_writer, device_properties->custom_properties->names[custom_property_index]));
      _az_RETURN_IF_FAILED(az_json_writer_append_string(
          ref_json_writer, device_properties->custom_properties->values[custom_property_index]));
    }
  }

  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INTERFACE_ID)));
  _az_RETURN_IF_FAILED(az_json_writer_append_string(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_INTERFACE_ID)));

  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ADU_VERSION)));
  _az_RETURN_IF_FAILED(
      az_json_writer_append_string(ref_json_writer, device_properties->adu_version));

  if (!az_span_is_content_equal(
          device_properties->delivery_optimization_agent_version, AZ_SPAN_EMPTY))
  {
    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DO_VERSION)));
    _az_RETURN_IF_FAILED(az_json_writer_append_string(
        ref_json_writer, device_properties->delivery_optimization_agent_version));
  }

  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));

  /* Fill the compatibility property names. */
  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer,
      AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_COMPAT_PROPERTY_NAMES)));
  _az_RETURN_IF_FAILED(az_json_writer_append_string(
      ref_json_writer, client->_internal.options.device_compatibility_properties));

  /* Add last installed update information */
  if (last_install_result != NULL)
  {
    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer,
        AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_LAST_INSTALL_RESULT)));
    _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_CODE)));
    _az_RETURN_IF_FAILED(
        az_json_writer_append_int32(ref_json_writer, last_install_result->result_code));

    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer,
        AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE)));
    _az_RETURN_IF_FAILED(
        az_json_writer_append_int32(ref_json_writer, last_install_result->extended_result_code));

    if (!az_span_is_content_equal(last_install_result->result_details, AZ_SPAN_EMPTY))
    {
      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
          ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_DETAILS)));
      _az_RETURN_IF_FAILED(
          az_json_writer_append_string(ref_json_writer, last_install_result->result_details));
    }

    for (int32_t i = 0; i < last_install_result->step_results_count; i++)
    {
      az_span step_id = AZ_SPAN_FROM_BUFFER(step_id_scratch_buffer);
      _az_RETURN_IF_FAILED(_generate_step_id(step_id, (uint32_t)i, &step_id));

      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(ref_json_writer, step_id));
      _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
          ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_CODE)));
      _az_RETURN_IF_FAILED(az_json_writer_append_int32(
          ref_json_writer, last_install_result->step_results[i].result_code));

      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
          ref_json_writer,
          AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE)));
      _az_RETURN_IF_FAILED(az_json_writer_append_int32(
          ref_json_writer, last_install_result->step_results[i].extended_result_code));

      if (!az_span_is_content_equal(
              last_install_result->step_results[i].result_details, AZ_SPAN_EMPTY))
      {
        _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
            ref_json_writer,
            AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RESULT_DETAILS)));
        _az_RETURN_IF_FAILED(az_json_writer_append_string(
            ref_json_writer, last_install_result->step_results[i].result_details));
      }

      _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));
    }

    _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));
  }

  /* Fill the agent state.   */
  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_STATE)));
  _az_RETURN_IF_FAILED(az_json_writer_append_int32(ref_json_writer, (int32_t)agent_state));

  /* Fill the workflow.  */
  if (workflow != NULL && (az_span_ptr(workflow->id) != NULL && az_span_size(workflow->id) > 0))
  {
    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_WORKFLOW)));
    _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));

    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ACTION)));
    _az_RETURN_IF_FAILED(az_json_writer_append_int32(ref_json_writer, (int32_t)workflow->action));

    _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
        ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ID)));
    _az_RETURN_IF_FAILED(az_json_writer_append_string(ref_json_writer, workflow->id));

    /* Append retry timestamp in workflow if existed.  */
    if (!az_span_is_content_equal(workflow->retry_timestamp, AZ_SPAN_EMPTY))
    {
      _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
          ref_json_writer,
          AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP)));
      _az_RETURN_IF_FAILED(
          az_json_writer_append_string(ref_json_writer, workflow->retry_timestamp));
    }
    _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));
  }

  /* Fill installed update id. */
  _az_RETURN_IF_FAILED(az_json_writer_append_property_name(
      ref_json_writer,
      AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTALLED_UPDATE_ID)));
  _az_RETURN_IF_FAILED(az_json_writer_append_string(ref_json_writer, device_properties->update_id));

  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));

  _az_RETURN_IF_FAILED(az_iot_hub_client_properties_writer_end_component(NULL, ref_json_writer));
  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_adu_client_parse_service_properties(
    az_iot_adu_client* client,
    az_json_reader* ref_json_reader,
    az_iot_adu_client_update_request* update_request)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(ref_json_reader);
  _az_PRECONDITION_NOT_NULL(update_request);

  (void)client;

  RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_PROPERTY_NAME);
  RETURN_IF_JSON_TOKEN_NOT_TEXT(ref_json_reader, AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SERVICE);

  _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
  RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_BEGIN_OBJECT);
  _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

  update_request->workflow.action = 0;
  update_request->workflow.id = AZ_SPAN_EMPTY;
  update_request->workflow.retry_timestamp = AZ_SPAN_EMPTY;
  update_request->update_manifest = AZ_SPAN_EMPTY;
  update_request->update_manifest_signature = AZ_SPAN_EMPTY;
  update_request->file_urls_count = 0;

  while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
  {
    RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_PROPERTY_NAME);

    if (az_json_token_is_text_equal(
            &ref_json_reader->token,
            AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_WORKFLOW)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_BEGIN_OBJECT);
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

      while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
      {
        RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_PROPERTY_NAME);

        if (az_json_token_is_text_equal(
                &ref_json_reader->token,
                AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ACTION)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          _az_RETURN_IF_FAILED(az_json_token_get_int32(
              &ref_json_reader->token, (int32_t*)&update_request->workflow.action));
        }
        else if (az_json_token_is_text_equal(
                     &ref_json_reader->token,
                     AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_ID)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

          update_request->workflow.id = ref_json_reader->token.slice;
        }
        else if (az_json_token_is_text_equal(
                     &ref_json_reader->token,
                     AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          update_request->workflow.retry_timestamp = ref_json_reader->token.slice;
        }
        else
        {
          _az_LOG_WRITE(
              AZ_LOG_IOT_ADU,
              AZ_SPAN_FROM_STR("Unexpected property found in ADU manifest workflow:"));
          _az_LOG_WRITE(AZ_LOG_IOT_ADU, ref_json_reader->token.slice);
          return AZ_ERROR_JSON_INVALID_STATE;
        }

        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_MANIFEST)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

      if (ref_json_reader->token.kind != AZ_JSON_TOKEN_NULL)
      {
        update_request->update_manifest = ref_json_reader->token.slice;
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_MANIFEST_SIGNATURE)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

      if (ref_json_reader->token.kind != AZ_JSON_TOKEN_NULL)
      {
        update_request->update_manifest_signature = ref_json_reader->token.slice;
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILEURLS)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      if (ref_json_reader->token.kind != AZ_JSON_TOKEN_NULL)
      {
        RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_BEGIN_OBJECT);
        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

        while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
        {
          RETURN_IF_JSON_TOKEN_NOT_TYPE(ref_json_reader, AZ_JSON_TOKEN_PROPERTY_NAME);

          // If object isn't ended and we have reached max files allowed, next would overflow.
          if (update_request->file_urls_count == _az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT)
          {
            return AZ_ERROR_NOT_ENOUGH_SPACE;
          }

          update_request->file_urls[update_request->file_urls_count].id
              = ref_json_reader->token.slice;

          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          if (ref_json_reader->token.kind != AZ_JSON_TOKEN_NULL)
          {
            update_request->file_urls[update_request->file_urls_count].url
                = ref_json_reader->token.slice;

            update_request->file_urls_count++;
          }

          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        }
      }
    }

    _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
  }

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_adu_client_get_service_properties_response(
    az_iot_adu_client* client,
    int32_t version,
    az_iot_adu_client_request_decision status,
    az_json_writer* ref_json_writer)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(ref_json_writer);

  (void)client;

  // Component and response status
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));
  _az_RETURN_IF_FAILED(az_iot_hub_client_properties_writer_begin_component(
      NULL, ref_json_writer, AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_COMPONENT_NAME)));
  _az_RETURN_IF_FAILED(az_iot_hub_client_properties_writer_begin_response_status(
      NULL,
      ref_json_writer,
      AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SERVICE),
      (int32_t)status,
      version,
      AZ_SPAN_EMPTY));

  // It is not necessary to send the properties back in the acknowledgement.
  // We opt not to send them to reduce the size of the payload.
  _az_RETURN_IF_FAILED(az_json_writer_append_begin_object(ref_json_writer));
  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));

  _az_RETURN_IF_FAILED(
      az_iot_hub_client_properties_writer_end_response_status(NULL, ref_json_writer));
  _az_RETURN_IF_FAILED(az_iot_hub_client_properties_writer_end_component(NULL, ref_json_writer));
  _az_RETURN_IF_FAILED(az_json_writer_append_end_object(ref_json_writer));

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_adu_client_parse_update_manifest(
    az_iot_adu_client* client,
    az_json_reader* ref_json_reader,
    az_iot_adu_client_update_manifest* update_manifest)
{
  _az_PRECONDITION_NOT_NULL(client);
  _az_PRECONDITION_NOT_NULL(ref_json_reader);
  _az_PRECONDITION_NOT_NULL(update_manifest);

  (void)client;

  // Initialize the update_manifest with empty values.
  update_manifest->manifest_version = AZ_SPAN_EMPTY;
  update_manifest->update_id.name = AZ_SPAN_EMPTY;
  update_manifest->update_id.provider = AZ_SPAN_EMPTY;
  update_manifest->update_id.version = AZ_SPAN_EMPTY;
  update_manifest->instructions.steps_count = 0;
  update_manifest->files_count = 0;
  update_manifest->create_date_time = AZ_SPAN_EMPTY;

  _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
  RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
  _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

  while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
  {
    RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

    bool property_parsed = true;

    if (az_json_token_is_text_equal(
            &ref_json_reader->token,
            AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MANIFEST_VERSION)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
      update_manifest->manifest_version = ref_json_reader->token.slice;
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTRUCTIONS)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

      if (az_json_token_is_text_equal(
              &ref_json_reader->token,
              AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_STEPS)))
      {
        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_ARRAY);
        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

        update_manifest->instructions.steps_count = 0;

        while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_ARRAY)
        {
          uint32_t step_index = update_manifest->instructions.steps_count;

          RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

          while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
          {
            RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

            if (az_json_token_is_text_equal(
                    &ref_json_reader->token,
                    AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HANDLER)))
            {
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);

              update_manifest->instructions.steps[step_index].handler
                  = ref_json_reader->token.slice;
            }
            else if (az_json_token_is_text_equal(
                         &ref_json_reader->token,
                         AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILES)))
            {
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_ARRAY);
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

              update_manifest->instructions.steps[step_index].files_count = 0;

              while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_ARRAY)
              {
                // If array isn't ended and we have reached max files allowed, next would overflow.
                if (update_manifest->instructions.steps[step_index].files_count
                    == _az_IOT_ADU_CLIENT_MAX_FILE_COUNT_PER_STEP)
                {
                  return AZ_ERROR_NOT_ENOUGH_SPACE;
                }
                uint32_t file_index = update_manifest->instructions.steps[step_index].files_count;

                RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);

                update_manifest->instructions.steps[step_index].files[file_index]
                    = ref_json_reader->token.slice;
                update_manifest->instructions.steps[step_index].files_count++;

                _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              }
            }
            else if (az_json_token_is_text_equal(
                         &ref_json_reader->token,
                         AZ_SPAN_FROM_STR(
                             AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HANDLER_PROPERTIES)))
            {
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

              if (az_json_token_is_text_equal(
                      &ref_json_reader->token,
                      AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_INSTALLED_CRITERIA)))
              {
                _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
                RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
                update_manifest->instructions.steps[step_index]
                    .handler_properties.installed_criteria
                    = ref_json_reader->token.slice;
                _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
                RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_END_OBJECT);
              }
              else
              {
                return AZ_ERROR_JSON_INVALID_STATE;
              }
            }
            else
            {
              return AZ_ERROR_JSON_INVALID_STATE;
            }

            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          }

          update_manifest->instructions.steps_count++;

          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        }

        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_END_OBJECT);
      }
      else
      {
        _az_LOG_WRITE(
            AZ_LOG_IOT_ADU, AZ_SPAN_FROM_STR("Unexpected property found in ADU manifest steps:"));
        _az_LOG_WRITE(AZ_LOG_IOT_ADU, ref_json_reader->token.slice);
        return AZ_ERROR_JSON_INVALID_STATE;
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_UPDATE_ID)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

      while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
      {
        RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

        if (az_json_token_is_text_equal(
                &ref_json_reader->token,
                AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_PROVIDER)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
          update_manifest->update_id.provider = ref_json_reader->token.slice;
        }
        else if (az_json_token_is_text_equal(
                     &ref_json_reader->token,
                     AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_NAME)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
          update_manifest->update_id.name = ref_json_reader->token.slice;
        }
        else if (az_json_token_is_text_equal(
                     &ref_json_reader->token,
                     AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_VERSION)))
        {
          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
          update_manifest->update_id.version = ref_json_reader->token.slice;
        }
        else
        {
          _az_LOG_WRITE(
              AZ_LOG_IOT_ADU,
              AZ_SPAN_FROM_STR("Unexpected property found in ADU update id object:"));
          _az_LOG_WRITE(AZ_LOG_IOT_ADU, ref_json_reader->token.slice);
          return AZ_ERROR_JSON_INVALID_STATE;
        }

        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_COMPATIBILITY)))
    {
      /*
       * According to ADU design, the ADU service compatibility properties
       * are not intended to be consumed by the ADU agent.
       * To save on processing, the properties are not being exposed.
       */
      _az_RETURN_IF_FAILED(az_json_reader_skip_children(ref_json_reader));
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILES)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

      while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
      {
        uint32_t files_index = update_manifest->files_count;

        RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

        // If object isn't ended and we have reached max files allowed, next would overflow.
        if (files_index == _az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT)
        {
          return AZ_ERROR_NOT_ENOUGH_SPACE;
        }

        update_manifest->files[files_index].id = ref_json_reader->token.slice;

        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

        while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
        {
          RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);

          if (az_json_token_is_text_equal(
                  &ref_json_reader->token,
                  AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_FILE_NAME)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
            RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
            update_manifest->files[files_index].file_name = ref_json_reader->token.slice;
          }
          else if (az_json_token_is_text_equal(
                       &ref_json_reader->token,
                       AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_SIZE_IN_BYTES)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
            RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_NUMBER);

            _az_RETURN_IF_FAILED(az_json_token_get_int64(
                &ref_json_reader->token, &update_manifest->files[files_index].size_in_bytes));
          }
          else if (az_json_token_is_text_equal(
                       &ref_json_reader->token,
                       AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_HASHES)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
            RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_BEGIN_OBJECT);
            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));

            update_manifest->files[files_index].hashes_count = 0;

            while (ref_json_reader->token.kind != AZ_JSON_TOKEN_END_OBJECT)
            {
              uint32_t hashes_count = update_manifest->files[files_index].hashes_count;

              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_PROPERTY_NAME);
              update_manifest->files[files_index].hashes[hashes_count].hash_type
                  = ref_json_reader->token.slice;
              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
              RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
              update_manifest->files[files_index].hashes[hashes_count].hash_value
                  = ref_json_reader->token.slice;

              update_manifest->files[files_index].hashes_count++;

              _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
            }
          }
          /*
           * Embedded C SDK will not support delta updates at this time, so relatedFiles,
           * downloadHandler, and mimeType are not exposed or processed.
           */
          else if (az_json_token_is_text_equal(
                       &ref_json_reader->token,
                       AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_RELATED_FILES)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_skip_children(ref_json_reader));
          }
          else if (az_json_token_is_text_equal(
                       &ref_json_reader->token,
                       AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_DOWNLOAD_HANDLER)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_skip_children(ref_json_reader));
          }
          else if (az_json_token_is_text_equal(
                       &ref_json_reader->token,
                       AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_MIME_TYPE)))
          {
            _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
          }
          else
          {
            return AZ_ERROR_JSON_INVALID_STATE;
          }

          _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
        }

        update_manifest->files_count++;

        _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      }
    }
    else if (az_json_token_is_text_equal(
                 &ref_json_reader->token,
                 AZ_SPAN_FROM_STR(AZ_IOT_ADU_CLIENT_AGENT_PROPERTY_NAME_CREATED_DATE_TIME)))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      RETURN_IF_JSON_TOKEN_NOT_TYPE((ref_json_reader), AZ_JSON_TOKEN_STRING);
      update_manifest->create_date_time = ref_json_reader->token.slice;
    }
    else
    {
      property_parsed = false;
    }

    if (!property_parsed)
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
      _az_RETURN_IF_FAILED(az_json_reader_skip_children(ref_json_reader));
    }

    _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
  }

  return AZ_OK;
}
