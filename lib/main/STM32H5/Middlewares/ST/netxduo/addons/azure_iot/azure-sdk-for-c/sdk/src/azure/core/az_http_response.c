// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_http.h>

#include "az_http_header_validation_private.h"
#include "az_http_private.h"
#include "az_span_private.h"
#include <azure/core/az_precondition.h>
#include <azure/core/internal/az_precondition_internal.h>
#include <azure/core/internal/az_result_internal.h>

#include <azure/core/_az_cfg.h>
#include <ctype.h>

// HTTP Response utility functions

static AZ_NODISCARD bool _az_is_http_whitespace(uint8_t c)
{
  switch (c)
  {
    case ' ':
    case '\t':
      return true;
      ;
    default:
      return false;
  }
}

/* PRIVATE Function. parse next  */
static AZ_NODISCARD az_result _az_get_digit(az_span* ref_span, uint8_t* save_here)
{

  if (az_span_size(*ref_span) == 0)
  {
    return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
  }

  uint8_t c_ptr = az_span_ptr(*ref_span)[0];
  if (!isdigit(c_ptr))
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }
  //
  *save_here = (uint8_t)(c_ptr - '0');

  // move reader after the expected digit (means it was parsed as expected)
  *ref_span = az_span_slice_to_end(*ref_span, 1);

  return AZ_OK;
}

/**
 * Status line https://tools.ietf.org/html/rfc7230#section-3.1.2
 * HTTP-version SP status-code SP reason-phrase CRLF
 */
static AZ_NODISCARD az_result
_az_get_http_status_line(az_span* ref_span, az_http_response_status_line* out_status_line)
{

  // HTTP-version = HTTP-name "/" DIGIT "." DIGIT
  // https://tools.ietf.org/html/rfc7230#section-2.6
  az_span const start = AZ_SPAN_FROM_STR("HTTP/");
  az_span const dot = AZ_SPAN_FROM_STR(".");
  az_span const space = AZ_SPAN_FROM_STR(" ");

  // parse and move reader if success
  _az_RETURN_IF_FAILED(_az_is_expected_span(ref_span, start));
  _az_RETURN_IF_FAILED(_az_get_digit(ref_span, &out_status_line->major_version));
  _az_RETURN_IF_FAILED(_az_is_expected_span(ref_span, dot));
  _az_RETURN_IF_FAILED(_az_get_digit(ref_span, &out_status_line->minor_version));

  // SP = " "
  _az_RETURN_IF_FAILED(_az_is_expected_span(ref_span, space));

  // status-code = 3DIGIT
  {
    uint64_t code = 0;
    _az_RETURN_IF_FAILED(az_span_atou64(az_span_create(az_span_ptr(*ref_span), 3), &code));
    out_status_line->status_code = (az_http_status_code)code;
    // move reader
    *ref_span = az_span_slice_to_end(*ref_span, 3);
  }

  // SP
  _az_RETURN_IF_FAILED(_az_is_expected_span(ref_span, space));

  // get a pointer to read response until end of reason-phrase is found
  // reason-phrase = *(HTAB / SP / VCHAR / obs-text)
  // HTAB = "\t"
  // VCHAR or obs-text is %x21-FF,
  int32_t offset = 0;
  int32_t input_size = az_span_size(*ref_span);
  uint8_t const* const ptr = az_span_ptr(*ref_span);
  for (; offset < input_size; ++offset)
  {
    uint8_t next_byte = ptr[offset];
    if (next_byte == '\n')
    {
      break;
    }
  }
  if (offset == input_size)
  {
    return AZ_ERROR_ITEM_NOT_FOUND;
  }

  // save reason-phrase in status line now that we got the offset. Remove 1 last chars(\r)
  out_status_line->reason_phrase = az_span_slice(*ref_span, 0, offset - 1);
  // move position of reader after reason-phrase (parsed done)
  *ref_span = az_span_slice_to_end(*ref_span, offset + 1);
  // CR LF
  // _az_RETURN_IF_FAILED(_az_is_expected_span(response, AZ_SPAN_FROM_STR("\r\n")));

  return AZ_OK;
}

AZ_NODISCARD az_result az_http_response_get_status_line(
    az_http_response* ref_response,
    az_http_response_status_line* out_status_line)
{
  _az_PRECONDITION_NOT_NULL(ref_response);
  _az_PRECONDITION_NOT_NULL(out_status_line);

  // Restart parser to the beginning
  ref_response->_internal.parser.remaining = ref_response->_internal.http_response;

  // read an HTTP status line.
  _az_RETURN_IF_FAILED(
      _az_get_http_status_line(&ref_response->_internal.parser.remaining, out_status_line));

  // set state.kind of the next HTTP response value.
  ref_response->_internal.parser.next_kind = _az_HTTP_RESPONSE_KIND_HEADER;

  return AZ_OK;
}

AZ_NODISCARD az_result az_http_response_get_next_header(
    az_http_response* ref_response,
    az_span* out_name,
    az_span* out_value)
{
  _az_PRECONDITION_NOT_NULL(ref_response);
  _az_PRECONDITION_NOT_NULL(out_name);
  _az_PRECONDITION_NOT_NULL(out_value);

  az_span* reader = &ref_response->_internal.parser.remaining;
  {
    _az_http_response_kind const kind = ref_response->_internal.parser.next_kind;
    // if reader is expecting to read body (all headers were read), return
    // AZ_ERROR_HTTP_END_OF_HEADERS so we know we reach end of headers
    if (kind == _az_HTTP_RESPONSE_KIND_BODY)
    {
      return AZ_ERROR_HTTP_END_OF_HEADERS;
    }
    // Can't read a header if status line was not previously called,
    // User needs to call az_http_response_status_line() which would reset parser and set kind to
    // headers
    if (kind != _az_HTTP_RESPONSE_KIND_HEADER)
    {
      return AZ_ERROR_HTTP_INVALID_STATE;
    }
  }

  if (az_span_size(ref_response->_internal.parser.remaining) == 0)
  {
    // avoid reading address if span is size 0
    return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
  }

  // check if we are at the end of all headers to change state to Body.
  // We keep state to Headers if current char is not '\r' (there is another header)
  if (az_span_ptr(ref_response->_internal.parser.remaining)[0] == '\r')
  {
    _az_RETURN_IF_FAILED(_az_is_expected_span(reader, AZ_SPAN_FROM_STR("\r\n")));
    ref_response->_internal.parser.next_kind = _az_HTTP_RESPONSE_KIND_BODY;
    return AZ_ERROR_HTTP_END_OF_HEADERS;
  }

  // https://tools.ietf.org/html/rfc7230#section-3.2
  // header-field   = field-name ":" OWS field-value OWS
  // field-name     = token
  {
    // https://tools.ietf.org/html/rfc7230#section-3.2.6
    // token = 1*tchar
    // tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." / "^" /
    //         "_" / "`" / "|" / "~" / DIGIT / ALPHA;
    // any VCHAR,
    //    except delimiters
    int32_t field_name_length = 0;
    int32_t input_size = az_span_size(*reader);
    uint8_t const* const ptr = az_span_ptr(*reader);
    for (; field_name_length < input_size; ++field_name_length)
    {
      uint8_t next_byte = ptr[field_name_length];
      if (next_byte == ':')
      {
        break;
      }
      if (!az_http_valid_token[next_byte])
      {
        return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
      }
    }
    if (field_name_length == input_size)
    {
      return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
    }

    // form a header name. Reader is currently at char ':'
    *out_name = az_span_slice(*reader, 0, field_name_length);
    // update reader to next position after colon (add one)
    *reader = az_span_slice_to_end(*reader, field_name_length + 1);

    // Remove whitespace characters from header name
    // https://github.com/Azure/azure-sdk-for-c/issues/604
    *out_name = _az_span_trim_whitespace(*out_name);

    // OWS -> remove the optional whitespace characters before header value
    int32_t ows_len = 0;
    input_size = az_span_size(*reader);
    uint8_t const* const ptr_space = az_span_ptr(*reader);
    for (; ows_len < input_size; ++ows_len)
    {
      uint8_t next_byte = ptr_space[ows_len];
      if (next_byte != ' ' && next_byte != '\t')
      {
        break;
      }
    }
    if (ows_len == input_size)
    {
      return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
    }

    *reader = az_span_slice_to_end(*reader, ows_len);
  }
  // field-value    = *( field-content / obs-fold )
  // field-content  = field-vchar [ 1*( SP / HTAB ) field-vchar ]
  // field-vchar    = VCHAR / obs-text
  //
  // obs-fold       = CRLF 1*( SP / HTAB )
  //                ; obsolete line folding
  //                ; see Section 3.2.4
  //
  // Note: obs-fold is not implemented.
  {
    int32_t offset = 0;
    int32_t offset_value_end = offset;
    while (true)
    {
      uint8_t c = az_span_ptr(*reader)[offset];
      offset += 1;
      if (c == '\r')
      {
        break; // break as soon as end of value char is found
      }
      if (_az_is_http_whitespace(c))
      {
        continue; // whitespace or tab is accepted. It can be any number after value (OWS)
      }
      if (c < ' ')
      {
        return AZ_ERROR_HTTP_CORRUPT_RESPONSE_HEADER;
      }
      offset_value_end = offset; // increasing index only for valid chars,
    }
    *out_value = az_span_slice(*reader, 0, offset_value_end);
    // moving reader. It is currently after \r was found
    *reader = az_span_slice_to_end(*reader, offset);

    // Remove whitespace characters from value https://github.com/Azure/azure-sdk-for-c/issues/604
    *out_value = _az_span_trim_whitespace_from_end(*out_value);
  }

  _az_RETURN_IF_FAILED(_az_is_expected_span(reader, AZ_SPAN_FROM_STR("\n")));

  return AZ_OK;
}

AZ_NODISCARD az_result az_http_response_get_body(az_http_response* ref_response, az_span* out_body)
{
  _az_PRECONDITION_NOT_NULL(ref_response);
  _az_PRECONDITION_NOT_NULL(out_body);

  // Make sure get body works no matter where is the current parsing. Allow users to call get body
  // directly and ignore headers and status line
  _az_http_response_kind current_parsing_section = ref_response->_internal.parser.next_kind;
  if (current_parsing_section != _az_HTTP_RESPONSE_KIND_BODY)
  {
    if (current_parsing_section == _az_HTTP_RESPONSE_KIND_EOF
        || current_parsing_section == _az_HTTP_RESPONSE_KIND_STATUS_LINE)
    {
      // Reset parser and get status line
      az_http_response_status_line ignore = { 0 };
      _az_RETURN_IF_FAILED(az_http_response_get_status_line(ref_response, &ignore));
      // update current parsing section
      current_parsing_section = ref_response->_internal.parser.next_kind;
    }
    // parse any remaining header
    if (current_parsing_section == _az_HTTP_RESPONSE_KIND_HEADER)
    {
      // Parse and ignore all remaining headers
      for (az_span n = { 0 }, v = { 0 };
           az_result_succeeded(az_http_response_get_next_header(ref_response, &n, &v));)
      {
        // ignoring header
      }
    }
  }

  // take all the remaining content from reader as body
  *out_body = az_span_slice_to_end(ref_response->_internal.parser.remaining, 0);

  ref_response->_internal.parser.next_kind = _az_HTTP_RESPONSE_KIND_EOF;
  return AZ_OK;
}

void _az_http_response_reset(az_http_response* ref_response)
{
  // never fails, discard the result
  // init will set written to 0 and will use the same az_span. Internal parser's state is also
  // reset
  az_result result = az_http_response_init(ref_response, ref_response->_internal.http_response);
  (void)result;
}

// internal function to get az_http_response remainder
static az_span _az_http_response_get_remaining(az_http_response const* response)
{
  return az_span_slice_to_end(response->_internal.http_response, response->_internal.written);
}

AZ_NODISCARD az_result az_http_response_append(az_http_response* ref_response, az_span source)
{
  _az_PRECONDITION_NOT_NULL(ref_response);

  az_span remaining = _az_http_response_get_remaining(ref_response);
  int32_t write_size = az_span_size(source);
  _az_RETURN_IF_NOT_ENOUGH_SIZE(remaining, write_size);

  az_span_copy(remaining, source);
  ref_response->_internal.written += write_size;

  return AZ_OK;
}
