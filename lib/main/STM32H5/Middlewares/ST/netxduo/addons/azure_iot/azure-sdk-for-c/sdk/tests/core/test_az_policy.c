// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <az_test_precondition.h>
#include <azure/core/az_credentials.h>
#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_span.h>
#include <azure/core/az_version.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <setjmp.h>
#include <stdarg.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()
#endif // AZ_NO_PRECONDITION_CHECKING

#ifdef _az_MOCK_ENABLED
az_result test_policy_transport_retry_response(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

az_result test_policy_transport_retry_response_with_header(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

az_result test_policy_transport_retry_response_with_header_2(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);
void test_az_http_pipeline_policy_credential(void** state);
void test_az_http_pipeline_policy_retry(void** state);
void test_az_http_pipeline_policy_retry_with_header(void** state);
void test_az_http_pipeline_policy_retry_with_header_2(void** state);
#endif // _az_MOCK_ENABLED

static az_result test_policy_transport(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response);

void test_az_http_pipeline_policy_apiversion(void** state);
void test_az_http_pipeline_policy_telemetry(void** state);

az_result test_policy_transport(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies;
  (void)ref_options;
  (void)ref_request;
  (void)ref_response;
  return AZ_OK;
}

static az_span telemetry_policy_expected_value;

static az_result validate_telemetry_policy_value(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies;
  (void)ref_options;
  (void)ref_response;

  az_span header_name = { 0 };
  az_span header_value = { 0 };

  assert_return_code(
      az_http_request_get_header(ref_request, 0, &header_name, &header_value), AZ_OK);

  assert_true(az_span_is_content_equal(header_name, AZ_SPAN_FROM_STR("User-Agent")));
  assert_true(az_span_is_content_equal(header_value, telemetry_policy_expected_value));

  return AZ_OK;
}

void test_az_http_pipeline_policy_telemetry(void** state)
{
  (void)state;

  uint8_t url_buf[100] = { 0 };
  uint8_t header_buf[(2 * sizeof(_az_http_request_header))] = { 0 };

  az_span url_span = AZ_SPAN_FROM_BUFFER(url_buf);
  az_span remainder = az_span_copy(url_span, AZ_SPAN_FROM_STR("url"));
  assert_int_equal(az_span_size(remainder), 97);
  az_span header_span = AZ_SPAN_FROM_BUFFER(header_buf);
  az_http_request request;

  assert_return_code(
      az_http_request_init(
          &request,
          &az_context_application,
          az_http_method_get(),
          url_span,
          3,
          header_span,
          AZ_SPAN_EMPTY),
      AZ_OK);

  // Create policy options
  _az_http_policy_telemetry_options telemetry = _az_http_policy_telemetry_options_create(
      AZ_SPAN_FROM_STR("a-fourty-character-component-id-for-test"));

  telemetry_policy_expected_value
      = AZ_SPAN_FROM_STR("azsdk-c-a-fourty-character-component-id-for-test/" AZ_SDK_VERSION_STRING);

  _az_http_policy policies[1] = {
            {
              ._internal = {
                .process = validate_telemetry_policy_value,
                .options = NULL,
              },
            },
        };

  // Valid Component ID
  assert_return_code(
      az_http_pipeline_policy_telemetry(policies, &telemetry, &request, NULL), AZ_OK);

#ifndef AZ_NO_PRECONDITION_CHECKING
  policies[0] = (_az_http_policy){
              ._internal = {
                .process = test_policy_transport,
                .options = NULL,
              },
            };

  // Empty Component ID
  telemetry.component_name = AZ_SPAN_EMPTY;
  ASSERT_PRECONDITION_CHECKED(
      az_http_pipeline_policy_telemetry(policies, &telemetry, &request, NULL));

  // Component ID too long
  telemetry.component_name = AZ_SPAN_FROM_STR("a-fourty1-character-component-id-for-test");
  ASSERT_PRECONDITION_CHECKED(
      az_http_pipeline_policy_telemetry(policies, &telemetry, &request, NULL));
#endif // AZ_NO_PRECONDITION_CHECKING
}

void test_az_http_pipeline_policy_apiversion(void** state)
{
  (void)state;

  uint8_t buf[100];
  uint8_t header_buf[(2 * sizeof(_az_http_request_header))];
  memset(buf, 0, sizeof(buf));
  memset(header_buf, 0, sizeof(header_buf));

  az_span url_span = AZ_SPAN_FROM_BUFFER(buf);
  az_span remainder = az_span_copy(url_span, AZ_SPAN_FROM_STR("url"));
  assert_int_equal(az_span_size(remainder), 97);
  az_span header_span = AZ_SPAN_FROM_BUFFER(header_buf);
  az_http_request request;

  assert_return_code(
      az_http_request_init(
          &request,
          &az_context_application,
          az_http_method_get(),
          url_span,
          3,
          header_span,
          AZ_SPAN_EMPTY),
      AZ_OK);

  // Create policy options
  _az_http_policy_apiversion_options api_version = _az_http_policy_apiversion_options_default();
  api_version._internal.option_location = _az_http_policy_apiversion_option_location_queryparameter;
  api_version._internal.name = AZ_SPAN_FROM_STR("name");
  api_version._internal.version = AZ_SPAN_FROM_STR("version");

  _az_http_policy policies[1] = {
            {
              ._internal = {
                .process = test_policy_transport,
                .options = NULL,
              },
            },
        };

  // make sure token is not expired
  assert_return_code(
      az_http_pipeline_policy_apiversion(policies, &api_version, &request, NULL), AZ_OK);

  api_version._internal.option_location = _az_http_policy_apiversion_option_location_header;
  assert_return_code(
      az_http_pipeline_policy_apiversion(policies, &api_version, &request, NULL), AZ_OK);
}

#ifdef _az_MOCK_ENABLED

const az_span retry_response = AZ_SPAN_LITERAL_FROM_STR("HTTP/1.1 408 Request Timeout\r\n"
                                                        "Content-Type: text/html; charset=UTF-8\r\n"
                                                        "\r\n"
                                                        "{\r\n"
                                                        "  \"body\":0,\r"
                                                        "}\n");

const az_span retry_response_with_header
    = AZ_SPAN_LITERAL_FROM_STR("HTTP/1.1 408 Request Timeout\r\n"
                               "Content-Type: text/html; charset=UTF-8\r\n"
                               "retry-after-ms: 1600\r\n"
                               "\r\n"
                               "{\r\n"
                               "  \"body\":0,\r"
                               "}\n");

const az_span retry_response_with_header_2
    = AZ_SPAN_LITERAL_FROM_STR("HTTP/1.1 408 Request Timeout\r\n"
                               "Content-Type: text/html; charset=UTF-8\r\n"
                               "Retry-After: 1600\r\n"
                               "\r\n"
                               "{\r\n"
                               "  \"body\":0,\r"
                               "}\n");

az_result test_policy_transport_retry_response(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies;
  (void)ref_options;
  (void)ref_request;
  assert_return_code(az_http_response_init(ref_response, retry_response), AZ_OK);
  return AZ_OK;
}

az_result test_policy_transport_retry_response_with_header(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies;
  (void)ref_options;
  (void)ref_request;
  assert_return_code(az_http_response_init(ref_response, retry_response_with_header), AZ_OK);
  return AZ_OK;
}

az_result test_policy_transport_retry_response_with_header_2(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_policies;
  (void)ref_options;
  (void)ref_request;
  assert_return_code(az_http_response_init(ref_response, retry_response_with_header_2), AZ_OK);
  return AZ_OK;
}

void test_az_http_pipeline_policy_retry(void** state)
{
  (void)state;

  uint8_t buf[100];
  uint8_t header_buf[(2 * sizeof(_az_http_request_header))];
  memset(buf, 0, sizeof(buf));
  memset(header_buf, 0, sizeof(header_buf));

  az_span url_span = AZ_SPAN_FROM_BUFFER(buf);
  az_span remainder = az_span_copy(url_span, AZ_SPAN_FROM_STR("url"));
  assert_int_equal(az_span_size(remainder), 97);
  az_span header_span = AZ_SPAN_FROM_BUFFER(header_buf);
  az_http_request request;

  assert_return_code(
      az_http_request_init(
          &request,
          &az_context_application,
          az_http_method_get(),
          url_span,
          3,
          header_span,
          AZ_SPAN_EMPTY),
      AZ_OK);

  // Create policy options
  az_http_policy_retry_options retry_options = _az_http_policy_retry_options_default();

  _az_http_policy policies[1] = {
            {
              ._internal = {
                .process = test_policy_transport_retry_response,
                .options = NULL,
              },
            },
        };

  // set clock sec required when retrying (will retry 4 times)
  will_return_count(__wrap_az_platform_clock_msec, 0, 4);
  az_http_response response;
  assert_return_code(
      az_http_pipeline_policy_retry(policies, &retry_options, &request, &response), AZ_OK);
}

void test_az_http_pipeline_policy_retry_with_header(void** state)
{
  (void)state;

  uint8_t buf[100];
  uint8_t header_buf[(2 * sizeof(_az_http_request_header))];
  memset(buf, 0, sizeof(buf));
  memset(header_buf, 0, sizeof(header_buf));

  az_span url_span = AZ_SPAN_FROM_BUFFER(buf);
  az_span remainder = az_span_copy(url_span, AZ_SPAN_FROM_STR("url"));
  assert_int_equal(az_span_size(remainder), 97);
  az_span header_span = AZ_SPAN_FROM_BUFFER(header_buf);
  az_http_request request;

  assert_return_code(
      az_http_request_init(
          &request,
          &az_context_application,
          az_http_method_get(),
          url_span,
          3,
          header_span,
          AZ_SPAN_EMPTY),
      AZ_OK);

  // Create policy options
  az_http_policy_retry_options retry_options = _az_http_policy_retry_options_default();
  // make just one retry
  retry_options.max_retries = 1;

  _az_http_policy policies[1] = {
            {
              ._internal = {
                .process = test_policy_transport_retry_response_with_header,
                .options = NULL,
              },
            },
        };

  will_return(__wrap_az_platform_clock_msec, 0);

  az_http_response response;
  assert_return_code(
      az_http_pipeline_policy_retry(policies, &retry_options, &request, &response), AZ_OK);
}

void test_az_http_pipeline_policy_retry_with_header_2(void** state)
{
  (void)state;

  uint8_t buf[100];
  uint8_t header_buf[(2 * sizeof(_az_http_request_header))];
  memset(buf, 0, sizeof(buf));
  memset(header_buf, 0, sizeof(header_buf));

  az_span url_span = AZ_SPAN_FROM_BUFFER(buf);
  az_span remainder = az_span_copy(url_span, AZ_SPAN_FROM_STR("url"));
  assert_int_equal(az_span_size(remainder), 97);
  az_span header_span = AZ_SPAN_FROM_BUFFER(header_buf);
  az_http_request request;

  assert_return_code(
      az_http_request_init(
          &request,
          &az_context_application,
          az_http_method_get(),
          url_span,
          3,
          header_span,
          AZ_SPAN_EMPTY),
      AZ_OK);

  // Create policy options
  az_http_policy_retry_options retry_options = _az_http_policy_retry_options_default();
  // make just one retry
  retry_options.max_retries = 1;

  _az_http_policy policies[1] = {
            {
              ._internal = {
                .process = test_policy_transport_retry_response_with_header_2,
                .options = NULL,
              },
            },
        };

  will_return(__wrap_az_platform_clock_msec, 0);

  az_http_response response;
  assert_return_code(
      az_http_pipeline_policy_retry(policies, &retry_options, &request, &response), AZ_OK);
}

az_result __wrap_az_platform_clock_msec(int64_t* out_clock_msec);
az_result __wrap_az_platform_clock_msec(int64_t* out_clock_msec)
{
  _az_PRECONDITION_NOT_NULL(out_clock_msec);
  *out_clock_msec = (int64_t)mock();
  return AZ_OK;
}

az_result __wrap_az_platform_sleep_msec(int32_t milliseconds);
az_result __wrap_az_platform_sleep_msec(int32_t milliseconds)
{
  (void)milliseconds;
  return AZ_OK;
}

#endif // _az_MOCK_ENABLED

int test_az_policy()
{
#ifndef AZ_NO_PRECONDITION_CHECKING
  SETUP_PRECONDITION_CHECK_TESTS();
#endif // AZ_NO_PRECONDITION_CHECKING

  const struct CMUnitTest tests[] = {
#ifdef _az_MOCK_ENABLED
    cmocka_unit_test(test_az_http_pipeline_policy_retry),
    cmocka_unit_test(test_az_http_pipeline_policy_retry_with_header),
    cmocka_unit_test(test_az_http_pipeline_policy_retry_with_header_2),
#endif // _az_MOCK_ENABLED
    cmocka_unit_test(test_az_http_pipeline_policy_apiversion),
    cmocka_unit_test(test_az_http_pipeline_policy_telemetry),
  };
  return cmocka_run_group_tests_name("az_core_policy", tests, NULL, NULL);
}
