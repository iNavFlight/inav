// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines private implementation used by http.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HTTP_PRIVATE_H
#define _az_HTTP_PRIVATE_H

#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_precondition.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <stdbool.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Mark that the HTTP headers that are gong to be added via
 * `az_http_request_append_header` are going to be considered as retry headers.
 *
 * @param ref_request HTTP request.
 *
 * @return
 *   - *`AZ_OK`* success.
 *   - *`AZ_ERROR_ARG`* `ref_request` is _NULL_.
 */
AZ_NODISCARD AZ_INLINE az_result
_az_http_request_mark_retry_headers_start(az_http_request* ref_request)
{
  _az_PRECONDITION_NOT_NULL(ref_request);
  ref_request->_internal.retry_headers_start_byte_offset
      = ref_request->_internal.headers_length * (int32_t)sizeof(_az_http_request_header);
  return AZ_OK;
}

AZ_NODISCARD AZ_INLINE az_result _az_http_request_remove_retry_headers(az_http_request* ref_request)
{
  _az_PRECONDITION_NOT_NULL(ref_request);
  ref_request->_internal.headers_length = ref_request->_internal.retry_headers_start_byte_offset
      / (int32_t)sizeof(_az_http_request_header);
  return AZ_OK;
}

/**
 * @brief Sets buffer and parser to its initial state.
 *
 */
void _az_http_response_reset(az_http_response* ref_response);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HTTP_PRIVATE_H
