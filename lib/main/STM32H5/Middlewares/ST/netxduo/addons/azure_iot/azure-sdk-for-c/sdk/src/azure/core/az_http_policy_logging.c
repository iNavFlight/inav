// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_http_policy_logging_private.h"
#include "az_span_private.h"
#include <azure/core/az_http_transport.h>
#include <azure/core/az_platform.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <azure/core/_az_cfg.h>

enum
{
  _az_LOG_LENGTHY_VALUE_MAX_LENGTH
  = 50, // When we print values, such as header values, if they are longer than
        // _az_LOG_VALUE_MAX_LENGTH, we trim their contents (decorate with ellipsis in the middle)
        // to make sure each individual header value does not exceed _az_LOG_VALUE_MAX_LENGTH so
        // that they don't blow up the logs.
};

static az_span _az_http_policy_logging_copy_lengthy_value(az_span ref_log_msg, az_span value)
{
  int32_t value_size = az_span_size(value);

  // The caller should validate that ref_log_msg is large enough to contain the value az_span
  // This means, ref_log_msg must have available at least _az_LOG_LENGTHY_VALUE_MAX_LENGTH (i.e. 50)
  // bytes or as much as the size of the value az_span, whichever is smaller.
  _az_PRECONDITION(
      az_span_size(ref_log_msg) >= _az_LOG_LENGTHY_VALUE_MAX_LENGTH
      || az_span_size(ref_log_msg) >= value_size);

  if (value_size <= _az_LOG_LENGTHY_VALUE_MAX_LENGTH)
  {
    return az_span_copy(ref_log_msg, value);
  }

  az_span const ellipsis = AZ_SPAN_FROM_STR(" ... ");
  int32_t const ellipsis_len = az_span_size(ellipsis);

  int32_t const first
      = (_az_LOG_LENGTHY_VALUE_MAX_LENGTH / 2) - ((ellipsis_len / 2) + (ellipsis_len % 2)); // 22

  int32_t const last
      = ((_az_LOG_LENGTHY_VALUE_MAX_LENGTH / 2) + (_az_LOG_LENGTHY_VALUE_MAX_LENGTH % 2)) // 23
      - (ellipsis_len / 2);

  _az_PRECONDITION((first + last + ellipsis_len) == _az_LOG_LENGTHY_VALUE_MAX_LENGTH);

  ref_log_msg = az_span_copy(ref_log_msg, az_span_slice(value, 0, first));
  ref_log_msg = az_span_copy(ref_log_msg, ellipsis);
  return az_span_copy(ref_log_msg, az_span_slice(value, value_size - last, value_size));
}

static az_result _az_http_policy_logging_append_http_request_msg(
    az_http_request const* request,
    az_span* ref_log_msg)
{
  static az_span const auth_header_name = AZ_SPAN_LITERAL_FROM_STR("authorization");

  az_span http_request_string = AZ_SPAN_FROM_STR("HTTP Request : ");
  az_span null_string = AZ_SPAN_FROM_STR("NULL");

  int32_t required_length = az_span_size(http_request_string);
  if (request == NULL)
  {
    required_length += az_span_size(null_string);
  }
  else
  {
    required_length += az_span_size(request->_internal.method) + request->_internal.url_length + 1;
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(*ref_log_msg, required_length);

  az_span remainder = az_span_copy(*ref_log_msg, http_request_string);

  if (request == NULL)
  {
    remainder = az_span_copy(remainder, null_string);
    *ref_log_msg = az_span_slice(*ref_log_msg, 0, _az_span_diff(remainder, *ref_log_msg));
    return AZ_OK;
  }

  remainder = az_span_copy(remainder, request->_internal.method);
  remainder = az_span_copy_u8(remainder, ' ');
  remainder = az_span_copy(
      remainder, az_span_slice(request->_internal.url, 0, request->_internal.url_length));

  int32_t const headers_count = az_http_request_headers_count(request);

  az_span new_line_tab_string = AZ_SPAN_FROM_STR("\n\t");
  az_span colon_separator_string = AZ_SPAN_FROM_STR(" : ");

  for (int32_t index = 0; index < headers_count; ++index)
  {
    az_span header_name = { 0 };
    az_span header_value = { 0 };
    _az_RETURN_IF_FAILED(az_http_request_get_header(request, index, &header_name, &header_value));

    required_length = az_span_size(new_line_tab_string) + az_span_size(header_name);
    if (az_span_size(header_value) > 0)
    {
      required_length += _az_LOG_LENGTHY_VALUE_MAX_LENGTH + az_span_size(colon_separator_string);
    }

    _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, required_length);
    remainder = az_span_copy(remainder, new_line_tab_string);
    remainder = az_span_copy(remainder, header_name);

    if (az_span_size(header_value) > 0 && !az_span_is_content_equal(header_name, auth_header_name))
    {
      remainder = az_span_copy(remainder, colon_separator_string);
      remainder = _az_http_policy_logging_copy_lengthy_value(remainder, header_value);
    }
  }
  *ref_log_msg = az_span_slice(*ref_log_msg, 0, _az_span_diff(remainder, *ref_log_msg));

  return AZ_OK;
}

static az_result _az_http_policy_logging_append_http_response_msg(
    az_http_response* ref_response,
    int64_t duration_msec,
    az_http_request const* request,
    az_span* ref_log_msg)
{
  az_span http_response_string = AZ_SPAN_FROM_STR("HTTP Response (");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(*ref_log_msg, az_span_size(http_response_string));
  az_span remainder = az_span_copy(*ref_log_msg, http_response_string);

  _az_RETURN_IF_FAILED(az_span_i64toa(remainder, duration_msec, &remainder));

  az_span ms_string = AZ_SPAN_FROM_STR("ms)");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(ms_string));
  remainder = az_span_copy(remainder, ms_string);

  if (ref_response == NULL || az_span_size(ref_response->_internal.http_response) == 0)
  {
    az_span is_empty_string = AZ_SPAN_FROM_STR(" is empty");
    _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(is_empty_string));
    remainder = az_span_copy(remainder, is_empty_string);

    *ref_log_msg = az_span_slice(*ref_log_msg, 0, _az_span_diff(remainder, *ref_log_msg));
    return AZ_OK;
  }

  az_span colon_separator_string = AZ_SPAN_FROM_STR(" : ");
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(colon_separator_string));
  remainder = az_span_copy(remainder, colon_separator_string);

  az_http_response_status_line status_line = { 0 };
  _az_RETURN_IF_FAILED(az_http_response_get_status_line(ref_response, &status_line));
  _az_RETURN_IF_FAILED(az_span_u64toa(remainder, (uint64_t)status_line.status_code, &remainder));

  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, az_span_size(status_line.reason_phrase) + 1);
  remainder = az_span_copy_u8(remainder, ' ');
  remainder = az_span_copy(remainder, status_line.reason_phrase);

  az_span new_line_tab_string = AZ_SPAN_FROM_STR("\n\t");

  az_result result = AZ_OK;
  az_span header_name = { 0 };
  az_span header_value = { 0 };
  while (az_result_succeeded(
      result = az_http_response_get_next_header(ref_response, &header_name, &header_value)))
  {
    int32_t required_length = az_span_size(new_line_tab_string) + az_span_size(header_name);
    if (az_span_size(header_value) > 0)
    {
      required_length += _az_LOG_LENGTHY_VALUE_MAX_LENGTH + az_span_size(colon_separator_string);
    }

    _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, required_length);

    remainder = az_span_copy(remainder, new_line_tab_string);
    remainder = az_span_copy(remainder, header_name);

    if (az_span_size(header_value) > 0)
    {
      remainder = az_span_copy(remainder, colon_separator_string);
      remainder = _az_http_policy_logging_copy_lengthy_value(remainder, header_value);
    }
  }

  // Response payload was invalid or corrupted in some way.
  if (result != AZ_ERROR_HTTP_END_OF_HEADERS)
  {
    return result;
  }

  az_span new_lines_string = AZ_SPAN_FROM_STR("\n\n");
  az_span arrow_separator_string = AZ_SPAN_FROM_STR(" -> ");
  int32_t required_length = az_span_size(new_lines_string) + az_span_size(arrow_separator_string);
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, required_length);

  remainder = az_span_copy(remainder, new_lines_string);
  remainder = az_span_copy(remainder, arrow_separator_string);

  az_span append_request = remainder;
  _az_RETURN_IF_FAILED(_az_http_policy_logging_append_http_request_msg(request, &append_request));

  *ref_log_msg = az_span_slice(
      *ref_log_msg, 0, _az_span_diff(remainder, *ref_log_msg) + az_span_size(append_request));
  return AZ_OK;
}

void _az_http_policy_logging_log_http_request(az_http_request const* request)
{
  uint8_t log_msg_buf[AZ_LOG_MESSAGE_BUFFER_SIZE] = { 0 };
  az_span log_msg = AZ_SPAN_FROM_BUFFER(log_msg_buf);

  (void)_az_http_policy_logging_append_http_request_msg(request, &log_msg);

  _az_LOG_WRITE(AZ_LOG_HTTP_REQUEST, log_msg);
}

void _az_http_policy_logging_log_http_response(
    az_http_response const* response,
    int64_t duration_msec,
    az_http_request const* request)
{
  uint8_t log_msg_buf[AZ_LOG_MESSAGE_BUFFER_SIZE] = { 0 };
  az_span log_msg = AZ_SPAN_FROM_BUFFER(log_msg_buf);

  az_http_response response_copy = *response;

  (void)_az_http_policy_logging_append_http_response_msg(
      &response_copy, duration_msec, request, &log_msg);

  _az_LOG_WRITE(AZ_LOG_HTTP_RESPONSE, log_msg);
}

#ifndef AZ_NO_LOGGING
AZ_NODISCARD az_result az_http_pipeline_policy_logging(
    _az_http_policy* ref_policies,
    void* ref_options,
    az_http_request* ref_request,
    az_http_response* ref_response)
{
  (void)ref_options;

  if (_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_REQUEST))
  {
    _az_http_policy_logging_log_http_request(ref_request);
  }

  if (!_az_LOG_SHOULD_WRITE(AZ_LOG_HTTP_RESPONSE))
  {
    // If no logging is needed, do not even measure the response time.
    return _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);
  }

  int64_t start = 0;
  _az_RETURN_IF_FAILED(az_platform_clock_msec(&start));

  az_result const result = _az_http_pipeline_nextpolicy(ref_policies, ref_request, ref_response);

  int64_t end = 0;
  _az_RETURN_IF_FAILED(az_platform_clock_msec(&end));
  _az_http_policy_logging_log_http_response(ref_response, end - start, ref_request);

  return result;
}
#endif // AZ_NO_LOGGING
