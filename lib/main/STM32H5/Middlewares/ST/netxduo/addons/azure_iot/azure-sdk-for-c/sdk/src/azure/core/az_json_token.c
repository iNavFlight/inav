// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include "az_json_private.h"

#include "az_span_private.h"
#include <azure/core/_az_cfg.h>

static az_span _az_json_token_copy_into_span_helper(
    az_json_token const* json_token,
    az_span destination)
{
  _az_PRECONDITION(json_token->_internal.is_multisegment);

  if (json_token->size == 0)
  {
    return destination;
  }

  for (int32_t i = json_token->_internal.start_buffer_index;
       i <= json_token->_internal.end_buffer_index;
       i++)
  {
    az_span source = json_token->_internal.pointer_to_first_buffer[i];
    if (i == json_token->_internal.start_buffer_index)
    {
      source = az_span_slice_to_end(source, json_token->_internal.start_buffer_offset);
    }
    else if (i == json_token->_internal.end_buffer_index)
    {
      source = az_span_slice(source, 0, json_token->_internal.end_buffer_offset);
    }
    destination = az_span_copy(destination, source);
  }

  return destination;
}

az_span az_json_token_copy_into_span(az_json_token const* json_token, az_span destination)
{
  _az_PRECONDITION_VALID_SPAN(destination, json_token->size, false);

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_copy(destination, token_slice);
  }

  // Token straddles more than one segment
  return _az_json_token_copy_into_span_helper(json_token, destination);
}

AZ_NODISCARD static uint8_t _az_json_unescape_single_byte(uint8_t ch)
{
  switch (ch)
  {
    case 'b':
      return '\b';
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case '\\':
    case '"':
    case '/':
    default:
    {
      // We are assuming the JSON token string has already been validated before this and we won't
      // have unexpected bytes folowing the back slash (for example \q). Therefore, just return the
      // same character back for such cases.
      return ch;
    }
  }
}

AZ_NODISCARD static bool _az_json_token_is_text_equal_helper(
    az_span token_slice,
    az_span* expected_text,
    bool* next_char_escaped)
{
  int32_t token_size = az_span_size(token_slice);
  uint8_t* token_ptr = az_span_ptr(token_slice);

  int32_t expected_size = az_span_size(*expected_text);
  uint8_t* expected_ptr = az_span_ptr(*expected_text);

  int32_t token_idx = 0;
  for (int32_t i = 0; i < expected_size; i++)
  {
    if (token_idx >= token_size)
    {
      *expected_text = az_span_slice_to_end(*expected_text, i);
      return false;
    }
    uint8_t token_byte = token_ptr[token_idx];

    if (token_byte == '\\' || *next_char_escaped)
    {
      if (*next_char_escaped)
      {
        token_byte = _az_json_unescape_single_byte(token_byte);
      }
      else
      {
        token_idx++;
        if (token_idx >= token_size)
        {
          *next_char_escaped = true;
          *expected_text = az_span_slice_to_end(*expected_text, i);
          return false;
        }
        token_byte = _az_json_unescape_single_byte(token_ptr[token_idx]);
      }
      *next_char_escaped = false;

      // TODO: Characters escaped in the form of \uXXXX where XXXX is the UTF-16 code point, is
      // not currently supported.
      // To do this, we need to encode UTF-16 codepoints (including surrogate pairs) into UTF-8.
      if (token_byte == 'u')
      {
        *expected_text = AZ_SPAN_EMPTY;
        return false;
      }
    }

    if (token_byte != expected_ptr[i])
    {
      *expected_text = AZ_SPAN_EMPTY;
      return false;
    }

    token_idx++;
  }

  *expected_text = AZ_SPAN_EMPTY;

  // Only return true if the size of the unescaped token matches the expected size exactly.
  return token_idx == token_size;
}

AZ_NODISCARD bool az_json_token_is_text_equal(
    az_json_token const* json_token,
    az_span expected_text)
{
  _az_PRECONDITION_NOT_NULL(json_token);

  // Cannot compare the value of non-string token kinds
  if (json_token->kind != AZ_JSON_TOKEN_STRING && json_token->kind != AZ_JSON_TOKEN_PROPERTY_NAME)
  {
    return false;
  }

  az_span token_slice = json_token->slice;

  // There is nothing to unescape here, compare directly.
  if (!json_token->_internal.string_has_escaped_chars)
  {
    // Contiguous token
    if (!json_token->_internal.is_multisegment)
    {
      return az_span_is_content_equal(token_slice, expected_text);
    }

    // Token straddles more than one segment
    for (int32_t i = json_token->_internal.start_buffer_index;
         i <= json_token->_internal.end_buffer_index;
         i++)
    {
      az_span source = json_token->_internal.pointer_to_first_buffer[i];
      if (i == json_token->_internal.start_buffer_index)
      {
        source = az_span_slice_to_end(source, json_token->_internal.start_buffer_offset);
      }
      else if (i == json_token->_internal.end_buffer_index)
      {
        source = az_span_slice(source, 0, json_token->_internal.end_buffer_offset);
      }

      int32_t source_size = az_span_size(source);
      if (az_span_size(expected_text) < source_size
          || !az_span_is_content_equal(source, az_span_slice(expected_text, 0, source_size)))
      {
        return false;
      }
      expected_text = az_span_slice_to_end(expected_text, source_size);
    }
    // Only return true if we have gone through and compared the entire expected_text.
    return az_span_size(expected_text) == 0;
  }

  int32_t token_size = json_token->size;
  int32_t expected_size = az_span_size(expected_text);

  // No need to try to unescape the token slice, since the lengths won't match anyway.
  // Unescaping always shrinks the string, at most by a factor of 6.
  if (token_size < expected_size
      || (token_size / _az_MAX_EXPANSION_FACTOR_WHILE_ESCAPING) > expected_size)
  {
    return false;
  }

  bool next_char_escaped = false;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return _az_json_token_is_text_equal_helper(token_slice, &expected_text, &next_char_escaped);
  }

  // Token straddles more than one segment
  for (int32_t i = json_token->_internal.start_buffer_index;
       i <= json_token->_internal.end_buffer_index;
       i++)
  {
    az_span source = json_token->_internal.pointer_to_first_buffer[i];
    if (i == json_token->_internal.start_buffer_index)
    {
      source = az_span_slice_to_end(source, json_token->_internal.start_buffer_offset);
    }
    else if (i == json_token->_internal.end_buffer_index)
    {
      source = az_span_slice(source, 0, json_token->_internal.end_buffer_offset);
    }

    if (!_az_json_token_is_text_equal_helper(source, &expected_text, &next_char_escaped)
        && az_span_size(expected_text) == 0)
    {
      return false;
    }
  }

  // Only return true if we have gone through and compared the entire expected_text.
  return az_span_size(expected_text) == 0;
}

AZ_NODISCARD az_result az_json_token_get_boolean(az_json_token const* json_token, bool* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_TRUE && json_token->kind != AZ_JSON_TOKEN_FALSE)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  // We assume the az_json_token is well-formed and self-consistent when returned from the
  // az_json_reader and that if json_token->kind == AZ_JSON_TOKEN_TRUE, then the slice contains the
  // characters "true", otherwise it contains "false". Therefore, there is no need to check the
  // contents again.

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    *out_value = az_span_size(token_slice) == _az_STRING_LITERAL_LEN("true");
  }
  else
  {
    // Token straddles more than one segment
    *out_value = json_token->size == _az_STRING_LITERAL_LEN("true");
  }

  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_token_get_string_helper(
    az_span source,
    char* destination,
    int32_t destination_max_size,
    int32_t* dest_idx,
    bool* next_char_escaped)
{
  int32_t source_size = az_span_size(source);
  uint8_t* source_ptr = az_span_ptr(source);
  for (int32_t i = 0; i < source_size; i++)
  {
    if (*dest_idx >= destination_max_size)
    {
      return AZ_ERROR_NOT_ENOUGH_SPACE;
    }
    uint8_t token_byte = source_ptr[i];

    if (token_byte == '\\' || *next_char_escaped)
    {
      if (*next_char_escaped)
      {
        token_byte = _az_json_unescape_single_byte(token_byte);
      }
      else
      {
        i++;
        if (i >= source_size)
        {
          *next_char_escaped = true;
          break;
        }
        token_byte = _az_json_unescape_single_byte(source_ptr[i]);
      }
      *next_char_escaped = false;

      // TODO: Characters escaped in the form of \uXXXX where XXXX is the UTF-16 code point, is
      // not currently supported.
      // To do this, we need to encode UTF-16 codepoints (including surrogate pairs) into UTF-8.
      if (token_byte == 'u')
      {
        return AZ_ERROR_NOT_IMPLEMENTED;
      }
    }

    destination[*dest_idx] = (char)token_byte;
    *dest_idx = *dest_idx + 1;
  }

  return AZ_OK;
}

AZ_NODISCARD az_span az_json_string_unescape(az_span json_string, az_span destination)
{
  _az_PRECONDITION_VALID_SPAN(json_string, 1, false);

  int32_t span_size = az_span_size(json_string);

  // The destination needs to be at least as large as the input, in the worst case.
  _az_PRECONDITION_VALID_SPAN(destination, span_size, false);

  int32_t position = 0;
  uint8_t* span_ptr = az_span_ptr(json_string);
  uint8_t* destination_ptr = az_span_ptr(destination);
  int32_t destination_size = az_span_size(destination);
  for (int32_t i = 0; i < span_size; i++)
  {
    uint8_t current_char = span_ptr[i];
    if (current_char == '\\' && i < span_size - 1)
    {
      uint8_t next_char = span_ptr[i + 1];
      // check that we have something to escape
      if (_az_is_valid_escaped_character(next_char))
      {
        current_char = _az_json_unescape_single_byte(next_char);
        i++;
      }
      else
      {
        // We assume that the input json is well-formed, but stop processing, in-case it isn't.
        return az_span_slice(destination, 0, position);
      }
    }
    else if (current_char == '\\')
    {
      // At this point, we are at the last character, i == span_size - 1
      // We assume that the input json is well-formed, but stop processing, in-case it isn't.
      return az_span_slice(destination, 0, position);
    }

    if (position > destination_size)
    {
      // We assume that the destination buffer is large enough, but stop processing, in-case it
      // isn't.
      return az_span_slice(destination, 0, position);
    }

    destination_ptr[position] = current_char;
    position++;
  }

  return az_span_slice(destination, 0, position);
}

AZ_NODISCARD az_result az_json_token_get_string(
    az_json_token const* json_token,
    char* destination,
    int32_t destination_max_size,
    int32_t* out_string_length)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(destination);
  _az_PRECONDITION(destination_max_size > 0);

  if (json_token->kind != AZ_JSON_TOKEN_STRING && json_token->kind != AZ_JSON_TOKEN_PROPERTY_NAME)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;
  int32_t token_size = json_token->size;

  // There is nothing to unescape here, copy directly.
  if (!json_token->_internal.string_has_escaped_chars)
  {
    // We need enough space to add a null terminator.
    if (token_size >= destination_max_size)
    {
      return AZ_ERROR_NOT_ENOUGH_SPACE;
    }

    // Contiguous token
    if (!json_token->_internal.is_multisegment)
    {
      // This will add a null terminator.
      az_span_to_str(destination, destination_max_size, token_slice);
    }
    else
    {
      // Token straddles more than one segment
      az_span remainder = _az_json_token_copy_into_span_helper(
          json_token, az_span_create((uint8_t*)destination, destination_max_size));

      // Add a null terminator.
      az_span_copy_u8(remainder, 0);
    }

    if (out_string_length != NULL)
    {
      *out_string_length = token_size;
    }
    return AZ_OK;
  }

  // No need to try to unescape the token slice, if the destination is known to be too small.
  // Unescaping always shrinks the string, at most by a factor of 6.
  // We also need enough space to add a null terminator.
  if (token_size / _az_MAX_EXPANSION_FACTOR_WHILE_ESCAPING >= destination_max_size)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }

  int32_t dest_idx = 0;
  bool next_char_escaped = false;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    _az_RETURN_IF_FAILED(_az_json_token_get_string_helper(
        token_slice, destination, destination_max_size, &dest_idx, &next_char_escaped));
  }
  else
  {
    // Token straddles more than one segment
    for (int32_t i = json_token->_internal.start_buffer_index;
         i <= json_token->_internal.end_buffer_index;
         i++)
    {
      az_span source = json_token->_internal.pointer_to_first_buffer[i];
      if (i == json_token->_internal.start_buffer_index)
      {
        source = az_span_slice_to_end(source, json_token->_internal.start_buffer_offset);
      }
      else if (i == json_token->_internal.end_buffer_index)
      {
        source = az_span_slice(source, 0, json_token->_internal.end_buffer_offset);
      }

      _az_RETURN_IF_FAILED(_az_json_token_get_string_helper(
          source, destination, destination_max_size, &dest_idx, &next_char_escaped));
    }
  }

  if (dest_idx >= destination_max_size)
  {
    return AZ_ERROR_NOT_ENOUGH_SPACE;
  }
  destination[dest_idx] = 0;

  if (out_string_length != NULL)
  {
    *out_string_length = dest_idx;
  }

  return AZ_OK;
}

AZ_NODISCARD az_result
az_json_token_get_uint64(az_json_token const* json_token, uint64_t* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_NUMBER)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_atou64(token_slice, out_value);
  }

  // Any number that won't fit in the scratch buffer, will overflow.
  if (json_token->size > _az_MAX_SIZE_FOR_UINT64)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // Token straddles more than one segment.
  // Used to copy discontiguous token values into a contiguous buffer, for number parsing.
  uint8_t scratch_buffer[_az_MAX_SIZE_FOR_UINT64] = { 0 };
  az_span scratch = AZ_SPAN_FROM_BUFFER(scratch_buffer);

  az_span remainder = _az_json_token_copy_into_span_helper(json_token, scratch);

  return az_span_atou64(az_span_slice(scratch, 0, _az_span_diff(remainder, scratch)), out_value);
}

AZ_NODISCARD az_result
az_json_token_get_uint32(az_json_token const* json_token, uint32_t* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_NUMBER)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_atou32(token_slice, out_value);
  }

  // Any number that won't fit in the scratch buffer, will overflow.
  if (json_token->size > _az_MAX_SIZE_FOR_UINT32)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // Token straddles more than one segment.
  // Used to copy discontiguous token values into a contiguous buffer, for number parsing.
  uint8_t scratch_buffer[_az_MAX_SIZE_FOR_UINT32] = { 0 };
  az_span scratch = AZ_SPAN_FROM_BUFFER(scratch_buffer);

  az_span remainder = _az_json_token_copy_into_span_helper(json_token, scratch);

  return az_span_atou32(az_span_slice(scratch, 0, _az_span_diff(remainder, scratch)), out_value);
}

AZ_NODISCARD az_result az_json_token_get_int64(az_json_token const* json_token, int64_t* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_NUMBER)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_atoi64(token_slice, out_value);
  }

  // Any number that won't fit in the scratch buffer, will overflow.
  if (json_token->size > _az_MAX_SIZE_FOR_INT64)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // Token straddles more than one segment.
  // Used to copy discontiguous token values into a contiguous buffer, for number parsing.
  uint8_t scratch_buffer[_az_MAX_SIZE_FOR_INT64] = { 0 };
  az_span scratch = AZ_SPAN_FROM_BUFFER(scratch_buffer);

  az_span remainder = _az_json_token_copy_into_span_helper(json_token, scratch);

  return az_span_atoi64(az_span_slice(scratch, 0, _az_span_diff(remainder, scratch)), out_value);
}

AZ_NODISCARD az_result az_json_token_get_int32(az_json_token const* json_token, int32_t* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_NUMBER)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_atoi32(token_slice, out_value);
  }

  // Any number that won't fit in the scratch buffer, will overflow.
  if (json_token->size > _az_MAX_SIZE_FOR_INT32)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // Token straddles more than one segment.
  // Used to copy discontiguous token values into a contiguous buffer, for number parsing.
  uint8_t scratch_buffer[_az_MAX_SIZE_FOR_INT32] = { 0 };
  az_span scratch = AZ_SPAN_FROM_BUFFER(scratch_buffer);

  az_span remainder = _az_json_token_copy_into_span_helper(json_token, scratch);

  return az_span_atoi32(az_span_slice(scratch, 0, _az_span_diff(remainder, scratch)), out_value);
}

AZ_NODISCARD az_result az_json_token_get_double(az_json_token const* json_token, double* out_value)
{
  _az_PRECONDITION_NOT_NULL(json_token);
  _az_PRECONDITION_NOT_NULL(out_value);

  if (json_token->kind != AZ_JSON_TOKEN_NUMBER)
  {
    return AZ_ERROR_JSON_INVALID_STATE;
  }

  az_span token_slice = json_token->slice;

  // Contiguous token
  if (!json_token->_internal.is_multisegment)
  {
    return az_span_atod(token_slice, out_value);
  }

  // Any number that won't fit in the scratch buffer, will overflow.
  if (json_token->size > _az_MAX_SIZE_FOR_PARSING_DOUBLE)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // Token straddles more than one segment.
  // Used to copy discontiguous token values into a contiguous buffer, for number parsing.
  uint8_t scratch_buffer[_az_MAX_SIZE_FOR_PARSING_DOUBLE] = { 0 };
  az_span scratch = AZ_SPAN_FROM_BUFFER(scratch_buffer);

  az_span remainder = _az_json_token_copy_into_span_helper(json_token, scratch);

  return az_span_atod(az_span_slice(scratch, 0, _az_span_diff(remainder, scratch)), out_value);
}
