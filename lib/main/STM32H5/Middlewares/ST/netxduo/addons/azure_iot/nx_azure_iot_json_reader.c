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

#include "nx_azure_iot_json_reader.h"

#include "nx_azure_iot.h"

static UINT nx_azure_iot_json_reader_translate_error_code(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                          az_result core_result)
{
    if (az_result_failed(core_result))
    {
        if (core_result == AZ_ERROR_JSON_READER_DONE)
        {
            return(NX_AZURE_IOT_NOT_FOUND);
        }

        if (reader_ptr -> json_length == 0)
        {
            return(NX_AZURE_IOT_EMPTY_JSON);
        }

        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_with_buffer_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               const UCHAR *buffer_ptr, UINT buffer_len)
{
az_span span = az_span_create((UCHAR *)buffer_ptr, (INT)buffer_len);

    if ((reader_ptr == NX_NULL) ||
        (buffer_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset(reader_ptr, 0, sizeof(NX_AZURE_IOT_JSON_READER));

    /* Allow initializing with zero size buffer, as any futher call will fail for empty JSON */
    if (buffer_len == 0)
    {
        reader_ptr -> json_reader.token.kind = AZ_JSON_TOKEN_NONE;
        
        return(NX_AZURE_IOT_SUCCESS);
    }

    reader_ptr -> json_length = (ULONG)buffer_len;

    if (az_result_failed(az_json_reader_init(&(reader_ptr -> json_reader),
                                             span, NX_NULL)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                   NX_PACKET *packet_ptr)
{
UINT count = 0;

    if ((reader_ptr == NX_NULL) ||
        (packet_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset(reader_ptr, 0, sizeof(NX_AZURE_IOT_JSON_READER));

    reader_ptr -> packet_ptr = packet_ptr;

    /* Allow initializing with zero size packet, as any futher call will fail for empty JSON */
    if (packet_ptr -> nx_packet_length == 0)
    {
        reader_ptr -> json_reader.token.kind = AZ_JSON_TOKEN_NONE;

        return(NX_AZURE_IOT_SUCCESS);
    }
    
    reader_ptr -> json_length = packet_ptr -> nx_packet_length;
    
    while (packet_ptr != NX_NULL)
    {
        if (count >= NX_AZURE_IOT_READER_MAX_LIST)
        {
            return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
        }

        reader_ptr -> span_list[count] = az_span_create(packet_ptr -> nx_packet_prepend_ptr,
                                                        (INT)(packet_ptr -> nx_packet_append_ptr -
                                                              packet_ptr -> nx_packet_prepend_ptr));

        packet_ptr = packet_ptr -> nx_packet_next;
        count++;
    }

    if (az_result_failed(az_json_reader_chunked_init(&(reader_ptr -> json_reader),
                                                     reader_ptr -> span_list,
                                                     (INT)count, NX_NULL)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_deinit(NX_AZURE_IOT_JSON_READER *reader_ptr)
{
    if ((reader_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (reader_ptr -> packet_ptr)
    {
        reader_ptr -> packet_ptr = NX_NULL;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_token_is_text_equal(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                  UCHAR *value_ptr, UINT value_len)
{
az_span span = az_span_create(value_ptr, (INT)value_len);

    if ((reader_ptr == NX_NULL) ||
        (value_ptr == NX_NULL) || (value_len == 0))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return((UINT)az_json_token_is_text_equal(&(reader_ptr -> json_reader.token), span));
}

UINT nx_azure_iot_json_reader_next_token(NX_AZURE_IOT_JSON_READER *reader_ptr)
{
az_result core_result;

    if ((reader_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    core_result = az_json_reader_next_token(&(reader_ptr -> json_reader));
    
    return(nx_azure_iot_json_reader_translate_error_code(reader_ptr, core_result));
}

UINT nx_azure_iot_json_reader_skip_children(NX_AZURE_IOT_JSON_READER *reader_ptr)
{
az_result core_result;

    if ((reader_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    core_result = az_json_reader_skip_children(&(reader_ptr -> json_reader));

    return(nx_azure_iot_json_reader_translate_error_code(reader_ptr, core_result));
}

UINT nx_azure_iot_json_reader_token_bool_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                             UINT *value_ptr)
{
bool bool_value;

    if ((reader_ptr == NX_NULL) || (value_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_token_get_boolean(&(reader_ptr -> json_reader.token), &bool_value)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    *value_ptr = bool_value;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_token_uint32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               uint32_t *value_ptr)
{
    if ((reader_ptr == NX_NULL) || (value_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_token_get_uint32(&(reader_ptr -> json_reader.token), value_ptr)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_token_int32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                              int32_t *value_ptr)
{
    if ((reader_ptr == NX_NULL) || (value_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_token_get_int32(&(reader_ptr -> json_reader.token), value_ptr)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT  nx_azure_iot_json_reader_token_double_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                double *value_ptr)
{
    if ((reader_ptr == NX_NULL) || (value_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_token_get_double(&(reader_ptr -> json_reader.token), value_ptr)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_token_string_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_size,
                                               UINT *bytes_copied)
{
int32_t bytes;

    if ((reader_ptr == NX_NULL) ||
        (buffer_ptr == NX_NULL) ||
        (buffer_size == 0) ||
        (bytes_copied == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_token_get_string(&(reader_ptr -> json_reader.token),
                                                  (CHAR *)buffer_ptr, (INT)buffer_size,
                                                  &bytes)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    *bytes_copied = (UINT)bytes;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_reader_token_type(NX_AZURE_IOT_JSON_READER *reader_ptr)
{
    if ((reader_ptr == NX_NULL))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(reader_ptr -> json_reader.token.kind);
}
