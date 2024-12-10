// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_common.h"
#include <az_test_log.h>
#include <az_test_precondition.h>
#include <az_test_span.h>
#include <azure/core/az_log.h>
#include <azure/core/az_precondition.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/iot/az_iot_common.h>
#include <azure/iot/internal/az_iot_common_internal.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#define TEST_SPAN_BUFFER_SIZE 256

// Properties
#define TEST_KEY "key"
#define TEST_KEY_ONE "key_one"
#define TEST_KEY_TWO "key_two"
#define TEST_KEY_THREE "key_three"
#define TEST_VALUE_ONE "value_one"
#define TEST_VALUE_TWO "value_two"
#define TEST_VALUE_THREE "value_three"
#define TEST_KEY_VALUE_ONE "key_one=value_one"
#define TEST_KEY_VALUE_TWO "key_one=value_one&key_two=value_two"
#define TEST_KEY_VALUE_SUBSTRING "key_one=value_one&key=value_two"
#define TEST_KEY_VALUE_SAME "key_one=key&key=value_two"
#define TEST_KEY_VALUE_THREE "key_one=value_one&key_two=value_two&key_three=value_three"

static const az_span test_key = AZ_SPAN_LITERAL_FROM_STR(TEST_KEY);
static const az_span test_key_one = AZ_SPAN_LITERAL_FROM_STR(TEST_KEY_ONE);
static const az_span test_key_two = AZ_SPAN_LITERAL_FROM_STR(TEST_KEY_TWO);
static const az_span test_key_three = AZ_SPAN_LITERAL_FROM_STR(TEST_KEY_THREE);
static const az_span test_value_one = AZ_SPAN_LITERAL_FROM_STR(TEST_VALUE_ONE);
static const az_span test_value_two = AZ_SPAN_LITERAL_FROM_STR(TEST_VALUE_TWO);
static const az_span test_value_three = AZ_SPAN_LITERAL_FROM_STR(TEST_VALUE_THREE);

static const char test_correct_one_key_value[] = "key_one=value_one";
static const char test_correct_two_key_value[] = "key_one=value_one&key_two=value_two";

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()

static void test_az_iot_message_properties_init_NULL_props_fails(void** state)
{
  (void)state;

  uint8_t test_span_buf[TEST_SPAN_BUFFER_SIZE];
  az_span test_span = az_span_for_test_init(test_span_buf, _az_COUNTOF(test_span_buf));

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_init(NULL, test_span, 0));
}

static void test_az_iot_message_properties_init_user_set_params_too_many_written_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_ONE);
  az_iot_message_properties props;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span) + 1));
}

static void test_az_iot_message_properties_init_user_set_params_negative_written_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_ONE);
  az_iot_message_properties props;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_init(&props, test_span, -1));
}

static void test_az_iot_message_properties_append_get_NULL_props_fails(void** state)
{
  (void)state;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_append(NULL, test_key_one, test_value_one));
}

static void test_az_iot_message_properties_append_NULL_name_span_fails(void** state)
{
  (void)state;

  az_iot_message_properties props;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_message_properties_append(&props, AZ_SPAN_EMPTY, test_value_one));
}

static void test_az_iot_message_properties_append_NULL_value_span_fails(void** state)
{
  (void)state;

  az_iot_message_properties props;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_message_properties_append(&props, test_key_one, AZ_SPAN_EMPTY));
}

static void test_az_iot_message_properties_find_NULL_props_fail(void** state)
{
  (void)state;

  az_span out_value;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_find(NULL, test_key_one, &out_value));
}

static void test_az_iot_message_properties_find_NULL_name_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;

  az_span out_value;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_find(&props, AZ_SPAN_EMPTY, &out_value));
}

static void test_az_iot_message_properties_find_NULL_value_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_find(&props, test_key_one, NULL));
}

static void test_az_iot_message_properties_next_NULL_props_fail(void** state)
{
  (void)state;

  az_span name;
  az_span value;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_next(NULL, &name, &value));
}

static void test_az_iot_message_properties_next_NULL_out_name_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;
  az_span value;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_next(&props, NULL, &value));
}

static void test_az_iot_message_properties_next_NULL_out_value_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;
  az_span name;

  ASSERT_PRECONDITION_CHECKED(az_iot_message_properties_next(&props, &name, NULL));
}

static void test_az_iot_message_properties_next_written_less_than_size_succeed(void** state)
{
  (void)state;
#define STRSIZE sizeof(TEST_KEY_VALUE_THREE) / sizeof(TEST_KEY_VALUE_THREE[0])
  uint8_t buffer[STRSIZE * 2];

  az_span test_span = AZ_SPAN_FROM_BUFFER(buffer);

  az_span_copy(test_span, az_span_create_from_str(TEST_KEY_VALUE_THREE));

  az_iot_message_properties props;
  assert_int_equal(az_iot_message_properties_init(&props, test_span, STRSIZE), AZ_OK);

  az_span name;
  az_span value;

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_one), (size_t)az_span_size(test_key_one));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_one), (size_t)az_span_size(test_value_one));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_two), (size_t)az_span_size(test_key_two));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_three), (size_t)az_span_size(test_key_three));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_three), (size_t)az_span_size(test_value_three));

  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
  // Call again to show subsequent calls do nothing
  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void test_az_iot_u32toa_size_success()
{
  assert_int_equal(_az_iot_u32toa_size(0), 1);
  assert_int_equal(_az_iot_u32toa_size(9), 1);
  assert_int_equal(_az_iot_u32toa_size(10), 2);
  assert_int_equal(_az_iot_u32toa_size(99), 2);
  assert_int_equal(_az_iot_u32toa_size(100), 3);
  assert_int_equal(_az_iot_u32toa_size(199), 3);
  assert_int_equal(_az_iot_u32toa_size(1000), 4);
  assert_int_equal(_az_iot_u32toa_size(1999), 4);
  assert_int_equal(_az_iot_u32toa_size(10000), 5);
  assert_int_equal(_az_iot_u32toa_size(19999), 5);
  assert_int_equal(_az_iot_u32toa_size(100000), 6);
  assert_int_equal(_az_iot_u32toa_size(199999), 6);
  assert_int_equal(_az_iot_u32toa_size(1000000), 7);
  assert_int_equal(_az_iot_u32toa_size(1999999), 7);
  assert_int_equal(_az_iot_u32toa_size(10000000), 8);
  assert_int_equal(_az_iot_u32toa_size(19999999), 8);
  assert_int_equal(_az_iot_u32toa_size(100000000), 9);
  assert_int_equal(_az_iot_u32toa_size(199999999), 9);
  assert_int_equal(_az_iot_u32toa_size(1000000000), 10);
  assert_int_equal(_az_iot_u32toa_size(4294967295), 10);
}

static void test_az_iot_u64toa_size_success()
{
  assert_int_equal(_az_iot_u64toa_size(0), 1);
  assert_int_equal(_az_iot_u64toa_size(9), 1);
  assert_int_equal(_az_iot_u64toa_size(10), 2);
  assert_int_equal(_az_iot_u64toa_size(99), 2);
  assert_int_equal(_az_iot_u64toa_size(10000000000ul), 11);
  assert_int_equal(_az_iot_u64toa_size(19999999999ul), 11);
  assert_int_equal(_az_iot_u64toa_size(1000000000000000000ul), 19);
  assert_int_equal(_az_iot_u64toa_size(1999999999999999999ul), 19);
  assert_int_equal(_az_iot_u64toa_size(10000000000000000000ul), 20);
  assert_int_equal(_az_iot_u64toa_size(18446744073709551615ul), 20);
}

static void test_az_iot_is_status_succeeded_translate_success()
{
  assert_true(az_iot_status_succeeded(AZ_IOT_STATUS_OK));
  assert_true(az_iot_status_succeeded(AZ_IOT_STATUS_NO_CONTENT));
  assert_true(az_iot_status_succeeded(0));
  assert_true(az_iot_status_succeeded(350));

  assert_false(az_iot_status_succeeded(AZ_IOT_STATUS_BAD_REQUEST));
  assert_false(az_iot_status_succeeded(AZ_IOT_STATUS_TIMEOUT));
  assert_false(az_iot_status_succeeded(600));
}

static void test_az_iot_status_retriable_translate_success()
{
  assert_true(az_iot_status_retriable(AZ_IOT_STATUS_THROTTLED));
  assert_true(az_iot_status_retriable(AZ_IOT_STATUS_SERVER_ERROR));

  assert_false(az_iot_status_retriable(AZ_IOT_STATUS_OK));
  assert_false(az_iot_status_retriable(AZ_IOT_STATUS_UNAUTHORIZED));
}

static void test_az_iot_calculate_retry_delay_common_timings_success()
{
  assert_int_equal(2229, az_iot_calculate_retry_delay(5, 1, 500, 100000, 1234));
  assert_int_equal(321, az_iot_calculate_retry_delay(5000, 1, 500, 100000, 4321));

  // Operation already took more than the back-off interval.
  assert_int_equal(0, az_iot_calculate_retry_delay(10000, 1, 500, 100000, 4321));

  // Max retry exceeded.
  assert_int_equal(9995, az_iot_calculate_retry_delay(5, 5, 500, 10000, 4321));
}

static void test_az_iot_calculate_retry_delay_overflow_time_success()
{
  assert_int_equal(
      0,
      az_iot_calculate_retry_delay(
          INT32_MAX - 1, INT16_MAX - 1, INT32_MAX - 1, INT32_MAX - 1, INT32_MAX - 1));

  assert_int_equal(
      INT32_MAX - 1,
      az_iot_calculate_retry_delay(0, INT16_MAX - 1, INT32_MAX - 1, INT32_MAX - 1, INT32_MAX - 1));
}

static int _log_retry = 0;
static void _log_listener(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_IOT_RETRY:
      _log_retry++;
      assert_int_equal(az_span_size(message), 0);
      break;
    default:
      assert_true(false);
  }
}

static bool _should_write_iot_retry_only(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_IOT_RETRY:
      return true;
    default:
      return false;
  }
}

static bool _should_write_nothing(az_log_classification classification)
{
  (void)classification;
  return false;
}

static void test_az_iot_calculate_retry_delay_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_iot_retry_only);

  _log_retry = 0;
  assert_int_equal(2229, az_iot_calculate_retry_delay(5, 1, 500, 100000, 1234));
  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_retry);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void test_az_iot_calculate_retry_delay_no_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_nothing);

  _log_retry = 0;
  assert_int_equal(2229, az_iot_calculate_retry_delay(5, 1, 500, 100000, 1234));
  assert_int_equal(_az_BUILT_WITH_LOGGING(0, 0), _log_retry);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void test_az_span_copy_url_encode_succeed()
{
  az_span url_decoded_span = AZ_SPAN_FROM_STR("abc/=%012");

  uint8_t url_encoded[15];
  az_span url_encoded_span = AZ_SPAN_FROM_BUFFER(url_encoded);

  az_span remaining;

  uint8_t expected_result[] = "abc%2F%3D%25012";

  assert_int_equal(_az_span_copy_url_encode(url_encoded_span, url_decoded_span, &remaining), AZ_OK);
  assert_int_equal(az_span_size(remaining), 0);
  assert_int_equal(
      az_span_size(url_encoded_span) - az_span_size(remaining), _az_COUNTOF(expected_result) - 1);
}

static void test_az_span_copy_url_encode_insufficient_size_fail()
{
  az_span url_decoded_span = AZ_SPAN_FROM_STR("abc/=%012");

  uint8_t url_encoded[14]; // Needs 15 bytes, this will cause a failure (as expected by this test).
  az_span url_encoded_span = AZ_SPAN_FROM_BUFFER(url_encoded);

  az_span remaining;

  assert_int_equal(
      _az_span_copy_url_encode(url_encoded_span, url_decoded_span, &remaining),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_message_properties_init_succeed(void** state)
{
  (void)state;

  uint8_t test_span_buf[TEST_SPAN_BUFFER_SIZE] = { 0 };
  az_span test_span = az_span_create(test_span_buf, sizeof(test_span_buf));
  az_iot_message_properties props;

  assert_int_equal(az_iot_message_properties_init(&props, test_span, 0), AZ_OK);
  assert_int_equal(props._internal.current_property_index, 0);
}

static void test_az_iot_message_properties_init_user_set_params_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_ONE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  assert_memory_equal(
      az_span_ptr(props._internal.properties_buffer),
      test_correct_one_key_value,
      sizeof(test_correct_one_key_value) - 1);
}

static void test_az_iot_message_properties_append_succeed(void** state)
{
  (void)state;

  uint8_t test_span_buf[TEST_SPAN_BUFFER_SIZE];
  az_span test_span = az_span_for_test_init(test_span_buf, sizeof(test_span_buf));

  az_iot_message_properties props;
  assert_int_equal(az_iot_message_properties_init(&props, test_span, 0), AZ_OK);

  assert_int_equal(az_iot_message_properties_append(&props, test_key_one, test_value_one), AZ_OK);
  az_span_for_test_verify(
      az_span_slice(props._internal.properties_buffer, 0, props._internal.properties_written),
      test_correct_one_key_value,
      _az_COUNTOF(test_correct_one_key_value) - 1,
      az_span_create(test_span_buf, _az_COUNTOF(test_span_buf)),
      TEST_SPAN_BUFFER_SIZE);
}

static void test_az_iot_message_properties_append_empty_buffer_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;
  assert_int_equal(az_iot_message_properties_init(&props, AZ_SPAN_EMPTY, 0), AZ_OK);

  assert_int_equal(
      az_iot_message_properties_append(&props, test_key_one, test_value_one),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_message_properties_append_small_buffer_fail(void** state)
{
  (void)state;

  uint8_t test_span_buf[sizeof(test_correct_one_key_value) - 2];
  az_span test_span = az_span_create(test_span_buf, sizeof(test_span_buf));
  az_iot_message_properties props;

  assert_int_equal(az_iot_message_properties_init(&props, test_span, 0), AZ_OK);
  assert_int_equal(
      az_iot_message_properties_append(&props, test_key_one, test_value_one),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(
      az_span_size(props._internal.properties_buffer), sizeof(test_correct_one_key_value) - 2);
}

static void test_az_iot_message_properties_append_twice_succeed(void** state)
{
  (void)state;

  uint8_t test_span_buf[TEST_SPAN_BUFFER_SIZE];
  az_span test_span = az_span_for_test_init(test_span_buf, sizeof(test_span_buf));

  az_iot_message_properties props;
  assert_int_equal(az_iot_message_properties_init(&props, test_span, 0), AZ_OK);

  assert_int_equal(az_iot_message_properties_append(&props, test_key_one, test_value_one), AZ_OK);
  assert_int_equal(az_iot_message_properties_append(&props, test_key_two, test_value_two), AZ_OK);
  az_span_for_test_verify(
      az_span_slice(props._internal.properties_buffer, 0, props._internal.properties_written),
      test_correct_two_key_value,
      _az_COUNTOF(test_correct_two_key_value) - 1,
      az_span_create(test_span_buf, _az_COUNTOF(test_span_buf)),
      TEST_SPAN_BUFFER_SIZE);
}

static void test_az_iot_message_properties_append_twice_small_buffer_fail(void** state)
{
  (void)state;

  uint8_t test_span_buf[sizeof(test_correct_two_key_value) - 2];
  az_span test_span = az_span_create(test_span_buf, sizeof(test_span_buf));
  az_iot_message_properties props;

  assert_int_equal(az_iot_message_properties_init(&props, test_span, 0), AZ_OK);
  assert_int_equal(az_iot_message_properties_append(&props, test_key_one, test_value_one), AZ_OK);
  assert_int_equal(
      az_iot_message_properties_append(&props, test_key_two, test_value_two),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(props._internal.properties_written, sizeof(test_correct_one_key_value) - 1);
  assert_int_equal(
      az_span_size(props._internal.properties_buffer), sizeof(test_correct_two_key_value) - 2);
}

static void test_az_iot_message_properties_find_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_ONE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(az_iot_message_properties_find(&props, test_key_one, &out_value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(out_value), az_span_ptr(test_value_one), (size_t)az_span_size(test_value_one));
}

static void test_az_iot_message_properties_find_middle_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_THREE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(az_iot_message_properties_find(&props, test_key_two, &out_value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(out_value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));
}

static void test_az_iot_message_properties_find_end_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_TWO);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(az_iot_message_properties_find(&props, test_key_two, &out_value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(out_value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));
}

static void test_az_iot_message_properties_find_substring_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_SUBSTRING);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(az_iot_message_properties_find(&props, test_key, &out_value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(out_value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));
}

static void test_az_iot_message_properties_find_name_value_same_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_SAME);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(az_iot_message_properties_find(&props, test_key, &out_value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(out_value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));
}

static void test_az_iot_message_properties_find_empty_buffer_fail(void** state)
{
  (void)state;

  az_iot_message_properties props;

  assert_int_equal(az_iot_message_properties_init(&props, AZ_SPAN_EMPTY, 0), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, test_key_one, &out_value), AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_find_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_ONE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, AZ_SPAN_FROM_STR("key_foo"), &out_value),
      AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_find_substring_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_TWO);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, AZ_SPAN_FROM_STR("key"), &out_value),
      AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_find_substring_suffix_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_TWO);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, AZ_SPAN_FROM_STR("one"), &out_value),
      AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_find_value_match_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_THREE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, AZ_SPAN_FROM_STR("value_two"), &out_value),
      AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_find_value_match_end_fail(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_THREE);
  az_iot_message_properties props;

  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span out_value;
  assert_int_equal(
      az_iot_message_properties_find(&props, AZ_SPAN_FROM_STR("value_three"), &out_value),
      AZ_ERROR_ITEM_NOT_FOUND);
}

static void test_az_iot_message_properties_next_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_THREE);
  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span name;
  az_span value;

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_one), (size_t)az_span_size(test_key_one));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_one), (size_t)az_span_size(test_value_one));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_two), (size_t)az_span_size(test_key_two));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_three), (size_t)az_span_size(test_key_three));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_three), (size_t)az_span_size(test_value_three));

  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
  // Call again to show subsequent calls do nothing
  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
}

static void test_az_iot_message_properties_next_twice_succeed(void** state)
{
  (void)state;

  az_span test_span = az_span_create_from_str(TEST_KEY_VALUE_TWO);
  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  az_span name;
  az_span value;

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_one), (size_t)az_span_size(test_key_one));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_one), (size_t)az_span_size(test_value_one));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_two), (size_t)az_span_size(test_key_two));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));

  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);

  // Reset to beginning of span
  assert_int_equal(
      az_iot_message_properties_init(&props, test_span, az_span_size(test_span)), AZ_OK);

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_one), (size_t)az_span_size(test_key_one));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_one), (size_t)az_span_size(test_value_one));

  assert_int_equal(az_iot_message_properties_next(&props, &name, &value), AZ_OK);
  assert_memory_equal(
      az_span_ptr(name), az_span_ptr(test_key_two), (size_t)az_span_size(test_key_two));
  assert_memory_equal(
      az_span_ptr(value), az_span_ptr(test_value_two), (size_t)az_span_size(test_value_two));

  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
}

static void test_az_iot_message_properties_next_empty_succeed(void** state)
{
  (void)state;

  az_iot_message_properties props;
  assert_int_equal(az_iot_message_properties_init(&props, AZ_SPAN_EMPTY, 0), AZ_OK);

  az_span name;
  az_span value;

  assert_int_equal(
      az_iot_message_properties_next(&props, &name, &value), AZ_ERROR_IOT_END_OF_PROPERTIES);
}

#ifdef _MSC_VER
// warning C4113: 'void (__cdecl *)()' differs in parameter lists from 'CMUnitTestFunction'
#pragma warning(disable : 4113)
#endif

int test_az_iot_common()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_message_properties_init_NULL_props_fails),
    cmocka_unit_test(test_az_iot_message_properties_init_user_set_params_too_many_written_fail),
    cmocka_unit_test(test_az_iot_message_properties_init_user_set_params_negative_written_fail),
    cmocka_unit_test(test_az_iot_message_properties_append_get_NULL_props_fails),
    cmocka_unit_test(test_az_iot_message_properties_append_NULL_name_span_fails),
    cmocka_unit_test(test_az_iot_message_properties_append_NULL_value_span_fails),
    cmocka_unit_test(test_az_iot_message_properties_find_NULL_props_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_NULL_name_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_NULL_value_fail),
    cmocka_unit_test(test_az_iot_message_properties_next_NULL_props_fail),
    cmocka_unit_test(test_az_iot_message_properties_next_NULL_out_name_fail),
    cmocka_unit_test(test_az_iot_message_properties_next_NULL_out_value_fail),
    cmocka_unit_test(test_az_iot_message_properties_next_written_less_than_size_succeed),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_u32toa_size_success),
    cmocka_unit_test(test_az_iot_u64toa_size_success),
    cmocka_unit_test(test_az_iot_is_status_succeeded_translate_success),
    cmocka_unit_test(test_az_iot_status_retriable_translate_success),
    cmocka_unit_test(test_az_iot_calculate_retry_delay_common_timings_success),
    cmocka_unit_test(test_az_iot_calculate_retry_delay_overflow_time_success),
    cmocka_unit_test(test_az_iot_calculate_retry_delay_logging_succeed),
    cmocka_unit_test(test_az_iot_calculate_retry_delay_no_logging_succeed),
    cmocka_unit_test(test_az_span_copy_url_encode_succeed),
    cmocka_unit_test(test_az_span_copy_url_encode_insufficient_size_fail),
    cmocka_unit_test(test_az_iot_message_properties_init_succeed),
    cmocka_unit_test(test_az_iot_message_properties_init_user_set_params_succeed),
    cmocka_unit_test(test_az_iot_message_properties_append_succeed),
    cmocka_unit_test(test_az_iot_message_properties_append_empty_buffer_fail),
    cmocka_unit_test(test_az_iot_message_properties_append_small_buffer_fail),
    cmocka_unit_test(test_az_iot_message_properties_append_twice_succeed),
    cmocka_unit_test(test_az_iot_message_properties_append_twice_small_buffer_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_succeed),
    cmocka_unit_test(test_az_iot_message_properties_find_middle_succeed),
    cmocka_unit_test(test_az_iot_message_properties_find_end_succeed),
    cmocka_unit_test(test_az_iot_message_properties_find_substring_succeed),
    cmocka_unit_test(test_az_iot_message_properties_find_name_value_same_succeed),
    cmocka_unit_test(test_az_iot_message_properties_find_empty_buffer_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_substring_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_substring_suffix_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_value_match_fail),
    cmocka_unit_test(test_az_iot_message_properties_find_value_match_end_fail),
    cmocka_unit_test(test_az_iot_message_properties_next_succeed),
    cmocka_unit_test(test_az_iot_message_properties_next_twice_succeed),
    cmocka_unit_test(test_az_iot_message_properties_next_empty_succeed),
  };
  return cmocka_run_group_tests_name("az_iot_common", tests, NULL, NULL);
}
