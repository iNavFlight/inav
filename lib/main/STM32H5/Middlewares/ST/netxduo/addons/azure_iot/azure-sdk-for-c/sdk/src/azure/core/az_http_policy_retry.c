// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_http_private.h"
#include <azure/core/az_config.h>
#include <azure/core/az_platform.h>
#include <azure/core/internal/az_config_internal.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_retry_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <azure/core/_az_cfg.h>

AZ_NODISCARD az_http_policy_retry_options _az_http_policy_retry_options_default()
{
  return (az_http_policy_retry_options){
    .max_retries = 4,
    .retry_delay_msec = 4 * _az_TIME_MILLISECONDS_PER_SECOND, // 4 seconds
    .max_retry_delay_msec
    = 2 * _az_TIME_SECONDS_PER_MINUTE * _az_TIME_MILLISECONDS_PER_SECOND, // 2 minutes
  };
}

// TODO: Add unit tests
AZ_INLINE az_result _az_http_policy_retry_append_http_retry_msg(
    int32_t attempt,
    int32_t delay_msec,
    az_span* ref_log_msg)
{
  az_span retry_count_string = AZ_SPAN_FROM_STR("HTTP Retry attempt #");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(*ref_log_msg, az_span_size(retry_count_string));
  az_span remainder = az_span_copy(*ref_log_msg, retry_count_string);

  _az_RETURN_IF_FAILED(az_span_i32toa(remainder, attempt, &remainder));

  az_span infix_string = AZ_SPAN_FROM_STR(" will be made in ");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(infix_string));
  remainder = az_span_copy(remainder, infix_string);

  _az_RETURN_IF_FAILED(az_span_i32toa(remainder, delay_msec, &remainder));

  az_span suffix_string = AZ_SPAN_FROM_STR("ms.");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(suffix_string));
  remainder = az_span_copy(remainder, suffix_string);

  *ref_log_msg = az_span_slice(*ref_log_msg, 0, _az_span_diff(remainder, *ref_log_msg));

  return AZ_OK;
}

AZ_INLINE void _az_http_policy_retry_log(int32_t attempt, int32_t delay_msec)
{
  uint8_t log_msg_buf[AZ_LOG_MESSAGE_BUFFER_SIZE] = { 0 };
  az_span log_msg = AZ_SPAN_FROM_BUFFER(log_msg_buf);

  (void)_az_http_policy_retry_append_http_retry_msg(attempt, delay_msec, &log_msg);

  _az_LOG_WRITE(AZ_LOG_HTTP_RETRY, log_msg);
}

AZ_INLINE AZ_NODISCARD int32_t _az_uint32_span_to_int32(az_span span)
{
  uint32_t value = 0;
  if (az_result_failed(az_span_atou32(span, &value)))
  {
    return -1;
  }

  return value < INT32_MAX ? (int32_t)value : INT32_MAX;
}

AZ_INLINE AZ_NODISCARD bool _az_http_policy_retry_should_retry_http_response_code(
    az_http_status_code http_response_code)
{
  switch (http_response_code)
  {
    case AZ_HTTP_STATUS_CODE_REQUEST_TIMEOUT:
    case AZ_HTTP_STATUS_CODE_TOO_MANY_REQUESTS:
    case AZ_HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR:
    case AZ_HTTP_STATUS_CODE_BAD_GATEWAY:
    case AZ_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE:
    case AZ_HTTP_STATUS_CODE_GATEWAY_TIMEOUT:
      return true;
    default:
      return false;
  }
}

AZ_INLINE AZ_NODISCARD az_result _az_http_policy_retry_get_retry_after(
    az_http_response* ref_response,
    bool* should_retry,
    int32_t* retry_after_msec)
{
  az_http_response_status_line status_line = { 0 };
  _az_RETURN_IF_FAILED(az_http_response_get_status_line(ref_response, &status_line));

  if (!_az_http_policy_retry_should_retry_http_response_code(status_line.status_code))
  {
    *should_retry = false;
    *retry_after_msec = -1;
    return AZ_OK;
  }

  *should_retry = true;

  // Try to get the value of retry-after header, if there's one.
  az_span header_name = { 0 };
  az_span header_value = { 0 };
  while (az_result_succeeded(
      az_http_response_get_next_header(ref_response, &header_name, &header_value)))
  {
    if (az_span_is_content_equal_ignoring_case(header_name, AZ_SPAN_FROM_STR("retry-after-ms"))
        || az_span_is_content_equal_ignoring_case(
            header_name, AZ_SPAN_FROM_STR("x-ms-retry-after-ms")))
    {
      // The value is in milliseconds.
      int32_t const msec = _az_uint32_span_to_int32(header_value);
      if (msec >= 0) // int32_t max == ~24 days
      {
        *retry_after_msec = msec;
        return AZ_OK;
      }
    }
    else if (az_span_is_content_equal_ignoring_case(header_name, AZ_SPAN_FROM_STR("Retry-After")))
    {
      // The value is either seconds or date.
      int32_t const seconds = _az_uint32_span_to_int32(header_value);
      if (seconds >= 0) // int32_t max == ~68 years
      {
        *retry_after_msec = (seconds <= (INT32_MAX / _az_TIME_MILLISECONDS_PER_SECOND))
            ? seconds * _az_TIME_MILLISECONDS_PER_SECOND
            : INT32_MAX;

        return AZ_OK;
      }

      // TODO: Other possible value is HTTP Date. For that, we'll need to parse date, get
      // current date, subtract one from another, get seconds. And the device should have a
      // sense of calendar clock.
    }
  }

  *retry_after_msec = -1;
  return AZ_OK;
}

AZ_NODISCARD az_result az_http_pipeline_policy_retry(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  az_http_policy_retry_options const* const retry_options
      = (az_http_policy_retry_options const*)ref_options;

  int32_t const max_retries = retry_options->max_retries;
  int32_t const retry_delay_msec = retry_options->retry_delay_msec;
  int32_t const max_retry_delay_msec = retry_options->max_retry_delay_msec;

  _az_RETURN_IF_FAILED(_az_http_request_mark_retry_headers_start(ref_request));

  az_context* const context = ref_request->_internal.context;

  bool const should_log = _az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_RETRY);
  az_result result = AZ_OK;
  int32_t attempt = 1;
  while (true)
  {
    _az_RETURN_IF_FAILED(
        az_http_response_init(ref_response, ref_response->_internal.http_response));
    _az_RETURN_IF_FAILED(_az_http_request_remove_retry_headers(ref_request));

    result = _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);

    // Even HTTP 429, or 502 are expected to be AZ_OK, so the failed result is not retriable.
    if (attempt > max_retries || az_result_failed(result))
    {
      return result;
    }

    int32_t retry_after_msec = -1;
    bool should_retry = false;
    az_http_response response_copy = *ref_response;

    _az_RETURN_IF_FAILED(
        _az_http_policy_retry_get_retry_after(&response_copy, &should_retry, &retry_after_msec));

    if (!should_retry)
    {
      return result;
    }

    ++attempt;

    if (retry_after_msec < 0)
    { // there wasn't any kind of "retry-after" response header
      retry_after_msec = _az_retry_calc_delay(attempt, retry_delay_msec, max_retry_delay_msec);
    }

    if (should_log)
    {
      _az_http_policy_retry_log(attempt, retry_after_msec);
    }

    _az_RETURN_IF_FAILED(az_platform_sleep_msec(retry_after_msec));

    if (context != NULL)
    {
      int64_t clock = 0;
      _az_RETURN_IF_FAILED(az_platform_clock_msec(&clock));
      if (az_context_has_expired(context, clock))
      {
        return AZ_ERROR_CANCELED;
      }
    }
  }
}
