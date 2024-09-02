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
/*    _nx_ipv6_option_error                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles an invalid Options header packet by examining */
/*    the option header option type's most significant bits and           */
/*    determining if an error message is sent and if the rest of the      */
/*    packet should be processed or discarded.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    option_type                           The type of option            */
/*    offset                                Where the error occurs        */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                             Skip this option; no errors  */
/*    NX_OPTION_HEADER_ERROR                 Error; Drop the entire packet*/
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_process_hop_by_hop_option                                  */
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
UINT _nx_ipv6_option_error(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UCHAR option_type, UINT offset)
{

UINT rv = NX_SUCCESS;

/*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
NX_IPV6_HEADER *ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    /* Top 2 bits of the option type indicate how we shall process this option
       in case of an error. */
    switch (option_type >> 6)
    {

    case 3: /* Discard the packet and send ICMP Parameter Problem to unicast address */
        if ((ip_header_ptr -> nx_ip_header_destination_ip[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000)
        {

            /* If the destination address is a multicast address, we discard the packet. */
            rv = NX_OPTION_HEADER_ERROR;
            break;
        }
    /*
       No need to break here:  execute the next two cases:
       (1) transmit ICMP error message
       (2) release the packet.
     */

    /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
    case 2: /* Discard the packet and send ICMP Parameter Problem */

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE

        NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 2, (ULONG)(offset + sizeof(NX_IPV6_HEADER)));
#else
        NX_PARAMETER_NOT_USED(ip_ptr);
        NX_PARAMETER_NOT_USED(offset);
#endif

    /* No break here.  Execute the next statement to release the packet. */

    /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
    case 1: /* Discard the packet */

        /* Error status - Drop the packet */
        rv = NX_OPTION_HEADER_ERROR;
        break;

    case 0: /* Skip over this option and continue processing the rest of the packet. */
    default:
        break;
    }

    return(rv);
}


#endif /*  FEATURE_NX_IPV6 */

