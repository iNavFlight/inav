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

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_validate_neighbor_message                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function validates incoming neighbor advertisement    */
/*    message and neighbor solicitation message.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                   ICMPv6 packet to validate              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                   ICMPv6 options are valid               */
/*    NX_NOT_SUCCESS               ICMPv6 options are invalid             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    IPv6_Address_Type             Determine address type (e.g. unicast) */
/*    _nx_icmpv6_validate_options   Validate option fields in ICMP header */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_ns         Process received NS packet            */
/*    _nx_icmpv6_process_na         Process received NA packet            */
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
UINT _nx_icmpv6_validate_neighbor_message(NX_PACKET *packet_ptr)
{

NX_ICMPV6_ND     *nd_header_ptr;
NX_IPV6_HEADER   *ipv6_header;
NX_ICMPV6_OPTION *option_ptr;
UINT              option_length;
UINT              option_check;
ULONG             source_address_type;
ULONG             dest_address_type;


    /* Points to the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    /*
     * Validate the IP header information.
     * The following validation procedure is defined in RFC2461
     * 7.1.1: Validation of Neighbor Solicitations
     * 7.1.2: Validation of Neighbor Advertisements
     */

    /* Hop limit must be 255, I.e., the packet could not possibly have been forwarded by a router. */
    if ((ipv6_header -> nx_ip_header_word_1 & 0xFF) != 0xFF)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Points to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    nd_header_ptr = (NX_ICMPV6_ND *)packet_ptr -> nx_packet_prepend_ptr;

    /* ICMP code must be 0 */
    if (nd_header_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_code != 0)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* ICMP length (derived from the IP Length) is 24 or more. */
    if (packet_ptr -> nx_packet_length < 24)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Target Address must not be a multicast address. */
    if ((nd_header_ptr -> nx_icmpv6_nd_targetAddress[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Find out the destination IP address type.  */
    dest_address_type = IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip);

    option_check = 0;

    if (nd_header_ptr -> nx_icmpv6_nd_header.nx_icmpv6_header_type == NX_ICMPV6_NEIGHBOR_SOLICITATION_TYPE)
    {

        /* Find out the source IP address type.  */
        source_address_type = IPv6_Address_Type(ipv6_header -> nx_ip_header_source_ip);

        /* If the IP source address is the unspecified address, the IP destination address
           is a solicated-node multicast address. */
        if (source_address_type == IPV6_ADDRESS_UNSPECIFIED)
        {

            if ((dest_address_type & IPV6_SOLICITED_NODE_MCAST) != IPV6_SOLICITED_NODE_MCAST)
            {
                return(NX_NOT_SUCCESSFUL);
            }

            /* if the IP source address is the unspecified address, it must not contain
               source link-layer address option in the message. */
            option_check = NX_NO_SLLA;
        }
    }
    else
    {

        /* If the IP Destination Address is a multicast address, the
           Solicted flag must be zero. */
        if (((ipv6_header -> nx_ip_header_destination_ip[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000) &&
            nd_header_ptr -> nx_icmpv6_nd_flag & 0x40000000)
        {

            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* Locate the option field. */
    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    option_ptr    = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(nd_header_ptr, sizeof(NX_ICMPV6_ND));
    option_length = (UINT)(packet_ptr -> nx_packet_length - sizeof(NX_ICMPV6_ND));

    /* Validate option fields if there are any. */
    if (option_length)
    {
        return(_nx_icmpv6_validate_options(option_ptr, (INT)option_length, (INT)option_check));
    }

    return(NX_SUCCESS);
}


#endif /* FEATURE_NX_IPV6 */

