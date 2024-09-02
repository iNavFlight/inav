// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <azure/core/az_base64.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

static void az_base64_max_encode_test(void** state)
{
  (void)state;
  assert_int_equal(az_base64_get_max_encoded_size(1), 4);
  assert_int_equal(az_base64_get_max_encoded_size(2), 4);
  assert_int_equal(az_base64_get_max_encoded_size(3), 4);
  assert_int_equal(az_base64_get_max_encoded_size(4), 8);
  assert_int_equal(az_base64_get_max_encoded_size(5), 8);
  assert_int_equal(az_base64_get_max_encoded_size(30), 40);
  assert_int_equal(az_base64_get_max_encoded_size(1610612729), 2147483640);
  assert_int_equal(az_base64_get_max_encoded_size(1610612730), 2147483640);
  assert_int_equal(az_base64_get_max_encoded_size(1610612731), 2147483644);
  assert_int_equal(az_base64_get_max_encoded_size(1610612732), 2147483644);
  assert_int_equal(az_base64_get_max_encoded_size(1610612733), 2147483644);
}

static void az_base64_max_decode_test(void** state)
{
  (void)state;
  assert_int_equal(az_base64_get_max_decoded_size(1), 0);
  assert_int_equal(az_base64_get_max_decoded_size(2), 0);
  assert_int_equal(az_base64_get_max_decoded_size(3), 0);
  assert_int_equal(az_base64_get_max_decoded_size(4), 3);
  assert_int_equal(az_base64_get_max_decoded_size(5), 3);
  assert_int_equal(az_base64_get_max_decoded_size(30), 21);
  assert_int_equal(az_base64_get_max_decoded_size(40), 30);
  assert_int_equal(az_base64_get_max_decoded_size(1610612733), 1207959549);
  assert_int_equal(az_base64_get_max_decoded_size(INT32_MAX - 4), 1610612730);
  assert_int_equal(az_base64_get_max_decoded_size(INT32_MAX - 3), 1610612733);
  assert_int_equal(az_base64_get_max_decoded_size(INT32_MAX - 2), 1610612733);
  assert_int_equal(az_base64_get_max_decoded_size(INT32_MAX - 1), 1610612733);
  assert_int_equal(az_base64_get_max_decoded_size(INT32_MAX), 1610612733);
}

static void _az_base64_encode_test_helper(
    int32_t input_length,
    const char* expected,
    int32_t expected_length)
{
  uint8_t input_buffer[7];
  for (int i = 0; i < input_length; i++)
  {
    input_buffer[i] = (uint8_t)(i + 1);
  }

  az_span source = AZ_SPAN_FROM_BUFFER(input_buffer);

  uint8_t destination_buffer[12];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(
      az_base64_encode(destination, az_span_slice(source, 0, input_length), &bytes_written)));
  assert_int_equal(bytes_written, expected_length);

  char actual[13];
  az_span_to_str(actual, 13, az_span_slice(destination, 0, bytes_written));
  assert_string_equal(actual, expected);
}

static void az_base64_encode_test(void** state)
{
  (void)state;

  uint8_t input_buffer[1];
  input_buffer[0] = 0;
  az_span source = AZ_SPAN_FROM_BUFFER(input_buffer);

  uint8_t destination_buffer[4];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(az_base64_encode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 4);

  char actual[5];
  az_span_to_str(actual, 5, destination);
  assert_string_equal(actual, "AA==");

  _az_base64_encode_test_helper(1, "AQ==", 4);
  _az_base64_encode_test_helper(2, "AQI=", 4);
  _az_base64_encode_test_helper(3, "AQID", 4);
  _az_base64_encode_test_helper(4, "AQIDBA==", 8);
  _az_base64_encode_test_helper(5, "AQIDBAU=", 8);
  _az_base64_encode_test_helper(6, "AQIDBAUG", 8);
  _az_base64_encode_test_helper(7, "AQIDBAUGBw==", 12);
}

static void az_base64_encode_destination_small_test(void** state)
{
  (void)state;

  uint8_t input_buffer[10] = { 23, 51, 61, 250, 131, 184, 127, 228, 250, 66 };
  az_span source = AZ_SPAN_FROM_BUFFER(input_buffer);

  uint8_t destination_buffer[16];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_int_equal(
      az_base64_encode(az_span_slice(destination, 0, 4), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_encode(az_span_slice(destination, 0, 14), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_encode(az_span_slice(destination, 0, 15), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_true(az_result_succeeded(az_base64_encode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 16);

  char actual[17];
  az_span_to_str(actual, 17, destination);
  assert_string_equal(actual, "FzM9+oO4f+T6Qg==");
}

static void _az_base64_decode_test_helper(az_span source, az_span expected)
{
  uint8_t destination_buffer[7];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(az_base64_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, az_span_size(expected));

  assert_true(az_span_is_content_equal(az_span_slice(destination, 0, bytes_written), expected));
}

static void az_base64_decode_test(void** state)
{
  (void)state;

  az_span source = AZ_SPAN_FROM_STR("AA==");

  uint8_t destination_buffer[1];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(az_base64_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 1);

  uint8_t input_buffer[1] = { 0 };
  az_span expected = AZ_SPAN_FROM_BUFFER(input_buffer);
  assert_true(az_span_is_content_equal(destination, expected));

  uint8_t expected_buffer1[1] = { 1 };
  _az_base64_decode_test_helper(AZ_SPAN_FROM_STR("AQ=="), AZ_SPAN_FROM_BUFFER(expected_buffer1));
  uint8_t expected_buffer2[2] = { 1, 2 };
  _az_base64_decode_test_helper(AZ_SPAN_FROM_STR("AQI="), AZ_SPAN_FROM_BUFFER(expected_buffer2));
  uint8_t expected_buffer3[3] = { 1, 2, 3 };
  _az_base64_decode_test_helper(AZ_SPAN_FROM_STR("AQID"), AZ_SPAN_FROM_BUFFER(expected_buffer3));
  uint8_t expected_buffer4[4] = { 1, 2, 3, 4 };
  _az_base64_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBA=="), AZ_SPAN_FROM_BUFFER(expected_buffer4));
  uint8_t expected_buffer5[5] = { 1, 2, 3, 4, 5 };
  _az_base64_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAU="), AZ_SPAN_FROM_BUFFER(expected_buffer5));
  uint8_t expected_buffer6[6] = { 1, 2, 3, 4, 5, 6 };
  _az_base64_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAUG"), AZ_SPAN_FROM_BUFFER(expected_buffer6));
  uint8_t expected_buffer7[7] = { 1, 2, 3, 4, 5, 6, 7 };
  _az_base64_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAUGBw=="), AZ_SPAN_FROM_BUFFER(expected_buffer7));
}

static void az_base64_decode_destination_small_test(void** state)
{
  (void)state;

  uint8_t expected_buffer[10] = { 23, 51, 61, 250, 131, 184, 127, 228, 250, 66 };
  az_span source = AZ_SPAN_FROM_STR("FzM9+oO4f+T6Qg==");

  uint8_t destination_buffer[10];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 4), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 8), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 9), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_true(az_result_succeeded(az_base64_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 10);

  assert_true(az_span_is_content_equal(destination, AZ_SPAN_FROM_BUFFER(expected_buffer)));

  source = AZ_SPAN_FROM_STR("AQI=");
  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 1), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);

  source = AZ_SPAN_FROM_STR("AQID");
  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 2), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);

  source = AZ_SPAN_FROM_STR("AQIDBA==");
  assert_int_equal(
      az_base64_decode(az_span_slice(destination, 0, 3), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);
}

static void az_base64_decode_source_small_test(void** state)
{
  (void)state;

  uint8_t destination_buffer[10];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDB"), &bytes_written),
      AZ_ERROR_UNEXPECTED_END);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBA"), &bytes_written),
      AZ_ERROR_UNEXPECTED_END);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBA="), &bytes_written),
      AZ_ERROR_UNEXPECTED_END);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBAU"), &bytes_written),
      AZ_ERROR_UNEXPECTED_END);
  assert_int_equal(bytes_written, 0);
}

static void az_base64_decode_invalid_test(void** state)
{
  (void)state;

  // Invalid Bytes:
  // 0-42
  // 44-46
  // 58-64
  // 91-96
  // 123-255

  uint8_t destination_buffer[20];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("A---"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("A-=="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("A!B="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("A:BC"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQ-_"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQ=_"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQI_"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDB..."), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBA.."), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBA=|"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("AQIDBAU?"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("FzM9+oO4f+T6Qg==}}}}"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_decode(destination, AZ_SPAN_FROM_STR("\\zM9+oO4f+T6Qg=="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);
}

static void _az_base64_url_decode_test_helper(az_span source, az_span expected)
{
  uint8_t destination_buffer[7];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(az_base64_url_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, az_span_size(expected));

  assert_true(az_span_is_content_equal(az_span_slice(destination, 0, bytes_written), expected));
}

static void az_base64_url_decode_test(void** state)
{
  (void)state;

  az_span source = AZ_SPAN_FROM_STR("AA==");

  uint8_t destination_buffer[1];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_true(az_result_succeeded(az_base64_url_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 1);

  uint8_t input_buffer[1] = { 0 };
  az_span expected = AZ_SPAN_FROM_BUFFER(input_buffer);
  assert_true(az_span_is_content_equal(destination, expected));

  uint8_t expected_buffer1[1] = { 1 };
  _az_base64_url_decode_test_helper(AZ_SPAN_FROM_STR("AQ"), AZ_SPAN_FROM_BUFFER(expected_buffer1));
  uint8_t expected_buffer2[2] = { 1, 2 };
  _az_base64_url_decode_test_helper(AZ_SPAN_FROM_STR("AQI"), AZ_SPAN_FROM_BUFFER(expected_buffer2));
  uint8_t expected_buffer3[3] = { 1, 2, 3 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("AQID"), AZ_SPAN_FROM_BUFFER(expected_buffer3));
  uint8_t expected_buffer4[4] = { 1, 2, 3, 4 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBA"), AZ_SPAN_FROM_BUFFER(expected_buffer4));
  uint8_t expected_buffer5[5] = { 1, 2, 3, 4, 5 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAU"), AZ_SPAN_FROM_BUFFER(expected_buffer5));
  uint8_t expected_buffer6[6] = { 1, 2, 3, 4, 5, 6 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAUG"), AZ_SPAN_FROM_BUFFER(expected_buffer6));
  uint8_t expected_buffer7[7] = { 1, 2, 3, 4, 5, 6, 7 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("AQIDBAUGBw"), AZ_SPAN_FROM_BUFFER(expected_buffer7));
  uint8_t expected_buffer8[6] = { 0xF8, 0x01, 0x02, 0xFC, 0x03, 0x04 };
  _az_base64_url_decode_test_helper(
      AZ_SPAN_FROM_STR("-AEC_AME"), AZ_SPAN_FROM_BUFFER(expected_buffer8));
}

static void az_base64_url_decode_destination_small_test(void** state)
{
  (void)state;

  uint8_t expected_buffer[10] = { 23, 51, 61, 250, 131, 184, 127, 228, 250, 66 };
  az_span source = AZ_SPAN_FROM_STR("FzM9-oO4f-T6Qg==");

  uint8_t destination_buffer[10];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;
  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 4), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 8), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 9), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 0);

  assert_true(az_result_succeeded(az_base64_url_decode(destination, source, &bytes_written)));
  assert_int_equal(bytes_written, 10);

  assert_true(az_span_is_content_equal(destination, AZ_SPAN_FROM_BUFFER(expected_buffer)));

  source = AZ_SPAN_FROM_STR("AQI=");
  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 1), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);

  source = AZ_SPAN_FROM_STR("AQID");
  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 2), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);

  source = AZ_SPAN_FROM_STR("AQIDBA==");
  assert_int_equal(
      az_base64_url_decode(az_span_slice(destination, 0, 3), source, &bytes_written),
      AZ_ERROR_NOT_ENOUGH_SPACE);
  assert_int_equal(bytes_written, 10);
}

static void az_base64_url_decode_source_small_test(void** state)
{
  (void)state;

  uint8_t destination_buffer[10];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQIDB"), &bytes_written),
      AZ_ERROR_UNEXPECTED_END);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(bytes_written, 0);
}

static void az_base64_url_decode_invalid_test(void** state)
{
  (void)state;

  // Invalid Bytes:
  // 0-44
  // 46
  // 58-64
  // 91-94
  // 96
  // 123-255

  uint8_t destination_buffer[20];
  az_span destination = AZ_SPAN_FROM_BUFFER(destination_buffer);

  int32_t bytes_written = 0;

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("A+++"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("A+=="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("A!B="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("A:BC"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQ+/"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQ=/"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQI/"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQIDB..."), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQIDBA.."), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQIDBA=|"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("AQIDBAU?"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("FzM9+oO4f+T6Qg==}}}}"), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);

  assert_int_equal(
      az_base64_url_decode(destination, AZ_SPAN_FROM_STR("\\zM9+oO4f+T6Qg=="), &bytes_written),
      AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(bytes_written, 0);
}

int test_az_base64()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(az_base64_max_encode_test),
    cmocka_unit_test(az_base64_max_decode_test),
    cmocka_unit_test(az_base64_encode_test),
    cmocka_unit_test(az_base64_encode_destination_small_test),
    cmocka_unit_test(az_base64_decode_test),
    cmocka_unit_test(az_base64_decode_destination_small_test),
    cmocka_unit_test(az_base64_decode_source_small_test),
    cmocka_unit_test(az_base64_decode_invalid_test),
    cmocka_unit_test(az_base64_url_decode_test),
    cmocka_unit_test(az_base64_url_decode_destination_small_test),
    cmocka_unit_test(az_base64_url_decode_source_small_test),
    cmocka_unit_test(az_base64_url_decode_invalid_test),
  };
  return cmocka_run_group_tests_name("az_core_base64", tests, NULL, NULL);
}
