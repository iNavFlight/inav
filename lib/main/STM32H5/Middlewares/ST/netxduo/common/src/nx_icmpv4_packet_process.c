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
#include "nx_icmp.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv4_packet_process                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the ICMPv4 received packet and lifts any    */
/*    associated threads suspended on it.                                 */
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
/*    _nx_ip_checksum_compute               Compute checksum              */
/*    _nx_ip_packet_send                    Send ICMP packet out          */
/*    _nx_packet_release                    Release packet to packet pool */
/*    _tx_thread_system_resume              Resume suspended thread       */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_packet_process               Main ICMP packet pocess       */
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
VOID  _nx_icmpv4_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_ICMPV4_HEADER *header_ptr;
USHORT            checksum;
#if defined(NX_DISABLE_ICMPV4_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT              compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV4_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
#ifdef TX_ENABLE_EVENT_TRACE
NX_IPV4_HEADER   *ip_header_ptr;
#endif /* TX_ENABLE_EVENT_TRACE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Point to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

#ifdef NX_DISABLE_ICMPV4_RX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV4_RX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_IPSEC_ENABLE
    if ((packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
    {
        compute_checksum = 1;
    }
#endif
#if defined(NX_DISABLE_ICMPV4_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV4_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Calculate the ICMP message checksum.  */
        checksum =  _nx_ip_checksum_compute(packet_ptr, NX_IP_ICMP,
                                            (UINT)packet_ptr -> nx_packet_length,
                                            /* ICMPv4 checksum does not include
                                               src/dest addresses */
                                            NX_NULL, NX_NULL);

        checksum =  ((USHORT) ~checksum) & NX_LOWER_16_MASK;

        /* Determine if the checksum is valid.  */
        if (checksum)
        {

#ifndef NX_DISABLE_ICMP_INFO

            /* Increment the ICMP invalid packet error.  */
            ip_ptr -> nx_ip_icmp_invalid_packets++;

            /* Increment the ICMP checksum error count.  */
            ip_ptr -> nx_ip_icmp_checksum_errors++;
#endif

            /* Nope, the checksum is invalid.  Toss this ICMP packet out.  */
            _nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Determine the message type and call the appropriate handler.  */
    if (header_ptr -> nx_icmpv4_header_type == NX_ICMP_ECHO_REPLY_TYPE)
    {
        _nx_icmpv4_process_echo_reply(ip_ptr, packet_ptr);
    }
    else if (header_ptr -> nx_icmpv4_header_type == NX_ICMP_ECHO_REQUEST_TYPE)
    {
        _nx_icmpv4_process_echo_request(ip_ptr, packet_ptr);
    }
    else
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP unhandled message count.  */
        ip_ptr -> nx_ip_icmp_unhandled_messages++;
#endif

#ifdef TX_ENABLE_EVENT_TRACE

        /* Set the IP header.  */
        ip_header_ptr = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ICMP_RECEIVE, ip_ptr, ip_header_ptr -> nx_ip_header_source_ip, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE  */

        /* Unhandled ICMP message, just release it.  */
        _nx_packet_release(packet_ptr);
    }
}
#endif /* !NX_DISABLE_IPV4  */

