// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_hub_client.h"
#include <az_test_log.h>
#include <az_test_precondition.h>
#include <az_test_span.h>
#include <azure/core/az_log.h>
#include <azure/core/az_precondition.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/iot/az_iot_hub_client.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#define TEST_SPAN_BUFFER_SIZE 128

#define TEST_DEVICE_ID_STR "my_device"
#define TEST_DEVICE_HOSTNAME_STR "myiothub.azure-devices.net"

static const az_span test_device_hostname = AZ_SPAN_LITERAL_FROM_STR(TEST_DEVICE_HOSTNAME_STR);
static const az_span test_device_id = AZ_SPAN_LITERAL_FROM_STR(TEST_DEVICE_ID_STR);
static const az_span test_model_id
    = AZ_SPAN_LITERAL_FROM_STR("dtmi:YOUR_COMPANY_NAME_HERE:sample_device;1");
static uint8_t g_expected_methods_subscribe_topic[] = "$iothub/methods/POST/#";

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()

static void test_az_iot_hub_client_commands_response_get_publish_topic_NULL_client_fail()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_commands_response_get_publish_topic(
      NULL, request_id, status, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_commands_response_get_publish_topic_NULL_out_topic_fail()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_commands_response_get_publish_topic(
      &client, request_id, status, NULL, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_commands_response_get_publish_topic_zero_size_buffer_fail()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_commands_response_get_publish_topic(
      &client, request_id, status, test_buf, 0, &test_length));
}

static void test_az_iot_hub_client_commands_response_get_publish_topic_EMPTY_request_id_fail()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("");
  uint16_t status = 200;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_commands_response_get_publish_topic(
      &client, request_id, status, test_buf, sizeof(test_buf), &test_length));
}

static void
test_az_iot_hub_client_commands_response_get_publish_topic_AZ_SPAN_NULL_request_id_fail()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_EMPTY;
  uint16_t status = 200;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_commands_response_get_publish_topic(
      &client, request_id, status, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_commands_parse_received_topic_NULL_client_fail()
{
  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/methods/POST/TestMethod/?$rid=1");

  az_iot_hub_client_command_request out_request;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_commands_parse_received_topic(NULL, received_topic, &out_request));
}

static void test_az_iot_hub_client_commands_parse_received_topic_EMPTY_received_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_FROM_STR("");

  az_iot_hub_client_command_request out_request;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request));
}

static void test_az_iot_hub_client_commands_parse_received_topic_AZ_SPAN_NULL_received_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_EMPTY;

  az_iot_hub_client_command_request out_request;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request));
}

static void test_az_iot_hub_client_commands_parse_received_topic_NULL_out_request_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/methods/POST/TestMethod/?$rid=1");

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, NULL));
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void test_az_iot_hub_client_commands_response_get_publish_topic_succeed()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;
  const char expected_topic[] = "$iothub/methods/res/200/?$rid=2";

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(expected_topic, test_buf);
  assert_int_equal(sizeof(expected_topic) - 1, test_length);
}

static void test_az_iot_hub_client_commands_response_get_publish_topic_user_status_succeed()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = UINT16_MAX;
  const char expected_topic[] = "$iothub/methods/res/65535/?$rid=2";

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(expected_topic, test_buf);
  assert_int_equal(sizeof(expected_topic) - 1, test_length);
}

static void test_az_iot_hub_client_commands_response_get_publish_topic_user_status_small_buf_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = UINT16_MAX;
  const char expected_topic[] = "$iothub/methods/res/65535/?$rid=2";

  char test_buf[sizeof(expected_topic) - 2];
  size_t test_length;

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void
test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_prefix_fail()
{
  char test_buf[10];
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void
test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_status_fail()
{
  char test_buf[21]; // Enough for "$iothub/methods/res/2"
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void
test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_reqid_fail()
{
  char test_buf[24]; // Enough for "$iothub/methods/res/200/"
  size_t test_length;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span request_id = AZ_SPAN_LITERAL_FROM_STR("2");
  uint16_t status = 200;

  assert_true(
      az_iot_hub_client_commands_response_get_publish_topic(
          &client, request_id, status, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_hub_client_commands_parse_received_topic_succeed()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  const char expected_name[] = "TestMethod";
  const char expected_request_id[] = "1";
  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/methods/POST/TestMethod/?$rid=1");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_OK);
  assert_int_equal(az_span_size(out_request.component_name), 0);
  assert_ptr_equal(az_span_ptr(out_request.component_name), NULL);
  assert_int_equal(az_span_size(out_request.command_name), _az_COUNTOF(expected_name) - 1);
  assert_memory_equal(
      az_span_ptr(out_request.command_name), expected_name, _az_COUNTOF(expected_name) - 1);
  assert_int_equal(az_span_size(out_request.request_id), _az_COUNTOF(expected_request_id) - 1);
  assert_memory_equal(
      az_span_ptr(out_request.request_id),
      expected_request_id,
      _az_COUNTOF(expected_request_id) - 1);
}

static void test_az_iot_hub_client_commands_parse_received_topic_with_component_succeed()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  const char expected_component[] = "component_one";
  const char expected_name[] = "TestMethod";
  const char expected_request_id[] = "1";
  az_span received_topic
      = AZ_SPAN_FROM_STR("$iothub/methods/POST/component_one*TestMethod/?$rid=1");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_OK);
  assert_int_equal(az_span_size(out_request.component_name), _az_COUNTOF(expected_component) - 1);
  assert_memory_equal(
      az_span_ptr(out_request.component_name),
      expected_component,
      _az_COUNTOF(expected_component) - 1);
  assert_int_equal(az_span_size(out_request.command_name), _az_COUNTOF(expected_name) - 1);
  assert_memory_equal(
      az_span_ptr(out_request.command_name), expected_name, _az_COUNTOF(expected_name) - 1);
  assert_int_equal(az_span_size(out_request.request_id), _az_COUNTOF(expected_request_id) - 1);
  assert_memory_equal(
      az_span_ptr(out_request.request_id),
      expected_request_id,
      _az_COUNTOF(expected_request_id) - 1);
}

static void test_az_iot_hub_client_commands_parse_received_topic_c2d_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic
      = AZ_SPAN_FROM_STR("$iothub/devices/useragent_c/messages/devicebound/"
                         "%24.to=%2Fdevices%2Fuseragent_c%2Fmessages%2FdeviceBound&abc=123&ghi=%"
                         "2Fsome%2Fthing&jkl=%2Fsome%2Fthing%2F%3Fbla%3Dbla");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static void test_az_iot_hub_client_commands_parse_received_topic_get_property_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/twin/res/200/?$rid=2");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static void test_az_iot_hub_client_commands_parse_received_topic_property_patch_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/twin/res/204/?$rid=4&$version=3");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static void test_az_iot_hub_client_commands_parse_received_topic_topic_filter_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = az_span_create(
      g_expected_methods_subscribe_topic, _az_COUNTOF(g_expected_methods_subscribe_topic));

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static void test_az_iot_hub_client_commands_parse_received_topic_response_topic_fail()
{
  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_span received_topic = AZ_SPAN_FROM_STR("$iothub/methods/res/200/?$rid=2");

  az_iot_hub_client_command_request out_request;

  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, received_topic, &out_request)
      == AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static const az_span _log_expected_topic
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/methods/POST/TestMethod/?$rid=1");
static int _log_invoked_topic = 0;
static void _log_listener(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_MQTT_RECEIVED_TOPIC:
      assert_memory_equal(
          az_span_ptr(_log_expected_topic), az_span_ptr(message), (size_t)az_span_size(message));
      _log_invoked_topic++;
      break;
    default:
      assert_true(false);
  }
}

static void test_az_iot_hub_client_commands_logging_succeed()
{
  az_log_set_message_callback(_log_listener);

  assert_int_equal(0, _log_invoked_topic);

  _log_invoked_topic = 0;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.model_id = test_model_id;

  az_iot_hub_client client;
  assert_true(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options) == AZ_OK);

  az_iot_hub_client_command_request out_request;
  assert_true(
      az_iot_hub_client_commands_parse_received_topic(&client, _log_expected_topic, &out_request)
      == AZ_OK);

  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_invoked_topic);

  az_log_set_message_callback(NULL);
}

#ifdef _MSC_VER
// warning C4113: 'void (__cdecl *)()' differs in parameter lists from 'CMUnitTestFunction'
#pragma warning(disable : 4113)
#endif

int test_az_iot_hub_client_commands()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_NULL_out_topic_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_response_get_publish_topic_NULL_client_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_zero_size_buffer_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_EMPTY_request_id_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_AZ_SPAN_NULL_request_id_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_NULL_client_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_parse_received_topic_EMPTY_received_topic_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_parse_received_topic_AZ_SPAN_NULL_received_topic_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_NULL_out_request_fail),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_hub_client_commands_response_get_publish_topic_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_user_status_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_user_status_small_buf_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_prefix_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_status_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_response_get_publish_topic_INSUFFICIENT_BUFFER_for_reqid_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_succeed),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_with_component_succeed),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_c2d_topic_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_get_property_topic_fail),
    cmocka_unit_test(
        test_az_iot_hub_client_commands_parse_received_topic_property_patch_topic_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_topic_filter_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_parse_received_topic_response_topic_fail),
    cmocka_unit_test(test_az_iot_hub_client_commands_logging_succeed)
  };

  return cmocka_run_group_tests_name("az_iot_hub_commands", tests, NULL, NULL);
}
