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
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_packet.h"
#include "nx_icmpv6.h"

#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_FRAGMENTATION)



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_process_fragment_option                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the IPv6 Fragmentation Option header.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to process  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion         */
/*    NX_CONTINUE                           Continue processing           */
/*    NX_OPTION_HEADER_ERROR                Error with fragment option    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_dispatch_process               Process IPv6 optional headers */
/*    NX_ICMPV6_SEND_PARAMETER_PROBLEM      Report IPv6 errors via ICMP   */
/*                                            message                     */
/*    tx_event_flags_set                    Wake up the IP helper thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_dispatch_process               Process IPv6 optional header  */
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
UINT _nx_ipv6_process_fragment_option(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA
NX_IPV6_HEADER_FRAGMENT_OPTION *fragment_option;



    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_IP_INFO

    /* Increment the IP receive fragments count */
    ip_ptr -> nx_ip_total_fragments_received++;

#endif /* NX_DISABLE_IP_INFO */

    /* If fragmentation is not enabled, we drop this packet. */
    if (!ip_ptr -> nx_ip_fragment_assembly)
    {
        return(NX_OPTION_HEADER_ERROR);
    }

    /* Check packet length is at least sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION). */
    if (packet_ptr -> nx_packet_length < sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION))
    {
        return(NX_OPTION_HEADER_ERROR);
    }

    /* Set a pointer to the starting of the fragment option. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)packet_ptr -> nx_packet_prepend_ptr;

    /* Byte swap the offset_flag.  The identification field is only used for checking matches.
       The absolute value of the Id is not used in arithmatic operations.  Therefore there is
       need to byte-swap this field. */
    NX_CHANGE_USHORT_ENDIAN(fragment_option -> nx_ipv6_header_fragment_option_offset_flag);

    /* Check whether or not the payload size is not multiple of 8 bytes if the
       M bit is set. */
    if (fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0x0001) /* M bit is set */
    {

    NX_IPV6_HEADER *ip_header;
    ULONG           payload_length;

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ip_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        payload_length = ip_header -> nx_ip_header_word_1 >> 16;

        /* If not multiple of 8 bytes... */
        if ((payload_length & 0xFFF8) != payload_length)
        {

            /* Return the option header error status and abort. */

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE

            /* Cover offset flag field. */
            NX_CHANGE_USHORT_ENDIAN(fragment_option -> nx_ipv6_header_fragment_option_offset_flag);

            /*lint -e{835} -e{845} suppress operating on zero. */
            NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 0, 4);
#endif
            return(NX_OPTION_HEADER_ERROR);
        }
    }
    /* M bit is clear: This is the last (tail) packet fragment. */
    else if ((fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8) == 0)
    {

        /* Continue processing. */
        return(NX_CONTINUE);
    }

    /* Payload size cannot exceeding 65535. */
    if (((fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8) + packet_ptr -> nx_packet_length -
         sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION)) > 65535)
    {

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE

        /* Cover offset flag field. */
        NX_CHANGE_USHORT_ENDIAN(fragment_option -> nx_ipv6_header_fragment_option_offset_flag);

        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        /*lint -e{835} -e{845} suppress operating on zero. */
        NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 0,
                                         (ULONG)((packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_ip_header) + 2));
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */

        /* Return an Option header error status. */
        return(NX_OPTION_HEADER_ERROR);
    }

    /* Disable interrupt */
    TX_DISABLE

    /* In IPv6 IP fragmentation is required. */

    /* Determine if the queue is empty.  */
    if (ip_ptr -> nx_ip_received_fragment_head)
    {

        /* Raw receive queue is not empty, add this packet to the end of the queue.  */
        (ip_ptr -> nx_ip_received_fragment_tail) -> nx_packet_queue_next =  packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
    }
    else
    {

        /* Raw receive queue is empty.  Just set the head and tail pointers
           to point to this packet.  */
        ip_ptr -> nx_ip_received_fragment_head =  packet_ptr;
        ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
        packet_ptr -> nx_packet_queue_next     =  NX_NULL;
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(NX_PACKET_IP_FRAGMENT_QUEUE, __LINE__, packet_ptr);

    /* Restore interrupts.  */
    TX_RESTORE

#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
    /* Wakeup IP helper thread to process the IP fragment re-assembly.  */
    tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_UNFRAG_EVENT, TX_OR);
#else
    /* Process the IP fragment reassemble.  */
    (ip_ptr -> nx_ip_fragment_assembly)(ip_ptr);
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 && NX_DISABLE_FRAGMENTATION*/

