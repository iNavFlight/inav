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
/**   Internet Group Management Protocol (IGMP)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_igmp.h"
#include "nx_ipv4.h"
#include "nx_packet.h"


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_interface_report_send                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds and sends an IGMP report.  If it is a JOIN     */
/*    report, the IP nx_igmp_reports_sent statistic is incremented.       */
/*                                                                        */
/*    Note: An IGMPv1 host does not send a LEAVE message. The caller in   */
/*    that case, _nx_igmp_multicast_interface_leave_internal, checks the  */
/*    IGMP host version and only calls this function for IGMPv2 hosts.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    group_address                         Multicast group to join       */
/*    interface_index                       Index to the interface        */
/*    is_joining                            Indicate if joining or leaving*/
/*                                            NX_TRUE = send join report  */
/*                                            NX_FALSE = send leave report*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_send                    Send packet from the IP layer */
/*    _nx_packet_allocate                   Allocate a packet for report  */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_igmp_periodic_processing           Performs periodic IGMP tasks  */
/*    nx_igmp_multicast_interface_leave_internal                          */
/*                                          Processes a LEAVE report for  */
/*                                            transmission to all routers */
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
UINT  _nx_igmp_interface_report_send(NX_IP *ip_ptr, ULONG group_address, UINT interface_index, UINT is_joining)
{

NX_INTERFACE   *nx_interface;
UINT            router_alert = 0;
UINT            status;
ULONG           checksum;
ULONG           temp;
NX_PACKET      *packet_ptr;
NX_IGMP_HEADER *header_ptr;


#ifndef NX_DISABLE_IGMPV2
    if (ip_ptr -> nx_ip_igmp_router_version == NX_IGMP_HOST_VERSION_2)
    {
        router_alert = 4;
    }
#endif

    /* Obtain the IP mutex so we can search the multicast join list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    nx_interface = &ip_ptr -> nx_ip_interface[interface_index];

    /* Build an IGMP host response packet and send it!  */

    /* Allocate a packet to place the IGMP host response message in.  */
#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Allocate from auxiliary packet pool first. */
    status = _nx_packet_allocate(ip_ptr -> nx_ip_auxiliary_packet_pool, &packet_ptr, (ULONG)(NX_IGMP_PACKET + router_alert + NX_IGMP_HEADER_SIZE), TX_NO_WAIT);
    if ((status != NX_SUCCESS) && (ip_ptr -> nx_ip_auxiliary_packet_pool != ip_ptr -> nx_ip_default_packet_pool))
#endif /* NX_ENABLE_DUAL_PACKET_POOL */
    {
        status = _nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &packet_ptr, (ULONG)(NX_IGMP_PACKET + router_alert + NX_IGMP_HEADER_SIZE), TX_NO_WAIT);
    }

    if (status)
    {

        /* Packet allocation failed. Release the mutex and return error status. */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(status);
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Prepare an IGMP response and send on the "all hosts" multicast
       address.  */

#ifndef NX_DISABLE_IGMP_INFO
    /* Increase the IGMP reports sent count.  */
    if (is_joining == NX_TRUE)
    {
        ip_ptr -> nx_ip_igmp_reports_sent++;
    }

#endif

    /* Calculate the IGMP response message size and store it in the
       packet header.  */
    /*lint -e{644} suppress variable might not be initialized, since "packet_ptr" was initialized as long as status is NX_SUCCESS. */
    packet_ptr -> nx_packet_length =  NX_IGMP_HEADER_SIZE;

    /* Setup the prepend pointer.  */
    packet_ptr -> nx_packet_prepend_ptr -= NX_IGMP_HEADER_SIZE;

    /* Stamp the outgoing interface. */
    packet_ptr -> nx_packet_address.nx_packet_interface_ptr = nx_interface;

    /* Build the IGMP host response packet.  */

    /* Setup the pointer to the message area.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_IGMP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

#ifndef NX_DISABLE_IGMPV2

    /* Build the IGMPv2 response message.  */

    /* Is the router using IGMPv1? */
    if (ip_ptr -> nx_ip_igmp_router_version == NX_IGMP_HOST_VERSION_1)
    {
#endif /* NX_DISABLE_IGMPV2 */

        /* Yes; Set the header fields with the max response time
           zero and the version/type 0x12. */
        header_ptr -> nx_igmp_header_word_0 =  (ULONG)(NX_IGMP_VERSION | NX_IGMP_HOST_RESPONSE_TYPE);
        header_ptr -> nx_igmp_header_word_1 =  group_address;
#ifndef NX_DISABLE_IGMPV2
    }
    /* The router is running the IGMPv2 (or higher) protocol. */
    else
    {

        /* Indicate if the report is a join or leave report. */
        if (is_joining)
        {

            header_ptr -> nx_igmp_header_word_0 =  (ULONG)(NX_IGMP_HOST_V2_JOIN_TYPE);
        }
        else
        {
            header_ptr -> nx_igmp_header_word_0 =  (ULONG)(NX_IGMP_HOST_V2_LEAVE_TYPE);
        }

        header_ptr -> nx_igmp_header_word_1 =  group_address;
    }
#endif /* NX_DISABLE_IGMPV2 */


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (!(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM))
#endif /* NX_ENABLE_INTERFACE_CAPABILITY  */
    {

        /* Calculate the checksum.  */
        temp =      header_ptr -> nx_igmp_header_word_0;
        checksum =  (temp >> NX_SHIFT_BY_16);
        checksum += (temp & NX_LOWER_16_MASK);
        temp =      header_ptr -> nx_igmp_header_word_1;
        checksum += (temp >> NX_SHIFT_BY_16);
        checksum += (temp & NX_LOWER_16_MASK);

        /* Add in the carry bits into the checksum.  */
        checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

        /* Do it again in case previous operation generates an overflow.  */
        checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

        /* Place the checksum into the first header word.  */
        header_ptr -> nx_igmp_header_word_0 =  header_ptr -> nx_igmp_header_word_0 | (~checksum & NX_LOWER_16_MASK);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* IGMPv2 packets must be IPv4 packets. */
    packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V4;

    /* If NX_LITTLE_ENDIAN is defined, the headers need to be swapped.  */
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_igmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_igmp_header_word_1);

    /* Send the IGMP response packet out!  */
    if (is_joining == NX_TRUE)
    {

        /* For JOIN reports, set the packet destination to the group address. */
        _nx_ip_packet_send(ip_ptr, packet_ptr,
                           group_address,
                           NX_IP_NORMAL, NX_IGMP_TTL, NX_IP_IGMP, NX_FRAGMENT_OKAY,
                           group_address);
    }
    else
    {

        /* For LEAVE reports, set the destination to ALL ROUTERS as per RFC 2236 Section 3 page 4.*/
        _nx_ip_packet_send(ip_ptr, packet_ptr,
                           NX_ALL_ROUTERS_ADDRESS,
                           NX_IP_NORMAL, NX_IGMP_TTL, NX_IP_IGMP, NX_FRAGMENT_OKAY,
                           NX_ALL_ROUTERS_ADDRESS);
    }

    /* Release the protection over the IP instance.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return NX_SUCCESS;
}
#endif /* NX_DISABLE_IPV4 */

