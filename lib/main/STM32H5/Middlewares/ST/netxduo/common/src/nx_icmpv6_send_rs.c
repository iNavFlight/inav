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

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
static const ULONG _nx_ipv6_all_router_address[4] = {0xff020000, 0, 0, 2};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_send_rs                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends an ICMPv6 Router Solicitation message.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    if_index                              Index of interface            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS: RS packet is sent                                       */
/*    NX_NOT_SUCCESSFUL: RS packet is not sent                            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Computer ICMP checksum        */
/*    _nx_ipv6_packet_send                  Send ICMPv6 packet out        */
/*    _nx_packet_allocate                   Packet allocation function    */
/*    _nxd_ipv6_interface_find              Find outgoing interface for   */
/*                                             sending packet             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_ipv6_router_solicitation_check   IPv6 router solicitation      */
/*                                             timeout routine.           */
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
UINT  _nx_icmpv6_send_rs(NX_IP *ip_ptr, UINT if_index)
{

USHORT           *mac_addr;
NX_PACKET        *pkt_ptr;
USHORT            checksum;
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT              compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_ICMPV6_RS     *rs_ptr;
NX_ICMPV6_OPTION *rs_options;


    /* Do not send RS packet if ICMPv6 is not enabled. */
    if (ip_ptr -> nx_ip_icmpv6_packet_process == NX_NULL)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Allocate a packet to build the ICMPv6 router
       solicitation message in.  */
#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Allocate from auxiliary packet pool first. */
    if (_nx_packet_allocate(ip_ptr -> nx_ip_auxiliary_packet_pool, &pkt_ptr, (NX_ICMP_PACKET + sizeof(NX_ICMPV6_RS) + 8), NX_NO_WAIT))
    {
        if (ip_ptr -> nx_ip_auxiliary_packet_pool != ip_ptr -> nx_ip_default_packet_pool)
#endif /* NX_ENABLE_DUAL_PACKET_POOL */
        {
            if (_nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool,
                                    &pkt_ptr, (NX_ICMP_PACKET + sizeof(NX_ICMPV6_RS) + 8), NX_NO_WAIT))
            {

                /* Error getting packet, so just get out!  */
                return(NX_NOT_SUCCESSFUL);
            }
        }
#ifdef NX_ENABLE_DUAL_PACKET_POOL
        else
        {

            /* Error getting packet, so just get out!  */
            return(NX_NOT_SUCCESSFUL);
        }
    }
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, pkt_ptr);

    /* Find a valid IPv6 address. */
    if (_nxd_ipv6_interface_find(ip_ptr, (ULONG *)_nx_ipv6_all_router_address,
                                 &pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr,
                                 &ip_ptr -> nx_ip_interface[if_index]))
    {
        _nx_packet_release(pkt_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /*lint -e{644} suppress variable might not be initialized, since "pkt_ptr" was initialized in _nx_packet_allocate. */
    pkt_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Set the size of the ICMPv6 router solicitation message. */
    /* Size of the message is ICMPv6 + options, which is 8 bytes. */
    pkt_ptr -> nx_packet_length = (sizeof(NX_ICMPV6_RS) + 8);

    /* Set the prepend pointer. */
    pkt_ptr -> nx_packet_prepend_ptr -= pkt_ptr -> nx_packet_length;

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    rs_ptr = (NX_ICMPV6_RS *)(pkt_ptr -> nx_packet_prepend_ptr);
    rs_ptr -> nx_icmpv6_rs_icmpv6_header.nx_icmpv6_header_type = NX_ICMPV6_ROUTER_SOLICITATION_TYPE;
    rs_ptr -> nx_icmpv6_rs_icmpv6_header.nx_icmpv6_header_code = 0;
    rs_ptr -> nx_icmpv6_rs_icmpv6_header.nx_icmpv6_header_checksum = 0;
    rs_ptr -> nx_icmpv6_rs_reserved = 0;

    /* Get a pointer to the Option header in the ICMPv6 header. */
    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    rs_options = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(rs_ptr, sizeof(NX_ICMPV6_RS));

    /* Fill in the options field */
    rs_options -> nx_icmpv6_option_type = ICMPV6_OPTION_TYPE_SRC_LINK_ADDR;
    rs_options -> nx_icmpv6_option_length = 1;

    /* Fill in the source mac address. */
    mac_addr = &rs_options -> nx_icmpv6_option_data;
    mac_addr[0] = (USHORT)(ip_ptr -> nx_ip_interface[if_index].nx_interface_physical_address_msw);
    mac_addr[1] = (USHORT)((ip_ptr -> nx_ip_interface[if_index].nx_interface_physical_address_lsw & 0xFFFF0000) >> 16); /* lgtm[cpp/overflow-buffer] */
    mac_addr[2] = (USHORT)(ip_ptr -> nx_ip_interface[if_index].nx_interface_physical_address_lsw & 0x0000FFFF); /* lgtm[cpp/overflow-buffer] */

    /* Byte swapping. */
    NX_CHANGE_USHORT_ENDIAN(mac_addr[0]);
    NX_CHANGE_USHORT_ENDIAN(mac_addr[1]); /* lgtm[cpp/overflow-buffer] */
    NX_CHANGE_USHORT_ENDIAN(mac_addr[2]); /* lgtm[cpp/overflow-buffer] */

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (ip_ptr -> nx_ip_interface[if_index].nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Compute checksum.  The returned value is already in network byte order. */
        /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
        checksum = _nx_ip_checksum_compute(pkt_ptr, NX_PROTOCOL_ICMPV6,
                                           (UINT)pkt_ptr -> nx_packet_length,
                                           pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                                           (ULONG *)_nx_ipv6_all_router_address);

        checksum = (USHORT)(~checksum);

        /* Byte swapping. */
        NX_CHANGE_USHORT_ENDIAN(checksum);

        rs_ptr -> nx_icmpv6_rs_icmpv6_header.nx_icmpv6_header_checksum = checksum;
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        pkt_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /*lint -e{929} suppress cast of pointer to pointer, since it is necessary  */
    _nx_ipv6_packet_send(ip_ptr, pkt_ptr, NX_PROTOCOL_ICMPV6, pkt_ptr -> nx_packet_length, 255,
                         pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                         (ULONG *)_nx_ipv6_all_router_address);

    return(NX_SUCCESS);
}
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */
#endif /* FEATURE_NX_IPV6 */

