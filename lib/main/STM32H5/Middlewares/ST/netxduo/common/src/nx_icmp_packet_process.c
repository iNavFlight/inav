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
#include "nx_icmp.h"
#include "nx_icmpv6.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_packet_process                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function examines the incoming ICMP received packet and        */
/*    route the packet to the appropriate ICMP packet process handler.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            ICMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*  _nx_icmpv4_packet_process               Process received ICMPv4 packet*/
/*  _nx_icmpv6_packet_process               Process received ICMPv6 packet*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_packet_receive               ICMP packet receive           */
/*    _nx_icmp_queue_process                ICMP packet queue processing  */
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
VOID  _nx_icmp_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* FEATURE_NX_IPV6 not defined */
    if (ip_ptr -> nx_ip_icmpv4_packet_process)
    {
        ip_ptr -> nx_ip_icmpv4_packet_process(ip_ptr, packet_ptr);
        return;
    }

#ifndef NX_DISABLE_ICMP_INFO
    ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif /* NX_DISABLE_ICMP_INFO */

    _nx_packet_release(packet_ptr);
}
#endif /* !NX_DISABLE_IPV4  */

