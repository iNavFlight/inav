// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Definition for the Azure IoT ADU Client
 *
 * @note More details about Azure Device Update can be found online
 * at https://docs.microsoft.com/azure/iot-hub-device-update/understand-device-update
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 *
 */

#ifndef _az_IOT_ADU_H
#define _az_IOT_ADU_H

#include <azure/core/az_json.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_hub_client.h>
#include <azure/iot/internal/az_iot_adu_internal.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief  Define the ADU agent model ID.
 *
 * https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/azure/iot/deviceupdatecontractmodel-2.json
 */
#define AZ_IOT_ADU_CLIENT_AGENT_MODEL_ID "dtmi:azure:iot:deviceUpdateContractModel;2"

/**
 * @brief ADU Agent Version
 */
#define AZ_IOT_ADU_CLIENT_AGENT_VERSION "DU;agent/1.0.0"

/**
 * @brief ADU PnP Component Name
 */
#define AZ_IOT_ADU_CLIENT_PROPERTIES_COMPONENT_NAME "deviceUpdate"

/**
 * @brief Default Agent Compatibility Properties
 */
#define AZ_IOT_ADU_CLIENT_AGENT_DEFAULT_COMPATIBILITY_PROPERTIES "manufacturer,model"

/**
 * @brief Decision codes to accept or reject a received update deployment.
 */
typedef enum
{
  /// ADU Service Response (Accept)
  AZ_IOT_ADU_CLIENT_REQUEST_DECISION_ACCEPT = 200,
  /// ADU Service Response (Reject)
  AZ_IOT_ADU_CLIENT_REQUEST_DECISION_REJECT = 406
} az_iot_adu_client_request_decision;

/**
 * @brief Agent states used to notify the ADU service of current state.
 */
typedef enum
{
  /// ADU Agent State (Idle)
  AZ_IOT_ADU_CLIENT_AGENT_STATE_IDLE = 0,
  /// ADU Agent State (In Progress)
  AZ_IOT_ADU_CLIENT_AGENT_STATE_DEPLOYMENT_IN_PROGRESS = 6,
  /// ADU Agent State (Failed)
  AZ_IOT_ADU_CLIENT_AGENT_STATE_FAILED = 255
} az_iot_adu_client_agent_state;

/**
 * @brief Actions specified by the service for the agent to process.
 */
typedef enum
{
  /// ADU Service Action (Apply)
  AZ_IOT_ADU_CLIENT_SERVICE_ACTION_APPLY_DEPLOYMENT = 3,
  /// ADU Service Action (Cancel)
  AZ_IOT_ADU_CLIENT_SERVICE_ACTION_CANCEL = 255
} az_iot_adu_client_service_action;

/**
 * @brief     Identity of the update request.
 * @remark    This version refers to the update request itself.
 *            For verifying if an update request is applicable to an
 *            ADU agent, use the
 *            #az_iot_adu_client_update_manifest_instructions_step_handler_properties.installed_criteria.
 */
typedef struct
{
  /**
   * The provider for the update.
   */
  az_span provider;
  /**
   * The name for the update.
   */
  az_span name;
  /**
   * The version for the update.
   */
  az_span version;
} az_iot_adu_update_id;

/**
 * @brief Holds any user-defined custom properties for the device.
 * @remark Implementer can define other device properties to be used
 *         for the compatibility check when targeting the update deployment.
 */
typedef struct
{
  /**
   * An array holding the custom names for the device properties.
   */
  az_span names[_az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES];
  /**
   * An array holding the custom values for the device properties.
   */
  az_span values[_az_IOT_ADU_CLIENT_MAX_DEVICE_CUSTOM_PROPERTIES];
  /**
   * The number of custom names and values.
   */
  int32_t count;
} az_iot_adu_device_custom_properties;

/**
 * @brief      Holds the ADU agent device properties.
 *
 * az_iot_adu_client_device_properties_default() must be called to first initialize the
 * device properties.
 *
 * @remarks    These properties are used by the ADU service for matching
 *             update groups and verifying the current update deployed.
 * https://docs.microsoft.com/azure/iot-hub-device-update/device-update-plug-and-play
 */
typedef struct
{
  /**
   * The device manufacturer of the device, reported through deviceProperties.
   */
  az_span manufacturer;
  /**
   * The device model of the device, reported through deviceProperties.
   */
  az_span model;
  /**
   * Implementer can define other device properties to be used for the
   * compatibility check while targeting the update deployment.
   */
  az_iot_adu_device_custom_properties* custom_properties;
  /**
   * Version of the Device Update agent running on the device.
   * @remark Must be set to AZ_IOT_ADU_CLIENT_AGENT_VERSION.
   */
  az_span adu_version;
  /**
   * Version of the Delivery Optimization agent.
   * @remark Please see Azure Device Update documentation on how to use
   *         the delivery optimization agent. If unused, set to #AZ_SPAN_EMPTY.
   *
   * https://docs.microsoft.com/azure/iot-hub-device-update/device-update-plug-and-play#device-properties
   */
  az_span delivery_optimization_agent_version;
  /**
   * An ID of the update that is currently installed.
   */
  az_span update_id;
} az_iot_adu_client_device_properties;

/**
 * @brief The update step result reported by the agent.
 *
 * This helps provide detailed results for a specific step of the update process.
 *
 */
typedef struct
{
  /**
   * A code that contains information about the result of the last update action.
   * Example: 700
   */
  int32_t result_code;
  /**
   * A code that contains additional information about the result.
   * Example: 0x80004005
   */
  int32_t extended_result_code;
  /**
   * Customer-defined free form string to provide additional result details.
   */
  az_span result_details;
} az_iot_adu_client_step_result;

/**
 * @brief The update result reported by the agent.
 *
 * This details the result for the overall update.
 */
typedef struct
{
  /**
   * A code that contains information about the result of the last update action.
   * Example: 700
   */
  int32_t result_code;
  /**
   * A code that contains additional information about the result.
   * Example: 0x80004006
   */
  int32_t extended_result_code;
  /**
   * Customer-defined free form string to provide additional result details.
   */
  az_span result_details;
  /**
   * Number of items in \p step_results.
   */
  int32_t step_results_count;
  /**
   * The results for each step in the update manifest instructions.
   * The number of steps MUST match the number of steps in the
   * update manifest for the resulting state to be property generated.
   */
  az_iot_adu_client_step_result step_results[_az_IOT_ADU_CLIENT_MAX_INSTRUCTIONS_STEPS];
} az_iot_adu_client_install_result;

/**
 * @brief A set of values that indicate which deployment the agent is currently working on.
 *
 */
typedef struct
{
  /**
   * An integer that corresponds to an action the agent should perform.
   * @remark Refer to the #az_iot_adu_client_service_action for possible values
   */
  az_iot_adu_client_service_action action;
  /**
   * ID of current deployment.
   */
  az_span id;
  /**
   * Time of last deployment retry.
   */
  az_span retry_timestamp;
} az_iot_adu_client_workflow;

/**
 * @brief A map of file ID to download url.
 */
typedef struct
{
  /**
   * File ID, mapped in the updated manifest.
   */
  az_span id;
  /**
   * Complete url to a file.
   */
  az_span url;
} az_iot_adu_client_file_url;

/**
 * @brief Structure that holds the parsed contents of the ADU
 *        request in the Plug and Play writable properties sent
 *        by the ADU service.
 */
typedef struct
{
  /**
   * A set of values that indicate which deployment the agent is currently working on.
   */
  az_iot_adu_client_workflow workflow;
  /**
   * Description of the content of an update.
   * @note This will come as an escaped string. This is done to guarantee ordering
   * of JSON values so that we may verify a signature over the payload.
   * The user must unescape it using an API such as az_json_string_unescape() before
   * subsequently calling az_iot_adu_client_parse_update_manifest() with it.
   */
  az_span update_manifest;
  /**
   * A JSON Web Signature (JWS) with JSON Web Keys used for source verification.
   */
  az_span update_manifest_signature;
  /**
   * An array of files associated with the deployment. These are then correlated
   * with specific steps of the update via their IDs.
   */
  az_iot_adu_client_file_url file_urls[_az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT];
  /**
   * Number of items in \p file_urls.
   */
  uint32_t file_urls_count;
} az_iot_adu_client_update_request;

/**
 * @brief User-defined properties for handling an update request.
 *
 */
typedef struct
{
  /**
   * The installed criteria which defines a successful installation for a given step.
   */
  az_span installed_criteria;
} az_iot_adu_client_update_manifest_instructions_step_handler_properties;

/**
 * @brief Step in the instructions of an update manifest.
 *
 */
typedef struct
{
  /**
   * Name of the update agent handler type that is expected to handle the step.
   * @remark For more details on handler types, please see here:
   * https://learn.microsoft.com/azure/iot-hub-device-update/device-update-agent-overview#update-handlers
   *
   * Generally, for full image updates on embedded devices, the update handler type will
   * be `microsoft/swupdate:<version-number>`.
   */
  az_span handler;
  /**
   * Files needed for this update step, as an array of file ids. These ids
   * can also be found in #az_iot_adu_client_update_manifest.files
   * with their respective urls.
   */
  az_span files[_az_IOT_ADU_CLIENT_MAX_FILE_COUNT_PER_STEP];
  /**
   * Number of items in \p files.
   */
  uint32_t files_count;
  /**
   * Additional user-defined properties for the update step handler.
   */
  az_iot_adu_client_update_manifest_instructions_step_handler_properties handler_properties;
} az_iot_adu_client_update_manifest_instructions_step;

/**
 * @brief Instructions in the update manifest.
 */
typedef struct
{
  /**
   * Steps for the instructions in an update request.
   */
  az_iot_adu_client_update_manifest_instructions_step
      steps[_az_IOT_ADU_CLIENT_MAX_INSTRUCTIONS_STEPS];
  /**
   * Number of items in \p steps.
   */
  uint32_t steps_count;
} az_iot_adu_client_update_manifest_instructions;

/**
 * @brief Hash value for a given file.
 *
 */
typedef struct
{
  /**
   * The hash type for the file (Example: sha256).
   */
  az_span hash_type;
  /**
   * The value of the hash.
   */
  az_span hash_value;
} az_iot_adu_client_update_manifest_file_hash;

/**
 * @brief Details of a file referenced in the update request.
 *
 * @note C SDK will not support delta updates at this time, so relatedFiles,
 * downloadHandler, and mimeType are not exposed or processed.
 */
typedef struct
{
  /**
   * Identity of a file, correlated with the same id in #az_iot_adu_client_file_url
   * and #az_iot_adu_client_update_manifest_instructions_step.files.
   */
  az_span id;
  /**
   * Name of the file.
   */
  az_span file_name;
  /**
   * Size of a file, in bytes.
   */
  int64_t size_in_bytes;
  /**
   * Hashes provided for a given file in the update request.
   */
  az_iot_adu_client_update_manifest_file_hash hashes[_az_IOT_ADU_CLIENT_MAX_FILE_HASH_COUNT];
  /**
   * Number of items in \p hashes.
   */
  uint32_t hashes_count;
} az_iot_adu_client_update_manifest_file;

/**
 * @brief Structure that holds the parsed contents of the update manifest
 *        sent by the ADU service.
 */
typedef struct
{
  /**
   * Version of the update manifest schema.
   */
  az_span manifest_version;
  /**
   * User-defined identity of the update manifest.
   */
  az_iot_adu_update_id update_id;
  /**
   * Instructions of the update manifest.
   */
  az_iot_adu_client_update_manifest_instructions instructions;
  /**
   * Download urls for the files referenced in the update manifest instructions.
   */
  az_iot_adu_client_update_manifest_file files[_az_IOT_ADU_CLIENT_MAX_TOTAL_FILE_COUNT];
  /**
   * Number of items in \p files.
   */
  uint32_t files_count;
  /**
   * The creation date and time.
   */
  az_span create_date_time;
} az_iot_adu_client_update_manifest;

/**
 * @brief User-defined options for the Azure IoT ADU client.
 *
 */
typedef struct
{
  /**
   * The custom device compatibility properties for the device.
   */
  az_span device_compatibility_properties;
} az_iot_adu_client_options;

/**
 * @brief Structure that holds the state of the Azure IoT ADU client.
 *
 */
typedef struct
{
  struct
  {
    az_iot_adu_client_options options;
  } _internal;
} az_iot_adu_client;

/**
 * @brief Gets the default Azure IoT ADU Client options.
 * @details Call this to obtain an initialized #az_iot_adu_client_options structure that can be
 * afterwards modified and passed to #az_iot_adu_client_init.
 *
 * @return #az_iot_adu_client_options.
 */
AZ_NODISCARD az_iot_adu_client_options az_iot_adu_client_options_default();

/**
 * @brief Gets the default #az_iot_adu_client_device_properties.
 * @details Call this to obtain an initialized #az_iot_adu_client_device_properties structure that
 * can be afterwards modified and passed to necessary APIs.
 *
 * @return #az_iot_adu_client_device_properties.
 */
AZ_NODISCARD az_iot_adu_client_device_properties az_iot_adu_client_device_properties_default();

/**
 * @brief Initializes an Azure IoT ADU Client.
 *
 * @param client  The #az_iot_adu_client to use for this call.
 * @param options A reference to an #az_iot_adu_client_options structure. If `NULL` is passed,
 * the adu client will use the default options. If using custom options, please initialize first by
 * calling az_iot_adu_client_options_default() and then populating relevant options with your own
 * values.
 * @pre \p client must not be `NULL`.
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result
az_iot_adu_client_init(az_iot_adu_client* client, az_iot_adu_client_options* options);

/**
 * @brief Verifies if the Azure Plug-and-Play writable properties component
 *        is for ADU device update.
 *
 * @param[in] client            The #az_iot_adu_client to use for this call.
 * @param[in] component_name    #az_span pointing to the component name in the
 *                              writable properties.
 * @return A boolean indicating if the component name is for ADU device update.
 */
AZ_NODISCARD bool az_iot_adu_client_is_component_device_update(
    az_iot_adu_client* client,
    az_span component_name);

/**
 * @brief Generates the Azure Plug-and-Play (reported) properties payload
 *        with the state of the ADU agent.
 *
 * @param[in] client                The #az_iot_adu_client to use for this call.
 * @param[in] device_properties     A pointer to a #az_iot_adu_client_device_properties
 *                                  structure with all the details of the device,
 *                                  as required by the ADU service.
 * @param[in] agent_state           An integer value indicating the current state of
 *                                  the ADU agent. Use the values defined by the
 *                                  #az_iot_adu_client_agent_state.
 *                                  Please see the ADU online documentation for more
 *                                  details.
 * @param[in] workflow              A pointer to a #az_iot_adu_client_workflow instance
 *                                  indicating the current ADU workflow being processed,
 *                                  if an ADU service workflow was received. Use NULL
 *                                  if no device update is in progress.
 * @param[in] last_install_result   A pointer to a #az_iot_adu_client_install_result
 *                                  instance with the results of the current or past
 *                                  device update workflow, if available. Use NULL
 *                                  if no results are available.
 * @param[in,out] ref_json_writer   An #az_json_writer initialized with the memory where
 *                                  to write the property payload.
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_iot_adu_client_get_agent_state_payload(
    az_iot_adu_client* client,
    az_iot_adu_client_device_properties* device_properties,
    az_iot_adu_client_agent_state agent_state,
    az_iot_adu_client_workflow* workflow,
    az_iot_adu_client_install_result* last_install_result,
    az_json_writer* ref_json_writer);

/**
 * @brief Parses the json content from the ADU service writable properties into
 *        a pre-defined structure.
 *
 * @param[in] client               The #az_iot_adu_client to use for this call.
 * @param[in] ref_json_reader      A #az_json_reader initialized with the ADU
 *                                 service writable properties json, set to the
 *                                 beginning of the json object that is the value
 *                                 of the ADU component.
 * @param[out] update_request      A pointer to the #az_iot_adu_client_update_request
 *                                 structure where to store the parsed contents
 *                                 read from the `ref_json_reader` json reader.
 *                                 In summary, this structure holds #az_span
 *                                 instances that point to the actual data
 *                                 parsed from `ref_json_reader` and copied to `buffer`.
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_iot_adu_client_parse_service_properties(
    az_iot_adu_client* client,
    az_json_reader* ref_json_reader,
    az_iot_adu_client_update_request* update_request);

/**
 * @brief    Generates the payload necessary to respond to the service
             after receiving incoming properties.
 *
 * @param[in] client            The #az_iot_adu_client to use for this call.
 * @param[in] version           Version of the writable properties.
 * @param[in] status            Azure Plug-and-Play status code for the
 *                              writable properties acknowledgement.
 * @param[in] ref_json_writer   An #az_json_writer pointing to the memory buffer where to
 *                              write the resulting Azure Plug-and-Play properties.
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_iot_adu_client_get_service_properties_response(
    az_iot_adu_client* client,
    int32_t version,
    az_iot_adu_client_request_decision status,
    az_json_writer* ref_json_writer);

/**
 * @brief Parses the json content from the ADU service update manifest into
 *        a pre-defined structure.
 *
 * @param[in] client              The #az_iot_adu_client to use for this call.
 * @param[in] ref_json_reader     ADU update manifest, as initialized json reader.
 * @param[out] update_manifest    The structure where the parsed values of the
 *                                manifest are stored. Values are not copied from
 *                                `payload`, the fields of the structure just
 *                                point to the positions in `payload` where the
 *                                data is present, except for numeric and boolean
 *                                values (which are parsed into the respective
 *                                data types).
 * @return An #az_result value indicating the result of the operation.
 */
AZ_NODISCARD az_result az_iot_adu_client_parse_update_manifest(
    az_iot_adu_client* client,
    az_json_reader* ref_json_reader,
    az_iot_adu_client_update_manifest* update_manifest);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_IOT_ADU_H
