// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines internals used by retry.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_RETRY_INTERNAL_H
#define _az_RETRY_INTERNAL_H

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

AZ_NODISCARD AZ_INLINE int32_t
_az_retry_calc_delay(int32_t attempt, int32_t retry_delay_msec, int32_t max_retry_delay_msec)
{
  // scale exponentially
  int32_t const exponential_retry_after
      = retry_delay_msec * (attempt <= 30 ? (int32_t)(1U << (uint32_t)attempt) : INT32_MAX);

  return exponential_retry_after > max_retry_delay_msec ? max_retry_delay_msec
                                                        : exponential_retry_after;
}

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_RETRY_INTERNAL_H
