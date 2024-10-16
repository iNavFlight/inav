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

#include "nx_azure_iot_hub_client.h"

#include "azure/core/az_version.h"


#define NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE     10
#define NX_AZURE_IOT_HUB_CLIENT_EMPTY_JSON              "{}"
#define NX_AZURE_IOT_HUB_CLIENT_THROTTLE_STATUS_CODE    429

#ifndef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT
#ifndef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_INTERFACE_TYPE
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_INTERFACE_TYPE NX_INTERFACE_TYPE_UNKNOWN
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_UPDATE
#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_INTERFACE_TYPE */

#ifndef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_VENDOR
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_VENDOR 0
#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_VENDOR */

#ifndef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_TYPE
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_TYPE "U"
#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_TYPE */

/* Useragent e.g: azsdk-c%2F1.3.0%20%28azrtos%206.1.2%3B0%3B0%3BU%29 */
#define NX_AZURE_IOT_HUB_CLIENT_STR(C)          #C
#define NX_AZURE_IOT_HUB_CLIENT_TO_STR(x)       NX_AZURE_IOT_HUB_CLIENT_STR(x)
#define NX_AZURE_IOT_HUB_CLIENT_USER_AGENT      "azsdk-c%2F" AZ_SDK_VERSION_STRING "%20%28azrtos%20" \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(THREADX_MAJOR_VERSION) "." \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(THREADX_MINOR_VERSION) "." \
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(THREADX_PATCH_VERSION) "%3B"\
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_INTERFACE_TYPE) "%3B"\
                                                NX_AZURE_IOT_HUB_CLIENT_TO_STR(NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_VENDOR) "%3B"\
                                                NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_DEVICE_TYPE "%29"
static UCHAR _nx_azure_iot_hub_client_user_agent[] = NX_AZURE_IOT_HUB_CLIENT_USER_AGENT;

#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT */

#define NX_AZURE_IOT_HUB_CLIENT_COMPONENT_STRING        "$.sub"

#define NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH         "/$iothub/websocket"

static VOID nx_azure_iot_hub_client_received_message_cleanup(NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE *message);
static UINT nx_azure_iot_hub_client_cloud_message_sub_unsub(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT is_subscribe);
static UINT nx_azure_iot_hub_client_process_publish_packet(UCHAR *start_ptr,
                                                           ULONG *topic_offset_ptr,
                                                           USHORT *topic_length_ptr);
static VOID nx_azure_iot_hub_client_mqtt_receive_callback(NXD_MQTT_CLIENT* client_ptr,
                                                          UINT number_of_messages);
static UINT nx_azure_iot_hub_client_c2d_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_PACKET *packet_ptr,
                                                ULONG topic_offset,
                                                USHORT topic_length);
static UINT nx_azure_iot_hub_client_properties_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                       NX_PACKET *packet_ptr,
                                                       ULONG topic_offset,
                                                       USHORT topic_length);
static UINT nx_azure_iot_hub_client_device_twin_parse(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET *packet_ptr, ULONG topic_offset,
                                                      USHORT topic_length, UINT *request_id_ptr,
                                                      ULONG *version_ptr, UINT *message_type_ptr,
                                                      UINT *status_ptr);
extern UINT _nxd_mqtt_process_publish_packet(NX_PACKET *packet_ptr, ULONG *topic_offset_ptr,
                                             USHORT *topic_length_ptr, ULONG *message_offset_ptr,
                                             ULONG *message_length_ptr);
static VOID nx_azure_iot_hub_client_mqtt_connect_notify(struct NXD_MQTT_CLIENT_STRUCT *client_ptr,
                                                        UINT status, VOID *context);
static VOID nx_azure_iot_hub_client_mqtt_disconnect_notify(NXD_MQTT_CLIENT *client_ptr);
static VOID nx_azure_iot_hub_client_mqtt_ack_receive_notify(NXD_MQTT_CLIENT *client_ptr, UINT type,
                                                            USHORT packet_id, NX_PACKET *transmit_packet_ptr,
                                                            VOID *context);
static VOID nx_azure_iot_hub_client_thread_dequeue(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_AZURE_IOT_THREAD *thread_list_ptr);
static UINT nx_azure_iot_hub_client_sas_token_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  ULONG expiry_time_secs, const UCHAR *key, UINT key_len,
                                                  UCHAR *sas_buffer, UINT sas_buffer_len, UINT *sas_length);
static UINT nx_azure_iot_hub_client_messages_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
UINT nx_azure_iot_hub_client_adjust_payload(NX_PACKET *packet_ptr);
UINT nx_azure_iot_hub_client_component_add_internal(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                    const UCHAR *component_name_ptr,
                                                    USHORT component_name_length,
                                                    UINT (*callback_ptr)(VOID *json_reader_ptr,
                                                                        ULONG version,
                                                                        VOID *args),
                                                    VOID *callback_args);

static UINT nx_azure_iot_hub_client_throttle_with_jitter(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT jitter;
UINT base_delay = NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC;
UINT retry_count = hub_client_ptr -> nx_azure_iot_hub_client_throttle_count;
uint64_t delay;

    if (retry_count < (sizeof(UINT) * 8 - 1))
    {
        retry_count++;
        delay = (uint64_t)((1 << retry_count) * NX_AZURE_IOT_HUB_CLIENT_INITIAL_BACKOFF_IN_SEC);

        if (delay <= (UINT)(-1))
        {
            base_delay = (UINT)delay;
        }
    }

    if (base_delay > NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC)
    {
        base_delay = NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_IN_SEC;
    }
    else
    {
       hub_client_ptr -> nx_azure_iot_hub_client_throttle_count = retry_count;
    }

    jitter = base_delay * NX_AZURE_IOT_HUB_CLIENT_MAX_BACKOFF_JITTER_PERCENT * (NX_RAND() & 0xFF) / 25600;
    return((UINT)(base_delay + jitter));
}

static UINT nx_azure_iot_hub_client_throttled_check(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
ULONG current_time;
UINT status = NX_AZURE_IOT_SUCCESS;

    if (hub_client_ptr -> nx_azure_iot_hub_client_throttle_count != 0)
    {
        if ((status = nx_azure_iot_unix_time_get(hub_client_ptr -> nx_azure_iot_ptr, &current_time)))
        {
            LogError(LogLiteralArgs("IoTHub client fail to get unix time: %d"), status);
            return(status);
        }

        if (current_time < hub_client_ptr -> nx_azure_iot_hub_client_throttle_end_time)
        {
            return(NX_AZURE_IOT_THROTTLED);
        }
    }

    return(status);
}

UINT nx_azure_iot_hub_client_initialize(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr,
                                        NX_AZURE_IOT *nx_azure_iot_ptr,
                                        const UCHAR *host_name, UINT host_name_length,
                                        const UCHAR *device_id, UINT device_id_length,
                                        const UCHAR *module_id, UINT module_id_length,
                                        const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                        const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                        UCHAR * metadata_memory, UINT memory_size,
                                        NX_SECURE_X509_CERT *trusted_certificate)
{

UINT status;
NX_AZURE_IOT_RESOURCE *resource_ptr;
az_span hostname_span = az_span_create((UCHAR *)host_name, (INT)host_name_length);
az_span device_id_span = az_span_create((UCHAR *)device_id, (INT)device_id_length);
az_iot_hub_client_options options = az_iot_hub_client_options_default();
az_result core_result;

    if ((nx_azure_iot_ptr == NX_NULL) || (hub_client_ptr == NX_NULL) || (host_name == NX_NULL) ||
        (device_id == NX_NULL) || (host_name_length == 0) || (device_id_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub client initialization fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    memset(hub_client_ptr, 0, sizeof(NX_AZURE_IOT_HUB_CLIENT));

    hub_client_ptr -> nx_azure_iot_ptr = nx_azure_iot_ptr;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_crypto_array = crypto_array;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_crypto_array_size = crypto_array_size;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_cipher_map = cipher_map;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_cipher_map_size = cipher_map_size;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_metadata_ptr = metadata_memory;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_metadata_size = memory_size;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_trusted_certificates[0] = trusted_certificate;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_hostname = host_name;
    hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_hostname_length = host_name_length;
    options.module_id = az_span_create((UCHAR *)module_id, (INT)module_id_length);
    options.user_agent = AZ_SPAN_FROM_STR(NX_AZURE_IOT_HUB_CLIENT_USER_AGENT);
    options.component_names = hub_client_ptr -> nx_azure_iot_hub_client_component_list;
    options.component_names_length = 0;

    core_result = az_iot_hub_client_init(&hub_client_ptr -> iot_hub_client_core,
                                         hostname_span, device_id_span, &options);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client failed initialization with error status: %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    /* Set resource pointer.  */
    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);

    /* Create MQTT client.  */
    status = _nxd_mqtt_client_cloud_create(&(resource_ptr -> resource_mqtt),
                                           (CHAR *)nx_azure_iot_ptr -> nx_azure_iot_name,
                                           "", 0,
                                           nx_azure_iot_ptr -> nx_azure_iot_ip_ptr,
                                           nx_azure_iot_ptr -> nx_azure_iot_pool_ptr,
                                           &nx_azure_iot_ptr -> nx_azure_iot_cloud);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client initialization fail: MQTT CLIENT CREATE FAIL status: %d"), status);
        return(status);
    }

    /* Set mqtt receive notify.  */
    status = nxd_mqtt_client_receive_notify_set(&(resource_ptr -> resource_mqtt),
                                                nx_azure_iot_hub_client_mqtt_receive_callback);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client set message callback status: %d"), status);
        nxd_mqtt_client_delete(&(resource_ptr -> resource_mqtt));
        return(status);
    }

    /* Obtain the mutex.   */
    tx_mutex_get(nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Link the resource.  */
    resource_ptr -> resource_data_ptr = (VOID *)hub_client_ptr;
    resource_ptr -> resource_type = NX_AZURE_IOT_RESOURCE_IOT_HUB;
    nx_azure_iot_resource_add(nx_azure_iot_ptr, resource_ptr);

    /* Release the mutex.  */
    tx_mutex_put(nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_connection_status_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            VOID (*connection_status_cb)(
                                                                  struct NX_AZURE_IOT_HUB_CLIENT_STRUCT *client_ptr,
                                                                  UINT status))
{

    /* Check for invalid input pointers.  */
    if ((hub_client_ptr == NX_NULL) || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client connect callback fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Set callback function for disconnection.  */
    nxd_mqtt_client_disconnect_notify_set(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                          nx_azure_iot_hub_client_mqtt_disconnect_notify);

    /* Obtain the mutex.   */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Set connection status callback.  */
    hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback = connection_status_cb;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    /* Return success.  */
    return(NX_AZURE_IOT_SUCCESS);

}

#ifdef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_UPDATE
static VOID nx_azure_iot_hub_client_user_agent_update(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, NXD_ADDRESS *server_address)
{
NX_IP        *ip_ptr = hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_ip_ptr;
NX_INTERFACE *outgoing_interface = NX_NULL;
UINT          status;
#ifndef NX_DISABLE_IPV4
ULONG         next_hop_address;
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
NXD_IPV6_ADDRESS *ipv6_addr;
#endif /* FEATURE_NX_IPV6 */
ULONG         interface_type;
ULONG         index;

    /* Make sure the server IP address is accesible. */
#ifndef NX_DISABLE_IPV4
    if (server_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        if (_nx_ip_route_find(ip_ptr, server_address -> nxd_ip_address.v4, &outgoing_interface, &next_hop_address) != NX_SUCCESS)
        {
            return;
        }
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* For IPv6 connections, find a suitable outgoing interface, based on the TCP peer address. */
    if (server_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        status = _nxd_ipv6_interface_find(ip_ptr, server_address -> nxd_ip_address.v6,
                                          &ipv6_addr,
                                          NX_NULL);

        if (status != NX_SUCCESS)
        {
            return;
        }

        outgoing_interface = ipv6_addr -> nxd_ipv6_address_attached;
    }
#endif /* FEATURE_NX_IPV6 */

    /* Check if found the outgoing interface.  */
    if (outgoing_interface == NX_NULL)
    {
        return;
    }

    /* Get the interface type.  */
    status = nx_ip_driver_interface_direct_command(ip_ptr, NX_LINK_GET_INTERFACE_TYPE, outgoing_interface -> nx_interface_index, &interface_type);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Check if the interface type is listed in nx_api.h.  */
    if (interface_type > NX_INTERFACE_TYPE_LORAWAN)
    {
        interface_type = NX_INTERFACE_TYPE_OTHER;
    }

    /* Update the interface type in user agent.  */
    for (index = 0; index < sizeof(_nx_azure_iot_hub_client_user_agent) - 3; index++)
    {

        /* The interface type is after the first semicolon.  */
        if ((_nx_azure_iot_hub_client_user_agent[index] == '%') &&
            (_nx_azure_iot_hub_client_user_agent[index + 1] == '3') &&
            (_nx_azure_iot_hub_client_user_agent[index + 2] == 'B'))
        {
            _nx_azure_iot_hub_client_user_agent[index + 3] = (UCHAR)(interface_type + '0');
            hub_client_ptr -> iot_hub_client_core._internal.options.user_agent = AZ_SPAN_FROM_BUFFER(_nx_azure_iot_hub_client_user_agent);
            return;
        }
    }
}
#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_UPDATE */

#ifdef NXD_MQTT_OVER_WEBSOCKET
UINT nx_azure_iot_hub_client_websocket_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
NX_AZURE_IOT_RESOURCE *resource_ptr;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub WebSocket enable fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Set resource pointer.  */
    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);

    return(nxd_mqtt_client_websocket_set(&(resource_ptr -> resource_mqtt), (UCHAR *)resource_ptr -> resource_hostname, resource_ptr -> resource_hostname_length, 
                                         (UCHAR *)NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH, sizeof(NX_AZURE_IOT_HUB_CLIENT_WEB_SOCKET_PATH) - 1));
}
#endif /* NXD_MQTT_OVER_WEBSOCKET */

UINT nx_azure_iot_hub_client_connect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                     UINT clean_session, UINT wait_option)
{
UINT            status;
NXD_ADDRESS     server_address;
NX_AZURE_IOT_RESOURCE *resource_ptr;
NXD_MQTT_CLIENT *mqtt_client_ptr;
UCHAR           *buffer_ptr;
UINT            buffer_size;
VOID            *buffer_context;
UINT            buffer_length;
ULONG           expiry_time_secs;
az_result       core_result;
UINT            server_port;

    /* Check for invalid input pointers.  */
    if ((hub_client_ptr == NX_NULL) || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client connect fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Check for status.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_state == NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        LogError(LogLiteralArgs("IoTHub client already connected"));
        return(NX_AZURE_IOT_ALREADY_CONNECTED);
    }
    else if (hub_client_ptr -> nx_azure_iot_hub_client_state == NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTING)
    {
        LogError(LogLiteralArgs("IoTHub client is connecting"));
        return(NX_AZURE_IOT_CONNECTING);
    }

    /* Resolve the host name.  */
    /* Note: always using default dns timeout.  */
    status = nxd_dns_host_by_name_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_dns_ptr,
                                      (UCHAR *)hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_hostname,
                                      &server_address, NX_AZURE_IOT_HUB_CLIENT_DNS_TIMEOUT, NX_IP_VERSION_V4);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client connect fail: DNS RESOLVE FAIL status: %d"), status);
        return(status);
    }

    /* Allocate buffer for client id, username and sas token.  */
    status = nx_azure_iot_buffer_allocate(hub_client_ptr -> nx_azure_iot_ptr,
                                          &buffer_ptr, &buffer_size, &buffer_context);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client failed initialization: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    /* Obtain the mutex.   */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

#ifdef NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_UPDATE
    /* Add the real interface type into the user agent string.  */
    nx_azure_iot_hub_client_user_agent_update(hub_client_ptr, &server_address);
#endif /* NX_AZURE_IOT_HUB_CLIENT_USER_AGENT_UPDATE */

    /* Set resource pointer and buffer context.  */
    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);

    /* Build client id.  */
    buffer_length = buffer_size;
    core_result = az_iot_hub_client_get_client_id(&(hub_client_ptr -> iot_hub_client_core),
                                                  (CHAR *)buffer_ptr, buffer_length, (size_t *)&buffer_length);
    if (az_result_failed(core_result))
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        nx_azure_iot_buffer_free(buffer_context);
        LogError(LogLiteralArgs("IoTHub client failed to get clientId with error status: "), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }
    resource_ptr -> resource_mqtt_client_id = buffer_ptr;
    resource_ptr -> resource_mqtt_client_id_length = buffer_length;

    /* Update buffer for user name.  */
    buffer_ptr += resource_ptr -> resource_mqtt_client_id_length;
    buffer_size -= resource_ptr -> resource_mqtt_client_id_length;

    /* Build user name.  */
    buffer_length = buffer_size;
    core_result = az_iot_hub_client_get_user_name(&hub_client_ptr -> iot_hub_client_core,
                                                  (CHAR *)buffer_ptr, buffer_length, (size_t *)&buffer_length);
    if (az_result_failed(core_result))
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        nx_azure_iot_buffer_free(buffer_context);
        LogError(LogLiteralArgs("IoTHub client connect fail, with error status: %d"), core_result);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }
    resource_ptr -> resource_mqtt_user_name = buffer_ptr;
    resource_ptr -> resource_mqtt_user_name_length = buffer_length;

    /* Build sas token.  */
    resource_ptr -> resource_mqtt_sas_token = buffer_ptr + buffer_length;
    resource_ptr -> resource_mqtt_sas_token_length = buffer_size - buffer_length;

    /* Check if token refersh is setup.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_token_refresh)
    {
        status = nx_azure_iot_unix_time_get(hub_client_ptr -> nx_azure_iot_ptr, &expiry_time_secs);
        if (status)
        {

            /* Release the mutex.  */
            tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
            nx_azure_iot_buffer_free(buffer_context);
            LogError(LogLiteralArgs("IoTHub client connect fail: unixtime get failed status: %d"), status);
            return(status);
        }

        expiry_time_secs += NX_AZURE_IOT_HUB_CLIENT_TOKEN_CONNECTION_TIMEOUT;
        status = hub_client_ptr -> nx_azure_iot_hub_client_token_refresh(hub_client_ptr,
                                                                         expiry_time_secs,
                                                                         hub_client_ptr -> nx_azure_iot_hub_client_symmetric_key,
                                                                         hub_client_ptr -> nx_azure_iot_hub_client_symmetric_key_length,
                                                                         resource_ptr -> resource_mqtt_sas_token,
                                                                         resource_ptr -> resource_mqtt_sas_token_length,
                                                                         &(resource_ptr -> resource_mqtt_sas_token_length));
        if (status)
        {

            /* Release the mutex.  */
            tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
            nx_azure_iot_buffer_free(buffer_context);
            LogError(LogLiteralArgs("IoTHub client connect fail: Token generation failed status: %d"), status);
            return(status);
        }
        hub_client_ptr -> nx_azure_iot_hub_client_sas_token_expiry_time = expiry_time_secs;
    }
    else
    {
        resource_ptr ->  resource_mqtt_sas_token_length = 0;
    }

    /* Set azure IoT and MQTT client.  */
    mqtt_client_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt);

    /* Update client id.  */
    mqtt_client_ptr -> nxd_mqtt_client_id = (CHAR *)resource_ptr -> resource_mqtt_client_id;
    mqtt_client_ptr -> nxd_mqtt_client_id_length = resource_ptr -> resource_mqtt_client_id_length;

    /* Set login info.  */
    status = nxd_mqtt_client_login_set(&(resource_ptr -> resource_mqtt),
                                       (CHAR *)resource_ptr -> resource_mqtt_user_name,
                                       resource_ptr -> resource_mqtt_user_name_length,
                                       (CHAR *)resource_ptr -> resource_mqtt_sas_token,
                                       resource_ptr -> resource_mqtt_sas_token_length);
    if (status)
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        nx_azure_iot_buffer_free(buffer_context);
        LogError(LogLiteralArgs("IoTHub client connect fail: MQTT CLIENT LOGIN SET FAIL status: %d"), status);
        return(status);
    }

    /* Set connect notify for non-blocking mode.  */
    if (wait_option == 0)
    {
        mqtt_client_ptr -> nxd_mqtt_connect_notify = nx_azure_iot_hub_client_mqtt_connect_notify;
        mqtt_client_ptr -> nxd_mqtt_connect_context = hub_client_ptr;
    }

    /* Save the resource buffer.  */
    resource_ptr -> resource_mqtt_buffer_context = buffer_context;
    resource_ptr -> resource_mqtt_buffer_size = buffer_size;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (mqtt_client_ptr -> nxd_mqtt_client_use_websocket == NX_TRUE)
    {
        server_port = NXD_MQTT_OVER_WEBSOCKET_TLS_PORT;
    }
    else
    {
        server_port = NXD_MQTT_TLS_PORT;
    }
#else
    server_port = NXD_MQTT_TLS_PORT;
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Start MQTT connection.  */
    status = nxd_mqtt_client_secure_connect(mqtt_client_ptr, &server_address, server_port,
                                            nx_azure_iot_mqtt_tls_setup, NX_AZURE_IOT_MQTT_KEEP_ALIVE,
                                            clean_session, wait_option);

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Check status for non-blocking mode.  */
    if ((wait_option == 0) && (status == NX_IN_PROGRESS))
    {
        hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTING;

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        /* Return in-progress completion status.  */
        return(NX_AZURE_IOT_CONNECTING);
    }

    /* Release the mqtt connection resource.  */
    if (resource_ptr -> resource_mqtt_buffer_context)
    {
        nx_azure_iot_buffer_free(resource_ptr -> resource_mqtt_buffer_context);
        resource_ptr -> resource_mqtt_buffer_context = NX_NULL;
    }

    /* Check status.  */
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;
        LogError(LogLiteralArgs("IoTHub client connect fail: MQTT CONNECT FAIL status: %d"), status);
    }
    else
    {

        /* Connected to IoT Hub.  */
        hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED;
    }

    if (status == NX_AZURE_IOT_SUCCESS)
    {
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        status = nx_azure_iot_hub_client_messages_enable(hub_client_ptr);

        tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

        if (status)
        {
            hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;
            LogError(LogLiteralArgs("IoTHub client connect fail: MQTT SUBSCRIBE FAIL status: %d"), status);
        }
    }

    /* Call connection notify if it is set.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback(hub_client_ptr, status);
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(status);
}

static VOID nx_azure_iot_hub_client_mqtt_connect_notify(struct NXD_MQTT_CLIENT_STRUCT *client_ptr,
                                                        UINT status, VOID *context)
{

NX_AZURE_IOT_HUB_CLIENT *iot_hub_client = (NX_AZURE_IOT_HUB_CLIENT*)context;


    NX_PARAMETER_NOT_USED(client_ptr);

    /* Obtain the mutex.  */
    tx_mutex_get(iot_hub_client -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Release the mqtt connection resource.  */
    if (iot_hub_client -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context)
    {
        nx_azure_iot_buffer_free(iot_hub_client -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context);
        iot_hub_client -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context = NX_NULL;
    }

    /* Update hub client status.  */
    if (status == NXD_MQTT_SUCCESS)
    {
        iot_hub_client -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED;
    }
    else
    {
        iot_hub_client -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;
    }

    if (status == NXD_MQTT_SUCCESS)
    {
        tx_mutex_put(iot_hub_client -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        status = nx_azure_iot_hub_client_messages_enable(iot_hub_client);

        tx_mutex_get(iot_hub_client -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

        if (status)
        {
            iot_hub_client -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;
            LogError(LogLiteralArgs("IoTHub client connect fail: MQTT SUBSCRIBE FAIL status: %d"), status);
        }
    }

    /* Call connection notify if it is set.  */
    if (iot_hub_client -> nx_azure_iot_hub_client_connection_status_callback)
    {
        iot_hub_client -> nx_azure_iot_hub_client_connection_status_callback(iot_hub_client, status);
    }

    /* Release the mutex.  */
    tx_mutex_put(iot_hub_client -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
}

static VOID nx_azure_iot_hub_client_mqtt_disconnect_notify(NXD_MQTT_CLIENT *client_ptr)
{
NX_AZURE_IOT_RESOURCE *resource = nx_azure_iot_resource_search(client_ptr);
NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = NX_NULL;
NX_AZURE_IOT_THREAD *thread_list_ptr;
ULONG current_time = 0;

    /* This function is protected by MQTT mutex.  */

    if (resource && (resource -> resource_type == NX_AZURE_IOT_RESOURCE_IOT_HUB))
    {
        hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)resource -> resource_data_ptr;
    }

    if (hub_client_ptr == NX_NULL)
    {
        return;
    }

    /* Wake up all threads.  */
    for (thread_list_ptr = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
         thread_list_ptr;
         thread_list_ptr = thread_list_ptr -> thread_next)
    {
        tx_thread_wait_abort(thread_list_ptr -> thread_ptr);
    }

    /* Do not call callback if not connected, as at our layer connected means : mqtt connect + subscribe messages topic.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_state == NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;

        /* Call connection notify if it is set.  */
        if (hub_client_ptr && hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback)
        {
            if (hub_client_ptr -> nx_azure_iot_hub_client_token_refresh &&
                (nx_azure_iot_unix_time_get(hub_client_ptr -> nx_azure_iot_ptr,
                                            &current_time) == NX_AZURE_IOT_SUCCESS) &&
                (current_time > hub_client_ptr -> nx_azure_iot_hub_client_sas_token_expiry_time))
            {
                hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback(hub_client_ptr,
                                                                                     NX_AZURE_IOT_SAS_TOKEN_EXPIRED);
            }
            else
            {
                hub_client_ptr -> nx_azure_iot_hub_client_connection_status_callback(hub_client_ptr,
                                                                                     NX_AZURE_IOT_DISCONNECTED);
            }
            
        }
    }
}

UINT nx_azure_iot_hub_client_disconnect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;
NX_AZURE_IOT_THREAD *thread_list_ptr;


    /* Check for invalid input pointers.  */
    if ((hub_client_ptr == NX_NULL) || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client disconnect fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Disconnect.  */
    status = nxd_mqtt_client_disconnect(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt));
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client disconnect fail status: %d"), status);
        return(status);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Release the mqtt connection resource.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context)
    {
        nx_azure_iot_buffer_free(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context);
        hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt_buffer_context = NX_NULL;
    }

    /* Wakeup all suspend threads.  */
    for (thread_list_ptr = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
         thread_list_ptr;
         thread_list_ptr = thread_list_ptr -> thread_next)
    {
        tx_thread_wait_abort(thread_list_ptr -> thread_ptr);
    }

    /* Cleanup received messages.  */
    nx_azure_iot_hub_client_received_message_cleanup(&(hub_client_ptr -> nx_azure_iot_hub_client_c2d_message));
    nx_azure_iot_hub_client_received_message_cleanup(&(hub_client_ptr -> nx_azure_iot_hub_client_properties_message));
    nx_azure_iot_hub_client_received_message_cleanup(&(hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message));
    nx_azure_iot_hub_client_received_message_cleanup(&(hub_client_ptr -> nx_azure_iot_hub_client_command_message));

    hub_client_ptr -> nx_azure_iot_hub_client_state = NX_AZURE_IOT_HUB_CLIENT_STATUS_NOT_CONNECTED;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_hub_client_received_message_cleanup(NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE *message)
{
NX_PACKET *current_ptr;
NX_PACKET *next_ptr;

    for (current_ptr = message -> message_head; current_ptr; current_ptr = next_ptr)
    {

        /* Get next packet in queue.  */
        next_ptr = current_ptr -> nx_packet_queue_next;

        /* Release current packet.  */
        current_ptr -> nx_packet_queue_next = NX_NULL;
        nx_packet_release(current_ptr);
    }

    /* Reset received messages.  */
    message -> message_head = NX_NULL;
    message -> message_tail = NX_NULL;
}

static UINT nx_azure_iot_hub_client_messages_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = NX_AZURE_IOT_SUCCESS;

    if (status == NX_AZURE_IOT_SUCCESS &&
        (hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process != NX_NULL))
    {
        status = hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_enable(hub_client_ptr);
    }

    if (status == NX_AZURE_IOT_SUCCESS &&
        (hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_process != NX_NULL))
    {
        status = hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_enable(hub_client_ptr);
    }

    if (status == NX_AZURE_IOT_SUCCESS &&
        (hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process != NX_NULL))
    {
        status = hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_enable(hub_client_ptr);
    }

    return(status);
}

UINT nx_azure_iot_hub_client_deinitialize(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((hub_client_ptr == NX_NULL) || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client deinitialize fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    nx_azure_iot_hub_client_disconnect(hub_client_ptr);

    status = nxd_mqtt_client_delete(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt));
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client delete fail status: %d"), status);
        return(status);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Remove resource from list.  */
    status = nx_azure_iot_resource_remove(hub_client_ptr -> nx_azure_iot_ptr,
                                          &(hub_client_ptr -> nx_azure_iot_hub_client_resource));
    if (status)
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        LogError(LogLiteralArgs("IoTHub client handle not found"));
        return(status);
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_trusted_cert_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                              NX_SECURE_X509_CERT *trusted_certificate)
{
UINT i;
NX_AZURE_IOT_RESOURCE *resource_ptr;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (trusted_certificate == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub device certificate set fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);
    for (i = 0; i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_trusted_certificates); i++)
    {
        if (resource_ptr -> resource_trusted_certificates[i] == NX_NULL)
        {
            resource_ptr -> resource_trusted_certificates[i] = trusted_certificate;
            break;
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    if (i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_trusted_certificates))
    {
        return(NX_AZURE_IOT_SUCCESS);
    }
    else
    {

        /* No more space to store trusted certificate.  */
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }
}

UINT nx_azure_iot_hub_client_device_cert_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                             NX_SECURE_X509_CERT *device_certificate)
{
UINT i;
NX_AZURE_IOT_RESOURCE *resource_ptr;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (device_certificate == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub device certificate set fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    resource_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource);
    for (i = 0; i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_device_certificates); i++)
    {
        if (resource_ptr -> resource_device_certificates[i] == NX_NULL)
        {
            resource_ptr -> resource_device_certificates[i] = device_certificate;
            break;
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    if (i < NX_AZURE_IOT_ARRAY_SIZE(resource_ptr -> resource_device_certificates))
    {
        return(NX_AZURE_IOT_SUCCESS);
    }
    else
    {

        /* No more space to store device certificate.  */
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }
}

UINT nx_azure_iot_hub_client_symmetric_key_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                               const UCHAR *symmetric_key, UINT symmetric_key_length)
{
    if ((hub_client_ptr == NX_NULL)  || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (symmetric_key == NX_NULL) || (symmetric_key_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub client symmetric key fail: Invalid argument"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    hub_client_ptr -> nx_azure_iot_hub_client_symmetric_key = symmetric_key;
    hub_client_ptr -> nx_azure_iot_hub_client_symmetric_key_length = symmetric_key_length;

    hub_client_ptr -> nx_azure_iot_hub_client_token_refresh = nx_azure_iot_hub_client_sas_token_get;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_model_id_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                          const UCHAR *model_id_ptr, UINT model_id_length)
{
    if ((hub_client_ptr == NX_NULL)  || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (model_id_ptr == NX_NULL) || (model_id_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub client model Id fail: Invalid argument"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Had no way to update option, so had to access the internal fields of iot_hub_client_core.  */
    hub_client_ptr -> iot_hub_client_core._internal.options.model_id =
        az_span_create((UCHAR *)model_id_ptr, (INT)model_id_length);

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_component_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                           const UCHAR *component_name_ptr,
                                           USHORT component_name_length)
{
    return(nx_azure_iot_hub_client_component_add_internal(hub_client_ptr, component_name_ptr, component_name_length, NX_NULL, NX_NULL));
}

UINT nx_azure_iot_hub_client_component_add_internal(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                    const UCHAR *component_name_ptr,
                                                    USHORT component_name_length,
                                                    UINT (*callback_ptr)(VOID *json_reader_ptr,
                                                                         ULONG version,
                                                                         VOID *args),
                                                    VOID *callback_args)
{
UINT length_of_componet_list;

    if ((hub_client_ptr == NX_NULL) ||
        (component_name_ptr == NX_NULL) ||
        (component_name_length == NX_NULL))
    {
        LogError(LogLiteralArgs("IoT Hub add component fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    length_of_componet_list =
        (UINT)hub_client_ptr -> iot_hub_client_core._internal.options.component_names_length;

    if (length_of_componet_list >= NX_AZURE_IOT_HUB_CLIENT_MAX_COMPONENT_LIST)
    {
        LogError(LogLiteralArgs("IoT Hub fail due to buffer insufficient"));
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    /* Using internal fields for faster update */
    hub_client_ptr -> nx_azure_iot_hub_client_component_list[length_of_componet_list] =
        az_span_create((UCHAR *)component_name_ptr, (INT)component_name_length);
    hub_client_ptr -> iot_hub_client_core._internal.options.component_names_length++;
    hub_client_ptr -> nx_azure_iot_hub_client_component_callback[length_of_componet_list] = callback_ptr;
    hub_client_ptr -> nx_azure_iot_hub_client_component_callback_args[length_of_componet_list] = callback_args;

    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_telemetry_message_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET **packet_pptr, UINT wait_option)
{
NX_PACKET *packet_ptr;
UINT topic_length;
UINT status;
az_result core_result;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub telemetry message create fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_publish_packet_get(hub_client_ptr -> nx_azure_iot_ptr,
                                             &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Create telemetry data fail"));
        return(status);
    }

    topic_length = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr);
    core_result = az_iot_hub_client_telemetry_get_publish_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                NULL, (CHAR *)packet_ptr -> nx_packet_prepend_ptr,
                                                                topic_length, (size_t *)&topic_length);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client telemetry message create fail with error status: %d"), core_result);
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + topic_length;
    packet_ptr -> nx_packet_length = topic_length;
    *packet_pptr = packet_ptr;
    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_telemetry_message_delete(NX_PACKET *packet_ptr)
{
    return(nx_packet_release(packet_ptr));
}

UINT nx_azure_iot_hub_client_telemetry_component_set(NX_PACKET *packet_ptr,
                                                     const UCHAR *component_name_ptr,
                                                     USHORT component_name_length,
                                                     UINT wait_option)
{
UINT status;

    if ((packet_ptr == NX_NULL) ||
        (component_name_ptr == NX_NULL) ||
        (component_name_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub telemetry component set fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_hub_client_telemetry_property_add(packet_ptr, 
                                                            (const UCHAR*)NX_AZURE_IOT_HUB_CLIENT_COMPONENT_STRING,
                                                            sizeof(NX_AZURE_IOT_HUB_CLIENT_COMPONENT_STRING) - 1,
                                                            component_name_ptr, component_name_length,
                                                            wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Telemetry component append fail: error status: %d"), status);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_telemetry_property_add(NX_PACKET *packet_ptr,
                                                    const UCHAR *property_name, USHORT property_name_length,
                                                    const UCHAR *property_value, USHORT property_value_length,
                                                    UINT wait_option)
{
UINT status;

    if ((packet_ptr == NX_NULL) ||
        (property_name == NX_NULL) ||
        (property_value == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub telemetry property add fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (*(packet_ptr -> nx_packet_append_ptr - 1) != '/')
    {
        status = nx_packet_data_append(packet_ptr, "&", 1,
                                       packet_ptr -> nx_packet_pool_owner,
                                       wait_option);
        if (status)
        {
            LogError(LogLiteralArgs("Telemetry data append fail"));
            return(status);
        }
    }

    status = nx_packet_data_append(packet_ptr, (VOID *)property_name, (UINT)property_name_length,
                                   packet_ptr -> nx_packet_pool_owner, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Telemetry data append fail"));
        return(status);
    }

    status = nx_packet_data_append(packet_ptr, "=", 1,
                                   packet_ptr -> nx_packet_pool_owner,
                                   wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Telemetry data append fail"));
        return(status);
    }

    status = nx_packet_data_append(packet_ptr, (VOID *)property_value, (UINT)property_value_length,
                                   packet_ptr -> nx_packet_pool_owner, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Telemetry data append fail"));
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_telemetry_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                            NX_PACKET *packet_ptr, const UCHAR *telemetry_data,
                                            UINT data_size, UINT wait_option)
{
UINT status;
UINT topic_len;
UCHAR packet_id[2];

    if ((hub_client_ptr == NX_NULL) || (packet_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub telemetry send fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    topic_len = packet_ptr -> nx_packet_length;

    status = nx_azure_iot_mqtt_packet_id_get(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             packet_id);
    if (status)
    {
        LogError(LogLiteralArgs("Failed to get packet id"));
        return(status);
    }

    /* Append packet identifier.  */
    status = nx_packet_data_append(packet_ptr, packet_id, sizeof(packet_id),
                                   packet_ptr -> nx_packet_pool_owner,
                                   wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Telemetry append fail"));
        return(status);
    }

    if (telemetry_data && (data_size != 0))
    {

        /* Append payload.  */
        status = nx_packet_data_append(packet_ptr, (VOID *)telemetry_data, data_size,
                                       packet_ptr -> nx_packet_pool_owner,
                                       wait_option);
        if (status)
        {
            LogError(LogLiteralArgs("Telemetry data append fail"));
            return(status);
        }
    }

    status = nx_azure_iot_publish_mqtt_packet(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                              packet_ptr, topic_len, packet_id, NX_AZURE_IOT_HUB_CLIENT_TELEMETRY_QOS,
                                              wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client send fail: PUBLISH FAIL status: %d"), status);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_receive_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  UINT message_type,
                                                  VOID (*callback_ptr)(
                                                        NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        VOID *args),
                                                  VOID *callback_args)
{
    if ((hub_client_ptr == NX_NULL) || (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub receive callback set fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    if (message_type == NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_callback = callback_ptr;
        hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_callback_args = callback_args;
    }
    else if (message_type == NX_AZURE_IOT_HUB_PROPERTIES)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_callback = callback_ptr;
        hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_callback_args = callback_args;
    }
    else if (message_type == NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_callback = callback_ptr;
        hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_callback_args = callback_args;
    }
    else if (message_type == NX_AZURE_IOT_HUB_COMMAND)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_callback = callback_ptr;
        hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_callback_args = callback_args;
    }
    else
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        return(NX_AZURE_IOT_NOT_SUPPORTED);
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_cloud_message_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub cloud message subscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_cloud_message_sub_unsub(hub_client_ptr, NX_TRUE));
}

UINT nx_azure_iot_hub_client_cloud_message_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub cloud message unsubscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_cloud_message_sub_unsub(hub_client_ptr, NX_FALSE));
}

static UINT nx_azure_iot_hub_client_process_publish_packet(UCHAR *start_ptr,
                                                           ULONG *topic_offset_ptr,
                                                           USHORT *topic_length_ptr)
{
UCHAR *byte = start_ptr;
UINT byte_count = 0;
UINT multiplier = 1;
UINT remaining_length = 0;
UINT topic_length;

    /* Validate packet start contains fixed header.  */
    do
    {
        if (byte_count >= 4)
        {
            LogError(LogLiteralArgs("Invalid mqtt packet start position"));
            return(NX_AZURE_IOT_INVALID_PACKET);
        }

        byte++;
        remaining_length += (((*byte) & 0x7F) * multiplier);
        multiplier = multiplier << 7;
        byte_count++;
    } while ((*byte) & 0x80);

    if (remaining_length < 2)
    {
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    /* Retrieve topic length.  */
    byte++;
    topic_length = (UINT)(*(byte) << 8) | (*(byte + 1));

    if (topic_length > remaining_length - 2u)
    {
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    *topic_offset_ptr = (ULONG)((byte + 2) - start_ptr);
    *topic_length_ptr = (USHORT)topic_length;

    /* Return.  */
    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT message_type,
                                                    NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE *receive_message,
                                                    NX_PACKET **packet_pptr, UINT wait_option)
{
NX_PACKET *packet_ptr = NX_NULL;
UINT old_threshold;
NX_AZURE_IOT_THREAD thread_list;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub message receive fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if (receive_message -> message_process == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub message receive fail: NOT ENABLED"));
        return(NX_AZURE_IOT_NOT_ENABLED);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    if (receive_message -> message_head)
    {
        packet_ptr = receive_message -> message_head;
        if (receive_message -> message_tail == packet_ptr)
        {
            receive_message -> message_tail = NX_NULL;
        }
        receive_message -> message_head = packet_ptr -> nx_packet_queue_next;
    }
    else if (wait_option)
    {
        thread_list.thread_message_type = message_type;
        thread_list.thread_ptr = tx_thread_identify();
        thread_list.thread_received_message = NX_NULL;
        thread_list.thread_expected_id = 0;
        thread_list.thread_next = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
        hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended = &thread_list;

        /* Disable preemption.  */
        tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        tx_thread_sleep(wait_option);

        /* Obtain the mutex.  */
        tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

        nx_azure_iot_hub_client_thread_dequeue(hub_client_ptr, &thread_list);

        /* Restore preemption.  */
        tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
        packet_ptr = thread_list.thread_received_message;
    }

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    if (packet_ptr == NX_NULL)
    {
        if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
        {
            LogError(LogLiteralArgs("IoTHub message receive fail:  IoTHub client not connected"));
            return(NX_AZURE_IOT_DISCONNECTED);
        }

        return(NX_AZURE_IOT_NO_PACKET);
    }

    *packet_pptr = packet_ptr;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_adjust_payload(NX_PACKET *packet_ptr)
{
UINT status;
ULONG topic_offset;
USHORT topic_length;
ULONG message_offset;
ULONG message_length;

    status = _nxd_mqtt_process_publish_packet(packet_ptr, &topic_offset,
                                              &topic_length, &message_offset,
                                              &message_length);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    packet_ptr -> nx_packet_length = message_length;

    /* Adjust packet to pointer to message payload.  */
    while (packet_ptr)
    {
        if ((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) > message_offset)
        {

            /* This packet contains message payload.  */
            packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + message_offset;
            break;
        }

        message_offset -= (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);

        /* Set current packet to empty.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_append_ptr;

        /* Move to next packet.  */
        packet_ptr = packet_ptr -> nx_packet_next;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_cloud_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_PACKET **packet_pptr, UINT wait_option)
{
UINT status;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub cloud message receive fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_hub_client_message_receive(hub_client_ptr, NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE,
                                                     &(hub_client_ptr -> nx_azure_iot_hub_client_c2d_message),
                                                     packet_pptr, wait_option);
    if (status)
    {
        return(status);
    }

    return(nx_azure_iot_hub_client_adjust_payload(*packet_pptr));
}

UINT nx_azure_iot_hub_client_cloud_message_property_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET *packet_ptr, const UCHAR *property_name,
                                                        USHORT property_name_length, const UCHAR **property_value,
                                                        USHORT *property_value_length)
{
USHORT topic_size;
UINT status;
ULONG topic_offset;
UCHAR *topic_name;
az_iot_hub_client_c2d_request request;
az_span receive_topic;
az_result core_result;
az_span span;

    if (packet_ptr == NX_NULL ||
        property_name == NX_NULL ||
        property_value == NX_NULL ||
        property_value_length == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub cloud message get property fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_hub_client_process_publish_packet(packet_ptr -> nx_packet_data_start,
                                                            &topic_offset, &topic_size);
    if (status)
    {
        return(status);
    }

    topic_name = packet_ptr -> nx_packet_data_start + topic_offset;

    /* NOTE: Current implementation does not support topic to span multiple packets.  */
    if ((ULONG)(packet_ptr -> nx_packet_append_ptr - topic_name) < (ULONG)topic_size)
    {
        LogError(LogLiteralArgs("IoTHub cloud message get property fail: topic out of boundaries of single packet"));
        return(NX_AZURE_IOT_TOPIC_TOO_LONG);
    }

    receive_topic = az_span_create(topic_name, (INT)topic_size);
    core_result = az_iot_hub_client_c2d_parse_received_topic(&hub_client_ptr -> iot_hub_client_core,
                                                             receive_topic, &request);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub cloud message get property fail: parsing error"));
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    span = az_span_create((UCHAR *)property_name, property_name_length);
    core_result = az_iot_message_properties_find(&request.properties, span, &span);
    if (az_result_failed(core_result))
    {
        if (core_result == AZ_ERROR_ITEM_NOT_FOUND)
        {
            status = NX_AZURE_IOT_NOT_FOUND;
        }
        else
        {
            LogError(LogLiteralArgs("IoTHub cloud message get property fail: property find"));
            status = NX_AZURE_IOT_SDK_CORE_ERROR;
        }

        return(status);
    }

    *property_value = (UCHAR *)az_span_ptr(span);
    *property_value_length = (USHORT)az_span_size(span);

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_hub_client_mqtt_ack_receive_notify(NXD_MQTT_CLIENT *client_ptr, UINT type,
                                                            USHORT packet_id, NX_PACKET *transmit_packet_ptr,
    VOID *context)
{

NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)context;
UCHAR buffer[sizeof(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC) - 1];
ULONG bytes_copied;


    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(packet_id);
    NX_PARAMETER_NOT_USED(context);

    /* Monitor subscribe ack.  */
    if (type == MQTT_CONTROL_PACKET_TYPE_SUBACK)
    {

        /* Get the topic.  */
        if (nx_packet_data_extract_offset(transmit_packet_ptr,
                                          NX_AZURE_IOT_MQTT_SUBSCRIBE_TOPIC_OFFSET,
                                          buffer, sizeof(buffer), &bytes_copied))
        {
            return;
        }

        /* Compare the topic.  */
        if ((bytes_copied == sizeof(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC) - 1) &&
            (!memcmp(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC,
                     buffer, sizeof(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC) - 1)))
        {
            hub_client_ptr -> nx_azure_iot_hub_client_properties_subscribe_ack = NX_TRUE;
        }
    }
}

UINT nx_azure_iot_hub_client_properties_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;
NXD_MQTT_CLIENT *client_ptr;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client properties subscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Atomically update the handler as we need to serialize the handler with incoming messages.  */
    hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process =
                      nx_azure_iot_hub_client_properties_process;
    hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_process =
                      nx_azure_iot_hub_client_properties_process;
    hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_enable =
                      nx_azure_iot_hub_client_properties_enable;
    hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_enable =
                      nx_azure_iot_hub_client_properties_enable;

    /* Register callbacks even if not connect and when connect complete subscribe for topics.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        return(NX_AZURE_IOT_SUCCESS);
    }

    /* Initialize variables.  */
    hub_client_ptr -> nx_azure_iot_hub_client_properties_subscribe_ack = NX_FALSE;

    /* Set ack receive notify for subscribe ack.  */
    client_ptr = &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt);
    client_ptr -> nxd_mqtt_ack_receive_notify = nx_azure_iot_hub_client_mqtt_ack_receive_notify;
    client_ptr -> nxd_mqtt_ack_receive_context = hub_client_ptr;

    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    status = nxd_mqtt_client_subscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                       AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC,
                                       sizeof(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC) - 1,
                                       NX_AZURE_IOT_MQTT_QOS_0);
    if (status)
    {

        /* Clean ack receive notify.  */
        client_ptr -> nxd_mqtt_ack_receive_notify = NX_NULL;
        LogError(LogLiteralArgs("IoTHub client device twin subscribe fail status: %d"), status);
        return(status);
    }

    status = nxd_mqtt_client_subscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                       AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC,
                                       sizeof(AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC) - 1,
                                       NX_AZURE_IOT_MQTT_QOS_0);
    if (status)
    {

        /* Clean ack receive notify.  */
        client_ptr -> nxd_mqtt_ack_receive_notify = NX_NULL;
        LogError(LogLiteralArgs("IoTHub client device twin subscribe fail status: %d"), status);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_properties_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client properties unsubscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nxd_mqtt_client_unsubscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                         AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC,
                                         sizeof(AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_SUBSCRIBE_TOPIC) - 1);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin unsubscribe fail status: %d"), status);
        return(status);
    }

    status = nxd_mqtt_client_unsubscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                         AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC,
                                         sizeof(AZ_IOT_HUB_CLIENT_TWIN_PATCH_SUBSCRIBE_TOPIC) - 1);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin unsubscribe fail status: %d"), status);
        return(status);
    }

    hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process = NX_NULL;
    hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_process = NX_NULL;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_response_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                       VOID (*callback_ptr)(
                                                                             NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                             UINT request_id,
                                                                             UINT response_status,
                                                                             ULONG version,
                                                                             VOID *args),
                                                                       VOID *callback_args)
{
    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin set callback fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    hub_client_ptr -> nx_azure_iot_hub_client_report_properties_response_callback = callback_ptr;
    hub_client_ptr -> nx_azure_iot_hub_client_report_properties_response_callback_args = callback_args;

    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_request_id_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                              UCHAR *buffer_ptr, UINT buffer_len,
                                                              az_span *request_id_span_ptr,
                                                              UINT *request_id_ptr, UINT odd_seq)
{
az_span span;

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Check if current request_id is even and new requested is also even or
       current request_id is odd and new requested is also odd.  */
    if ((hub_client_ptr -> nx_azure_iot_hub_client_request_id & 0x1) == odd_seq)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_request_id += 2;
    }
    else
    {
        hub_client_ptr -> nx_azure_iot_hub_client_request_id += 1;
    }

    if (hub_client_ptr -> nx_azure_iot_hub_client_request_id == 0)
    {
        hub_client_ptr -> nx_azure_iot_hub_client_request_id = 2;
    }

    *request_id_ptr = hub_client_ptr -> nx_azure_iot_hub_client_request_id;
    span = az_span_create(buffer_ptr, (INT)buffer_len);
    if (az_result_failed(az_span_u32toa(span, *request_id_ptr, &span)))
    {

        /* Release the mutex.  */
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        LogError(LogLiteralArgs("IoTHub client device failed to u32toa"));
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    *request_id_span_ptr = az_span_create(buffer_ptr, (INT)(buffer_len - (UINT)az_span_size(span)));

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_subscribe_status_check(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT wait_option)
{

    /* Wait for subscribe ack of topic "$iothub/twin/res/#".  */
    while (1)
    {
        tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

        /* Check if it is still in connected status.  */
        if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
        {

            /* Clean ack receive notify.  */
            hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt.nxd_mqtt_ack_receive_notify = NX_NULL;
            tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
            return(NX_AZURE_IOT_DISCONNECTED);
        }

        /* Check if receive the subscribe ack.  */
        if (hub_client_ptr -> nx_azure_iot_hub_client_properties_subscribe_ack == NX_TRUE)
        {

            /* Clean ack receive notify.  */
            hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt.nxd_mqtt_ack_receive_notify = NX_NULL;
            tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
            break;
        }

        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        /* Update wait time.  */
        if (wait_option != NX_WAIT_FOREVER)
        {
            if (wait_option > 0)
            {
                wait_option--;
            }
            else
            {
                return(NX_AZURE_IOT_NO_SUBSCRIBE_ACK);
            }
        }

        tx_thread_sleep(1);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_request_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                UINT request_id, NX_PACKET *packet_ptr, ULONG topic_length,
                                                                const UCHAR *message_buffer, UINT message_length, UINT message_type,
                                                                NX_PACKET **response_packet_pptr, UINT wait_option)
{
NX_AZURE_IOT_THREAD thread_list;
UINT status;

    if ((message_buffer != NX_NULL) && (message_length != 0))
    {

        /* Append payload.  */
        status = nx_packet_data_append(packet_ptr, (VOID *)message_buffer, message_length,
                                       packet_ptr -> nx_packet_pool_owner,
                                       wait_option);
        if (status)
        {
            LogError(LogLiteralArgs("IoTHub client reported state send fail: append failed"));
            return(status);
        }
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    thread_list.thread_message_type = message_type;
    thread_list.thread_ptr = tx_thread_identify();
    thread_list.thread_expected_id = request_id;
    thread_list.thread_received_message = NX_NULL;
    thread_list.thread_next = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
    hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended = &thread_list;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    status = nx_azure_iot_publish_mqtt_packet(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                              packet_ptr, topic_length, NX_NULL, NX_AZURE_IOT_MQTT_QOS_0,
                                              wait_option);

    if (status)
    {

        /* Remove thread from waiting suspend queue.  */
        tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);
        nx_azure_iot_hub_client_thread_dequeue(hub_client_ptr, &thread_list);
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        LogError(LogLiteralArgs("IoTHub transport send: PUBLISH FAIL status: %d"), status);
        return(status);
    }

    if ((thread_list.thread_received_message) == NX_NULL && wait_option)
    {
        tx_thread_sleep(wait_option);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    nx_azure_iot_hub_client_thread_dequeue(hub_client_ptr, &thread_list);
    *response_packet_pptr = thread_list.thread_received_message;

    /* Release the mutex.  */
    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET **packet_pptr,
                                                        UINT wait_option)
{
UINT status;
NX_PACKET *packet_ptr;
UINT request_id;
UCHAR *buffer_ptr;
ULONG buffer_size;
az_span request_id_span;
az_result core_result;
UINT topic_length;

    if ((hub_client_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub reported properties create fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_publish_packet_get(hub_client_ptr -> nx_azure_iot_ptr,
                                             &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported properties create fail: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    buffer_size = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr);
    if (buffer_size <= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE)
    {
        LogError(LogLiteralArgs("IoTHub client reported properties create fail: BUFFER INSUFFICENT"));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    buffer_size -= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE;

    /* Generate odd request id for reported properties send */
    status = nx_azure_iot_hub_client_properties_request_id_get(hub_client_ptr,
                                                               (UCHAR *)(packet_ptr -> nx_packet_data_end -
                                                                 NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE),
                                                               NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE,
                                                               &request_id_span, &request_id, NX_TRUE);

    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported properties create fail: get request id failed"));
        nx_packet_release(packet_ptr);
        return(status);
    }

    core_result = az_iot_hub_client_properties_get_reported_publish_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                          request_id_span,
                                                                          (CHAR *)packet_ptr -> nx_packet_prepend_ptr,
                                                                          buffer_size, &topic_length);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client reported properties create fail: NX_AZURE_IOT_PNP_CLIENT_TOPIC_SIZE is too small."));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + topic_length;
    packet_ptr -> nx_packet_length = topic_length;

    /* Set the buffer pointer.  */
    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr - NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET;

    /* encode topic length */
    buffer_ptr[5] = (UCHAR)(packet_ptr -> nx_packet_length >> 8);
    buffer_ptr[6] = (UCHAR)(packet_ptr -> nx_packet_length & 0xFF);

    /* encode request id */
    buffer_ptr[4] = (UCHAR)((request_id & 0xFF));
    request_id >>= 8;
    buffer_ptr[3] = (UCHAR)((request_id & 0xFF));
    request_id >>= 8;
    buffer_ptr[2] = (UCHAR)((request_id & 0xFF));
    request_id >>= 8;
    buffer_ptr[1] = (UCHAR)(request_id & 0xFF);

    *packet_pptr = packet_ptr;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET *packet_ptr,
                                                      UINT *request_id_ptr, UINT *response_status_ptr,
                                                      ULONG *version_ptr, UINT wait_option)
{
NX_PACKET *response_packet_ptr;
UINT topic_length;
UINT request_id = 0;
ULONG topic_offset;
USHORT length;
UCHAR *buffer_ptr;
UINT status;

    if ((hub_client_ptr == NX_NULL) ||
        (packet_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client reported properties send fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Check if properties is subscribed */
    if ((status = nx_azure_iot_hub_client_properties_subscribe_status_check(hub_client_ptr, wait_option)))
    {
        LogError(LogLiteralArgs("IoTHub client reported properties send fail with error %d"), status);
        return(status);
    }

    /* Check if the last request was throttled and if the next need to be throttled.  */
    if ((status = nx_azure_iot_hub_client_throttled_check(hub_client_ptr)))
    {
        LogError(LogLiteralArgs("IoTHub client reported properties send fail with error %d"), status);
        return(status);
    }

    /* Steps.
     * 1. Publish message to topic "$iothub/twin/PATCH/properties/reported/?$rid={request id}"
     * 2. Wait for the response if required.
     * 3. Return result if present.
     * */

    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr - NX_AZURE_IOT_PUBLISH_PACKET_START_OFFSET;

    topic_length = (UINT)((buffer_ptr[5] << 8) | buffer_ptr[6]);

    request_id += (buffer_ptr[1] & 0xFF);
    request_id <<= 8;
    request_id += (buffer_ptr[2] & 0xFF);
    request_id <<= 8;
    request_id += (buffer_ptr[3] & 0xFF);
    request_id <<= 8;
    request_id += (buffer_ptr[4] & 0xFF);

    status = nx_azure_iot_hub_client_properties_request_response(hub_client_ptr,
                                                                 request_id, packet_ptr, topic_length, NX_NULL, 0,
                                                                 NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE,
                                                                 &response_packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported properties send fail: append failed"));
        return(status);
    }

    /* The packet of reported properties has been sent out successfully,
       next the return value should be NX_AZURE_IOT_SUCCESS.  */

    /* Continue to process response and the caller can check the response status to see if iothub accept the properties or not,
       the reponse status is available only when the return status is NX_AZURE_IOT_SUCCESS.    */
    if (request_id_ptr)
    {
        *request_id_ptr = request_id;
    }

    if (response_status_ptr)
    {
        *response_status_ptr = 0;
    }

    if (response_packet_ptr)
    {
        if(nx_azure_iot_hub_client_process_publish_packet(response_packet_ptr -> nx_packet_prepend_ptr,
                                                          &topic_offset, &length) == NX_AZURE_IOT_SUCCESS)
        {
            nx_azure_iot_hub_client_device_twin_parse(hub_client_ptr,
                                                      response_packet_ptr, topic_offset, length,
                                                      NX_NULL, version_ptr, NX_NULL,
                                                      response_status_ptr);
        }

        nx_packet_release(response_packet_ptr);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                UINT wait_option)
{
UINT status;
UINT topic_length;
UINT buffer_size;
NX_PACKET *packet_ptr;
az_span request_id_span;
UINT request_id;
az_result core_result;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client properties request failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }
    
    if ((status = nx_azure_iot_hub_client_properties_subscribe_status_check(hub_client_ptr, wait_option)))
    {
        LogError(LogLiteralArgs("IoTHub client properties request failed with error %d"), status);
        return(status);
    }

    /* Check if the last request was throttled and if the next need to be throttled.  */
    if ((status = nx_azure_iot_hub_client_throttled_check(hub_client_ptr)))
    {
        LogError(LogLiteralArgs("IoTHub client properties request failed with error %d"), status);
        return(status);
    }

    /* Steps.
     * 1. Publish message to topic "$iothub/twin/GET/?$rid={request id}"
     * */
    status = nx_azure_iot_publish_packet_get(hub_client_ptr -> nx_azure_iot_ptr,
                                             &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client properties request failed: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    buffer_size = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr);
    if (buffer_size <= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE)
    {
        LogError(LogLiteralArgs("IoTHub client device twin publish fail: BUFFER ALLOCATE FAIL"));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    buffer_size -= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE;

    /* Generate even request id for properties request.  */
    status = nx_azure_iot_hub_client_properties_request_id_get(hub_client_ptr,
                                                               (UCHAR *)(packet_ptr -> nx_packet_data_end -
                                                                 NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE),
                                                               NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE,
                                                               &request_id_span, &request_id, NX_FALSE);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin failed to get request id"));
        nx_packet_release(packet_ptr);
        return(status);
    }

    core_result = az_iot_hub_client_properties_document_get_publish_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                          request_id_span, (CHAR *)packet_ptr -> nx_packet_prepend_ptr,
                                                                          buffer_size, (size_t *)&topic_length);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client device twin get topic fail."));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + topic_length;
    packet_ptr -> nx_packet_length = topic_length;

    status = nx_azure_iot_publish_mqtt_packet(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                              packet_ptr, topic_length, NX_NULL, NX_AZURE_IOT_MQTT_QOS_0,
                                              wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin: PUBLISH FAIL status: %d"), status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_PACKET **packet_pptr,
                                                UINT wait_option)
{
UINT status;
ULONG topic_offset;
USHORT topic_length;
az_result core_result;
az_span topic_span;
az_iot_hub_client_properties_message out_message;
NX_PACKET *packet_ptr;

    if ((hub_client_ptr == NX_NULL) || (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client properties receive failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Steps.
     * 1. Check if the properties document is available to receive from linklist.
     * 2. If present check the response.
     * 3. Return the payload of the response.
     * */
    status = nx_azure_iot_hub_client_message_receive(hub_client_ptr, NX_AZURE_IOT_HUB_PROPERTIES,
                                                     &(hub_client_ptr -> nx_azure_iot_hub_client_properties_message),
                                                     &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin receive failed status: %d"), status);
        return(status);
    }
    
    if (nx_azure_iot_hub_client_process_publish_packet(packet_ptr -> nx_packet_prepend_ptr, &topic_offset, &topic_length))
    {

        /* Message not supported. It will be released.  */
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INVALID_PACKET);
    }

    topic_span = az_span_create(&(packet_ptr -> nx_packet_prepend_ptr[topic_offset]), (INT)topic_length);
    core_result = az_iot_hub_client_properties_parse_received_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                    topic_span, &out_message);
    if (az_result_failed(core_result))
    {

        /* Topic name does not match properties format.  */
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    if ((out_message.status < 200) || (out_message.status >= 300))
    {
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_SERVER_RESPONSE_ERROR);
    }

    if ((status = nx_azure_iot_hub_client_adjust_payload(packet_ptr)))
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    *packet_pptr = packet_ptr;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_writable_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET **packet_pptr,
                                                        UINT wait_option)
{
UINT status;
NX_PACKET *packet_ptr;

    if ((hub_client_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client receive properties failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Steps.
     * 1. Check if the writable properties document is available to receive from linklist.
     * 2. Parse result if present.
     * 3. Return parse result.
     * */
    status = nx_azure_iot_hub_client_message_receive(hub_client_ptr, NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                     &(hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message),
                                                     &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client device twin receive failed status: %d"), status);
        return(status);
    }

    if ((status = nx_azure_iot_hub_client_adjust_payload(packet_ptr)))
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    *packet_pptr = packet_ptr;

    return(NX_AZURE_IOT_SUCCESS);
}


UINT nx_azure_iot_hub_client_device_twin_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin subscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_properties_enable(hub_client_ptr));
}

UINT nx_azure_iot_hub_client_device_twin_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin unsubscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_properties_disable(hub_client_ptr));
}


UINT nx_azure_iot_hub_client_device_twin_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                  const UCHAR *message_buffer, UINT message_length,
                                                                  UINT *request_id_ptr, UINT *response_status_ptr,
                                                                  ULONG *version_ptr, UINT wait_option)
{
UINT status;
UINT buffer_size;
NX_PACKET *packet_ptr;
UINT topic_length;
UINT request_id;
az_span request_id_span;
az_result core_result;
ULONG topic_offset;
USHORT length;
NX_PACKET *response_packet_ptr;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    if ((status = nx_azure_iot_hub_client_properties_subscribe_status_check(hub_client_ptr, wait_option)))
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail with error %d"), status);
        return(status);
    }

    /* Check if the last request was throttled and if the next need to be throttled.  */
    if ((status = nx_azure_iot_hub_client_throttled_check(hub_client_ptr)))
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail with error %d"), status);
        return(status);
    }

    /* Steps.
     * 1. Publish message to topic "$iothub/twin/PATCH/properties/reported/?$rid={request id}"
     * 2. Wait for the response if required.
     * 3. Return result if present.
     * */
    if (hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: NOT ENABLED"));
        return(NX_AZURE_IOT_NOT_ENABLED);
    }

    status = nx_azure_iot_publish_packet_get(hub_client_ptr -> nx_azure_iot_ptr,
                                             &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    buffer_size = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr);
    if (buffer_size <= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: BUFFER INSUFFICENT"));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    buffer_size -= NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE;

    /* Generate odd request id for reported properties send.  */
    status = nx_azure_iot_hub_client_properties_request_id_get(hub_client_ptr,
                                                               (UCHAR *)(packet_ptr -> nx_packet_data_end -
                                                                 NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE),
                                                               NX_AZURE_IOT_HUB_CLIENT_U32_MAX_BUFFER_SIZE,
                                                               &request_id_span, &request_id, NX_TRUE);

    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send failed to get request id"));
        nx_packet_release(packet_ptr);
        return(status);
    }

    core_result = az_iot_hub_client_twin_patch_get_publish_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                 request_id_span, (CHAR *)packet_ptr -> nx_packet_prepend_ptr,
                                                                 buffer_size, (size_t *)&topic_length);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: NX_AZURE_IOT_HUB_CLIENT_TOPIC_SIZE is too small."));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + topic_length;
    packet_ptr -> nx_packet_length = topic_length;

    status = nx_azure_iot_hub_client_properties_request_response(hub_client_ptr,
                                                                request_id, packet_ptr, topic_length, message_buffer, message_length,
                                                                NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE,
                                                                &response_packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client reported state send fail: append failed"));
        nx_packet_release(packet_ptr);
        return(status);
    }

    if (request_id_ptr)
    {
        *request_id_ptr = request_id;
    }

    if (response_packet_ptr == NX_NULL)
    {
        if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
        {
            return(NX_AZURE_IOT_DISCONNECTED);
        }
        return(NX_AZURE_IOT_NO_PACKET);
    }

    if ((status = nx_azure_iot_hub_client_process_publish_packet(response_packet_ptr -> nx_packet_prepend_ptr,
                                                                 &topic_offset, &length)))
    {
        nx_packet_release(response_packet_ptr);
        return(status);
    }

    if ((status = nx_azure_iot_hub_client_device_twin_parse(hub_client_ptr,
                                                            response_packet_ptr, topic_offset, length,
                                                            NX_NULL, version_ptr, NX_NULL,
                                                            response_status_ptr)))
    {
        nx_packet_release(response_packet_ptr);
        return(status);
    }

    /* Release message block.  */
    nx_packet_release(response_packet_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_device_twin_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT wait_option)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin properties request failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_properties_request(hub_client_ptr, wait_option));
}

UINT nx_azure_iot_hub_client_device_twin_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            NX_PACKET **packet_pptr, UINT wait_option)
{

    if (hub_client_ptr == NX_NULL || packet_pptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin properties receive failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_properties_receive(hub_client_ptr, packet_pptr, wait_option));
}

UINT nx_azure_iot_hub_client_device_twin_desired_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                    NX_PACKET **packet_pptr, UINT wait_option)
{

    if (hub_client_ptr == NX_NULL || packet_pptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client device twin desired properties receive failed: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_writable_properties_receive(hub_client_ptr, packet_pptr, wait_option));
}

static UINT nx_azure_iot_hub_client_cloud_message_sub_unsub(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT is_subscribe)
{
UINT status;

    if (is_subscribe)
    {
        tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

        /* Atomically update the handler as we need to serialize the handler with incoming messages.  */
        hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process = nx_azure_iot_hub_client_c2d_process;
        hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_enable = nx_azure_iot_hub_client_cloud_message_enable;

        /* Register callbacks even if not connect and when connect complete subscribe for topics.  */
        if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
        {
            tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
            return(NX_AZURE_IOT_SUCCESS);
        }

        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

        status = nxd_mqtt_client_subscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                           AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC, sizeof(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC) - 1,
                                           NX_AZURE_IOT_MQTT_QOS_1);
        if (status)
        {
            LogError(LogLiteralArgs("IoTHub cloud message subscribe fail: status: %d"), status);
            return(status);
        }
    }
    else
    {
        status = nxd_mqtt_client_unsubscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC, sizeof(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC) - 1);
        if (status)
        {
            LogError(LogLiteralArgs("IoTHub cloud message subscribe fail: status: %d"), status);
            return(status);
        }

        hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process = NX_NULL;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_hub_client_mqtt_receive_callback(NXD_MQTT_CLIENT* client_ptr,
                                                          UINT number_of_messages)
{
NX_AZURE_IOT_RESOURCE *resource = nx_azure_iot_resource_search(client_ptr);
NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = NX_NULL;
NX_PACKET *packet_ptr;
NX_PACKET *packet_next_ptr;
ULONG topic_offset;
USHORT topic_length;

    /* This function is protected by MQTT mutex.  */

    NX_PARAMETER_NOT_USED(number_of_messages);

    if (resource && (resource -> resource_type == NX_AZURE_IOT_RESOURCE_IOT_HUB))
    {
        hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)resource -> resource_data_ptr;
    }

    if (hub_client_ptr)
    {
        for (packet_ptr = client_ptr -> message_receive_queue_head;
             packet_ptr;
             packet_ptr = packet_next_ptr)
        {

            /* Store next packet in case current packet is consumed.  */
            packet_next_ptr = packet_ptr -> nx_packet_queue_next;

            /* Adjust packet to simply process logic.  */
            nx_azure_iot_mqtt_packet_adjust(packet_ptr);

            if (nx_azure_iot_hub_client_process_publish_packet(packet_ptr -> nx_packet_prepend_ptr, &topic_offset,
                                                               &topic_length))
            {

                /* Message not supported. It will be released.  */
                nx_packet_release(packet_ptr);
                continue;
            }

            if ((topic_offset + topic_length) >
                (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr))
            {

                /* Only process topic in the first packet since the fixed topic is short enough to fit into one packet.  */
                topic_length = (USHORT)(((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) -
                                         topic_offset) & 0xFFFF);
            }

            if (hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_process &&
                (hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_process(hub_client_ptr, packet_ptr,
                                                                                                 topic_offset,
                                                                                                 topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Direct method message is processed.  */
                continue;
            }

            if (hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process &&
                (hub_client_ptr -> nx_azure_iot_hub_client_c2d_message.message_process(hub_client_ptr, packet_ptr,
                                                                                       topic_offset,
                                                                                       topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Could to Device message is processed.  */
                continue;
            }

            if ((hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process) &&
                (hub_client_ptr -> nx_azure_iot_hub_client_properties_message.message_process(hub_client_ptr,
                                                                                               packet_ptr, topic_offset,
                                                                                               topic_length) == NX_AZURE_IOT_SUCCESS))
            {

                /* Device Twin message is processed.  */
                continue;
            }

            /* Message not supported. It will be released.  */
            nx_packet_release(packet_ptr);
        }

        /* Clear all message from MQTT receive queue.  */
        client_ptr -> message_receive_queue_head = NX_NULL;
        client_ptr -> message_receive_queue_tail = NX_NULL;
        client_ptr -> message_receive_queue_depth = 0;
    }
}

static VOID nx_azure_iot_hub_client_message_notify(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_AZURE_IOT_HUB_CLIENT_RECEIVE_MESSAGE *receive_message,
                                                   NX_PACKET *packet_ptr)
{
    if (receive_message -> message_tail)
    {
        receive_message -> message_tail -> nx_packet_queue_next = packet_ptr;
    }
    else
    {
        receive_message -> message_head = packet_ptr;
    }
    receive_message -> message_tail = packet_ptr;

    /* Check for user callback function.  */
    if (receive_message -> message_callback)
    {
        receive_message -> message_callback(hub_client_ptr, receive_message -> message_callback_args);
    }
}

static UINT nx_azure_iot_hub_client_receive_thread_find(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET *packet_ptr, UINT message_type,
                                                        UINT request_id, NX_AZURE_IOT_THREAD **thread_list_pptr)
{
NX_AZURE_IOT_THREAD *thread_list_prev = NX_NULL;
NX_AZURE_IOT_THREAD *thread_list_ptr;

    /* Search thread waiting for message type.  */
    for (thread_list_ptr = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
         thread_list_ptr;
         thread_list_ptr = thread_list_ptr -> thread_next)
    {
        if ((thread_list_ptr -> thread_message_type == message_type) &&
            (request_id == thread_list_ptr -> thread_expected_id))
        {

            /* Found a thread waiting for message type.  */
            if (thread_list_prev == NX_NULL)
            {
                hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended = thread_list_ptr -> thread_next;
            }
            else
            {
                thread_list_prev -> thread_next = thread_list_ptr -> thread_next;
            }
            thread_list_ptr -> thread_received_message = packet_ptr;
            *thread_list_pptr =  thread_list_ptr;
            return(NX_AZURE_IOT_SUCCESS);
        }

        thread_list_prev = thread_list_ptr;
    }

    return(NX_AZURE_IOT_NOT_FOUND);
}

static UINT nx_azure_iot_hub_client_c2d_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_PACKET *packet_ptr,
                                                ULONG topic_offset,
                                                USHORT topic_length)
{
UCHAR *topic_name;
az_iot_hub_client_c2d_request request;
az_span receive_topic;
az_result core_result;
UINT status;
NX_AZURE_IOT_THREAD *thread_list_ptr;

    /* This function is protected by MQTT mutex.  */

    /* Check message type first.  */
    topic_name = &(packet_ptr -> nx_packet_prepend_ptr[topic_offset]);

    /* NOTE: Current implementation does not support topic to span multiple packets.  */
    if ((ULONG)(packet_ptr -> nx_packet_append_ptr - topic_name) < topic_length)
    {
        LogError(LogLiteralArgs("topic out of boundaries of single packet"));
        return(NX_AZURE_IOT_TOPIC_TOO_LONG);
    }

    receive_topic = az_span_create(topic_name, topic_length);
    core_result = az_iot_hub_client_c2d_parse_received_topic(&hub_client_ptr -> iot_hub_client_core,
                                                             receive_topic, &request);
    if (az_result_failed(core_result))
    {

        /* Topic name does not match C2D format.  */
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    status = nx_azure_iot_hub_client_receive_thread_find(hub_client_ptr,
                                                         packet_ptr,
                                                         NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE,
                                                         0, &thread_list_ptr);
    if (status == NX_AZURE_IOT_SUCCESS)
    {
        tx_thread_wait_abort(thread_list_ptr -> thread_ptr);
        return(NX_AZURE_IOT_SUCCESS);
    }

    /* No thread is waiting for C2D message yet.  */
    nx_azure_iot_hub_client_message_notify(hub_client_ptr,
                                           &(hub_client_ptr -> nx_azure_iot_hub_client_c2d_message),
                                           packet_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_command_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                    NX_PACKET *packet_ptr,
                                                    ULONG topic_offset,
                                                    USHORT topic_length)
{
UCHAR *topic_name;
az_iot_hub_client_command_request request;
az_span receive_topic;
az_result core_result;
UINT status;
NX_AZURE_IOT_THREAD *thread_list_ptr;

    /* This function is protected by MQTT mutex.  */

    /* Check message type first.  */
    topic_name = &(packet_ptr -> nx_packet_prepend_ptr[topic_offset]);

    /* NOTE: Current implementation does not support topic to span multiple packets.  */
    if ((ULONG)(packet_ptr -> nx_packet_append_ptr - topic_name) < topic_length)
    {
        LogError(LogLiteralArgs("topic out of boundaries of single packet"));
        return(NX_AZURE_IOT_TOPIC_TOO_LONG);
    }

    receive_topic = az_span_create(topic_name, topic_length);
    core_result = az_iot_hub_client_commands_parse_received_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                  receive_topic, &request);
    if (az_result_failed(core_result))
    {

        /* Topic name does not match direct method format.  */
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    status = nx_azure_iot_hub_client_receive_thread_find(hub_client_ptr,
                                                         packet_ptr,
                                                         NX_AZURE_IOT_HUB_COMMAND,
                                                         0, &thread_list_ptr);
    if (status == NX_AZURE_IOT_SUCCESS)
    {
        tx_thread_wait_abort(thread_list_ptr -> thread_ptr);
        return(NX_AZURE_IOT_SUCCESS);
    }

    /* No thread is waiting for direct method message yet.  */
    nx_azure_iot_hub_client_message_notify(hub_client_ptr,
                                           &(hub_client_ptr -> nx_azure_iot_hub_client_command_message),
                                           packet_ptr);
    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_message_type_get(UINT core_message_type, UINT request_id)
{
UINT message_type;

    switch (core_message_type)
    {
        case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_GET_RESPONSE :

        /* Fall through.  */

        case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_ACKNOWLEDGEMENT :
        {

            /* Odd requests are of reported properties and even of twin properties.  */
            message_type = request_id % 2 == 0 ? NX_AZURE_IOT_HUB_PROPERTIES : NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE;
        }
        break;

        case AZ_IOT_HUB_CLIENT_PROPERTIES_MESSAGE_TYPE_WRITABLE_UPDATED :
        {
            message_type = NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES;
        }
        break;

        default :
        {
            message_type = NX_AZURE_IOT_HUB_NONE;
        }
    }

    return message_type;
}

static UINT nx_azure_iot_hub_client_device_twin_parse(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET *packet_ptr, ULONG topic_offset,
                                                      USHORT topic_length, UINT *request_id_ptr,
                                                      ULONG *version_ptr, UINT *message_type_ptr,
                                                      UINT *status_ptr)
{
az_result core_result;
az_span topic_span;
az_iot_hub_client_twin_response out_twin_response;
uint32_t request_id = 0;
uint32_t version;

    topic_span = az_span_create(&(packet_ptr -> nx_packet_prepend_ptr[topic_offset]), (INT)topic_length);
    core_result = az_iot_hub_client_twin_parse_received_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                              topic_span, &out_twin_response);
    if (az_result_failed(core_result))
    {
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    if (version_ptr != NX_NULL && az_span_ptr(out_twin_response.version))
    {
        core_result = az_span_atou32(out_twin_response.version, &version);
        if (az_result_failed(core_result))
        {
            return(NX_AZURE_IOT_SDK_CORE_ERROR);
        }

        *version_ptr = (ULONG)version;
    }

    if (az_span_ptr(out_twin_response.request_id))
    {
        core_result = az_span_atou32(out_twin_response.request_id, &request_id);
        if (az_result_failed(core_result))
        {
            return(NX_AZURE_IOT_SDK_CORE_ERROR);
        }
    }

    if (request_id_ptr)
    {
        *request_id_ptr = (UINT)request_id;
    }

    if (message_type_ptr)
    {
        *message_type_ptr = nx_azure_iot_hub_client_properties_message_type_get((UINT)out_twin_response.response_type,
                                                                                (UINT)request_id);
    }

    if (status_ptr)
    {
        *status_ptr = (UINT)out_twin_response.status;
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_hub_client_properties_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                       NX_PACKET *packet_ptr,
                                                       ULONG topic_offset,
                                                       USHORT topic_length)
{
NX_AZURE_IOT_THREAD *thread_list_ptr;
UINT message_type;
UINT response_status;
UINT request_id = 0;
ULONG version = 0;
UINT correlation_id;
UINT status;
ULONG current_time;

    /* This function is protected by MQTT mutex. */
    if ((status = nx_azure_iot_hub_client_device_twin_parse(hub_client_ptr, packet_ptr,
                                                            topic_offset, topic_length,
                                                            &request_id, &version,
                                                            &message_type, &response_status)))
    {
        return(status);
    }

    if (response_status == NX_AZURE_IOT_HUB_CLIENT_THROTTLE_STATUS_CODE)
    {
        if ((status = nx_azure_iot_unix_time_get(hub_client_ptr -> nx_azure_iot_ptr, &current_time)))
        {
            LogError(LogLiteralArgs("IoTHub client fail to get unix time: %d"), status);
            return(status);
        }

        hub_client_ptr -> nx_azure_iot_hub_client_throttle_end_time =
            current_time + nx_azure_iot_hub_client_throttle_with_jitter(hub_client_ptr);
    }
    else
    {
        hub_client_ptr -> nx_azure_iot_hub_client_throttle_count = 0;
        hub_client_ptr -> nx_azure_iot_hub_client_throttle_end_time = 0;
    }

    if (message_type == NX_AZURE_IOT_HUB_NONE)
    {
        LogError(LogLiteralArgs("IoTHub client fail to parse device twin: %d"), NX_AZURE_IOT_SERVER_RESPONSE_ERROR);
        return(NX_AZURE_IOT_SERVER_RESPONSE_ERROR);
    }
    else if (message_type == NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE)
    {

        /* Only requested thread should be woken.  */
        correlation_id = request_id;
    }
    else
    {

        /* Any thread can be woken.  */
        correlation_id = 0;

        /* Process system component.  */
        if ((hub_client_ptr -> nx_azure_iot_hub_client_component_properties_process) &&
            ((response_status >= 200) && (response_status < 300)))
        {
            hub_client_ptr -> nx_azure_iot_hub_client_component_properties_process(hub_client_ptr, packet_ptr, message_type);
        }
    }

    status = nx_azure_iot_hub_client_receive_thread_find(hub_client_ptr,
                                                         packet_ptr,
                                                         message_type,
                                                         correlation_id, &thread_list_ptr);
    if (status == NX_AZURE_IOT_SUCCESS)
    {
        tx_thread_wait_abort(thread_list_ptr -> thread_ptr);
        return(NX_AZURE_IOT_SUCCESS);
    }

    switch(message_type)
    {
        case NX_AZURE_IOT_HUB_REPORTED_PROPERTIES_RESPONSE :
        {
            if (hub_client_ptr -> nx_azure_iot_hub_client_report_properties_response_callback)
            {
                hub_client_ptr -> nx_azure_iot_hub_client_report_properties_response_callback(hub_client_ptr,
                                                                                              request_id,
                                                                                              response_status,
                                                                                              version,
                                                                                              hub_client_ptr -> nx_azure_iot_hub_client_report_properties_response_callback_args);
            }

            nx_packet_release(packet_ptr);
        }
        break;

        case NX_AZURE_IOT_HUB_PROPERTIES :
        {

            /* No thread is waiting for device twin message yet.  */
            nx_azure_iot_hub_client_message_notify(hub_client_ptr,
                                                   &(hub_client_ptr -> nx_azure_iot_hub_client_properties_message),
                                                   packet_ptr);
        }
        break;

        case NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES :
        {

            /* No thread is waiting for device twin message yet.  */
            nx_azure_iot_hub_client_message_notify(hub_client_ptr,
                                                   &(hub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message),
                                                   packet_ptr);
        }
        break;

        default :
            nx_packet_release(packet_ptr);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_hub_client_thread_dequeue(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_AZURE_IOT_THREAD *thread_list_ptr)
{
NX_AZURE_IOT_THREAD *thread_list_prev = NX_NULL;
NX_AZURE_IOT_THREAD *thread_list_current;

    for (thread_list_current = hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended;
         thread_list_current;
         thread_list_current = thread_list_current -> thread_next)
    {
        if (thread_list_current == thread_list_ptr)
        {

            /* Found the thread to dequeue.  */
            if (thread_list_prev == NX_NULL)
            {
                hub_client_ptr -> nx_azure_iot_hub_client_thread_suspended = thread_list_current -> thread_next;
            }
            else
            {
                thread_list_prev -> thread_next = thread_list_current -> thread_next;
            }
            break;
        }

        thread_list_prev = thread_list_current;
    }
}

static UINT nx_azure_iot_hub_client_sas_token_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  ULONG expiry_time_secs, const UCHAR *key, UINT key_len,
                                                  UCHAR *sas_buffer, UINT sas_buffer_len, UINT *sas_length)
{
UCHAR *buffer_ptr;
UINT buffer_size;
VOID *buffer_context;
az_span span = az_span_create(sas_buffer, (INT)sas_buffer_len);
az_span buffer_span;
UINT status;
UCHAR *output_ptr;
UINT output_len;
az_result core_result;

    status = nx_azure_iot_buffer_allocate(hub_client_ptr -> nx_azure_iot_ptr, &buffer_ptr, &buffer_size, &buffer_context);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client sas token fail: BUFFER ALLOCATE FAIL"));
        return(status);
    }

    core_result = az_iot_hub_client_sas_get_signature(&(hub_client_ptr -> iot_hub_client_core),
                                                      expiry_time_secs, span, &span);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub failed failed to get signature with error status: %d"), core_result);
        nx_azure_iot_buffer_free(buffer_context);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    status = nx_azure_iot_base64_hmac_sha256_calculate(&(hub_client_ptr -> nx_azure_iot_hub_client_resource),
                                                       key, key_len, az_span_ptr(span), (UINT)az_span_size(span),
                                                       buffer_ptr, buffer_size, &output_ptr, &output_len);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub failed to encoded hash"));
        nx_azure_iot_buffer_free(buffer_context);
        return(status);
    }

    buffer_span = az_span_create(output_ptr, (INT)output_len);
    core_result= az_iot_hub_client_sas_get_password(&(hub_client_ptr -> iot_hub_client_core),
                                                    expiry_time_secs, buffer_span, AZ_SPAN_EMPTY,
                                                    (CHAR *)sas_buffer, sas_buffer_len, (size_t *)&sas_buffer_len);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("IoTHub failed to generate token with error status: %d"), core_result);
        nx_azure_iot_buffer_free(buffer_context);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    *sas_length = sas_buffer_len;
    nx_azure_iot_buffer_free(buffer_context);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_command_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client command subscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    tx_mutex_get(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr, TX_WAIT_FOREVER);

    /* Atomically update the handler as we need to serialize the handler with incoming messages.  */
    hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_process = nx_azure_iot_hub_client_command_process;
    hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_enable = nx_azure_iot_hub_client_command_enable;

    /* Register callbacks even if not connect and when connect complete subscribe for topics.  */
    if (hub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);
        return(NX_AZURE_IOT_SUCCESS);
    }

    tx_mutex_put(hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr);

    status = nxd_mqtt_client_subscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                       AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC,
                                       sizeof(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC) - 1,
                                       NX_AZURE_IOT_MQTT_QOS_0);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client command subscribe fail %d"), status);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_command_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client command unsubscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nxd_mqtt_client_unsubscribe(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                         AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC,
                                         sizeof(AZ_IOT_HUB_CLIENT_METHODS_SUBSCRIBE_TOPIC) - 1);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client command unsubscribe fail status: %d"), status);
        return(status);
    }

    hub_client_ptr -> nx_azure_iot_hub_client_command_message.message_process = NX_NULL;

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_command_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                     const UCHAR **component_name_pptr, USHORT *component_name_length_ptr,
                                                     const UCHAR **command_name_pptr, USHORT *command_name_length_ptr,
                                                     VOID **context_pptr, USHORT *context_length_ptr,
                                                     NX_PACKET **packet_pptr, UINT wait_option)
{

UINT status;
ULONG topic_offset = 0;
USHORT topic_length = 0;
az_span topic_span;
NX_PACKET *packet_ptr = NX_NULL;
az_result core_result;
az_iot_hub_client_command_request request;

    if ((hub_client_ptr == NX_NULL) ||
        (command_name_pptr == NX_NULL) ||
        (command_name_length_ptr == NX_NULL) ||
        (context_pptr == NX_NULL) ||
        (context_length_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoT PnP client command receive fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    status = nx_azure_iot_hub_client_message_receive(hub_client_ptr, NX_AZURE_IOT_HUB_COMMAND,
                                                     &(hub_client_ptr -> nx_azure_iot_hub_client_command_message),
                                                     &packet_ptr, wait_option);
    if (status)
    {
        return(status);
    }

    status = nx_azure_iot_hub_client_process_publish_packet(packet_ptr -> nx_packet_prepend_ptr, &topic_offset, &topic_length);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    topic_span = az_span_create(&(packet_ptr -> nx_packet_prepend_ptr[topic_offset]), topic_length);
    core_result = az_iot_hub_client_commands_parse_received_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                  topic_span, &request);
    if (az_result_failed(core_result))
    {
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    if ((status = nx_azure_iot_hub_client_adjust_payload(packet_ptr)))
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    *packet_pptr = packet_ptr;
    if ((component_name_pptr) && (component_name_length_ptr))
    {
        *component_name_pptr = (const UCHAR *)az_span_ptr(request.component_name);
        *component_name_length_ptr = (USHORT)az_span_size(request.component_name);
    }
    *command_name_pptr = (const UCHAR *)az_span_ptr(request.command_name);
    *command_name_length_ptr = (USHORT)az_span_size(request.command_name);
    *context_pptr = (VOID*)az_span_ptr(request.request_id);
    *context_length_ptr =  (USHORT)az_span_size(request.request_id);

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_command_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      UINT status_code, VOID *context_ptr,
                                                      USHORT context_length, const UCHAR *payload,
                                                      UINT payload_length, UINT wait_option)
{
NX_PACKET *packet_ptr;
UINT topic_length;
az_span request_id_span;
UINT status;
az_result core_result;

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (context_ptr == NX_NULL) ||
        (context_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub client command response fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Prepare response packet.  */
    status = nx_azure_iot_publish_packet_get(hub_client_ptr -> nx_azure_iot_ptr,
                                             &(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                             &packet_ptr, wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("Create response data fail"));
        return(status);
    }

    topic_length = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr);
    request_id_span = az_span_create((UCHAR*)context_ptr, (INT)context_length);
    core_result = az_iot_hub_client_commands_response_get_publish_topic(&(hub_client_ptr -> iot_hub_client_core),
                                                                        request_id_span, (USHORT)status_code,
                                                                        (CHAR *)packet_ptr -> nx_packet_prepend_ptr,
                                                                        topic_length, (size_t *)&topic_length);
    if (az_result_failed(core_result))
    {
        LogError(LogLiteralArgs("Failed to create the command response topic"));
        nx_packet_release(packet_ptr);
        return(NX_AZURE_IOT_SDK_CORE_ERROR);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + topic_length;
    packet_ptr -> nx_packet_length = topic_length;

    if (payload && (payload_length != 0))
    {

        /* Append payload.  */
        status = nx_packet_data_append(packet_ptr, (VOID *)payload, payload_length,
                                       packet_ptr -> nx_packet_pool_owner,
                                       wait_option);
        if (status)
        {
            LogError(LogLiteralArgs("Command response data append fail"));
            nx_packet_release(packet_ptr);
            return(status);
        }
    }
    else
    {

        /* Append payload.  */
        status = nx_packet_data_append(packet_ptr, NX_AZURE_IOT_HUB_CLIENT_EMPTY_JSON,
                                       sizeof(NX_AZURE_IOT_HUB_CLIENT_EMPTY_JSON) - 1,
                                       packet_ptr -> nx_packet_pool_owner,
                                       wait_option);
        if (status)
        {
            LogError(LogLiteralArgs("Adding empty json failed."));
            nx_packet_release(packet_ptr);
            return(status);
        }
    }

    status = nx_azure_iot_publish_mqtt_packet(&(hub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt),
                                              packet_ptr, topic_length, NX_NULL, NX_AZURE_IOT_MQTT_QOS_0,
                                              wait_option);
    if (status)
    {
        LogError(LogLiteralArgs("IoTHub client command response fail: PUBLISH FAIL status: %d"), status);
        nx_packet_release(packet_ptr);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_hub_client_direct_method_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client direct method subscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_command_enable(hub_client_ptr));
}

UINT nx_azure_iot_hub_client_direct_method_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{

    if (hub_client_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("IoTHub client direct method unsubscribe fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_command_disable(hub_client_ptr));
}

UINT nx_azure_iot_hub_client_direct_method_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                           const UCHAR **method_name_pptr, USHORT *method_name_length_ptr,
                                                           VOID **context_pptr, USHORT *context_length_ptr,
                                                           NX_PACKET **packet_pptr, UINT wait_option)
{

    if ((hub_client_ptr == NX_NULL) ||
        (method_name_pptr == NX_NULL) ||
        (method_name_length_ptr == NX_NULL) ||
        (context_pptr == NX_NULL) ||
        (context_length_ptr == NX_NULL) ||
        (packet_pptr == NX_NULL))
    {
        LogError(LogLiteralArgs("IoTHub client direct method receive fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_command_message_receive(hub_client_ptr, NX_NULL, NX_NULL,
                                                           method_name_pptr, method_name_length_ptr,
                                                           context_pptr, context_length_ptr,
                                                           packet_pptr, wait_option));
}

UINT nx_azure_iot_hub_client_direct_method_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT status_code, VOID *context_ptr,
                                                            USHORT context_length, const UCHAR *payload,
                                                            UINT payload_length, UINT wait_option)
{

    if ((hub_client_ptr == NX_NULL) ||
        (hub_client_ptr -> nx_azure_iot_ptr == NX_NULL) ||
        (context_ptr == NX_NULL) ||
        (context_length == 0))
    {
        LogError(LogLiteralArgs("IoTHub direct method response fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    return(nx_azure_iot_hub_client_command_message_response(hub_client_ptr, status_code,
                                                            context_ptr, context_length,
                                                            payload, payload_length,
                                                            wait_option));
}
