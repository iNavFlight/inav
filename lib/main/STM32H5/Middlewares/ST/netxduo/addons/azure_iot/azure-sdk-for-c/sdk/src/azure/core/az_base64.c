// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_base64.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <azure/core/_az_cfg.h>

// The maximum integer length of binary data that can be encoded into base 64 text and still fit
// into an az_span which has an int32_t length, i.e. (INT32_MAX / 4) * 3;
#define _az_MAX_SAFE_ENCODED_LENGTH 1610612733

#define _az_ENCODING_PAD '='

typedef enum
{
  _az_base64_mode_standard,
  _az_base64_mode_url
} _az_base64_mode;

static char const _az_base64_encode_array[65]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static AZ_NODISCARD int32_t _az_base64_encode(uint8_t* three_bytes)
{
  int32_t i = (*three_bytes << 16) | (*(three_bytes + 1) << 8) | *(three_bytes + 2);

  int32_t i0 = _az_base64_encode_array[i >> 18];
  int32_t i1 = _az_base64_encode_array[(i >> 12) & 0x3F];
  int32_t i2 = _az_base64_encode_array[(i >> 6) & 0x3F];
  int32_t i3 = _az_base64_encode_array[i & 0x3F];

  return i0 | (i1 << 8) | (i2 << 16) | (i3 << 24);
}

static AZ_NODISCARD int32_t _az_base64_encode_and_pad_one(uint8_t* two_bytes)
{
  int32_t i = (*two_bytes << 16) | (*(two_bytes + 1) << 8);

  int32_t i0 = _az_base64_encode_array[i >> 18];
  int32_t i1 = _az_base64_encode_array[(i >> 12) & 0x3F];
  int32_t i2 = _az_base64_encode_array[(i >> 6) & 0x3F];

  return i0 | (i1 << 8) | (i2 << 16) | (_az_ENCODING_PAD << 24);
}

static AZ_NODISCARD int32_t _az_base64_encode_and_pad_two(uint8_t* one_byte)
{
  int32_t i = (*one_byte << 8);

  int32_t i0 = _az_base64_encode_array[i >> 10];
  int32_t i1 = _az_base64_encode_array[(i >> 4) & 0x3F];

  return i0 | (i1 << 8) | (_az_ENCODING_PAD << 16) | (_az_ENCODING_PAD << 24);
}

static void _az_base64_write_int_as_four_bytes(uint8_t* destination, int32_t value)
{
  *(destination + 3) = (uint8_t)((value >> 24) & 0xFF);
  *(destination + 2) = (uint8_t)((value >> 16) & 0xFF);
  *(destination + 1) = (uint8_t)((value >> 8) & 0xFF);
  *(destination + 0) = (uint8_t)(value & 0xFF);
}

AZ_NODISCARD az_result
az_base64_encode(az_span destination_base64_text, az_span source_bytes, int32_t* out_written)
{
  _az_PRECONDITION_VALID_SPAN(destination_base64_text, 4, false);
  _az_PRECONDITION_VALID_SPAN(source_bytes, 1, false);
  _az_PRECONDITION_NOT_NULL(out_written);

  int32_t source_length = az_span_size(source_bytes);
  uint8_t* source_ptr = az_span_ptr(source_bytes);

  int32_t destination_length = az_span_size(destination_base64_text);
  uint8_t* destination_ptr = az_span_ptr(destination_base64_text);

  if (destination_length < az_base64_get_max_encoded_size(source_length))
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  int32_t source_index = 0;
  int32_t result = 0;

  while (source_index < source_length - 2)
  {
    result = _az_base64_encode(source_ptr + source_index);
    _az_base64_write_int_as_four_bytes(destination_ptr, result);
    destination_ptr += 4;
    source_index += 3;
  }

  if (source_index == source_length - 1)
  {
    result = _az_base64_encode_and_pad_two(source_ptr + source_index);
    _az_base64_write_int_as_four_bytes(destination_ptr, result);
    destination_ptr += 4;
    source_index += 1;
  }
  else if (source_index == source_length - 2)
  {
    result = _az_base64_encode_and_pad_one(source_ptr + source_index);
    _az_base64_write_int_as_four_bytes(destination_ptr, result);
    destination_ptr += 4;
    source_index += 2;
  }

  *out_written = (int32_t)(destination_ptr - az_span_ptr(destination_base64_text));
  return AZ_OK;
}

AZ_NODISCARD int32_t az_base64_get_max_encoded_size(int32_t source_bytes_size)
{
  _az_PRECONDITION_RANGE(0, source_bytes_size, _az_MAX_SAFE_ENCODED_LENGTH);
  return (((source_bytes_size + 2) / 3) * 4);
}

static int32_t _get_base64_decoded_char(int32_t c, _az_base64_mode mode)
{
  if (mode == _az_base64_mode_url)
  {
    if (c == '+' || c == '/')
    {
      return -1; // can't use + or / with URL encoding
    }

    if (c == '-')
    {
      c = '+'; // - becomes a +
    }
    else if (c == '_')
    {
      c = '/'; // _ becomes a /
    }
  }

  if (c >= 'A' && c <= 'Z')
  {
    return (c - 'A');
  }

  if (c >= 'a' && c <= 'z')
  {
    return 26 + (c - 'a');
  }

  if (c >= '0' && c <= '9')
  {
    return 52 + (c - '0');
  }

  if (c == '+')
  {
    return 62;
  }

  if (c == '/')
  {
    return 63;
  }

  return -1;
}

static AZ_NODISCARD int32_t
_az_base64_decode_four_bytes(uint8_t* encoded_bytes, _az_base64_mode mode)
{
  int32_t i0 = *encoded_bytes;
  int32_t i1 = *(encoded_bytes + 1);
  int32_t i2 = *(encoded_bytes + 2);
  int32_t i3 = *(encoded_bytes + 3);

  i0 = _get_base64_decoded_char(i0, mode);
  i1 = _get_base64_decoded_char(i1, mode);
  i2 = _get_base64_decoded_char(i2, mode);
  i3 = _get_base64_decoded_char(i3, mode);

  if (i0 == -1 || i1 == -1 || i2 == -1 || i3 == -1)
  {
    return -1;
  }

  i0 <<= 18;
  i1 <<= 12;
  i2 <<= 6;

  i0 |= i3;
  i1 |= i2;

  i0 |= i1;
  return i0;
}

static void _az_base64_write_three_low_order_bytes(uint8_t* destination, int32_t value)
{
  *destination = (uint8_t)(value >> 16);
  *(destination + 1) = (uint8_t)(value >> 8);
  *(destination + 2) = (uint8_t)(value);
}

static az_result _az_base64_decode(
    az_span destination_bytes,
    az_span source_base64_url_text,
    int32_t* out_written,
    _az_base64_mode mode)
{
  int32_t source_length = az_span_size(source_base64_url_text);
  uint8_t* source_ptr = az_span_ptr(source_base64_url_text);

  int32_t destination_length = az_span_size(destination_bytes);
  uint8_t* destination_ptr = az_span_ptr(destination_bytes);

  if (destination_length < az_base64_get_max_decoded_size(source_length) - 2)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  int32_t source_index = 0;
  int32_t destination_index = 0;

  while (source_index < source_length - 4)
  {
    int32_t result = _az_base64_decode_four_bytes(source_ptr + source_index, mode);
    if (result < 0)
    {
      return AZ_ERROR_UNEXPECTED_CHAR;
    }
    _az_base64_write_three_low_order_bytes(destination_ptr, result);
    destination_ptr += 3;
    destination_index += 3;
    source_index += 4;
  }

  // If using standard base64 decoding, there is a precondition guaranteeing size is divisible by 4.
  // Otherwise with url encoding, we can assume padding characters.
  // If length is divisible by four, do nothing. Else, we assume up to two padding characters.
  int32_t source_length_mod_four = source_length % 4;
  int32_t i0 = *(source_ptr + source_index);
  int32_t i1 = *(source_ptr + source_index + 1);
  int32_t i2 = source_length_mod_four == 2 ? _az_ENCODING_PAD : *(source_ptr + source_index + 2);
  int32_t i3 = source_length_mod_four == 2 || source_length_mod_four == 3
      ? _az_ENCODING_PAD
      : *(source_ptr + source_index + 3);

  i0 = _get_base64_decoded_char(i0, mode);
  i1 = _get_base64_decoded_char(i1, mode);

  i0 <<= 18;
  i1 <<= 12;

  i0 |= i1;

  if (i3 != _az_ENCODING_PAD)
  {
    i2 = _get_base64_decoded_char(i2, mode);
    i3 = _get_base64_decoded_char(i3, mode);

    i2 <<= 6;

    i0 |= i3;
    i0 |= i2;

    if (i0 < 0)
    {
      return AZ_ERROR_UNEXPECTED_CHAR;
    }
    if (destination_index > destination_length - 3)
    {
      return AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    _az_base64_write_three_low_order_bytes(destination_ptr, i0);
    destination_ptr += 3;
  }
  else if (i2 != _az_ENCODING_PAD)
  {
    i2 = _get_base64_decoded_char(i2, mode);

    i2 <<= 6;

    i0 |= i2;

    if (i0 < 0)
    {
      return AZ_ERROR_UNEXPECTED_CHAR;
    }
    if (destination_index > destination_length - 2)
    {
      return AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    *(destination_ptr + 1) = (uint8_t)(i0 >> 8);
    *destination_ptr = (uint8_t)(i0 >> 16);
    destination_ptr += 2;
  }
  else
  {
    if (i0 < 0)
    {
      return AZ_ERROR_UNEXPECTED_CHAR;
    }
    if (destination_index > destination_length - 1)
    {
      return AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    *destination_ptr = (uint8_t)(i0 >> 16);
    destination_ptr += 1;
  }

  *out_written = (int32_t)(destination_ptr - az_span_ptr(destination_bytes));
  return AZ_OK;
}

AZ_NODISCARD az_result
az_base64_decode(az_span destination_bytes, az_span source_base64_text, int32_t* out_written)
{
  _az_PRECONDITION_VALID_SPAN(destination_bytes, 1, false);
  _az_PRECONDITION_VALID_SPAN(source_base64_text, 4, false);
  _az_PRECONDITION_NOT_NULL(out_written);

  int32_t source_length = az_span_size(source_base64_text);

  // The input must be non-empty and a multiple of 4 to be valid.
  if (source_length == 0 || source_length % 4 != 0)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }

  return _az_base64_decode(
      destination_bytes, source_base64_text, out_written, _az_base64_mode_standard);
}

AZ_NODISCARD int32_t az_base64_get_max_decoded_size(int32_t source_base64_text_size)
{
  _az_PRECONDITION(source_base64_text_size >= 0);
  return (source_base64_text_size / 4) * 3;
}

AZ_NODISCARD az_result az_base64_url_decode(
    az_span destination_bytes,
    az_span source_base64_url_text,
    int32_t* out_written)
{
  _az_PRECONDITION_VALID_SPAN(destination_bytes, 1, false);
  _az_PRECONDITION_VALID_SPAN(source_base64_url_text, 2, false);
  _az_PRECONDITION_NOT_NULL(out_written);

  int32_t source_length = az_span_size(source_base64_url_text);

  // The input must be non-empty and a minimum of two characters long.
  // There can only be two assumed padding characters.
  if (source_length == 0 || source_length % 4 == 1)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }

  return _az_base64_decode(
      destination_bytes, source_base64_url_text, out_written, _az_base64_mode_url);
}

AZ_NODISCARD int32_t az_base64_url_get_max_decoded_size(int32_t source_base64_url_text_size)
{
  _az_PRECONDITION(source_base64_url_text_size >= 0);
  return (source_base64_url_text_size / 4) * 3;
}
