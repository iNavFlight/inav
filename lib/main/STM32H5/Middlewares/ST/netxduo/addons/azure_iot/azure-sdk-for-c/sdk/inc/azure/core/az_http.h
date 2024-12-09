// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief This header defines the types and functions your application uses to leverage HTTP request
 * and response functionality.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HTTP_H
#define _az_HTTP_H

#include <azure/core/az_config.h>
#include <azure/core/az_context.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Defines the possible HTTP status codes.
 */
typedef enum
{
  /// No HTTP status code.
  AZ_HTTP_STATUS_CODE_NONE = 0,

  // === 1xx (information) Status Codes: ===
  AZ_HTTP_STATUS_CODE_CONTINUE = 100, ///< HTTP 100 Continue.
  AZ_HTTP_STATUS_CODE_SWITCHING_PROTOCOLS = 101, ///< HTTP 101 Switching Protocols.
  AZ_HTTP_STATUS_CODE_PROCESSING = 102, ///< HTTP 102 Processing.
  AZ_HTTP_STATUS_CODE_EARLY_HINTS = 103, ///< HTTP 103 Early Hints.

  // === 2xx (successful) Status Codes: ===
  AZ_HTTP_STATUS_CODE_OK = 200, ///< HTTP 200 OK.
  AZ_HTTP_STATUS_CODE_CREATED = 201, ///< HTTP 201 Created.
  AZ_HTTP_STATUS_CODE_ACCEPTED = 202, ///< HTTP 202 Accepted.
  AZ_HTTP_STATUS_CODE_NON_AUTHORITATIVE_INFORMATION
  = 203, ///< HTTP 203 Non-Authoritative Information.
  AZ_HTTP_STATUS_CODE_NO_CONTENT = 204, ///< HTTP 204 No Content.
  AZ_HTTP_STATUS_CODE_RESET_CONTENT = 205, ///< HTTP 205 Rest Content.
  AZ_HTTP_STATUS_CODE_PARTIAL_CONTENT = 206, ///< HTTP 206 Partial Content.
  AZ_HTTP_STATUS_CODE_MULTI_STATUS = 207, ///< HTTP 207 Multi-Status.
  AZ_HTTP_STATUS_CODE_ALREADY_REPORTED = 208, ///< HTTP 208 Already Reported.
  AZ_HTTP_STATUS_CODE_IM_USED = 226, ///< HTTP 226 IM Used.

  // === 3xx (redirection) Status Codes: ===
  AZ_HTTP_STATUS_CODE_MULTIPLE_CHOICES = 300, ///< HTTP 300 Multiple Choices.
  AZ_HTTP_STATUS_CODE_MOVED_PERMANENTLY = 301, ///< HTTP 301 Moved Permanently.
  AZ_HTTP_STATUS_CODE_FOUND = 302, ///< HTTP 302 Found.
  AZ_HTTP_STATUS_CODE_SEE_OTHER = 303, ///< HTTP 303 See Other.
  AZ_HTTP_STATUS_CODE_NOT_MODIFIED = 304, ///< HTTP 304 Not Modified.
  AZ_HTTP_STATUS_CODE_USE_PROXY = 305, ///< HTTP 305 Use Proxy.
  AZ_HTTP_STATUS_CODE_TEMPORARY_REDIRECT = 307, ///< HTTP 307 Temporary Redirect.
  AZ_HTTP_STATUS_CODE_PERMANENT_REDIRECT = 308, ///< HTTP 308 Permanent Redirect.

  // === 4xx (client error) Status Codes: ===
  AZ_HTTP_STATUS_CODE_BAD_REQUEST = 400, ///< HTTP 400 Bad Request.
  AZ_HTTP_STATUS_CODE_UNAUTHORIZED = 401, ///< HTTP 401 Unauthorized.
  AZ_HTTP_STATUS_CODE_PAYMENT_REQUIRED = 402, ///< HTTP 402 Payment Required.
  AZ_HTTP_STATUS_CODE_FORBIDDEN = 403, ///< HTTP 403 Forbidden.
  AZ_HTTP_STATUS_CODE_NOT_FOUND = 404, ///< HTTP 404 Not Found.
  AZ_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED = 405, ///< HTTP 405 Method Not Allowed.
  AZ_HTTP_STATUS_CODE_NOT_ACCEPTABLE = 406, ///< HTTP 406 Not Acceptable.
  AZ_HTTP_STATUS_CODE_PROXY_AUTHENTICATION_REQUIRED
  = 407, ///< HTTP 407 Proxy Authentication Required.
  AZ_HTTP_STATUS_CODE_REQUEST_TIMEOUT = 408, ///< HTTP 408 Request Timeout.
  AZ_HTTP_STATUS_CODE_CONFLICT = 409, ///< HTTP 409 Conflict.
  AZ_HTTP_STATUS_CODE_GONE = 410, ///< HTTP 410 Gone.
  AZ_HTTP_STATUS_CODE_LENGTH_REQUIRED = 411, ///< HTTP 411 Length Required.
  AZ_HTTP_STATUS_CODE_PRECONDITION_FAILED = 412, ///< HTTP 412 Precondition Failed.
  AZ_HTTP_STATUS_CODE_PAYLOAD_TOO_LARGE = 413, ///< HTTP 413 Payload Too Large.
  AZ_HTTP_STATUS_CODE_URI_TOO_LONG = 414, ///< HTTP 414 URI Too Long.
  AZ_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA_TYPE = 415, ///< HTTP 415 Unsupported Media Type.
  AZ_HTTP_STATUS_CODE_RANGE_NOT_SATISFIABLE = 416, ///< HTTP 416 Range Not Satisfiable.
  AZ_HTTP_STATUS_CODE_EXPECTATION_FAILED = 417, ///< HTTP 417 Expectation Failed.
  AZ_HTTP_STATUS_CODE_MISDIRECTED_REQUEST = 421, ///< HTTP 421 Misdirected Request.
  AZ_HTTP_STATUS_CODE_UNPROCESSABLE_ENTITY = 422, ///< HTTP 422 Unprocessable Entity.
  AZ_HTTP_STATUS_CODE_LOCKED = 423, ///< HTTP 423 Locked.
  AZ_HTTP_STATUS_CODE_FAILED_DEPENDENCY = 424, ///< HTTP 424 Failed Dependency.
  AZ_HTTP_STATUS_CODE_TOO_EARLY = 425, ///< HTTP 425 Too Early.
  AZ_HTTP_STATUS_CODE_UPGRADE_REQUIRED = 426, ///< HTTP 426 Upgrade Required.
  AZ_HTTP_STATUS_CODE_PRECONDITION_REQUIRED = 428, ///< HTTP 428 Precondition Required.
  AZ_HTTP_STATUS_CODE_TOO_MANY_REQUESTS = 429, ///< HTTP 429 Too Many Requests.
  AZ_HTTP_STATUS_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE
  = 431, ///< HTTP 431 Request Header Fields Too Large.
  AZ_HTTP_STATUS_CODE_UNAVAILABLE_FOR_LEGAL_REASONS
  = 451, ///< HTTP 451 Unavailable For Legal Reasons.

  // === 5xx (server error) Status Codes: ===
  AZ_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR = 500, ///< HTTP 500 Internal Server Error.
  AZ_HTTP_STATUS_CODE_NOT_IMPLEMENTED = 501, ///< HTTP 501 Not Implemented.
  AZ_HTTP_STATUS_CODE_BAD_GATEWAY = 502, ///< HTTP 502 Bad Gateway.
  AZ_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE = 503, ///< HTTP 503 Unavailable.
  AZ_HTTP_STATUS_CODE_GATEWAY_TIMEOUT = 504, ///< HTTP 504 Gateway Timeout.
  AZ_HTTP_STATUS_CODE_HTTP_VERSION_NOT_SUPPORTED = 505, ///< HTTP 505 HTTP Version Not Supported.
  AZ_HTTP_STATUS_CODE_VARIANT_ALSO_NEGOTIATES = 506, ///< HTTP 506 Variant Also Negotiates.
  AZ_HTTP_STATUS_CODE_INSUFFICIENT_STORAGE = 507, ///< HTTP 507 Insufficient Storage.
  AZ_HTTP_STATUS_CODE_LOOP_DETECTED = 508, ///< HTTP 508 Loop Detected.
  AZ_HTTP_STATUS_CODE_NOT_EXTENDED = 510, ///< HTTP 510 Not Extended.
  AZ_HTTP_STATUS_CODE_NETWORK_AUTHENTICATION_REQUIRED
  = 511, ///< HTTP 511 Network Authentication Required.
} az_http_status_code;

/**
 * @brief Allows you to customize the retry policy used by SDK clients whenever they perform an I/O
 * operation.
 *
 * @details Client libraries should acquire an initialized instance of this struct and then modify
 * any fields necessary before passing a pointer to this struct when initializing the specific
 * client.
 */
typedef struct
{
  /// The minimum time, in milliseconds, to wait before a retry.
  int32_t retry_delay_msec;

  /// The maximum time, in milliseconds, to wait before a retry.
  int32_t max_retry_delay_msec;

  /// Maximum number of retries.
  int32_t max_retries;
} az_http_policy_retry_options;

typedef enum
{
  _az_HTTP_RESPONSE_KIND_STATUS_LINE = 0,
  _az_HTTP_RESPONSE_KIND_HEADER = 1,
  _az_HTTP_RESPONSE_KIND_BODY = 2,
  _az_HTTP_RESPONSE_KIND_EOF = 3,
} _az_http_response_kind;

/**
 * @brief Allows you to parse an HTTP response's status line, headers, and body.
 *
 * @details Users create an instance of this and pass it in to an Azure service client's operation
 * function. The function initializes the #az_http_response and application code can query the
 * response after the operation completes by calling the #az_http_response_get_status_line(),
 * #az_http_response_get_next_header() and #az_http_response_get_body() functions.
 */
typedef struct
{
  struct
  {
    az_span http_response;
    int32_t written;
    struct
    {
      az_span remaining; // the remaining un-parsed portion of the original http_response.
      _az_http_response_kind next_kind;
      // After parsing an element, next_kind refers to the next expected element
    } parser;
  } _internal;
} az_http_response;

/**
 * @brief Initializes an #az_http_response instance over a byte buffer (span) which will be filled
 * with the HTTP response data as it comes in from the network.
 *
 * @param[out] out_response The pointer to an #az_http_response instance which is to be initialized.
 * @param[in] buffer A span over the byte buffer that is to be filled with the HTTP response data.
 * This buffer must be large enough to hold the entire response.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval other Initialization failed.
 */
AZ_NODISCARD AZ_INLINE az_result
az_http_response_init(az_http_response* out_response, az_span buffer)
{
  *out_response = (az_http_response){
    ._internal = {
      .http_response = buffer,
      .written = 0,
      .parser = {
        .remaining = AZ_SPAN_EMPTY,
        .next_kind = _az_HTTP_RESPONSE_KIND_STATUS_LINE,
      },
    },
  };

  return AZ_OK;
}

/**
 * @brief Represents the result of making an HTTP request.
 * An application obtains this initialized structure by calling #az_http_response_get_status_line().
 *
 * @see https://tools.ietf.org/html/rfc7230#section-3.1.2
 */
// Member order is optimized for alignment.
typedef struct
{
  az_span reason_phrase; ///< Reason Phrase.
  az_http_status_code status_code; ///< Status Code.
  uint8_t major_version; ///< HTTP Major Version.
  uint8_t minor_version; ///< HTTP Minor Version.
} az_http_response_status_line;

/**
 * @brief Returns the #az_http_response_status_line information within an HTTP response.
 *
 * @param[in,out] ref_response The #az_http_response with an HTTP response.
 * @param[out] out_status_line The pointer to an #az_http_response_status_line structure to be
 * filled in by this function.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Response status line was parsed to \p out_status_line.
 * @retval other HTTP response was not parsed.
 */
AZ_NODISCARD az_result az_http_response_get_status_line(
    az_http_response* ref_response,
    az_http_response_status_line* out_status_line);

/**
 * @brief Returns the #az_http_status_code information from an HTTP response.
 * @details Invokes #az_http_response_get_status_line(), so it advances the reading position
 * accordingly.
 *
 * @remark Use this function when HTTP Reason Phrase is not needed, and when it is not important for
 * the invoking code to distinguish between #AZ_HTTP_STATUS_CODE_NONE and any possible error when
 * parsing an HTTP response.
 *
 * @param[in,out] ref_response The #az_http_response with an HTTP response.
 *
 * @return An HTTP status code.
 */
AZ_INLINE AZ_NODISCARD az_http_status_code
az_http_response_get_status_code(az_http_response* ref_response)
{
  az_http_response_status_line status_line = { 0 };
  return az_result_failed(az_http_response_get_status_line(ref_response, &status_line))
      ? AZ_HTTP_STATUS_CODE_NONE
      : status_line.status_code;
}

/**
 * @brief Returns the next HTTP response header.
 *
 * @details
 * When called right after #az_http_response_get_status_line(), or after
 * #az_http_response_get_status_code(), this function returns the first header. When called after
 * calling #az_http_response_get_next_header(), this function returns the next header.
 *
 * If called after parsing HTTP body or before parsing status line, this function
 * will return #AZ_ERROR_HTTP_INVALID_STATE.
 *
 * @param[in,out] ref_response A pointer to an #az_http_response instance.
 * @param[out] out_name A pointer to an #az_span to receive the header's name.
 * @param[out] out_value A pointer to an #az_span to receive the header's value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK A header was returned.
 * @retval #AZ_ERROR_HTTP_END_OF_HEADERS There are no more headers within the HTTP response payload.
 * @retval #AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER The HTTP response contains an unexpected invalid
 * character or is incomplete.
 * @retval #AZ_ERROR_HTTP_INVALID_STATE The #az_http_response instance is in an invalid state.
 * Consider calling #az_http_response_get_status_line() to reset its state.
 */
AZ_NODISCARD az_result az_http_response_get_next_header(
    az_http_response* ref_response,
    az_span* out_name,
    az_span* out_value);

/**
 * @brief Returns a span over the HTTP body within an HTTP response.
 *
 * @param[in,out] ref_response A pointer to an #az_http_response instance.
 * @param[out] out_body A pointer to an #az_span to receive the HTTP response's body.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK An #az_span over the response body was returned.
 * @retval other Error while trying to read and parse body.
 */
AZ_NODISCARD az_result az_http_response_get_body(az_http_response* ref_response, az_span* out_body);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HTTP_H
