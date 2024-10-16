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
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"


#ifdef FEATURE_NX_IPV6
static const ULONG _nx_ipv6_unspecified_address[4] = {0, 0, 0, 0};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_send_ns                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function sends out an ICMPv6 Neighbor Solicitation (NS) message.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    targetIPAddr                          Target IPv6 Address           */
/*    send_slla                             Send Source Link Layer Address*/
/*    outgoing_address                      IP interface to transmit the  */
/*                                                 packet out on          */
/*    sendUnicast                           Send out a unicast NS         */
/*    NDCacheEntry                          Pointer to ND cache entry     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Computer ICMP checksum        */
/*    _nx_ipv6_packet_send                  Send packet out               */
/*    _nx_packet_allocate                   Packet allocate function      */
/*    _nx_packet_release                    Packet release function       */
/*    _nx_ipv6_header_add                   Add IPv6 header               */
/*    (ip_link_driver)                      User supplied link driver     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_packet_process             Main ICMP packet pocess       */
/*    _nx_icmpv6_perform_DAD                Procedure for Duplicate       */
/*                                             Address Detection.         */
/*    _nx_ipv6_packet_send                  IPv6 packet transmit process  */
/*    _nx_nd_cache_periodic_update          ND Cache timeout routine.     */
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
VOID _nx_icmpv6_send_ns(NX_IP                 *ip_ptr,
                        ULONG                 *neighbor_IP_address,
                        INT                    send_slla,
                        NXD_IPV6_ADDRESS      *outgoing_address,
                        INT                    sendUnicast,
                        ND_CACHE_ENTRY        *NDCacheEntry)
{

NX_PACKET    *pkt_ptr;
USHORT        checksum;
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT          compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_ICMPV6_ND *nd_ptr;
ULONG        *src_address;
ULONG         dest_address[4];
NX_IP_DRIVER  driver_request;


    /* Allocate a packet to build the ICMPv6 NS message in.  */
#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Allocate from auxiliary packet pool first. */
    if (_nx_packet_allocate(ip_ptr -> nx_ip_auxiliary_packet_pool, &pkt_ptr, NX_IPv6_ICMP_PACKET, NX_NO_WAIT))
    {
        if (ip_ptr -> nx_ip_auxiliary_packet_pool != ip_ptr -> nx_ip_default_packet_pool)
#endif /* NX_ENABLE_DUAL_PACKET_POOL */
        {
            if (_nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &pkt_ptr, NX_IPv6_ICMP_PACKET, NX_NO_WAIT))
            {

                /* Error getting packet, so just get out!  */
                return;
            }
        }
#ifdef NX_ENABLE_DUAL_PACKET_POOL
        else
        {

            /* Error getting packet, so just get out!  */
            return;
        }
    }
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, pkt_ptr);

    /* Mark the packet as IPv6 packet. */
    /*lint -e{644} suppress variable might not be initialized, since "pkt_ptr" was initialized in _nx_packet_allocate. */
    pkt_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Setup the size of the ICMPv6 NS message */
    pkt_ptr -> nx_packet_length = sizeof(NX_ICMPV6_ND);

    /* Add 8 more bytes if sending source link layer address. */
    if (send_slla)
    {
        pkt_ptr -> nx_packet_length += 8;
    }

    /* Check to see if the packet has enough room to fill with NS.  */
    if ((UINT)(pkt_ptr -> nx_packet_data_end - pkt_ptr -> nx_packet_prepend_ptr) < pkt_ptr -> nx_packet_length)
    {

        /* Error getting packet, so just get out!  */
        _nx_packet_release(pkt_ptr);
        return;
    }

    /* Setup the append pointer to the end of the message. */
    pkt_ptr -> nx_packet_append_ptr = pkt_ptr -> nx_packet_prepend_ptr + pkt_ptr -> nx_packet_length;

    /* Set up the ND message in the buffer. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    nd_ptr = (NX_ICMPV6_ND *)(pkt_ptr -> nx_packet_prepend_ptr);
    nd_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_type = NX_ICMPV6_NEIGHBOR_SOLICITATION_TYPE;
    nd_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_code = 0;
    nd_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_checksum = 0;
    nd_ptr -> nx_icmpv6_nd_flag = 0;

    /* copy the target IP address */
    COPY_IPV6_ADDRESS(neighbor_IP_address, nd_ptr -> nx_icmpv6_nd_targetAddress);

    /* Convert the IP address to network byte order. */
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(nd_ptr -> nx_icmpv6_nd_targetAddress);

    if (sendUnicast)
    {

        COPY_IPV6_ADDRESS(neighbor_IP_address, dest_address);
    }
    else
    {

        /* Set up the next hop address, which is the target host's Solicited-Node
           Multicast Address.  The address is formed by taking the last 24 bits of
           the target IP address, in the form of:
           0xFF02:0000:0000:0000:0000:0001:FFxx:xxxx */
        SET_SOLICITED_NODE_MULTICAST_ADDRESS(dest_address, neighbor_IP_address);
    }

    /* Set up source IP address to use for this packet.
       If the global address is not valid yet, we use the unspecified address (::)
       Otherwise the global address is used */
    if (outgoing_address -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID)
    {

        src_address = outgoing_address -> nxd_ipv6_address;
    }
    else
    {

        /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
        src_address = (ULONG *)_nx_ipv6_unspecified_address;
    }

    pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = outgoing_address;

    /* outgoing_address -> nxd_ipv6_address_attached can not be NULL. */
    NX_ASSERT(outgoing_address -> nxd_ipv6_address_attached != NX_NULL);

    if (send_slla)  /* Need to send SLLA option */
    {

    USHORT           *mac_addr;
    NX_ICMPV6_OPTION *nd_options;

        /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
        nd_options = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(nd_ptr, sizeof(NX_ICMPV6_ND));

        /* Fill in the options field */
        nd_options -> nx_icmpv6_option_type = 1;
        nd_options -> nx_icmpv6_option_length = 1;

        /* Fill in the source MAC address */
        mac_addr = &nd_options ->  nx_icmpv6_option_data;
        mac_addr[0] = (USHORT)(outgoing_address -> nxd_ipv6_address_attached -> nx_interface_physical_address_msw);
        mac_addr[1] = (USHORT)((outgoing_address -> nxd_ipv6_address_attached -> nx_interface_physical_address_lsw & 0xFFFF0000) >> 16); /* lgtm[cpp/overflow-buffer] */
        mac_addr[2] = (USHORT)(outgoing_address -> nxd_ipv6_address_attached -> nx_interface_physical_address_lsw & 0x0000FFFF); /* lgtm[cpp/overflow-buffer] */

        /* Byte swapping. */
        NX_CHANGE_USHORT_ENDIAN(mac_addr[0]);
        NX_CHANGE_USHORT_ENDIAN(mac_addr[1]); /* lgtm[cpp/overflow-buffer] */
        NX_CHANGE_USHORT_ENDIAN(mac_addr[2]); /* lgtm[cpp/overflow-buffer] */
    }

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (outgoing_address -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        /* Compute checksum.  The return value is already in network byte order */
        checksum = _nx_ip_checksum_compute(pkt_ptr, NX_PROTOCOL_ICMPV6, (UINT)pkt_ptr -> nx_packet_length, src_address, dest_address);

        checksum = (USHORT)(~checksum);

        /* Byte swapping. */
        NX_CHANGE_USHORT_ENDIAN(checksum);

        nd_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_checksum = checksum;
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        pkt_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Add IPv6 header. */
    if (_nx_ipv6_header_add(ip_ptr, &pkt_ptr, NX_PROTOCOL_ICMPV6, pkt_ptr -> nx_packet_length,
                            255, src_address, dest_address, NX_NULL) != NX_SUCCESS)
    {

        /* Failed to add header. */
        return;
    }

    /* Build the driver request. */
    driver_request.nx_ip_driver_ptr                  = ip_ptr;
    driver_request.nx_ip_driver_command              = NX_LINK_PACKET_SEND;
    driver_request.nx_ip_driver_packet               = pkt_ptr;
    driver_request.nx_ip_driver_interface            = outgoing_address -> nxd_ipv6_address_attached;
    if (sendUnicast)
    {
    UCHAR *mac_addr;
        mac_addr = NDCacheEntry -> nx_nd_cache_mac_addr;

        /* Set unicast destination MAC. */
        driver_request.nx_ip_driver_physical_address_msw = ((ULONG)mac_addr[0] << 8) | mac_addr[1];
        driver_request.nx_ip_driver_physical_address_lsw =
            ((ULONG)mac_addr[2] << 24) | ((ULONG)mac_addr[3] << 16) | ((ULONG)mac_addr[4] << 8) | mac_addr[5];
    }
    else
    {

        /*lint -e{644} suppress variable might not be initialized, since dest_address was initialized. */
        driver_request.nx_ip_driver_physical_address_msw = 0x00003333;
        driver_request.nx_ip_driver_physical_address_lsw = dest_address[3];
    }

#ifndef NX_DISABLE_IP_INFO

    /* Increment the IP packet sent count.  */
    ip_ptr -> nx_ip_total_packets_sent++;

    /* Increment the IP bytes sent count.  */
    ip_ptr -> nx_ip_total_bytes_sent +=  pkt_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER);
#endif

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, pkt_ptr);

    /* Driver entry must not be NULL. */
    NX_ASSERT(outgoing_address -> nxd_ipv6_address_attached -> nx_interface_link_driver_entry != NX_NULL);

    /* Send the IP packet out on the network via the attached driver.  */
    (outgoing_address -> nxd_ipv6_address_attached -> nx_interface_link_driver_entry)(&driver_request);
}

#endif /* FEATURE_NX_IPV6 */

