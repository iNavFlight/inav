// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_provisioning_client.h"
#include <az_test_log.h>
#include <az_test_span.h>
#include <azure/core/az_log.h>
#include <azure/core/az_span.h>
#include <azure/iot/az_iot_provisioning_client.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

#define TEST_REGISTRATION_ID "myRegistrationId"
#define TEST_ID_SCOPE "0neFEEDC0DE"
#define TEST_HUB_HOSTNAME "contoso.azure-devices.net"
#define TEST_DEVICE_ID "my-device-id1"
#define TEST_OPERATION_ID "4.d0a671905ea5b2c8.42d78160-4c78-479e-8be7-61d5e55dac0d"

static const az_span test_global_device_hostname
    = AZ_SPAN_LITERAL_FROM_STR("global.azure-devices-provisioning.net");
static const az_span test_id_scope = AZ_SPAN_LITERAL_FROM_STR(TEST_ID_SCOPE);
static const az_span test_registration_id = AZ_SPAN_LITERAL_FROM_STR(TEST_REGISTRATION_ID);

#define TEST_STATUS_ASSIGNING "assigning"
#define TEST_STATUS_ASSIGNED "assigned"
#define TEST_STATUS_FAILED "failed"
#define TEST_STATUS_DISABLED "disabled"
#define TEST_STATUS_UNASSIGNED "unassigned"
#define TEST_STATUS_UNKNOWN "unknown status123!@#"

#define TEST_ERROR_MESSAGE_INVALID_CERT "Invalid certificate."
#define TEST_ERROR_MESSAGE_ALLOCATION "Custom allocation failed with status code: 400"
#define TEST_ERROR_TRACKING_ID "8ad0463c-6427-4479-9dfa-3e8bb7003e9b"
#define TEST_ERROR_TIMESTAMP "2020-04-10T05:24:22.4718526Z"

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_assigning_state_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/202/?$rid=1&retry-after=3");
  az_span received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                              "\",\"status\":\"" TEST_STATUS_ASSIGNING "\"}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_ACCEPTED, response.status); // 202
  assert_int_equal(3, response.retry_after_seconds);

  // From payload
  assert_memory_equal(
      az_span_ptr(response.operation_id), TEST_OPERATION_ID, strlen(TEST_OPERATION_ID));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_ASSIGNING, response.operation_status);
}

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_topic_not_matched_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/unknown");
  az_span received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                              "\",\"status\":\"" TEST_STATUS_ASSIGNING "\"}");

  az_iot_provisioning_client_register_response response
      = { .status = AZ_IOT_STATUS_FORBIDDEN,
          .retry_after_seconds = 0xBAADC0DE,
          .operation_id = AZ_SPAN_EMPTY,
          .operation_status = AZ_IOT_PROVISIONING_STATUS_FAILED,
          .registration_state = { 0 } };

  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_IOT_TOPIC_NO_MATCH, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_FORBIDDEN, response.status); // 202
  assert_int_equal(0xBAADC0DE, response.retry_after_seconds);
}

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_parse_assigning2_state_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/202/?retry-after=120&$rid=1");
  az_span received_payload = AZ_SPAN_FROM_STR(
      "{\"operationId\":\"" TEST_OPERATION_ID "\",\"status\":\"" TEST_STATUS_ASSIGNING
      "\",\"registrationState\":{\"registrationId\":\"" TEST_REGISTRATION_ID
      "\",\"status\":\"" TEST_STATUS_ASSIGNING "\"}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_ACCEPTED, response.status); // 202
  assert_int_equal(120, response.retry_after_seconds);

  // From payload
  assert_memory_equal(
      az_span_ptr(response.operation_id), TEST_OPERATION_ID, strlen(TEST_OPERATION_ID));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_ASSIGNING, response.operation_status);
}

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_assigned_state_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         "\",\"status\":\"" TEST_STATUS_ASSIGNED "\",\"registrationState\":{"
                         "\"x509\":{},"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"assignedHub\":\"" TEST_HUB_HOSTNAME "\","
                         "\"deviceId\":\"" TEST_DEVICE_ID "\","
                         "\"status\":\"" TEST_STATUS_ASSIGNED "\","
                         "\"substatus\":\"initialAssignment\","
                         "\"lastUpdatedDateTimeUtc\":\"2020-04-10T03:11:13.2096201Z\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\","
                         "\"payload\":{\"hello\":\"world\",\"arr\":[1,2,3,4,5,6],\"num\":123}}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_OK, response.status); // 200
  assert_int_equal(0, response.retry_after_seconds);

  // From payload
  assert_memory_equal(
      az_span_ptr(response.operation_id), TEST_OPERATION_ID, strlen(TEST_OPERATION_ID));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_ASSIGNED, response.operation_status);
  assert_memory_equal(
      az_span_ptr(response.registration_state.assigned_hub_hostname),
      TEST_HUB_HOSTNAME,
      strlen(TEST_HUB_HOSTNAME));
  assert_memory_equal(
      az_span_ptr(response.registration_state.device_id), TEST_DEVICE_ID, strlen(TEST_DEVICE_ID));

  assert_int_equal(0, response.registration_state.error_code);
  assert_int_equal(0, response.registration_state.extended_error_code);
  assert_int_equal(0, az_span_size(response.registration_state.error_message));
}

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_invalid_certificate_error_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/401/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"errorCode\":401002,\"trackingId\":\"" TEST_ERROR_TRACKING_ID "\","
                         "\"message\":\"" TEST_ERROR_MESSAGE_INVALID_CERT
                         "\",\"timestampUtc\":\"" TEST_ERROR_TIMESTAMP "\"}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_UNAUTHORIZED, response.status); // 401
  assert_int_equal(0, response.retry_after_seconds);

  // From payload
  assert_int_equal(0, az_span_size(response.operation_id));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_FAILED, response.operation_status);

  assert_int_equal(0, az_span_size(response.registration_state.assigned_hub_hostname));
  assert_int_equal(0, az_span_size(response.registration_state.device_id));

  assert_int_equal(AZ_IOT_STATUS_UNAUTHORIZED, response.registration_state.error_code);
  assert_int_equal(401002, response.registration_state.extended_error_code);

  assert_memory_equal(
      az_span_ptr(response.registration_state.error_message),
      TEST_ERROR_MESSAGE_INVALID_CERT,
      strlen(TEST_ERROR_MESSAGE_INVALID_CERT));
  assert_memory_equal(
      az_span_ptr(response.registration_state.error_timestamp),
      TEST_ERROR_TIMESTAMP,
      strlen(TEST_ERROR_TIMESTAMP));
  assert_memory_equal(
      az_span_ptr(response.registration_state.error_tracking_id),
      TEST_ERROR_TRACKING_ID,
      strlen(TEST_ERROR_TRACKING_ID));
}

static void test_az_iot_provisioning_client_parse_received_topic_payload_disabled_state_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"\",\"status\":\"" TEST_STATUS_DISABLED
                         "\",\"registrationState\":{\"registrationId\":\"" TEST_REGISTRATION_ID
                         "\",\"status\":\"" TEST_STATUS_DISABLED "\"}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic
  assert_int_equal(AZ_IOT_STATUS_OK, response.status); // 200
  assert_int_equal(0, response.retry_after_seconds);

  // From payload
  assert_int_equal(0, az_span_size(response.operation_id));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_DISABLED, response.operation_status);
}

static void
test_az_iot_provisioning_client_parse_received_topic_and_payload_allocation_error_state_succeed()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         "\",\"status\":\"" TEST_STATUS_FAILED "\",\"registrationState\":{"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"status\":\"" TEST_STATUS_FAILED "\","
                         "\"errorCode\":400207,"
                         "\"errorMessage\":\"" TEST_ERROR_MESSAGE_ALLOCATION "\","
                         "\"lastUpdatedDateTimeUtc\":\"" TEST_ERROR_TIMESTAMP "\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\"}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);

  // From topic (the last request succeeded, entire operation failed)
  assert_int_equal(AZ_IOT_STATUS_OK, response.status); // 200
  assert_int_equal(0, response.retry_after_seconds);

  // From payload
  assert_memory_equal(
      az_span_ptr(response.operation_id), TEST_OPERATION_ID, strlen(TEST_OPERATION_ID));
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_FAILED, response.operation_status);

  assert_int_equal(0, az_span_size(response.registration_state.assigned_hub_hostname));
  assert_int_equal(0, az_span_size(response.registration_state.device_id));

  assert_int_equal(AZ_IOT_STATUS_BAD_REQUEST, response.registration_state.error_code);
  assert_int_equal(400207, response.registration_state.extended_error_code);

  assert_memory_equal(
      az_span_ptr(response.registration_state.error_message),
      TEST_ERROR_MESSAGE_ALLOCATION,
      strlen(TEST_ERROR_MESSAGE_ALLOCATION));
  assert_memory_equal(
      az_span_ptr(response.registration_state.error_timestamp),
      TEST_ERROR_TIMESTAMP,
      strlen(TEST_ERROR_TIMESTAMP));

  assert_int_equal(0, az_span_size(response.registration_state.error_tracking_id));
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_json_payload_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload = AZ_SPAN_FROM_STR("123");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_UNEXPECTED_CHAR, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_operationid_not_found_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR(/*\"operationId\":\"" TEST_OPERATION_ID*/
                         "{\"status\":\"" TEST_STATUS_FAILED "\",\"registrationState\":{"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"status\":\"" TEST_STATUS_FAILED "\","
                         "\"errorCode\":400207,"
                         "\"errorMessage\":\"" TEST_ERROR_MESSAGE_ALLOCATION "\","
                         "\"lastUpdatedDateTimeUtc\":\"" TEST_ERROR_TIMESTAMP "\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\"}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_operation_status_not_found_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         /*"\",\"status\":\"" TEST_STATUS_FAILED */ "\",\"registrationState\":{"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"status\":\"" TEST_STATUS_FAILED "\","
                         "\"errorCode\":400207,"
                         "\"errorMessage\":\"" TEST_ERROR_MESSAGE_ALLOCATION "\","
                         "\"lastUpdatedDateTimeUtc\":\"" TEST_ERROR_TIMESTAMP "\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\"}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_error_code_not_found_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload = AZ_SPAN_FROM_STR(
      "{" /*\"errorCode\":401002,*/ "\"trackingId\":\"" TEST_ERROR_TRACKING_ID "\","
      "\"message\":\"" TEST_ERROR_MESSAGE_INVALID_CERT "\",\"timestampUtc\":\"" TEST_ERROR_TIMESTAMP
      "\"}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_operation_status_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         "\",\"status\":\"" TEST_STATUS_FAILED "\",\"registrationState\":123}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_UNEXPECTED_CHAR, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_result_json_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         /*"\",\"status\":\"" TEST_STATUS_FAILED */ "\",\"registrationState\":{"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\",");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void test_az_iot_provisioning_client_received_topic_and_payload_parse_hub_not_found_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         "\",\"status\":\"" TEST_STATUS_ASSIGNED "\",\"registrationState\":{"
                         "\"x509\":{},"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"deviceId\":\"" TEST_DEVICE_ID "\","
                         "\"status\":\"" TEST_STATUS_ASSIGNED "\","
                         "\"substatus\":\"initialAssignment\","
                         "\"lastUpdatedDateTimeUtc\":\"2020-04-10T03:11:13.2096201Z\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\","
                         "\"payload\":{\"hello\":\"world\",\"arr\":[1,2,3,4,5,6],\"num\":123}}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void
test_az_iot_provisioning_client_received_topic_and_payload_parse_device_not_found_fails()
{
  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/200/?$rid=1");
  az_span received_payload
      = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                         "\",\"status\":\"" TEST_STATUS_ASSIGNED "\",\"registrationState\":{"
                         "\"x509\":{},"
                         "\"registrationId\":\"" TEST_REGISTRATION_ID "\","
                         "\"createdDateTimeUtc\":\"2020-04-10T03:11:13.0276997Z\","
                         "\"assignedHub\":\"" TEST_HUB_HOSTNAME "\","
                         "\"status\":\"" TEST_STATUS_ASSIGNED "\","
                         "\"substatus\":\"initialAssignment\","
                         "\"lastUpdatedDateTimeUtc\":\"2020-04-10T03:11:13.2096201Z\","
                         "\"etag\":\"IjYxMDA4ZDQ2LTAwMDAtMDEwMC0wMDAwLTVlOGZlM2QxMDAwMCI=\","
                         "\"payload\":{\"hello\":\"world\",\"arr\":[1,2,3,4,5,6],\"num\":123}}}");

  az_iot_provisioning_client_register_response response;
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_ITEM_NOT_FOUND, ret);
}

static void test_az_iot_provisioning_client_parse_operation_status_translate_succeed()
{
  az_iot_provisioning_client_register_response response;

  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_span received_topic = AZ_SPAN_FROM_STR("$dps/registrations/res/202/?$rid=1&retry-after=3");

  // TEST_STATUS_UNKNOWN
  az_span received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                              "\",\"status\":\"" TEST_STATUS_UNKNOWN "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_ERROR_UNEXPECTED_CHAR, ret);

  // AZ_IOT_PROVISIONING_STATUS_UNASSIGNED
  received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                      "\",\"status\":\"" TEST_STATUS_UNASSIGNED "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_UNASSIGNED, response.operation_status);

  // AZ_IOT_PROVISIONING_STATUS_ASSIGNING
  received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                      "\",\"status\":\"" TEST_STATUS_ASSIGNING "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_ASSIGNING, response.operation_status);

  // AZ_IOT_PROVISIONING_STATUS_ASSIGNED
  received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                      "\",\"status\":\"" TEST_STATUS_ASSIGNED "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_ASSIGNED, response.operation_status);

  // AZ_IOT_PROVISIONING_STATUS_FAILED
  received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                      "\",\"status\":\"" TEST_STATUS_FAILED "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_FAILED, response.operation_status);

  // AZ_IOT_PROVISIONING_STATUS_DISABLED
  received_payload = AZ_SPAN_FROM_STR("{\"operationId\":\"" TEST_OPERATION_ID
                                      "\",\"status\":\"" TEST_STATUS_DISABLED "\"}");
  ret = az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, received_topic, received_payload, &response);
  assert_int_equal(AZ_OK, ret);
  assert_int_equal(AZ_IOT_PROVISIONING_STATUS_DISABLED, response.operation_status);
}

static void test_az_iot_provisioning_client_operation_complete_translate_succeed()
{
  assert_false(
      az_iot_provisioning_client_operation_complete(AZ_IOT_PROVISIONING_STATUS_UNASSIGNED));
  assert_false(az_iot_provisioning_client_operation_complete(AZ_IOT_PROVISIONING_STATUS_ASSIGNING));
  assert_true(az_iot_provisioning_client_operation_complete(AZ_IOT_PROVISIONING_STATUS_ASSIGNED));
  assert_true(az_iot_provisioning_client_operation_complete(AZ_IOT_PROVISIONING_STATUS_FAILED));
  assert_true(az_iot_provisioning_client_operation_complete(AZ_IOT_PROVISIONING_STATUS_DISABLED));
}

static const az_span _log_received_topic = AZ_SPAN_LITERAL_FROM_STR("$dps/registrations/res/202");
static const az_span _log_received_payload = AZ_SPAN_LITERAL_FROM_STR("LOG_PAYLOAD");

static int _log_invoked_topic = 0;
static int _log_invoked_payload = 0;
static void _log_listener(az_log_classification classification, az_span message)
{
  switch (classification)
  {
    case AZ_LOG_MQTT_RECEIVED_TOPIC:
      assert_memory_equal(
          az_span_ptr(_log_received_topic), az_span_ptr(message), (size_t)az_span_size(message));
      _log_invoked_topic++;
      break;
    case AZ_LOG_MQTT_RECEIVED_PAYLOAD:
      assert_memory_equal(
          az_span_ptr(_log_received_payload), az_span_ptr(message), (size_t)az_span_size(message));
      _log_invoked_payload++;
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

static bool _should_write_nothing(az_log_classification classification)
{
  (void)classification;
  return false;
}

static void test_az_iot_provisioning_client_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_any_mqtt);

  _log_invoked_topic = 0;
  _log_invoked_payload = 0;

  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_iot_provisioning_client_register_response response;
  assert_true(az_result_failed(az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, _log_received_topic, _log_received_payload, &response)));

  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_invoked_topic);
  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_invoked_payload);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void test_az_iot_provisioning_client_no_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_nothing);

  _log_invoked_topic = 0;
  _log_invoked_payload = 0;

  az_iot_provisioning_client client = { 0 };
  az_result ret = az_iot_provisioning_client_init(
      &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL);
  assert_int_equal(AZ_OK, ret);

  az_iot_provisioning_client_register_response response;
  assert_true(az_result_failed(az_iot_provisioning_client_parse_received_topic_and_payload(
      &client, _log_received_topic, _log_received_payload, &response)));

  assert_int_equal(_az_BUILT_WITH_LOGGING(0, 0), _log_invoked_topic);
  assert_int_equal(_az_BUILT_WITH_LOGGING(0, 0), _log_invoked_payload);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

#ifdef _MSC_VER
// warning C4113: 'void (__cdecl *)()' differs in parameter lists from 'CMUnitTestFunction'
#pragma warning(disable : 4113)
#endif

int test_az_iot_provisioning_client_parser()
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_assigning_state_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_topic_not_matched_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_parse_assigning2_state_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_assigned_state_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_invalid_certificate_error_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_payload_disabled_state_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_parse_received_topic_and_payload_allocation_error_state_succeed),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_json_payload_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_operationid_not_found_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_operation_status_not_found_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_error_code_not_found_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_operation_status_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_invalid_result_json_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_hub_not_found_fails),
    cmocka_unit_test(
        test_az_iot_provisioning_client_received_topic_and_payload_parse_device_not_found_fails),
    cmocka_unit_test(test_az_iot_provisioning_client_parse_operation_status_translate_succeed),
    cmocka_unit_test(test_az_iot_provisioning_client_operation_complete_translate_succeed),
    cmocka_unit_test(test_az_iot_provisioning_client_logging_succeed),
    cmocka_unit_test(test_az_iot_provisioning_client_no_logging_succeed),
  };

  return cmocka_run_group_tests_name("az_iot_provisioning_client_parser", tests, NULL, NULL);
}
