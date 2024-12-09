// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <az_http_policy_logging_private.h>
#include <az_http_private.h>
#include <az_test_log.h>
#include <azure/core/az_context.h>
#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_log.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_log_internal.h>

#include <setjmp.h>
#include <stdarg.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

#define TEST_EXPECT_SUCCESS(exp) assert_true(az_result_succeeded(exp))

static bool _log_invoked_for_http_request = false;
static bool _log_invoked_for_http_response = false;

static inline void _reset_log_invocation_status()
{
  _log_invoked_for_http_request = false;
  _log_invoked_for_http_response = false;
}

static void _log_listener(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      _log_invoked_for_http_request = true;
      assert_true(az_span_is_content_equal(
          message,
          AZ_SPAN_FROM_STR("HTTP Request : GET https://www.example.com\n"
                           "\tHeader1 : Value1\n"
                           "\tHeader2 : ZZZZYYYYXXXXWWWWVVVVUU ... SSSRRRRQQQQPPPPOOOONNNN\n"
                           "\tHeader3 : 1111112222223333334444 ... 55666666777777888888abc\n"
                           "\tauthorization")));
      break;
    case AZ_LOG_HTTP_RESPONSE:
      _log_invoked_for_http_response = true;
      assert_true(az_span_is_content_equal(
          message,
          AZ_SPAN_FROM_STR("HTTP Response (3456ms) : 404 Not Found\n"
                           "\tHeader11 : Value11\n"
                           "\tHeader22 : NNNNOOOOPPPPQQQQRRRRSS ... UUUVVVVWWWWXXXXYYYYZZZZ\n"
                           "\tHeader33\n"
                           "\tHeader44 : cba8888887777776666665 ... 44444333333222222111111\n"
                           "\n"
                           " -> HTTP Request : GET https://www.example.com\n"
                           "\tHeader1 : Value1\n"
                           "\tHeader2 : ZZZZYYYYXXXXWWWWVVVVUU ... SSSRRRRQQQQPPPPOOOONNNN\n"
                           "\tHeader3 : 1111112222223333334444 ... 55666666777777888888abc\n"
                           "\tauthorization")));
      break;
    default:
      assert_true(false);
      break;
  }
}

static bool _should_write_everything_valid(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_HTTP_RETRY:
    case AZ_LOG_HTTP_RESPONSE:
    case AZ_LOG_HTTP_REQUEST:
      return true;
    default:
      return false;
  }
}

static bool _should_write_http_request_only(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      return true;
    default:
      return false;
  }
}

static void _log_listener_NULL(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      _log_invoked_for_http_request = true;
      assert_true(az_span_is_content_equal(message, AZ_SPAN_FROM_STR("HTTP Request : NULL")));
      break;
    case AZ_LOG_HTTP_RESPONSE:
      _log_invoked_for_http_response = true;
      assert_true(az_span_is_content_equal(message, AZ_SPAN_FROM_STR("")));
      break;
    default:
      assert_true(false);
      break;
  }
}

static void test_az_log(void** state)
{
  (void)state;
  // Set up test values etc.
  uint8_t headers[4 * 1024] = { 0 };
  az_http_request request = { 0 };
  az_span url = AZ_SPAN_FROM_STR("https://www.example.com");
  TEST_EXPECT_SUCCESS(az_http_request_init(
      &request,
      &az_context_application,
      az_http_method_get(),
      url,
      az_span_size(url),
      AZ_SPAN_FROM_BUFFER(headers),
      AZ_SPAN_FROM_STR("AAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHHIIIIIJJJJJKKKKK")));

  TEST_EXPECT_SUCCESS(az_http_request_append_header(
      &request, AZ_SPAN_FROM_STR("Header1"), AZ_SPAN_FROM_STR("Value1")));

  TEST_EXPECT_SUCCESS(az_http_request_append_header(
      &request,
      AZ_SPAN_FROM_STR("Header2"),
      AZ_SPAN_FROM_STR("ZZZZYYYYXXXXWWWWVVVVUUUUTTTTSSSSRRRRQQQQPPPPOOOONNNN")));

  TEST_EXPECT_SUCCESS(_az_http_request_mark_retry_headers_start(&request));

  TEST_EXPECT_SUCCESS(az_http_request_append_header(
      &request,
      AZ_SPAN_FROM_STR("Header3"),
      AZ_SPAN_FROM_STR("111111222222333333444444555555666666777777888888abc")));

  TEST_EXPECT_SUCCESS(az_http_request_append_header(
      &request, AZ_SPAN_FROM_STR("authorization"), AZ_SPAN_FROM_STR("BigSecret!")));

  uint8_t response_buf[1024] = { 0 };
  az_span response_buf_span = AZ_SPAN_FROM_BUFFER(response_buf);

  az_span response_span
      = AZ_SPAN_FROM_STR("HTTP/1.1 404 Not Found\r\n"
                         "Header11: Value11\r\n"
                         "Header22: NNNNOOOOPPPPQQQQRRRRSSSSTTTTUUUUVVVVWWWWXXXXYYYYZZZZ\r\n"
                         "Header33:\r\n"
                         "Header44: cba888888777777666666555555444444333333222222111111\r\n"
                         "\r\n"
                         "KKKKKJJJJJIIIIIHHHHHGGGGGFFFFFEEEEEDDDDDCCCCCBBBBBAAAAA");
  az_span_copy(response_buf_span, response_span);
  response_buf_span = az_span_slice(response_buf_span, 0, az_span_size(response_span));
  assert_int_equal(az_span_size(response_buf_span), az_span_size(response_span));

  az_http_response response = { 0 };
  TEST_EXPECT_SUCCESS(az_http_response_init(&response, response_buf_span));
  // Finish setting up

  {
    // null request
    _reset_log_invocation_status();
    az_log_set_message_callback(_log_listener_NULL);
    az_log_set_classification_filter_callback(_should_write_everything_valid);
    _az_http_policy_logging_log_http_request(NULL);
    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
    assert_true(_log_invoked_for_http_response == false);
  }
  // Actual test below
  {
    // Verify that log callback gets invoked, and with the correct classification type.
    // Also, our callback function does the verification for the message content.
    _reset_log_invocation_status();
    az_log_set_message_callback(_log_listener);
    az_log_set_classification_filter_callback(_should_write_everything_valid);
    assert_true(_log_invoked_for_http_request == false);
    assert_true(_log_invoked_for_http_response == false);

    _az_http_policy_logging_log_http_request(&request);
    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
    assert_true(_log_invoked_for_http_response == false);

    _az_http_policy_logging_log_http_response(&response, 3456, &request);
    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
    assert_true(_log_invoked_for_http_response == _az_BUILT_WITH_LOGGING(true, false));
  }
  {
    _reset_log_invocation_status();
    az_log_set_message_callback(NULL);
    az_log_set_classification_filter_callback(NULL);

    // Verify that user can unset log callback, and we are not going to call the previously set one.
    assert_true(_log_invoked_for_http_request == false);
    assert_true(_log_invoked_for_http_response == false);

    _az_http_policy_logging_log_http_request(&request);
    _az_http_policy_logging_log_http_response(&response, 3456, &request);

    assert_true(_log_invoked_for_http_request == false);
    assert_true(_log_invoked_for_http_response == false);

    {
      // Verify that our internal should_write() function would return false if none is listening.
      assert_true(_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST) == false);
      assert_true(_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_RESPONSE) == false);

      // If a callback is set, and no classification filter callback is specified, we are going to
      // log all of them (and customer is going to get all of them).
      az_log_set_message_callback(_log_listener);
      az_log_set_classification_filter_callback(NULL);

      assert_true(_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST) == _az_BUILT_WITH_LOGGING(true, false));

      assert_true(
          _az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_RESPONSE) == _az_BUILT_WITH_LOGGING(true, false));
    }

    // Verify that if customer overrides the classification filter callback, we'll only invoke the
    // logging callback with the classification that it allows, and nothing is going to happen when
    // our code attempts to log a classification that it doesn't.
    az_log_set_classification_filter_callback(_should_write_http_request_only);

    assert_true(_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST) == _az_BUILT_WITH_LOGGING(true, false));
    assert_true(_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_RESPONSE) == false);

    _az_http_policy_logging_log_http_request(&request);
    _az_http_policy_logging_log_http_response(&response, 3456, &request);

    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
    assert_true(_log_invoked_for_http_response == false);
  }

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void _log_listener_stop_logging_corrupted_response(
    az_log_classification classification,
    az_span message)
{
  (void)message;
  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      _log_invoked_for_http_request = true;
      assert_string_equal(
          az_span_ptr(message),
          az_span_ptr(AZ_SPAN_FROM_STR("HTTP Request : GET https://www.example.com")));
      break;
    case AZ_LOG_HTTP_RESPONSE:
      _log_invoked_for_http_response = true;
      assert_string_equal(
          az_span_ptr(message),
          az_span_ptr(AZ_SPAN_FROM_STR("HTTP Response (3456ms) : 404 Not Found")));
      break;
    default:
      assert_true(false);
      break;
  }
}

static void test_az_log_corrupted_response(void** state)
{
  (void)state;
  uint8_t headers[1024] = { 0 };
  az_http_request request = { 0 };
  az_span url = AZ_SPAN_FROM_STR("https://www.example.com");
  TEST_EXPECT_SUCCESS(az_http_request_init(
      &request,
      &az_context_application,
      az_http_method_get(),
      url,
      az_span_size(url),
      AZ_SPAN_FROM_BUFFER(headers),
      AZ_SPAN_FROM_STR("AAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHHIIIIIJJJJJKKKKK")));

  az_span response_span = AZ_SPAN_FROM_STR("HTTP/1.1 404 Not Found\r\n"
                                           "key:\n");
  az_http_response response = { 0 };
  TEST_EXPECT_SUCCESS(az_http_response_init(&response, response_span));

  _reset_log_invocation_status();
  az_log_set_message_callback(_log_listener_stop_logging_corrupted_response);
  az_log_set_classification_filter_callback(NULL);
  assert_true(_log_invoked_for_http_request == false);
  assert_true(_log_invoked_for_http_response == false);

  _az_http_policy_logging_log_http_request(&request);
  assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
  assert_true(_log_invoked_for_http_response == false);

  _az_http_policy_logging_log_http_response(&response, 3456, &request);
  assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
  assert_true(_log_invoked_for_http_response == _az_BUILT_WITH_LOGGING(true, false));

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void _log_listener_no_op(az_log_classification classification, az_span message)
{
  assert_true(false);
  (void)classification;
  (void)message;
}

static bool _should_write_http_retry_only(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_HTTP_RETRY:
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

static void test_az_log_incorrect_list_fails_gracefully(void** state)
{
  (void)state;
  {
    az_log_set_message_callback(_log_listener_no_op);
    az_log_set_classification_filter_callback(_should_write_http_retry_only);

    assert_false(_az_LOG_SHOULD_WRITE((az_log_classification)12345));
    _az_LOG_WRITE((az_log_classification)12345, AZ_SPAN_EMPTY);

    az_log_set_message_callback(_log_listener_no_op);
    az_log_set_classification_filter_callback(_should_write_nothing);

    assert_false(_az_LOG_SHOULD_WRITE((az_log_classification)12345));
    _az_LOG_WRITE((az_log_classification)12345, AZ_SPAN_EMPTY);

    az_log_set_message_callback(NULL);
    az_log_set_classification_filter_callback(NULL);
  }
}

static int _number_of_log_attempts = 0;
static void _log_listener_count_logs(az_log_classification classification, az_span message)
{
  _number_of_log_attempts++;
  (void)classification;
  (void)message;
}

static void test_az_log_everything_valid(void** state)
{
  (void)state;
  {
    az_log_set_message_callback(_log_listener_count_logs);
    az_log_set_classification_filter_callback(_should_write_everything_valid);

    _number_of_log_attempts = 0;

    assert_true(_az_BUILT_WITH_LOGGING(true, false) == _az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST));
    assert_false(_az_LOG_SHOULD_WRITE((az_log_classification)12345));

    _az_LOG_WRITE(AZ_LOG_HTTP_REQUEST, AZ_SPAN_EMPTY);
    _az_LOG_WRITE((az_log_classification)12345, AZ_SPAN_EMPTY);

    assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _number_of_log_attempts);

    az_log_set_message_callback(NULL);
    az_log_set_classification_filter_callback(NULL);
  }
}

static void test_az_log_everything_on_null(void** state)
{
  (void)state;
  {
    az_log_set_message_callback(_log_listener_count_logs);
    az_log_set_classification_filter_callback(NULL);

    _number_of_log_attempts = 0;

    assert_true(_az_BUILT_WITH_LOGGING(true, false) == _az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST));
    assert_true(
        _az_BUILT_WITH_LOGGING(true, false) == _az_LOG_SHOULD_WRITE((az_log_classification)12345));

    _az_LOG_WRITE(AZ_LOG_HTTP_REQUEST, AZ_SPAN_EMPTY);
    _az_LOG_WRITE((az_log_classification)12345, AZ_SPAN_EMPTY);

    assert_int_equal(_az_BUILT_WITH_LOGGING(2, 0), _number_of_log_attempts);

    az_log_set_message_callback(NULL);
    az_log_set_classification_filter_callback(NULL);
  }
}

#define _az_TEST_LOG_URL_PREFIX "HTTP Request : GET "
#define _az_TEST_LOG_URL_PROTOCOL "https://"
#define _az_TEST_LOG_URL_HOST ".microsoft.com"

#define _az_TEST_LOG_MAX_URL_SIZE \
  (AZ_LOG_MESSAGE_BUFFER_SIZE - (sizeof(_az_TEST_LOG_URL_PREFIX) - 1))

static void _test_az_log_http_request_max_size_url_init(az_span url)
{
  int32_t url_size = az_span_size(url);

  az_span protocol = AZ_SPAN_FROM_STR(_az_TEST_LOG_URL_PROTOCOL);
  az_span host = AZ_SPAN_FROM_STR(_az_TEST_LOG_URL_HOST);

  url = az_span_copy(url, protocol);

  for (int i = 0; i < (url_size - (az_span_size(protocol) + az_span_size(host))); ++i)
  {
    url = az_span_copy_u8(url, (uint8_t)'w');
  }

  az_span_copy(url, host);
}

static void _max_buf_size_log_listener(az_log_classification classification, az_span message)
{
  uint8_t expected_msg_buf[AZ_LOG_MESSAGE_BUFFER_SIZE] = { 0 };
  az_span expected_msg = AZ_SPAN_FROM_BUFFER(expected_msg_buf);

  _test_az_log_http_request_max_size_url_init(
      az_span_copy(expected_msg, AZ_SPAN_FROM_STR(_az_TEST_LOG_URL_PREFIX)));

  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      _log_invoked_for_http_request = true;
      assert_true(az_span_is_content_equal(message, expected_msg));

      break;

    default:
      assert_true(false);
      break;
  }
}

static void _toobig_buf_size_log_listener(az_log_classification classification, az_span message)
{
  uint8_t expected_msg_buf[AZ_LOG_MESSAGE_BUFFER_SIZE] = { 0 };
  az_span expected_msg = AZ_SPAN_FROM_BUFFER(expected_msg_buf);

  switch (classification)
  {
    case AZ_LOG_HTTP_REQUEST:
      _log_invoked_for_http_request = true;
      assert_true(az_span_is_content_equal(message, expected_msg));

      break;

    default:
      assert_true(false);
      break;
  }
}

static void test_az_log_http_request_buffer_size(void** state)
{
  (void)state;

  _reset_log_invocation_status();
  az_log_set_message_callback(_max_buf_size_log_listener);
  {
    uint8_t max_url_buf[_az_TEST_LOG_MAX_URL_SIZE] = { 0 };
    az_span max_url = AZ_SPAN_FROM_BUFFER(max_url_buf);
    _test_az_log_http_request_max_size_url_init(max_url);

    az_http_request request = { 0 };
    TEST_EXPECT_SUCCESS(az_http_request_init(
        &request,
        &az_context_application,
        az_http_method_get(),
        max_url,
        az_span_size(max_url),
        AZ_SPAN_FROM_STR(""),
        AZ_SPAN_EMPTY));

    _az_http_policy_logging_log_http_request(&request);
    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
  }

  _reset_log_invocation_status();
  az_log_set_message_callback(_toobig_buf_size_log_listener);
  {
    uint8_t toobig_url_buf[_az_TEST_LOG_MAX_URL_SIZE + 1] = { 0 };
    az_span toobig_url = AZ_SPAN_FROM_BUFFER(toobig_url_buf);
    _test_az_log_http_request_max_size_url_init(toobig_url);

    az_http_request request = { 0 };
    TEST_EXPECT_SUCCESS(az_http_request_init(
        &request,
        &az_context_application,
        az_http_method_get(),
        toobig_url,
        az_span_size(toobig_url),
        AZ_SPAN_FROM_STR(""),
        AZ_SPAN_EMPTY));

    _az_http_policy_logging_log_http_request(&request);
    assert_true(_log_invoked_for_http_request == _az_BUILT_WITH_LOGGING(true, false));
  }

  _reset_log_invocation_status();
  az_log_set_message_callback(NULL);
}

#undef _az_TEST_LOG_URL_PREFIX
#undef _az_TEST_LOG_URL_PROTOCOL
#undef _az_TEST_LOG_URL_HOST
#undef _az_TEST_LOG_MAX_URL_SIZE

int test_az_logging()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_az_log),
    cmocka_unit_test(test_az_log_corrupted_response),
    cmocka_unit_test(test_az_log_incorrect_list_fails_gracefully),
    cmocka_unit_test(test_az_log_everything_valid),
    cmocka_unit_test(test_az_log_everything_on_null),
    cmocka_unit_test(test_az_log_http_request_buffer_size),
  };
  return cmocka_run_group_tests_name("az_core_logging", tests, NULL, NULL);
}
