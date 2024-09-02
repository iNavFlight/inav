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

#include "nx_azure_iot_json_writer.h"

#include "nx_azure_iot.h"

static az_result nx_azure_iot_json_wirter_packet_allocator_cb(az_span_allocator_context* allocator_context,
                                                              az_span* out_next_destination)
{
NX_AZURE_IOT_JSON_WRITER *writer_ptr = (NX_AZURE_IOT_JSON_WRITER *)allocator_context -> user_context;
NX_PACKET *packet_ptr = writer_ptr -> packet_ptr;
NX_PACKET *new_paket_ptr;
NX_PACKET *tail_packet_ptr;

    /* Allocate a new packet.  */
    if (nx_packet_allocate(packet_ptr -> nx_packet_pool_owner,
                           &new_paket_ptr, 0, writer_ptr -> wait_option))
    {
        return(AZ_ERROR_OUT_OF_MEMORY);
    }

    if (allocator_context -> minimum_required_size > (INT)(new_paket_ptr -> nx_packet_data_end -
                                                           new_paket_ptr -> nx_packet_data_start))
    {
        nx_packet_release(new_paket_ptr);
        return(AZ_ERROR_OUT_OF_MEMORY);
    }

    if (packet_ptr -> nx_packet_last)
    {
        tail_packet_ptr = packet_ptr -> nx_packet_last;
    }
    else
    {
        tail_packet_ptr = packet_ptr;
    }

    NX_ASSERT((tail_packet_ptr -> nx_packet_data_start + writer_ptr -> nx_tail_packet_offset +
               (ULONG)(allocator_context -> bytes_used)) <=
              tail_packet_ptr -> nx_packet_data_end);

    /* Update tail.  */
    tail_packet_ptr -> nx_packet_append_ptr =
        tail_packet_ptr -> nx_packet_data_start + writer_ptr -> nx_tail_packet_offset +
        (ULONG)(allocator_context -> bytes_used);
    tail_packet_ptr -> nx_packet_next = new_paket_ptr;

    /* Update head.  */
    packet_ptr -> nx_packet_length += (ULONG)(allocator_context -> bytes_used);
    packet_ptr -> nx_packet_last = new_paket_ptr;

    writer_ptr -> nx_tail_packet_offset = 0;

    *out_next_destination = az_span_create(new_paket_ptr -> nx_packet_data_start,
                                           (INT)(new_paket_ptr -> nx_packet_data_end -
                                                 new_paket_ptr -> nx_packet_data_start));

    return(AZ_OK);
}

static VOID nx_azure_iot_json_writer_packet_update(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
NX_PACKET *tail_packet_ptr;
UINT last_chunk_size =
    (UINT)az_span_size(az_json_writer_get_bytes_used_in_destination(&(json_writer_ptr -> json_writer)));

    if (json_writer_ptr -> packet_ptr == NX_NULL)
    {
        return;
    }

    if (json_writer_ptr -> packet_ptr -> nx_packet_last)
    {
        tail_packet_ptr = json_writer_ptr -> packet_ptr -> nx_packet_last;
    }
    else
    {
        tail_packet_ptr = json_writer_ptr -> packet_ptr;
    }

    NX_ASSERT((tail_packet_ptr -> nx_packet_data_start + json_writer_ptr -> nx_tail_packet_offset +
               last_chunk_size) <= tail_packet_ptr -> nx_packet_data_end);

    tail_packet_ptr -> nx_packet_append_ptr =
        tail_packet_ptr -> nx_packet_data_start + json_writer_ptr -> nx_tail_packet_offset + last_chunk_size;
    json_writer_ptr -> packet_ptr -> nx_packet_length =
        json_writer_ptr -> nx_packet_init_length + (UINT)json_writer_ptr -> json_writer.total_bytes_written;
}

UINT nx_azure_iot_json_writer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                   NX_PACKET *packet_ptr, UINT wait_option)
{
az_span span;
NX_PACKET *tail_packet_ptr;

    if ((json_writer_ptr == NX_NULL) ||
        (packet_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("Json writer init fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset((VOID *)json_writer_ptr, 0, sizeof(NX_AZURE_IOT_JSON_WRITER));

    if (packet_ptr -> nx_packet_last)
    {
        tail_packet_ptr = packet_ptr -> nx_packet_last;
    }
    else
    {
        tail_packet_ptr = packet_ptr;
    }

    span = az_span_create(tail_packet_ptr -> nx_packet_append_ptr,
                          (INT)(tail_packet_ptr -> nx_packet_data_end - tail_packet_ptr -> nx_packet_append_ptr));

    if (az_result_failed(az_json_writer_chunked_init(&(json_writer_ptr -> json_writer), span,
                                                     nx_azure_iot_json_wirter_packet_allocator_cb,
                                                     (VOID *)json_writer_ptr, NX_NULL)))
    {
        LogError(LogLiteralArgs("Json writer failed to init chunked writer"));
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    json_writer_ptr -> packet_ptr = packet_ptr;
    json_writer_ptr -> wait_option = wait_option;
    json_writer_ptr -> nx_tail_packet_offset =
        (ULONG)(tail_packet_ptr -> nx_packet_append_ptr - tail_packet_ptr -> nx_packet_data_start);
    json_writer_ptr -> nx_packet_init_length = packet_ptr -> nx_packet_length;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_with_buffer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_len)
{
az_span span;

    if ((json_writer_ptr == NX_NULL) ||
        (buffer_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("Json writer init fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset((VOID *)json_writer_ptr, 0, sizeof(NX_AZURE_IOT_JSON_WRITER));

    span = az_span_create(buffer_ptr, (INT)(buffer_len));

    if (az_result_failed(az_json_writer_init(&(json_writer_ptr -> json_writer), span, NX_NULL)))
    {
        LogError(LogLiteralArgs("Json writer failed to init writer"));
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_deinit(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer deinit fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (json_writer_ptr -> packet_ptr)
    {
        json_writer_ptr -> packet_ptr = NX_NULL;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_get_bytes_used(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer get bytes used fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return((UINT)json_writer_ptr -> json_writer.total_bytes_written);
}

UINT nx_azure_iot_json_writer_append_string(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            const UCHAR *value, UINT value_len)
{
az_span span = az_span_create((UCHAR *)value, (INT)value_len);

    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append string fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_string(&(json_writer_ptr -> json_writer), span)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_json_text(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               const UCHAR *json, UINT json_len)
{
az_span span = az_span_create((UCHAR *)json, (INT)json_len);

    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append text fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_json_text(&(json_writer_ptr -> json_writer), span)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_property_name(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                   const UCHAR *value, UINT value_len)
{
az_span span = az_span_create((UCHAR *)value, (INT)value_len);

    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append property name fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_property_name(&(json_writer_ptr -> json_writer), span)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_bool(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, UINT value)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append bool fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_bool(&(json_writer_ptr -> json_writer), value)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_int32(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, int32_t value)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append int32 fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_int32(&(json_writer_ptr -> json_writer), value)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_double(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            double value, int32_t fractional_digits)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append double fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_double(&(json_writer_ptr -> json_writer), value, fractional_digits)))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_null(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append null fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_null(&(json_writer_ptr -> json_writer))))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_begin_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append begin object fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_begin_object(&(json_writer_ptr -> json_writer))))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_begin_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append begin array fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_begin_array(&(json_writer_ptr -> json_writer))))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_end_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append end object fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_end_object(&(json_writer_ptr -> json_writer))))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_end_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr)
{
    if (json_writer_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Json writer append end array fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (az_result_failed(az_json_writer_append_end_array(&(json_writer_ptr -> json_writer))))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    nx_azure_iot_json_writer_packet_update(json_writer_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_json_writer_append_property_with_int32_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                               const UCHAR *property_name, UINT property_name_len,
                                                               int32_t value)
{
    return ((UINT)(nx_azure_iot_json_writer_append_property_name(json_writer_ptr, property_name, property_name_len) ||
                   nx_azure_iot_json_writer_append_int32(json_writer_ptr, value)));
}

UINT nx_azure_iot_json_writer_append_property_with_double_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                double value, UINT fractional_digits)
{
    return ((UINT)(nx_azure_iot_json_writer_append_property_name(json_writer_ptr, property_name, property_name_len) ||
                   nx_azure_iot_json_writer_append_double(json_writer_ptr, value, (int32_t)fractional_digits)));
}

UINT nx_azure_iot_json_writer_append_property_with_bool_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                              const UCHAR *property_name, UINT property_name_len,
                                                              UINT value)
{
    return ((UINT)(nx_azure_iot_json_writer_append_property_name(json_writer_ptr, property_name, property_name_len) ||
                   nx_azure_iot_json_writer_append_bool(json_writer_ptr, value)));
}

UINT nx_azure_iot_json_writer_append_property_with_string_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                const UCHAR *value, UINT value_len)
{
    return ((UINT)(nx_azure_iot_json_writer_append_property_name(json_writer_ptr, property_name, property_name_len) ||
                   nx_azure_iot_json_writer_append_string(json_writer_ptr, value, value_len)));
}
