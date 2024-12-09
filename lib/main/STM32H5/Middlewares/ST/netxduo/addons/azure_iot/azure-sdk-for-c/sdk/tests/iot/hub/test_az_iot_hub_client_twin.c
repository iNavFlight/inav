// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_hub_client.h"
#include <az_test_log.h>
#include <az_test_precondition.h>
#include <az_test_span.h>
#include <azure/core/az_log.h>
#include <azure/core/az_precondition.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/iot/az_iot_hub_client.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#define TEST_SPAN_BUFFER_SIZE 128

static const az_span test_device_id = AZ_SPAN_LITERAL_FROM_STR("my_device");
static const az_span test_device_hostname = AZ_SPAN_LITERAL_FROM_STR("myiothub.azure-devices.net");
static const az_span test_device_request_id = AZ_SPAN_LITERAL_FROM_STR("id_one");
static const az_span test_twin_received_topic_desired_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/PATCH/properties/desired/?$version=id_one");
static const az_span test_twin_received_topic_fail
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/rez/200"); // rez does not exist.
static const az_span test_twin_received_topic_incomplete_fail
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/200"); // Missing rid.
static const az_span test_twin_received_topic_prefix_fail
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/contoso/res/200"); // contoso dones not exist.

static const az_span test_twin_received_topic_get_response_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/200/?$rid=id_one");
static const az_span test_twin_received_topic_reported_response_version_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/204/?$rid=id_one&$version=16");
static const az_span test_twin_received_topic_reported_response_no_version_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/204/?$rid=id_one");
static const az_span test_twin_received_topic_400_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/400/?$rid=id_one");
static const az_span test_twin_received_topic_504_success
    = AZ_SPAN_LITERAL_FROM_STR("$iothub/twin/res/504/?$rid=id_one");

static const char test_correct_twin_get_request_topic[] = "$iothub/twin/GET/?$rid=id_one";
static const char test_correct_twin_patch_pub_topic[]
    = "$iothub/twin/PATCH/properties/reported/?$rid=id_one";

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()

static void test_az_iot_hub_client_twin_document_get_publish_topic_NULL_client_fails()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_document_get_publish_topic(
      NULL, test_device_request_id, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_document_get_publish_topic_NULL_request_id_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  uint8_t test_bad_request_id_buf[1];
  az_span test_bad_request_id
      = az_span_create(test_bad_request_id_buf, _az_COUNTOF(test_bad_request_id_buf));
  test_bad_request_id._internal.ptr = NULL;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_document_get_publish_topic(
      &client, test_bad_request_id, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_document_get_publish_topic_NULL_char_buffer_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_document_get_publish_topic(
      &client, test_device_request_id, NULL, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_document_get_publish_topic_NULL_out_span_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_document_get_publish_topic(
      &client, test_device_request_id, test_buf, 0, &test_length));
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_client_fails()
{
  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_patch_get_publish_topic(
      NULL, test_device_request_id, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_invalid_request_id_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  uint8_t test_bad_request_id_buf[1];
  az_span test_bad_request_id
      = az_span_create(test_bad_request_id_buf, _az_COUNTOF(test_bad_request_id_buf));
  test_bad_request_id._internal.ptr = NULL;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_patch_get_publish_topic(
      &client, test_bad_request_id, test_buf, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_char_buffer_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_patch_get_publish_topic(
      &client, test_device_request_id, NULL, sizeof(test_buf), &test_length));
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_out_span_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_patch_get_publish_topic(
      &client, test_device_request_id, test_buf, 0, &test_length));
}

static void test_az_iot_hub_client_twin_parse_received_topic_NULL_client_fails()
{
  az_iot_hub_client_twin_response response;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_parse_received_topic(
      NULL, test_twin_received_topic_desired_success, &response));
}

static void test_az_iot_hub_client_twin_parse_received_topic_NULL_rec_topic_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  az_iot_hub_client_twin_response response;

  ASSERT_PRECONDITION_CHECKED(
      az_iot_hub_client_twin_parse_received_topic(&client, AZ_SPAN_EMPTY, &response));
}

static void test_az_iot_hub_client_twin_parse_received_topic_NULL_response_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  ;

  ASSERT_PRECONDITION_CHECKED(az_iot_hub_client_twin_parse_received_topic(
      &client, test_twin_received_topic_desired_success, NULL));
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void test_az_iot_hub_client_twin_document_get_publish_topic_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_int_equal(
      az_iot_hub_client_twin_document_get_publish_topic(
          &client, test_device_request_id, test_buf, sizeof(test_buf), &test_length),
      AZ_OK);
  assert_string_equal(test_correct_twin_get_request_topic, test_buf);
  assert_int_equal(sizeof(test_correct_twin_get_request_topic) - 1, test_length);
}

static void test_az_iot_hub_client_twin_document_get_publish_topic_small_buffer_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[_az_COUNTOF(test_correct_twin_get_request_topic) - 2];
  size_t test_length;

  assert_int_equal(
      az_iot_hub_client_twin_document_get_publish_topic(
          &client, test_device_request_id, test_buf, sizeof(test_buf), &test_length),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[TEST_SPAN_BUFFER_SIZE];
  size_t test_length;

  assert_int_equal(
      az_iot_hub_client_twin_patch_get_publish_topic(
          &client, test_device_request_id, test_buf, sizeof(test_buf), &test_length),
      AZ_OK);
  assert_string_equal(test_correct_twin_patch_pub_topic, test_buf);
  assert_int_equal(sizeof(test_correct_twin_patch_pub_topic) - 1, test_length);
}

static void test_az_iot_hub_client_twin_patch_get_publish_topic_small_buffer_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);

  char test_buf[_az_COUNTOF(test_correct_twin_patch_pub_topic) - 2];
  size_t test_length;

  assert_int_equal(
      az_iot_hub_client_twin_patch_get_publish_topic(
          &client, test_device_request_id, test_buf, sizeof(test_buf), &test_length),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void test_az_iot_hub_client_twin_parse_received_topic_desired_found_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_desired_success, &response),
      AZ_OK);
  assert_true(az_span_is_content_equal((response.version), test_device_request_id));
  assert_true(az_span_is_content_equal(response.request_id, AZ_SPAN_EMPTY));
  assert_int_equal(response.status, AZ_IOT_STATUS_OK);
  assert_int_equal(response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES);
}

static void test_az_iot_hub_client_twin_parse_received_topic_get_response_found_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_get_response_success, &response),
      AZ_OK);
  assert_true(az_span_is_content_equal(response.request_id, test_device_request_id));
  assert_true(az_span_is_content_equal(response.version, AZ_SPAN_EMPTY));
  assert_int_equal(response.status, AZ_IOT_STATUS_OK);
  assert_int_equal(response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_GET);
}

static void test_az_iot_hub_client_twin_parse_received_topic_reported_props_version_found_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_reported_response_version_success, &response),
      AZ_OK);
  assert_true(az_span_is_content_equal(response.request_id, test_device_request_id));
  assert_true(az_span_is_content_equal(response.version, AZ_SPAN_FROM_STR("16")));
  assert_int_equal(response.status, AZ_IOT_STATUS_NO_CONTENT);
  assert_int_equal(
      response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES);
}

static void
test_az_iot_hub_client_twin_parse_received_topic_reported_props_no_version_found_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_reported_response_no_version_success, &response),
      AZ_OK);
  assert_true(az_span_is_content_equal(response.request_id, test_device_request_id));
  assert_true(az_span_is_content_equal(response.version, AZ_SPAN_EMPTY));
  assert_int_equal(response.status, AZ_IOT_STATUS_NO_CONTENT);
  assert_int_equal(
      response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES);
}

static void test_az_iot_hub_client_twin_parse_received_topic_400_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_400_success, &response),
      AZ_OK);

  assert_true(az_span_is_content_equal(response.request_id, test_device_request_id));
  assert_true(az_span_is_content_equal(response.version, AZ_SPAN_EMPTY));
  assert_true(response.status = AZ_IOT_STATUS_BAD_REQUEST);
  assert_int_equal(response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REQUEST_ERROR);
}

static void test_az_iot_hub_client_twin_parse_received_topic_504_succeed()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_504_success, &response),
      AZ_OK);

  assert_true(az_span_is_content_equal(response.request_id, test_device_request_id));
  assert_true(az_span_is_content_equal(response.version, AZ_SPAN_EMPTY));
  assert_true(response.status > AZ_IOT_STATUS_BAD_REQUEST);
  assert_int_equal(response.response_type, AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REQUEST_ERROR);
}

static void test_az_iot_hub_client_twin_parse_received_topic_not_found_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_fail, &response),
      AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static void test_az_iot_hub_client_twin_parse_received_topic_not_found_incomplete_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_incomplete_fail, &response),
      AZ_ERROR_UNEXPECTED_END);
}

static void test_az_iot_hub_client_twin_parse_received_topic_not_found_prefix_fails()
{
  az_iot_hub_client client;
  assert_int_equal(
      az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL), AZ_OK);
  az_iot_hub_client_twin_response response;

  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_prefix_fail, &response),
      AZ_ERROR_IOT_TOPIC_NO_MATCH);
}

static int _log_invoked_topic = 0;
static void _log_listener(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_MQTT_RECEIVED_TOPIC:
      assert_memory_equal(
          az_span_ptr(test_twin_received_topic_desired_success),
          az_span_ptr(message),
          (size_t)az_span_size(message));
      _log_invoked_topic++;
      break;
    default:
      assert_true(false);
  }
}

static bool _should_write_any_mqtt(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_MQTT_RECEIVED_TOPIC:
    case AZ_LOG_MQTT_RECEIVED_PAYLOAD:
      return true;
    default:
      return false;
  }
}

static bool _should_write_mqtt_received_payload_only(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_MQTT_RECEIVED_PAYLOAD:
      return true;
    default:
      return false;
  }
}

static void test_az_iot_hub_client_twin_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_any_mqtt);

  _log_invoked_topic = 0;

  az_iot_hub_client client;
  assert_true(az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL) == AZ_OK);

  az_iot_hub_client_twin_response response;
  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_desired_success, &response),
      AZ_OK);

  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_invoked_topic);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void test_az_iot_hub_client_twin_no_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_mqtt_received_payload_only);

  _log_invoked_topic = 0;

  az_iot_hub_client client;
  assert_true(az_iot_hub_client_init(&client, test_device_hostname, test_device_id, NULL) == AZ_OK);

  az_iot_hub_client_twin_response response;
  assert_int_equal(
      az_iot_hub_client_twin_parse_received_topic(
          &client, test_twin_received_topic_desired_success, &response),
      AZ_OK);

  assert_int_equal(_az_BUILT_WITH_LOGGING(0, 0), _log_invoked_topic);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

#ifdef _MSC_VER
// warning C4113: 'void (__cdecl *)()' differs in parameter lists from 'CMUnitTestFunction'
#pragma warning(disable : 4113)
#endif

int test_az_iot_hub_client_twin()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_NULL_client_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_NULL_request_id_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_NULL_char_buffer_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_NULL_out_span_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_client_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_invalid_request_id_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_char_buffer_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_NULL_out_span_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_NULL_client_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_NULL_rec_topic_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_NULL_response_fails),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_document_get_publish_topic_small_buffer_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_patch_get_publish_topic_small_buffer_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_desired_found_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_get_response_found_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_twin_parse_received_topic_reported_props_version_found_succeed),
    cmocka_unit_test(
        test_az_iot_hub_client_twin_parse_received_topic_reported_props_no_version_found_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_400_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_504_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_not_found_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_not_found_incomplete_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_parse_received_topic_not_found_prefix_fails),
    cmocka_unit_test(test_az_iot_hub_client_twin_logging_succeed),
    cmocka_unit_test(test_az_iot_hub_client_twin_no_logging_succeed),
  };

  return cmocka_run_group_tests_name("az_iot_hub_client_twin", tests, NULL, NULL);
}
