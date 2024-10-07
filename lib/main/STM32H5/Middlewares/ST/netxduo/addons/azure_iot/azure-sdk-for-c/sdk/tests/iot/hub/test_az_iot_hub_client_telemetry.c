// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_hub_client.h"
#include <az_test_span.h>
#include <azure/iot/az_iot_hub_client.h>

#include <azure/core/az_precondition.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <az_test_precondition.h>
#include <cmocka.h>

#define TEST_SPAN_BUFFER_SIZE 128

static const az_span test_device_id = AZ_SPAN_LITERAL_FROM_STR("my_device");
static const az_span test_device_hostname = AZ_SPAN_LITERAL_FROM_STR("myiothub.azure-devices.net");
static const az_span test_module_id = AZ_SPAN_LITERAL_FROM_STR("my_module_id");
static const az_span test_props = AZ_SPAN_LITERAL_FROM_STR("key=value&key_two=value2");

static const char g_test_correct_topic_no_options_no_props[] = "devices/my_device/messages/events/";
static const char g_test_correct_topic_with_options_no_props[]
    = "devices/my_device/modules/my_module_id/messages/events/";
static const char g_test_correct_topic_with_options_with_props[]
    = "devices/my_device/modules/my_module_id/messages/events/"
      "key=value&key_two=value2";
static const char g_test_correct_topic_no_options_with_props[]
    = "devices/my_device/messages/events/"
      "key=value&key_two=value2";
static const char g_test_correct_topic_with_options_module_id_with_props[]
    = "devices/my_device/modules/my_module_id/messages/events/key=value&key_two=value2";

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()

static void test_az_iot_hub_client_telemetry_get_publish_topic_NULL_client_fails(void** state)
{
  (void)state;

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_telemetry_get_publish_topic(
      NULL, NULL, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_NULL_mqtt_topic_fails(void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_telemetry_get_publish_topic(
      &client, NULL, NULL, TEST_SPAN_BUFFER_SIZE, &test_length));
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_NULL_out_mqtt_topic_fails(
    void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_telemetry_get_publish_topic(&client, NULL, test_buf, 0, &test_length));
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void test_az_iot_hub_client_telemetry_get_publish_topic_no_options_no_props_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);
  assert_string_equal(g_test_correct_topic_no_options_no_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_no_options_no_props) - 1, test_length);
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_with_options_no_props_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.module_id = test_module_id;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, NULL, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(g_test_correct_topic_with_options_no_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_with_options_no_props) - 1, test_length);
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_with_options_with_props_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.module_id = test_module_id;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(g_test_correct_topic_with_options_with_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_with_options_with_props) - 1, test_length);
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_with_props_unfilled_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  // Create unfilled property span
  az_iot_message_properties props;
  uint8_t test_prop_unfilled_buf[TEST_SPAN_BUFFER_SIZE];
  memset(test_prop_unfilled_buf, 0xFF, sizeof(test_prop_unfilled_buf));
  az_span test_prop_unfilled_span
      = az_span_create(test_prop_unfilled_buf, sizeof(test_prop_unfilled_buf));
  assert_int_equal(az_iot_message_properties_init(&props, test_prop_unfilled_span, 0), AZ_OK);
  assert_int_equal(
      az_iot_message_properties_append(&props, AZ_SPAN_FROM_STR("key"), AZ_SPAN_FROM_STR("value")),
      AZ_OK);
  assert_int_equal(
      az_iot_message_properties_append(
          &props, AZ_SPAN_FROM_STR("key_two"), AZ_SPAN_FROM_STR("value2")),
      AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(g_test_correct_topic_no_options_with_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_no_options_with_props) - 1, test_length);
}

static void
test_az_iot_hub_client_telemetry_get_publish_topic_with_options_with_props_small_buffer_fails(
    void** state)
{
  (void)state;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.module_id = test_module_id;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[sizeof(g_test_correct_topic_with_options_with_props) - 2];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_hub_client_telemetry_get_publish_topic_no_options_with_props_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(g_test_correct_topic_no_options_with_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_no_options_with_props) - 1, test_length);
}

static void
test_az_iot_hub_client_telemetry_get_publish_topic_no_options_with_props_small_buffer_fails(
    void** state)
{
  (void)state;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[sizeof(g_test_correct_topic_no_options_with_props) - 2];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void
test_az_iot_hub_client_telemetry_get_publish_topic_with_options_module_id_with_props_succeed(
    void** state)
{
  (void)state;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.module_id = test_module_id;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_OK);

  assert_string_equal(g_test_correct_topic_with_options_module_id_with_props, test_buf);
  assert_int_equal(sizeof(g_test_correct_topic_with_options_module_id_with_props) - 1, test_length);
}

static void
test_az_iot_hub_client_telemetry_get_publish_topic_with_options_module_id_with_props_small_buffer_fails(
    void** state)
{
  (void)state;

  az_iot_hub_client_options options = az_iot_hub_client_options_default();
  options.module_id = test_module_id;

  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, &options), AZ_OK);

  az_iot_message_properties props;
  assert_int_equal(
      az_iot_message_properties_init(&props, test_props, az_span_size(test_props)), AZ_OK);

  char test_buf[sizeof(g_test_correct_topic_with_options_module_id_with_props) - 2];
  size_t test_length;

  assert_true(
      az_iot_hub_client_telemetry_get_publish_topic(
          &client, &props, test_buf, sizeof(test_buf), &test_length)
      == AZ_ERROR_NOT_ENOUGH_SPACE);
}

int test_az_iot_hub_client_telemetry()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_hub_client_telemetry_get_publish_topic_NULL_client_fails),
    cmocka_unit_test(test_az_iot_hub_client_telemetry_get_publish_topic_NULL_mqtt_topic_fails),
    cmocka_unit_test(test_az_iot_hub_client_telemetry_get_publish_topic_NULL_out_mqtt_topic_fails),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_no_options_no_props_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_options_no_props_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_options_with_props_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_props_unfilled_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_options_with_props_small_buffer_fails),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_no_options_with_props_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_no_options_with_props_small_buffer_fails),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_options_module_id_with_props_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_telemetry_get_publish_topic_with_options_module_id_with_props_small_buffer_fails),
  };

  return cmocka_run_group_tests_name("az_iot_hub_client_telemetry", tests, NULL, NULL);
}
