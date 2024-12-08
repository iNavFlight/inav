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
 * @file nx_azure_iot_json_reader.h
 *
 */

#ifndef NX_AZURE_IOT_JSON_READER_H
#define NX_AZURE_IOT_JSON_READER_H

#include "azure/core/az_json.h"
#include "nx_api.h"

#ifdef __cplusplus
extern   "C" {
#endif

#ifndef NX_AZURE_IOT_READER_MAX_LIST
#define NX_AZURE_IOT_READER_MAX_LIST                (15)
#endif /* NX_AZURE_IOT_READER_MAX_LIST */

/**
 * Defines symbols for the various kinds of JSON tokens that make up any JSON text.
 */
#define NX_AZURE_IOT_READER_TOKEN_NONE              AZ_JSON_TOKEN_NONE
#define NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT      AZ_JSON_TOKEN_BEGIN_OBJECT
#define NX_AZURE_IOT_READER_TOKEN_END_OBJECT        AZ_JSON_TOKEN_END_OBJECT
#define NX_AZURE_IOT_READER_TOKEN_BEGIN_ARRAY       AZ_JSON_TOKEN_BEGIN_ARRAY
#define NX_AZURE_IOT_READER_TOKEN_END_ARRAY         AZ_JSON_TOKEN_END_ARRAY
#define NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME     AZ_JSON_TOKEN_PROPERTY_NAME
#define NX_AZURE_IOT_READER_TOKEN_STRING            AZ_JSON_TOKEN_STRING
#define NX_AZURE_IOT_READER_TOKEN_NUMBER            AZ_JSON_TOKEN_NUMBER
#define NX_AZURE_IOT_READER_TOKEN_TRUE              AZ_JSON_TOKEN_TRUE
#define NX_AZURE_IOT_READER_TOKEN_FALSE             AZ_JSON_TOKEN_FALSE
#define NX_AZURE_IOT_READER_TOKEN_NULL              AZ_JSON_TOKEN_NULL

/**
 * @brief Returns the JSON tokens contained within a JSON buffer, one at a time.
 *
 */
typedef struct NX_AZURE_IOT_READER_STRUCT
{
      NX_PACKET *packet_ptr;
      az_json_reader json_reader;
      az_span span_list[NX_AZURE_IOT_READER_MAX_LIST];
      ULONG json_length;
} NX_AZURE_IOT_JSON_READER;

/**
 * @brief Initializes an #NX_AZURE_IOT_JSON_READER to read the JSON payload contained within the provided
 * buffer.
 *
 * @param[out] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance to initialize.
 * @param[in] buffer_ptr An pointer to buffer containing the JSON text to read.
 * @param[in] buffer_len Length of buffer.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The #NX_AZURE_IOT_JSON_READER is initialized successfully.
 * @retval other Initialization failed.
 *
 */
UINT nx_azure_iot_json_reader_with_buffer_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               const UCHAR *buffer_ptr, UINT buffer_len);

/**
 * @brief Initializes an #NX_AZURE_IOT_JSON_READER to read the JSON payload contained within #NX_PACKET
 *
 * @param[out] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance to initialize.
 * @param[in] packet_ptr A pointer to #NX_PACKET containing the JSON text
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The #NX_AZURE_IOT_JSON_READER is initialized successfully.
 * @retval other Initialization failed.
 *
 * @remarks Ownership of #NX_PACKET is taken by the reader.
 */
UINT nx_azure_iot_json_reader_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                   NX_PACKET *packet_ptr);

/**
 * @brief De-initializes an #NX_AZURE_IOT_JSON_READER
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance to de-initialize
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The #NX_AZURE_IOT_JSON_READER is de-initialized successfully.
 */
UINT nx_azure_iot_json_reader_deinit(NX_AZURE_IOT_JSON_READER *reader_ptr);

/**
 * @brief Reads the next token in the JSON text and updates the reader state.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to
 * read.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The token was read successfully.
 */
UINT nx_azure_iot_json_reader_next_token(NX_AZURE_IOT_JSON_READER *reader_ptr);

/**
 * @brief Reads and skips over any nested JSON elements.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to
 * read.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The children of the current JSON token are skipped successfully.
 *
 * @remarks If the current token kind is a property name, the reader first moves to the property
 * value. Then, if the token kind is start of an object or array, the reader moves to the matching
 * end object or array. For all other token kinds, the reader doesn't move and returns #NX_AZURE_IOT_SUCCESS.
 */
UINT nx_azure_iot_json_reader_skip_children(NX_AZURE_IOT_JSON_READER *reader_ptr);

/**
 * @brief Gets the JSON token's boolean value.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 * @param[out] value_ptr A pointer to a variable to receive the value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The boolean value is returned.
 */
UINT nx_azure_iot_json_reader_token_bool_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                             UINT *value_ptr);

/**
 * @brief Gets the JSON token's number as a 32-bit unsigned integer.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 * @param[out] value_ptr A pointer to a variable to receive the value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The number is returned.
 */
UINT nx_azure_iot_json_reader_token_uint32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               uint32_t *value_ptr);

/**
 * @brief Gets the JSON token's number as a 32-bit signed integer.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 * @param[out] value_ptr A pointer to a variable to receive the value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The number is returned.
 */
UINT nx_azure_iot_json_reader_token_int32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                              int32_t *value_ptr);

/**
 * @brief Gets the JSON token's number as a `double`.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 * @param[out] value_ptr A pointer to a variable to receive the value.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The number is returned.
 */
UINT nx_azure_iot_json_reader_token_double_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               double *value_ptr);

/**
 * @brief Gets the JSON token's string after unescaping it, if required.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 * @param[out] buffer_ptr A pointer to a buffer where the string should be copied into.
 * @param[in] buffer_size The maximum available space within the buffer referred to by buffer_ptr.
 * @param[out] bytes_copied Contains the number of bytes written to the \p
 * destination which denote the length of the unescaped string.
 *
 * @return An `UINT` value indicating the result of the operation.
 * @retval #NX_AZURE_IOT_SUCCESS The property name was appended successfully.
 */
UINT nx_azure_iot_json_reader_token_string_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_size, UINT *bytes_copied);

/**
 * @brief Determines whether the unescaped JSON token value that the #NX_AZURE_IOT_JSON_READER points to is
 * equal to the expected text within the provided buffer bytes by doing a case-sensitive comparison.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON string token.
 * @param[in] expected_text_ptr A pointer to lookup text to compare the token against.
 * @param[in] expected_text_len Length of expected_text_ptr.
 *
 * @return `1` if the current JSON token value in the JSON source semantically matches the
 * expected lookup text, with the exact casing; otherwise, `0`.
 *
 * @remarks This operation is only valid for the string and property name token kinds. For all other
 * token kinds, it returns 0.
 */
UINT nx_azure_iot_json_reader_token_is_text_equal(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                  UCHAR *expected_text_ptr, UINT expected_text_len);

/**
 * @brief Determines type of token currently #NX_AZURE_IOT_JSON_READER points to.
 *
 * @param[in] reader_ptr A pointer to an #NX_AZURE_IOT_JSON_READER instance.
 *
 * @return An `UINT` value indicating the type of token.
 */
UINT nx_azure_iot_json_reader_token_type(NX_AZURE_IOT_JSON_READER *reader_ptr);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_JSON_READER_H */
