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

#include <asc_config.h>

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv4.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#include "tx_api.h"

#ifndef NX_DISABLE_IPV6
#include "nx_ipv6.h"
#endif /* NX_DISABLE_IPV6 */

#include "asc_security_core/logger.h"
#include "asc_security_core/collector.h"
#include "asc_security_core/components_factory_declarations.h"
#include "asc_security_core/components_manager.h"
#include "asc_security_core/serializer.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/serializer.h"

#include "iot_security_module/model/objects/object_network_activity_ext.h"


typedef enum byte_order_t {
    BYTE_ORDER_NETWORK,
    BYTE_ORDER_HOST
} byte_order_t;

typedef void (*nx_ip_transport_packet_receive_cb_t)(struct NX_IP_STRUCT *, struct NX_PACKET_STRUCT *);

/**
 * @brief   Switch between the hashtables.
 */
static void _switch_hashtables();


/**
 * @brief   A hashset_network_activity_ipv4_t_for_each function. Appends the payload object
 *          to the list pointed by context.
 *
 * @param   data_ptr The payload struct
 * @param   context     The event handle
 */
static void _append_ipv4_payload_to_list(network_activity_ipv4_t *data_ptr, void *context);


/**
 * @brief   Parse an IPv4 packet to an event payload struct.
 *
 * @param   ip_packet               Pointer to the IPv4 packet data.
 * @param   direction               The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 * @param   ip_header_byte_order    The byte order of the IP header
 *
 * @return  network_activity_t*
 */
static network_activity_ipv4_t *_ipv4_callback(VOID *ip_packet, UINT direction, byte_order_t ip_header_byte_order);


#ifndef NX_DISABLE_IPV6
/**
 * @brief   A hashset_network_activity_ipv6_t_for_each function. Appends the payload object
 *          to the list pointed by context.
 *
 * @param   data_ptr The payload struct
 * @param   context     The event handle
 */
static void _append_ipv6_payload_to_list(network_activity_ipv6_t *data_ptr, void *context);


/**
 * @brief   Parse an IPv6 packet to an event payload struct.
 *
 * @param   ip_packet               Pointer to the IPv6 packet data.
 * @param   direction               The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 * @param   ip_header_byte_order    The byte order of the IP header
 *
 * @return  network_activity_t*
 */
static network_activity_ipv6_t *_ipv6_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr_st, UINT direction, byte_order_t ip_header_byte_order);
#endif /* NX_DISABLE_IPV6 */

/**
 * @brief   Capture an IP packet.
 *
 * @param   ip_packet               Pointer to the IP packet data.
 * @param   direction               The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 * @param   ip_header_byte_order    The byte order of the IP header
 */
static VOID _collector_network_activity_ip_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr, UINT direction, byte_order_t ip_header_byte_order);

/**
 * @brief   Initialize the port layer of the network activity collector.
 *
 * @return  ASC_RESULT_OK on success, ASC_RESULT_EXCEPTION otherwise
 **/
static asc_result_t _collector_network_activity_port_init();

/**
 * @brief   The ip filter function.
 *
 * @param   ip_packet   A pointer to the IP packet, in network byte order.
 * @param   direction   The direction of the packet (NX_IP_PACKET_IN / NX_IP_PACKET_OUT)
 *
 * @return  NX_SUCCESS
 **/
static UINT _collector_network_activity_port_ip_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr, UINT direction);

#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
/**
 * @brief   The tcp_packet_receive hook function.
 *
 * @param   ip_ptr      A pointer to the IP struct.
 * @param   packet_ptr  A pointer to the packet struct. At this stage, the IP header is in host byte order.
 **/
static VOID _collector_network_activity_port_tcp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr);

/**
 * @brief   The udp_packet_receive hook function.
 *
 * @param   ip_ptr      A pointer to the IP struct.
 * @param   packet_ptr  A pointer to the packet struct. At this stage, the IP header is in host byte order.
 **/
static VOID _collector_network_activity_port_udp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr);

/**
 * @brief   The icmp_packet_receive hook function.
 *
 * @param   ip_ptr      A pointer to the IP struct.
 * @param   packet_ptr  A pointer to the packet struct. At this stage, the IP header is in host byte order.
 **/
static VOID _collector_network_activity_port_icmp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr);
#endif /* ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY */

/**
 * @brief   De-initialize the port layer of the network activity collector.
 **/
static VOID _collector_network_activity_port_deinit();

/**
 * @brief Serialize events from the collector
 *
 * @param collector_internal_ptr    A handle to the collector internal.
 * @param serializer                The serializer the collector should use.
 *
 * @return  ASC_RESULT_OK on success
 *          ASC_RESULT_EMPTY when there are no events to serialize. In that case, serializer remains unchanged.
 *          ASC_RESULT_EXCEPTION otherwise
 */
static asc_result_t _collector_network_activity_serialize_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer);

static network_activity_ipv4_t *_ipv4_hashtables[2][IPV4_HASHSET_SIZE] = { { NULL } };
static network_activity_ipv4_t **_current_ipv4_hashtable = _ipv4_hashtables[0];
static int _current_ipv4_hashtable_index = 0;

#ifndef NX_DISABLE_IPV6
static network_activity_ipv6_t *_ipv6_hashtables[2][IPV6_HASHSET_SIZE] = { { NULL } };
static network_activity_ipv6_t **_current_ipv6_hashtable = _ipv6_hashtables[0];
static int _current_ipv6_hashtable_index = 0;
#endif /* NX_DISABLE_IPV6 */

#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
static nx_ip_transport_packet_receive_cb_t _tcp_packet_receive_original = NULL;
static nx_ip_transport_packet_receive_cb_t _udp_packet_receive_original = NULL;
static nx_ip_transport_packet_receive_cb_t _icmp_packet_receive_original = NULL;
#endif /* ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY */

static asc_result_t _cm_init(component_id_t id);
static asc_result_t _cm_start(component_id_t id);
static asc_result_t _cm_stop(component_id_t id);

COLLECTOR_OPS_DEFINITIONS(, _cm_init, collector_default_deinit, collector_default_subscribe, collector_default_unsubscribe, _cm_start, _cm_stop);

COMPONENTS_FACTORY_DEFINITION(NetworkActivity, &_ops)

static asc_result_t _cm_init(component_id_t id)
{
    return collector_default_create(id, NetworkActivity, COLLECTOR_PRIORITY_HIGH,
        _collector_network_activity_serialize_events, ASC_HIGH_PRIORITY_INTERVAL, NULL);
}

static asc_result_t _cm_start(component_id_t id)
{
    hashset_network_activity_ipv4_t_init(_ipv4_hashtables[0]);
    hashset_network_activity_ipv4_t_init(_ipv4_hashtables[1]);
    _current_ipv4_hashtable_index = 0;
    _current_ipv4_hashtable = _ipv4_hashtables[_current_ipv4_hashtable_index];

#ifndef NX_DISABLE_IPV6
    hashset_network_activity_ipv6_t_init(_ipv6_hashtables[0]);
    hashset_network_activity_ipv6_t_init(_ipv6_hashtables[1]);
    _current_ipv6_hashtable_index = 0;
    _current_ipv6_hashtable = _ipv6_hashtables[_current_ipv6_hashtable_index];
#endif /* NX_DISABLE_IPV6 */
    return _collector_network_activity_port_init();
}

static asc_result_t _cm_stop(component_id_t id)
{
    _collector_network_activity_port_deinit();

    _current_ipv4_hashtable = NULL;
    hashset_network_activity_ipv4_t_clear(_ipv4_hashtables[0], network_activity_ipv4_deinit, NULL);
    hashset_network_activity_ipv4_t_clear(_ipv4_hashtables[1], network_activity_ipv4_deinit, NULL);

#ifndef NX_DISABLE_IPV6
    _current_ipv6_hashtable = NULL;
    hashset_network_activity_ipv6_t_clear(_ipv6_hashtables[0], network_activity_ipv6_deinit, NULL);
    hashset_network_activity_ipv6_t_clear(_ipv6_hashtables[1], network_activity_ipv6_deinit, NULL);
#endif /* NX_DISABLE_IPV6 */

    return ASC_RESULT_OK;
}

static asc_result_t _collector_network_activity_serialize_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer)
{
    asc_result_t result = ASC_RESULT_OK;

    network_activity_ipv4_t *ipv4_list = NULL;
    network_activity_ipv4_t **previous_ipv4_hashtable = NULL;

    network_activity_ipv6_t *ipv6_list = NULL;
#ifndef NX_DISABLE_IPV6
    network_activity_ipv6_t **previous_ipv6_hashtable = NULL;
#endif /* NX_DISABLE_IPV6 */

    unsigned long current_time;

    if (serializer == NULL)
    {
        log_error("bad argument, serializer=[NULL]");
        result = ASC_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    current_time = itime_time(NULL);

    previous_ipv4_hashtable = _current_ipv4_hashtable;
#ifndef NX_DISABLE_IPV6
    previous_ipv6_hashtable = _current_ipv6_hashtable;
#endif /* NX_DISABLE_IPV6 */

    _switch_hashtables();

    hashset_network_activity_ipv4_t_clear(previous_ipv4_hashtable, _append_ipv4_payload_to_list, &ipv4_list);
#ifndef NX_DISABLE_IPV6
    hashset_network_activity_ipv6_t_clear(previous_ipv6_hashtable, _append_ipv6_payload_to_list, &ipv6_list);
#endif /* NX_DISABLE_IPV6 */

    result = serializer_event_add_network_activity(serializer, current_time, collector_internal_ptr->interval, ipv4_list, ipv6_list);

    /* Release all IPv4 objects. */
    while (ipv4_list != NULL)
    {
        network_activity_ipv4_t *current = ipv4_list;
        ipv4_list = ipv4_list->next;
        network_activity_ipv4_deinit(current, NULL);
    }

#ifndef NX_DISABLE_IPV6
    /* Release all IPv6 objects. */
    while (ipv6_list != NULL)
    {
        network_activity_ipv6_t *current = ipv6_list;
        ipv6_list = ipv6_list->next;
        network_activity_ipv6_deinit(current, NULL);
    }
#endif /* NX_DISABLE_IPV6 */

cleanup:
    if (result != ASC_RESULT_OK)
    {
        log_error("failed to collect events");
    }

    return result;
}

static asc_result_t _collector_network_activity_port_init()
{
    asc_result_t result = ASC_RESULT_OK;

    NX_IP *nx_ip_ptr = _nx_ip_created_ptr;
    ULONG created_count = _nx_ip_created_count;

    if (nx_ip_ptr == NULL)
    {
        log_error("cannot initialize internal port. _nx_ip_created_ptr=[NULL]");
        result = ASC_RESULT_EXCEPTION;
        goto cleanup;
    }

    while (created_count--)
    {
        nx_ip_ptr->nx_ip_packet_filter_extended = _collector_network_activity_port_ip_callback;

#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
        if (nx_ip_ptr->nx_ip_tcp_packet_receive)
        {
            _tcp_packet_receive_original = nx_ip_ptr->nx_ip_tcp_packet_receive;
            nx_ip_ptr->nx_ip_tcp_packet_receive = _collector_network_activity_port_tcp_callback;
        }

        if (nx_ip_ptr->nx_ip_udp_packet_receive)
        {
            _udp_packet_receive_original = nx_ip_ptr->nx_ip_udp_packet_receive;
            nx_ip_ptr->nx_ip_udp_packet_receive = _collector_network_activity_port_udp_callback;
        }

        if (nx_ip_ptr->nx_ip_icmp_packet_receive)
        {
            _icmp_packet_receive_original = nx_ip_ptr->nx_ip_icmp_packet_receive;
            nx_ip_ptr->nx_ip_icmp_packet_receive = _collector_network_activity_port_icmp_callback;
        }
#endif

        nx_ip_ptr = nx_ip_ptr->nx_ip_created_next;
    }

cleanup:
    return result;
}


static UINT _collector_network_activity_port_ip_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr, UINT direction)
{
#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
    /* Incoming packets are captured on the transport layer */
    if (direction == NX_IP_PACKET_IN)
    {
        return NX_SUCCESS;
    }
#endif
    _collector_network_activity_ip_callback(ip_ptr, packet_ptr, direction, BYTE_ORDER_NETWORK);

    /* return NX_SUCCESS, otherwise NetX will drop the packet */
    return NX_SUCCESS;
}


#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
static VOID _collector_network_activity_port_tcp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr)
{
    _collector_network_activity_ip_callback(ip_ptr, packet_ptr, NX_IP_PACKET_IN, BYTE_ORDER_HOST);

    if (_tcp_packet_receive_original)
    {
        /* Continue processing the packet */
        _tcp_packet_receive_original(ip_ptr, packet_ptr);
    }
}

static VOID _collector_network_activity_port_udp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr)
{
    _collector_network_activity_ip_callback(ip_ptr, packet_ptr, NX_IP_PACKET_IN, BYTE_ORDER_HOST);

    if (_udp_packet_receive_original)
    {
        /* Continue processing the packet */
        _udp_packet_receive_original(ip_ptr, packet_ptr);
    }
}

static VOID _collector_network_activity_port_icmp_callback(struct NX_IP_STRUCT *ip_ptr, struct NX_PACKET_STRUCT *packet_ptr)
{
    _collector_network_activity_ip_callback(ip_ptr, packet_ptr, NX_IP_PACKET_IN, BYTE_ORDER_HOST);

    if (_icmp_packet_receive_original)
    {
        /* Continue processing the packet */
        _icmp_packet_receive_original(ip_ptr, packet_ptr);
    }
}
#endif


static void _collector_network_activity_port_deinit()
{
    NX_IP *nx_ip_ptr = _nx_ip_created_ptr;
    ULONG created_count = _nx_ip_created_count;

    if (nx_ip_ptr == NULL)
    {
        log_error("cannot deinitialize internal port. _nx_ip_created_ptr=[NULL]");
    }

    while (created_count--)
    {
        nx_ip_ptr->nx_ip_packet_filter = NULL;

#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY
        nx_ip_ptr->nx_ip_tcp_packet_receive = _tcp_packet_receive_original;
        nx_ip_ptr->nx_ip_udp_packet_receive = _udp_packet_receive_original;
        nx_ip_ptr->nx_ip_icmp_packet_receive = _icmp_packet_receive_original;
#endif

        nx_ip_ptr = nx_ip_ptr->nx_ip_created_next;
    }
}


static void _switch_hashtables()
{
    _current_ipv4_hashtable_index = (_current_ipv4_hashtable_index + 1) % 2;
    _current_ipv4_hashtable = _ipv4_hashtables[_current_ipv4_hashtable_index];

#ifndef NX_DISABLE_IPV6
    _current_ipv6_hashtable_index = (_current_ipv6_hashtable_index + 1) % 2;
    _current_ipv6_hashtable = _ipv6_hashtables[_current_ipv6_hashtable_index];
#endif /* NX_DISABLE_IPV6 */
}


static void _append_ipv4_payload_to_list(network_activity_ipv4_t *data_ptr, void *context)
{
    network_activity_ipv4_t **list = (network_activity_ipv4_t**)context;

    data_ptr->next = *list;
    if (*list != NULL)
    {
        (*list)->previous = data_ptr;
    }

    *list = data_ptr;
}



static network_activity_ipv4_t *_ipv4_callback(VOID *ip_packet, UINT direction, byte_order_t ip_header_byte_order)
{
    network_activity_ipv4_t *result = network_activity_ipv4_init();
    if (result == NULL)
    {
        log_warn("failed to allocate ipv4 object, increase ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE");
        return result;
    }

    uint32_t *ip_header_ptr = (uint32_t*)ip_packet;
    uint32_t ip_header_word_0 = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ntohl(ip_header_ptr[0]) : ip_header_ptr[0];
    uint8_t version_byte = (uint8_t)(ip_header_word_0 >> 24);
    uint16_t ip_header_length = (uint16_t)((version_byte & 0x0f) * 4);

    transport_protocol_t transport_protocol;
    uint32_t protocol_word = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ntohl(ip_header_ptr[2]) : ip_header_ptr[2];
    uint8_t protocol = (protocol_word >> 16) & 0xff;

    switch (protocol)
    {
#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_TCP_DISABLED
        case NX_PROTOCOL_TCP:
            transport_protocol = TRANSPORT_PROTOCOL_TCP;
            break;
#endif
#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_UDP_DISABLED
        case NX_PROTOCOL_UDP:
            transport_protocol = TRANSPORT_PROTOCOL_UDP;
            break;
#endif
#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_ICMP_DISABLED
        case NX_PROTOCOL_ICMP:
            transport_protocol = TRANSPORT_PROTOCOL_ICMP;
            break;
#endif
        default:
            network_activity_ipv4_deinit(result, NULL);
            return NULL;
    }

    uint16_t source_port = 0;
    uint16_t destination_port = 0;

    if (transport_protocol != TRANSPORT_PROTOCOL_ICMP)
    {
        uint32_t *transport_header_ptr = (uint32_t*) (((uint8_t*)ip_packet) + ip_header_length);
        uint32_t ports_word = ntohl(transport_header_ptr[0]);
        source_port = (uint16_t)(ports_word >> 16);
        destination_port = (uint16_t)(ports_word & 0xffff);
    }

    uint16_t total_bytes = (uint16_t)((ip_header_word_0 & 0xffff) - ip_header_length);

    /* For the addresses we keep the network byte order. These will get parsed using inet_ntop. */
    if (direction == NX_IP_PACKET_IN)
    {
        result->common.bytes_in = total_bytes;
        result->common.bytes_out = 0;
        result->common.local_port = destination_port;
        result->common.remote_port = source_port;
        result->local_address = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ip_header_ptr[4] : ntohl(ip_header_ptr[4]);
        result->remote_address = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ip_header_ptr[3] : ntohl(ip_header_ptr[3]);
    }
    else
    {
        /* NX_IP_PACKET_OUT */
        result->common.bytes_in = 0;
        result->common.bytes_out = total_bytes;
        result->common.local_port = source_port;
        result->common.remote_port = destination_port;
        result->local_address = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ip_header_ptr[3] : ntohl(ip_header_ptr[3]);
        result->remote_address = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ip_header_ptr[4] : ntohl(ip_header_ptr[4]);
    }

    result->common.transport_protocol = transport_protocol;

    return result;
}


#ifndef NX_DISABLE_IPV6
static void _append_ipv6_payload_to_list(network_activity_ipv6_t *data_ptr, void *context)
{
    network_activity_ipv6_t **list = (network_activity_ipv6_t**)context;

    data_ptr->next = *list;
    if (*list != NULL)
    {
        (*list)->previous = data_ptr;
    }

    *list = data_ptr;
}


static network_activity_ipv6_t *_ipv6_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr_st, UINT direction, byte_order_t ip_header_byte_order)
{
    VOID * ip_packet = packet_ptr_st->nx_packet_ip_header;
    network_activity_ipv6_t *result = network_activity_ipv6_init();
    if (result == NULL)
    {
        log_warn("failed to allocate ipv6 object, increase ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE");
        return result;
    }

    uint32_t *ip_header_ptr = (uint32_t*)ip_packet;
    uint8_t *packet_ptr = (uint8_t*)ip_packet;

    uint32_t ip_header_word_1 = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ntohl(ip_header_ptr[1]) : ip_header_ptr[1];
    uint8_t next_header_type = (uint8_t)((ip_header_word_1 >> 8) & 0xff);

    uint16_t payload_length = (uint16_t)(ip_header_word_1 >> 16);
    uint16_t header_length = 40;
    packet_ptr += header_length;
    bool found_transport_header = false;

    /*
        Traverse the IPv6 extenstion headers chain
        See following RFCs for header length calculations:
        - RFC-8200 (https://tools.ietf.org/html/rfc8200)
        - RFC-4302 (https://tools.ietf.org/html/rfc4302)
    */
    do
    {
        uint32_t ip_header_word_0 = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ntohl(*(uint32_t *)packet_ptr) : *(uint32_t *)packet_ptr;
        uint8_t length_byte = (ip_header_word_0 >> 16) & 0xff;
        switch (next_header_type)
        {
            case NX_PROTOCOL_NEXT_HEADER_HOP_BY_HOP:
            case NX_PROTOCOL_NEXT_HEADER_DESTINATION:
            case NX_PROTOCOL_NEXT_HEADER_ROUTING:
                header_length = (uint16_t)((length_byte + 1) * 8);
                break;
            case NX_PROTOCOL_NEXT_HEADER_FRAGMENT:
                header_length = 8;
                break;
            case NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION:
                header_length = (uint16_t)((length_byte + 2) * 4);
                break;
#ifndef  ASC_COLLECTOR_NETWORK_ACTIVITY_TCP_DISABLED
            case NX_PROTOCOL_TCP:
#endif
#ifndef  ASC_COLLECTOR_NETWORK_ACTIVITY_UDP_DISABLED
            case NX_PROTOCOL_UDP:
#endif
#ifndef  ASC_COLLECTOR_NETWORK_ACTIVITY_ICMP_DISABLED
            case NX_PROTOCOL_ICMPV6:
#endif
                header_length = 0;
                found_transport_header = true;
                break;
            default:
                network_activity_ipv6_deinit(result, NULL);
                return NULL;
        }

        payload_length = (uint16_t)(payload_length - header_length);
        if (!found_transport_header)
        {
            next_header_type = (uint8_t)(ip_header_word_0 >> 24);
        }
    
        if ((ALIGN_TYPE)(packet_ptr + header_length) >= (ALIGN_TYPE)(packet_ptr_st ->nx_packet_append_ptr))
        {
            network_activity_ipv6_deinit(result, NULL);
            return NULL;
        }
        packet_ptr += header_length;

    } while (!found_transport_header);

    transport_protocol_t transport_protocol;

    if (next_header_type == NX_PROTOCOL_TCP)
    {
        transport_protocol = TRANSPORT_PROTOCOL_TCP;
    }
    else if (next_header_type == NX_PROTOCOL_UDP)
    {
        transport_protocol = TRANSPORT_PROTOCOL_UDP;
    }
    else if (next_header_type == NX_PROTOCOL_ICMPV6)
    {
        transport_protocol = TRANSPORT_PROTOCOL_ICMP;
    }
    else
    {
        network_activity_ipv6_deinit(result, NULL);
        return NULL;
    }

    uint16_t source_port = 0;
    uint16_t destination_port = 0;

    if (transport_protocol != TRANSPORT_PROTOCOL_ICMP)
    {
        uint32_t *transport_header_ptr = (uint32_t*)packet_ptr;
        uint32_t ports_word = ntohl(transport_header_ptr[0]);
        source_port = (uint16_t)(ports_word >> 16);
        destination_port = ports_word & 0xffff;
    }

    if (direction == NX_IP_PACKET_IN)
    {
        result->common.bytes_in = payload_length;
        result->common.bytes_out = 0;
        result->common.local_port = destination_port;
        result->common.remote_port = source_port;
        memmove(result->local_address, ip_header_ptr + 6, 16);
        memmove(result->remote_address, ip_header_ptr + 2, 16);
    }
    else
    {
        /* NX_IP_PACKET_OUT */
        result->common.bytes_in = 0;
        result->common.bytes_out = payload_length;
        result->common.local_port = source_port;
        result->common.remote_port = destination_port;
        memmove(result->local_address, ip_header_ptr + 2, 16);
        memmove(result->remote_address, ip_header_ptr + 6, 16);
    }

    if (ip_header_byte_order == BYTE_ORDER_HOST)
    {
        NX_IPV6_ADDRESS_CHANGE_ENDIAN((ULONG *)result->local_address);
        NX_IPV6_ADDRESS_CHANGE_ENDIAN((ULONG *)result->remote_address);
    }


    result->common.transport_protocol = transport_protocol;

    return result;
}
#endif


static VOID _collector_network_activity_ip_callback(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr, UINT direction, byte_order_t ip_header_byte_order)
{
    VOID * ip_packet = packet_ptr->nx_packet_ip_header;
    uint32_t ip_header_word_0 = (ip_header_byte_order == BYTE_ORDER_NETWORK) ? ntohl(*(uint32_t *)ip_packet) : *(uint32_t *)ip_packet;
    uint8_t version = (uint8_t)(ip_header_word_0 >> 28);

    if (version == NX_IP_VERSION_V4)
    {
        network_activity_ipv4_t *ipv4_object = _ipv4_callback(ip_packet, direction, ip_header_byte_order);
        if (ipv4_object != NULL)
        {
            hashset_network_activity_ipv4_t_add_or_update(_current_ipv4_hashtable, ipv4_object);
        }
    }
    else if (version == NX_IP_VERSION_V6)
    {
#ifndef NX_DISABLE_IPV6
        network_activity_ipv6_t *ipv6_object = _ipv6_callback(ip_ptr, packet_ptr, direction, ip_header_byte_order);
        if (ipv6_object != NULL)
        {
            hashset_network_activity_ipv6_t_add_or_update(_current_ipv6_hashtable, ipv6_object);
        }
#endif
    }
    else
    {
        log_error("illegal IP version, event dropped");
    }
}
