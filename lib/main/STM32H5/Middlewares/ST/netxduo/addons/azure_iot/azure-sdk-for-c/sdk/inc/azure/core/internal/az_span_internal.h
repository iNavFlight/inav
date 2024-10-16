// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines internals used by span.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_SPAN_INTERNAL_H
#define _az_SPAN_INTERNAL_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

// The smallest number that has the same number of digits as _az_MAX_SIZE_FOR_UINT64 (i.e. 10^19).
#define _az_SMALLEST_20_DIGIT_NUMBER 10000000000000000000ULL

enum
{
  // For example: 2,147,483,648
  _az_MAX_SIZE_FOR_UINT32 = 10,

  // For example: 18,446,744,073,709,551,615
  _az_MAX_SIZE_FOR_UINT64 = 20,

  // The number of unique values in base 10 (decimal).
  _az_NUMBER_OF_DECIMAL_VALUES = 10,

  // The smallest number that has the same number of digits as _az_MAX_SIZE_FOR_UINT32 (i.e. 10^9).
  _az_SMALLEST_10_DIGIT_NUMBER = 1000000000,
};

// Use this helper to figure out how much the sliced_span has moved in comparison to the
// original_span while writing and slicing a copy of the original.
// The \p sliced_span must be some slice of the \p original_span (and have the same backing memory).
AZ_INLINE AZ_NODISCARD int32_t _az_span_diff(az_span sliced_span, az_span original_span)
{
  int32_t answer = az_span_size(original_span) - az_span_size(sliced_span);

  // The passed in span parameters cannot be any two arbitrary spans.
  // This validates the span parameters are valid and one is a sub-slice of another.
  _az_PRECONDITION(answer == (int32_t)(az_span_ptr(sliced_span) - az_span_ptr(original_span)));
  return answer;
}

/**
 * @brief Copies character from the \p source #az_span to the \p destination #az_span by
 * URL-encoding the \p source span characters.
 *
 * @param destination The #az_span whose bytes will receive the URL-encoded \p source.
 * @param[in] source The #az_span containing the non-URL-encoded bytes.
 * @param[out] out_length A pointer to an int32_t that is going to be assigned the length
 * of URL-encoding the \p source.
 * @return An #az_result value indicating the result of the operation:
 *         - #AZ_OK if successful
 *         - #AZ_ERROR_NOT_ENOUGH_SPACE if the \p destination is not big enough to contain the
 * encoded bytes
 *
 * @remark If \p destination can't fit the \p source, some data may still be written to it, but the
 * \p out_length will be set to 0, and the function will return #AZ_ERROR_NOT_ENOUGH_SPACE.
 * @remark The \p destination and \p source must not overlap.
 */
AZ_NODISCARD az_result
_az_span_url_encode(az_span destination, az_span source, int32_t* out_length);

/**
 * @brief Calculates what would be the length of \p source #az_span after url-encoding it.
 *
 * @param[in] source The #az_span containing the non-URL-encoded bytes.
 * @return The length of source if it would be url-encoded.
 *
 */
AZ_NODISCARD int32_t _az_span_url_encode_calc_length(az_span source);

/**
 * @brief String tokenizer for #az_span.
 *
 * @param[in] source The #az_span with the content to be searched on. It must be a non-empty
 * #az_span.
 * @param[in] delimiter The #az_span containing the delimiter to "split" `source` into tokens.  It
 * must be a non-empty #az_span.
 * @param[out] out_remainder The #az_span pointing to the remaining bytes in `source`, starting
 * after the occurrence of `delimiter`. If the position after `delimiter` is the end of `source`,
 * `out_remainder` is set to an empty #az_span.
 * @param[out] out_index The position of \p delimiter in \p source if \p source contains the \p
 * delimiter within it. Otherwise, it is set to -1.
 *
 * @return The #az_span pointing to the token delimited by the beginning of `source` up to the first
 * occurrence of (but not including the) `delimiter`, or the end of `source` if `delimiter` is not
 * found. If `source` is empty, #AZ_SPAN_EMPTY is returned instead.
 */
az_span _az_span_token(
    az_span source,
    az_span delimiter,
    az_span* out_remainder,
    int32_t* out_index);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_SPAN_INTERNAL_H
