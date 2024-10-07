// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_json_private.h"
#include "az_span_private.h"
#include <azure/core/az_precondition.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <ctype.h>

#include <azure/core/_az_cfg.h>

AZ_NODISCARD az_result az_json_reader_init(
    az_json_reader* out_json_reader,
    az_span json_buffer,
    az_json_reader_options const* options)
{
  _az_PRECONDITION(az_span_size(json_buffer) >= 1);

  *out_json_reader = (az_json_reader){
    .token = (az_json_token){
      .kind = AZ_JSON_TOKEN_NONE,
      .slice = AZ_SPAN_EMPTY,
      .size = 0,
      ._internal = {
        .is_multisegment = false,
        .string_has_escaped_chars = false,
        .pointer_to_first_buffer = &AZ_SPAN_EMPTY,
        .start_buffer_index = -1,
        .start_buffer_offset = -1,
        .end_buffer_index = -1,
        .end_buffer_offset = -1,
      },
    },
    .current_depth = 0,
    ._internal = {
      .json_buffer = json_buffer,
      .json_buffers = &AZ_SPAN_EMPTY,
      .number_of_buffers = 1,
      .buffer_index = 0,
      .bytes_consumed = 0,
      .total_bytes_consumed = 0,
      .is_complex_json = false,
      .bit_stack = { 0 },
      .options = options == NULL ? az_json_reader_options_default() : *options,
    },
  };
  return AZ_OK;
}

AZ_NODISCARD az_result az_json_reader_chunked_init(
    az_json_reader* out_json_reader,
    az_span json_buffers[],
    int32_t number_of_buffers,
    az_json_reader_options const* options)
{
  _az_PRECONDITION(number_of_buffers >= 1);
  _az_PRECONDITION(az_span_size(json_buffers[0]) >= 1);

  *out_json_reader = (az_json_reader)
  {
    .token = (az_json_token){
      .kind = AZ_JSON_TOKEN_NONE,
      .slice = AZ_SPAN_EMPTY,
      .size = 0,
      ._internal = {
        .is_multisegment = false,
        .string_has_escaped_chars = false,
        .pointer_to_first_buffer = json_buffers,
        .start_buffer_index = -1,
        .start_buffer_offset = -1,
        .end_buffer_index = -1,
        .end_buffer_offset = -1,
      },
    },
    .current_depth = 0,
    ._internal = {
      .json_buffer = json_buffers[0],
      .json_buffers = json_buffers,
      .number_of_buffers = number_of_buffers,
      .buffer_index = 0,
      .bytes_consumed = 0,
      .total_bytes_consumed = 0,
      .is_complex_json = false,
      .bit_stack = { 0 },
      .options = options == NULL ? az_json_reader_options_default() : *options,
    },
  };
  return AZ_OK;
}

AZ_NODISCARD static az_span _get_remaining_json(az_json_reader* json_reader)
{
  _az_PRECONDITION_NOT_NULL(json_reader);

  return az_span_slice_to_end(
      json_reader->_internal.json_buffer, json_reader->_internal.bytes_consumed);
}

static void _az_json_reader_update_state(
    az_json_reader* ref_json_reader,
    az_json_token_kind token_kind,
    az_span token_slice,
    int32_t current_segment_consumed,
    int32_t consumed)
{
  ref_json_reader->token.kind = token_kind;
  ref_json_reader->token.size = consumed;
  ref_json_reader->current_depth = ref_json_reader->_internal.bit_stack._internal.current_depth;

  // The depth of the start of the container will be one less than the bit stack managing the state.
  // That is because we push on the stack when we see a start of the container (above in the call
  // stack), but its actual depth and "indentation" level is one lower.
  if (token_kind == AZ_JSON_TOKEN_BEGIN_ARRAY || token_kind == AZ_JSON_TOKEN_BEGIN_OBJECT)
  {
    ref_json_reader->current_depth--;
  }

  ref_json_reader->_internal.bytes_consumed += current_segment_consumed;
  ref_json_reader->_internal.total_bytes_consumed += consumed;

  // We should have already set start_buffer_index and offset before moving to the next buffer.
  ref_json_reader->token._internal.end_buffer_index = ref_json_reader->_internal.buffer_index;
  ref_json_reader->token._internal.end_buffer_offset = ref_json_reader->_internal.bytes_consumed;

  ref_json_reader->token._internal.is_multisegment = false;

  // Token straddles more than one segment
  int32_t start_index = ref_json_reader->token._internal.start_buffer_index;
  if (start_index != -1 && start_index < ref_json_reader->token._internal.end_buffer_index)
  {
    ref_json_reader->token._internal.is_multisegment = true;
  }

  ref_json_reader->token.slice = token_slice;
}

AZ_NODISCARD static az_result _az_json_reader_get_next_buffer(
    az_json_reader* ref_json_reader,
    az_span* remaining,
    bool skip_whitespace)
{
  // If we only had one buffer, or we ran out of the set of discontiguous buffers, return error.
  if (ref_json_reader->_internal.buffer_index >= ref_json_reader->_internal.number_of_buffers - 1)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }

  if (!skip_whitespace && ref_json_reader->token._internal.start_buffer_index == -1)
  {
    ref_json_reader->token._internal.start_buffer_index = ref_json_reader->_internal.buffer_index;

    ref_json_reader->token._internal.start_buffer_offset
        = ref_json_reader->_internal.bytes_consumed;
  }

  ref_json_reader->_internal.buffer_index++;

  ref_json_reader->_internal.json_buffer
      = ref_json_reader->_internal.json_buffers[ref_json_reader->_internal.buffer_index];

  ref_json_reader->_internal.bytes_consumed = 0;

  az_span place_holder = _get_remaining_json(ref_json_reader);

  // Found an empty segment in the json_buffers array, which isn't allowed.
  if (az_span_size(place_holder) < 1)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }

  *remaining = place_holder;
  return AZ_OK;
}

AZ_NODISCARD static az_span _az_json_reader_skip_whitespace(az_json_reader* ref_json_reader)
{
  az_span json;
  az_span remaining = _get_remaining_json(ref_json_reader);

  while (true)
  {
    json = _az_span_trim_whitespace_from_start(remaining);

    // Find out how many whitespace characters were trimmed.
    int32_t consumed = _az_span_diff(json, remaining);

    ref_json_reader->_internal.bytes_consumed += consumed;
    ref_json_reader->_internal.total_bytes_consumed += consumed;

    if (az_span_size(json) >= 1
        || az_result_failed(_az_json_reader_get_next_buffer(ref_json_reader, &remaining, true)))
    {
      break;
    }
  }

  return json;
}

AZ_NODISCARD static az_result _az_json_reader_process_container_end(
    az_json_reader* ref_json_reader,
    az_json_token_kind token_kind)
{
  // The JSON payload is invalid if it has a mismatched container end without a matching open.
  if ((token_kind == AZ_JSON_TOKEN_END_OBJECT
       && _az_json_stack_peek(&ref_json_reader->_internal.bit_stack) != _az_JSON_STACK_OBJECT)
      || (token_kind == AZ_JSON_TOKEN_END_ARRAY
          && _az_json_stack_peek(&ref_json_reader->_internal.bit_stack) != _az_JSON_STACK_ARRAY))
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  az_span token = _get_remaining_json(ref_json_reader);
  _az_json_stack_pop(&ref_json_reader->_internal.bit_stack);
  _az_json_reader_update_state(ref_json_reader, token_kind, az_span_slice(token, 0, 1), 1, 1);
  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_reader_process_container_start(
    az_json_reader* ref_json_reader,
    az_json_token_kind token_kind,
    _az_json_stack_item container_kind)
{
  // The current depth is equal to or larger than the maximum allowed depth of 64. Cannot read the
  // next JSON object or array.
  if (ref_json_reader->_internal.bit_stack._internal.current_depth >= _az_MAX_JSON_STACK_SIZE)
  {
    return AZ_ERROR_JSON_NESTING_OVERFLOW;
  }

  az_span token = _get_remaining_json(ref_json_reader);

  _az_json_stack_push(&ref_json_reader->_internal.bit_stack, container_kind);
  _az_json_reader_update_state(ref_json_reader, token_kind, az_span_slice(token, 0, 1), 1, 1);
  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_reader_process_string(az_json_reader* ref_json_reader)
{
  // Move past the first '"' character
  ref_json_reader->_internal.bytes_consumed++;

  az_span token = _get_remaining_json(ref_json_reader);
  int32_t remaining_size = az_span_size(token);

  if (remaining_size < 1)
  {
    _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
    remaining_size = az_span_size(token);
  }

  int32_t current_index = 0;
  int32_t string_length = 0;
  uint8_t* token_ptr = az_span_ptr(token);
  uint8_t next_byte = token_ptr[0];

  // Clear the state of any previous string token.
  ref_json_reader->token._internal.string_has_escaped_chars = false;

  while (true)
  {
    if (next_byte == '"')
    {
      break;
    }

    if (next_byte == '\\')
    {
      ref_json_reader->token._internal.string_has_escaped_chars = true;
      current_index++;
      string_length++;
      if (current_index >= remaining_size)
      {
        _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
        current_index = 0;
        token_ptr = az_span_ptr(token);
        remaining_size = az_span_size(token);
      }
      next_byte = token_ptr[current_index];

      if (next_byte == 'u')
      {
        current_index++;
        string_length++;

        // Expecting 4 hex digits to follow the escaped 'u'
        for (int32_t i = 0; i < 4; i++)
        {
          if (current_index >= remaining_size)
          {
            _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
            current_index = 0;
            token_ptr = az_span_ptr(token);
            remaining_size = az_span_size(token);
          }

          string_length++;
          next_byte = token_ptr[current_index++];

          if (!isxdigit(next_byte))
          {
            return AZ_ERROR_UNEXPECTED_CHAR;
          }
        }

        // We have already skipped past the u and 4 hex digits. The loop accounts for incrementing
        // by 1 more, so subtract one to account for that.
        current_index--;
        string_length--;
      }
      else
      {
        if (!_az_is_valid_escaped_character(next_byte))
        {
          return AZ_ERROR_UNEXPECTED_CHAR;
        }
      }
    }
    else
    {
      // Control characters are invalid within a JSON string and should be correctly escaped.
      if (next_byte < _az_ASCII_SPACE_CHARACTER)
      {
        return AZ_ERROR_UNEXPECTED_CHAR;
      }
    }

    current_index++;
    string_length++;

    if (current_index >= remaining_size)
    {
      _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
      current_index = 0;
      token_ptr = az_span_ptr(token);
      remaining_size = az_span_size(token);
    }
    next_byte = token_ptr[current_index];
  }

  _az_json_reader_update_state(
      ref_json_reader,
      AZ_JSON_TOKEN_STRING,
      az_span_slice(token, 0, current_index),
      current_index,
      string_length);

  // Add 1 to number of bytes consumed to account for the last '"' character.
  ref_json_reader->_internal.bytes_consumed++;
  ref_json_reader->_internal.total_bytes_consumed++;

  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_reader_process_property_name(az_json_reader* ref_json_reader)
{
  _az_RETURN_IF_FAILED(_az_json_reader_process_string(ref_json_reader));

  az_span json = _az_json_reader_skip_whitespace(ref_json_reader);

  // Expected a colon to indicate that a value will follow after the property name, but instead
  // either reached end of data or some other character, which is invalid.
  if (az_span_size(json) < 1)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }
  if (az_span_ptr(json)[0] != ':')
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  // We don't need to set the json_reader->token.slice since that was already done
  // in _az_json_reader_process_string when processing the string portion of the property name.
  // Therefore, we don't call _az_json_reader_update_state here.
  ref_json_reader->token.kind = AZ_JSON_TOKEN_PROPERTY_NAME;
  ref_json_reader->_internal.bytes_consumed++; // For the name / value separator
  ref_json_reader->_internal.total_bytes_consumed++; // For the name / value separator

  return AZ_OK;
}

// Used to search for possible valid end of a number character, when we have complex JSON payloads
// (i.e. not a single JSON value).
// Whitespace characters, comma, or a container end character indicate the end of a JSON number.
static const az_span json_delimiters = AZ_SPAN_LITERAL_FROM_STR(",}] \n\r\t");

AZ_NODISCARD static bool _az_finished_consuming_json_number(
    uint8_t next_byte,
    az_span expected_next_bytes,
    az_result* out_result)
{
  az_span next_byte_span = az_span_create(&next_byte, 1);

  // Checking if we are done processing a JSON number
  int32_t index = az_span_find(json_delimiters, next_byte_span);
  if (index != -1)
  {
    *out_result = AZ_OK;
    return true;
  }

  // The next character after a "0" or a set of digits must either be a decimal or 'e'/'E' to
  // indicate scientific notation. For example "01" or "123f" is invalid.
  // The next character after "[-][digits].[digits]" must be 'e'/'E' if we haven't reached the end
  // of the number yet. For example, "1.1f" or "1.1-" are invalid.
  index = az_span_find(expected_next_bytes, next_byte_span);
  if (index == -1)
  {
    *out_result = AZ_ERROR_UNEXPECTED_CHAR;
    return true;
  }

  return false;
}

static void _az_json_reader_consume_digits(
    az_json_reader* ref_json_reader,
    az_span* token,
    int32_t* current_consumed,
    int32_t* total_consumed)
{
  int32_t counter = 0;
  az_span current = az_span_slice_to_end(*token, *current_consumed);
  while (true)
  {
    int32_t const token_size = az_span_size(current);
    uint8_t* next_byte_ptr = az_span_ptr(current);

    while (counter < token_size)
    {
      if (isdigit(*next_byte_ptr))
      {
        counter++;
        next_byte_ptr++;
      }
      else
      {
        break;
      }
    }
    if (counter == token_size
        && az_result_succeeded(_az_json_reader_get_next_buffer(ref_json_reader, token, false)))
    {
      *total_consumed += counter;
      counter = 0;
      *current_consumed = 0;
      current = *token;
      continue;
    }
    break;
  }

  *total_consumed += counter;
  *current_consumed += counter;
}

AZ_NODISCARD static az_result _az_json_reader_update_number_state_if_single_value(
    az_json_reader* ref_json_reader,
    az_span token_slice,
    int32_t current_consumed,
    int32_t total_consumed)
{
  if (ref_json_reader->_internal.is_complex_json)
  {
    return AZ_ERROR_UNEXPECTED_END;
  }

  _az_json_reader_update_state(
      ref_json_reader, AZ_JSON_TOKEN_NUMBER, token_slice, current_consumed, total_consumed);

  return AZ_OK;
}

AZ_NODISCARD static az_result _az_validate_next_byte_is_digit(
    az_json_reader* ref_json_reader,
    az_span* remaining_number,
    int32_t* current_consumed)
{
  az_span current = az_span_slice_to_end(*remaining_number, *current_consumed);
  if (az_span_size(current) < 1)
  {
    _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, remaining_number, false));
    current = *remaining_number;
    *current_consumed = 0;
  }

  if (!isdigit(az_span_ptr(current)[0]))
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_reader_process_number(az_json_reader* ref_json_reader)
{
  az_span token = _get_remaining_json(ref_json_reader);

  int32_t total_consumed = 0;
  int32_t current_consumed = 0;

  uint8_t next_byte = az_span_ptr(token)[0];
  if (next_byte == '-')
  {
    total_consumed++;
    current_consumed++;

    // A negative sign must be followed by at least one digit.
    _az_RETURN_IF_FAILED(
        _az_validate_next_byte_is_digit(ref_json_reader, &token, &current_consumed));

    next_byte = az_span_ptr(token)[current_consumed];
  }

  if (next_byte == '0')
  {
    total_consumed++;
    current_consumed++;

    if (current_consumed >= az_span_size(token))
    {
      if (az_result_failed(_az_json_reader_get_next_buffer(ref_json_reader, &token, false)))
      {
        // If there is no more JSON, this is a valid end state only when the JSON payload contains a
        // single value: "[-]0"
        // Otherwise, the payload is incomplete and ending too early.
        return _az_json_reader_update_number_state_if_single_value(
            ref_json_reader,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      current_consumed = 0;
    }

    next_byte = az_span_ptr(token)[current_consumed];
    az_result result = AZ_OK;
    if (_az_finished_consuming_json_number(next_byte, AZ_SPAN_FROM_STR(".eE"), &result))
    {
      if (az_result_succeeded(result))
      {
        _az_json_reader_update_state(
            ref_json_reader,
            AZ_JSON_TOKEN_NUMBER,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      return result;
    }
  }
  else
  {
    _az_PRECONDITION(isdigit(next_byte));

    // Integer part before decimal
    _az_json_reader_consume_digits(ref_json_reader, &token, &current_consumed, &total_consumed);

    if (current_consumed >= az_span_size(token))
    {
      if (az_result_failed(_az_json_reader_get_next_buffer(ref_json_reader, &token, false)))
      {
        // If there is no more JSON, this is a valid end state only when the JSON payload contains a
        // single value: "[-][digits]"
        // Otherwise, the payload is incomplete and ending too early.
        return _az_json_reader_update_number_state_if_single_value(
            ref_json_reader,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      current_consumed = 0;
    }

    next_byte = az_span_ptr(token)[current_consumed];
    az_result result = AZ_OK;
    if (_az_finished_consuming_json_number(next_byte, AZ_SPAN_FROM_STR(".eE"), &result))
    {
      if (az_result_succeeded(result))
      {
        _az_json_reader_update_state(
            ref_json_reader,
            AZ_JSON_TOKEN_NUMBER,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      return result;
    }
  }

  if (next_byte == '.')
  {
    total_consumed++;
    current_consumed++;

    // A decimal point must be followed by at least one digit.
    _az_RETURN_IF_FAILED(
        _az_validate_next_byte_is_digit(ref_json_reader, &token, &current_consumed));

    // Integer part after decimal
    _az_json_reader_consume_digits(ref_json_reader, &token, &current_consumed, &total_consumed);

    if (current_consumed >= az_span_size(token))
    {
      if (az_result_failed(_az_json_reader_get_next_buffer(ref_json_reader, &token, false)))
      {
        // If there is no more JSON, this is a valid end state only when the JSON payload contains a
        // single value: "[-][digits].[digits]"
        // Otherwise, the payload is incomplete and ending too early.
        return _az_json_reader_update_number_state_if_single_value(
            ref_json_reader,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      current_consumed = 0;
    }

    next_byte = az_span_ptr(token)[current_consumed];
    az_result result = AZ_OK;
    if (_az_finished_consuming_json_number(next_byte, AZ_SPAN_FROM_STR("eE"), &result))
    {
      if (az_result_succeeded(result))
      {
        _az_json_reader_update_state(
            ref_json_reader,
            AZ_JSON_TOKEN_NUMBER,
            az_span_slice(token, 0, current_consumed),
            current_consumed,
            total_consumed);
      }
      return result;
    }
  }

  // Move past 'e'/'E'
  total_consumed++;
  current_consumed++;

  // The 'e'/'E' character must be followed by a sign or at least one digit.
  if (current_consumed >= az_span_size(token))
  {
    _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
    current_consumed = 0;
  }

  next_byte = az_span_ptr(token)[current_consumed];
  if (next_byte == '-' || next_byte == '+')
  {
    total_consumed++;
    current_consumed++;

    // A sign must be followed by at least one digit.
    _az_RETURN_IF_FAILED(
        _az_validate_next_byte_is_digit(ref_json_reader, &token, &current_consumed));
  }

  // Integer part after the 'e'/'E'
  _az_json_reader_consume_digits(ref_json_reader, &token, &current_consumed, &total_consumed);

  if (current_consumed >= az_span_size(token))
  {
    if (az_result_failed(_az_json_reader_get_next_buffer(ref_json_reader, &token, false)))
    {

      // If there is no more JSON, this is a valid end state only when the JSON payload contains a
      // single value: "[-][digits].[digits]e[+|-][digits]"
      // Otherwise, the payload is incomplete and ending too early.
      return _az_json_reader_update_number_state_if_single_value(
          ref_json_reader,
          az_span_slice(token, 0, current_consumed),
          current_consumed,
          total_consumed);
    }
    current_consumed = 0;
  }

  // Checking if we are done processing a JSON number
  next_byte = az_span_ptr(token)[current_consumed];
  int32_t index = az_span_find(json_delimiters, az_span_create(&next_byte, 1));
  if (index == -1)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  _az_json_reader_update_state(
      ref_json_reader,
      AZ_JSON_TOKEN_NUMBER,
      az_span_slice(token, 0, current_consumed),
      current_consumed,
      total_consumed);

  return AZ_OK;
}

AZ_INLINE int32_t _az_min(int32_t a, int32_t b) { return a < b ? a : b; }

AZ_NODISCARD static az_result _az_json_reader_process_literal(
    az_json_reader* ref_json_reader,
    az_span literal,
    az_json_token_kind kind)
{
  az_span token = _get_remaining_json(ref_json_reader);

  int32_t const expected_literal_size = az_span_size(literal);

  int32_t already_matched = 0;

  int32_t max_comparable_size = 0;
  while (true)
  {
    int32_t token_size = az_span_size(token);
    max_comparable_size = _az_min(token_size, expected_literal_size - already_matched);

    token = az_span_slice(token, 0, max_comparable_size);

    // Return if the subslice that can be compared contains a mismatch.
    if (!az_span_is_content_equal(
            token, az_span_slice(literal, already_matched, already_matched + max_comparable_size)))
    {
      return AZ_ERROR_UNEXPECTED_CHAR;
    }
    already_matched += max_comparable_size;

    if (already_matched == expected_literal_size)
    {
      break;
    }

    // If there is no more data, return EOF because the token is smaller than the expected literal.
    _az_RETURN_IF_FAILED(_az_json_reader_get_next_buffer(ref_json_reader, &token, false));
  }

  _az_json_reader_update_state(
      ref_json_reader, kind, token, max_comparable_size, expected_literal_size);
  return AZ_OK;
}

AZ_NODISCARD static az_result _az_json_reader_process_value(
    az_json_reader* ref_json_reader,
    uint8_t const next_byte)
{
  if (next_byte == '"')
  {
    return _az_json_reader_process_string(ref_json_reader);
  }

  if (next_byte == '{')
  {
    return _az_json_reader_process_container_start(
        ref_json_reader, AZ_JSON_TOKEN_BEGIN_OBJECT, _az_JSON_STACK_OBJECT);
  }

  if (next_byte == '[')
  {
    return _az_json_reader_process_container_start(
        ref_json_reader, AZ_JSON_TOKEN_BEGIN_ARRAY, _az_JSON_STACK_ARRAY);
  }

  if (isdigit(next_byte) || next_byte == '-')
  {
    return _az_json_reader_process_number(ref_json_reader);
  }

  if (next_byte == 'f')
  {
    return _az_json_reader_process_literal(
        ref_json_reader, AZ_SPAN_FROM_STR("false"), AZ_JSON_TOKEN_FALSE);
  }

  if (next_byte == 't')
  {
    return _az_json_reader_process_literal(
        ref_json_reader, AZ_SPAN_FROM_STR("true"), AZ_JSON_TOKEN_TRUE);
  }

  if (next_byte == 'n')
  {
    return _az_json_reader_process_literal(
        ref_json_reader, AZ_SPAN_FROM_STR("null"), AZ_JSON_TOKEN_NULL);
  }

  return AZ_ERROR_UNEXPECTED_CHAR;
}

AZ_NODISCARD static az_result _az_json_reader_read_first_token(
    az_json_reader* ref_json_reader,
    az_span json,
    uint8_t const first_byte)
{
  if (first_byte == '{')
  {
    _az_json_stack_push(&ref_json_reader->_internal.bit_stack, _az_JSON_STACK_OBJECT);

    _az_json_reader_update_state(
        ref_json_reader, AZ_JSON_TOKEN_BEGIN_OBJECT, az_span_slice(json, 0, 1), 1, 1);

    ref_json_reader->_internal.is_complex_json = true;
    return AZ_OK;
  }

  if (first_byte == '[')
  {
    _az_json_stack_push(&ref_json_reader->_internal.bit_stack, _az_JSON_STACK_ARRAY);

    _az_json_reader_update_state(
        ref_json_reader, AZ_JSON_TOKEN_BEGIN_ARRAY, az_span_slice(json, 0, 1), 1, 1);

    ref_json_reader->_internal.is_complex_json = true;
    return AZ_OK;
  }

  return _az_json_reader_process_value(ref_json_reader, first_byte);
}

AZ_NODISCARD static az_result _az_json_reader_process_next_byte(
    az_json_reader* ref_json_reader,
    uint8_t next_byte)
{
  // Extra data after a single JSON value (complete object or array or one primitive value) is
  // invalid. Expected end of data.
  if (ref_json_reader->_internal.bit_stack._internal.current_depth == 0)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  bool within_object
      = _az_json_stack_peek(&ref_json_reader->_internal.bit_stack) == _az_JSON_STACK_OBJECT;

  if (next_byte == ',')
  {
    ref_json_reader->_internal.bytes_consumed++;

    az_span json = _az_json_reader_skip_whitespace(ref_json_reader);

    // Expected start of a property name or value, but instead reached end of data.
    if (az_span_size(json) < 1)
    {
      return AZ_ERROR_UNEXPECTED_END;
    }

    next_byte = az_span_ptr(json)[0];

    if (within_object)
    {
      // Expected start of a property name after the comma since we are within a JSON object.
      if (next_byte != '"')
      {
        return AZ_ERROR_UNEXPECTED_CHAR;
      }
      return _az_json_reader_process_property_name(ref_json_reader);
    }

    return _az_json_reader_process_value(ref_json_reader, next_byte);
  }

  if (next_byte == '}')
  {
    return _az_json_reader_process_container_end(ref_json_reader, AZ_JSON_TOKEN_END_OBJECT);
  }

  if (next_byte == ']')
  {
    return _az_json_reader_process_container_end(ref_json_reader, AZ_JSON_TOKEN_END_ARRAY);
  }

  // No other character is a valid token delimiter within JSON.
  return AZ_ERROR_UNEXPECTED_CHAR;
}

AZ_NODISCARD az_result az_json_reader_next_token(az_json_reader* ref_json_reader)
{
  _az_PRECONDITION_NOT_NULL(ref_json_reader);

  az_span json = _az_json_reader_skip_whitespace(ref_json_reader);

  if (az_span_size(json) < 1)
  {
    if (ref_json_reader->token.kind == AZ_JSON_TOKEN_NONE
        || ref_json_reader->_internal.bit_stack._internal.current_depth != 0)
    {
      // An empty JSON payload is invalid.
      return AZ_ERROR_UNEXPECTED_END;
    }

    // No more JSON text left to process, we are done.
    return AZ_ERROR_JSON_READER_DONE;
  }

  // Clear the internal state of any previous token.
  ref_json_reader->token._internal.start_buffer_index = -1;
  ref_json_reader->token._internal.start_buffer_offset = -1;
  ref_json_reader->token._internal.end_buffer_index = -1;
  ref_json_reader->token._internal.end_buffer_offset = -1;

  uint8_t const first_byte = az_span_ptr(json)[0];

  switch (ref_json_reader->token.kind)
  {
    case AZ_JSON_TOKEN_NONE:
    {
      return _az_json_reader_read_first_token(ref_json_reader, json, first_byte);
    }
    case AZ_JSON_TOKEN_BEGIN_OBJECT:
    {
      if (first_byte == '}')
      {
        return _az_json_reader_process_container_end(ref_json_reader, AZ_JSON_TOKEN_END_OBJECT);
      }

      // We expect the start of a property name as the first non-whitespace character within a
      // JSON object.
      if (first_byte != '"')
      {
        return AZ_ERROR_UNEXPECTED_CHAR;
      }
      return _az_json_reader_process_property_name(ref_json_reader);
    }
    case AZ_JSON_TOKEN_BEGIN_ARRAY:
    {
      if (first_byte == ']')
      {
        return _az_json_reader_process_container_end(ref_json_reader, AZ_JSON_TOKEN_END_ARRAY);
      }

      return _az_json_reader_process_value(ref_json_reader, first_byte);
    }
    case AZ_JSON_TOKEN_PROPERTY_NAME:
      return _az_json_reader_process_value(ref_json_reader, first_byte);
    case AZ_JSON_TOKEN_END_OBJECT:
    case AZ_JSON_TOKEN_END_ARRAY:
    case AZ_JSON_TOKEN_STRING:
    case AZ_JSON_TOKEN_NUMBER:
    case AZ_JSON_TOKEN_TRUE:
    case AZ_JSON_TOKEN_FALSE:
    case AZ_JSON_TOKEN_NULL:
      return _az_json_reader_process_next_byte(ref_json_reader, first_byte);
    default:
      return AZ_ERROR_JSON_INVALID_STATE;
  }
}

AZ_NODISCARD az_result az_json_reader_skip_children(az_json_reader* ref_json_reader)
{
  _az_PRECONDITION_NOT_NULL(ref_json_reader);

  if (ref_json_reader->token.kind == AZ_JSON_TOKEN_PROPERTY_NAME)
  {
    _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
  }

  az_json_token_kind const token_kind = ref_json_reader->token.kind;
  if (token_kind == AZ_JSON_TOKEN_BEGIN_OBJECT || token_kind == AZ_JSON_TOKEN_BEGIN_ARRAY)
  {
    // Keep moving the reader until we come back to the same depth.
    int32_t const depth = ref_json_reader->_internal.bit_stack._internal.current_depth;
    do
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(ref_json_reader));
    } while (depth <= ref_json_reader->_internal.bit_stack._internal.current_depth);
  }
  return AZ_OK;
}
