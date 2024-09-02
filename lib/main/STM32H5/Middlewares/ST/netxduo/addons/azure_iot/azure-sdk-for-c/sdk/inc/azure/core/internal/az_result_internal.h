// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Definition of #az_result related internal helper functions.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_RESULT_INTERNAL_H
#define _az_RESULT_INTERNAL_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Convenience macro to return if an operation failed.
 */
#define _az_RETURN_IF_FAILED(exp)       \
  do                                    \
  {                                     \
    az_result const _az_result = (exp); \
    if (az_result_failed(_az_result))   \
    {                                   \
      return _az_result;                \
    }                                   \
  } while (0)

/**
 * @brief Convenience macro to return if the provided span is not of the expected, required size.
 */
#define _az_RETURN_IF_NOT_ENOUGH_SIZE(span, required_size) \
  do                                                       \
  {                                                        \
    int32_t const _az_req_sz = (required_size);            \
    if (az_span_size(span) < _az_req_sz || _az_req_sz < 0) \
    {                                                      \
      return AZ_ERROR_NOT_ENOUGH_SPACE;                    \
    }                                                      \
  } while (0)

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_RESULT_INTERNAL_H
