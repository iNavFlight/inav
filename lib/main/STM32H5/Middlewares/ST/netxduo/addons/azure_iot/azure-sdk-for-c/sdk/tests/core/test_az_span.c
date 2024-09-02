// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_span_private.h"
#include "az_test_definitions.h"
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <stdarg.h>
#include <stddef.h>

#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

#define TEST_EXPECT_SUCCESS(exp) assert_true(az_result_succeeded(exp))

static void test_az_span_getters(void** state)
{
  (void)state;

  uint8_t example[] = "example";
  az_span span = AZ_SPAN_FROM_BUFFER(example);
  assert_int_equal(az_span_size(span), 8);
  assert_ptr_equal(az_span_ptr(span), &example);
}

static void az_single_char_ascii_lower_test(void** state)
{
  (void)state;

  for (uint8_t i = 0; i <= SCHAR_MAX; ++i)
  {
    uint8_t buffer[1] = { i };
    az_span span = AZ_SPAN_FROM_BUFFER(buffer);

    // Comparison to itself should return true for all values in the range.
    assert_true(az_span_is_content_equal_ignoring_case(span, span));

    // For ASCII letters, verify that comparing upper and lower case return true.
    if (i >= 'A' && i <= 'Z')
    {
      uint8_t lower[1] = { (uint8_t)(i + 32) };
      az_span lowerSpan = AZ_SPAN_FROM_BUFFER(lower);
      assert_true(az_span_is_content_equal_ignoring_case(span, lowerSpan));
      assert_true(az_span_is_content_equal_ignoring_case(lowerSpan, span));
    }
    else if (i >= 'a' && i <= 'z')
    {
      uint8_t upper[1] = { (uint8_t)(i - 32) };
      az_span upperSpan = AZ_SPAN_FROM_BUFFER(upper);
      assert_true(az_span_is_content_equal_ignoring_case(span, upperSpan));
      assert_true(az_span_is_content_equal_ignoring_case(upperSpan, span));
    }
    else
    {
      // Make sure that no other comparison returns true.
      for (uint8_t j = 0; j <= SCHAR_MAX; ++j)
      {
        uint8_t other[1] = { j };
        az_span otherSpan = AZ_SPAN_FROM_BUFFER(other);

        if (i == j)
        {
          assert_true(az_span_is_content_equal_ignoring_case(span, otherSpan));
        }
        else
        {
          assert_false(az_span_is_content_equal_ignoring_case(span, otherSpan));
        }
      }
    }
  }
}

static void az_span_to_lower_test(void** state)
{
  (void)state;

  az_span a = AZ_SPAN_FROM_STR("one");
  az_span b = AZ_SPAN_FROM_STR("One");
  az_span c = AZ_SPAN_FROM_STR("ones");
  az_span d = AZ_SPAN_FROM_STR("ona");
  assert_true(az_span_is_content_equal_ignoring_case(a, b));
  assert_false(az_span_is_content_equal_ignoring_case(a, c));
  assert_false(az_span_is_content_equal_ignoring_case(a, d));
}

static void test_az_span_is_content_equal(void** state)
{
  (void)state;

  az_span a = AZ_SPAN_FROM_STR("one");
  az_span b = AZ_SPAN_FROM_STR("One");
  az_span c = AZ_SPAN_FROM_STR("one1");
  az_span d = AZ_SPAN_FROM_STR("done"); // d contains a

  assert_false(az_span_is_content_equal(a, b));
  assert_false(az_span_is_content_equal(b, a));
  assert_false(az_span_is_content_equal(a, c));
  assert_false(az_span_is_content_equal(c, a));
  assert_false(az_span_is_content_equal(a, d));
  assert_false(az_span_is_content_equal(d, a));

  assert_true(az_span_is_content_equal(a, AZ_SPAN_FROM_STR("one")));
  assert_true(az_span_is_content_equal(a, a));

  // Comparing subsets
  assert_true(az_span_is_content_equal(a, az_span_slice_to_end(d, 1)));
  assert_true(az_span_is_content_equal(az_span_slice_to_end(d, 1), a));

  // Comparing empty to non-empty
  assert_false(az_span_is_content_equal(a, AZ_SPAN_EMPTY));
  assert_false(az_span_is_content_equal(AZ_SPAN_EMPTY, a));

  // Empty spans are equal
  assert_true(az_span_is_content_equal(AZ_SPAN_EMPTY, AZ_SPAN_EMPTY));

  assert_true(az_span_is_content_equal(az_span_slice_to_end(a, 3), AZ_SPAN_EMPTY));
  assert_true(az_span_is_content_equal(az_span_slice_to_end(a, 3), az_span_slice_to_end(b, 3)));

  assert_true(az_span_is_content_equal(AZ_SPAN_FROM_STR(""), AZ_SPAN_FROM_STR("")));
  assert_true(az_span_is_content_equal(AZ_SPAN_FROM_STR(""), AZ_SPAN_EMPTY));
  assert_true(az_span_is_content_equal(AZ_SPAN_FROM_STR(""), az_span_slice_to_end(a, 3)));
}

#define az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(source)            \
  do                                                                        \
  {                                                                         \
    uint32_t ui32 = 0;                                                      \
    int32_t i32 = 0;                                                        \
    uint64_t ui64 = 0;                                                      \
    int64_t i64 = 0;                                                        \
    assert_true(az_span_atou32(source, &ui32) == AZ_ERROR_UNEXPECTED_CHAR); \
    assert_true(az_span_atoi32(source, &i32) == AZ_ERROR_UNEXPECTED_CHAR);  \
    assert_true(az_span_atou64(source, &ui64) == AZ_ERROR_UNEXPECTED_CHAR); \
    assert_true(az_span_atoi64(source, &i64) == AZ_ERROR_UNEXPECTED_CHAR);  \
  } while (0)

#define AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(source)                            \
  do                                                                         \
  {                                                                          \
    uint32_t ui32 = 0;                                                       \
    int32_t i32 = 0;                                                         \
    uint64_t ui64 = 0;                                                       \
    int64_t i64 = 0;                                                         \
    double decimal = 0;                                                      \
    assert_true(az_span_atou32(source, &ui32) == AZ_ERROR_UNEXPECTED_CHAR);  \
    assert_true(az_span_atoi32(source, &i32) == AZ_ERROR_UNEXPECTED_CHAR);   \
    assert_true(az_span_atou64(source, &ui64) == AZ_ERROR_UNEXPECTED_CHAR);  \
    assert_true(az_span_atoi64(source, &i64) == AZ_ERROR_UNEXPECTED_CHAR);   \
    assert_true(az_span_atod(source, &decimal) == AZ_ERROR_UNEXPECTED_CHAR); \
  } while (0)

static void az_span_atox_return_errors(void** state)
{
  (void)state;

  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("test"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR(" "));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR(" 1"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("+"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("--1"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("++1"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("-+"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("+-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("-0+"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("0-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("+0-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("1-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("123a"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("123,"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("123 "));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("--123"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("-+123"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("+-123"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("  -1-"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("- INFINITY"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("- 0"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("+ 1"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("1.-e3"));
  AZ_SPAN_ATOX_RETURN_ERRORS_HELPER(AZ_SPAN_FROM_STR("1.-e/3"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("1.23"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("-1.23"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("11e2"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("-1.1e+2"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("1.23e3"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("99999999999999999999"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("999999999999999999999"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("18446744073709551616"));
  az_SPAN_ATOX_RETURN_ERRORS_HELPER_EXCLUDE_DOUBLE(AZ_SPAN_FROM_STR("-18446744073709551616"));
}

static void az_span_atou32_test(void** state)
{
  (void)state;
  uint32_t value = 0;

  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_int_equal(value, 0);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("+1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("001024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_int_equal(value, 2147483647);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("4294967295"), &value), AZ_OK);
  assert_int_equal(value, 4294967295);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("00004294967295"), &value), AZ_OK);
  assert_int_equal(value, 4294967295);

  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("-123"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("42949672950"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("-2147483648"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("-4294967295"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("-4294967296"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("4294967296"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("42949672950"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("9223372036854775807"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("9223372036854775808"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("18446744073709551615"), &value), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_atoi32_test(void** state)
{
  (void)state;
  int32_t value = 0;

  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_int_equal(value, 0);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("+1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("-1024"), &value), AZ_OK);
  assert_int_equal(value, -1024);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("001024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_int_equal(value, 2147483647);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("-2147483648"), &value), AZ_OK);
  assert_int_equal(value, -2147483647 - 1);
  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("-00002147483648"), &value), AZ_OK);
  assert_int_equal(value, -2147483647 - 1);

  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("21474836470"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("2147483648"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("-2147483649"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("-4294967295"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("-4294967296"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("9223372036854775807"), &value), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_atou64_test(void** state)
{
  (void)state;
  uint64_t value = 0;

  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_int_equal(value, 0);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("+1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("001024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_int_equal(value, 2147483647);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("4294967295"), &value), AZ_OK);
  assert_int_equal(value, 4294967295);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("9223372036854775807"), &value), AZ_OK);
  assert_int_equal(value, 9223372036854775807UL);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("18446744073709551615"), &value), AZ_OK);
  assert_int_equal(value, 18446744073709551615UL);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("000018446744073709551615"), &value), AZ_OK);
  assert_int_equal(value, 18446744073709551615UL);

  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("-123"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou64(AZ_SPAN_FROM_STR("184467440737095516150"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou64(AZ_SPAN_FROM_STR("18446744073709551616"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou64(AZ_SPAN_FROM_STR("-9223372036854775809"), &value), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_atoi64_test(void** state)
{
  (void)state;
  int64_t value = 0;

  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_int_equal(value, 0);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("+1024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("-1024"), &value), AZ_OK);
  assert_int_equal(value, -1024);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("001024"), &value), AZ_OK);
  assert_int_equal(value, 1024);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_int_equal(value, 2147483647);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("-2147483648"), &value), AZ_OK);
  assert_int_equal(value, -2147483647 - 1);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("4294967295"), &value), AZ_OK);
  assert_int_equal(value, 4294967295);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("-4294967296"), &value), AZ_OK);
  assert_int_equal(value, -4294967296);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("9223372036854775807"), &value), AZ_OK);
  assert_int_equal(value, 9223372036854775807L);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("-9223372036854775808"), &value), AZ_OK);
  assert_int_equal(value, -9223372036854775807L - 1);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("-00009223372036854775808"), &value), AZ_OK);
  assert_int_equal(value, -9223372036854775807L - 1);

  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("92233720368547758070"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("12233720368547758070"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("9223372036854775808"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("18446744073709551615"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("18446744073709551616"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("-9223372036854775809"), &value), AZ_ERROR_UNEXPECTED_CHAR);
}

#define TEST_AZ_ISFINITE_HELPER(source, expected)      \
  do                                                   \
  {                                                    \
    double decimal = 0.0;                              \
    memcpy(&decimal, &source, sizeof(decimal));        \
    assert_int_equal(_az_isfinite(decimal), expected); \
  } while (0)

static void test_az_isfinite(void** state)
{
  (void)state;

  uint64_t source = 0;

  TEST_AZ_ISFINITE_HELPER(source, true);
  source = 1;
  TEST_AZ_ISFINITE_HELPER(source, true);
  source = 0x6FFFFFFFFFFFFFFF;
  TEST_AZ_ISFINITE_HELPER(source, true);
  source = 0x7FEFFFFFFFFFFFFF;
  TEST_AZ_ISFINITE_HELPER(source, true);

  source = 0x7FF0000000000000; // +inf
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0x7FF0000000000001; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0x7FF7FFFFFFFFFFFF; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0x7FF8000000000000; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0x7FFFFFFFFFFFFFFF; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);

  source = 0x8000000000000000;
  TEST_AZ_ISFINITE_HELPER(source, true);
  source = 0xFFEFFFFFFFFFFFFF;
  TEST_AZ_ISFINITE_HELPER(source, true);

  source = 0xFFF0000000000000; // -inf
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0xFFF7FFFFFFFFFFFF; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0xFFF8000000000000; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);
  source = 0xFFFFFFFFFFFFFFFF; // nan
  TEST_AZ_ISFINITE_HELPER(source, false);

  source = 0xFFFFFFFFFFFFFFFF + 1;
  TEST_AZ_ISFINITE_HELPER(source, true);
}

// Disable warning for float comparisons, for this particular test
// error : comparing floating point with == or != is unsafe[-Werror = float - equal]
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif // __GNUC__

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif // __clang__

static void az_span_atod_test(void** state)
{
  (void)state;
  double value = 0;

  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1024"), &value), AZ_OK);
  assert_true(value == 1024);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+1024"), &value), AZ_OK);
  assert_true(value == 1024);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("001024"), &value), AZ_OK);
  assert_true(value == 1024);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1024"), &value), AZ_OK);
  assert_true(value == -1024);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_true(value == 2147483647);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-2147483648"), &value), AZ_OK);
  assert_true(value == -2147483647 - 1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("4294967295"), &value), AZ_OK);
  assert_true(value == 4294967295);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-4294967296"), &value), AZ_OK);
  assert_true(value == -4294967296);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9223372036854775807"), &value), AZ_OK);
  assert_true(value == 9223372036854775807);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-9223372036854775808"), &value), AZ_OK);
  assert_true(value == -2147483647 * (double)4294967298);

  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.23e3"), &value), AZ_OK);
  assert_true(value == 1.23e3);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+001.23e3"), &value), AZ_OK);
  assert_true(value == 1.23e3);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.23"), &value), AZ_OK);
  assert_true(value == 1.23);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-123.456e-78"), &value), AZ_OK);
  assert_true(value == -123.456e-78);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.456e+78"), &value), AZ_OK);
  assert_true(value == 123.456e+78);

  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-0"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.0"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1"), &value), AZ_OK);
  assert_true(value == 1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1."), &value), AZ_OK);
  assert_true(value == 1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.e3"), &value), AZ_OK);
  assert_true(value == 1000);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1"), &value), AZ_OK);
  assert_true(value == -1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.0"), &value), AZ_OK);
  assert_true(value == 1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.0"), &value), AZ_OK);
  assert_true(value == -1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("12345"), &value), AZ_OK);
  assert_true(value == 12345);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-12345"), &value), AZ_OK);
  assert_true(value == -12345);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.123"), &value), AZ_OK);
  assert_true(value == 123.123);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.1230"), &value), AZ_OK);
  assert_true(value == 123.1230);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.0100"), &value), AZ_OK);
  assert_true(value == 123.0100);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.01"), &value), AZ_OK);
  assert_true(value == 123.01);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("123.001"), &value), AZ_OK);
  assert_true(value == 123.001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.000000000000001"), &value), AZ_OK);
  assert_true(value == 0.000000000000001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.0000000001"), &value), AZ_OK);
  assert_true(value == 1.0000000001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.0000000001"), &value), AZ_OK);
  assert_true(value == -1.0000000001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("100.001"), &value), AZ_OK);
  assert_true(value == 100.001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("100.00100"), &value), AZ_OK);
  assert_true(value == 100.00100);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("00100.001"), &value), AZ_OK);
  assert_true(value == 00100.001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("00100.00100"), &value), AZ_OK);
  assert_true(value == 00100.00100);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.001"), &value), AZ_OK);
  assert_true(value == 0.001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.0012"), &value), AZ_OK);
  assert_true(value == 0.0012);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.2e4"), &value), AZ_OK);
  assert_true(value == 1.2e4);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.2e-4"), &value), AZ_OK);
  assert_true(value == 1.2e-4);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.2e+4"), &value), AZ_OK);
  assert_true(value == 1.2e+4);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.2e4"), &value), AZ_OK);
  assert_true(value == -1.2e4);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.2e-4"), &value), AZ_OK);
  assert_true(value == -1.2e-4);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.0001"), &value), AZ_OK);
  assert_true(value == 0.0001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.00102"), &value), AZ_OK);
  assert_true(value == 0.00102);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.34567"), &value), AZ_OK);
  assert_true(value == .34567);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+0.34567"), &value), AZ_OK);
  assert_true(value == .34567);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-0.34567"), &value), AZ_OK);
  assert_true(value == -.34567);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9876.54321"), &value), AZ_OK);
  assert_true(value == 9876.54321);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-9876.54321"), &value), AZ_OK);
  assert_true(value == -9876.54321);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("987654.321"), &value), AZ_OK);
  assert_true(value == 987654.321);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-987654.321"), &value), AZ_OK);
  assert_true(value == -987654.321);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("987654.0000321"), &value), AZ_OK);
  assert_true(value == 987654.0000321);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("2147483647"), &value), AZ_OK);
  assert_true(value == 2147483647);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("2147483648"), &value), AZ_OK);
  assert_true(value == 2147483648);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-2147483648"), &value), AZ_OK);
  assert_true(value == -2147483647 - 1);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("4503599627370496"), &value), AZ_OK);
  assert_true(value == 4503599627370496);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9007199254740991"), &value), AZ_OK);
  assert_true(value == 9007199254740991);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("4503599627370496.2"), &value), AZ_OK);
  assert_true(value == 4503599627370496.2);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e15"), &value), AZ_OK);
  assert_true(value == 1e15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1e15"), &value), AZ_OK);
  assert_true(value == -1e15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.8e10"), &value), AZ_OK);
  assert_true(value == 1.8e10);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.8e10"), &value), AZ_OK);
  assert_true(value == -1.8e10);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e-15"), &value), AZ_OK);
  assert_true(value == 1e-15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e-10"), &value), AZ_OK);
  assert_true(value == 1e-10);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e-5"), &value), AZ_OK);
  assert_true(value == 1e-5);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("0.1234567890123456"), &value), AZ_OK);
  assert_true(value == 0.1234567890123456);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("123456789012345.123456789012340000"), &value), AZ_OK);
  assert_true(value == 123456789012345.123456789012340000);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("1000000000000.123456789012340000"), &value), AZ_OK);
  assert_true(value == 1000000000000.123456789012340000);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("123456789012345.1234567890123400001"), &value), AZ_OK);
  assert_true(value == 123456789012345.1234567890123400001);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("1000000000000.1234567890123400001"), &value), AZ_OK);
  assert_true(value == 1000000000000.1234567890123400001);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("12345.123e-15"), &value), AZ_OK);
  assert_true(value == 12345.123e-15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("12345.12300000010e5"), &value), AZ_OK);
  assert_true(value == 12345.12300000010e5);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e-300"), &value), AZ_OK);
  assert_true(value == 1e-300);

  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9007199254740992"), &value), AZ_OK);
  assert_true(value == 9007199254740992);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9007199254740993"), &value), AZ_OK);
  assert_true(value == 9007199254740993);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("45035996273704961"), &value), AZ_OK);
  assert_true(value == 45035996273704961);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("9223372036854775806"), &value), AZ_OK);
  assert_true(value == 9223372036854775806);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-9223372036854775806"), &value), AZ_OK);
  assert_true(value == -9223372036854775806);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1844674407370955100"), &value), AZ_OK);
  assert_true(value == 1844674407370955100);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.844674407370955e+19"), &value), AZ_OK);
  assert_true(value == 1.844674407370955e+19);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.8446744073709551e+19"), &value), AZ_OK);
  assert_true(value == 1.8446744073709551e+19);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.8446744073709552e+19"), &value), AZ_OK);
  assert_true(value == 1.8446744073709552e+19);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("18446744073709551615"), &value), AZ_OK);
  assert_true(value == 18446744073709551615UL);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("18446744073709551615.18446744073709551615"), &value), AZ_OK);
  assert_true(value == 18446744073709551615.18446744073709551615);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("+000018446744073709551615.18446744073709551615"), &value),
      AZ_OK);
  assert_true(value == 18446744073709551615.18446744073709551615);
  assert_int_equal(
      az_span_atod(AZ_SPAN_FROM_STR("-000018446744073709551615.18446744073709551615"), &value),
      AZ_OK);
  assert_true(value == -18446744073709551615.18446744073709551615);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e16"), &value), AZ_OK);
  assert_true(value == 1e16);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("12345.123e15"), &value), AZ_OK);
  assert_true(value == 12345.123e15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-12345.123e15"), &value), AZ_OK);
  assert_true(value == -12345.123e15);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e300"), &value), AZ_OK);
  assert_true(value == 1e300);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1e300"), &value), AZ_OK);
  assert_true(value == -1e300);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.7e308"), &value), AZ_OK);
  assert_true(value == 1.7e308);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.7e308"), &value), AZ_OK);
  assert_true(value == -1.7e308);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("2.22507e-308"), &value), AZ_OK);
  assert_true(value == 2.22507e-308);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-2.22507e-308"), &value), AZ_OK);
  assert_true(value == -2.22507e-308);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("4.94e-325"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1e-400"), &value), AZ_OK);
  assert_true(value == 0);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1e-400"), &value), AZ_OK);
  assert_true(value == 0);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

static void az_span_atod_non_finite_not_allowed(void** state)
{
  (void)state;
  double value = 0;

  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("nan"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-nan"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+nan"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("inf"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+inf"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-inf"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("NAN"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("INF"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+INF"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-INF"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("INFINITY"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("+INFINITY"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-INFINITY"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.8e308"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-1.8e308"), &value), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("1.8e309"), &value), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_ato_number_whitespace_or_invalid_not_allowed(void** state)
{
  (void)state;
  int32_t value_i32 = 0;
  uint32_t value_u32 = 0;
  int64_t value_i64 = 0;
  uint64_t value_u64 = 0;
  double value_d = 0;

  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("   123"), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("   123"), &value_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("   123"), &value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou64(AZ_SPAN_FROM_STR("   123"), &value_u64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("   123"), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("\n123"), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("\n123"), &value_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("\n123"), &value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("\n123"), &value_u64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("\n123"), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("a123"), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("a123"), &value_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("a123"), &value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("a123"), &value_u64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("a123"), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(AZ_SPAN_FROM_STR("- 123"), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou32(AZ_SPAN_FROM_STR("- 123"), &value_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atoi64(AZ_SPAN_FROM_STR("- 123"), &value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atou64(AZ_SPAN_FROM_STR("- 123"), &value_u64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("- 123"), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(
      az_span_atoi32(AZ_SPAN_FROM_STR("-\n123"), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou32(AZ_SPAN_FROM_STR("-\n123"), &value_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atoi64(AZ_SPAN_FROM_STR("-\n123"), &value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(
      az_span_atou64(AZ_SPAN_FROM_STR("-\n123"), &value_u64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(AZ_SPAN_FROM_STR("-\n123"), &value_d), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_ato_number_no_out_of_bounds_reads(void** state)
{
  (void)state;
  int32_t value_i32 = 0;
  double value_d = 0;

  az_span source = AZ_SPAN_FROM_STR("   123456");
  // Makes sure we only read and parse up to the character '3', since that is the last character
  // within the span slice
  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 0, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 0, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(az_span_slice(source, 3, 6), &value_i32), AZ_OK);
  assert_int_equal(value_i32, 123);
  assert_int_equal(az_span_atod(az_span_slice(source, 3, 6), &value_d), AZ_OK);
  assert_int_equal(value_d, 123);

  source = AZ_SPAN_FROM_STR("   123A");
  // Makes sure we only read and parse up to the character '3', since that is the last character
  // within the span slice
  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 0, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 0, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(az_span_slice(source, 3, 6), &value_i32), AZ_OK);
  assert_int_equal(value_i32, 123);
  assert_int_equal(az_span_atod(az_span_slice(source, 3, 6), &value_d), AZ_OK);
  assert_int_equal(value_d, 123);

  source = AZ_SPAN_FROM_STR("   123.");
  // Makes sure we only read and parse up to the character '3', since that is the last character
  // within the span slice
  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 0, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 0, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(az_span_slice(source, 3, 6), &value_i32), AZ_OK);
  assert_int_equal(value_i32, 123);
  assert_int_equal(az_span_atod(az_span_slice(source, 3, 6), &value_d), AZ_OK);
  assert_int_equal(value_d, 123);

  source = AZ_SPAN_FROM_STR("   123-");
  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 0, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 0, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(az_span_atoi32(az_span_slice(source, 3, 6), &value_i32), AZ_OK);
  assert_int_equal(value_i32, 123);
  assert_int_equal(az_span_atod(az_span_slice(source, 3, 6), &value_d), AZ_OK);
  assert_int_equal(value_d, 123);

  source = AZ_SPAN_FROM_STR("   12-4");
  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 0, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 0, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  assert_int_equal(
      az_span_atoi32(az_span_slice(source, 3, 6), &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(az_span_slice(source, 3, 6), &value_d), AZ_ERROR_UNEXPECTED_CHAR);

  source = AZ_SPAN_FROM_STR("n1");
  assert_int_equal(az_span_atoi32(source, &value_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_span_atod(source, &value_d), AZ_ERROR_UNEXPECTED_CHAR);
}

static void az_span_to_str_test(void** state)
{
  (void)state;
  az_span sample = AZ_SPAN_FROM_STR("hello World!");
  char str[20];

  az_span_to_str(str, 20, sample);
  assert_string_equal(str, "hello World!");
}

static void az_span_find_beginning_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefg");
  az_span target = AZ_SPAN_FROM_STR("abc");

  assert_int_equal(az_span_find(span, target), 0);
}

static void az_span_find_middle_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefg");
  az_span target = AZ_SPAN_FROM_STR("gab");

  assert_int_equal(az_span_find(span, target), 6);
}

static void az_span_find_end_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefgh");
  az_span target = AZ_SPAN_FROM_STR("efgh");

  assert_int_equal(az_span_find(span, target), 11);
}

static void az_span_find_source_target_identical_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefg");
  az_span target = AZ_SPAN_FROM_STR("abcdefgabcdefg");

  assert_int_equal(az_span_find(span, target), 0);
}

static void az_span_find_not_found_fail(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefg");
  az_span target = AZ_SPAN_FROM_STR("abd");

  assert_int_equal(az_span_find(span, target), -1);
}

static void az_span_find_error_cases_fail(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefg");
  az_span target = AZ_SPAN_FROM_STR("abd");

  assert_int_equal(az_span_find(AZ_SPAN_EMPTY, AZ_SPAN_EMPTY), 0);
  assert_int_equal(az_span_find(span, AZ_SPAN_EMPTY), 0);
  assert_int_equal(az_span_find(AZ_SPAN_EMPTY, target), -1);
}

static void az_span_find_target_longer_than_source_fails(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("aa");
  az_span target = AZ_SPAN_FROM_STR("aaa");

  assert_int_equal(az_span_find(span, target), -1);
}

static void az_span_find_target_overlap_continuation_of_source_fails(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcd");
  az_span target = AZ_SPAN_FROM_STR("cde");

  assert_int_equal(az_span_find(span, target), -1);
}

static void az_span_find_target_more_chars_than_prefix_of_source_fails(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcd");
  az_span target = AZ_SPAN_FROM_STR("zab");

  assert_int_equal(az_span_find(span, target), -1);
}

static void az_span_find_overlapping_target_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefghij");
  az_span target = az_span_slice(span, 6, 9);

  assert_int_equal(az_span_find(span, target), 6);
}

static void az_span_find_embedded_NULLs_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcd\0\0fghij");
  az_span target = AZ_SPAN_FROM_STR("\0\0");

  assert_int_equal(az_span_find(span, target), 4);
}

static void az_span_find_capacity_checks_success(void** state)
{
  (void)state;

  uint8_t* buffer = (uint8_t*)"aaaa";

  assert_int_equal(az_span_find(az_span_create(buffer, 2), az_span_create(buffer, 2)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer, 2), az_span_create(buffer, 0)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer, 0), az_span_create(buffer, 0)), 0);

  assert_int_equal(az_span_find(az_span_create(buffer, 2), az_span_create(buffer + 1, 2)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer + 1, 2), az_span_create(buffer, 2)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer + 1, 2), az_span_create(buffer + 1, 2)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer, 2), az_span_create(buffer + 2, 2)), 0);
  assert_int_equal(az_span_find(az_span_create(buffer + 2, 2), az_span_create(buffer, 2)), 0);
}

static void az_span_find_overlapping_checks_success(void** state)
{
  (void)state;

  az_span span = AZ_SPAN_FROM_STR("abcdefghij");
  az_span source = az_span_slice(span, 1, 4);
  az_span target = az_span_slice(span, 6, 9);
  assert_int_equal(az_span_find(source, target), -1);
  assert_int_equal(az_span_find(source, az_span_slice(span, 1, 5)), -1);
  assert_int_equal(az_span_find(source, az_span_slice(span, 2, 4)), 1);
}

static void az_span_i64toa_test(void** state)
{
  (void)state;
  uint8_t buffer[100];
  az_span b_span = AZ_SPAN_FROM_BUFFER(buffer);
  az_span remainder;
  int32_t size_before_write = az_span_size(b_span);
  int64_t number = 123;
  az_span number_str = AZ_SPAN_FROM_STR("123");

  assert_int_equal(az_span_i64toa(b_span, number, &remainder), AZ_OK);
  assert_int_equal(size_before_write, az_span_size(b_span));
  // remainder should be size minus number of digits (3)
  assert_int_equal(az_span_size(remainder), size_before_write - 3);

  // create az_span for written data
  b_span = az_span_create(az_span_ptr(b_span), az_span_size(b_span) - az_span_size(remainder));

  assert_true(az_span_is_content_equal(b_span, number_str));

  // convert back
  uint64_t reverse = 0;
  assert_int_equal(az_span_atou64(b_span, &reverse), AZ_OK);
  assert_int_equal(reverse, number);
}

static void az_span_i64toa_negative_number_test(void** state)
{
  (void)state;
  uint8_t buffer[100];
  az_span b_span = AZ_SPAN_FROM_BUFFER(buffer);
  az_span remainder;
  int32_t size_before_write = az_span_size(b_span);
  int64_t number = -123;
  az_span number_str = AZ_SPAN_FROM_STR("-123");

  assert_int_equal(az_span_i64toa(b_span, number, &remainder), AZ_OK);
  assert_int_equal(size_before_write, az_span_size(b_span));
  // remainder should be size minus number of digits (4)
  assert_int_equal(az_span_size(remainder), size_before_write - 4);

  // create az_span for written data
  b_span = az_span_create(az_span_ptr(b_span), az_span_size(b_span) - az_span_size(remainder));

  assert_true(az_span_is_content_equal(b_span, number_str));

  int64_t reverse = 0;
  assert_int_equal(az_span_atoi64(b_span, &reverse), AZ_OK);
  assert_int_equal(reverse, number);
}

static void az_span_slice_to_end_test(void** state)
{
  (void)state;
  uint8_t raw_buffer[20];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);

  assert_int_equal(az_span_size(buffer), 20);

  az_span result = az_span_slice_to_end(buffer, 1);
  assert_int_equal(az_span_size(result), 19);

  result = az_span_slice_to_end(buffer, 5);
  assert_int_equal(az_span_size(result), 15);
}

static void az_span_test_macro_only_allows_byte_buffers(void** state)
{
  (void)state;
  {
    uint8_t uint8_buffer[2];
    assert_int_equal(_az_IS_ARRAY(uint8_buffer), 1);
    assert_int_equal(_az_IS_BYTE_ARRAY(uint8_buffer), 1);
    az_span valid = AZ_SPAN_FROM_BUFFER(uint8_buffer);
    assert_int_equal(az_span_size(valid), 2);
  }

  {
    char char_buffer[2];
    assert_int_equal(_az_IS_ARRAY(char_buffer), 1);
    assert_int_equal(_az_IS_BYTE_ARRAY(char_buffer), 1);
    az_span valid = AZ_SPAN_FROM_BUFFER(char_buffer);
    assert_int_equal(az_span_size(valid), 2);
  }

  {
    uint32_t uint32_buffer[2];
    assert_int_equal(_az_IS_ARRAY(uint32_buffer), 1);
    assert_int_equal(_az_IS_BYTE_ARRAY(uint32_buffer), 0);
  }

  {
    uint8_t x = 1;
    uint8_t* p1 = &x;
    assert_int_equal(_az_IS_ARRAY(p1), 0);
    assert_int_equal(_az_IS_BYTE_ARRAY(p1), 0);
  }

  {
    char* p1 = "HELLO";
    assert_int_equal(_az_IS_ARRAY(p1), 0);
    assert_int_equal(_az_IS_BYTE_ARRAY(p1), 0);
  }
}

static void az_span_create_from_str_succeeds(void** state)
{
  (void)state;
  char* str = "HelloWorld";
  az_span buffer = az_span_create_from_str(str);

  assert_int_equal(az_span_size(buffer), 10);
  assert_true(az_span_ptr(buffer) != NULL);
  assert_true((char*)az_span_ptr(buffer) == str);
}

static void az_span_copy_uint8_succeeds(void** state)
{
  (void)state;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);

  buffer = az_span_copy_u8(buffer, 'a');
  assert_int_equal(az_span_size(buffer), 14);
  buffer = az_span_copy_u8(buffer, 'b');
  assert_int_equal(az_span_size(buffer), 13);
  buffer = az_span_copy_u8(buffer, 'c');
  assert_int_equal(az_span_size(buffer), 12);

  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 3), AZ_SPAN_FROM_STR("abc")));
}

static void az_span_i32toa_succeeds(void** state)
{
  (void)state;
  int32_t v = 12345;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_i32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 10);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 5), AZ_SPAN_FROM_STR("12345")));
}

static void az_span_i32toa_negative_succeeds(void** state)
{
  (void)state;
  int32_t v = -12345;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_i32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 9);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 6), AZ_SPAN_FROM_STR("-12345")));
}

static void az_span_i32toa_zero_succeeds(void** state)
{
  (void)state;
  int32_t v = 0;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_i32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 14);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 1), AZ_SPAN_FROM_STR("0")));
}

static void az_span_i32toa_max_int_succeeds(void** state)
{
  (void)state;
  int32_t v = 2147483647;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_i32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 5);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 10), AZ_SPAN_FROM_STR("2147483647")));
}

static void az_span_i32toa_overflow_fails(void** state)
{
  (void)state;
  int32_t v = 2147483647;
  uint8_t raw_buffer[4];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_span_i32toa(buffer, v, &out_span) == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void az_span_u32toa_succeeds(void** state)
{
  (void)state;
  uint32_t v = 12345;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_u32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 10);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 5), AZ_SPAN_FROM_STR("12345")));
}

static void az_span_u32toa_zero_succeeds(void** state)
{
  (void)state;
  uint32_t v = 0;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_u32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 14);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 1), AZ_SPAN_FROM_STR("0")));
}

static void az_span_u32toa_max_uint_succeeds(void** state)
{
  (void)state;
  uint32_t v = 4294967295;
  uint8_t raw_buffer[15];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_result_succeeded(az_span_u32toa(buffer, v, &out_span)));
  assert_int_equal(az_span_size(out_span), 5);
  assert_true(az_span_is_content_equal(
      az_span_slice(AZ_SPAN_FROM_BUFFER(raw_buffer), 0, 10), AZ_SPAN_FROM_STR("4294967295")));
}

static void az_span_u32toa_overflow_fails(void** state)
{
  (void)state;
  uint32_t v = 2147483647;
  uint8_t raw_buffer[4];
  az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span out_span;

  assert_true(az_span_u32toa(buffer, v, &out_span) == AZ_ERROR_NOT_ENOUGH_SPACE);
}

#define AZ_SPAN_DTOA_SUCCEEDS_HELPER(v, fractional_digits, expected)                         \
  do                                                                                         \
  {                                                                                          \
    az_span buffer = AZ_SPAN_FROM_BUFFER(raw_buffer);                                        \
    az_span out_span = AZ_SPAN_EMPTY;                                                        \
    assert_true(az_result_succeeded(az_span_dtoa(buffer, v, fractional_digits, &out_span))); \
    az_span output = az_span_slice(buffer, 0, _az_span_diff(out_span, buffer));              \
    assert_memory_equal(                                                                     \
        az_span_ptr(output), az_span_ptr(expected), (size_t)az_span_size(expected));         \
    assert_true(az_result_succeeded(az_span_dtoa(buffer, v, fractional_digits, &out_span))); \
    double round_trip = 0;                                                                   \
    assert_true(az_result_succeeded(az_span_atod(output, &round_trip)));                     \
    assert_true(fabs(v - round_trip) < 0.01);                                                \
  } while (0)

static void az_span_dtoa_succeeds(void** state)
{
  (void)state;

  // We don't need more than 33 bytes to hold the supported doubles:
  // [-][0-9]{16}.[0-9]{15}, i.e. 1+16+1+15
  uint8_t raw_buffer[33] = { 0 };

  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0, 15, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1., 15, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.e3, 15, AZ_SPAN_FROM_STR("1000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-0, 15, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0, 15, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1, 15, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1, 15, AZ_SPAN_FROM_STR("-1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.0, 15, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.0, 15, AZ_SPAN_FROM_STR("-1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(12345, 15, AZ_SPAN_FROM_STR("12345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-12345, 15, AZ_SPAN_FROM_STR("-12345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.123, 15, AZ_SPAN_FROM_STR("123.123000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.1230, 15, AZ_SPAN_FROM_STR("123.123000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.0100, 15, AZ_SPAN_FROM_STR("123.010000000000005"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.001, 15, AZ_SPAN_FROM_STR("123.001000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.000000000000001, 15, AZ_SPAN_FROM_STR("0.000000000000001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.0000000001, 15, AZ_SPAN_FROM_STR("1.0000000001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.0000000001, 15, AZ_SPAN_FROM_STR("-1.0000000001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(100.001, 15, AZ_SPAN_FROM_STR("100.001000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(100.00100, 15, AZ_SPAN_FROM_STR("100.001000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(00100.001, 15, AZ_SPAN_FROM_STR("100.001000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(00100.00100, 15, AZ_SPAN_FROM_STR("100.001000000000004"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.001, 15, AZ_SPAN_FROM_STR("0.001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0012, 15, AZ_SPAN_FROM_STR("0.001199999999999"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e4, 15, AZ_SPAN_FROM_STR("12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e-4, 15, AZ_SPAN_FROM_STR("0.00012"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e+4, 15, AZ_SPAN_FROM_STR("12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.2e4, 15, AZ_SPAN_FROM_STR("-12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.2e-4, 15, AZ_SPAN_FROM_STR("-0.00012"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0001, 15, AZ_SPAN_FROM_STR("0.0001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.00102, 15, AZ_SPAN_FROM_STR("0.00102"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(.34567, 15, AZ_SPAN_FROM_STR("0.34567"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(+.34567, 15, AZ_SPAN_FROM_STR("0.34567"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-.34567, 15, AZ_SPAN_FROM_STR("-0.34567"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(9876.54321, 15, AZ_SPAN_FROM_STR("9876.543209999999817"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-9876.54321, 15, AZ_SPAN_FROM_STR("-9876.543209999999817"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(987654.321, 15, AZ_SPAN_FROM_STR("987654.320999999996274"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-987654.321, 15, AZ_SPAN_FROM_STR("-987654.320999999996274"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(987654.0000321, 15, AZ_SPAN_FROM_STR("987654.000032100011594"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(2147483647, 15, AZ_SPAN_FROM_STR("2147483647"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(2 * (double)1073741824, 15, AZ_SPAN_FROM_STR("2147483648"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-2147483647 - 1, 15, AZ_SPAN_FROM_STR("-2147483648"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(4503599627370496, 15, AZ_SPAN_FROM_STR("4503599627370496"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(9007199254740991, 15, AZ_SPAN_FROM_STR("9007199254740991"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      (double)4503599627370496.2, 15, AZ_SPAN_FROM_STR("4503599627370496"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e15, 15, AZ_SPAN_FROM_STR("1000000000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1e15, 15, AZ_SPAN_FROM_STR("-1000000000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.8e10, 15, AZ_SPAN_FROM_STR("18000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.8e10, 15, AZ_SPAN_FROM_STR("-18000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-15, 15, AZ_SPAN_FROM_STR("0.000000000000001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-10, 15, AZ_SPAN_FROM_STR("0.0000000001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-5, 15, AZ_SPAN_FROM_STR("0.00001"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.1234567890123456, 15, AZ_SPAN_FROM_STR("0.123456789012345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      123456789012345.123456789012340000, 15, AZ_SPAN_FROM_STR("123456789012345.125"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      1000000000000.123456789012340000, 15, AZ_SPAN_FROM_STR("1000000000000.1234130859375"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      123456789012345.1234567890123400001, 15, AZ_SPAN_FROM_STR("123456789012345.125"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      1000000000000.1234567890123400001, 15, AZ_SPAN_FROM_STR("1000000000000.1234130859375"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(12345.123e-15, 15, AZ_SPAN_FROM_STR("0.000000000012345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      12345.12300000010e5, 15, AZ_SPAN_FROM_STR("1234512300.000010013580322"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-300, 15, AZ_SPAN_FROM_STR("0"));

  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1., 2, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.e3, 2, AZ_SPAN_FROM_STR("1000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-0, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1, 2, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1, 2, AZ_SPAN_FROM_STR("-1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.0, 2, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.0, 2, AZ_SPAN_FROM_STR("-1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(12345, 2, AZ_SPAN_FROM_STR("12345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-12345, 2, AZ_SPAN_FROM_STR("-12345"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.123, 2, AZ_SPAN_FROM_STR("123.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.1230, 2, AZ_SPAN_FROM_STR("123.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.0100, 2, AZ_SPAN_FROM_STR("123.01"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(123.001, 2, AZ_SPAN_FROM_STR("123"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.000000000000001, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.0000000001, 2, AZ_SPAN_FROM_STR("1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.0000000001, 2, AZ_SPAN_FROM_STR("-1"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(100.001, 2, AZ_SPAN_FROM_STR("100"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(100.00100, 2, AZ_SPAN_FROM_STR("100"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(00100.001, 2, AZ_SPAN_FROM_STR("100"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(00100.00100, 2, AZ_SPAN_FROM_STR("100"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.001, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0012, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e4, 2, AZ_SPAN_FROM_STR("12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e-4, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.2e+4, 2, AZ_SPAN_FROM_STR("12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.2e4, 2, AZ_SPAN_FROM_STR("-12000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.2e-4, 2, AZ_SPAN_FROM_STR("-0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.0001, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.00102, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(.34567, 2, AZ_SPAN_FROM_STR("0.34"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(+.34567, 2, AZ_SPAN_FROM_STR("0.34"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-.34567, 2, AZ_SPAN_FROM_STR("-0.34"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(9876.54321, 2, AZ_SPAN_FROM_STR("9876.54"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-9876.54321, 2, AZ_SPAN_FROM_STR("-9876.54"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(987654.321, 2, AZ_SPAN_FROM_STR("987654.32"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-987654.321, 2, AZ_SPAN_FROM_STR("-987654.32"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(987654.0000321, 2, AZ_SPAN_FROM_STR("987654"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(2147483647, 2, AZ_SPAN_FROM_STR("2147483647"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(2 * (double)1073741824, 2, AZ_SPAN_FROM_STR("2147483648"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-2147483647 - 1, 2, AZ_SPAN_FROM_STR("-2147483648"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(4503599627370496, 2, AZ_SPAN_FROM_STR("4503599627370496"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(9007199254740991, 2, AZ_SPAN_FROM_STR("9007199254740991"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER((double)4503599627370496.2, 2, AZ_SPAN_FROM_STR("4503599627370496"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e15, 2, AZ_SPAN_FROM_STR("1000000000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1e15, 2, AZ_SPAN_FROM_STR("-1000000000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1.8e10, 2, AZ_SPAN_FROM_STR("18000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(-1.8e10, 2, AZ_SPAN_FROM_STR("-18000000000"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-15, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-10, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-5, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(0.1234567890123456, 2, AZ_SPAN_FROM_STR("0.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      123456789012345.123456789012340000, 2, AZ_SPAN_FROM_STR("123456789012345.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      1000000000000.123456789012340000, 2, AZ_SPAN_FROM_STR("1000000000000.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      123456789012345.1234567890123400001, 2, AZ_SPAN_FROM_STR("123456789012345.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(
      1000000000000.1234567890123400001, 2, AZ_SPAN_FROM_STR("1000000000000.12"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(12345.123e-15, 2, AZ_SPAN_FROM_STR("0"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(12345.12300000010e5, 2, AZ_SPAN_FROM_STR("1234512300"));
  AZ_SPAN_DTOA_SUCCEEDS_HELPER(1e-300, 2, AZ_SPAN_FROM_STR("0"));
}

static void az_span_dtoa_overflow_fails(void** state)
{
  (void)state;

  // We don't need more than 33 bytes to hold the supported doubles:
  // [-][0-9]{16}.[0-9]{15}, i.e. 1+16+1+15
  uint8_t raw_buffer[33];
  az_span buff = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span o;

  assert_int_equal(az_span_dtoa(az_span_slice(buff, 0, 0), 0, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(az_span_dtoa(az_span_slice(buff, 0, 0), 1., 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 3), 1.e3, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(az_span_dtoa(az_span_slice(buff, 0, 0), 1, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(az_span_dtoa(az_span_slice(buff, 0, 1), -1, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 4), 12345, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), -12345, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 123.123, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), 123.0100, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 16), 0.000000000000001, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 11), 1.0000000001, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 12), -1.0000000001, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 7), 100.001, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 4), 0.001, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 0.0012, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 4), 1.2e4, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 6), 1.2e-4, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 4), 1.2e+4, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), -1.2e4, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 7), -1.2e-4, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 4), 0.0001, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), 0.00102, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), .34567, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 6), -.34567, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 9876.54321, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 11), -9876.54321, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 987654.321, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 11), -987654.321, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 987654.0000321, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 9), 2147483647, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 9), 2 * (double)1073741824, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), -2147483647 - 1, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 15), 4503599627370496, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 15), 9007199254740991, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 15), (double)4503599627370496.2, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 14), 1e15, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 15), -1e15, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 1.8e10, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 11), -1.8e10, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 15), 1e-15, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 10), 1e-10, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 5), 1e-5, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 16), 0.1234567890123456, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 18), 123456789012345.123456789012340000, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 26), 1000000000000.1234567890123400001, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 16), 12345.123e-15, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 25), 12345.12300000010e5, 15, &o),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_dtoa(az_span_slice(buff, 0, 0), 1e-300, 15, &o), AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void az_span_dtoa_too_large(void** state)
{
  (void)state;

  // We don't need more than 33 bytes to hold the supported doubles:
  // [-][0-9]{16}.[0-9]{15}, i.e. 1+16+1+15
  uint8_t raw_buffer[33] = { 0 };
  az_span buff = AZ_SPAN_FROM_BUFFER(raw_buffer);
  az_span o = AZ_SPAN_EMPTY;

  assert_int_equal(az_span_dtoa(buff, 9007199254740992, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, (double)9007199254740993, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, (double)45035996273704961, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, 2147483647 * (double)4294967298, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, -2147483647 * (double)4294967298, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, (double)1844674407370955100, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, (double)1.844674407370955e+19, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, (double)1.8446744073709551e+19, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, (double)1.8446744073709552e+19, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, (double)18446744073709551615UL, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(
      az_span_dtoa(buff, 18446744073709551615.18446744073709551615, 15, &o),
      AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, 1e16, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, 12345.123e15, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, -12345.123e15, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, 1e300, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, -1e300, 15, &o), AZ_ERROR_NOT_SUPPORTED);
  assert_int_equal(az_span_dtoa(buff, 1.7e308, 15, &o), AZ_ERROR_NOT_SUPPORTED);
}

static void az_span_copy_empty(void** state)
{
  (void)state;
  uint8_t buff[10];
  az_span dst = AZ_SPAN_FROM_BUFFER(buff);
  assert_true(az_span_is_content_equal(az_span_copy(dst, AZ_SPAN_EMPTY), dst));
}

static void test_az_span_is_valid(void** state)
{
  (void)state;
  assert_true(_az_span_is_valid((az_span){ 0 }, 0, true));
  assert_false(_az_span_is_valid((az_span){ 0 }, 0, false));
  assert_false(_az_span_is_valid((az_span){ 0 }, 1, true));
  assert_false(_az_span_is_valid((az_span){ 0 }, 1, false));
  assert_false(_az_span_is_valid((az_span){ 0 }, -1, true));
  assert_false(_az_span_is_valid((az_span){ 0 }, -1, false));

  assert_true(_az_span_is_valid(AZ_SPAN_EMPTY, 0, true));
  assert_false(_az_span_is_valid(AZ_SPAN_EMPTY, 0, false));
  assert_false(_az_span_is_valid(AZ_SPAN_EMPTY, 1, true));
  assert_false(_az_span_is_valid(AZ_SPAN_EMPTY, 1, false));
  assert_false(_az_span_is_valid(AZ_SPAN_EMPTY, -1, true));
  assert_false(_az_span_is_valid(AZ_SPAN_EMPTY, -1, false));

  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR(""), 0, true));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR(""), 0, false));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR(""), 1, true));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR(""), 1, false));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR(""), -1, true));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR(""), -1, false));

  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 0, true));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 0, false));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 1, true));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 1, false));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 5, true));
  assert_true(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 5, false));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 6, true));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), 6, false));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), -1, true));
  assert_false(_az_span_is_valid(AZ_SPAN_FROM_STR("Hello"), -1, false));

  uint8_t* const max_ptr = (uint8_t*)~0;
  assert_true(_az_span_is_valid(az_span_create(max_ptr, 0), 0, false));
  assert_true(_az_span_is_valid(az_span_create(max_ptr, 0), 0, true));

  assert_false(_az_span_is_valid(az_span_create(max_ptr, 1), 0, false));
  assert_false(_az_span_is_valid(az_span_create(max_ptr, 1), 0, true));

  assert_true(_az_span_is_valid(az_span_create(max_ptr - 1, 1), 0, false));
  assert_true(_az_span_is_valid(az_span_create(max_ptr - 1, 1), 0, true));

  assert_false(_az_span_is_valid(az_span_create(max_ptr - 1, 2), 0, false));
  assert_false(_az_span_is_valid(az_span_create(max_ptr - 1, 2), 0, true));

  assert_false(_az_span_is_valid(az_span_create(max_ptr - 1, INT32_MAX), 0, false));
  assert_false(_az_span_is_valid(az_span_create(max_ptr - 1, INT32_MAX), 0, true));

  assert_true(_az_span_is_valid(az_span_create(max_ptr - INT32_MAX, INT32_MAX), 0, false));
  assert_true(_az_span_is_valid(az_span_create(max_ptr - INT32_MAX, INT32_MAX), 0, true));
}

static void test_az_span_overlap(void** state)
{
  (void)state;

  assert_false(
      _az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)30, 10)));
  assert_false(
      _az_span_overlap(az_span_create((uint8_t*)30, 10), az_span_create((uint8_t*)10, 10)));

  assert_false(
      _az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)20, 10)));
  assert_false(
      _az_span_overlap(az_span_create((uint8_t*)20, 10), az_span_create((uint8_t*)10, 10)));

  assert_false(_az_span_overlap(az_span_create((uint8_t*)10, 0), az_span_create((uint8_t*)10, 0)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)15, 0)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)15, 0), az_span_create((uint8_t*)10, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)10, 15)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 15), az_span_create((uint8_t*)10, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)15, 10), az_span_create((uint8_t*)10, 15)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 15), az_span_create((uint8_t*)15, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)5, 10)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)5, 10), az_span_create((uint8_t*)10, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)10, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)15, 10)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)15, 10), az_span_create((uint8_t*)10, 10)));

  assert_true(_az_span_overlap(az_span_create((uint8_t*)10, 10), az_span_create((uint8_t*)12, 5)));
  assert_true(_az_span_overlap(az_span_create((uint8_t*)12, 5), az_span_create((uint8_t*)10, 10)));
}

static void az_span_trim(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_FROM_STR("   abc   "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("abc")));
}

static void az_span_trim_left(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_start(AZ_SPAN_FROM_STR("   abc   "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("abc   ")));
}

static void az_span_trim_right(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_end(AZ_SPAN_FROM_STR("   abc   "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("   abc")));
}

static void az_span_trim_all_whitespace(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_FROM_STR("\t\n\r       "));
  assert_int_equal(az_span_size(source), 0);
}

static void az_span_trim_none(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_FROM_STR("abc"));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("abc")));
}

static void az_span_trim_spaced(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_FROM_STR("\ta\n b     c    "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("a\n b     c")));
}

static void az_span_trim_zero(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_FROM_STR(""));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("")));
}

static void az_span_trim_null(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace(AZ_SPAN_EMPTY);
  assert_int_equal(az_span_size(source), 0);
}

static void az_span_trim_start(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_start(AZ_SPAN_EMPTY);
  assert_int_equal(az_span_size(source), 0);
}

static void az_span_trim_end(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_end(AZ_SPAN_FROM_STR("\ta\n b     c    "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("\ta\n b     c")));
}

static void az_span_trim_unicode(void** state)
{
  (void)state;
  az_span source
      = _az_span_trim_whitespace_from_end(AZ_SPAN_FROM_STR("  \\U+00A0a\n b     c\\U+2028    "));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("  \\U+00A0a\n b     c\\U+2028")));
}

static void az_span_trim_two_calls(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_start(
      _az_span_trim_whitespace_from_end(AZ_SPAN_FROM_STR("  \\U+00A0a\n b     c\\U+2028    ")));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("\\U+00A0a\n b     c\\U+2028")));
}

static void az_span_trim_two_calls_inverse(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_end(
      _az_span_trim_whitespace_from_start(AZ_SPAN_FROM_STR("  \\U+00A0a\n b     c\\U+2028    ")));
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("\\U+00A0a\n b     c\\U+2028")));
}

static void az_span_trim_repeat_calls(void** state)
{
  (void)state;
  az_span source = _az_span_trim_whitespace_from_end(
      _az_span_trim_whitespace_from_start(AZ_SPAN_FROM_STR("  1234    ")));
  source = _az_span_trim_whitespace(source);
  source = _az_span_trim_whitespace(source);
  source = _az_span_trim_whitespace(source);
  assert_true(az_span_is_content_equal(source, AZ_SPAN_FROM_STR("1234")));
}

static void test_az_span_token_success(void** state)
{
  (void)state;
  az_span span = AZ_SPAN_FROM_STR("abcdefgabcdefgabcdefg");
  az_span delim = AZ_SPAN_FROM_STR("abc");
  az_span token;
  az_span out_span;
  int32_t index = 0;

  // token: ""
  token = _az_span_token(span, delim, &out_span, &index);
  assert_int_equal(index, 0);
  assert_non_null(az_span_ptr(token));
  assert_true(az_span_size(token) == 0);
  assert_true(az_span_ptr(out_span) == (az_span_ptr(span) + az_span_size(delim)));
  assert_true(az_span_size(out_span) == (az_span_size(span) - az_span_size(delim)));

  // token: "defg" (span+3)
  span = out_span;

  token = _az_span_token(span, delim, &out_span, &index);
  assert_int_equal(index, 4);
  assert_true(az_span_ptr(token) == az_span_ptr(span));
  assert_int_equal(az_span_size(token), 4);
  assert_true(
      az_span_ptr(out_span) == (az_span_ptr(span) + az_span_size(token) + az_span_size(delim)));
  assert_true(
      az_span_size(out_span) == (az_span_size(span) - az_span_size(token) - az_span_size(delim)));

  // token: "defg" (span+10)
  span = out_span;

  token = _az_span_token(span, delim, &out_span, &index);
  assert_int_equal(index, 4);
  assert_true(az_span_ptr(token) == az_span_ptr(span));
  assert_int_equal(az_span_size(token), 4);
  assert_true(
      az_span_ptr(out_span) == (az_span_ptr(span) + az_span_size(token) + az_span_size(delim)));
  assert_true(
      az_span_size(out_span) == (az_span_size(span) - az_span_size(token) - az_span_size(delim)));

  // token: "defg" (span+17)
  span = out_span;

  token = _az_span_token(span, delim, &out_span, &index);
  assert_int_equal(index, -1);
  assert_true(az_span_ptr(token) == az_span_ptr(span));
  assert_int_equal(az_span_size(token), 4);
  assert_true(az_span_size(out_span) == 0);
}

int test_az_span()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(az_span_slice_to_end_test),
    cmocka_unit_test(test_az_span_getters),
    cmocka_unit_test(az_single_char_ascii_lower_test),
    cmocka_unit_test(az_span_to_lower_test),
    cmocka_unit_test(az_span_to_str_test),
    cmocka_unit_test(test_az_span_is_content_equal),
    cmocka_unit_test(az_span_find_beginning_success),
    cmocka_unit_test(az_span_find_middle_success),
    cmocka_unit_test(az_span_find_end_success),
    cmocka_unit_test(az_span_find_source_target_identical_success),
    cmocka_unit_test(az_span_find_not_found_fail),
    cmocka_unit_test(az_span_find_error_cases_fail),
    cmocka_unit_test(az_span_find_target_longer_than_source_fails),
    cmocka_unit_test(az_span_find_target_overlap_continuation_of_source_fails),
    cmocka_unit_test(az_span_find_target_more_chars_than_prefix_of_source_fails),
    cmocka_unit_test(az_span_find_overlapping_target_success),
    cmocka_unit_test(az_span_find_embedded_NULLs_success),
    cmocka_unit_test(az_span_find_capacity_checks_success),
    cmocka_unit_test(az_span_find_overlapping_checks_success),
    cmocka_unit_test(az_span_atox_return_errors),
    cmocka_unit_test(az_span_atou32_test),
    cmocka_unit_test(az_span_atoi32_test),
    cmocka_unit_test(az_span_atou64_test),
    cmocka_unit_test(az_span_atoi64_test),
    cmocka_unit_test(test_az_isfinite),
    cmocka_unit_test(az_span_atod_test),
    cmocka_unit_test(az_span_atod_non_finite_not_allowed),
    cmocka_unit_test(az_span_ato_number_whitespace_or_invalid_not_allowed),
    cmocka_unit_test(az_span_ato_number_no_out_of_bounds_reads),
    cmocka_unit_test(az_span_i64toa_negative_number_test),
    cmocka_unit_test(az_span_i64toa_test),
    cmocka_unit_test(az_span_test_macro_only_allows_byte_buffers),
    cmocka_unit_test(az_span_create_from_str_succeeds),
    cmocka_unit_test(az_span_copy_uint8_succeeds),
    cmocka_unit_test(az_span_i32toa_succeeds),
    cmocka_unit_test(az_span_i32toa_negative_succeeds),
    cmocka_unit_test(az_span_i32toa_max_int_succeeds),
    cmocka_unit_test(az_span_i32toa_zero_succeeds),
    cmocka_unit_test(az_span_i32toa_overflow_fails),
    cmocka_unit_test(az_span_u32toa_succeeds),
    cmocka_unit_test(az_span_u32toa_zero_succeeds),
    cmocka_unit_test(az_span_u32toa_max_uint_succeeds),
    cmocka_unit_test(az_span_u32toa_overflow_fails),
    cmocka_unit_test(az_span_dtoa_succeeds),
    cmocka_unit_test(az_span_dtoa_overflow_fails),
    cmocka_unit_test(az_span_dtoa_too_large),
    cmocka_unit_test(az_span_copy_empty),
    cmocka_unit_test(test_az_span_is_valid),
    cmocka_unit_test(test_az_span_overlap),
    cmocka_unit_test(az_span_trim),
    cmocka_unit_test(az_span_trim_left),
    cmocka_unit_test(az_span_trim_right),
    cmocka_unit_test(az_span_trim_all_whitespace),
    cmocka_unit_test(az_span_trim_none),
    cmocka_unit_test(az_span_trim_spaced),
    cmocka_unit_test(az_span_trim_zero),
    cmocka_unit_test(az_span_trim_null),
    cmocka_unit_test(test_az_span_token_success),
    cmocka_unit_test(az_span_trim_start),
    cmocka_unit_test(az_span_trim_end),
    cmocka_unit_test(az_span_trim_unicode),
    cmocka_unit_test(az_span_trim_two_calls),
    cmocka_unit_test(az_span_trim_two_calls_inverse),
    cmocka_unit_test(az_span_trim_repeat_calls),
  };
  return cmocka_run_group_tests_name("az_core_span", tests, NULL, NULL);
}
