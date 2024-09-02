// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief This header defines the types and functions your application uses to read or write JSON
 * objects.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_JSON_H
#define _az_JSON_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Defines symbols for the various kinds of JSON tokens that make up any JSON text.
 */
typedef enum
{
  AZ_JSON_TOKEN_NONE, ///< There is no value (as distinct from #AZ_JSON_TOKEN_NULL).
  AZ_JSON_TOKEN_BEGIN_OBJECT, ///< The token kind is the start of a JSON object.
  AZ_JSON_TOKEN_END_OBJECT, ///< The token kind is the end of a JSON object.
  AZ_JSON_TOKEN_BEGIN_ARRAY, ///< The token kind is the start of a JSON array.
  AZ_JSON_TOKEN_END_ARRAY, ///< The token kind is the end of a JSON array.
  AZ_JSON_TOKEN_PROPERTY_NAME, ///< The token kind is a JSON property name.
  AZ_JSON_TOKEN_STRING, ///< The token kind is a JSON string.
  AZ_JSON_TOKEN_NUMBER, ///< The token kind is a JSON number.
  AZ_JSON_TOKEN_TRUE, ///< The token kind is the JSON literal `true`.
  AZ_JSON_TOKEN_FALSE, ///< The token kind is the JSON literal `false`.
  AZ_JSON_TOKEN_NULL, ///< The token kind is the JSON literal `null`.
} az_json_token_kind;

/**
 * @brief A limited stack used by the #az_json_writer and #az_json_reader to track state information
 * for processing and validation.
 */
typedef struct
{
  struct
  {
    // This uint64_t container represents a tiny stack to track the state during nested transitions.
    // The first bit represents the state of the current depth (1 == object, 0 == array).
    // Each subsequent bit is the parent / containing type (object or array).
    uint64_t az_json_stack;
    int32_t current_depth;
  } _internal;
} _az_json_bit_stack;

/**
 * @brief Represents a JSON token. The kind field indicates the type of the JSON token and the slice
 * represents the portion of the JSON payload that points to the token value.
 *
 * @remarks An instance of #az_json_token must not outlive the lifetime of the #az_json_reader it
 * came from.
 */
typedef struct
{
  /// This read-only field gives access to the slice of the JSON text that represents the token
  /// value, and it shouldn't be modified by the caller.
  /// If the token straddles non-contiguous buffers, this is set to the partial token value
  /// available in the last segment.
  /// The user can call #az_json_token_copy_into_span() to get the token value into a contiguous
  /// buffer.
  /// In the case of JSON strings, the slice does not include the surrounding quotes.
  az_span slice;

  // Avoid using enum as the first field within structs, to allow for { 0 } initialization.
  // This is a workaround for IAR compiler warning [Pe188]: enumerated type mixed with another type.

  /// This read-only field gives access to the type of the token returned by the #az_json_reader,
  /// and it shouldn't be modified by the caller.
  az_json_token_kind kind;

  /// This read-only field gives access to the size of the JSON text slice that represents the token
  /// value, and it shouldn't be modified by the caller. This is useful if the token straddles
  /// non-contiguous buffers, to figure out what sized destination buffer to provide when calling
  /// #az_json_token_copy_into_span().
  int32_t size;

  struct
  {
    /// A flag to indicate whether the JSON token straddles more than one buffer segment and is
    /// split amongst non-contiguous buffers. For tokens created from input JSON payloads within a
    /// contiguous buffer, this field is always false.
    bool is_multisegment;

    /// A flag to indicate whether the JSON string contained any escaped characters, used as an
    /// optimization to avoid redundant checks. It is meaningless for any other token kind.
    bool string_has_escaped_chars;

    /// This is the first segment in the entire JSON payload, if it was non-contiguous. Otherwise,
    /// its set to #AZ_SPAN_EMPTY.
    az_span* pointer_to_first_buffer;

    /// The segment index within the non-contiguous JSON payload where this token starts.
    int32_t start_buffer_index;

    /// The offset within the particular segment within which this token starts.
    int32_t start_buffer_offset;

    /// The segment index within the non-contiguous JSON payload where this token ends.
    int32_t end_buffer_index;

    /// The offset within the particular segment within which this token ends.
    int32_t end_buffer_offset;
  } _internal;
} az_json_token;

// TODO: Should the parameters be reversed?
/**
 * @brief Copies the content of the \p token #az_json_token to the \p destination #az_span.
 *
 * @param[in] json_token A pointer to an #az_json_token instance containing the JSON text to copy to
 * the \p destination.
 * @param destination The #az_span whose bytes will be replaced by the JSON text from the \p
 * json_token.
 *
 * @return An #az_span that is a slice of the \p destination #az_span (i.e. the remainder) after the
 * token bytes have been copied.
 *
 * @remarks The function assumes that the \p destination has a large enough size to hold the
 * contents of \p json_token.
 *
 * @remarks If \p json_token doesn't contain any text, this function will just return \p
 * destination.
 */
az_span az_json_token_copy_into_span(az_json_token const* json_token, az_span destination);

/**
 * @brief Gets the JSON token's boolean.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The boolean value is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_TRUE or #AZ_JSON_TOKEN_FALSE.
 */
AZ_NODISCARD az_result az_json_token_get_boolean(az_json_token const* json_token, bool* out_value);

/**
 * @brief Gets the JSON token's number as a 64-bit unsigned integer.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_NUMBER.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR A non-ASCII digit is found within the \p json_token or \p
 * json_token contains a number that would overflow or underflow `uint64_t`.
 */
AZ_NODISCARD az_result
az_json_token_get_uint64(az_json_token const* json_token, uint64_t* out_value);

/**
 * @brief Gets the JSON token's number as a 32-bit unsigned integer.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_NUMBER.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR A non-ASCII digit is found within the token or if it contains a
 * number that would overflow or underflow `uint32_t`.
 */
AZ_NODISCARD az_result
az_json_token_get_uint32(az_json_token const* json_token, uint32_t* out_value);

/**
 * @brief Gets the JSON token's number as a 64-bit signed integer.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_NUMBER.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR A non-ASCII digit is found within the token or if it contains
 * a number that would overflow or underflow `int64_t`.
 */
AZ_NODISCARD az_result az_json_token_get_int64(az_json_token const* json_token, int64_t* out_value);

/**
 * @brief Gets the JSON token's number as a 32-bit signed integer.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_NUMBER.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR A non-ASCII digit is found within the token or if it contains a
 * number that would overflow or underflow `int32_t`.
 */
AZ_NODISCARD az_result az_json_token_get_int32(az_json_token const* json_token, int32_t* out_value);

/**
 * @brief Gets the JSON token's number as a `double`.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param[out] out_value A pointer to a variable to receive the value.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_NUMBER.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR The resulting \p out_value wouldn't be a finite double number.
 */
AZ_NODISCARD az_result az_json_token_get_double(az_json_token const* json_token, double* out_value);

/**
 * @brief Gets the JSON token's string after unescaping it, if required.
 *
 * @param[in] json_token A pointer to an #az_json_token instance.
 * @param destination A pointer to a buffer where the string should be copied into.
 * @param[in] destination_max_size The maximum available space within the buffer referred to by
 * \p destination.
 * @param[out] out_string_length __[nullable]__ Contains the number of bytes written to the \p
 * destination which denote the length of the unescaped string. If `NULL` is passed, the parameter
 * is ignored.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The string is returned.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The kind is not #AZ_JSON_TOKEN_STRING.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE \p destination does not have enough size.
 */
AZ_NODISCARD az_result az_json_token_get_string(
    az_json_token const* json_token,
    char* destination,
    int32_t destination_max_size,
    int32_t* out_string_length);

/**
 * @brief Determines whether the unescaped JSON token value that the #az_json_token points to is
 * equal to the expected text within the provided byte span by doing a case-sensitive comparison.
 *
 * @param[in] json_token A pointer to an #az_json_token instance containing the JSON string token.
 * @param[in] expected_text The lookup text to compare the token against.
 *
 * @return `true` if the current JSON token value in the JSON source semantically matches the
 * expected lookup text, with the exact casing; otherwise, `false`.
 *
 * @remarks This operation is only valid for the string and property name token kinds. For all other
 * token kinds, it returns false.
 */
AZ_NODISCARD bool az_json_token_is_text_equal(
    az_json_token const* json_token,
    az_span expected_text);

/************************************ JSON WRITER ******************/

/**
 * @brief Allows the user to define custom behavior when writing JSON using the #az_json_writer.
 */
typedef struct
{
  struct
  {
    /// Currently, this is unused, but needed as a placeholder since we can't have an empty struct.
    bool unused;
  } _internal;
} az_json_writer_options;

/**
 * @brief Gets the default json writer options which builds minimized JSON (with no extra white
 * space) according to the JSON RFC.
 *
 * @details Call this to obtain an initialized #az_json_writer_options structure that can be
 * modified and passed to #az_json_writer_init().
 *
 * @return The default #az_json_writer_options.
 */
AZ_NODISCARD AZ_INLINE az_json_writer_options az_json_writer_options_default()
{
  az_json_writer_options options = {
    ._internal = {
      .unused = false,
    },
  };

  return options;
}

/**
 * @brief Provides forward-only, non-cached writing of UTF-8 encoded JSON text into the provided
 * buffer.
 *
 * @remarks #az_json_writer builds the text sequentially with no caching and by default adheres to
 * the JSON RFC: https://tools.ietf.org/html/rfc8259.
 */
typedef struct
{
  /// The total number of bytes written by the #az_json_writer to the output destination buffer(s).
  /// This read-only field tracks the number of bytes of JSON written so far, and it shouldn't be
  /// modified by the caller.
  int32_t total_bytes_written;

  struct
  {
    /// The destination to write the JSON into.
    az_span destination_buffer;

    /// The bytes written in the current destination buffer.
    int32_t bytes_written; // For single contiguous buffer, bytes_written == total_bytes_written

    /// Allocator used to support non-contiguous buffer as a destination.
    az_span_allocator_fn allocator_callback;

    /// Any struct that was provided by the user for their specific implementation, passed through
    /// to the #az_span_allocator_fn.
    void* user_context;

    /// A state to remember when to emit a comma between JSON array and object elements.
    bool need_comma;

    /// The current state of the writer based on the last token written, used for validating the
    /// correctness of the JSON being written.
    az_json_token_kind token_kind; // needed for validation, potentially #if/def with preconditions.

    /// The current state of the writer based on the last JSON container it is in (whether array or
    /// object), used for validating the correctness of the JSON being written, and so it doesn't
    /// overflow the maximum supported depth.
    _az_json_bit_stack bit_stack; // needed for validation, potentially #if/def with preconditions.

    /// A copy of the options provided by the user.
    az_json_writer_options options;
  } _internal;
} az_json_writer;

/**
 * @brief Initializes an #az_json_writer which writes JSON text into a buffer.
 *
 * @param[out] out_json_writer A pointer to an #az_json_writer instance to initialize.
 * @param destination_buffer An #az_span over the byte buffer where the JSON text is to be written.
 * @param[in] options __[nullable]__ A reference to an #az_json_writer_options
 * structure which defines custom behavior of the #az_json_writer. If `NULL` is passed, the writer
 * will use the default options (i.e. #az_json_writer_options_default()).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK #az_json_writer is initialized successfully.
 * @retval other Initialization failed.
 */
AZ_NODISCARD az_result az_json_writer_init(
    az_json_writer* out_json_writer,
    az_span destination_buffer,
    az_json_writer_options const* options);

/**
 * @brief Initializes an #az_json_writer which writes JSON text into a destination that can contain
 * non-contiguous buffers.
 *
 * @param[out] out_json_writer A pointer to an #az_json_writer the instance to initialize.
 * @param[in] first_destination_buffer An #az_span over the byte buffer where the JSON text is to be
 * written at the start.
 * @param[in] allocator_callback An #az_span_allocator_fn callback function that provides the
 * destination span to write the JSON text to once the previous buffer is full or too small to
 * contain the next token.
 * @param user_context A context specific user-defined struct or set of fields that is passed
 * through to calls to the #az_span_allocator_fn.
 * @param[in] options __[nullable]__ A reference to an #az_json_writer_options
 * structure which defines custom behavior of the #az_json_writer. If `NULL` is passed, the writer
 * will use the default options (i.e. #az_json_writer_options_default()).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The #az_json_writer is initialized successfully.
 * @retval other Failure.
 */
AZ_NODISCARD az_result az_json_writer_chunked_init(
    az_json_writer* out_json_writer,
    az_span first_destination_buffer,
    az_span_allocator_fn allocator_callback,
    void* user_context,
    az_json_writer_options const* options);

/**
 * @brief Returns the #az_span containing the JSON text written to the underlying buffer so far, in
 * the last provided destination buffer.
 *
 * @param[in] json_writer A pointer to an #az_json_writer instance wrapping the destination buffer.
 *
 * @note Do NOT modify or override the contents of the returned #az_span unless you are no longer
 * writing JSON text into it.
 *
 * @return An #az_span containing the JSON text built so far.
 *
 * @remarks This function returns the entire JSON text when it fits in the first provided buffer,
 * where the destination is a single, contiguous buffer. When the destination can be a set of
 * non-contiguous buffers (using #az_json_writer_chunked_init()), and the JSON is larger than the
 * first provided destination span, this function only returns the text written into the last
 * provided destination buffer from the allocator callback.
 */
AZ_NODISCARD AZ_INLINE az_span
az_json_writer_get_bytes_used_in_destination(az_json_writer const* json_writer)
{
  return az_span_slice(
      json_writer->_internal.destination_buffer, 0, json_writer->_internal.bytes_written);
}

/**
 * @brief Appends the UTF-8 text value (as a JSON string) into the buffer.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the string value to.
 * @param[in] value The UTF-8 encoded value to be written as a JSON string. The value is escaped
 * before writing.
 *
 * @note If you receive an #AZ_ERROR_NOT_ENOUGH_SPACE result while appending data for which there is
 * sufficient space, note that the JSON writer requires at least 64 bytes of slack within the
 * output buffer, above the theoretical minimal space needed. The JSON writer pessimistically
 * requires this extra space because it tries to write formatted text in chunks rather than one
 * character at a time, whenever the input data is dynamic in size.
 *
 * @remarks If \p value is #AZ_SPAN_EMPTY, the empty JSON string value is written (i.e. "").
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The string value was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_string(az_json_writer* ref_json_writer, az_span value);

/**
 * @brief Appends an existing UTF-8 encoded JSON text into the buffer, useful for appending nested
 * JSON.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the JSON text to.
 * @param[in] json_text A single, possibly nested, valid, UTF-8 encoded, JSON value to be written as
 * is, without any formatting or spacing changes. No modifications are made to this text, including
 * escaping.
 *
 * @note If you receive an #AZ_ERROR_NOT_ENOUGH_SPACE result while appending data for which there is
 * sufficient space, note that the JSON writer requires at least 64 bytes of slack within the
 * output buffer, above the theoretical minimal space needed. The JSON writer pessimistically
 * requires this extra space because it tries to write formatted text in chunks rather than one
 * character at a time, whenever the input data is dynamic in size.
 *
 * @remarks A single, possibly nested, JSON value is one that starts and ends with {} or [] or is a
 * single primitive token. The JSON cannot start with an end object or array, or a property name, or
 * be incomplete.
 *
 * @remarks The function validates that the provided JSON to be appended is valid and properly
 * escaped, and fails otherwise.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The provided \p json_text was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The destination is too small for the provided \p json_text.
 * @retval #AZ_ERROR_JSON_INVALID_STATE The \p ref_json_writer is in a state where the \p json_text
 * cannot be appended because it would result in invalid JSON.
 * @retval #AZ_ERROR_UNEXPECTED_END The provided \p json_text is invalid because it is incomplete
 * and ends too early.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR The provided \p json_text is invalid because of an unexpected
 * character.
 */
AZ_NODISCARD az_result
az_json_writer_append_json_text(az_json_writer* ref_json_writer, az_span json_text);

/**
 * @brief Appends the UTF-8 property name (as a JSON string) which is the first part of a name/value
 * pair of a JSON object.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the property name to.
 * @param[in] name The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 *
 * @note If you receive an #AZ_ERROR_NOT_ENOUGH_SPACE result while appending data for which there is
 * sufficient space, note that the JSON writer requires at least 64 bytes of slack within the
 * output buffer, above the theoretical minimal space needed. The JSON writer pessimistically
 * requires this extra space because it tries to write formatted text in chunks rather than one
 * character at a time, whenever the input data is dynamic in size.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The property name was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result
az_json_writer_append_property_name(az_json_writer* ref_json_writer, az_span name);

/**
 * @brief Appends a boolean value (as a JSON literal `true` or `false`).
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the boolean to.
 * @param[in] value The value to be written as a JSON literal `true` or `false`.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The boolean was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_bool(az_json_writer* ref_json_writer, bool value);

/**
 * @brief Appends an `int32_t` number value.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the number to.
 * @param[in] value The value to be written as a JSON number.
 *
 * @note If you receive an #AZ_ERROR_NOT_ENOUGH_SPACE result while appending data for which there is
 * sufficient space, note that the JSON writer requires at least 64 bytes of slack within the
 * output buffer, above the theoretical minimal space needed. The JSON writer pessimistically
 * requires this extra space because it tries to write formatted text in chunks rather than one
 * character at a time, whenever the input data is dynamic in size.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_int32(az_json_writer* ref_json_writer, int32_t value);

/**
 * @brief Appends a `double` number value.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the number to.
 * @param[in] value The value to be written as a JSON number.
 * @param[in] fractional_digits The number of digits of the \p value to write after the decimal
 * point and truncate the rest.
 *
 * @note If you receive an #AZ_ERROR_NOT_ENOUGH_SPACE result while appending data for which there is
 * sufficient space, note that the JSON writer requires at least 64 bytes of slack within the
 * output buffer, above the theoretical minimal space needed. The JSON writer pessimistically
 * requires this extra space because it tries to write formatted text in chunks rather than one
 * character at a time, whenever the input data is dynamic in size.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The number was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 * @retval #AZ_ERROR_NOT_SUPPORTED The \p value contains an integer component that is too large and
 * would overflow beyond `2^53 - 1`.
 *
 * @remark Only finite double values are supported. Values such as `NAN` and `INFINITY` are not
 * allowed and would lead to invalid JSON being written.
 *
 * @remark Non-significant trailing zeros (after the decimal point) are not written, even if \p
 * fractional_digits is large enough to allow the zero padding.
 *
 * @remark The \p fractional_digits must be between 0 and 15 (inclusive). Any value passed in that
 * is larger will be clamped down to 15.
 */
AZ_NODISCARD az_result az_json_writer_append_double(
    az_json_writer* ref_json_writer,
    double value,
    int32_t fractional_digits);

/**
 * @brief Appends the JSON literal `null`.
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the `null` literal to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK `null` was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_null(az_json_writer* ref_json_writer);

/**
 * @brief Appends the beginning of a JSON object (i.e. `{`).
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the start of object to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Object start was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 * @retval #AZ_ERROR_JSON_NESTING_OVERFLOW The depth of the JSON exceeds the maximum allowed
 * depth of 64.
 */
AZ_NODISCARD az_result az_json_writer_append_begin_object(az_json_writer* ref_json_writer);

/**
 * @brief Appends the beginning of a JSON array (i.e. `[`).
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the start of array to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Array start was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 * @retval #AZ_ERROR_JSON_NESTING_OVERFLOW The depth of the JSON exceeds the maximum allowed depth
 * of 64.
 */
AZ_NODISCARD az_result az_json_writer_append_begin_array(az_json_writer* ref_json_writer);

/**
 * @brief Appends the end of the current JSON object (i.e. `}`).
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the closing character to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Object end was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_end_object(az_json_writer* ref_json_writer);

/**
 * @brief Appends the end of the current JSON array (i.e. `]`).
 *
 * @param[in,out] ref_json_writer A pointer to an #az_json_writer instance containing the buffer to
 * append the closing character to.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK Array end was appended successfully.
 * @retval #AZ_ERROR_NOT_ENOUGH_SPACE The buffer is too small.
 */
AZ_NODISCARD az_result az_json_writer_append_end_array(az_json_writer* ref_json_writer);

/************************************ JSON READER ******************/

/**
 * @brief Allows the user to define custom behavior when reading JSON using the #az_json_reader.
 */
typedef struct
{
  struct
  {
    /// Currently, this is unused, but needed as a placeholder since we can't have an empty struct.
    bool unused;
  } _internal;
} az_json_reader_options;

/**
 * @brief Gets the default json reader options which reads the JSON strictly according to the JSON
 * RFC.
 *
 * @details Call this to obtain an initialized #az_json_reader_options structure that can be
 * modified and passed to #az_json_reader_init().
 *
 * @return The default #az_json_reader_options.
 */
AZ_NODISCARD AZ_INLINE az_json_reader_options az_json_reader_options_default()
{
  az_json_reader_options options = {
    ._internal = {
      .unused = false,
    },
  };

  return options;
}

/**
 * @brief Returns the JSON tokens contained within a JSON buffer, one at a time.
 *
 * @remarks The token field is meant to be used as read-only to return the #az_json_token while
 * reading the JSON. Do NOT modify it.
 */
typedef struct
{
  /// This read-only field gives access to the current token that the #az_json_reader has processed,
  /// and it shouldn't be modified by the caller.
  az_json_token token;

  /// The depth of the current token. This read-only field tracks the recursive depth of the nested
  /// objects or arrays within the JSON text processed so far, and it shouldn't be modified by the
  /// caller.
  int32_t current_depth;

  struct
  {
    /// The first buffer containing the JSON payload.
    az_span json_buffer;

    /// The array of non-contiguous buffers containing the JSON payload, which will be null for the
    /// single buffer case.
    az_span* json_buffers;

    /// The number of non-contiguous buffer segments in the array. It is set to one for the single
    /// buffer case.
    int32_t number_of_buffers;

    /// The current buffer segment being processed while reading the JSON in non-contiguous buffer
    /// segments.
    int32_t buffer_index;

    /// The number of bytes consumed so far in the current buffer segment.
    int32_t bytes_consumed;

    /// The total bytes consumed from the input JSON payload. In the case of a single buffer, this
    /// is identical to bytes_consumed.
    int32_t total_bytes_consumed;

    /// Flag which indicates that we have a JSON object or array in the payload, rather than a
    /// single primitive token (string, number, true, false, null).
    bool is_complex_json;

    /// A limited stack to track the depth and nested JSON objects or arrays read so far.
    _az_json_bit_stack bit_stack;

    /// A copy of the options provided by the user.
    az_json_reader_options options;
  } _internal;
} az_json_reader;

/**
 * @brief Initializes an #az_json_reader to read the JSON payload contained within the provided
 * buffer.
 *
 * @param[out] out_json_reader A pointer to an #az_json_reader instance to initialize.
 * @param[in] json_buffer An #az_span over the byte buffer containing the JSON text to read.
 * @param[in] options __[nullable]__ A reference to an #az_json_reader_options structure which
 * defines custom behavior of the #az_json_reader. If `NULL` is passed, the reader will use the
 * default options (i.e. #az_json_reader_options_default()).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The #az_json_reader is initialized successfully.
 * @retval other Initialization failed.
 *
 * @remarks The provided json buffer must not be empty, as that is invalid JSON.
 *
 * @remarks An instance of #az_json_reader must not outlive the lifetime of the JSON payload within
 * the \p json_buffer.
 */
AZ_NODISCARD az_result az_json_reader_init(
    az_json_reader* out_json_reader,
    az_span json_buffer,
    az_json_reader_options const* options);

/**
 * @brief Initializes an #az_json_reader to read the JSON payload contained within the provided
 * set of discontiguous buffers.
 *
 * @param[out] out_json_reader A pointer to an #az_json_reader instance to initialize.
 * @param[in] json_buffers An array of non-contiguous byte buffers, as spans, containing the JSON
 * text to read.
 * @param[in] number_of_buffers The number of buffer segments provided, i.e. the length of the \p
 * json_buffers array.
 * @param[in] options __[nullable]__ A reference to an #az_json_reader_options
 * structure which defines custom behavior of the #az_json_reader. If `NULL` is passed, the reader
 * will use the default options (i.e. #az_json_reader_options_default()).
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The #az_json_reader is initialized successfully.
 * @retval other Initialization failed.
 *
 * @remarks The provided array of json buffers must not be empty, as that is invalid JSON, and
 * therefore \p number_of_buffers must also be greater than 0. The array must also not contain any
 * empty span segments.
 *
 * @remarks An instance of #az_json_reader must not outlive the lifetime of the JSON payload within
 * the \p json_buffers.
 */
AZ_NODISCARD az_result az_json_reader_chunked_init(
    az_json_reader* out_json_reader,
    az_span json_buffers[],
    int32_t number_of_buffers,
    az_json_reader_options const* options);

/**
 * @brief Reads the next token in the JSON text and updates the reader state.
 *
 * @param[in,out] ref_json_reader A pointer to an #az_json_reader instance containing the JSON to
 * read.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The token was read successfully.
 * @retval #AZ_ERROR_UNEXPECTED_END The end of the JSON document is reached.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR An invalid character is detected.
 * @retval #AZ_ERROR_JSON_READER_DONE No more JSON text left to process.
 */
AZ_NODISCARD az_result az_json_reader_next_token(az_json_reader* ref_json_reader);

/**
 * @brief Reads and skips over any nested JSON elements.
 *
 * @param[in,out] ref_json_reader A pointer to an #az_json_reader instance containing the JSON to
 * read.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The children of the current JSON token are skipped successfully.
 * @retval #AZ_ERROR_UNEXPECTED_END The end of the JSON document is reached.
 * @retval #AZ_ERROR_UNEXPECTED_CHAR An invalid character is detected.
 *
 * @remarks If the current token kind is a property name, the reader first moves to the property
 * value. Then, if the token kind is start of an object or array, the reader moves to the matching
 * end object or array. For all other token kinds, the reader doesn't move and returns #AZ_OK.
 */
AZ_NODISCARD az_result az_json_reader_skip_children(az_json_reader* ref_json_reader);

/**
 * @brief Unescapes the JSON string within the provided #az_span.
 *
 * @param[in] json_string The #az_span that contains the string to be unescaped.
 * @param destination The destination buffer used to write the unescaped output into.
 *
 * @return An #az_span that is a slice of the \p destination #az_span containing the unescaped JSON
 * string, which denotes the length of the unescaped string.
 *
 * @remarks For user-defined or unknown input, the buffer referred to by \p destination must be at
 * least as large as the \p json_string #az_span. Content is copied from the source buffer, while
 * unescaping.
 *
 * @remarks This function assumes that the \p json_string input is well-formed JSON.
 *
 * @remarks This function assumes that the \p destination has a large enough size to hold the
 * unescaped \p json_string.
 *
 * @remarks This API can also be used to perform in place unescaping. However, doing so, is
 * destructive and the input JSON may no longer be valid or parsable.
 */
AZ_NODISCARD az_span az_json_string_unescape(az_span json_string, az_span destination);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_JSON_H
