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
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_process_routing_option                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the Routing Option header.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to process  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                             Successful completion        */
/*    NX_OPTION_HEADER_ERROR                 Error parsing router option  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_packet_receive                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_ipv6_process_routing_option(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_IPV6_HEADER_ROUTING_OPTION *option;
#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
UINT                           base_offset;
#endif


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Check packet length is at least sizeof(NX_IPV6_HEADER_ROUTING_OPTION). */
    if (packet_ptr -> nx_packet_length < sizeof(NX_IPV6_HEADER_ROUTING_OPTION))
    {
        return(NX_OPTION_HEADER_ERROR);
    }

    /* Set a pointer to the routing header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    option = (NX_IPV6_HEADER_ROUTING_OPTION *)(packet_ptr -> nx_packet_prepend_ptr);

    if (option -> nx_ipv6_header_routing_option_segments_left == 0)
    {
        /* Skip the rest of the routing header and continue processing this packet. */
        return(NX_SUCCESS);
    }

    /* According to RFC 5095, Routing Header 0 (described in RFC 2460) has been
       deprecated.  Therefore discard the packet that has segments left, and send
       an ICMP Parameter Problem if such feature is enabled. */

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE

    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    base_offset = (UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_ip_header);

    /*lint -e{835} -e{845} suppress operating on zero. */
    NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 0, base_offset + 2);
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
#endif

    /* Return error status, so the caller knows to free the packet. */
    return(NX_OPTION_HEADER_ERROR);
}


#endif /*  FEATURE_NX_IPV6 */

