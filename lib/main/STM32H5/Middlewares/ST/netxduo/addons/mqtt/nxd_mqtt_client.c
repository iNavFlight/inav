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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   MQTT (MQTT)                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NXD_MQTT_CLIENT_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include    "tx_api.h"
#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_mqtt_client.h"

/* Bring in externals for caller checking code.  */

#define MQTT_ALL_EVENTS               ((ULONG)0xFFFFFFFF)
#define MQTT_TIMEOUT_EVENT            ((ULONG)0x00000001)
#define MQTT_PACKET_RECEIVE_EVENT     ((ULONG)0x00000002)
#define MQTT_START_EVENT              ((ULONG)0x00000004)
#define MQTT_DELETE_EVENT             ((ULONG)0x00000008)
#define MQTT_PING_TIMEOUT_EVENT       ((ULONG)0x00000010)
#define MQTT_NETWORK_DISCONNECT_EVENT ((ULONG)0x00000020)
#define MQTT_TCP_ESTABLISH_EVENT      ((ULONG)0x00000040)

static UINT _nxd_mqtt_client_create_internal(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                                             CHAR *client_id, UINT client_id_length,
                                             NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                                             VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority);
static UINT _nxd_mqtt_packet_allocate(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
static UINT _nxd_mqtt_packet_send(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT wait_option);
static UINT _nxd_mqtt_packet_receive(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT wait_option);
static UINT _nxd_mqtt_copy_transmit_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET **new_packet_ptr,
                                           USHORT packet_id, UCHAR set_duplicate_flag, UINT wait_option);
static VOID _nxd_mqtt_release_transmit_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET *previous_packet_ptr);
static VOID _nxd_mqtt_release_receive_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET *previous_packet_ptr);
static UINT _nxd_mqtt_client_retransmit_message(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option);
static UINT _nxd_mqtt_client_connect_packet_send(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_set_fixed_header                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the fixed header filed in the outgoing         */
/*    MQTT packet.                                                        */
/*                                                                        */
/*    This function follows the logic outlined in 2.2 in MQTT             */
/*    specification.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Outgoing MQTT packet          */
/*    control_header                        Control byte                  */
/*    length                                Remaining length in bytes     */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append packet data            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                                          */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_client_publish                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_set_fixed_header(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, UCHAR control_header, UINT length, UINT wait_option)
{
UCHAR  fixed_header[5];
UCHAR *byte = fixed_header;
UINT   count = 0;
UINT   ret;

    *byte = control_header;
    byte++;

    do
    {
        if (length & 0xFFFFFF80)
        {
            *(byte + count) = (UCHAR)((length & 0x7F) | 0x80);
        }
        else
        {
            *(byte + count) = length & 0x7F;
        }
        length = length >> 7;

        count++;
    } while (length != 0);

    ret = nx_packet_data_append(packet_ptr, fixed_header, count + 1, 
                                client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);

    return(ret);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_read_remaining_length                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the remaining length filed in the incoming     */
/*    MQTT packet.                                                        */
/*                                                                        */
/*    This function follows the logic outlined in 2.2.3 in MQTT           */
/*    specification                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Incoming MQTT packet.         */
/*    remaining_length                      remaining length in bytes,    */
/*                                            this is the return value.   */
/*    offset                                Pointer to offset of the      */
/*                                            remaining data              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_client_message_get                                        */
/*    _nxd_mqtt_process_sub_unsub_ack                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_read_remaining_length(NX_PACKET *packet_ptr, UINT *remaining_length, ULONG *offset_ptr)
{
UINT   value = 0;
UCHAR  bytes[4] = {0};
UINT   multiplier = 1;
UINT   byte_count = 0;
ULONG  bytes_copied;

    if (nx_packet_data_extract_offset(packet_ptr, 1, &bytes, sizeof(bytes), &bytes_copied))
    {

        /* Packet is incomplete. */
        return(NXD_MQTT_PARTIAL_PACKET);
    }

    do
    {
        if (byte_count >= bytes_copied)
        {
            if (byte_count == 4)
            {
                return(NXD_MQTT_INTERNAL_ERROR);
            }
            else
            {

                /* Packet is incomplete. */
                return(NXD_MQTT_PARTIAL_PACKET);
            }
        }
        value += (((bytes[byte_count]) & 0x7F) * multiplier);
        multiplier = multiplier << 7;
    } while ((bytes[byte_count++]) & 0x80);

    if ((1 + byte_count + value) > packet_ptr -> nx_packet_length)
    {

        /* Packet is incomplete. */
        /* Remaining length is larger than packet size. */
        return(NXD_MQTT_PARTIAL_PACKET);
    }

    *remaining_length = value;
    *offset_ptr = (1 + byte_count);

    return(NXD_MQTT_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a subscribe or unsubscribe message to the       */
/*    broker.                                                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    op                                    Subscribe or Unsubscribe      */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    packet_id_ptr                         Pointer to packet id that     */
/*                                            will be filled with         */
/*                                            assigned packet id for      */
/*                                            sub/unsub message           */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    _nxd_mqtt_client_set_fixed_header                                   */
/*    _nxd_mqtt_client_append_message                                     */
/*    tx_mutex_put                                                        */
/*    _nxd_mqtt_packet_send                                               */
/*    nx_packet_release                                                   */
/*    _nxd_mqtt_copy_transmit_packet                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                                          */
/*    _nxd_mqtt_client_unsubscribe                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added packet id parameter,  */
/*                                            resulting in version 6.1.2  */
/*  07-29-2022     Spencer McDonough        Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_sub_unsub(NXD_MQTT_CLIENT *client_ptr, UINT op,
                                CHAR *topic_name, UINT topic_name_length,
                                USHORT *packet_id_ptr, UINT QoS)
{


NX_PACKET          *packet_ptr;
NX_PACKET          *transmit_packet_ptr;
UINT                status;
UINT                length = 0;
UINT                ret = NXD_MQTT_SUCCESS;
UCHAR               temp_data[2];

    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(NXD_MQTT_NOT_CONNECTED);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(status);
    }

    /* Compute the remaining length field, starting with 2 bytes of packet ID */
    length = 2;

    /* Count the topic. */
    length += (2 + topic_name_length);

    if (op == ((MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02))
    {
        /* Count one byte for QoS */
        length++;
    }

    /* Write out the control header and remaining length field. */
    ret = _nxd_mqtt_client_set_fixed_header(client_ptr, packet_ptr, (UCHAR )op, length, NX_WAIT_FOREVER);

    if (ret)
    {

        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    temp_data[0] = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier >> 8);
    temp_data[1] = (client_ptr -> nxd_mqtt_client_packet_identifier &  0xFF);

    if (packet_id_ptr)
    {
        *packet_id_ptr = (USHORT)(client_ptr -> nxd_mqtt_client_packet_identifier & 0xFFFF);
    }

    /* Append packet ID. */
    ret = nx_packet_data_append(packet_ptr, temp_data, 2, client_ptr -> nxd_mqtt_client_packet_pool_ptr, NX_WAIT_FOREVER);

    if (ret)
    {

        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    /* Append topic name */
    ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, topic_name, topic_name_length, NX_WAIT_FOREVER);

    if (ret)
    {

        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    if (op == ((MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02))
    {
        /* Fill in QoS value. */
        temp_data[0] = QoS & 0x3;

        ret = nx_packet_data_append(packet_ptr, temp_data, 1, client_ptr -> nxd_mqtt_client_packet_pool_ptr, NX_WAIT_FOREVER);

        if (ret)
        {

            /* Release the mutex. */
            tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            return(NXD_MQTT_PACKET_POOL_FAILURE);
        }
    }

    /* Copy packet for retransmission. */
    if (_nxd_mqtt_copy_transmit_packet(client_ptr, packet_ptr, &transmit_packet_ptr,
                                       (USHORT)(client_ptr -> nxd_mqtt_client_packet_identifier),
                                       NX_FALSE, NX_WAIT_FOREVER))
    {
        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    if (client_ptr -> message_transmit_queue_head == NX_NULL)
    {
        client_ptr -> message_transmit_queue_head = transmit_packet_ptr;
    }
    else
    {
        client_ptr -> message_transmit_queue_tail -> nx_packet_queue_next = transmit_packet_ptr;
    }
    client_ptr -> message_transmit_queue_tail = transmit_packet_ptr;

    client_ptr -> nxd_mqtt_client_packet_identifier = (client_ptr -> nxd_mqtt_client_packet_identifier + 1) & 0xFFFF;

    /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
    if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
        client_ptr -> nxd_mqtt_client_packet_identifier = 1;

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    /* Release the mutex. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* Ready to send the connect message to the server. */
    status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, NX_WAIT_FOREVER);

    if (status)
    {
        /* Release the packet. */
        nx_packet_release(packet_ptr);

        ret = NXD_MQTT_COMMUNICATION_FAILURE;
    }

    return(ret);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_allocate                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for transmitting MQTT message.     */
/*    Special care has to be taken for accommodating IPv4/IPv6 header,    */
/*    and possibly TLS record if TLS is being used. On failure, the       */
/*    TLS mutex is released and the caller can simply return.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Allocated packet to be        */
/*                                            returned to the caller.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_packet_allocate         Allocate packet for MQTT      */
/*                                            over TLS socket             */
/*    nx_packet_allocate                    Allocate a packet for MQTT    */
/*                                            over regular TCP socket     */
/*    tx_mutex_put                          Release a mutex               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_client_publish                                            */
/*    _nxd_mqtt_client_subscribe                                          */
/*    _nxd_mqtt_client_unsubscribe                                        */
/*    _nxd_mqtt_client_send_simple_message                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Spencer McDonough        Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_packet_allocate(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{
UINT status = NXD_MQTT_SUCCESS;

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {

        /* Use WebSocket packet allocate since it is able to count for WebSocket-related header space */
        status = nx_websocket_client_packet_allocate(&(client_ptr -> nxd_mqtt_client_websocket), packet_ptr, wait_option);
    }
    else
#endif /* NXD_MQTT_OVER_WEBSOCKET */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        /* Use TLS packet allocate.  The TLS packet allocate is able to count for 
           TLS-related header space including crypto initial vector area. */
        status = nx_secure_tls_packet_allocate(&client_ptr -> nxd_mqtt_tls_session, client_ptr -> nxd_mqtt_client_packet_pool_ptr,
                                               packet_ptr, wait_option);
    }
    /* Allocate a packet  */
    else
    {
#endif /* NX_SECURE_ENABLE */
        if (client_ptr -> nxd_mqtt_client_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            status = nx_packet_allocate(client_ptr -> nxd_mqtt_client_packet_pool_ptr, packet_ptr, NX_IPv4_TCP_PACKET,
                                        wait_option);
        }
        else
        {
            status = nx_packet_allocate(client_ptr -> nxd_mqtt_client_packet_pool_ptr, packet_ptr, NX_IPv6_TCP_PACKET,
                                        wait_option);
        }
#ifdef NX_SECURE_ENABLE
    }
#endif /* NX_SECURE_ENABLE */

    if (status != NX_SUCCESS)
    {
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_send                               PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends out a packet.                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to packet             */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_websocket_client_send                                            */
/*    nx_secure_tls_session_send                                          */
/*    nx_tcp_socket_send                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_packet_send(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT wait_option)
{
UINT status = NXD_MQTT_SUCCESS;

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
        status = nx_websocket_client_send(&(client_ptr -> nxd_mqtt_client_websocket), packet_ptr, NX_WEBSOCKET_OPCODE_BINARY_FRAME, NX_TRUE, wait_option);
    }
    else
#endif /* NXD_MQTT_OVER_WEBSOCKET */

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, wait_option);
    }
    else
#endif /* NX_SECURE_ENABLE */
    {
        status = nx_tcp_socket_send(&client_ptr -> nxd_mqtt_client_socket, packet_ptr, wait_option);
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_receive                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a packet.                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to packet             */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_websocket_client_receive                                         */
/*    nx_secure_tls_session_receive                                       */
/*    nx_tcp_socket_receive                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_packet_receive(NXD_MQTT_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT wait_option)
{
UINT status = NXD_MQTT_SUCCESS;
#ifdef NXD_MQTT_OVER_WEBSOCKET
UINT op_code = 0;
#endif /* NXD_MQTT_OVER_WEBSOCKET*/

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
        status = nx_websocket_client_receive(&(client_ptr -> nxd_mqtt_client_websocket), packet_ptr, &op_code, wait_option);
        if ((status == NX_SUCCESS) && (op_code != NX_WEBSOCKET_OPCODE_BINARY_FRAME))
        {
            return(NX_INVALID_PACKET);
        }
    }
    else
#endif /* NXD_MQTT_OVER_WEBSOCKET */

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_receive(&(client_ptr -> nxd_mqtt_tls_session), packet_ptr, wait_option);
    }
    else
#endif /* NX_SECURE_ENABLE */
    {
        status = nx_tcp_socket_receive(&(client_ptr -> nxd_mqtt_client_socket), packet_ptr, wait_option);
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_tcp_establish_notify                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is installed as TCP connection establish     */
/*    callback function.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            The socket that receives      */
/*                                           the message.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_tcp_establish_notify(NX_TCP_SOCKET *socket_ptr)
{
NXD_MQTT_CLIENT *client_ptr;

    client_ptr = (NXD_MQTT_CLIENT *)socket_ptr -> nx_tcp_socket_reserved_ptr;

    if (&(client_ptr -> nxd_mqtt_client_socket) == socket_ptr)
    {

        /* Set the event flag. */
#ifndef NXD_MQTT_CLOUD_ENABLE
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_TCP_ESTABLISH_EVENT, TX_OR);
#else
        nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_TCP_ESTABLISH_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_receive_callback                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is installed as TCP receive callback         */
/*    function.  On receiving a TCP message, the callback function        */
/*    sets an event flag to trigger MQTT client to process received       */
/*    message.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            The socket that receives      */
/*                                           the message.                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_receive_callback(NX_TCP_SOCKET *socket_ptr)
{
NXD_MQTT_CLIENT *client_ptr;

    client_ptr = (NXD_MQTT_CLIENT *)socket_ptr -> nx_tcp_socket_reserved_ptr;

    if (&(client_ptr -> nxd_mqtt_client_socket) == socket_ptr)
    {
        /* Set the event flag. */
#ifndef NXD_MQTT_CLOUD_ENABLE
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_PACKET_RECEIVE_EVENT, TX_OR);
#else
        nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_PACKET_RECEIVE_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_copy_transmit_packet                      PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function saves a transmit packet.                     */
/*    A transmit packet is allocated to store QoS 1 and 2 messages.       */
/*    Upon a message being properly acknowledged, the packet will         */
/*    be released.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the MQTT message   */
/*                                            packet to be saved          */
/*    new_packet_ptr                        Return a copied packet        */
/*    packet_id                             Current packet ID             */
/*    set_duplicate_flag                    Set duplicate flag for fixed  */
/*                                            header or not               */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_copy                                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                                          */
/*    _nxd_mqtt_process_publish                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            supported maximum transmit  */
/*                                            queue depth,                */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_copy_transmit_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET **new_packet_ptr,
                                           USHORT packet_id, UCHAR set_duplicate_flag, UINT wait_option)
{
UINT status;

#ifdef NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH
    if (client_ptr -> message_transmit_queue_depth >= NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH)
    {

        /* Hit the transmit queue depeth. No more packets should be queued. */
        return(NX_TX_QUEUE_DEPTH);
    }
#endif /* NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH */

    /* Copy current packet. */
    status = nx_packet_copy(packet_ptr, new_packet_ptr, client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);
    if (status)
    {
        
        /* No available packet to be stored. */
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    /* Save packet_id at the beginning of packet. */
    *((USHORT *)(*new_packet_ptr) -> nx_packet_data_start) = packet_id;

    if (set_duplicate_flag)
    {
        
        /* Update duplicate flag in fixed header. */
        *((*new_packet_ptr) -> nx_packet_prepend_ptr) = (*((*new_packet_ptr) -> nx_packet_prepend_ptr)) | MQTT_PUBLISH_DUP_FLAG;
    }

#ifdef NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH
    /* Increase the transmit queue depth.  */
    client_ptr -> message_transmit_queue_depth++;
#endif /* NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH */

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_release_transmit_packet                   PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function releases a transmit packet.                  */
/*    A transmit packet is allocated to store QoS 1 and 2 messages.       */
/*    Upon a message being properly acknowledged, the packet can          */
/*    be released.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the MQTT message   */
/*                                            packet to be removed        */
/*    previous_packet_ptr                   Pointer to the previous packet*/
/*                                            or NULL if none exists      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            supported maximum transmit  */
/*                                            queue depth,                */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_release_transmit_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET *previous_packet_ptr)
{

    if (previous_packet_ptr)
    {
        previous_packet_ptr -> nx_packet_queue_next = packet_ptr -> nx_packet_queue_next;
    }
    else
    {
        client_ptr -> message_transmit_queue_head = packet_ptr -> nx_packet_queue_next;
    }

    if (packet_ptr == client_ptr -> message_transmit_queue_tail)
    {
        client_ptr -> message_transmit_queue_tail = previous_packet_ptr;
    }
    nx_packet_release(packet_ptr);

#ifdef NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH
    client_ptr -> message_transmit_queue_depth--;
#endif /* NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_release_receive_packet                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function releases a receive packet.                   */
/*    A receive packet is allocated to store QoS 1 and 2 messages.        */
/*    Upon a message being properly acknowledged, the packet can          */
/*    be released.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the MQTT message   */
/*                                            packet to be removed        */
/*    previous_packet_ptr                   Pointer to the previous packet*/
/*                                            or NULL if none exists      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                                              */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_release_receive_packet(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_PACKET *previous_packet_ptr)
{

    if (previous_packet_ptr)
    {
        previous_packet_ptr -> nx_packet_queue_next = packet_ptr -> nx_packet_queue_next;
    }
    else
    {
        client_ptr -> message_receive_queue_head = packet_ptr -> nx_packet_queue_next;
    }

    if (packet_ptr == client_ptr -> message_receive_queue_tail)
    {
        client_ptr -> message_receive_queue_tail = previous_packet_ptr;
    }

    client_ptr -> message_receive_queue_depth--;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_connack                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes a CONNACK message from the broker. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nxd_mqtt_connect_notify]             User supplied connect         */
/*                                            callback function           */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    _nxd_mqtt_client_retransmit_message                                 */
/*    _nxd_mqtt_client_connection_end                                     */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_connack(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT    ret = NXD_MQTT_COMMUNICATION_FAILURE;
MQTT_PACKET_CONNACK *connack_packet_ptr = (MQTT_PACKET_CONNACK *)(packet_ptr -> nx_packet_prepend_ptr);


    /* Check the length.  */
    if ((packet_ptr -> nx_packet_length != sizeof(MQTT_PACKET_CONNACK)) ||
        (connack_packet_ptr -> mqtt_connack_packet_header >> 4 != MQTT_CONTROL_PACKET_TYPE_CONNACK))
    {
        /* Invalid packet length.  Free the packet and process error. */
        ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
    }
    else
    {

        /* Check remaining length.  */
        if (connack_packet_ptr -> mqtt_connack_packet_remaining_length != 2)
        {
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        /* Follow MQTT-3.2.2-1 rule.  */
        else if ((client_ptr -> nxd_mqtt_clean_session) && (connack_packet_ptr -> mqtt_connack_packet_ack_flags & MQTT_CONNACK_CONNECT_FLAGS_SP))
        {

            /* Client requested clean session, and server responded with Session Present.  This is a violation. */
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        else if (connack_packet_ptr -> mqtt_connack_packet_return_code >  MQTT_CONNACK_CONNECT_RETURN_CODE_NOT_AUTHORIZED)
        {
            ret = NXD_MQTT_SERVER_MESSAGE_FAILURE;
        }
        else if (connack_packet_ptr -> mqtt_connack_packet_return_code > 0)
        {

            /* Pass the server return code to the application. */
            ret = (UINT)(NXD_MQTT_ERROR_CONNECT_RETURN_CODE + connack_packet_ptr -> mqtt_connack_packet_return_code);
        }
        else
        {
            ret = NXD_MQTT_SUCCESS;
            
            /* Obtain mutex before we modify client control block. */
            tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

            client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_CONNECTED;

            /* Initialize the packet identification field. */
            client_ptr -> nxd_mqtt_client_packet_identifier = NXD_MQTT_INITIAL_PACKET_ID_VALUE;
            
            /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
            if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
                client_ptr -> nxd_mqtt_client_packet_identifier = 1;

            /* Release mutex */
            tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        }
    }

    /* Check callback function.  */
    if (client_ptr -> nxd_mqtt_connect_notify)
    {
        client_ptr -> nxd_mqtt_connect_notify(client_ptr, ret, client_ptr -> nxd_mqtt_connect_context);
    }

    if (ret == NXD_MQTT_SUCCESS)
    {

        /* If client doesn't start with Clean Session, and there are un-acked PUBLISH messages,
           we shall re-publish these messages. */
        if ((client_ptr -> nxd_mqtt_clean_session != NX_TRUE) && (client_ptr -> message_transmit_queue_head))
        {

            tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

            /* There are messages from the previous session that has not been acknowledged. */
            ret = _nxd_mqtt_client_retransmit_message(client_ptr, wait_option);

            /* Release mutex */
            tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        }
    }
    else
    {

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);
    }

    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_publish_packet                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes a packet and parses topic and      */
/*    message.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to the packet         */
/*    topic_offset_ptr                      Return topic offset           */
/*    topic_length_ptr                      Return topic length           */
/*    message_offset_ptr                    Return message offset         */
/*    message_length_ptr                    Return message length         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_read_remaining_length                                     */
/*    nx_packet_data_extract_offset                                       */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_process_publish                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_process_publish_packet(NX_PACKET *packet_ptr, ULONG *topic_offset_ptr, USHORT *topic_length_ptr,
                                      ULONG *message_offset_ptr, ULONG *message_length_ptr)
{
UCHAR  QoS;
UINT   remaining_length = 0;
UINT   topic_length;
ULONG  offset;
UCHAR  bytes[2];
ULONG  bytes_copied;


    QoS = (UCHAR)((*(packet_ptr -> nx_packet_prepend_ptr) & MQTT_PUBLISH_QOS_LEVEL_FIELD) >> 1);

    if (_nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &offset))
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    if (remaining_length < 2)
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    /* Get topic length fields. */
    if (nx_packet_data_extract_offset(packet_ptr, offset, &bytes, sizeof(bytes), &bytes_copied) ||
        (bytes_copied != sizeof(bytes)))
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    topic_length = (UINT)(*(bytes) << 8) | (*(bytes + 1));

    if (topic_length > remaining_length - 2u)
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    *topic_offset_ptr = offset + 2;
    *topic_length_ptr = (USHORT)topic_length;

    remaining_length = remaining_length - topic_length - 2;
    if ((QoS == 1) || (QoS == 2))
    {
        offset += 2 + 2 + topic_length;

        if (remaining_length < 2)
        {
            return(NXD_MQTT_INVALID_PACKET);
        }
        remaining_length = remaining_length - 2;
    }
    else
    {
        offset += 2 + topic_length;
    }

    *message_offset_ptr = offset;
    *message_length_ptr = (ULONG)remaining_length;

    /* Return */
    return(NXD_MQTT_SUCCESS);
}
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_publish                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a publish message from the broker.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_TRUE - packet is consumed                                        */
/*    NX_FALSE - packet is not consumed                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [receive_notify]                      User supplied receive         */
/*                                            callback function           */
/*    _nxd_mqtt_packet_allocate                                           */
/*    _nxd_mqtt_packet_send                                               */
/*    nx_packet_release                                                   */
/*    nx_secure_tls_session_send                                          */
/*    _nxd_mqtt_process_publish_packet                                    */
/*    _nxd_mqtt_copy_transmit_packet                                      */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_publish(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{
MQTT_PACKET_PUBLISH_RESPONSE *pubresp_ptr;
UINT                          status;
USHORT                        packet_id = 0;
UCHAR                         QoS;
UINT                          enqueue_message = 0;
NX_PACKET                    *transmit_packet_ptr;
UINT                          remaining_length = 0;
UINT                          packet_consumed = NX_FALSE;
UCHAR                         fixed_header;
USHORT                        transmit_packet_id;
UINT                          topic_length;
ULONG                         offset;
UCHAR                         bytes[2];
ULONG                         bytes_copied;

    QoS = (UCHAR)((*(packet_ptr -> nx_packet_prepend_ptr) & MQTT_PUBLISH_QOS_LEVEL_FIELD) >> 1);

    if (_nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &offset))
    {
        return(NX_FALSE);
    }

    if (remaining_length < 2)
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    /* Get topic length fields. */
    if (nx_packet_data_extract_offset(packet_ptr, offset, &bytes, sizeof(bytes), &bytes_copied) ||
        (bytes_copied != sizeof(bytes)))
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    topic_length = (UINT)(*(bytes) << 8) | (*(bytes + 1));

    if (topic_length > remaining_length - 2u)
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    if (QoS == 0)
    {
        enqueue_message = 1;
    }
    else
    {
        /* QoS 1 or QoS 2 messages. */
        /* Get packet id fields. */
        if (nx_packet_data_extract_offset(packet_ptr, offset + 2 + topic_length, &bytes, sizeof(bytes), &bytes_copied))
        {
            return(NXD_MQTT_INVALID_PACKET);
        }

        packet_id = (USHORT)(((*bytes) << 8) | (*(bytes + 1)));

        /* Look for an existing transmit packets with the same packet id */
        transmit_packet_ptr = client_ptr -> message_transmit_queue_head;

        while (transmit_packet_ptr)
        {
            fixed_header = *(transmit_packet_ptr -> nx_packet_prepend_ptr);
            transmit_packet_id = *((USHORT *)transmit_packet_ptr -> nx_packet_data_start);
            if ((transmit_packet_id == packet_id) &&
                ((fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_PUBREC << 4)))
            {

                /* Found a packet containing the packet_id */
                break;
            }
            transmit_packet_ptr = transmit_packet_ptr -> nx_packet_queue_next;
        }

        if (transmit_packet_ptr)
        {
            /* This published data is already in our system.  No need to deliver this message to the application. */
            enqueue_message = 0;
        }
        else
        {
            enqueue_message = 1;
        }
    }

    if (enqueue_message)
    {
        if (packet_ptr -> nx_packet_length > (offset + remaining_length))
        {

            /* This packet contains multiple messages. */
            if (nx_packet_copy(packet_ptr, &packet_ptr, client_ptr -> nxd_mqtt_client_packet_pool_ptr, NX_NO_WAIT))
            {

                /* No packet is available. */
                return(NX_FALSE);
            }
        }
        else
        {
            packet_consumed = NX_TRUE;
        }

        /* Increment the queue depth counter. */
        client_ptr -> message_receive_queue_depth++;

        if (client_ptr -> message_receive_queue_head == NX_NULL)
        {
            client_ptr -> message_receive_queue_head = packet_ptr;
        }
        else
        {
            client_ptr -> message_receive_queue_tail -> nx_packet_queue_next = packet_ptr;
        }
        client_ptr -> message_receive_queue_tail = packet_ptr;

        /* Invoke the user-defined receive notify function if it is set. */
        if (client_ptr -> nxd_mqtt_client_receive_notify)
        {
            (*(client_ptr -> nxd_mqtt_client_receive_notify))(client_ptr, client_ptr -> message_receive_queue_depth);
        }
    }

    /* If the message QoS level is 0, we are done. */
    if (QoS == 0)
    {
        /* Return */
        return(packet_consumed);
    }

    /* Send out proper ACKs for QoS 1 and 2 messages. */
    /* Allocate a new packet so we can send out a response. */
    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        /* Packet allocation fails. */
        return(packet_consumed);
    }

    /* Fill in the packet ID */
    pubresp_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)(packet_ptr -> nx_packet_prepend_ptr);
    pubresp_ptr -> mqtt_publish_response_packet_remaining_length = 2;
    pubresp_ptr -> mqtt_publish_response_packet_packet_identifier_msb = (UCHAR)(packet_id >> 8);
    pubresp_ptr -> mqtt_publish_response_packet_packet_identifier_lsb = (UCHAR)(packet_id & 0xFF);

    if (QoS == 1)
    {

        pubresp_ptr -> mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBACK << 4;
    }
    else
    {
        pubresp_ptr -> mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBREC << 4;
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + sizeof(MQTT_PACKET_PUBLISH_RESPONSE);
    packet_ptr -> nx_packet_length = sizeof(MQTT_PACKET_PUBLISH_RESPONSE);

    if (QoS == 2)
    {

        /* Copy packet for checking duplicate publish packet. */
        if (_nxd_mqtt_copy_transmit_packet(client_ptr, packet_ptr, &transmit_packet_ptr,
                                           packet_id, NX_FALSE, NX_WAIT_FOREVER))
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);
            return(packet_consumed);
        }
        if (client_ptr -> message_transmit_queue_head == NX_NULL)
        {
            client_ptr -> message_transmit_queue_head = transmit_packet_ptr;
        }
        else
        {
            client_ptr -> message_transmit_queue_tail -> nx_packet_queue_next = transmit_packet_ptr;
        }
        client_ptr -> message_transmit_queue_tail = transmit_packet_ptr;
    }

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* Send packet to server.  */
    status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, NX_WAIT_FOREVER);

    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);
    if (status)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);
    }
    else
    {
        /* Update the timeout value. */
        client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;
    }

    /* Return */
    return(packet_consumed);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_publish_response                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a publish response messages.         */
/*    Publish Response messages are: PUBACK, PUBREC, PUBREL               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nxd_mqtt_client_receive_notify]      User supplied publish         */
/*                                            callback function           */
/*    _nxd_mqtt_release_transmit_packet                                   */
/*    _nxd_mqtt_packet_send                                               */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added ack receive notify,   */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_publish_response(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{
MQTT_PACKET_PUBLISH_RESPONSE *response_ptr;
USHORT                        packet_id;
NX_PACKET                    *previous_packet_ptr;
NX_PACKET                    *transmit_packet_ptr;
NX_PACKET                    *response_packet;
UINT                          ret;
UCHAR                         fixed_header;
USHORT                        transmit_packet_id;

    response_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Validate the packet. */
    if (response_ptr -> mqtt_publish_response_packet_remaining_length != 2)
    {
        /* Invalid remaining_length value. Return 1 so the caller can release
           the packet. */

        return(1);
    }

    packet_id = (USHORT)((response_ptr -> mqtt_publish_response_packet_packet_identifier_msb << 8) |
                         (response_ptr -> mqtt_publish_response_packet_packet_identifier_lsb));

    /* Search all the outstanding transmitted packets for a match. */
    previous_packet_ptr = NX_NULL;
    transmit_packet_ptr = client_ptr -> message_transmit_queue_head;
    while (transmit_packet_ptr)
    {
        fixed_header = *(transmit_packet_ptr -> nx_packet_prepend_ptr);
        transmit_packet_id = *((USHORT *)transmit_packet_ptr -> nx_packet_data_start);
        if (transmit_packet_id == packet_id)
        {

            /* Found the matching packet id */
            if (((response_ptr -> mqtt_publish_response_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_PUBACK)
            {

                /* PUBACK is the response to a PUBLISH packet with QoS Level 1*/
                /* Therefore we verify that packet contains PUBLISH packet with QoS level 1*/
                if ((fixed_header & 0xF6) == ((MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | MQTT_PUBLISH_QOS_LEVEL_1))
                {

                    /* Check ack notify function.  */
                    if (client_ptr -> nxd_mqtt_ack_receive_notify)
                    {

                        /* Call notify function. Note: user routine should not release the packet.  */
                        client_ptr -> nxd_mqtt_ack_receive_notify(client_ptr, MQTT_CONTROL_PACKET_TYPE_PUBACK, packet_id, transmit_packet_ptr, client_ptr -> nxd_mqtt_ack_receive_context);
                    }

                    /* QoS Level1 message receives an ACK. */
                    /* This message can be released. */
                    _nxd_mqtt_release_transmit_packet(client_ptr, transmit_packet_ptr, previous_packet_ptr);

                    /* Return with value 1, so the caller will release packet_ptr */
                    return(1);
                }
            }
            else if (((response_ptr -> mqtt_publish_response_packet_header) >> 4) == MQTT_CONTROL_PACKET_TYPE_PUBREL)
            {

                /* QoS 2 publish Release received, part 2. */
                /* Therefore we verify that packet contains PUBLISH packet with QoS level 2*/
                if ((fixed_header & 0xF6) == (MQTT_CONTROL_PACKET_TYPE_PUBREC << 4))
                {

                    /* QoS Level2 message receives an ACK. */
                    /* This message can be released. */
                    /* Send PUBCOMP */

                    /* Allocate a packet to send the response. */
                    ret = _nxd_mqtt_packet_allocate(client_ptr, &response_packet, NX_WAIT_FOREVER);
                    if (ret)
                    {
                        return(1);
                    }

                    if (4u > ((ULONG)(response_packet -> nx_packet_data_end) - (ULONG)(response_packet -> nx_packet_append_ptr)))
                    {
                        nx_packet_release(response_packet);

                        /* Packet buffer is too small to hold the message. */
                        return(NX_SIZE_ERROR);
                    }

                    response_ptr = (MQTT_PACKET_PUBLISH_RESPONSE *)response_packet -> nx_packet_prepend_ptr;

                    response_ptr ->  mqtt_publish_response_packet_header = MQTT_CONTROL_PACKET_TYPE_PUBCOMP << 4;
                    response_ptr ->  mqtt_publish_response_packet_remaining_length = 2;

                    /* Fill in packet ID */
                    response_packet -> nx_packet_prepend_ptr[3] = packet_ptr -> nx_packet_prepend_ptr[3];
                    response_packet -> nx_packet_prepend_ptr[4] = packet_ptr -> nx_packet_prepend_ptr[4];
                    response_packet -> nx_packet_append_ptr = response_packet -> nx_packet_prepend_ptr + 4;
                    response_packet -> nx_packet_length = 4;

                    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

                    /* Send packet to server.  */
                    ret = _nxd_mqtt_packet_send(client_ptr, response_packet, NX_WAIT_FOREVER);

                    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);

                    /* Update the timeout value. */
                    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

                    if (ret)
                    {
                        nx_packet_release(response_packet);
                    }

                    /* Check ack notify function.  */
                    if (client_ptr -> nxd_mqtt_ack_receive_notify)
                    {

                        /* Call notify function. Note: user routine should not release the packet.  */
                        client_ptr -> nxd_mqtt_ack_receive_notify(client_ptr, MQTT_CONTROL_PACKET_TYPE_PUBREL, packet_id, transmit_packet_ptr, client_ptr -> nxd_mqtt_ack_receive_context);
                    }

                    /* This packet can be released. */
                    _nxd_mqtt_release_transmit_packet(client_ptr, transmit_packet_ptr, previous_packet_ptr);

                    /* Return with value 1, so the caller will release packet_ptr */
                    return(1);
                }
            }
        }

        /* Move on to the next packet */
        previous_packet_ptr = transmit_packet_ptr;
        transmit_packet_ptr = transmit_packet_ptr -> nx_packet_queue_next;
    }

    /* nothing is found.  Return 1 to release the packet.*/
    return(1);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_sub_unsub_ack                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process an ACK message for subscribe         */
/*    or unsubscribe request.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nxd_mqtt_client_receive_notify]      User supplied publish         */
/*                                            callback function           */
/*    _nxd_mqtt_release_transmit_packet                                   */
/*                                          Release the memory block      */
/*    _nxd_mqtt_read_remaining_length       Skip the remaining length     */
/*                                            field                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added ack receive notify,   */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_process_sub_unsub_ack(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{

USHORT     packet_id;
NX_PACKET *previous_packet_ptr;
NX_PACKET *transmit_packet_ptr;
UCHAR      response_header;
UCHAR      fixed_header;
USHORT     transmit_packet_id;
UINT       remaining_length;
ULONG      offset;
UCHAR      bytes[2];
ULONG      bytes_copied;


    response_header = *(packet_ptr -> nx_packet_prepend_ptr);

    if (_nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &offset))
    {

        /* Unable to process the sub/unsub ack.  Simply return and release the packet. */
        return(NXD_MQTT_INVALID_PACKET);
    }

    /* Get packet id fields. */
    if (nx_packet_data_extract_offset(packet_ptr, offset, &bytes, sizeof(bytes), &bytes_copied) ||
        (bytes_copied != sizeof(bytes)))
    {
        return(NXD_MQTT_INVALID_PACKET);
    }

    packet_id = (USHORT)(((*bytes) << 8) | (*(bytes + 1)));

    /* Search all the outstanding transmitted packets for a match. */
    previous_packet_ptr = NX_NULL;
    transmit_packet_ptr = client_ptr -> message_transmit_queue_head;
    while (transmit_packet_ptr)
    {
        fixed_header = *(transmit_packet_ptr -> nx_packet_prepend_ptr);
        transmit_packet_id = *((USHORT *)transmit_packet_ptr -> nx_packet_data_start);
        if (transmit_packet_id == packet_id)
        {

            /* Found the matching packet id */
            if (((response_header >> 4) == MQTT_CONTROL_PACKET_TYPE_SUBACK) &&
                ((fixed_header >> 4) == MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE))
            {
                /* Validate the packet. */
                if (remaining_length != 3)
                {
                    /* Invalid remaining_length value. */
                    return(1);
                }

                /* Check ack notify function.  */
                if (client_ptr -> nxd_mqtt_ack_receive_notify)
                {

                    /* Call notify function. Note: user routine should not release the packet.  */
                    client_ptr -> nxd_mqtt_ack_receive_notify(client_ptr, MQTT_CONTROL_PACKET_TYPE_SUBACK, packet_id, transmit_packet_ptr, client_ptr -> nxd_mqtt_ack_receive_context);
                }

                /* Release the transmit packet. */
                _nxd_mqtt_release_transmit_packet(client_ptr, transmit_packet_ptr, previous_packet_ptr);

                return(1);
            }
            else if (((response_header >> 4) == MQTT_CONTROL_PACKET_TYPE_UNSUBACK) &&
                     ((fixed_header >> 4) == MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE))
            {
                /* Validate the packet. */
                if (remaining_length != 2)
                {
                    /* Invalid remaining_length value. */
                    return(1);
                }

                /* Check ack notify function.  */
                if (client_ptr -> nxd_mqtt_ack_receive_notify)
                {

                    /* Call notify function. Note: user routine should not release the packet.  */
                    client_ptr -> nxd_mqtt_ack_receive_notify(client_ptr, MQTT_CONTROL_PACKET_TYPE_UNSUBACK, packet_id, transmit_packet_ptr, client_ptr -> nxd_mqtt_ack_receive_context);
                }

                /* Unsubscribe succeeded. */
                /* Release the transmit packet. */
                _nxd_mqtt_release_transmit_packet(client_ptr, transmit_packet_ptr, previous_packet_ptr);

                return(1);
            }
        }

        /* Move on to the next packet */
        previous_packet_ptr = transmit_packet_ptr;
        transmit_packet_ptr = transmit_packet_ptr -> nx_packet_queue_next;
    }
    return(1);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_pingresp                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a PINGRESP message.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                            callback function           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_process_pingresp(NXD_MQTT_CLIENT *client_ptr)
{


    /* If there is an outstanding ping, mark it as responded. */
    if (client_ptr -> nxd_mqtt_ping_not_responded == NX_TRUE)
    {
        client_ptr -> nxd_mqtt_ping_not_responded = NX_FALSE;

        client_ptr -> nxd_mqtt_ping_sent_time = 0;
    }

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_process_disconnect                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process a DISCONNECT message.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   nx_secure_tls_session_send                                           */
/*   nx_tcp_socket_disconnect                                             */
/*   nx_tcp_client_socket_unbind                                          */
/*   _nxd_mqtt_release_transmit_packet                                    */
/*   _nxd_mqtt_release_receive_packet                                     */
/*   _nxd_mqtt_client_connection_end                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   _nxd_mqtt_packet_receive_process                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_process_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
NX_PACKET  *previous = NX_NULL;
NX_PACKET  *current;
NX_PACKET  *next;
UINT        disconnect_callback = NX_FALSE;
UINT        status;
UCHAR       fixed_header;

    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        /* State changes from CONNECTED TO IDLE.  Call disconnect notify callback
           if the function is set. */
        disconnect_callback = NX_TRUE;
    }
    else if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTING)
    {

        /* If state isn't CONNECTED or CONNECTING, just return. */
        return;
    }

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* End connection. */
    _nxd_mqtt_client_connection_end(client_ptr, NXD_MQTT_SOCKET_TIMEOUT);

    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);

    /* Free up sub/unsub packets on the transmit queue. */
    current = client_ptr -> message_transmit_queue_head;

    while (current)
    {
        next = current -> nx_packet_queue_next;
        fixed_header = *(current -> nx_packet_prepend_ptr);

        if (((fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4)) ||
            ((fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE << 4)))
        {
            _nxd_mqtt_release_transmit_packet(client_ptr, current, previous);
        }
        else
        {
            previous = current;
        }
        current = next;
    }

    /* If a callback notification is defined, call it now. */
    if ((disconnect_callback == NX_TRUE) && (client_ptr -> nxd_mqtt_disconnect_notify))
    {
        client_ptr -> nxd_mqtt_disconnect_notify(client_ptr);
    }

    /* If a connect callback notification is defined and is still in connecting stage, call it now. */
    if ((disconnect_callback == NX_FALSE) && (client_ptr -> nxd_mqtt_connect_notify))
    {
        client_ptr -> nxd_mqtt_connect_notify(client_ptr, NXD_MQTT_CONNECT_FAILURE, client_ptr -> nxd_mqtt_connect_context);
    }

    if (status == TX_SUCCESS)
    {
        /* Remove all the packets in the receive queue. */
        while (client_ptr -> message_receive_queue_head)
        {
            _nxd_mqtt_release_receive_packet(client_ptr, client_ptr -> message_receive_queue_head, NX_NULL);
        }
        client_ptr -> message_receive_queue_depth = 0;

        /* Clear the MQTT_PACKET_RECEIVE_EVENT */
#ifndef NXD_MQTT_CLOUD_ENABLE
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, ~MQTT_PACKET_RECEIVE_EVENT, TX_AND);
#else
        nx_cloud_module_event_clear(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_PACKET_RECEIVE_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */
    }

    /* Clear flags if keep alive is enabled. */
    if (client_ptr -> nxd_mqtt_keepalive)
    {
        client_ptr -> nxd_mqtt_ping_not_responded = 0;
        client_ptr -> nxd_mqtt_ping_sent_time = 0;
    }

    /* Clean up the information when disconnecting. */
    client_ptr -> nxd_mqtt_client_username = NX_NULL;
    client_ptr -> nxd_mqtt_client_password = NX_NULL;
    client_ptr -> nxd_mqtt_client_will_topic = NX_NULL;
    client_ptr -> nxd_mqtt_client_will_message = NX_NULL;
    client_ptr -> nxd_mqtt_client_will_qos_retain = 0;

    /* Release current processing packet. */
    if (client_ptr -> nxd_mqtt_client_processing_packet)
    {
        nx_packet_release(client_ptr -> nxd_mqtt_client_processing_packet);
        client_ptr -> nxd_mqtt_client_processing_packet = NX_NULL;
    }

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_packet_receive_process                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function process MQTT message.                        */
/*    NOTE: MQTT Mutex is NOT obtained on entering this function.         */
/*    Therefore it shouldn't hold the mutex when it exists this function. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_receive                                       */
/*    nx_tcp_socket_receive                                               */
/*    _nxd_mqtt_process_publish                                           */
/*    _nxd_mqtt_process_publish_response                                  */
/*    _nxd_mqtt_process_sub_unsub_ack                                     */
/*    _nxd_mqtt_process_pingresp                                          */
/*    _nxd_mqtt_process_disconnect                                        */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_packet_receive_process(NXD_MQTT_CLIENT *client_ptr)
{
NX_PACKET *packet_ptr;
NX_PACKET *previous_packet_ptr;
UINT       status;
UCHAR      packet_type;
UINT       remaining_length;
UINT       packet_consumed;
ULONG      offset;
ULONG      bytes_copied;
ULONG      packet_length;

    for (;;)
    {

        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        /* Make a receive call. */
        status = _nxd_mqtt_packet_receive(client_ptr, &packet_ptr, NX_NO_WAIT);

        if (status != NX_SUCCESS)
        {
            if ((status != NX_NO_PACKET) && (status != NX_CONTINUE))
            {

                /* Network issue. Close the MQTT session. */
#ifndef NXD_MQTT_CLOUD_ENABLE
                tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_NETWORK_DISCONNECT_EVENT, TX_OR);
#else
                nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_NETWORK_DISCONNECT_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */
            }

            break;
        }

        /* Obtain the mutex. */
        tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

        /* Is there a packet waiting for processing? */
        if (client_ptr -> nxd_mqtt_client_processing_packet)
        {

            /* Yes. Link received packet to existing one. */
            if (client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_last)
            {
                client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_last -> nx_packet_next = packet_ptr;
            }
            else
            {
                client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_next = packet_ptr;
            }
            if (packet_ptr -> nx_packet_last)
            {
                client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_last = packet_ptr -> nx_packet_last;
            }
            else
            {
                client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_last = packet_ptr;
            }
            client_ptr -> nxd_mqtt_client_processing_packet -> nx_packet_length += packet_ptr -> nx_packet_length;

            /* Start to process existing packet. */
            packet_ptr = client_ptr -> nxd_mqtt_client_processing_packet;
            client_ptr -> nxd_mqtt_client_processing_packet = NX_NULL;
        }

        /* Check notify function.  */
        if (client_ptr -> nxd_mqtt_packet_receive_notify)
        {

            /* Call notify function. Return NX_TRUE if the packet has been consumed.  */
            if (client_ptr -> nxd_mqtt_packet_receive_notify(client_ptr, packet_ptr, client_ptr -> nxd_mqtt_packet_receive_context) == NX_TRUE)
            {
                continue;
            }
        }

        packet_consumed = NX_FALSE;
        while (packet_ptr)
        {
            /* Parse the incoming packet. */
            status = _nxd_mqtt_read_remaining_length(packet_ptr, &remaining_length, &offset);
            if (status == NXD_MQTT_PARTIAL_PACKET)
            {

                /* We only have partial MQTT message. 
                 * Put it to waiting list for more packets. */
                client_ptr -> nxd_mqtt_client_processing_packet = packet_ptr;
                packet_consumed = NX_TRUE;
                break;
            }
            else if (status)
            {
                
                /* Invalid packet. */
                break;
            }

            /* Get packet type. */
            if (nx_packet_data_extract_offset(packet_ptr, 0, &packet_type, 1, &bytes_copied))
            {

                /* Unable to read packet type. */
                break;
            }

            /* Right shift 4 bits to get the packet type. */
            packet_type = packet_type >> 4;

            /* Process based on packet type. */
            switch (packet_type)
            {
            case MQTT_CONTROL_PACKET_TYPE_CONNECT:
                /* Client does not accept connections.  Nothing needs to be done. */
                break;
            case MQTT_CONTROL_PACKET_TYPE_CONNACK:
                _nxd_mqtt_process_connack(client_ptr, packet_ptr, NX_NO_WAIT);
                break;

            case MQTT_CONTROL_PACKET_TYPE_PUBLISH:
                packet_consumed = _nxd_mqtt_process_publish(client_ptr, packet_ptr);
                break;

            case MQTT_CONTROL_PACKET_TYPE_PUBACK:
            case MQTT_CONTROL_PACKET_TYPE_PUBREL:
                _nxd_mqtt_process_publish_response(client_ptr, packet_ptr);
                break;

            case MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE:
            case MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE:
                /* Client should not process subscribe or unsubscribe message. */
                break;

            case MQTT_CONTROL_PACKET_TYPE_SUBACK:
            case MQTT_CONTROL_PACKET_TYPE_UNSUBACK:
                _nxd_mqtt_process_sub_unsub_ack(client_ptr, packet_ptr);
                break;


            case MQTT_CONTROL_PACKET_TYPE_PINGREQ:
                /* Client is not supposed to receive ping req.  Ignore it. */
                break;

            case MQTT_CONTROL_PACKET_TYPE_PINGRESP:
                _nxd_mqtt_process_pingresp(client_ptr);
                break;

            case MQTT_CONTROL_PACKET_TYPE_DISCONNECT:
                _nxd_mqtt_process_disconnect(client_ptr);
                break;

            /* Publisher sender message type for QoS 2. Not supported. */
            case MQTT_CONTROL_PACKET_TYPE_PUBREC:
            case MQTT_CONTROL_PACKET_TYPE_PUBCOMP:
            default:
                /* Unknown type. */
                break;
            }

            if (packet_consumed)
            {
                break;
            }

            /* Trim current packet. */
            offset += remaining_length;
            packet_length = packet_ptr -> nx_packet_length;
            if (packet_length > offset)
            {

                /* Multiple MQTT message in one packet. */
                packet_length = packet_ptr -> nx_packet_length - offset;
                while ((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) <= offset)
                {
                    offset -= (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);

                    /* Current packet can be released. */
                    previous_packet_ptr = packet_ptr;
                    packet_ptr = packet_ptr -> nx_packet_next;
                    previous_packet_ptr -> nx_packet_next = NX_NULL;
                    nx_packet_release(previous_packet_ptr);
                    if (packet_ptr == NX_NULL)
                    {

                        /* Invalid packet. */
                        break;
                    }
                }

                if (packet_ptr)
                {

                    /* Adjust current packet. */
                    packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + offset;
                    packet_ptr -> nx_packet_length = packet_length;
                }
            }
            else
            {

                /* All messages in current packet is processed. */
                break;
            }
        }

        if (!packet_consumed)
        {
            nx_packet_release(packet_ptr);
        }
    }

    /* No more data in the receive queue.  Return. */

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_tcp_establish_process                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes MQTT TCP connection establish event.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_start                                         */
/*    _nxd_mqtt_client_connection_end                                     */
/*    _nxd_mqtt_client_connect_packet_send                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_tcp_establish_process(NXD_MQTT_CLIENT *client_ptr)
{
UINT       status;


    /* TCP connection is established.  */

    /* If TLS is enabled, start TLS */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        status = nx_secure_tls_session_start(&(client_ptr -> nxd_mqtt_tls_session), &(client_ptr -> nxd_mqtt_client_socket), NX_NO_WAIT);

        if (status != NX_CONTINUE)
        {

            /* End connection. */
            _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

            /* Check callback function.  */
            if (client_ptr -> nxd_mqtt_connect_notify)
            {
                client_ptr -> nxd_mqtt_connect_notify(client_ptr, status, client_ptr -> nxd_mqtt_connect_context);
            }

            return;
        }

        /* TLS in progress.  */
        client_ptr -> nxd_mqtt_tls_in_progress = NX_TRUE;

        return;
    }
#endif /* NX_SECURE_ENABLE */

#ifdef NXD_MQTT_OVER_WEBSOCKET

    /* If using websocket, start websocket connection.  */
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
        status = nx_websocket_client_connect(&(client_ptr -> nxd_mqtt_client_websocket), &(client_ptr -> nxd_mqtt_client_socket),
                                             client_ptr -> nxd_mqtt_client_websocket_host, client_ptr -> nxd_mqtt_client_websocket_host_length,
                                             client_ptr -> nxd_mqtt_client_websocket_uri_path, client_ptr -> nxd_mqtt_client_websocket_uri_path_length,
                                             (UCHAR *)NXD_MQTT_OVER_WEBSOCKET_PROTOCOL, sizeof(NXD_MQTT_OVER_WEBSOCKET_PROTOCOL) - 1,
                                             NX_NO_WAIT);

        if (status != NX_IN_PROGRESS)
        {

            /* End connection. */
            _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

            /* Check callback function.  */
            if (client_ptr -> nxd_mqtt_connect_notify)
            {
                client_ptr -> nxd_mqtt_connect_notify(client_ptr, status, client_ptr -> nxd_mqtt_connect_context);
            }

            return;
        }

        return;
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Start to send MQTT connect packet.  */
    status = _nxd_mqtt_client_connect_packet_send(client_ptr, NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

        /* Check callback function.  */
        if (client_ptr -> nxd_mqtt_connect_notify)
        {
            client_ptr -> nxd_mqtt_connect_notify(client_ptr, status, client_ptr -> nxd_mqtt_connect_context);
        }
    }
}


#ifdef NX_SECURE_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_tls_establish_process                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes TLS connection establish event.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_handshake_process                                    */
/*    _nxd_mqtt_client_connect_packet_send                                */
/*    _nxd_mqtt_client_connection_end                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_tls_establish_process(NXD_MQTT_CLIENT *client_ptr)
{
UINT       status;


    /* Directly call handshake process for async mode. */
    status = _nx_secure_tls_handshake_process(&(client_ptr -> nxd_mqtt_tls_session), NX_NO_WAIT);
    if (status == NX_SUCCESS)
    {

        /* TLS session established.   */
        client_ptr -> nxd_mqtt_tls_in_progress = NX_FALSE;

#ifdef NXD_MQTT_OVER_WEBSOCKET

        /* If using websocket, start websocket connection.  */
        if (client_ptr -> nxd_mqtt_client_use_websocket)
        {
            status = nx_websocket_client_secure_connect(&(client_ptr -> nxd_mqtt_client_websocket), &(client_ptr -> nxd_mqtt_tls_session),
                                                        client_ptr -> nxd_mqtt_client_websocket_host, client_ptr -> nxd_mqtt_client_websocket_host_length,
                                                        client_ptr -> nxd_mqtt_client_websocket_uri_path, client_ptr -> nxd_mqtt_client_websocket_uri_path_length,
                                                        (UCHAR *)NXD_MQTT_OVER_WEBSOCKET_PROTOCOL, sizeof(NXD_MQTT_OVER_WEBSOCKET_PROTOCOL) - 1,
                                                        NX_NO_WAIT);

            if (status == NX_IN_PROGRESS)
            {
                return;
            }
        }
        else
#endif /* NXD_MQTT_OVER_WEBSOCKET */
        {

            /* Start to send MQTT connect packet.  */
            status = _nxd_mqtt_client_connect_packet_send(client_ptr, NX_NO_WAIT);
        }
    }
    else if (status == NX_CONTINUE)
    {
        return;
    }

    /* Check status.  */
    if (status)
    {

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

        /* Check callback function.  */
        if (client_ptr -> nxd_mqtt_connect_notify)
        {
            client_ptr -> nxd_mqtt_connect_notify(client_ptr, status, client_ptr -> nxd_mqtt_connect_context);
        }
    }

    return;
}
#endif /* NX_SECURE_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_append_message                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function writes the message length and message in the outgoing */
/*    MQTT packet.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Outgoing MQTT packet          */
/*    message                               Pointer to the message        */
/*    length                                Length of the message         */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append packet data            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                                          */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_client_publish                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_append_message(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, CHAR *message, UINT length, ULONG wait_option)
{
UINT ret = 0;
UCHAR len[2];

    len[0] = (length >> 8) & 0xFF;
    len[1] = length  & 0xFF;

    /* Append message length field. */
    ret = nx_packet_data_append(packet_ptr, len, 2, client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);

    if (ret)
    {
        return(ret);
    }

    if (length)
    {
        /* Copy the string into the packet. */
        ret = nx_packet_data_append(packet_ptr, message, length, 
                                    client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);
    }

    return(ret);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_connection_end                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used to end the MQTT connection.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_end             End TLS session               */
/*    nx_secure_tls_session_delete          Delete TLS session            */
/*    nx_tcp_socket_disconnect              Close TCP connection          */
/*    nx_tcp_client_socket_unbind           Unbind TCP socket             */
/*    tx_timer_delete                       Delete timer                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_process_disconnect                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID _nxd_mqtt_client_connection_end(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option)
{

    /* Obtain the mutex. */
    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    /* Mark the session as terminated. */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

    /* Release the mutex. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
        nx_websocket_client_disconnect(&(client_ptr -> nxd_mqtt_client_websocket), wait_option);
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {
        nx_secure_tls_session_end(&(client_ptr -> nxd_mqtt_tls_session), wait_option);
        nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
    }
#endif
    nx_tcp_socket_disconnect(&(client_ptr -> nxd_mqtt_client_socket), wait_option);
    nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));

    /* Disable timer if timer has been started. */
    if (client_ptr -> nxd_mqtt_keepalive)
    {
         tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
    }
}


static UINT _nxd_mqtt_send_simple_message(NXD_MQTT_CLIENT *client_ptr, UCHAR header_value);


/* MQTT internal function */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_periodic_timer_entry                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is passed to MQTT client socket create call. */
/*    This callback function notifies MQTT client thread when the TCP     */
/*    connection is lost.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket that    */
/*                                            disconnected.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TCP socket disconnect callback                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_periodic_timer_entry(ULONG client)
{
/* Check if it is time to send out a ping message. */
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)client;

    /* If an outstanding ping response has not been received, and the client exceeds the time waiting for ping response,
       the client shall disconnect from the server. */
    if (client_ptr -> nxd_mqtt_ping_not_responded)
    {
        /* If current time is greater than the ping timeout */
        if ((tx_time_get() - client_ptr -> nxd_mqtt_ping_sent_time) >= client_ptr -> nxd_mqtt_ping_timeout)
        {
            /* Ping timed out.  Need to terminate the connection. */
#ifndef NXD_MQTT_CLOUD_ENABLE
            tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_PING_TIMEOUT_EVENT, TX_OR);
#else
            nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_PING_TIMEOUT_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */

            return;
        }
    }

    /* About to timeout? */
    if ((client_ptr -> nxd_mqtt_timeout - tx_time_get()) <= client_ptr -> nxd_mqtt_timer_value)
    {
        /* Set the flag so the MQTT thread can send the ping. */
#ifndef NXD_MQTT_CLOUD_ENABLE
        tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_TIMEOUT_EVENT, TX_OR);
#else
        nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_TIMEOUT_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */
    }

    /* If keepalive is not enabled, just return. */
    return;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_event_process                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function serves as the entry point for the MQTT       */
/*    client thread.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqtt_client                           Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_release_transmit_packet                                   */
/*    _nxd_mqtt_release_receive_packet                                    */
/*    _nxd_mqtt_send_simple_message                                       */
/*    _nxd_mqtt_process_disconnect                                        */
/*    _nxd_mqtt_packet_receive_process                                    */
/*    tx_timer_delete                                                     */
/*    tx_event_flags_delete                                               */
/*    nx_tcp_socket_delete                                                */
/*    tx_timer_delete                                                     */
/*    tx_mutex_delete                                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   _nxd_mqtt_client_create                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_client_event_process(VOID *mqtt_client, ULONG common_events, ULONG module_own_events)
{
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)mqtt_client;


    /* Obtain the mutex. */
    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);

    /* Process common events.  */
    NX_PARAMETER_NOT_USED(common_events);

    if (module_own_events & MQTT_TIMEOUT_EVENT)
    {
        /* Send out PING only if the client is connected. */
        if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
        {
            _nxd_mqtt_send_simple_message(client_ptr, MQTT_CONTROL_PACKET_TYPE_PINGREQ);
        }
    }

    if (module_own_events & MQTT_TCP_ESTABLISH_EVENT)
    {
        _nxd_mqtt_tcp_establish_process(client_ptr);
    }

    if (module_own_events & MQTT_PACKET_RECEIVE_EVENT)
    {
#ifdef NX_SECURE_ENABLE
        /* TLS in progress on async mode.  */
        if (client_ptr -> nxd_mqtt_tls_in_progress)
        {
            _nxd_mqtt_tls_establish_process(client_ptr);
        }
        else
#endif /* NX_SECURE_ENABLE */

        _nxd_mqtt_packet_receive_process(client_ptr);
    }

    if (module_own_events & MQTT_PING_TIMEOUT_EVENT)
    {
        /* The server/broker didn't respond to our ping request message. Disconnect from the server. */
        _nxd_mqtt_process_disconnect(client_ptr);
    }
    if (module_own_events & MQTT_NETWORK_DISCONNECT_EVENT)
    {
        /* The server closed TCP socket. We shall go through the disconnect code path. */
        _nxd_mqtt_process_disconnect(client_ptr);
    }

    if (module_own_events & MQTT_DELETE_EVENT)
    {

        /* Stop the client and disconnect from the server. */
        if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
        {
            _nxd_mqtt_process_disconnect(client_ptr);
        }

        /* Delete the timer. Check first if it is already deleted. */
        if ((client_ptr -> nxd_mqtt_timer).tx_timer_id != 0)
            tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));

#ifndef NXD_MQTT_CLOUD_ENABLE
        /* Delete the event flag. Check first if it is already deleted. */
        if ((client_ptr -> nxd_mqtt_events).tx_event_flags_group_id != 0)
            tx_event_flags_delete(&client_ptr -> nxd_mqtt_events);
#endif /* NXD_MQTT_CLOUD_ENABLE */

        /* Release all the messages on the receive queue. */
        while (client_ptr -> message_receive_queue_head)
        {
            _nxd_mqtt_release_receive_packet(client_ptr, client_ptr -> message_receive_queue_head, NX_NULL);
        }
        client_ptr -> message_receive_queue_depth = 0;

        /* Delete all the messages sitting in the receive and transmit queue. */
        while (client_ptr -> message_transmit_queue_head)
        {
            _nxd_mqtt_release_transmit_packet(client_ptr, client_ptr -> message_transmit_queue_head, NX_NULL);
        }

        /* Release mutex */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        
#ifndef NXD_MQTT_CLOUD_ENABLE
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);
#endif /* NXD_MQTT_CLOUD_ENABLE */

        /* Deleting the socket, (the socket ID is cleared); this signals it is ok to delete this thread. */
        nx_tcp_socket_delete(&client_ptr -> nxd_mqtt_client_socket);
    }
    else
    {

        /* Release mutex */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
    }
}


#ifndef NXD_MQTT_CLOUD_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_thread_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function serves as the entry point for the MQTT       */
/*    client thread.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    mqtt_client                           Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_get                                                  */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   _nxd_mqtt_client_create                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nxd_mqtt_thread_entry(ULONG mqtt_client)
{
NXD_MQTT_CLIENT *client_ptr;
ULONG            events;

    client_ptr = (NXD_MQTT_CLIENT *)mqtt_client;

    /* Loop to process events on the MQTT client */
    for (;;)
    {

        tx_event_flags_get(&client_ptr -> nxd_mqtt_events, MQTT_ALL_EVENTS, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Call the event processing routine.  */
        _nxd_mqtt_client_event_process(client_ptr, NX_NULL, events);

        if (events & MQTT_DELETE_EVENT)
        {
            break;
        }
    }
}
#endif /* NXD_MQTT_CLOUD_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _mqtt_client_disconnect_callback                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is passed to MQTT client socket create call. */
/*    This callback function notifies MQTT client thread when the TCP     */
/*    connection is lost.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket that    */
/*                                            disconnected.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TCP socket disconnect callback                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _mqtt_client_disconnect_callback(NX_TCP_SOCKET *socket_ptr)
{
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)(socket_ptr -> nx_tcp_socket_reserved_ptr);

    /* Set the MQTT_NETWORK_DISCONNECT  event.  This event indicates
       that the disconnect is initiated from the network. */
#ifndef NXD_MQTT_CLOUD_ENABLE
    tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_NETWORK_DISCONNECT_EVENT, TX_OR);
#else
    nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_NETWORK_DISCONNECT_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_create                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an instance for MQTT Client.  It initializes  */
/*    MQTT Client control block, creates a thread, mutex and event queue  */
/*    for MQTT Client, and creates the TCP socket for MQTT messaging.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_id                             Client ID for this instance   */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    stack_ptr                             Client thread's stack pointer */
/*    stack_size                            Client thread's stack size    */
/*    mqtt_thread_priority                  Priority for MQTT thread      */
/*    memory_ptr                            Deprecated and not used       */
/*    memory_size                           Deprecated and not used       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_create_internal      Create mqtt client            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                             CHAR *client_id, UINT client_id_length,
                             NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                             VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                             VOID *memory_ptr, ULONG memory_size)
{

UINT    status;


    NX_PARAMETER_NOT_USED(memory_ptr);
    NX_PARAMETER_NOT_USED(memory_size);

    /* Create MQTT client.  */
    status = _nxd_mqtt_client_create_internal(client_ptr, client_name, client_id, client_id_length, ip_ptr,
                                              pool_ptr, stack_ptr, stack_size, mqtt_thread_priority);

    /* Check status.  */
    if (status)
    {
        return(status);
    }

#ifdef NXD_MQTT_CLOUD_ENABLE

    /* Create cloud helper.  */
    status = nx_cloud_create(&(client_ptr -> nxd_mqtt_client_cloud), "Cloud Helper", stack_ptr, stack_size, mqtt_thread_priority);

    /* Determine if an error occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Delete internal resource created in _nxd_mqtt_client_create_internal().  */

        /* Delete socket.  */
        nx_tcp_socket_delete(&(client_ptr -> nxd_mqtt_client_socket));

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    /* Save the cloud pointer.  */
    client_ptr -> nxd_mqtt_client_cloud_ptr = &(client_ptr -> nxd_mqtt_client_cloud);

    /* Save the mutex pointer.  */
    client_ptr -> nxd_mqtt_client_mutex_ptr = &(client_ptr -> nxd_mqtt_client_cloud.nx_cloud_mutex);

    /* Register MQTT on cloud helper.  */
    status = nx_cloud_module_register(client_ptr -> nxd_mqtt_client_cloud_ptr, &(client_ptr -> nxd_mqtt_client_cloud_module), client_name, NX_CLOUD_MODULE_MQTT_EVENT,
                                      _nxd_mqtt_client_event_process, client_ptr);

    /* Determine if an error occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Delete own created cloud.  */
        nx_cloud_delete(&(client_ptr -> nxd_mqtt_client_cloud));

        /* Delete internal resource created in _nxd_mqtt_client_create_internal().  */

        /* Delete socket.  */
        nx_tcp_socket_delete(&(client_ptr -> nxd_mqtt_client_socket));

        return(NXD_MQTT_INTERNAL_ERROR);
    }

#endif /* NXD_MQTT_CLOUD_ENABLE */

    /* Update state.  */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

    /* Return.  */
    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_create_internal                    PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an instance for MQTT Client.  It initializes  */
/*    MQTT Client control block, creates a thread, mutex and event queue  */
/*    for MQTT Client, and creates the TCP socket for MQTT messaging.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_id                             Client ID for this instance   */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    stack_ptr                             Client thread's stack pointer */
/*    stack_size                            Client thread's stack size    */
/*    mqtt_thread_priority                  Priority for MQTT thread      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_create                      Create a thread               */
/*    tx_mutex_create                       Create mutex                  */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    tx_event_flag_create                  Create event flag             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*  07-29-2022     Spencer McDonough        Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_client_create_internal(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                                             CHAR *client_id, UINT client_id_length,
                                             NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                                             VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority)
{
#ifndef NXD_MQTT_CLOUD_ENABLE
UINT                status;
#endif /* NXD_MQTT_CLOUD_ENABLE */

#ifdef NXD_MQTT_CLOUD_ENABLE
    NX_PARAMETER_NOT_USED(stack_ptr);
    NX_PARAMETER_NOT_USED(stack_size);
    NX_PARAMETER_NOT_USED(mqtt_thread_priority);
#endif /* NXD_MQTT_CLOUD_ENABLE */

    /* Clear the MQTT Client control block. */
    NXD_MQTT_SECURE_MEMSET((void *)client_ptr, 0, sizeof(NXD_MQTT_CLIENT));

#ifndef NXD_MQTT_CLOUD_ENABLE

    /* Create MQTT mutex.  */
    status = tx_mutex_create(&client_ptr -> nxd_mqtt_protection, client_name, TX_NO_INHERIT);

    /* Determine if an error occurred. */
    if (status != TX_SUCCESS)
    {

        return(NXD_MQTT_INTERNAL_ERROR);
    }
    client_ptr -> nxd_mqtt_client_mutex_ptr = &(client_ptr -> nxd_mqtt_protection);

    /* Now create MQTT client thread */
    status = tx_thread_create(&(client_ptr -> nxd_mqtt_thread), client_name, _nxd_mqtt_thread_entry,
                              (ULONG)client_ptr, stack_ptr, stack_size, mqtt_thread_priority, mqtt_thread_priority,
                              NXD_MQTT_CLIENT_THREAD_TIME_SLICE, TX_DONT_START);

    /* Determine if an error occurred. */
    if (status != TX_SUCCESS)
    {
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

        /* Return error code. */
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    status = tx_event_flags_create(&(client_ptr -> nxd_mqtt_events), client_name);

    if (status != TX_SUCCESS)
    {
        /* Delete the mutex. */
        tx_mutex_delete(&client_ptr -> nxd_mqtt_protection);

        /* Delete the thread. */
        tx_thread_delete(&(client_ptr -> nxd_mqtt_thread));
        
        /* Return error code. */
        return(NXD_MQTT_INTERNAL_ERROR);
    }
#endif /* NXD_MQTT_CLOUD_ENABLE */

    /* Record the client ID information. */
    client_ptr -> nxd_mqtt_client_id = client_id;
    client_ptr -> nxd_mqtt_client_id_length = client_id_length;
    client_ptr -> nxd_mqtt_client_ip_ptr = ip_ptr;
    client_ptr -> nxd_mqtt_client_packet_pool_ptr = pool_ptr;
    client_ptr -> nxd_mqtt_client_name = client_name;

    /* Create the socket. */
    nx_tcp_socket_create(client_ptr -> nxd_mqtt_client_ip_ptr, &(client_ptr -> nxd_mqtt_client_socket), client_ptr -> nxd_mqtt_client_name,
                         NX_IP_NORMAL, NX_DONT_FRAGMENT, 0x80, NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE,
                         NX_NULL, _mqtt_client_disconnect_callback);


    /* Record the client_ptr in the socket structure. */
    client_ptr -> nxd_mqtt_client_socket.nx_tcp_socket_reserved_ptr = (VOID *)client_ptr;

#ifndef NXD_MQTT_CLOUD_ENABLE
    /* Start MQTT thread. */
    tx_thread_resume(&(client_ptr -> nxd_mqtt_thread));
#endif /* NXD_MQTT_CLOUD_ENABLE */
    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_login_set                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets optional MQTT username and password.  Note       */
/*    if the broker requires username and password, this information      */
/*    must be set prior to calling nxd_mqtt_client_connect or             */
/*    nxd_mqtt_client_secure_connect.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                CHAR *username, UINT username_length, CHAR *password, UINT password_length)
{
UINT status;

    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }
    client_ptr -> nxd_mqtt_client_username = username;
    client_ptr -> nxd_mqtt_client_username_length = (USHORT)username_length;
    client_ptr -> nxd_mqtt_client_password = password;
    client_ptr -> nxd_mqtt_client_password_length = (USHORT)password_length;

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_will_message_set                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets optional MQTT will topic and will message.       */
/*    Note that if will message is needed, application must set will      */
/*    message prior to calling nxd_mqtt_client_connect or                 */
/*    nxd_mqtt_client_secure_connect.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    will_topic                            Will topic string.            */
/*    will_topic_length                     Length of the will topic.     */
/*    will_message                          Will message string.          */
/*    will_message_length                   Length of the will message.   */
/*    will_retain_flag                      Whether or not the will       */
/*                                            message is to be retained   */
/*                                            when it is published.       */
/*                                            Valid values are NX_TRUE    */
/*                                            NX_FALSE                    */
/*    will_QoS                              QoS level to be used when     */
/*                                            publishing will message.    */
/*                                            Valid values are 0, 1, 2    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                       const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                       UINT will_message_length, UINT will_retain_flag, UINT will_QoS)
{
UINT status;

    if (will_QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }

    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    client_ptr -> nxd_mqtt_client_will_topic = will_topic;
    client_ptr -> nxd_mqtt_client_will_topic_length = will_topic_length;
    client_ptr -> nxd_mqtt_client_will_message = will_message;
    client_ptr -> nxd_mqtt_client_will_message_length = will_message_length;

    if (will_retain_flag == NX_TRUE)
    {
        client_ptr -> nxd_mqtt_client_will_qos_retain = 0x80;
    }
    client_ptr -> nxd_mqtt_client_will_qos_retain = (UCHAR)(client_ptr -> nxd_mqtt_client_will_qos_retain | will_QoS);

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_will_message_set                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the                              */
/*    nxd_mqtt_client_will_message_set call.                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    will_topic                            Will topic string.            */
/*    will_topic_length                     Length of the will topic.     */
/*    will_message                          Will message string.          */
/*    will_message_length                   Length of the will message.   */
/*    will_retain_flag                      Whether or not the will       */
/*                                            message is to be retained   */
/*                                            when it is published.       */
/*                                            Valid values are NX_TRUE    */
/*                                            NX_FALSE                    */
/*    will_QoS                              QoS level to be used when     */
/*                                            publishing will message.    */
/*                                            Valid values are 0, 1, 2    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_will_message_set                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                        const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                        UINT will_message_length, UINT will_retain_flag, UINT will_QoS)
{

    if (client_ptr == NX_NULL)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Valid will_topic string.  The will topic string cannot be NULL. */
    if ((will_topic == NX_NULL) || (will_topic_length  == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if ((will_retain_flag != NX_TRUE) && (will_retain_flag != NX_FALSE))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if (will_QoS > 2)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_will_message_set(client_ptr, will_topic, will_topic_length, will_message,
                                             will_message_length, will_retain_flag, will_QoS));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_login_set                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the nxd_mqtt_client_login call.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_login_set                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                 CHAR *username, UINT username_length, CHAR *password, UINT password_length)
{
    if (client_ptr == NX_NULL)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Username and username length don't match,
       or password and password length don't match. */
    if (((username == NX_NULL) && (username_length != 0)) ||
        ((password == NX_NULL) && (password_length != 0)))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_login_set(client_ptr, username, username_length, password, password_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_retransmit_message                 PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retransmit QoS1 messages upon reconnection, if the    */
/*    connection is not set CLEAN_SESSION.                                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    _nxd_mqtt_packet_send                                               */
/*    nx_packet_release                                                   */
/*    tx_time_get                                                         */
/*    nx_packet_copy                                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_client_retransmit_message(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option)
{
NX_PACKET          *transmit_packet_ptr;
NX_PACKET          *packet_ptr;
UINT                status = NXD_MQTT_SUCCESS;
UINT                mutex_status;
UCHAR               fixed_header;

    transmit_packet_ptr = client_ptr -> message_transmit_queue_head;

    while (transmit_packet_ptr)
    {
        fixed_header = *(transmit_packet_ptr -> nx_packet_prepend_ptr);

        if ((fixed_header & 0xF0) == (MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4))
        {

            /* Retransmit publish packet only. */
            /* Obtain a NetX Packet. */
            status = nx_packet_copy(transmit_packet_ptr, &packet_ptr, client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);

            if (status != NXD_MQTT_SUCCESS)
            {
                return(NXD_MQTT_PACKET_POOL_FAILURE);
            }

            /* Release the mutex. */
            tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

            /* Send packet to server.  */
            status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, wait_option);

            if (status)
            {
                /* Release the packet. */
                nx_packet_release(packet_ptr);

                status = NXD_MQTT_COMMUNICATION_FAILURE;
            }
            /* Obtain the mutex. */
            mutex_status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, wait_option);

            if (mutex_status != TX_SUCCESS)
            {
                return(NXD_MQTT_MUTEX_FAILURE);
            }
            if (status)
            {
                return(status);
            }
        }
        transmit_packet_ptr = transmit_packet_ptr -> nx_packet_queue_next;
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_connect                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes an initial connection to the MQTT server.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    keepalive                             Keepalive flag                */
/*    clean_session                         Clean session flag            */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    nx_tcp_socket_create                  Create TCP socket             */
/*    nx_tcp_socket_receive_notify                                        */
/*    nx_tcp_client_socket_bind                                           */
/*    nxd_tcp_client_socket_connect                                       */
/*    nx_tcp_client_socket_unbind                                         */
/*    tx_mutex_put                                                        */
/*    nx_secure_tls_session_start                                         */
/*    nx_secure_tls_session_send                                          */
/*    nx_secure_tls_session_receive                                       */
/*    _nxd_mqtt_packet_allocate                                           */
/*    _nxd_mqtt_client_set_fixed_header                                   */
/*    _nxd_mqtt_client_append_message                                     */
/*    nx_tcp_socket_send                                                  */
/*    nx_packet_release                                                   */
/*    nx_tcp_socket_receive                                               */
/*    tx_event_flag_set                                                   */
/*    _nxd_mqtt_release_transmit_packet                                   */
/*    tx_timer_create                                                     */
/*    nx_secure_tls_session_receive                                       */
/*    _nxd_mqtt_client_connection_end                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed return value when it  */
/*                                            is set in CONNACK, corrected*/
/*                                            mqtt client state,          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the logic for     */
/*                                            non-blocking mode,          */
/*                                            resulting in version 6.1.8  */
/*  07-29-2022     Spencer McDonough        Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                              UINT keepalive, UINT clean_session, ULONG wait_option)
{
NX_PACKET           *packet_ptr;
UINT                 status;
TX_THREAD           *thread_ptr;
UINT                 new_priority;
UINT                 old_priority;


    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif /* NX_SECURE_ENABLE */

        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(NXD_MQTT_ALREADY_CONNECTED);
    }

    /* Check if client is connecting. */
    if (client_ptr -> nxd_mqtt_client_state == NXD_MQTT_CLIENT_STATE_CONNECTING)
    {
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(NXD_MQTT_CONNECTING);
    }

    /* Client state must be in IDLE.  */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_IDLE)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif /* NX_SECURE_ENABLE */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(NXD_MQTT_INVALID_STATE);
    }

#if defined(NX_SECURE_ENABLE) && defined(NXD_MQTT_REQUIRE_TLS)
    if (!client_ptr -> nxd_mqtt_client_use_tls)
    {

        /* NXD_MQTT_REQUIRE_TLS is defined but the application does not use TLS.
           This is security violation.  Return with failure code. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(NXD_MQTT_CONNECT_FAILURE);
    }
#endif /* NX_SECURE_ENABLE && NXD_MQTT_REQUIRE_TLS*/

    /* Record the keepalive value, converted to TX timer ticks. */
    client_ptr -> nxd_mqtt_keepalive = keepalive * NX_IP_PERIODIC_RATE;
    if (keepalive)
    {
        client_ptr -> nxd_mqtt_timer_value = NXD_MQTT_KEEPALIVE_TIMER_RATE;
        client_ptr -> nxd_mqtt_ping_timeout = NXD_MQTT_PING_TIMEOUT_DELAY;

        /* Create timer */
        tx_timer_create(&(client_ptr -> nxd_mqtt_timer), "MQTT Timer", _nxd_mqtt_periodic_timer_entry, (ULONG)client_ptr,
                        client_ptr -> nxd_mqtt_timer_value, client_ptr -> nxd_mqtt_timer_value, TX_AUTO_ACTIVATE);
    }
    else
    {
        client_ptr -> nxd_mqtt_timer_value = 0;
        client_ptr -> nxd_mqtt_ping_timeout = 0;
    }

    /* Record the clean session flag.  */
    client_ptr -> nxd_mqtt_clean_session = clean_session;

    /* Set TCP connection establish notify for non-blocking mode.  */
    if (wait_option == 0)
    {
        nx_tcp_socket_establish_notify(&client_ptr -> nxd_mqtt_client_socket, _nxd_mqtt_tcp_establish_notify);

        /* Set the receive callback. */
        nx_tcp_socket_receive_notify(&client_ptr -> nxd_mqtt_client_socket, _nxd_mqtt_receive_callback);

#ifdef NXD_MQTT_OVER_WEBSOCKET
        if (client_ptr -> nxd_mqtt_client_use_websocket)
        {

            /* Set the websocket connection callback.  */
            nx_websocket_client_connection_status_callback_set(&client_ptr -> nxd_mqtt_client_websocket, client_ptr, _nxd_mqtt_client_websocket_connection_status_callback);
        }
#endif /* NXD_MQTT_OVER_WEBSOCKET */
    }
    else
    {

        /* Clean receive callback.  */
        client_ptr -> nxd_mqtt_client_socket.nx_tcp_receive_callback = NX_NULL;

#ifdef NXD_MQTT_OVER_WEBSOCKET
        if (client_ptr -> nxd_mqtt_client_use_websocket)
        {

            /* Clean the websocket connection callback.  */
            nx_websocket_client_connection_status_callback_set(&client_ptr -> nxd_mqtt_client_websocket, NX_NULL, NX_NULL);
        }
#endif /* NXD_MQTT_OVER_WEBSOCKET */
    }

    /* Release mutex */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* First attempt to bind the client socket. */
    nx_tcp_client_socket_bind(&(client_ptr -> nxd_mqtt_client_socket), NX_ANY_PORT, wait_option);

    /* Obtain the mutex. */
    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    /* Make state as NXD_MQTT_CLIENT_STATE_CONNECTING. */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_CONNECTING;

    /* Release mutex. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* Connect to the MQTT server */
    status = nxd_tcp_client_socket_connect(&(client_ptr -> nxd_mqtt_client_socket), server_ip, server_port, wait_option);
    if ((status != NX_SUCCESS) && (status != NX_IN_PROGRESS))
    {

        /* Obtain the mutex. */
        tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

        /* Make state as NXD_MQTT_CLIENT_STATE_IDLE. */
        client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

        /* Release mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            nx_secure_tls_session_delete(&(client_ptr -> nxd_mqtt_tls_session));
        }
#endif /* NX_SECURE_ENABLE */
        nx_tcp_client_socket_unbind(&(client_ptr -> nxd_mqtt_client_socket));
        tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        return(NXD_MQTT_CONNECT_FAILURE);
    }

    /* Just return for non-blocking mode.  */
    if (wait_option == 0)
    {
        return(NX_IN_PROGRESS);
    }

    /* Increase priority to the same of internal thread to avoid out of order packet process. */
#ifndef NXD_MQTT_CLOUD_ENABLE
    thread_ptr = &(client_ptr -> nxd_mqtt_thread);
#else
    thread_ptr = &(client_ptr -> nxd_mqtt_client_cloud_ptr -> nx_cloud_thread);
#endif /* NXD_MQTT_CLOUD_ENABLE */
    tx_thread_info_get(thread_ptr, NX_NULL, NX_NULL, NX_NULL, 
                       &new_priority, NX_NULL, NX_NULL, NX_NULL, NX_NULL);
    tx_thread_priority_change(tx_thread_identify(), new_priority, &old_priority);

    /* If TLS is enabled, start TLS */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nxd_mqtt_client_use_tls)
    {

        status = nx_secure_tls_session_start(&(client_ptr -> nxd_mqtt_tls_session), &(client_ptr -> nxd_mqtt_client_socket), wait_option);

        if (status != NX_SUCCESS)
        {

            /* Revert thread priority. */
            tx_thread_priority_change(tx_thread_identify(), old_priority, &old_priority);

            /* End connection. */
            _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

            return(NXD_MQTT_CONNECT_FAILURE);
        }
    }
#endif /* NX_SECURE_ENABLE */

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
#ifdef NX_SECURE_ENABLE
        if (client_ptr -> nxd_mqtt_client_use_tls)
        {
            status = nx_websocket_client_secure_connect(&(client_ptr -> nxd_mqtt_client_websocket), &(client_ptr -> nxd_mqtt_tls_session),
                                                        client_ptr -> nxd_mqtt_client_websocket_host, client_ptr -> nxd_mqtt_client_websocket_host_length,
                                                        client_ptr -> nxd_mqtt_client_websocket_uri_path, client_ptr -> nxd_mqtt_client_websocket_uri_path_length,
                                                        (UCHAR *)NXD_MQTT_OVER_WEBSOCKET_PROTOCOL, sizeof(NXD_MQTT_OVER_WEBSOCKET_PROTOCOL) - 1,
                                                        wait_option);
        }
        else
#endif /* NX_SECURE_ENABLE */
        {
            status = nx_websocket_client_connect(&(client_ptr -> nxd_mqtt_client_websocket), &(client_ptr -> nxd_mqtt_client_socket),
                                                 client_ptr -> nxd_mqtt_client_websocket_host, client_ptr -> nxd_mqtt_client_websocket_host_length,
                                                 client_ptr -> nxd_mqtt_client_websocket_uri_path, client_ptr -> nxd_mqtt_client_websocket_uri_path_length,
                                                 (UCHAR *)NXD_MQTT_OVER_WEBSOCKET_PROTOCOL, sizeof(NXD_MQTT_OVER_WEBSOCKET_PROTOCOL) - 1,
                                                 wait_option);
        }

        if (status != NX_SUCCESS)
        {

            /* Revert thread priority. */
            tx_thread_priority_change(tx_thread_identify(), old_priority, &old_priority);

            /* End connection. */
            _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

            return(NXD_MQTT_CONNECT_FAILURE);
        }
    }

#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Start to send connect packet.  */
    status = _nxd_mqtt_client_connect_packet_send(client_ptr, wait_option);

    if (status != NX_SUCCESS)
    {

        /* Revert thread priority. */
        tx_thread_priority_change(tx_thread_identify(), old_priority, &old_priority);

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);
        return(NXD_MQTT_CONNECT_FAILURE);
    }

    /* Call a receive. */
    status = _nxd_mqtt_packet_receive(client_ptr, &packet_ptr, wait_option);

    /* Revert thread priority. */
    tx_thread_priority_change(tx_thread_identify(), old_priority, &old_priority);

    /* Check status.  */
    if (status)
    {

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);
        return(NXD_MQTT_COMMUNICATION_FAILURE);
    }

    /* Process CONNACK message.  */
    status = _nxd_mqtt_process_connack(client_ptr, packet_ptr, wait_option);

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {

        /* Set the receive callback. */
        nx_tcp_socket_receive_notify(&client_ptr -> nxd_mqtt_client_socket, _nxd_mqtt_receive_callback);
    }

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_connect_packet_send                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends CONNECT packet to MQTT server.                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    keepalive                             Keepalive flag                */
/*    clean_session                         Clean session flag            */
/*    wait_option                           Timeout value                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_release                                                   */
/*    _nxd_mqtt_packet_allocate                                           */
/*    _nxd_mqtt_release_transmit_packet                                   */
/*    _nxd_mqtt_client_connection_end                                     */
/*    _nxd_mqtt_client_set_fixed_header                                   */
/*    _nxd_mqtt_client_append_message                                     */
/*    _nxd_mqtt_packet_send                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_connect                                            */
/*    _nxd_mqtt_tcp_establish_process                                     */
/*    _nxd_mqtt_tls_establish_process                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Spencer McDonough        Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_connect_packet_send(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option)
{
NX_PACKET           *packet_ptr;
UINT                 status;
UINT                 length = 0;
UCHAR                connection_flags = 0;
UINT                 ret = NXD_MQTT_SUCCESS;
UCHAR                temp_data[4];
UINT                 keepalive = (client_ptr -> nxd_mqtt_keepalive/NX_IP_PERIODIC_RATE);


    /* Construct connect flags by taking the connect flag user supplies, or'ing the username and
       password bits, if they are supplied. */
    if (client_ptr -> nxd_mqtt_client_username)
    {
        connection_flags |= MQTT_CONNECT_FLAGS_USERNAME;

        /* Add the password flag only if username is supplied. */
        if (client_ptr -> nxd_mqtt_client_password)
        {
            connection_flags |= MQTT_CONNECT_FLAGS_PASSWORD;
        }
    }

    if (client_ptr -> nxd_mqtt_client_will_topic)
    {
        connection_flags = connection_flags | MQTT_CONNECT_FLAGS_WILL_FLAG;


        if (client_ptr -> nxd_mqtt_client_will_qos_retain & 0x80)
        {
            connection_flags = connection_flags | MQTT_CONNECT_FLAGS_WILL_RETAIN;
        }

        connection_flags = (UCHAR)(connection_flags | ((client_ptr -> nxd_mqtt_client_will_qos_retain & 0x3) << 3));
    }

    if (client_ptr -> nxd_mqtt_clean_session == NX_TRUE)
    {
        connection_flags = connection_flags | MQTT_CONNECT_FLAGS_CLEAN_SESSION;

        /* Clear any transmit blocks from the previous session. */
        while (client_ptr -> message_transmit_queue_head)
        {
            _nxd_mqtt_release_transmit_packet(client_ptr, client_ptr -> message_transmit_queue_head, NX_NULL);
        }
    }

    /* Set the length of the packet. */
    length = 10;

    /* Add the size of the client Identifier. */
    length += (client_ptr -> nxd_mqtt_client_id_length + 2);

    /* Add the will topic, if present. */
    if (connection_flags & MQTT_CONNECT_FLAGS_WILL_FLAG)
    {
        length += (client_ptr -> nxd_mqtt_client_will_topic_length + 2);
        length += (client_ptr -> nxd_mqtt_client_will_message_length + 2);
    }
    if (connection_flags & MQTT_CONNECT_FLAGS_USERNAME)
    {
        length += (UINT)(client_ptr -> nxd_mqtt_client_username_length + 2);
    }
    if (connection_flags & MQTT_CONNECT_FLAGS_PASSWORD)
    {
        length += (UINT)(client_ptr -> nxd_mqtt_client_password_length + 2);
    }

    /* Check for invalid length. */
    if (length > (127 * 127 * 127 * 127))
    {
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr, wait_option);

    if (status)
    {
        return(status);
    }

    /* Construct MQTT CONNECT message. */
    temp_data[0] = ((MQTT_CONTROL_PACKET_TYPE_CONNECT << 4) & 0xF0);

    /* Fill in fixed header. */
    ret = _nxd_mqtt_client_set_fixed_header(client_ptr, packet_ptr, temp_data[0], length, wait_option);

    if (ret)
    {
        /* Release the packet. */
        nx_packet_release(packet_ptr);
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }
    
    /* Fill in protocol name. */
    ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, "MQTT", 4, wait_option);

    if (ret)
    {
        /* Release the packet. */
        nx_packet_release(packet_ptr);
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    /* Fill in protocol level, */
    temp_data[0] = MQTT_PROTOCOL_LEVEL;

    /* Fill in byte 8: connect flags */
    temp_data[1] = connection_flags;

    /* Fill in byte 9 and 10: keep alive */
    temp_data[2] = (keepalive >> 8) & 0xFF;
    temp_data[3] = (keepalive & 0xFF);

    ret = nx_packet_data_append(packet_ptr, temp_data, 4, client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);

    if (ret)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    /* Fill in payload area, in the order of: client identifier, will topic, will message,
       user name, and password. */
    ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, client_ptr -> nxd_mqtt_client_id, 
                                          client_ptr -> nxd_mqtt_client_id_length, wait_option);

    /* Next fill will topic and will message if the will flag is set. */
    if (!ret && (connection_flags & MQTT_CONNECT_FLAGS_WILL_FLAG))
    {
        ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, (CHAR *)client_ptr -> nxd_mqtt_client_will_topic,
                                              client_ptr -> nxd_mqtt_client_will_topic_length, wait_option);

        if (!ret)
        {
            ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, (CHAR *)client_ptr -> nxd_mqtt_client_will_message,
                                                  client_ptr -> nxd_mqtt_client_will_message_length, wait_option);
        }
    }

    /* Fill username if username flag is set */
    if (!ret && (connection_flags & MQTT_CONNECT_FLAGS_USERNAME))
    {
        ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, client_ptr -> nxd_mqtt_client_username, 
                                              client_ptr -> nxd_mqtt_client_username_length, wait_option);
    }

    /* Fill password if password flag is set */
    if (!ret && (connection_flags & MQTT_CONNECT_FLAGS_PASSWORD))
    {
        ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, client_ptr -> nxd_mqtt_client_password, 
                                              client_ptr -> nxd_mqtt_client_password_length, wait_option);
    }

    if (ret)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    /* Ready to send the connect message to the server. */
    status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, wait_option);

    if (status)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_secure_connect                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes an initial secure (TLS) connection to           */
/*    the MQTT server.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    username                              User name, or NULL if user    */
/*                                            name is not required        */
/*    username_length                       Length of the user name, or   */
/*                                            0 if user name is NULL      */
/*    password                              Password string, or NULL if   */
/*                                            password is not required    */
/*    password_length                       Length of the password, or    */
/*                                            0 if password is NULL       */
/*    connection_flag                       Flag passed to the server     */
/*    timeout                               Timeout value                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE
UINT _nxd_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                     UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                       NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                     UINT keepalive, UINT clean_session, ULONG wait_option)
{
UINT ret;

    /* Set up TLS session information. */
    ret = (*tls_setup)(client_ptr, &client_ptr -> nxd_mqtt_tls_session,
                       &client_ptr -> nxd_mqtt_tls_certificate,
                       &client_ptr -> nxd_mqtt_tls_trusted_certificate);

    if (ret)
    {
        return(ret);
    }

    /* Mark the connection as secure. */
    client_ptr -> nxd_mqtt_client_use_tls = 1;

    ret = _nxd_mqtt_client_connect(client_ptr, server_ip, server_port, keepalive, clean_session, wait_option);

    return(ret);
}

#endif /* NX_SECURE_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_delete                             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created MQTT client instance.    */
/*    If the NXD_MQTT_SOCKET_TIMEOUT is set to NX_WAIT_FOREVER, this may  */
/*    suspend infinitely. This is because the MQTT Client must            */
/*    disconnect with the server, and if the network link is disabled or  */
/*    the server is not responding, this will blocks this function from   */
/*    completing.                                                         */ 
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                                                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr)
{


    /* Set the event flag for DELETE. Next time when the MQTT client thread
       wakes up, it will perform the deletion process. */
#ifndef NXD_MQTT_CLOUD_ENABLE
    tx_event_flags_set(&client_ptr -> nxd_mqtt_events, MQTT_DELETE_EVENT, TX_OR);
#else
    nx_cloud_module_event_set(&(client_ptr -> nxd_mqtt_client_cloud_module), MQTT_DELETE_EVENT);
#endif /* NXD_MQTT_CLOUD_ENABLE */

    /* Check if the MQTT client thread has completed. */
    while((client_ptr -> nxd_mqtt_client_socket).nx_tcp_socket_id != 0) 
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }

#ifndef NXD_MQTT_CLOUD_ENABLE
    /* Now we can delete the Client instance. */
    tx_thread_delete(&(client_ptr -> nxd_mqtt_thread));
#else
    /* Deregister mqtt module from cloud helper.  */
    nx_cloud_module_deregister(client_ptr -> nxd_mqtt_client_cloud_ptr, &(client_ptr -> nxd_mqtt_client_cloud_module));

    /* Delete own created cloud.  */
    if (client_ptr -> nxd_mqtt_client_cloud.nx_cloud_id == NX_CLOUD_ID)
        nx_cloud_delete(&(client_ptr -> nxd_mqtt_client_cloud));
#endif /* NXD_MQTT_CLOUD_ENABLE */

#ifdef NXD_MQTT_OVER_WEBSOCKET
    if (client_ptr -> nxd_mqtt_client_use_websocket)
    {
        nx_websocket_client_delete(&client_ptr -> nxd_mqtt_client_websocket);
        client_ptr -> nxd_mqtt_client_use_websocket = NX_FALSE;
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_publish_packet_send                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a publish packet to the connected broker.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    packet_ptr                            Pointer to publish packet     */
/*    packet_id                             Current packet ID             */
/*    QoS                                   Quality of service            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*    nx_packet_release                                                   */
/*    _nxd_mqtt_copy_transmit_packet                                      */
/*    _nxd_mqtt_packet_send                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_publish                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            supported maximum transmit  */
/*                                            queue depth,                */
/*                                            resulting in version 6.1.8  */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_publish_packet_send(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr,
                                          USHORT packet_id, UINT QoS, ULONG wait_option)
{

UINT       status;
UINT       ret = NXD_MQTT_SUCCESS;

    if (QoS != 0)
    {
    /* This packet needs to be stored locally for possible retransmission. */
    NX_PACKET *transmit_packet_ptr;

        /* Copy packet for retransmission. */
        if (_nxd_mqtt_copy_transmit_packet(client_ptr, packet_ptr, &transmit_packet_ptr,
                                           packet_id, NX_TRUE, wait_option))
        {
            return(NXD_MQTT_PACKET_POOL_FAILURE);
        }

        /* Obtain the mutex. */
        status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

        if (status != TX_SUCCESS)
        {
            nx_packet_release(transmit_packet_ptr);

#ifdef NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH
            /* Decrease the transmit queue depth.  */
            client_ptr -> message_transmit_queue_depth--;
#endif /* NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH */
            return(NXD_MQTT_MUTEX_FAILURE);
        }

        if (client_ptr -> message_transmit_queue_head == NX_NULL)
        {
            client_ptr -> message_transmit_queue_head = transmit_packet_ptr;
        }
        else
        {
            client_ptr -> message_transmit_queue_tail -> nx_packet_queue_next = transmit_packet_ptr;
        }
        client_ptr -> message_transmit_queue_tail = transmit_packet_ptr;
    }
    else
    {

        /* Obtain the mutex. */
        status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

        if (status != TX_SUCCESS)
        {
            return(NXD_MQTT_MUTEX_FAILURE);
        }
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;


    /* Release the mutex. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* Ready to send the connect message to the server. */
    status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, wait_option);

    if (status)
    {
        ret = NXD_MQTT_COMMUNICATION_FAILURE;
    }

    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_publish                            PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function publishes a message to the connected broker.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Name of the topic             */
/*    topic_name_length                     Length of the topic name      */
/*    message                               Message string                */
/*    message_length                        Length of the message,        */
/*                                            in bytes                    */
/*    retain                                The retain flag, whether      */
/*                                            or not the broker should    */
/*                                            store this message          */
/*    QoS                                   Expected QoS level            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    _nxd_mqtt_client_set_fixed_header                                   */
/*    _nxd_mqtt_client_append_message                                     */
/*    tx_mutex_put                                                        */
/*    nx_packet_release                                                   */
/*    _nxd_mqtt_client_publish_packet_send                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Spencer McDonough        Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                              CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG wait_option)
{

NX_PACKET *packet_ptr;
UINT       status;
UINT       length = 0;
UCHAR      flags;
USHORT     packet_id = 0;
UINT       ret = NXD_MQTT_SUCCESS;

    if (QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }


    /* Do nothing if the client is already connected. */
    if (client_ptr -> nxd_mqtt_client_state != NXD_MQTT_CLIENT_STATE_CONNECTED)
    {
        return(NXD_MQTT_NOT_CONNECTED);
    }

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr, wait_option);

    if (status != NXD_MQTT_SUCCESS)
    {
        return(NXD_MQTT_PACKET_POOL_FAILURE);
    }

    flags = (UCHAR)((MQTT_CONTROL_PACKET_TYPE_PUBLISH << 4) | (QoS << 1));

    if (retain)
    {
        flags = flags | MQTT_PUBLISH_RETAIN;
    }


    /* Compute the remaining length. */

    /* Topic Name. */
    /* Compute Topic Name length. */
    length = topic_name_length + 2;

    /* Count packet ID for QoS 1 or QoS 2 message. */
    if ((QoS == 1) || (QoS == 2))
    {
        length += 2;
    }

    /* Count message. */
    if ((message != NX_NULL) && (message_length != 0))
    {
        length += message_length;
    }

    /* Write out the control header and remaining length field. */
    ret = _nxd_mqtt_client_set_fixed_header(client_ptr, packet_ptr, flags, length, wait_option);

    if (ret)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_INTERNAL_ERROR);
    }


    /* Write out topic */
    ret = _nxd_mqtt_client_append_message(client_ptr, packet_ptr, topic_name, topic_name_length, wait_option);

    if (ret)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    /* Append Packet Identifier for QoS level 1 or 2  MQTT 3.3.2.2 */
    if ((QoS == 1) || (QoS == 2))
    {
    UCHAR identifier[2];

        /* Obtain the mutex. */
        status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

        if (status != TX_SUCCESS)
        {
            return(NXD_MQTT_MUTEX_FAILURE);
        }

        packet_id = (USHORT)client_ptr -> nxd_mqtt_client_packet_identifier;
        identifier[0] = (UCHAR)(client_ptr -> nxd_mqtt_client_packet_identifier >> 8);
        identifier[1] = (client_ptr -> nxd_mqtt_client_packet_identifier & 0xFF);

        /* Update packet id. */
        client_ptr -> nxd_mqtt_client_packet_identifier = (client_ptr -> nxd_mqtt_client_packet_identifier + 1) & 0xFFFF;

        /* Prevent packet identifier from being zero. MQTT-2.3.1-1 */
        if(client_ptr -> nxd_mqtt_client_packet_identifier == 0)
            client_ptr -> nxd_mqtt_client_packet_identifier = 1;

        /* Release the mutex. */
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

        ret = nx_packet_data_append(packet_ptr, identifier, 2, 
                                    client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);

        if (ret)
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            return(NXD_MQTT_INTERNAL_ERROR);
        }
    }

    /* Append message. */
    if ((message != NX_NULL) && (message_length) != 0)
    {
        
        /* Use nx_packet_data_append to move user-supplied message data into the packet.
           nx_packet_data_append uses chained packet if the additional storage space is 
           needed. */
        ret = nx_packet_data_append(packet_ptr, message, message_length, 
                                       client_ptr -> nxd_mqtt_client_packet_pool_ptr, wait_option);
        if(ret)
        {
            /* Unable to obtain a new packet to store the message. */

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            return(NXD_MQTT_INTERNAL_ERROR);
        }
    }

    /* Send publish packet. */
    ret = _nxd_mqtt_client_publish_packet_send(client_ptr, packet_ptr, packet_id, QoS, wait_option);

    if (ret)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);
    }
    return(ret);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                          PORTABLE C      */
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a subscribe message to the broker.              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub            The actual routine that       */
/*                                            performs the sub/unsub      */
/*                                            action.                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added packet id parameter,  */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS)
{

    if (QoS == 2)
    {
        return(NXD_MQTT_QOS2_NOT_SUPPORTED);
    }

    return(_nxd_mqtt_client_sub_unsub(client_ptr, (MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE << 4) | 0x02,
                                      topic_name, topic_name_length, NX_NULL, QoS));
}




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_unsubscribe                        PORTABLE C      */
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unsubscribes a topic from the broker.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_sub_unsub                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added packet id parameter,  */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length)
{
    return(_nxd_mqtt_client_sub_unsub(client_ptr, (MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE << 4) | 0x02,
                                      topic_name, topic_name_length, NX_NULL, 0));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_send_simple_message                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function handles the transmission of PINGREQ or       */
/*    DISCONNECT message.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    header_value                          Value to be programmed into   */
/*                                            MQTT header.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    _nxd_mqtt_packet_allocate                                           */
/*    tx_mutex_put                                                        */
/*    _nxd_mqtt_packet_send                                               */
/*    nx_packet_release                                                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_ping                                               */
/*    _nxd_mqtt_client_disconnect                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Spencer McDonough        Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Bo Chen                  Modified comment(s), improved */
/*                                            the logic of sending packet,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_mqtt_send_simple_message(NXD_MQTT_CLIENT *client_ptr, UCHAR header_value)
{

NX_PACKET *packet_ptr;
UINT       status;
UINT       status_mutex;
UCHAR     *byte;

    status = _nxd_mqtt_packet_allocate(client_ptr, &packet_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(NXD_MQTT_INTERNAL_ERROR);
    }

    if (2u > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
    {
        nx_packet_release(packet_ptr);

        /* Packet buffer is too small to hold the message. */
        return(NX_SIZE_ERROR);
    }

    byte = packet_ptr -> nx_packet_prepend_ptr;

    *byte = (UCHAR)(header_value << 4);
    *(byte + 1) = 0;

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
    packet_ptr -> nx_packet_length = 2;


    /* Release MQTT protection before making NetX/TLS calls. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    /* Send packet to server.  */
    status = _nxd_mqtt_packet_send(client_ptr, packet_ptr, NX_WAIT_FOREVER);

    status_mutex = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);
    if (status)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        status = NXD_MQTT_COMMUNICATION_FAILURE;
    }
    if (status_mutex)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    if (header_value == MQTT_CONTROL_PACKET_TYPE_PINGREQ)
    {
        /* Do not update the ping sent time if the outstanding ping has not been responded yet */
        if (client_ptr -> nxd_mqtt_ping_not_responded != 1)
        {
            /* Record the time we send out the PINGREG */
            client_ptr -> nxd_mqtt_ping_sent_time = tx_time_get();
            client_ptr -> nxd_mqtt_ping_not_responded = 1;
        }
    }

    /* Update the timeout value. */
    client_ptr -> nxd_mqtt_timeout = tx_time_get() + client_ptr -> nxd_mqtt_keepalive;

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disconnects the MQTT client from a server.            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_send_simple_message                                       */
/*    _nxd_mqtt_process_disconnect                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
UINT status;

    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        /* Disable timer if timer has been started. */
        if (client_ptr -> nxd_mqtt_keepalive)
        {
            tx_timer_delete(&(client_ptr -> nxd_mqtt_timer));
        }

        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Let the server know we are ending the MQTT session. */
    _nxd_mqtt_send_simple_message(client_ptr, MQTT_CONTROL_PACKET_TYPE_DISCONNECT);

    /* Call the disconnect routine to disconnect the socket,
       release transmit packets, release received packets,
       and delete the client timer. */
    _nxd_mqtt_process_disconnect(client_ptr);

    /* Release the mutex. */
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_receive_notify_set                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function installs the MQTT client publish notify callback      */
/*    function.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    mqtt_client_receive_notify            User-supplied callback        */
/*                                            function, which is invoked  */
/*                                            upon receiving a publish    */
/*                                            message.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                         VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count))
{

    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    client_ptr -> nxd_mqtt_client_receive_notify = receive_notify;

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);

    return(NXD_MQTT_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_message_get                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves a published MQTT message.                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_buffer                          Pointer to the topic buffer   */
/*                                            where topic is copied to    */
/*    topic_buffer_size                     Size of the topic buffer.     */
/*    actual_topic_length                   Number of bytes copied into   */
/*                                            topic_buffer                */
/*    message_buffer                        Pointer to the buffer where   */
/*                                            message is copied to        */
/*    message_buffer_size                   Size of the message_buffer    */
/*    actual_message_length                 Number of bytes copied into   */
/*                                            the message buffer.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect           Actual MQTT Client disconnect */
/*                                            call                        */
/*    _nxd_mqtt_read_remaining_length       Skip the remaining length     */
/*                                            field                       */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed uninitialized value,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                  UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length)
{

UINT                status;
NX_PACKET          *packet_ptr;
ULONG               topic_offset;
USHORT              topic_length;
ULONG               message_offset;
ULONG               message_length;

    tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);
    while (client_ptr -> message_receive_queue_depth)
    {
        packet_ptr = client_ptr -> message_receive_queue_head;
        status = _nxd_mqtt_process_publish_packet(packet_ptr, &topic_offset, &topic_length, &message_offset, &message_length);
        if (status == NXD_MQTT_SUCCESS)
        {
            if ((topic_buffer_size < topic_length) ||
                (message_buffer_size < message_length))
            {
                tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
                return(NXD_MQTT_INSUFFICIENT_BUFFER_SPACE);
            }
        }

        client_ptr -> message_receive_queue_head = packet_ptr -> nx_packet_queue_next;
        if (client_ptr -> message_receive_queue_tail == packet_ptr)
        {
            client_ptr -> message_receive_queue_tail = NX_NULL;
        }
        client_ptr -> message_receive_queue_depth--;

        if (status == NXD_MQTT_SUCCESS)
        {

            /* Set topic and message lengths to avoid uninitialized value. */
            *actual_topic_length = 0;
            *actual_message_length = 0;
            nx_packet_data_extract_offset(packet_ptr, topic_offset, topic_buffer,
                                          topic_length, (ULONG *)actual_topic_length);
            nx_packet_data_extract_offset(packet_ptr, message_offset, message_buffer,
                                          message_length, (ULONG *)actual_message_length);
            nx_packet_release(packet_ptr);

            tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
            return(NXD_MQTT_SUCCESS);
        }
        nx_packet_release(packet_ptr);
    }
    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
    return(NXD_MQTT_NO_MESSAGE);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_create                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in nxd_mqt_client_create call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_name                           Name string used in by the    */
/*                                            client                      */
/*    client_id                             Client ID used by the client  */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    stack_ptr                             Client thread's stack pointer */
/*    stack_size                            Client thread's stack size    */
/*    mqtt_thread_priority                  Priority for MQTT thread      */
/*    memory_ptr                            Deprecated and not used       */
/*    memory_size                           Deprecated and not used       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_create               Actual client create call     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name, CHAR *client_id, UINT client_id_length,
                              NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                              VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                              VOID *memory_ptr, ULONG memory_size)
{


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (stack_ptr == NX_NULL) || (stack_size == 0) || (pool_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }
    
    return(_nxd_mqtt_client_create(client_ptr, client_name, client_id, client_id_length, ip_ptr,
                                   pool_ptr, stack_ptr, stack_size, mqtt_thread_priority,
                                   memory_ptr, memory_size));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_connect                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client stop call.           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    keepalive                             The MQTT keepalive timer      */
/*    clean_session                         Clean session flag            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                               UINT keepalive, UINT clean_session, ULONG wait_option)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_ip == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Test for IP version flag. */
    if ((server_ip -> nxd_ip_version != 4) && (server_ip -> nxd_ip_version != 6))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    if (server_port == 0)
    {
        return(NX_INVALID_PORT);
    }

    status = _nxd_mqtt_client_connect(client_ptr, server_ip, server_port, keepalive, clean_session, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_secure_connect                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client TLS secure connect.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    server_ip                             Server IP address structure   */
/*    server_port                           Server port number, in host   */
/*                                            byte order                  */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    keepalive                             The MQTT keepalive timer      */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect              Actual MQTT Client connect    */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE

UINT _nxde_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                      UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                        NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                      UINT keepalive, UINT clean_session, ULONG wait_option)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_ip == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (server_port == 0)
    {
        return(NX_INVALID_PORT);
    }

    status = _nxd_mqtt_client_secure_connect(client_ptr, server_ip, server_port, tls_setup,
                                             keepalive, clean_session, wait_option);

    return(status);
}
#endif /* NX_SECURE_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_delete                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client delete call.         */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_delete               Actual MQTT Client delete     */
/*                                            call.                       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    status = _nxd_mqtt_client_delete(client_ptr);

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_publish                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the publish service.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Name of the topic             */
/*    topic_name_length                     Length of the topic name      */
/*    message                               Message string                */
/*    message_length                        Length of the message,        */
/*                                            in bytes                    */
/*    retain                                The retain flag, whether      */
/*                                            or not the broker should    */
/*                                            store this message          */
/*    QoS                                   Expected QoS level            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                               CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG wait_option)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate message length. */
    if (message && (message_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate QoS value. */
    if (QoS > 3)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_publish(client_ptr, topic_name, topic_name_length, message, message_length, retain, QoS, wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_subscribe                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the subscribe service.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to subscribe to             */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*    QoS                                   Expected QoS level            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_subscribe                                          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS)
{


    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    /* Validate QoS value. */
    if (QoS > 2)
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_subscribe(client_ptr, topic_name, topic_name_length, QoS));
}




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_unsubscribe                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking to the unsubscribe service.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_name                            Pointer to the topic string   */
/*                                            to unsubscribe              */
/*    topic_name_length                     Length of the topic string    */
/*                                            in bytes                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Validate topic_name */
    if ((topic_name == NX_NULL) || (topic_name_length == 0))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }

    return(_nxd_mqtt_client_unsubscribe(client_ptr, topic_name, topic_name_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_disconnect                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client disconnect call.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect           Actual MQTT Client disconnect */
/*                                            call                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr)
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    return(_nxd_mqtt_client_disconnect(client_ptr));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_message_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client message get call.    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    topic_buffer                          Pointer to the topic buffer   */
/*                                            where topic is copied to    */
/*    topic_buffer_size                     Size of the topic buffer.     */
/*    actual_topic_length                   Number of bytes copied into   */
/*                                            topic_buffer                */
/*    message_buffer                        Pointer to the buffer where   */
/*                                            message is copied to        */
/*    message_buffer_size                   Size of the message_buffer    */
/*    actual_message_length                 Number of bytes copied into   */
/*                                            the message buffer.         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_message_get                                        */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                   UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length)
{

    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Topic and topic_length can be NULL if caller does not care the topic string. */

    /* Validate message.  Message_length can be NULL if caller does not care message length. */
    if ((message_buffer == NX_NULL) || (topic_buffer == NX_NULL))
    {
        return(NXD_MQTT_INVALID_PARAMETER);
    }


    return(_nxd_mqtt_client_message_get(client_ptr, topic_buffer, topic_buffer_size, actual_topic_length,
                                        message_buffer, message_buffer_size, actual_message_length));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_receive_notify_set                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in MQTT client publish notify call. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    receive_notify                        User-supplied callback        */
/*                                            function, which is invoked  */
/*                                            upon receiving a publish    */
/*                                            message.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_receive_notify_set                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                          VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count))
{
    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (receive_notify == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    return(_nxd_mqtt_client_receive_notify_set(client_ptr, receive_notify));
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect_notify_set              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the notify function for the disconnect event.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    disconnect_notify                     The notify function to be     */
/*                                            used when the client is     */
/*                                            disconnected from the       */
/*                                            server.                     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *))
{

    client_ptr -> nxd_mqtt_disconnect_notify = disconnect_notify;

    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_disconnect_notify_set             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in setting MQTT client disconnect   */
/*    callback function.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    disconnect_callback                   The callback function to be   */
/*                                            used when an on-going       */
/*                                            connection is disconnected. */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_disconnect_notify_set                              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *))
{

    /* Validate client_ptr */
    if (client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }
    return(_nxd_mqtt_client_disconnect_notify_set(client_ptr, disconnect_notify));
}


#ifdef NXD_MQTT_CLOUD_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_cloud_create                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates mqtt client running on cloud helper thread.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    client_name                           Name string used in by the    */
/*                                            client                      */
/*    client_id                             Client ID used by the client  */
/*    client_id_length                      Length of Client ID, in bytes */
/*    ip_ptr                                Pointer to IP instance        */
/*    pool_ptr                              Pointer to packet pool        */
/*    cloud_ptr                             Pointer to Cloud instance     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_create               Actual client create call     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected mqtt client state,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_cloud_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name, CHAR *client_id, UINT client_id_length,
                                   NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_CLOUD *cloud_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (pool_ptr == NX_NULL) || (cloud_ptr == NX_NULL) || (cloud_ptr -> nx_cloud_id != NX_CLOUD_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Create MQTT client.  */
    status = _nxd_mqtt_client_create_internal(client_ptr, client_name, client_id, client_id_length, ip_ptr,
                                              pool_ptr, NX_NULL, 0, 0);

    /* Check status.  */
    if (status)
    {
        return(status);
    }

    /* Save the cloud pointer.  */
    client_ptr -> nxd_mqtt_client_cloud_ptr = cloud_ptr;

    /* Save the mutex pointer.  */
    client_ptr -> nxd_mqtt_client_mutex_ptr = &(cloud_ptr -> nx_cloud_mutex);

    /* Register MQTT on cloud helper.  */
    status = nx_cloud_module_register(client_ptr -> nxd_mqtt_client_cloud_ptr, &(client_ptr -> nxd_mqtt_client_cloud_module), client_name, NX_CLOUD_MODULE_MQTT_EVENT,
                                      _nxd_mqtt_client_event_process, client_ptr);

    /* Determine if an error occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Delete internal resource created in _nxd_mqtt_client_create_internal().  */

        /* Delete socket.  */
        nx_tcp_socket_delete(&(client_ptr -> nxd_mqtt_client_socket));

        return(NXD_MQTT_INTERNAL_ERROR);
    }

    /* Update state.  */
    client_ptr -> nxd_mqtt_client_state = NXD_MQTT_CLIENT_STATE_IDLE;

    return(NXD_MQTT_SUCCESS);
}
#endif /* NXD_MQTT_CLOUD_ENABLE */

#ifdef NXD_MQTT_OVER_WEBSOCKET
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_websocket_connection_status_callback               */
/*                                                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the websocket connection status callback.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    websocket_client_ptr                  Pointer to websocket client   */
/*    context                               Pointer to MQTT client        */
/*    status                                Websocket connection status   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_connect_packet_send                                */
/*    _nxd_mqtt_client_connection_end                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_mqtt_client_event_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yuxin Zhou               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
VOID _nxd_mqtt_client_websocket_connection_status_callback(NX_WEBSOCKET_CLIENT *websocket_client_ptr, VOID *context, UINT status)
{
NXD_MQTT_CLIENT *client_ptr = (NXD_MQTT_CLIENT *)context;


    NX_PARAMETER_NOT_USED(websocket_client_ptr);

    if (status == NX_SUCCESS)
    {

        /* Start to send MQTT connect packet.  */
        status = _nxd_mqtt_client_connect_packet_send(client_ptr, NX_NO_WAIT);
    }

    /* If an error occurs.  */
    if (status)
    {

        /* End connection. */
        _nxd_mqtt_client_connection_end(client_ptr, NX_NO_WAIT);

        /* Check callback function.  */
        if (client_ptr -> nxd_mqtt_connect_notify)
        {
            client_ptr -> nxd_mqtt_connect_notify(client_ptr, status, client_ptr -> nxd_mqtt_connect_context);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_mqtt_client_websocket_set                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the websocket.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    host                                  Host used by the client       */
/*    host_length                           Length of host, in bytes      */
/*    uri_path                              URI path used by the client   */
/*    uri_path_length                       Length of uri path, in bytes  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yuxin Zhou               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nxd_mqtt_client_websocket_set(NXD_MQTT_CLIENT *client_ptr, UCHAR *host, UINT host_length, UCHAR *uri_path, UINT uri_path_length)
{
UINT status;

    /* Obtain the mutex. */
    status = tx_mutex_get(client_ptr -> nxd_mqtt_client_mutex_ptr, NX_WAIT_FOREVER);

    if (status != TX_SUCCESS)
    {
        return(NXD_MQTT_MUTEX_FAILURE);
    }

    /* Set the host info.  */
    client_ptr -> nxd_mqtt_client_websocket_host = host;
    client_ptr -> nxd_mqtt_client_websocket_host_length = host_length;
    client_ptr -> nxd_mqtt_client_websocket_uri_path = uri_path;
    client_ptr -> nxd_mqtt_client_websocket_uri_path_length = uri_path_length;

    /* Create WebSocket.  */
    status = nx_websocket_client_create(&client_ptr -> nxd_mqtt_client_websocket, (UCHAR *)"",
                                        client_ptr -> nxd_mqtt_client_ip_ptr,
                                        client_ptr -> nxd_mqtt_client_packet_pool_ptr);

    /* Check status.  */
    if (status)
    {
        tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
        return(status);
    }

    client_ptr -> nxd_mqtt_client_use_websocket = NX_TRUE;

    tx_mutex_put(client_ptr -> nxd_mqtt_client_mutex_ptr);
    return(NXD_MQTT_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_mqtt_client_websocket_set                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in setting MQTT client websocket.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to MQTT Client        */
/*    host                                  Host used by the client       */
/*    host_length                           Length of host, in bytes      */
/*    uri_path                              URI path used by the client   */
/*    uri_path_length                       Length of uri path, in bytes  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_mqtt_client_websocket_set                                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yuxin Zhou               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nxde_mqtt_client_websocket_set(NXD_MQTT_CLIENT *client_ptr, UCHAR *host, UINT host_length, UCHAR *uri_path, UINT uri_path_length)
{

    /* Validate the parameters.  */
    if ((client_ptr == NX_NULL) || (host == NX_NULL) || (host_length == 0) ||
        (uri_path == NX_NULL) || (uri_path_length == 0))
    {
        return(NX_PTR_ERROR);
    }

    return(_nxd_mqtt_client_websocket_set(client_ptr, host, host_length, uri_path, uri_path_length));
}
#endif /* NXD_MQTT_OVER_WEBSOCKET */
