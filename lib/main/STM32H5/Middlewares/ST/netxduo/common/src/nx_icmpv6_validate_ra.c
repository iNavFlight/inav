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


#ifndef NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_validate_ra                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function validates incoming router advertisement messages.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            ICMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion (no     */
/*                                            option fields to validate)  */
/*    NX_NOT_SUCCESSFUL                     Invalid ICMP header data      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    IPv6_Address_Type                     Find IP address type.         */
/*    _nx_icmpv6_validate_options           Validate option field.        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_ra                 Main ICMP packet pocess       */
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
UINT _nx_icmpv6_validate_ra(NX_PACKET *packet_ptr)
{

NX_IPV6_HEADER   *ipv6_header;
NX_ICMPV6_RA     *header_ptr;
NX_ICMPV6_OPTION *option_ptr;
INT               option_length;
ULONG             source_address_type, dest_address_type;


    /* Set a pointer to he ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV6_RA *)packet_ptr -> nx_packet_prepend_ptr;

    /* Set a pointer to the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    source_address_type = IPv6_Address_Type(ipv6_header -> nx_ip_header_source_ip);
    dest_address_type = IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip);

    /* Validate the IP header information. */

    /*  The source address must be the link local router address. RFC2461 4.2 */
    if ((source_address_type & IPV6_ADDRESS_LINKLOCAL) != IPV6_ADDRESS_LINKLOCAL)
    {

        return(NX_NOT_SUCCESSFUL);
    }

    /* IP destination address must be multicast address or solicited sender link local address. */
    if ((dest_address_type  != (ULONG)(IPV6_ADDRESS_LINKLOCAL | IPV6_ADDRESS_UNICAST)) &&
        (dest_address_type  != (ULONG)(IPV6_ALL_NODE_MCAST | IPV6_ADDRESS_MULTICAST)))
    {

        return(NX_NOT_SUCCESSFUL);
    }

    /*  The IP header hop limit must be 255 */
    if ((ipv6_header -> nx_ip_header_word_1 & 0xFF) != 0xFF)
    {

        return(NX_NOT_SUCCESSFUL);
    }

    /* Validate ICMP fields */
    if (header_ptr -> nx_icmpv6_ra_icmpv6_header.nx_icmpv6_header_code != 0)
    {

        return(NX_NOT_SUCCESSFUL);
    }

    /* Locate the option field. */
    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    option_ptr = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(header_ptr, sizeof(NX_ICMPV6_RA));
    option_length = (INT)(packet_ptr -> nx_packet_length - sizeof(NX_ICMPV6_RA));

    /* Check for options (if there is a non zero option length ICMPv6 header field). */
    if (option_length)
    {

        /* Validate option field(s). */
        return(_nx_icmpv6_validate_options(option_ptr, option_length, NX_NULL));
    }

    return(NX_SUCCESS);
}


#endif /* NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS */

#endif /* FEATURE_NX_IPV6 */

