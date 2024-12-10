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
#include <stdio.h>
#include <stdarg.h>


#ifndef NX_AZURE_DISABLE_IOT_SECURITY_MODULE
#include "nx_azure_iot_security_module.h"
#endif /* NX_AZURE_DISABLE_IOT_SECURITY_MODULE */

#include "nx_azure_iot.h"

#include "azure/core/internal/az_log_internal.h"

#ifndef NX_AZURE_IOT_WAIT_OPTION
#define NX_AZURE_IOT_WAIT_OPTION NX_WAIT_FOREVER
#endif /* NX_AZURE_IOT_WAIT_OPTION */

/* Convert number to upper hex.  */
#define NX_AZURE_IOT_NUMBER_TO_UPPER_HEX(number)    (CHAR)(number + (number < 10 ? '0' : 'A' - 10))

/* Define the prototypes for Azure RTOS IoT.  */
NX_AZURE_IOT *_nx_azure_iot_created_ptr;

/* Define the callback for logging.  */
static VOID(*_nx_azure_iot_log_callback)(az_log_classification classification, UCHAR *msg, UINT msg_len);

extern UINT _nxd_mqtt_client_publish_packet_send(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr,
                                                 USHORT packet_id, UINT QoS, ULONG wait_option);

static VOID nx_azure_iot_event_process(VOID *nx_azure_iot, ULONG common_events, ULONG module_own_events)
{

NX_AZURE_IOT *nx_azure_iot_ptr = (NX_AZURE_IOT *)nx_azure_iot;

    /* Process DPS events.  */
    if (nx_azure_iot_ptr -> nx_azure_iot_provisioning_client_event_process)
    {
        nx_azure_iot_ptr -> nx_azure_iot_provisioning_client_event_process(nx_azure_iot_ptr, common_events,
                                                                           module_own_events);
    }
}

static UINT nx_azure_iot_publish_packet_header_add(NX_PACKET* packet_ptr, UINT topic_len, UINT qos)
{
UCHAR *buffer_ptr;
UINT length;

    /* Check if packet has enough space to write MQTT header.  */
    if (NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET >
        (packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start))
    {
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    /* Start to fill MQTT header.  */
    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr - NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET;

    /* Set flags.  */
    buffer_ptr[0] = (UCHAR)(MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4);
    if (qos == NX_AZURE_IOT_MQTT_QOS_1)
    {
        buffer_ptr[0] |= MQTT_PUBLISH_QOS_LEVEL_1;
    }

    /* Set topic length.  */
    buffer_ptr[5] = (UCHAR)(topic_len >> 8);
    buffer_ptr[6] = (UCHAR)(topic_len & 0xFF);

    /* Set total length.
     * 2 bytes for topic length.
     * 2 bytes for packet id.
     * data_size for payload.
     *
     * packet already contains topic length, packet id (optional) and data payload
     */
    length = packet_ptr -> nx_packet_length + 2;

    /* Total length is encoded in fixed four bytes format.  */
    buffer_ptr[1] = (UCHAR)((length & 0x7F) | 0x80);
    length >>= 7;
    buffer_ptr[2] = (UCHAR)((length & 0x7F) | 0x80);
    length >>= 7;
    buffer_ptr[3] = (UCHAR)((length & 0x7F) | 0x80);
    length >>= 7;
    buffer_ptr[4] = (UCHAR)(length & 0x7F);

    /* Update packet.  */
    packet_ptr -> nx_packet_prepend_ptr = buffer_ptr;
    packet_ptr -> nx_packet_length += NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_log(UCHAR *type_ptr, UINT type_len, UCHAR *msg_ptr, UINT msg_len, ...)
{
va_list ap;
UCHAR buffer[10] = { 0 };
az_span span;
az_span num_span;

    span = az_span_create(type_ptr, (INT)type_len);
    _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, span);

    if (msg_len >= 2 && (msg_ptr[msg_len - 2] == '%') &&
        ((msg_ptr[msg_len - 1] == 'd') || (msg_ptr[msg_len - 1] == 's')))
    {
        span = az_span_create((UCHAR *)msg_ptr, (INT)(msg_len - 2));
        _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, span);

        va_start(ap, msg_len);

        /* Handles %d  */
        if (msg_ptr[msg_len - 1] == 'd')
        {
            num_span = az_span_create(buffer, sizeof(buffer));
            if (!az_result_failed(az_span_u32toa(num_span, va_arg(ap, UINT), &span)))
            {
                num_span = az_span_slice(num_span, 0, (INT)sizeof(buffer) - az_span_size(span));
                _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, num_span);
            }
        }

        /* Handles %s  */
        if (msg_ptr[msg_len - 1] == 's')
        {
            msg_ptr = va_arg(ap, UCHAR*);
            msg_len = va_arg(ap, UINT);
            span = az_span_create((UCHAR *)msg_ptr, (INT)msg_len);
            _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, span);
        }

        va_end(ap);
    }
    else
    {
        span = az_span_create((UCHAR *)msg_ptr, (INT)msg_len);
        _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, span);
    }

    _az_LOG_WRITE(AZ_LOG_IOT_AZURERTOS, AZ_SPAN_FROM_STR("\r\n"));

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_log_listener(az_log_classification classification, az_span message)
{
    NX_PARAMETER_NOT_USED(classification);

    if (_nx_azure_iot_log_callback != NX_NULL)
    {
        _nx_azure_iot_log_callback(classification, az_span_ptr(message), (UINT)az_span_size(message));
    }
}

VOID nx_azure_iot_log_init(VOID(*log_callback)(az_log_classification classification, UCHAR *msg, UINT msg_len))
{
    _nx_azure_iot_log_callback = log_callback;
    az_log_set_message_callback(nx_azure_iot_log_listener);
}

NX_AZURE_IOT_RESOURCE *nx_azure_iot_resource_search(NXD_MQTT_CLIENT *client_ptr)
{
NX_AZURE_IOT_RESOURCE *resource_ptr;

    /* Check if created Azure RTOS IoT.  */
    if ((_nx_azure_iot_created_ptr == NX_NULL) || (client_ptr == NX_NULL))
    {
        return(NX_NULL);
    }

    /* Loop to find the resource associated with current MQTT client.  */
    for (resource_ptr = _nx_azure_iot_created_ptr -> nx_azure_iot_resource_list_header;
         resource_ptr; resource_ptr = resource_ptr -> resource_next)
    {

        if (&(resource_ptr -> resource_mqtt) == client_ptr)
        {
            return(resource_ptr);
        }
    }

    return(NX_NULL);
}

UINT nx_azure_iot_resource_add(NX_AZURE_IOT *nx_azure_iot_ptr, NX_AZURE_IOT_RESOURCE *resource_ptr)
{

    resource_ptr -> resource_next = nx_azure_iot_ptr -> nx_azure_iot_resource_list_header;
    nx_azure_iot_ptr -> nx_azure_iot_resource_list_header = resource_ptr;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_resource_remove(NX_AZURE_IOT *nx_azure_iot_ptr, NX_AZURE_IOT_RESOURCE *resource_ptr)
{

NX_AZURE_IOT_RESOURCE   *resource_previous;

    if (nx_azure_iot_ptr -> nx_azure_iot_resource_list_header == NX_NULL)
    {
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    if (nx_azure_iot_ptr -> nx_azure_iot_resource_list_header == resource_ptr)
    {
        nx_azure_iot_ptr -> nx_azure_iot_resource_list_header = nx_azure_iot_ptr -> nx_azure_iot_resource_list_header -> resource_next;
        return(NX_AZURE_IOT_SUCCESS);
    }

    for (resource_previous = nx_azure_iot_ptr -> nx_azure_iot_resource_list_header;
         resource_previous -> resource_next;
         resource_previous = resource_previous -> resource_next)
    {
        if (resource_previous -> resource_next == resource_ptr)
        {
            resource_previous -> resource_next = resource_previous -> resource_next -> resource_next;
            return(NX_AZURE_IOT_SUCCESS);
        }
    }

    return(NX_AZURE_IOT_NOT_FOUND);
}

UINT nx_azure_iot_create(NX_AZURE_IOT *nx_azure_iot_ptr, const UCHAR *name_ptr,
                         NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr,
                         VOID *stack_memory_ptr, UINT stack_memory_size,
                         UINT priority, UINT (*unix_time_callback)(ULONG *unix_time))
{
UINT status;

    if ((nx_azure_iot_ptr == NX_NULL) || (ip_ptr == NX_NULL) ||
        (pool_ptr == NX_NULL) || (dns_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoT create fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    nx_azure_iot_ptr -> nx_azure_iot_name = name_ptr;
    nx_azure_iot_ptr -> nx_azure_iot_ip_ptr = ip_ptr;
    nx_azure_iot_ptr -> nx_azure_iot_dns_ptr = dns_ptr;
    nx_azure_iot_ptr -> nx_azure_iot_pool_ptr = pool_ptr;
    nx_azure_iot_ptr -> nx_azure_iot_unix_time_get = unix_time_callback;

    status = nx_cloud_create(&nx_azure_iot_ptr -> nx_azure_iot_cloud, (CHAR *)name_ptr, stack_memory_ptr,
                             stack_memory_size, priority);
    if (status)
    {
        LogError(LogLiteralArgs("IoT create fail status: %d"), status);
        return(status);
    }

    /* Register SDK module on cloud helper.  */
    status = nx_cloud_module_register(&(nx_azure_iot_ptr -> nx_azure_iot_cloud), &(nx_azure_iot_ptr -> nx_azure_iot_cloud_module),
                                      "Azure SDK Module", NX_CLOUD_MODULE_AZURE_SDK_EVENT | NX_CLOUD_COMMON_PERIODIC_EVENT,
                                      nx_azure_iot_event_process, nx_azure_iot_ptr);
    if (status)
    {
        LogError(LogLiteralArgs("IoT module register fail status: %d"), status);
        return(status);
    }

    /* Set the mutex.  */
    nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr = &(nx_azure_iot_ptr -> nx_azure_iot_cloud.nx_cloud_mutex);

    /* Set created IoT pointer.  */
    _nx_azure_iot_created_ptr = nx_azure_iot_ptr;

#ifndef NX_AZURE_DISABLE_IOT_SECURITY_MODULE
    /* Enable Azure IoT Security Module.  */
    status = nx_azure_iot_security_module_enable(nx_azure_iot_ptr);
    if (status)
    {
        LogError(LogLiteralArgs("IoT failed to enable IoT Security Module, status: %d"), status);
        nx_azure_iot_delete(nx_azure_iot_ptr);
        return(status);
    }
#endif /* NX_AZURE_DISABLE_IOT_SECURITY_MODULE */

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_delete(NX_AZURE_IOT *nx_azure_iot_ptr)
{
UINT status;

    if (nx_azure_iot_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoT delete fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (nx_azure_iot_ptr -> nx_azure_iot_resource_list_header)
    {
        LogError(LogLiteralArgs("IoT delete fail: Resource NOT DELETED"));
        return(NX_AZURE_IOT_DELETE_ERROR);
    }

#ifndef NX_AZURE_DISABLE_IOT_SECURITY_MODULE
    /* Disable IoT Security Module.  */
    status = nx_azure_iot_security_module_disable(nx_azure_iot_ptr);
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("IoT failed to disable IoT Security Module, status: %d"), status);
        return(status);
    }
#endif /* NX_AZURE_DISABLE_IOT_SECURITY_MODULE */

    /* Deregister SDK module on cloud helper.  */
    nx_cloud_module_deregister(&(nx_azure_iot_ptr -> nx_azure_iot_cloud), &(nx_azure_iot_ptr -> nx_azure_iot_cloud_module));

    /* Delete cloud.  */
    status = nx_cloud_delete(&nx_azure_iot_ptr -> nx_azure_iot_cloud);
    if (status)
    {
        LogError(LogLiteralArgs("IoT delete fail status: %d"), status);
        return(status);
    }

    _nx_azure_iot_created_ptr = NX_NULL;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_buffer_allocate(NX_AZURE_IOT *nx_azure_iot_ptr, UCHAR **buffer_pptr,
                                  UINT *buffer_size, VOID **buffer_context)
{
NX_PACKET *packet_ptr;
UINT status;

    status = nx_packet_allocate(nx_azure_iot_ptr -> nx_azure_iot_pool_ptr,
                                &packet_ptr, 0, NX_AZURE_IOT_WAIT_OPTION);
    if (status)
    {
        return(status);
    }

    *buffer_pptr = packet_ptr -> nx_packet_data_start;
    *buffer_size = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start);
    *buffer_context = (VOID *)packet_ptr;
    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_buffer_free(VOID *buffer_context)
{
NX_PACKET *packet_ptr = (NX_PACKET *)buffer_context;

    return(nx_packet_release(packet_ptr));
}

UINT nx_azure_iot_publish_packet_get(NX_AZURE_IOT *nx_azure_iot_ptr, NXD_MQTT_CLIENT *client_ptr,
                                     NX_PACKET **packet_pptr, UINT wait_option)
{
UINT status;

    status = nx_secure_tls_packet_allocate(&(client_ptr -> nxd_mqtt_tls_session),
                                           nx_azure_iot_ptr -> nx_azure_iot_pool_ptr,
                                           packet_pptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Create publish packet failed"));
        return(status);
    }

    /* Preserve room for fixed MQTT header.  */
    (*packet_pptr) -> nx_packet_prepend_ptr += NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_publish_mqtt_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr,
                                      UINT topic_len, UCHAR *packet_id, UINT qos, UINT wait_option)
{
UINT status;
USHORT id = 0;

    status = nx_azure_iot_publish_packet_header_add(packet_ptr, topic_len, qos);
    if (status)
    {
        LogError(LogLiteralArgs("failed to add mqtt header"));
        return(status);
    }

    if (qos != 0)
    {
        id = (USHORT)((packet_id[0] << 8) | packet_id[1]);
    }

    /* Note, mutex will be released by this function.  */
    status = _nxd_mqtt_client_publish_packet_send(client_ptr, packet_ptr,
                                                  id, qos, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Mqtt client send fail: PUBLISH FAIL status: %d"), status);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_mqtt_packet_id_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *packet_id)
{
UINT status;

    /* Get packet id under mutex */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(status);
    }

    /* Do nothing if the client is not connected.  */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        LogError(LogLiteralArgs("MQTT NOT CONNECTED"));
        return(NX_AZURE_IOT_DISCONNECTED);
    }

    /* Internal API assuming it to be 2 Byte buffer */
    packet_id[0] = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier >> 8);
    packet_id[1] = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier & 0xFF);

    /* Update packet id.  */
    client_ptr -> nxd_mqtt_client_packet_identifier = (client_ptr -> nxd_mqtt_client_packet_identifier + 1) & 0xFFFF;

    /* Prevent packet identifier from being zero. MQTT-2.3.1-1.  */
    if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
    {
        client_ptr -> nxd_mqtt_client_packet_identifier = 1;
    }

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

VOID nx_azure_iot_mqtt_packet_adjust(NX_PACKET *packet_ptr)
{
UINT size;
UINT copy_size;
NX_PACKET *current_packet_ptr;

    /* Adjust the packet to make sure,
     * 1. nx_packet_prepend_ptr does not pointer to nx_packet_data_start.
     * 2. The first packet is full if it is chained with multiple packets.  */

    if (packet_ptr -> nx_packet_prepend_ptr != packet_ptr -> nx_packet_data_start)
    {

        /* Move data to the nx_packet_data_start.  */
        size = (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
        memmove(packet_ptr -> nx_packet_data_start, packet_ptr -> nx_packet_prepend_ptr, size); /* Use case of memmove is verified.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_data_start + size;
    }

    if (packet_ptr -> nx_packet_next == NX_NULL)
    {

        /* All data are in the first packet.  */
        return;
    }

    /* Move data in the chained packet into first one until it is full.  */
    for (current_packet_ptr = packet_ptr -> nx_packet_next;
         current_packet_ptr;
         current_packet_ptr = packet_ptr -> nx_packet_next)
    {

        /* Calculate remaining buffer size in the first packet.  */
        size = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr);

        /* Calculate copy size from current packet.  */
        copy_size = (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_packet_ptr -> nx_packet_prepend_ptr);

        if (size >= copy_size)
        {

            /* Copy all data from current packet.  */
            memcpy((VOID *)packet_ptr -> nx_packet_append_ptr, (VOID *)current_packet_ptr -> nx_packet_prepend_ptr, copy_size); /* Use case of memcpy is verified.  */
            packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_append_ptr + copy_size;
        }
        else
        {

            /* Copy partial data from current packet.  */
            memcpy(packet_ptr -> nx_packet_append_ptr, current_packet_ptr -> nx_packet_prepend_ptr, size); /* Use case of memcpy is verified.  */
            packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_data_end;

            /* Move data in current packet to nx_packet_data_start.  */
            memmove((VOID *)current_packet_ptr -> nx_packet_data_start, /* Use case of memmove is verified.  */
                    (VOID *)(current_packet_ptr -> nx_packet_prepend_ptr + size),
                    (copy_size - size));
            current_packet_ptr -> nx_packet_prepend_ptr = current_packet_ptr -> nx_packet_data_start;
            current_packet_ptr -> nx_packet_append_ptr = current_packet_ptr -> nx_packet_data_start + (copy_size - size);

            /* First packet is full.  */
            break;
        }

        /* Remove current packet from packet chain.  */
        packet_ptr -> nx_packet_next = current_packet_ptr -> nx_packet_next;

        /* Release current packet.  */
        current_packet_ptr -> nx_packet_next = NX_NULL;
        nx_packet_release(current_packet_ptr);
    }
}

static ULONG nx_azure_iot_certificate_verify(NX_SECURE_TLS_SESSION *session, NX_SECURE_X509_CERT* certificate)
{
NX_AZURE_IOT_RESOURCE *resource_ptr;
UINT old_threshold;
UINT status = NX_AZURE_IOT_SUCCESS;

    /* Disable preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

    /* Check if created Azure RTOS IoT.  */
    if (_nx_azure_iot_created_ptr == NX_NULL)
    {

        /* Restore preemption.  */
        tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
        return(NX_AZURE_IOT_NOT_INITIALIZED);
    }

    /* Get mutex */
    status = tx_mutex_get(_nx_azure_iot_created_ptr -> nx_azure_iot_mutex_ptr, NX_WAIT_FOREVER);

    /* Restore preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
    if (status)
    {
        return(status);
    }

    /* Loop to find the resource associated with current TLS session.  */
    for (resource_ptr = _nx_azure_iot_created_ptr -> nx_azure_iot_resource_list_header;
         resource_ptr; resource_ptr = resource_ptr -> resource_next)
    {

        if (&(resource_ptr -> resource_mqtt.nxd_mqtt_tls_session) == session)
        {
            break;
        }
    }

    if (resource_ptr)
    {

        /* Check DNS entry string.  */
        status = nx_secure_x509_common_name_dns_check(certificate,
                                                      resource_ptr -> resource_hostname,
                                                      resource_ptr -> resource_hostname_length);
        if (status)
        {
            LogError(LogLiteralArgs("Error in certificate verification: DNS name did not match CN"));
        }
    }
    else
    {

        /* TLS session not match.  */
        status = NX_AZURE_IOT_NOT_FOUND;
    }

    /* Release mutex.  */
    tx_mutex_put(_nx_azure_iot_created_ptr -> nx_azure_iot_mutex_ptr);

    return(status);
}

#ifndef NX_AZURE_IOT_DISABLE_CERTIFICATE_DATE
static ULONG nx_azure_iot_tls_time_function(VOID)
{
ULONG unix_time = 0;
UINT old_threshold;

    /* Disable preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

    if (nx_azure_iot_unix_time_get(_nx_azure_iot_created_ptr, &unix_time))
    {

        /* Unable to get Unix time.  */
        LogError(LogLiteralArgs("Unable to get Unix time"));
        unix_time = 0;
    }

    /* Restore preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
    return(unix_time);
}
#endif /* NX_AZURE_IOT_DISABLE_CERTIFICATE_DATE */

UINT nx_azure_iot_mqtt_tls_setup(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                 NX_SECURE_X509_CERT *certificate,
                                 NX_SECURE_X509_CERT *trusted_certificate)
{
UINT status;
UINT i;
NX_AZURE_IOT_RESOURCE *resource_ptr;

    NX_PARAMETER_NOT_USED(certificate);
    NX_PARAMETER_NOT_USED(trusted_certificate);

    /* Obtain the mutex.   */
    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);

    resource_ptr = nx_azure_iot_resource_search(client_ptr);

    /* Release the mutex.  */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    if (resource_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("Failed to find associated resource"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Create TLS session.  */
    status = _nx_secure_tls_session_create_ext(tls_session,
                                               resource_ptr -> resource_crypto_array,
                                               resource_ptr -> resource_crypto_array_size,
                                               resource_ptr -> resource_cipher_map,
                                               resource_ptr -> resource_cipher_map_size,
                                               resource_ptr -> resource_metadata_ptr,
                                               resource_ptr -> resource_metadata_size);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to create TLS session status: %d"), status);
        return(status);
    }
    
    for (i = 0; i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_trusted_certificates); i++)
    {
        if (resource_ptr -> resource_trusted_certificates[i])
        {
            status = nx_secure_tls_trusted_certificate_add(tls_session, resource_ptr -> resource_trusted_certificates[i]);
            if (status)
            {
                LogError(LogLiteralArgs("Failed to add trusted CA certificate to session status: %d"), status);
                return(status);
            }
        }
    }

    for (i = 0; i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_device_certificates); i++)
    {
        if (resource_ptr -> resource_device_certificates[i])
        {
            status = nx_secure_tls_local_certificate_add(tls_session, resource_ptr -> resource_device_certificates[i]);
            if (status)
            {
                LogError(LogLiteralArgs("Failed to add device certificate to session status: %d"), status);
                return(status);
            }
        }
    }

    status = nx_secure_tls_session_packet_buffer_set(tls_session,
                                                     resource_ptr -> resource_tls_packet_buffer,
                                                     sizeof(resource_ptr -> resource_tls_packet_buffer));
    if (status)
    {
        LogError(LogLiteralArgs("Failed to set the session packet buffer: status: %d"), status);
        return(status);
    }

    /* Setup the callback invoked when TLS has a certificate it wants to verify so we can
       do additional checks not done automatically by TLS.  */
    status = nx_secure_tls_session_certificate_callback_set(tls_session,
                                                            nx_azure_iot_certificate_verify);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to set the session certificate callback: status: %d"), status);
        return(status);
    }

#ifndef NX_AZURE_IOT_DISABLE_CERTIFICATE_DATE
    /* Setup the callback function used by checking certificate valid date.  */
    nx_secure_tls_session_time_function_set(tls_session, nx_azure_iot_tls_time_function);
#endif /* NX_AZURE_IOT_DISABLE_CERTIFICATE_DATE */

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_unix_time_get(NX_AZURE_IOT *nx_azure_iot_ptr, ULONG *unix_time)
{

    if ((nx_azure_iot_ptr == NX_NULL) ||
        (nx_azure_iot_ptr -> nx_azure_iot_unix_time_get == NX_NULL) ||
        (unix_time == NX_NULL))
    {
        LogError(LogLiteralArgs("Unix time callback not set"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_ptr -> nx_azure_iot_unix_time_get(unix_time));
}

/* HMAC-SHA256(master key, message ) */
static UINT nx_azure_iot_hmac_sha256_calculate(NX_AZURE_IOT_RESOURCE *resource_ptr, UCHAR *key, UINT key_size,
                                               const UCHAR *message, UINT message_size, UCHAR *output)
{
UINT i;
UINT status;
VOID *handler;
UCHAR *metadata_ptr = resource_ptr -> resource_metadata_ptr;
UINT metadata_size = resource_ptr -> resource_metadata_size;
const NX_CRYPTO_METHOD *hmac_sha_256_crypto_method = NX_NULL;


    /* Find hmac sha256 crypto method.  */
    for(i = 0; i < resource_ptr -> resource_crypto_array_size; i++)
    {
        if(resource_ptr -> resource_crypto_array[i] -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256)
        {
            hmac_sha_256_crypto_method = resource_ptr -> resource_crypto_array[i];
            break;
        }
    }

    /* Check if find the crypto method.  */
    if (hmac_sha_256_crypto_method == NX_NULL)
    {
        return(NX_AZURE_IOT_NO_AVAILABLE_CIPHER);
    }

    /* Initialize.  */
    status = hmac_sha_256_crypto_method -> nx_crypto_init((NX_CRYPTO_METHOD *)hmac_sha_256_crypto_method,
                                                          key, (key_size << 3),
                                                          &handler,
                                                          metadata_ptr,
                                                          metadata_size);
    if (status)
    {
        return(status);
    }

    /* Authenticate.  */
    status = hmac_sha_256_crypto_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                               handler,
                                                               (NX_CRYPTO_METHOD *)hmac_sha_256_crypto_method,
                                                               key,
                                                               (key_size << 3),
                                                               (VOID *)message,
                                                               message_size,
                                                               NX_CRYPTO_NULL,
                                                               output,
                                                               32,
                                                               metadata_ptr,
                                                               metadata_size,
                                                               NX_CRYPTO_NULL,
                                                               NX_CRYPTO_NULL);
    if (status)
    {
        return(status);
    }

    /* Cleanup.  */
    status = hmac_sha_256_crypto_method -> nx_crypto_cleanup(metadata_ptr);

    return(status);
}

UINT nx_azure_iot_base64_hmac_sha256_calculate(NX_AZURE_IOT_RESOURCE *resource_ptr,
                                               const UCHAR *key_ptr, UINT key_size,
                                               const UCHAR *message_ptr, UINT message_size,
                                               UCHAR *buffer_ptr, UINT buffer_len,
                                               UCHAR **output_pptr, UINT *output_len_ptr)
{
UINT status;
UCHAR *hash_buf;
UINT hash_buf_size = 33;
UCHAR *encoded_hash_buf;
UINT encoded_hash_buf_size = 48;
UINT encoded_hash_size;
UINT binary_key_buf_size;

    binary_key_buf_size = buffer_len;
    status = _nx_utility_base64_decode((UCHAR *)key_ptr, key_size,
                                       buffer_ptr, binary_key_buf_size, &binary_key_buf_size);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to base64 decode"));
        return(status);
    }

    buffer_len -= binary_key_buf_size;
    if ((hash_buf_size + encoded_hash_buf_size) > buffer_len)
    {
        LogError(LogLiteralArgs("Failed to not enough memory"));
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    hash_buf = buffer_ptr + binary_key_buf_size;
    status = nx_azure_iot_hmac_sha256_calculate(resource_ptr, buffer_ptr, binary_key_buf_size,
                                                message_ptr, (UINT)message_size, hash_buf);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to get hash256"));
        return(status);
    }

    buffer_len -= hash_buf_size;
    encoded_hash_buf = hash_buf + hash_buf_size;

    /* Additional space is required by encoder.  */
    hash_buf[hash_buf_size - 1] = 0;
    status = _nx_utility_base64_encode(hash_buf, hash_buf_size - 1,
                                       encoded_hash_buf, encoded_hash_buf_size, &encoded_hash_size);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to base64 encode"));
        return(status);
    }

    *output_pptr = encoded_hash_buf;
    *output_len_ptr = encoded_hash_size;

    return(NX_AZURE_IOT_SUCCESS);
}
