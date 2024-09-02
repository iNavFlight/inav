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

#include "nx_azure_iot_hub_client_properties.h"

extern UINT nx_azure_iot_hub_client_adjust_payload(NX_PACKET *packet_ptr);
VOID nx_azure_iot_hub_client_properties_component_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                          NX_PACKET *packet_ptr, UINT message_type);

UINT nx_azure_iot_hub_client_reported_properties_component_begin(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                 NX_AZURE_IOT_JSON_WRITER *writer_ptr,
                                                                 const UCHAR *component_name_ptr,
                                                                 USHORT component_name_length)
{
az_result core_result;
az_span component_name;

    if ((hub_client_ptr == NX_NULL) ||
        (writer_ptr == NX_NULL) ||
        (component_name_ptr == NX_NULL) ||
        (component_name_length == 0))
    {
        LogError(LogLiteralArgs("IoT PnP reported property begin fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    component_name = az_span_create((UCHAR *)component_name_ptr, (INT)component_name_length);

    core_result = az_iot_hub_client_properties_writer_begin_component(&(hub_client_ptr -> iot_hub_client_core),
                                                                      &(writer_ptr -> json_writer), component_name);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoT PnP failed to append component, core error : %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_component_end(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                               NX_AZURE_IOT_JSON_WRITER *writer_ptr)
{
az_result core_result;

    if ((hub_client_ptr == NX_NULL) ||
        (writer_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoT PnP reported property end fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    core_result = az_iot_hub_client_properties_writer_end_component(&(hub_client_ptr -> iot_hub_client_core),
                                                                    &(writer_ptr -> json_writer));
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoT PnP failed to append component, core error : %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_status_begin(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                              NX_AZURE_IOT_JSON_WRITER *writer_ptr,
                                                              const UCHAR *property_name_ptr, UINT property_name_length,
                                                              UINT status_code, ULONG version,
                                                              const UCHAR *description_ptr, UINT description_length)
{
az_span property_name;
az_span description;
az_result core_result;

    if ((hub_client_ptr == NX_NULL) ||
        (writer_ptr == NX_NULL) ||
        (property_name_ptr == NX_NULL) ||
        (property_name_length == 0) )
    {
        LogError(LogLiteralArgs("IoT PnP client begin reported status failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    property_name = az_span_create((UCHAR *)property_name_ptr, (INT)property_name_length);
    description = az_span_create((UCHAR *)description_ptr, (INT)description_length);

    core_result = az_iot_hub_client_properties_writer_begin_response_status(&(hub_client_ptr -> iot_hub_client_core),
                                                                            &(writer_ptr -> json_writer),
                                                                            property_name, (int32_t)status_code,
                                                                            (int32_t)version, description);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("Failed to prefix data with core error : %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_status_end(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            NX_AZURE_IOT_JSON_WRITER *writer_ptr)
{
az_result core_result;

    if ((hub_client_ptr == NX_NULL) ||
        (writer_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoT PnP client end reported status failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    core_result = az_iot_hub_client_properties_writer_end_response_status(&(hub_client_ptr -> iot_hub_client_core),
                                                                           &(writer_ptr -> json_writer));
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("Failed to suffix data with core error : %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_properties_version_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                    NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                    UINT message_type, ULONG *version_ptr)
{
az_result core_result;
az_iot_hub_client_properties_message_type core_message_type;

    if ((hub_client_ptr == NX_NULL) ||
        (reader_ptr == NX_NULL) ||
        (version_ptr == NX_NULL) ||
        ((message_type != NX_AZURE_IOT_HUB_PROPERTIES) && 
         (message_type != NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES)))
    {
        LogError(LogLiteralArgs("IoTHub client get properties version failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    core_message_type = (message_type == NX_AZURE_IOT_HUB_PROPERTIES) ? AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE :
                        AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED;

    core_result = az_iot_hub_client_properties_get_properties_version(&(hub_client_ptr -> iot_hub_client_core),
                                                                      &(reader_ptr -> json_reader),
                                                                      core_message_type,
                                                                      (int32_t *)version_ptr);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client get properties version failed : %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_component_property_next_get_internal(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                                    NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                                                    UINT message_type, UINT property_type,
                                                                                    const UCHAR **component_name_pptr, 
                                                                                    USHORT *component_name_length_ptr,
                                                                                    UINT *component_index,
                                                                                    UINT parse_system_component)
{
az_span component_name;
az_iot_hub_client_properties_message_type core_message_type;
az_iot_hub_client_property_type core_property_type;
az_result core_result;
UINT index;
UINT system_component;

    if (((message_type != NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES) && 
         (message_type != NX_AZURE_IOT_HUB_PROPERTIES)) ||
        ((property_type != NX_AZURE_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE) &&
         (property_type != NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE)) ||
        ((property_type == NX_AZURE_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE) && 
         (message_type != NX_AZURE_IOT_HUB_PROPERTIES)))
    {
        LogError(LogLiteralArgs("Invalid response type or property type passed"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    component_name = az_span_create((UCHAR *)*component_name_pptr, (INT)*component_name_length_ptr);
    core_message_type = (message_type == NX_AZURE_IOT_HUB_PROPERTIES) ? AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE :
                        AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED;
    core_property_type = (property_type == NX_AZURE_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE) ? AZ_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE :
                        AZ_IOT_HUB_CLIENT_PROPERTY_WRITABLE;

    do
    {
        core_result = az_iot_hub_client_properties_get_next_component_property(&(hub_client_ptr -> iot_hub_client_core),
                                                                               &(reader_ptr -> json_reader),
                                                                               core_message_type, core_property_type, 
                                                                               &component_name);
        if (core_result == AZ_ERROR_IOT_END_OF_PROPERTIES)
        {
            return(NX_AZURE_IOT_NOT_FOUND);
        }
        else if (az_result_failed(core_result))
        {
            LogError(LogLiteralArgs("Failed to parse document with core error : %d"), core_result);
            return(NX_AZURE_IOT_SDK_CORE_ERROR);
        }

        *component_name_pptr = az_span_ptr(component_name);
        *component_name_length_ptr = (USHORT)az_span_size(component_name);

        /* Check if it is system component.  */
        system_component = NX_FALSE;
        for (index = 0; index < (UINT)hub_client_ptr -> iot_hub_client_core._internal.options.component_names_length; index++)
        {
            if ((az_span_is_content_equal(component_name, hub_client_ptr -> nx_azure_iot_hub_client_component_list[index]))&&
                (hub_client_ptr -> nx_azure_iot_hub_client_component_callback[index] != NX_NULL))
            {
                system_component = NX_TRUE;
                break;
            }
        }

        /* System component.  */
        if ((parse_system_component == NX_TRUE) && (system_component == NX_TRUE))
        {
            *component_index = index;
            return(NX_AZURE_IOT_SUCCESS);
        }
        else if ((parse_system_component == NX_FALSE) && (system_component == NX_FALSE))
        {
            return(NX_AZURE_IOT_SUCCESS);
        }

        /* Skip it and find the next one.  */
        nx_azure_iot_json_reader_next_token(reader_ptr);
 
        /* Skip children in case the property value is an object.  */
        if (nx_azure_iot_json_reader_token_type(reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            nx_azure_iot_json_reader_skip_children(reader_ptr);
        }
        nx_azure_iot_json_reader_next_token(reader_ptr);

    } while(1);
}

UINT nx_azure_iot_hub_client_properties_component_property_next_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                    NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                                    UINT message_type, UINT property_type,
                                                                    const UCHAR **component_name_pptr,
                                                                    USHORT *component_name_length_ptr)
{

UINT status;

    if ((hub_client_ptr == NX_NULL) ||
        (reader_ptr == NX_NULL) ||
        (component_name_pptr == NX_NULL) ||
        (component_name_length_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client component next property failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_hub_client_properties_component_property_next_get_internal(hub_client_ptr, reader_ptr,
                                                                                     message_type, property_type,
                                                                                     component_name_pptr, component_name_length_ptr,
                                                                                     NX_NULL, NX_FALSE);

    return(status);
}

VOID nx_azure_iot_hub_client_properties_component_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                          NX_PACKET *packet_ptr, UINT message_type)
{

UINT status;
ULONG version;
NX_AZURE_IOT_JSON_READER reader_ptr;
const UCHAR *component_name_ptr = NX_NULL;
USHORT component_name_length = 0;
UINT component_index;
UCHAR *prepend_ptr = NX_NULL;


    /* Check the message type.  */
    if ((message_type != NX_AZURE_IOT_HUB_PROPERTIES) &&
        (message_type != NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES))
    {
        return;
    }

    /* Record the prepend pointer.  */
    prepend_ptr = packet_ptr -> nx_packet_prepend_ptr;

    if (nx_azure_iot_hub_client_adjust_payload(packet_ptr))
    {
        return;
    }

    if (nx_azure_iot_json_reader_init(&reader_ptr, packet_ptr))
    {

        /* Recover the prepend pointer since other functions will retreive the topic again.  */
        packet_ptr -> nx_packet_length += (ULONG)(packet_ptr -> nx_packet_prepend_ptr - prepend_ptr);
        packet_ptr -> nx_packet_prepend_ptr = prepend_ptr;
        return;
    }

    /* Get the version.  */
    status = nx_azure_iot_hub_client_properties_version_get(hub_client_ptr, &reader_ptr,
                                                            message_type, &version);
    if (status)
    {

        /* Recover the prepend pointer since other functions will retreive the topic again.  */
        packet_ptr -> nx_packet_length += (ULONG)(packet_ptr -> nx_packet_prepend_ptr - prepend_ptr);
        packet_ptr -> nx_packet_prepend_ptr = prepend_ptr;
        return;
    }

    /* Re-initialize the JSON reader state */
    if (nx_azure_iot_json_reader_init(&reader_ptr, packet_ptr))
    {

        /* Recover the prepend pointer since other functions will retreive the topic again.  */
        packet_ptr -> nx_packet_length += (ULONG)(packet_ptr -> nx_packet_prepend_ptr - prepend_ptr);
        packet_ptr -> nx_packet_prepend_ptr = prepend_ptr;
        return;
    }

    /* Loop to process system component.  */
    while (nx_azure_iot_hub_client_properties_component_property_next_get_internal(hub_client_ptr,
                                                                                   &reader_ptr, message_type,
                                                                                   NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
                                                                                   &component_name_ptr, &component_name_length,
                                                                                   &component_index, NX_TRUE) == NX_AZURE_IOT_SUCCESS)
    {

        /* Check if it is system component.  */
        if ((component_name_ptr) && (component_name_length) &&
            (hub_client_ptr -> nx_azure_iot_hub_client_component_callback[component_index]))
        {
            status = hub_client_ptr -> nx_azure_iot_hub_client_component_callback[component_index](&reader_ptr, version,
                                                                                                   hub_client_ptr -> nx_azure_iot_hub_client_component_callback_args[component_index]);
            if (status)
            {

                /* Recover the prepend pointer since other functions will retreive the topic again.  */
                packet_ptr -> nx_packet_length += (ULONG)(packet_ptr -> nx_packet_prepend_ptr - prepend_ptr);
                packet_ptr -> nx_packet_prepend_ptr = prepend_ptr;
                return;
            }

        }
        else
        {

            /* The JSON reader must be advanced regardless of whether the property
                is of interest or not.  */
            nx_azure_iot_json_reader_next_token(&reader_ptr);
 
            /* Skip children in case the property value is an object.  */
            if (nx_azure_iot_json_reader_token_type(&reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
            {
                nx_azure_iot_json_reader_skip_children(&reader_ptr);
            }
            nx_azure_iot_json_reader_next_token(&reader_ptr);
        }
    }

    /* Recover the prepend pointer since other functions will retreive the topic again.  */
    packet_ptr -> nx_packet_length += (ULONG)(packet_ptr -> nx_packet_prepend_ptr - prepend_ptr);
    packet_ptr -> nx_packet_prepend_ptr = prepend_ptr;

    return;
}
