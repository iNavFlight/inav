// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines internal constants.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_CONFIG_INTERNAL_H
#define _az_CONFIG_INTERNAL_H

#include <azure/core/az_config.h>
#include <azure/core/az_span.h>

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

enum
{
  _az_TIME_SECONDS_PER_MINUTE = 60,
  _az_TIME_MILLISECONDS_PER_SECOND = 1000,
  _az_TIME_MICROSECONDS_PER_MILLISECOND = 1000,
};

/*
 *  Int64 is max value 9223372036854775808   (19 characters)
 *           min value -9223372036854775808  (20 characters)
 */
enum
{
  _az_INT64_AS_STR_BUFFER_SIZE = 20,
};

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_CONFIG_INTERNAL_H
