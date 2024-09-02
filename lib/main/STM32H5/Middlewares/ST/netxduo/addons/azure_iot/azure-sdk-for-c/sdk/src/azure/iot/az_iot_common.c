// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>
#include <azure/iot/az_iot_common.h>
#include <azure/iot/internal/az_iot_common_internal.h>

#include <azure/core/internal/az_log_internal.h>
#include <azure/core/internal/az_retry_internal.h>

#include <azure/core/_az_cfg.h>

static const az_span hub_client_param_separator_span = AZ_SPAN_LITERAL_FROM_STR("&");
static const az_span hub_client_param_equals_span = AZ_SPAN_LITERAL_FROM_STR("=");

AZ_NODISCARD az_result az_iot_message_properties_init(
    az_iot_message_properties* properties,
    az_span buffer,
    int32_t written_length)
{
  _az_PRECONDITION_NOT_NULL(properties);
  _az_PRECONDITION_VALID_SPAN(buffer, 0, true);
  _az_PRECONDITION_RANGE(0, written_length, az_span_size(buffer));

  properties->_internal.properties_buffer = buffer;
  properties->_internal.properties_written = written_length;
  properties->_internal.current_property_index = 0;

  return AZ_OK;
}

AZ_NODISCARD az_result
az_iot_message_properties_append(az_iot_message_properties* properties, az_span name, az_span value)
{
  _az_PRECONDITION_NOT_NULL(properties);
  _az_PRECONDITION_VALID_SPAN(name, 1, false);
  _az_PRECONDITION_VALID_SPAN(value, 1, false);

  int32_t prop_length = properties->_internal.properties_written;

  az_span remainder = az_span_slice_to_end(properties->_internal.properties_buffer, prop_length);

  int32_t required_length = az_span_size(name) + az_span_size(value) + 1;

  if (prop_length > 0)
  {
    required_length += 1;
  }

  _az_RETURN_IF_NOT_ENOUGH_SIZE(remainder, required_length);

  if (prop_length > 0)
  {
    remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_separator_span));
  }

  remainder = az_span_copy(remainder, name);
  remainder = az_span_copy_u8(remainder, *az_span_ptr(hub_client_param_equals_span));
  az_span_copy(remainder, value);

  properties->_internal.properties_written += required_length;

  return AZ_OK;
}

AZ_NODISCARD az_result az_iot_message_properties_find(
    az_iot_message_properties* properties,
    az_span name,
    az_span* out_value)
{
  _az_PRECONDITION_NOT_NULL(properties);
  _az_PRECONDITION_VALID_SPAN(name, 1, false);
  _az_PRECONDITION_NOT_NULL(out_value);

  az_span remaining = az_span_slice(
      properties->_internal.properties_buffer, 0, properties->_internal.properties_written);

  while (az_span_size(remaining) != 0)
  {
    int32_t index = 0;
    az_span delim_span
        = _az_span_token(remaining, hub_client_param_equals_span, &remaining, &index);
    if (index != -1)
    {
      if (az_span_is_content_equal(delim_span, name))
      {
        *out_value = _az_span_token(remaining, hub_client_param_separator_span, &remaining, &index);
        return AZ_OK;
      }

      _az_span_token(remaining, hub_client_param_separator_span, &remaining, &index);
    }
  }

  return AZ_ERROR_ITEM_NOT_FOUND;
}

AZ_NODISCARD az_result az_iot_message_properties_next(
    az_iot_message_properties* properties,
    az_span* out_name,
    az_span* out_value)
{
  _az_PRECONDITION_NOT_NULL(properties);
  _az_PRECONDITION_NOT_NULL(out_name);
  _az_PRECONDITION_NOT_NULL(out_value);

  int32_t index = (int32_t)properties->_internal.current_property_index;
  int32_t prop_length = properties->_internal.properties_written;

  if (index == prop_length)
  {
    *out_name = AZ_SPAN_EMPTY;
    *out_value = AZ_SPAN_EMPTY;
    return AZ_ERROR_IOT_END_OF_PROPERTIES;
  }

  az_span remainder;
  az_span prop_span = az_span_slice(properties->_internal.properties_buffer, index, prop_length);

  int32_t location = 0;
  *out_name = _az_span_token(prop_span, hub_client_param_equals_span, &remainder, &location);
  *out_value = _az_span_token(remainder, hub_client_param_separator_span, &remainder, &location);
  if (az_span_size(remainder) == 0)
  {
    properties->_internal.current_property_index = (uint32_t)prop_length;
  }
  else
  {
    properties->_internal.current_property_index += (uint32_t)(_az_span_diff(remainder, prop_span));
  }

  return AZ_OK;
}

AZ_NODISCARD int32_t az_iot_calculate_retry_delay(
    int32_t operation_msec,
    int16_t attempt,
    int32_t min_retry_delay_msec,
    int32_t max_retry_delay_msec,
    int32_t random_jitter_msec)
{
  _az_PRECONDITION_RANGE(0, operation_msec, INT32_MAX - 1);
  _az_PRECONDITION_RANGE(0, attempt, INT16_MAX - 1);
  _az_PRECONDITION_RANGE(0, min_retry_delay_msec, INT32_MAX - 1);
  _az_PRECONDITION_RANGE(0, max_retry_delay_msec, INT32_MAX - 1);
  _az_PRECONDITION_RANGE(0, random_jitter_msec, INT32_MAX - 1);

  if (_az_LOG_SHOULD_WRITE(AZ_LOG_IOT_RETRY))
  {
    _az_LOG_WRITE(AZ_LOG_IOT_RETRY, AZ_SPAN_EMPTY);
  }

  int32_t delay = _az_retry_calc_delay(attempt, min_retry_delay_msec, max_retry_delay_msec);

  if (delay < 0)
  {
    delay = max_retry_delay_msec;
  }

  if (max_retry_delay_msec - delay > random_jitter_msec)
  {
    delay += random_jitter_msec;
  }

  delay -= operation_msec;

  return delay > 0 ? delay : 0;
}

AZ_NODISCARD int32_t _az_iot_u32toa_size(uint32_t number)
{
  if (number == 0)
  {
    return 1;
  }

  uint32_t div = _az_SMALLEST_10_DIGIT_NUMBER;
  int32_t digit_count = _az_MAX_SIZE_FOR_UINT32;
  while (number / div == 0)
  {
    div /= _az_NUMBER_OF_DECIMAL_VALUES;
    digit_count--;
  }

  return digit_count;
}

AZ_NODISCARD int32_t _az_iot_u64toa_size(uint64_t number)
{
  if (number == 0)
  {
    return 1;
  }

  uint64_t div = _az_SMALLEST_20_DIGIT_NUMBER;
  int32_t digit_count = _az_MAX_SIZE_FOR_UINT64;
  while (number / div == 0)
  {
    div /= _az_NUMBER_OF_DECIMAL_VALUES;
    digit_count--;
  }

  return digit_count;
}

AZ_NODISCARD az_result
_az_span_copy_url_encode(az_span destination, az_span source, az_span* out_remainder)
{
  int32_t length = 0;
  _az_RETURN_IF_FAILED(_az_span_url_encode(destination, source, &length));
  *out_remainder = az_span_slice(destination, length, az_span_size(destination));
  return AZ_OK;
}
