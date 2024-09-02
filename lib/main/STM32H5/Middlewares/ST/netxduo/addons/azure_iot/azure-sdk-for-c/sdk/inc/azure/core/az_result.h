// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Definition of #az_result and helper functions.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_RESULT_H
#define _az_RESULT_H

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

enum
{
  _az_FACILITY_CORE = 0x1,
  _az_FACILITY_CORE_PLATFORM = 0x2,
  _az_FACILITY_CORE_JSON = 0x3,
  _az_FACILITY_CORE_HTTP = 0x4,
  _az_FACILITY_IOT = 0x5,
  _az_FACILITY_IOT_MQTT = 0x6,
  _az_FACILITY_ULIB = 0x7,
};

enum
{
  _az_ERROR_FLAG = (int32_t)0x80000000,
};

/**
 * @brief The type represents the various success and error conditions.
 *
 * @note See the following `az_result` values from various headers:
 * - #az_result_core
 * - #az_result_iot
 */
typedef int32_t az_result;

// az_result Bits:
//   - 31 Severity (0 - success, 1 - failure).
//   - 16..30 Facility.
//   - 0..15 Code.

#define _az_RESULT_MAKE_ERROR(facility, code) \
  ((az_result)((uint32_t)_az_ERROR_FLAG | ((uint32_t)(facility) << 16U) | (uint32_t)(code)))

#define _az_RESULT_MAKE_SUCCESS(facility, code) \
  ((az_result)(((uint32_t)(facility) << 16U) | (uint32_t)(code)))

/**
 * @brief The type represents the various #az_result success and error conditions specific to SDK
 * Core.
 */
enum az_result_core
{
  // === Core: Success results ====
  /// Success.
  AZ_OK = _az_RESULT_MAKE_SUCCESS(_az_FACILITY_CORE, 0),

  // === Core: Error results ===
  /// A context was canceled, and a function had to return before result was ready.
  AZ_ERROR_CANCELED = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 0),

  /// Input argument does not comply with the expected range of values.
  AZ_ERROR_ARG = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 1),

  /// The destination size is too small for the operation.
  AZ_ERROR_NOT_ENOUGH_SPACE = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 2),

  /// Requested functionality is not implemented.
  AZ_ERROR_NOT_IMPLEMENTED = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 3),

  /// Requested item was not found.
  AZ_ERROR_ITEM_NOT_FOUND = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 4),

  /// Input can't be successfully parsed.
  AZ_ERROR_UNEXPECTED_CHAR = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 5),

  /// Unexpected end of the input data.
  AZ_ERROR_UNEXPECTED_END = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 6),

  /// Not supported.
  AZ_ERROR_NOT_SUPPORTED = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 7),

  /// An external dependency required to perform the operation was not provided. The operation needs
  /// an implementation of the platform layer or an HTTP transport adapter.
  AZ_ERROR_DEPENDENCY_NOT_PROVIDED = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE, 8),

  // === Platform ===
  /// Dynamic memory allocation request was not successful.
  AZ_ERROR_OUT_OF_MEMORY = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_PLATFORM, 1),

  // === JSON error codes ===
  /// The kind of the token being read is not compatible with the expected type of the value.
  AZ_ERROR_JSON_INVALID_STATE = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_JSON, 1),

  /// The JSON depth is too large.
  AZ_ERROR_JSON_NESTING_OVERFLOW = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_JSON, 2),

  /// No more JSON text left to process.
  AZ_ERROR_JSON_READER_DONE = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_JSON, 3),

  // === HTTP error codes ===
  /// The #az_http_response instance is in an invalid state.
  AZ_ERROR_HTTP_INVALID_STATE = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 1),

  /// HTTP pipeline is malformed.
  AZ_ERROR_HTTP_PIPELINE_INVALID_POLICY = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 2),

  /// Unknown HTTP method verb.
  AZ_ERROR_HTTP_INVALID_METHOD_VERB = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 3),

  /// Authentication failed.
  AZ_ERROR_HTTP_AUTHENTICATION_FAILED = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 4),

  /// HTTP response overflow.
  AZ_ERROR_HTTP_RESPONSE_OVERFLOW = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 5),

  /// Couldn't resolve host.
  AZ_ERROR_HTTP_RESPONSE_COULDNT_RESOLVE_HOST = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 6),

  /// Error while parsing HTTP response header.
  AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 7),

  /// There are no more headers within the HTTP response payload.
  AZ_ERROR_HTTP_END_OF_HEADERS = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 8),

  // === HTTP Adapter error codes ===
  /// Generic error in the HTTP transport adapter implementation.
  AZ_ERROR_HTTP_ADAPTER = _az_RESULT_MAKE_ERROR(_az_FACILITY_CORE_HTTP, 9),
};

/**
 * @brief Checks whether the \p result provided indicates a failure.
 *
 * @param[in] result Result value to check for failure.
 *
 * @return `true` if the operation that returned this \p result failed, otherwise return `false`.
 */
AZ_NODISCARD AZ_INLINE bool az_result_failed(az_result result)
{
  return ((uint32_t)result & (uint32_t)_az_ERROR_FLAG) != 0;
}

/**
 * @brief Checks whether the \p result provided indicates a success.
 *
 * @param[in] result Result value to check for success.
 *
 * @return `true` if the operation that returned this \p result was successful, otherwise return
 * `false`.
 */
AZ_NODISCARD AZ_INLINE bool az_result_succeeded(az_result result)
{
  return !az_result_failed(result);
}

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_RESULT_H
