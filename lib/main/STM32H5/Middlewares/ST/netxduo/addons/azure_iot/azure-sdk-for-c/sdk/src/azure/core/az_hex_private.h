// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines private implementation used by hex.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HEX_PRIVATE_H
#define _az_HEX_PRIVATE_H

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

enum
{
  _az_HEX_LOWER_OFFSET = 'a' - 10,
  _az_HEX_UPPER_OFFSET = 'A' - 10,
};

/**
 * Converts a number [0..15] into uppercase hexadecimal digit character (base16).
 */
AZ_NODISCARD AZ_INLINE uint8_t _az_number_to_upper_hex(uint8_t number)
{
  // The 10 is one more than the last non-alphabetic digit of the hex values (0-9, A-F).
  // Every value under 10 is a single digit, whereas values 10 and above are hex letters.

  // NOLINTNEXTLINE(readability-magic-numbers, cppcoreguidelines-avoid-magic-numbers)
  return (uint8_t)(number + (number < 10 ? '0' : _az_HEX_UPPER_OFFSET));
}

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HEX_PRIVATE_H
