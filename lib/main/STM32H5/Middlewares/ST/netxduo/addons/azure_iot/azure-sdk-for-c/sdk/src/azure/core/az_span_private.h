// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines private implementation used by span.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_SPAN_PRIVATE_H
#define _az_SPAN_PRIVATE_H

#include <azure/core/az_precondition.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <stdbool.h>

#include <azure/core/_az_cfg_prefix.h>

// In IEEE 754, +inf is represented as 0 for the sign bit, all 1s for the biased exponent, and 0s
// for the fraction bits.
#define _az_BINARY_VALUE_OF_POSITIVE_INFINITY 0x7FF0000000000000ULL

enum
{
  _az_ASCII_LOWER_DIF = 'a' - 'A',

  // One less than the number of digits in _az_MAX_SAFE_INTEGER
  // This many digits can roundtrip between double and uint64_t without loss of precision
  // or causing integer overflow. We can't choose 16, because 9999999999999999 is larger than
  // _az_MAX_SAFE_INTEGER.
  _az_MAX_SUPPORTED_FRACTIONAL_DIGITS = 15,

  // 10 + sign (i.e. -2,147,483,648)
  _az_MAX_SIZE_FOR_INT32 = 11,

  // 19 + sign (i.e. -9,223,372,036,854,775,808)
  _az_MAX_SIZE_FOR_INT64 = 20,

  // Two digit length to create the "format" passed to sscanf.
  _az_MAX_SIZE_FOR_PARSING_DOUBLE = 99,

  // The number value of the ASCII space character ' '.
  _az_ASCII_SPACE_CHARACTER = 0x20,

  // The largest digit in base 16 (hexadecimal).
  _az_LARGEST_HEX_VALUE = 0xF,
};

/**
 * @brief A portable implementation of the standard `isfinite()` function, which may not be
 * available on certain embedded systems that use older compilers.
 *
 * @param value The 64-bit floating point value to test.
 * @return `true` if the \p value is finite (that is, it is not infinite or not a number), otherwise
 * return `false`.
 */
AZ_NODISCARD AZ_INLINE bool _az_isfinite(double value)
{
  uint64_t binary_value = 0;

  // Workaround for strict-aliasing rules.
  // Get the 8-byte binary representation of the double value, by re-interpreting it as an uint64_t.
  // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling)
  memcpy(&binary_value, &value, sizeof(binary_value));

  // These are the binary representations of the various non-finite value ranges,
  // according to the IEEE 754 standard:
  // +inf - 0x7FF0000000000000
  // -inf - 0xFFF0000000000000 Anything in the following
  // nan - 0x7FF0000000000001 to 0x7FFFFFFFFFFFFFFF and 0xFFF0000000000001 to 0xFFFFFFFFFFFFFFFF

  // This is equivalent to checking the following ranges, condensed into a single check:
  // (binary_value < 0x7FF0000000000000 ||
  //   (binary_value > 0x7FFFFFFFFFFFFFFF && binary_value < 0xFFF0000000000000))
  return (binary_value & _az_BINARY_VALUE_OF_POSITIVE_INFINITY)
      != _az_BINARY_VALUE_OF_POSITIVE_INFINITY;
}

AZ_NODISCARD az_result _az_is_expected_span(az_span* ref_span, az_span expected);

/**
 * @brief Removes all leading and trailing whitespace characters from the \p span. Function will
 * create a new #az_span pointing to the first non-whitespace (` `, \\n, \\r, \\t) character found
 * in \p span and up to the last non-whitespace character.
 *
 * @remarks If \p span is full of non-whitespace characters, this function will return empty
 * #az_span.
 *
 * Example:
 * \code{.c}
 *  az_span a = AZ_SPAN_FROM_STR("  text with   \\n spaces   ");
 *  az_span b = _az_span_trim_whitespace(a);
 *  // assert( b ==  AZ_SPAN_FROM_STR("text with   \\n spaces"));
 * \endcode
 *
 * @param[in] source #az_span pointing to a memory address that might contain whitespace characters.
 * @return The trimmed #az_span.
 */
AZ_NODISCARD az_span _az_span_trim_whitespace(az_span source);

/**
 * @brief Removes all leading whitespace characters from the start of \p span.
 * Function will create a new #az_span pointing to the first non-whitespace (` `, \\n, \\r, \\t)
 * character found in \p span and up to the last character.
 *
 * @remarks If \p span is full of non-whitespace characters, this function will return empty
 * #az_span.
 *
 * Example:
 * \code{.c}
 *  az_span a = AZ_SPAN_FROM_STR("  text with   \\n spaces   ");
 *  az_span b = _az_span_trim_whitespace_from_start(a);
 *  // assert( b ==  AZ_SPAN_FROM_STR("text with   \\n spaces   "));
 * \endcode
 *
 * @param[in] source #az_span pointing to a memory address that might contain whitespace characters.
 * @return The trimmed #az_span.
 */
AZ_NODISCARD az_span _az_span_trim_whitespace_from_start(az_span source);

/**
 * @brief Removes all trailing whitespace characters from the end of \p span.
 * Function will create a new #az_span pointing to the first character in \p span and up to the last
 * non-whitespace (` `, \\n, \\r, \\t) character.
 *
 * @remarks If \p span is full of non-whitespace characters, this function will return empty
 * #az_span.
 *
 * Example:
 * \code{.c}
 *  az_span a = AZ_SPAN_FROM_STR("  text with   \\n spaces   ");
 *  az_span b = _az_span_trim_whitespace_from_end(a);
 *  // assert( b ==  AZ_SPAN_FROM_STR("  text with   \\n spaces"));
 * \endcode
 *
 * @param[in] source #az_span pointing to a memory address that might contain whitespace characters.
 * @return The trimmed #az_span.
 */
AZ_NODISCARD az_span _az_span_trim_whitespace_from_end(az_span source);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_SPAN_PRIVATE_H
