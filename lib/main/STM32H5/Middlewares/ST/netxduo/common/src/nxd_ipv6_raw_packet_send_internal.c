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

#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_raw_packet_send_internal                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is the internal function NetX Duo uses to send a raw IP packet */
/*    through the specified IPv6 interface.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    protocol                              Information that goes into    */
/*                                             the next header field.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_packet_send                  Core IPv6 packet send service */
/*    _nxd_ipv6_interface_find              Determines a valid interface  */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    COPY_IPV6_ADDRESS                     Copy IPv6 address             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nxd_ipv6_raw_packet_send_internal(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                         NXD_ADDRESS *destination_ip,
                                         ULONG protocol)
{

#ifdef TX_ENABLE_EVENT_TRACE
ULONG ip_address_lsw;
#endif /* TX_ENABLE_EVENT_TRACE */


#ifdef TX_ENABLE_EVENT_TRACE

    ip_address_lsw = destination_ip -> nxd_ip_address.v6[3];

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_RAW_PACKET_SEND, ip_ptr, ip_address_lsw, protocol, packet_ptr, NX_TRACE_IP_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE */

    NX_ASSERT(packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr != NX_NULL);

    /* Tag the IP version for this packet. */
    packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Ok to send the packet! */
    _nx_ipv6_packet_send(ip_ptr, packet_ptr, protocol, packet_ptr -> nx_packet_length, ip_ptr -> nx_ipv6_hop_limit,
                         packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                         destination_ip -> nxd_ip_address.v6);

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
}
#endif /* FEATURE_NX_IPV6 */

