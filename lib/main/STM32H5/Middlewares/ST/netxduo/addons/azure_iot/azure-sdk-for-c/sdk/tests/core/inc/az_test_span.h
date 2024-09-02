// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_test_span.h
 *
 * @brief Test only utilities for helping validate spans.
 */

#ifndef _az_TEST_SPAN_H
#define _az_TEST_SPAN_H

// These headers must be included prior to including cmocka.h.
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <assert.h>
#include <cmocka.h>
#include <stdint.h>

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdio.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Returns a span over a byte buffer, directly invoking #az_span_create. The buffer is
 * initialized with `0xCC` to help tests check against buffer overflow.
 *
 * @param[in] ptr The memory address of the 1st byte in the byte buffer.
 * @param[in] size The number of total bytes in the byte buffer.
 *
 * @return #az_span The "view" over the byte buffer, with the buffer filled with `0xCC`.
 */
AZ_NODISCARD AZ_INLINE az_span az_span_for_test_init(uint8_t* ptr, int32_t size)
{
  az_span new_span = az_span_create(ptr, size);
  az_span_fill(new_span, 0xcc);
  return new_span;
}

/**
 * @brief Verifies that the span has been correctly set during the test.
 * In addition to memcmp of expected/actual data, it also checks against buffer overflow.
 *
 * The function will assert on any unexpected results.
 *
 * @param[in] result_span Span that has the result of the test run, which should be a slice of the
 * original span.
 * @param[in] buffer_expected Buffer that contains expected results of the test and that result_span
 * will match on success.
 * @param[in] result_size_expected The expected size of result_span.
 * @param[in] original_span The span around the entire input buffer which is use to verify only the
 * resulting slice got updated, while the rest retain their original sentinel values.
 * @param[in] original_size_expected The expected size of original span, which should remain
 * unchanged.
 */
AZ_INLINE void az_span_for_test_verify(
    az_span result_span,
    const void* const buffer_expected,
    int32_t result_size_expected,
    az_span original_span,
    int32_t original_size_expected)
{
  assert_int_equal(az_span_size(result_span), result_size_expected);
  assert_int_equal(az_span_size(original_span), original_size_expected);
  assert_memory_equal(
      az_span_ptr(result_span), (size_t)buffer_expected, (size_t)result_size_expected);
  assert_memory_equal(
      az_span_ptr(original_span), (size_t)buffer_expected, (size_t)result_size_expected);

  for (int32_t i = result_size_expected; i < original_size_expected; i++)
  {
    assert_true(*(uint8_t*)(az_span_ptr(original_span) + i) == 0xcc);
  }
}

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_TEST_SPAN_H
