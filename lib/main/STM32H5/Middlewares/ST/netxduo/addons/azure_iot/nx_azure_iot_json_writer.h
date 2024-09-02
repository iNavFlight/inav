/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Version: 6.1 */

/**
 * @file nx_azure_iot_json_writer.h
 *
 */

#ifndef NX_AZURE_IOT_JSON_WRITER_H
#define NX_AZURE_IOT_JSON_WRITER_H

#include "azure/core/az_json.h"
#include "nx_api.h"

#ifdef __cplusplus
extern   "C" {
#endif

/**
 * @brief Provides forward-only, non-cached writing of UTF-8 encoded JSON text into the provided
 * buffer.
 *
 * @remarks #az_json_writer builds the text sequentially with no caching and by default adheres to
 * the JSON RFC: https://tools.ietf.org/html/rfc8259.
 *
 */
typedef struct NX_AZURE_IOT_JSON_WRITER_STRUCT
{
    NX_PACKET *packet_ptr;
    az_json_writer json_writer;
    UINT wait_option;
    ULONG nx_tail_packet_offset;
    ULONG nx_packet_init_length;
} NX_AZURE_IOT_JSON_WRITER;

/**
 * @brief Initializes an #NX_AZURE_IOT_JSON_WRITER which writes JSON text into a NX_PACKET.
 *
 * @param[out] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to initialize.
 * @param[in] packet_ptr A pointer to #NX_PACKET.
 * @param[in] wait_option Ticks to wait for allocating next packet
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Successfully initialized JSON writer.
 */
UINT nx_azure_iot_json_writer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                   NX_PACKET *packet_ptr, UINT wait_option);

/**
 * @brief Initializes an #NX_AZURE_IOT_JSON_WRITER which writes JSON text into a buffer passed.
 *
 * @param[out] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to initialize.
 * @param[in] buffer_ptr A buffer pointer to which JSON text will be written.
 * @param[in] buffer_len Length of buffer.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Successfully initialized JSON writer.
 */
UINT nx_azure_iot_json_writer_with_buffer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_len);

/**
 * @brief Deinitializes an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @param[out] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to de-initialize.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Successfully de-initialized JSON writer.
 */
UINT nx_azure_iot_json_writer_deinit(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the UTF-8 property name and value where value is int32
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] property_name The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] property_name_len Length of property_name.
 * @param[in] value The value to be written as a JSON number.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name and int32 value was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_property_with_int32_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                               const UCHAR *property_name, UINT property_name_len,
                                                               int32_t value);

/**
 * @brief Appends the UTF-8 property name and value where value is double
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] property_name The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] property_name_len Length of property_name.
 * @param[in] value The value to be written as a JSON number.
 * @param[in] fractional_digits The number of digits of the value to write after the decimal point and truncate the rest.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name and double value was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_property_with_double_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                double value, UINT fractional_digits);

/**
 * @brief Appends the UTF-8 property name and value where value is boolean
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] property_name The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] property_name_len Length of property_name.
 * @param[in] value The value to be written as a JSON literal `true` or `false`.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name and bool value was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_property_with_bool_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                              const UCHAR *property_name, UINT property_name_len,
                                                              UINT value);

/**
 * @brief Appends the UTF-8 property name and value where value is string
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] property_name The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] property_name_len Length of property_name.
 * @param[in] value The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] value_len Length of value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name and string value was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_property_with_string_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                const UCHAR *value, UINT value_len);

/**
 * @brief Returns the length containing the JSON text written to the underlying buffer.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An UINT containing the length of JSON text built so far.
 */
UINT nx_azure_iot_json_writer_get_bytes_used(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the UTF-8 text value (as a JSON string) into the buffer.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] value Pointer of UCHAR buffer that contains UTF-8 encoded value to be written as a JSON string.
 * The value is escaped before writing.
 * @param[in] value_len Length of value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The string value was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_string(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            const UCHAR *value, UINT value_len);

/**
 * @brief Appends an existing UTF-8 encoded JSON text into the buffer, useful for appending nested
 * JSON.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] json A pointer to single, possibly nested, valid, UTF-8 encoded, JSON value to be written as
 * is, without any formatting or spacing changes. No modifications are made to this text, including
 * escaping.
 * @param[in] json_len Length of json
 * 
 * @remarks A single, possibly nested, JSON value is one that starts and ends with {} or [] or is a
 * single primitive token. The JSON cannot start with an end object or array, or a property name, or
 * be incomplete.
 *
 * @remarks The function validates that the provided JSON to be appended is valid and properly
 * escaped, and fails otherwise.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The provided json_text was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_json_text(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               const UCHAR *json, UINT json_len);

/**
 * @brief Appends the UTF-8 property name (as a JSON string) which is the first part of a name/value
 * pair of a JSON object.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] value The UTF-8 encoded property name of the JSON value to be written. The name is
 * escaped before writing.
 * @param[in] value_len Length of name.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_property_name(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                   const UCHAR *value, UINT value_len);

/**
 * @brief Appends a boolean value (as a JSON literal `true` or `false`).
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] value The value to be written as a JSON literal `true` or `false`.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The bool was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_bool(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, UINT value);

/**
 * @brief Appends an `int32_t` number value.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] value The value to be written as a JSON number.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The number was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_int32(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, int32_t value);

/**
 * @brief Appends a `double` number value.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 * @param[in] value The value to be written as a JSON number.
 * @param[in] fractional_digits The number of digits of the \p value to write after the decimal
 * point and truncate the rest.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The number was appended successfully.
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
UINT nx_azure_iot_json_writer_append_double(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            double value, int32_t fractional_digits);

/**
 * @brief Appends the JSON literal `null`.
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS `null` was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_null(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the beginning of a JSON object (i.e. `{`).
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Object start was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_begin_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the beginning of a JSON array (i.e. `[`).
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Array start was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_begin_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the end of the current JSON object (i.e. `}`).
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Object end was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_end_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

/**
 * @brief Appends the end of the current JSON array (i.e. `]`).
 *
 * @param[in] json_writer_ptr A pointer to an #NX_AZURE_IOT_JSON_WRITER.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS Array end was appended successfully.
 */
UINT nx_azure_iot_json_writer_append_end_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_JSON_WRITER_H */
