// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines platform-specific functionality used by the Azure SDK.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_PLATFORM_H
#define _az_PLATFORM_H

#include <azure/core/az_result.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Gets the platform clock in milliseconds.
 *
 * @remark The moment of time where clock starts is undefined, but if this function is getting
 * called twice with one second interval, the difference between the values returned should be equal
 * to 1000.
 *
 * @param[out] out_clock_msec Platform clock in milliseconds.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_DEPENDENCY_NOT_PROVIDED No platform implementation was supplied to support this
 * function.
 */
AZ_NODISCARD az_result az_platform_clock_msec(int64_t* out_clock_msec);

/**
 * @brief Tells the platform to sleep for a given number of milliseconds.
 *
 * @param[in] milliseconds Number of milliseconds to sleep.
 *
 * @remarks The behavior is undefined when \p milliseconds is a non-positive value (0 or less than
 * 0).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Success.
 * @retval #AZ_ERROR_DEPENDENCY_NOT_PROVIDED No platform implementation was supplied to support this
 * function.
 */
AZ_NODISCARD az_result az_platform_sleep_msec(int32_t milliseconds);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_PLATFORM_H
