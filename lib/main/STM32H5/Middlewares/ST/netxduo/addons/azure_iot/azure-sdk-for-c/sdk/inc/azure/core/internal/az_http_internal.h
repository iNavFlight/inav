// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines internals used by http.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HTTP_INTERNAL_H
#define _az_HTTP_INTERNAL_H

#include <azure/core/az_context.h>
#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_result.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <azure/core/_az_cfg_prefix.h>

enum
{
  /// The maximum number of HTTP pipeline policies allowed.
  _az_MAXIMUM_NUMBER_OF_POLICIES = 10,
};

/**
 * @brief Internal definition of an HTTP pipeline.
 * Defines the number of policies inside a pipeline.
 */
typedef struct
{
  struct
  {
    _az_http_policy policies[_az_MAXIMUM_NUMBER_OF_POLICIES];
  } _internal;
} _az_http_pipeline;

typedef enum
{
  _az_http_policy_apiversion_option_location_header,
  _az_http_policy_apiversion_option_location_queryparameter
} _az_http_policy_apiversion_option_location;

/**
 * @brief Defines the options structure used by the API Version policy.
 */
typedef struct
{
  // Services pass API versions in the header or in query parameters
  struct
  {
    az_span name;
    az_span version;

    // Avoid using enum as the first field within structs, to allow for { 0 } initialization.
    // This is a workaround for IAR compiler warning [Pe188]: enumerated type mixed with another
    // type.

    _az_http_policy_apiversion_option_location option_location;
  } _internal;
} _az_http_policy_apiversion_options;

/**
 * @brief options for the telemetry policy
 * os = string representation of currently executing Operating System
 *
 */
typedef struct
{
  az_span component_name;
} _az_http_policy_telemetry_options;

/**
 * @brief Creates _az_http_policy_telemetry_options with default values.
 *
 * @param[in] component_name The name of the SDK component.
 *
 * @return Initialized telemetry options.
 */
AZ_NODISCARD AZ_INLINE _az_http_policy_telemetry_options
_az_http_policy_telemetry_options_create(az_span component_name)
{
  _az_PRECONDITION_VALID_SPAN(component_name, 1, false);
  return (_az_http_policy_telemetry_options){ .component_name = component_name };
}

AZ_NODISCARD AZ_INLINE _az_http_policy_apiversion_options
_az_http_policy_apiversion_options_default()
{
  return (_az_http_policy_apiversion_options){
    ._internal = { .name = AZ_SPAN_EMPTY,
                   .version = AZ_SPAN_EMPTY,
                   .option_location = _az_http_policy_apiversion_option_location_header }
  };
}

/**
 * @brief Initialize az_http_policy_retry_options with default values
 *
 */
AZ_NODISCARD az_http_policy_retry_options _az_http_policy_retry_options_default();

// PipelinePolicies
//   Policies are non-allocating caveat the TransportPolicy
//   Transport policies can only allocate if the transport layer they call allocates
// Client ->
//  ===HttpPipelinePolicies===
//    UniqueRequestID
//    Retry
//    Authentication
//    Logging
//    Buffer Response
//    Distributed Tracing
//    TransportPolicy
//  ===Transport Layer===
// PipelinePolicies must implement the process function
//

// Start the pipeline
AZ_NODISCARD az_result az_http_pipeline_process(
    _az_http_pipeline* ref_pipeline,
    az_http_request* ref_request,
    az_http_response* ref_response);

AZ_NODISCARD az_result az_http_pipeline_policy_apiversion(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

AZ_NODISCARD az_result az_http_pipeline_policy_telemetry(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

AZ_NODISCARD az_result az_http_pipeline_policy_retry(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

AZ_NODISCARD az_result az_http_pipeline_policy_credential(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

#ifndef AZ_NO_LOGGING
AZ_NODISCARD az_result az_http_pipeline_policy_logging(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);
#endif // AZ_NO_LOGGING

AZ_NODISCARD az_result az_http_pipeline_policy_transport(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

AZ_NODISCARD AZ_INLINE az_result _az_http_pipeline_nextpolicy(
    _az_http_policy* ref_policies,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  // Transport Policy is the last policy in the pipeline
  //  it returns without calling nextpolicy
  if (ref_policies[0]._internal.process == NULL)
  {
    return AZ_ERROR_HTTP_PIPELINE_INVALID_POLICY;
  }

  return ref_policies[0]._internal.process(
      &(ref_policies[1]), ref_policies[0]._internal.options, ref_request, ref_response);
}

/**
 * @brief Format buffer as a http request containing URL and header spans.
 *
 * @remark The initial \p url provided by the caller is expected to already be url-encoded.
 *
 * @param[out] out_request HTTP request to initialize.
 * @param[in] context A pointer to an #az_context node.
 * @param[in] method HTTP verb: `"GET"`, `"POST"`, etc.
 * @param[in] url The #az_span to be used for storing the url. An initial value is expected to be in
 * the buffer containing url schema and the server address. It can contain query parameters (like
 * https://service.azure.com?query=1). This value is expected to be url-encoded.
 * @param[in] url_length The size of the initial url value within url #az_span.
 * @param[in] headers_buffer The #az_span to be used for storing headers for the request. The total
 * number of headers are calculated automatically based on the size of the buffer.
 * @param[in] body The #az_span buffer that contains a payload for the request. Use #AZ_SPAN_EMPTY
 * for requests that don't have a body.
 *
 * @return
 *   - *`AZ_OK`* success.
 *   - *`AZ_ERROR_ARG`*
 *     - `out_request` is _NULL_.
 *     - `url`, `method`, or `headers_buffer` are invalid spans (see @ref _az_span_is_valid).
 */
AZ_NODISCARD az_result az_http_request_init(
    az_http_request* out_request,
    az_context* context,
    az_http_method method,
    az_span url,
    int32_t url_length,
    az_span headers_buffer,
    az_span body);

/**
 * @brief Set a query parameter at the end of url.
 *
 * @remark Query parameters are stored url-encoded. This function will not check if
 * the a query parameter already exists in the URL. Calling this function twice with same \p name
 * would duplicate the query parameter.
 *
 * @param[out] ref_request HTTP request that holds the URL to set the query parameter to.
 * @param[in] name URL parameter name.
 * @param[in] value URL parameter value.
 * @param[in] \p is_value_url_encoded boolean value that defines if the query parameter (name and
 * value) is url-encoded or not.
 *
 * @remarks if \p is_value_url_encoded is set to false, before setting query parameter, it would be
 * url-encoded.
 *
 * @return
 *   - *`AZ_OK`* success.
 *   - *`AZ_ERROR_NOT_ENOUGH_SPACE`* the `URL` would grow past the `max_url_size`, should
 * the parameter get set.
 *   - *`AZ_ERROR_ARG`*
 *     - `p_request` is _NULL_.
 *     - `name` or `value` are invalid spans (see @ref _az_span_is_valid).
 *     - `name` or `value` are empty.
 *     - `name`'s or `value`'s buffer overlap resulting `url`'s buffer.
 */
AZ_NODISCARD az_result az_http_request_set_query_parameter(
    az_http_request* ref_request,
    az_span name,
    az_span value,
    bool is_value_url_encoded);

/**
 * @brief Add a new HTTP header for the request.
 *
 * @param ref_request HTTP request builder that holds the URL to set the query parameter to.
 * @param name Header name (e.g. `"Content-Type"`).
 * @param value Header value (e.g. `"application/x-www-form-urlencoded"`).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE There isn't enough space in the \p ref_request to add a
 * header.
 */
AZ_NODISCARD az_result
az_http_request_append_header(az_http_request* ref_request, az_span name, az_span value);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HTTP_INTERNAL_H
