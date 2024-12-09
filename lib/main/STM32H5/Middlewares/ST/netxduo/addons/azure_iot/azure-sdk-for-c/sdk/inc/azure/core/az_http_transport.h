// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Utilities to be used by HTTP transport policy implementations.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HTTP_TRANSPORT_H
#define _az_HTTP_TRANSPORT_H

#include <azure/core/az_http.h>
#include <azure/core/az_span.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief A type representing an HTTP method (`POST`, `PUT`, `GET`, `DELETE`, etc.).
 */
typedef az_span az_http_method;

/**
 * @brief HTTP GET method name.
 */
AZ_INLINE az_http_method az_http_method_get() { return AZ_SPAN_FROM_STR("GET"); }

/**
 * @brief HTTP HEAD method name.
 */
AZ_INLINE az_http_method az_http_method_head() { return AZ_SPAN_FROM_STR("HEAD"); }

/**
 * @brief HTTP POST method name.
 */
AZ_INLINE az_http_method az_http_method_post() { return AZ_SPAN_FROM_STR("POST"); }

/**
 * @brief HTTP PUT method name.
 */
AZ_INLINE az_http_method az_http_method_put() { return AZ_SPAN_FROM_STR("PUT"); }

/**
 * @brief HTTP DELETE method name.
 */
AZ_INLINE az_http_method az_http_method_delete() { return AZ_SPAN_FROM_STR("DELETE"); }

/**
 * @brief HTTP PATCH method name.
 */
AZ_INLINE az_http_method az_http_method_patch() { return AZ_SPAN_FROM_STR("PATCH"); }

/**
 * @brief Represents a name/value pair of #az_span instances.
 */
typedef struct
{
  az_span name; ///< Name.
  az_span value; ///< Value.
} _az_http_request_header;

/**
 * @brief A type representing a buffer of #_az_http_request_header instances for HTTP request
 * headers.
 */
typedef az_span _az_http_request_headers;

/**
 * @brief Structure used to represent an HTTP request.
 * It contains an HTTP method, URL, headers and body. It also contains
 * another utility variables.
 */
typedef struct
{
  struct
  {
    az_context* context;
    az_http_method method;
    az_span url;
    int32_t url_length;
    int32_t query_start;
    _az_http_request_headers headers; // Contains instances of _az_http_request_header
    int32_t headers_length;
    int32_t max_headers;
    int32_t retry_headers_start_byte_offset;
    az_span body;
  } _internal;
} az_http_request;

/**
 * @brief Used to declare policy process callback #_az_http_policy_process_fn definition.
 */
// Definition is below.
typedef struct _az_http_policy _az_http_policy;

/**
 * @brief Defines the callback signature of a policy process which should receive an
 * #_az_http_policy, options reference (as `void*`), an #az_http_request and an #az_http_response.
 *
 * @remark `void*` is used as polymorphic solution for any policy. Each policy implementation would
 * know the specific pointer type to cast options to.
 */
typedef AZ_NODISCARD az_result (*_az_http_policy_process_fn)(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

/**
 * @brief HTTP policy.
 * An HTTP pipeline inside SDK clients is an array of HTTP policies.
 */
struct _az_http_policy
{
  struct
  {
    _az_http_policy_process_fn process;
    void* options;
  } _internal;
};

/**
 * @brief Gets the HTTP header by index.
 *
 * @param[in] request HTTP request to get HTTP header from.
 * @param[in] index Index of the HTTP header to get.
 * @param[out] out_name A pointer to an #az_span to write the header's name.
 * @param[out] out_value A pointer to an #az_span to write the header's value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_ARG \p index is out of range.
 */
AZ_NODISCARD az_result az_http_request_get_header(
    az_http_request const* request,
    int32_t index,
    az_span* out_name,
    az_span* out_value);

/**
 * @brief Get method of an HTTP request.
 *
 * @remarks This function is expected to be used by transport layer only.
 *
 * @param[in] request The HTTP request from which to get the method.
 * @param[out] out_method Pointer to write the HTTP method to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval other Failure.
 */
AZ_NODISCARD az_result
az_http_request_get_method(az_http_request const* request, az_http_method* out_method);

/**
 * @brief Get the URL from an HTTP request.
 *
 * @remarks This function is expected to be used by transport layer only.
 *
 * @param[in] request The HTTP request from which to get the URL.
 * @param[out] out_url Pointer to write the HTTP URL to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval other Failure.
 */
AZ_NODISCARD az_result az_http_request_get_url(az_http_request const* request, az_span* out_url);

/**
 * @brief Get body from an HTTP request.
 *
 * @remarks This function is expected to be used by transport layer only.
 *
 * @param[in] request The HTTP request from which to get the body.
 * @param[out] out_body Pointer to write the HTTP request body to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval other Failure.
 */
AZ_NODISCARD az_result az_http_request_get_body(az_http_request const* request, az_span* out_body);

/**
 * @brief This function is expected to be used by transport adapters like curl. Use it to write
 * content from \p source to \p ref_response.
 *
 * @remarks The \p source can be an empty #az_span. If so, nothing will be written.
 *
 * @param[in,out] ref_response Pointer to an #az_http_response.
 * @param[in] source This is an #az_span with the content to be written into \p ref_response.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The \p response buffer is not big enough to contain the \p
 * source content.
 */
AZ_NODISCARD az_result az_http_response_append(az_http_response* ref_response, az_span source);

/**
 * @brief Returns the number of headers within the request.
 *
 * @param[in] request Pointer to an #az_http_request to be used by this function.
 *
 * @return Number of headers in the \p request.
 */
AZ_NODISCARD int32_t az_http_request_headers_count(az_http_request const* request);

/**
 * @brief Sends an HTTP request through the wire and write the response into \p ref_response.
 *
 * @param[in] request Points to an #az_http_request that contains the settings and data that is
 * used to send the request through the wire.
 * @param[in,out] ref_response Points to an #az_http_response where the response from the wire will
 * be written.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_HTTP_RESPONSE_OVERFLOW There was an issue while trying to write into \p
 * ref_response. It might mean that there was not enough space in \p ref_response to hold the entire
 * response from the network.
 * @retval #AZ_ERROR_HTTP_RESPONSE_COULDNT_RESOLVE_HOST The URL from \p ref_request can't be
 * resolved by the HTTP stack and the request was not sent.
 * @retval #AZ_ERROR_HTTP_ADAPTER Any other issue from the transport adapter layer.
 * @retval #AZ_ERROR_DEPENDENCY_NOT_PROVIDED No platform implementation was supplied to support this
 * function.
 */
AZ_NODISCARD az_result
az_http_client_send_request(az_http_request const* request, az_http_response* ref_response);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HTTP_TRANSPORT_H
