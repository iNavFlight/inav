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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_packet_receive                               PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a packet from the link driver (usually the   */
/*    link driver's input ISR) and either processes it or places it in a  */
/*    deferred processing queue, depending on the complexity of the       */
/*    packet.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to received packet    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    (ipv4_packet_receive)                 Receive an IPv4 packet        */
/*    (ipv6_packet_receive)                 Receive an IPv6 packet        */
/*    _nx_packet_release                    Packet release                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application I/O Driver                                              */
/*    _nx_ip_packet_send                    IP loopback packet send       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            added new ip filter,        */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ip_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UCHAR ip_version;
UCHAR version_byte;


#ifndef NX_DISABLE_IP_INFO
    /* Increment the IP packet count.  */
    ip_ptr -> nx_ip_total_packets_received++;
#endif

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* If packet_ptr -> nx_packet_interface_ptr is not set, stamp the packet with interface[0].
       Legacy Ethernet drivers do not stamp incoming packets. */
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr == NX_NULL)
    {
        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &(ip_ptr -> nx_ip_interface[0]);
    }

    /* It's assumed that the IP link driver has positioned the top pointer in the
       packet to the start of the IP address... so that's where we will start.  */
    version_byte =  *(packet_ptr -> nx_packet_prepend_ptr);

    /* Check the version number */
    ip_version = (version_byte >> 4);

    packet_ptr -> nx_packet_ip_version = ip_version;

    packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;

#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Check if the IP packet filter is set. */
    if (ip_ptr -> nx_ip_packet_filter)
    {

        /* Yes, call the IP packet filter routine. */
        if (ip_ptr -> nx_ip_packet_filter((VOID *)(packet_ptr -> nx_packet_prepend_ptr),
                                          NX_IP_PACKET_IN) != NX_SUCCESS)
        {

            /* Drop the packet. */
            _nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Check if the IP packet filter extended is set. */
    if (ip_ptr -> nx_ip_packet_filter_extended)
    {

        /* Yes, call the IP packet filter extended routine. */
        if (ip_ptr -> nx_ip_packet_filter_extended(ip_ptr, packet_ptr, NX_IP_PACKET_IN) != NX_SUCCESS)
        {

            /* Drop the packet. */
            _nx_packet_release(packet_ptr);
            return;
        }
    }
#endif /* NX_ENABLE_IP_PACKET_FILTER */

#ifndef NX_DISABLE_IPV4

    /* Process the packet according to IP version. */
    if (ip_version == NX_IP_VERSION_V4 && ip_ptr -> nx_ipv4_packet_receive)
    {

        /* Call the IPv4 packet handler. */
        (ip_ptr -> nx_ipv4_packet_receive)(ip_ptr, packet_ptr);
        return;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (ip_version == NX_IP_VERSION_V6 && ip_ptr -> nx_ipv6_packet_receive)
    {

        /* Call the IPv6 packet handler. */
        (ip_ptr -> nx_ipv6_packet_receive)(ip_ptr, packet_ptr);
        return;
    }
#endif /* FEATURE_NX_IPV6 */

    /* Either the ip_version number is unkonwn, or the ip_packet_receive function is
        not defined.  In this case, the packet is reclaimed. */

#ifndef NX_DISABLE_IP_INFO

    /* Increment the IP invalid packet error.  */
    ip_ptr -> nx_ip_invalid_packets++;

    /* Increment the IP receive packets dropped count.  */
    ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

    _nx_packet_release(packet_ptr);

    return;
}

