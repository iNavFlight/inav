// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_az_iot_provisioning_client.h"
#include <az_test_log.h>
#include <az_test_precondition.h>
#include <az_test_span.h>
#include <azure/core/az_log.h>
#include <azure/core/az_precondition.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/iot/az_iot_provisioning_client.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#define TEST_SPAN_BUFFER_SIZE 256

#define TEST_REGISTRATION_ID_STR "myRegistrationId"
#define TEST_ID_SCOPE "0neFEEDC0DE"
#define TEST_URL_ENCODED_RESOURCE_URI "0neFEEDC0DE%2fregistrations%2fmyRegistrationId"

#define TEST_SIG "cS1eHM/lDjsRsrZV9508wOFrgmZk4g8FNg8NwHVSiSQ"
#define TEST_URL_ENC_SIG "cS1eHM%2FlDjsRsrZV9508wOFrgmZk4g8FNg8NwHVSiSQ"
#define TEST_EXPIRATION_STR "1578941692"
#define TEST_KEY_NAME "iothubowner"

static const az_span test_global_device_hostname
    = AZ_SPAN_LITERAL_FROM_STR("global.azure-devices-provisioning.net");
static const az_span test_id_scope = AZ_SPAN_LITERAL_FROM_STR(TEST_ID_SCOPE);
static const az_span test_registration_id = AZ_SPAN_LITERAL_FROM_STR(TEST_REGISTRATION_ID_STR);
static const uint32_t test_sas_expiry_time_secs = 1578941692;
static const az_span test_signature = AZ_SPAN_LITERAL_FROM_STR(TEST_SIG);

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()

static void az_iot_provisioning_client_sas_get_signature_NULL_signature_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span signature = AZ_SPAN_EMPTY;

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_signature(
      &client, test_sas_expiry_time_secs, signature, NULL));
}

static void az_iot_provisioning_client_sas_get_signature_NULL_signature_span_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span signature = AZ_SPAN_EMPTY;

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_signature(
      &client, test_sas_expiry_time_secs, signature, &signature));
}

static void az_iot_provisioning_client_sas_get_signature_NULL_client_fails()
{
  uint8_t signature_buffer[TEST_SPAN_BUFFER_SIZE];
  az_span signature = az_span_create(signature_buffer, _az_COUNTOF(signature_buffer));

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_signature(
      NULL, test_sas_expiry_time_secs, signature, &signature));
}

static void az_iot_provisioning_client_sas_get_password_EMPTY_signature_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span key_name = AZ_SPAN_EMPTY;
  az_span signature = AZ_SPAN_EMPTY;

  char password[TEST_SPAN_BUFFER_SIZE];
  size_t length = 0;

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_password(
      &client,
      signature,
      test_sas_expiry_time_secs,
      key_name,
      password,
      _az_COUNTOF(password),
      &length));
}

static void az_iot_provisioning_client_sas_get_password_NULL_password_span_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span key_name = AZ_SPAN_EMPTY;
  size_t length = 0;
  char password[TEST_SPAN_BUFFER_SIZE];

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_password(
      &client,
      test_signature,
      test_sas_expiry_time_secs,
      key_name,
      NULL,
      _az_COUNTOF(password),
      &length));
}

static void az_iot_provisioning_client_sas_get_password_empty_password_buffer_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span key_name = AZ_SPAN_EMPTY;

  char password[TEST_SPAN_BUFFER_SIZE];
  size_t length = 0;

  ASSERT_PRECONDITION_CHECKED(az_iot_provisioning_client_sas_get_password(
      &client, test_signature, test_sas_expiry_time_secs, key_name, password, 0, &length));
}

#endif // AZ_NO_PRECONDITION_CHECKING

static void az_iot_provisioning_client_sas_get_signature_device_succeeds()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  const char expected_signature[] = TEST_URL_ENCODED_RESOURCE_URI "\n" TEST_EXPIRATION_STR;

  uint8_t signature_buffer[TEST_SPAN_BUFFER_SIZE];
  az_span signature = az_span_for_test_init(signature_buffer, _az_COUNTOF(signature_buffer));
  az_span out_signature;

  assert_true(az_result_succeeded(az_iot_provisioning_client_sas_get_signature(
      &client, test_sas_expiry_time_secs, signature, &out_signature)));

  az_span_for_test_verify(
      out_signature,
      expected_signature,
      _az_COUNTOF(expected_signature) - 1,
      signature,
      TEST_SPAN_BUFFER_SIZE);
}

static void az_iot_provisioning_client_sas_get_password_device_succeeds()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  const char expected_password[] = "SharedAccessSignature sr=" TEST_URL_ENCODED_RESOURCE_URI
                                   "&sig=" TEST_URL_ENC_SIG "&se=" TEST_EXPIRATION_STR;

  az_span key_name = AZ_SPAN_EMPTY;

  char password[TEST_SPAN_BUFFER_SIZE];
  size_t length = 0;

  assert_true(az_result_succeeded(az_iot_provisioning_client_sas_get_password(
      &client,
      test_signature,
      test_sas_expiry_time_secs,
      key_name,
      password,
      _az_COUNTOF(password),
      &length)));

  assert_int_equal(length, _az_COUNTOF(expected_password) - 1);
  assert_memory_equal(password, expected_password, length + 1); // +1 to account for '\0'.
}

static void az_iot_provisioning_client_sas_get_password_device_with_keyname_succeeds()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  const char expected_password[]
      = "SharedAccessSignature sr=" TEST_URL_ENCODED_RESOURCE_URI "&sig=" TEST_URL_ENC_SIG
        "&se=" TEST_EXPIRATION_STR "&skn=" TEST_KEY_NAME;

  az_span key_name = AZ_SPAN_FROM_STR(TEST_KEY_NAME);

  char password[TEST_SPAN_BUFFER_SIZE];
  size_t length = 0;

  assert_true(az_result_succeeded(az_iot_provisioning_client_sas_get_password(
      &client,
      test_signature,
      test_sas_expiry_time_secs,
      key_name,
      password,
      _az_COUNTOF(password),
      &length)));

  assert_int_equal(length, _az_COUNTOF(expected_password) - 1);
  assert_memory_equal(password, expected_password, length + 1); // +1 to account for '\0'.
}

static void az_iot_provisioning_client_sas_get_password_device_overflow_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  az_span key_name = AZ_SPAN_EMPTY;

  char password[132];
  size_t length = 0;

  assert_int_equal(
      az_iot_provisioning_client_sas_get_password(
          &client,
          test_signature,
          test_sas_expiry_time_secs,
          key_name,
          password,
          _az_COUNTOF(password),
          &length),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static void az_iot_provisioning_client_sas_get_signature_device_signature_overflow_fails()
{
  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  uint8_t signature_buffer[54];
  az_span signature = az_span_create(signature_buffer, _az_COUNTOF(signature_buffer));

  assert_int_equal(
      az_iot_provisioning_client_sas_get_signature(
          &client, test_sas_expiry_time_secs, signature, &signature),
      AZ_ERROR_NOT_ENOUGH_SPACE);
}

static int _log_invoked_sas = 0;
static void _log_listener(az_log_classification classification, az_span message)
{
  const char expected[] = TEST_URL_ENCODED_RESOURCE_URI "\n" TEST_EXPIRATION_STR;
  switch (classification)
  {
    case AZ_LOG_IOT_SAS_TOKEN:
      assert_memory_equal(expected, az_span_ptr(message), (size_t)az_span_size(message));
      _log_invoked_sas++;
      break;
    default:
      assert_true(false);
  }
}

static bool _should_write_iot_sas_token_only(az_log_classification classification)
{
  switch (classification)
  {
    case AZ_LOG_IOT_SAS_TOKEN:
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

static void test_az_iot_provisioning_client_sas_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_iot_sas_token_only);

  _log_invoked_sas = 0;

  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  uint8_t signature_buffer[TEST_SPAN_BUFFER_SIZE];
  az_span signature = az_span_create(signature_buffer, _az_COUNTOF(signature_buffer));
  az_span out_signature;

  assert_true(az_result_succeeded(az_iot_provisioning_client_sas_get_signature(
      &client, test_sas_expiry_time_secs, signature, &out_signature)));

  assert_int_equal(_az_BUILT_WITH_LOGGING(1, 0), _log_invoked_sas);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

static void test_az_iot_provisioning_client_sas_no_logging_succeed()
{
  az_log_set_message_callback(_log_listener);
  az_log_set_classification_filter_callback(_should_write_nothing);

  _log_invoked_sas = 0;

  az_iot_provisioning_client client;
  assert_int_equal(
      az_iot_provisioning_client_init(
          &client, test_global_device_hostname, test_id_scope, test_registration_id, NULL),
      AZ_OK);

  uint8_t signature_buffer[TEST_SPAN_BUFFER_SIZE];
  az_span signature = az_span_create(signature_buffer, _az_COUNTOF(signature_buffer));
  az_span out_signature;

  assert_true(az_result_succeeded(az_iot_provisioning_client_sas_get_signature(
      &client, test_sas_expiry_time_secs, signature, &out_signature)));

  assert_int_equal(_az_BUILT_WITH_LOGGING(0, 0), _log_invoked_sas);

  az_log_set_message_callback(NULL);
  az_log_set_classification_filter_callback(NULL);
}

#ifdef _MSC_VER
// warning C4113: 'void (__cdecl *)()' differs in parameter lists from 'CMUnitTestFunction'
#pragma warning(disable : 4113)
#endif

int test_az_iot_provisioning_client_sas_token()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifndef AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(az_iot_provisioning_client_sas_get_signature_NULL_signature_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_signature_NULL_signature_span_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_signature_NULL_client_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_EMPTY_signature_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_NULL_password_span_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_empty_password_buffer_fails),
#endif // AZ_NO_PRECONDITION_CHECKING
    cmocka_unit_test(az_iot_provisioning_client_sas_get_signature_device_succeeds),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_device_succeeds),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_device_with_keyname_succeeds),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_password_device_overflow_fails),
    cmocka_unit_test(az_iot_provisioning_client_sas_get_signature_device_signature_overflow_fails),
    cmocka_unit_test(test_az_iot_provisioning_client_sas_logging_succeed),
    cmocka_unit_test(test_az_iot_provisioning_client_sas_no_logging_succeed),
  };
  return cmocka_run_group_tests_name("az_iot_provisioning_client_sas", tests, NULL, NULL);
}
