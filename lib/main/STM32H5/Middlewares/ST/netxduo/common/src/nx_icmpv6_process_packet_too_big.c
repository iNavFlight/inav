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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"
#include "nx_packet.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef FEATURE_NX_IPV6


#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_process_packet_too_big                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles an ICMPv6 error message indicating the next   */
/*    hop or beyond has a smaller MTU that the packet payload.  It sets   */
/*    the IP task MTU to the next hop MTU in the option header.           */
/*                                                                        */
/*    This function is called from IP thread periodic process routine.    */
/*    It starts the minimum Path MTU Discovery process if the interface   */
/*    address after the IP instance is initialized and its MTU established*/
/*    by the Ethernet driver.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    packet_ptr                            Pointer to Packet Too Big     */
/*                                            ICMPv6 packet received      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*     _nx_icmpv6_dest_table_find         Find destination in table and   */
/*                                             update path MTU in table   */
/*    memset                              Set the memory                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
UINT _nx_icmpv6_process_packet_too_big(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UINT                       status = NX_INVALID_MTU_DATA;
NX_ICMPV6_OPTION_MTU      *icmpv6_mtu_option_ptr;
ULONG                      mtu;
NX_IPV6_DESTINATION_ENTRY *dest_entry_ptr;
ULONG                     *original_destination_ip;
ULONG                      default_next_hop_address[4];
NX_IPV6_HEADER            *ip_header_ptr, *original_ip_header_ptr;
NX_INTERFACE              *if_ptr;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_RX_SIZE_CHECKING
    if (packet_ptr -> nx_packet_length < sizeof(NX_ICMPV6_OPTION_MTU))
    {
#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid message count.  */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif

        /* Invalid ICMP message, just release it.  */
        _nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING */

    memset(&default_next_hop_address[0], 0, (4 * sizeof(ULONG)));

    if_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

    /* Set a local pointer to the ICMPv6 Option data. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    icmpv6_mtu_option_ptr = (NX_ICMPV6_OPTION_MTU *)packet_ptr -> nx_packet_prepend_ptr;

    /* Parse the original sender data. */
    /*lint -e{929} -e{826} -e{740} suppress cast of pointer to pointer, since it is necessary  */
    original_ip_header_ptr = (NX_IPV6_HEADER *)(&(icmpv6_mtu_option_ptr ->  nx_icmpv6_option_mtu_message));

    /* Extract the original sender from the IP header. */
    packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_IPV6_HEADER);

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;


    /* Yes this looks like a valid IP header for the original offending packet. */
    original_destination_ip = original_ip_header_ptr -> nx_ip_header_destination_ip;
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(original_destination_ip);
    COPY_IPV6_ADDRESS(ip_header_ptr -> nx_ip_header_source_ip, &default_next_hop_address[0]);

    /*  Handle endianness. */
    NX_CHANGE_ULONG_ENDIAN(icmpv6_mtu_option_ptr -> nx_icmpv6_option_mtu_path_mtu);

    /* Create a local variable for convenience. */
    mtu = icmpv6_mtu_option_ptr -> nx_icmpv6_option_mtu_path_mtu;

    /* MTU data is valid if it is non zero, and is no more than the driver MTU. */
    if ((mtu > 0) && (mtu <= if_ptr -> nx_interface_ip_mtu_size))
    {

        /* Add destination table. */
        status = _nx_icmpv6_dest_table_add(ip_ptr, original_destination_ip,
                                           &dest_entry_ptr, &default_next_hop_address[0], mtu, 0,
                                           packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr);
    }

    /* Release the packet. */
    _nx_packet_release(packet_ptr);

    /* Return the completion code. */
    return(status);
}
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */


#endif /* FEATURE_NX_IPV6 */

