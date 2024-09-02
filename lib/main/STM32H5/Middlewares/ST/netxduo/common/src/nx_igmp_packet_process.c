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
/**   Internet Group Management Protocol (IGMP)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_igmp.h"


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_igmp_packet_process                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function handles reception of IGMP packets.  There are two types*/
/*   of IGMP packets.  Routers will send IGMP query messages and other    */
/*   send responses intended for the router but will be seen by all other */
/*   hosts belonging to the same multicast group.  In the latter case the */
/*   other hosts will cancel their own response for the same multicast    */
/*   group.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    packet_ptr                            IGMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release IGMP packet           */
/*    tx_time_get                           Get current time              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_igmp_packet_receive               IGMP packet receive           */
/*    _nx_igmp_queue_process                IGMP queue processing         */
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
VOID  _nx_igmp_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UINT            i;
ULONG           update_time;
NX_IGMP_HEADER *header_ptr;
USHORT          max_update_time;
ULONG           checksum;
ULONG           length;
UCHAR          *word_ptr;
NX_PACKET      *current_packet;
ULONG           long_temp;
USHORT          short_temp;


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Setup a pointer to the IGMP packet header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_IGMP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (!(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IGMP_RX_CHECKSUM))
#endif /* FEATURE_LINK_CAPABILITY */
    {


        /* First verify the checksum is correct. */

        /* Setup the length of the packet checksum.  */
        length =  packet_ptr -> nx_packet_length;

        /* Determine if we need to add a padding byte.  */
        if (((length / sizeof(USHORT)) * sizeof(USHORT)) != length)
        {

            /* We have single byte alignment and we need two byte alignment.  */
            length++;

#ifndef NX_DISABLE_PACKET_CHAIN
            /* Determine if there is a last packet pointer.  */
            if (packet_ptr -> nx_packet_last)
            {

                /* Multi-packet message, add a zero byte at the end.  */
                *((packet_ptr -> nx_packet_last) -> nx_packet_append_ptr) =  0;
            }
            else
            {
#endif

                /* Write a zero byte at the end of the first and only packet.  */
                *(packet_ptr -> nx_packet_append_ptr) =  0;
#ifndef NX_DISABLE_PACKET_CHAIN
            }
#endif
        }

        /* Setup the pointer to the start of the packet.  */
        word_ptr =  (UCHAR *)packet_ptr -> nx_packet_prepend_ptr;

        /* Initialize the current packet to the input packet pointer.  */
        current_packet =  packet_ptr;

        checksum = 0;


        /* Loop to calculate the checksum over the entire packet.  */
        while (length)
        {
            /* Determine if there is at least one ULONG left.  */
            if ((UINT)(current_packet -> nx_packet_append_ptr - word_ptr) >= sizeof(ULONG))
            {

                /* Pickup a whole ULONG.  */
                long_temp =  *((ULONG *)word_ptr);

                /* Add upper 16-bits into checksum.  */
                checksum =  checksum + (long_temp >> NX_SHIFT_BY_16);

                /* Check for carry bits.  */
                if (checksum & NX_CARRY_BIT)
                {
                    checksum =  (checksum & NX_LOWER_16_MASK) + 1;
                }

                /* Add lower 16-bits into checksum.  */
                checksum =  checksum + (long_temp & NX_LOWER_16_MASK);

                /* Check for carry bits.  */

                if (checksum & NX_CARRY_BIT)
                {
                    checksum =  (checksum & NX_LOWER_16_MASK) + 1;
                }

                /* Move the word pointer and decrease the length.  */
                word_ptr =  word_ptr + sizeof(ULONG);
                length = length - (ULONG)sizeof(ULONG);
            }
            else
            {

                /* Pickup the 16-bit word.  */
                short_temp =  *((USHORT *)word_ptr);

                /* Add next 16-bit word into checksum.  */
                checksum =  checksum + short_temp;

                /* Check for carry bits.  */
                if (checksum & NX_CARRY_BIT)
                {
                    checksum =  (checksum & NX_LOWER_16_MASK) + 1;
                }

                /* Move the word pointer and decrease the length.  */
                word_ptr =  word_ptr + sizeof(USHORT);
                length = length - (ULONG)sizeof(USHORT);
            }

#ifndef NX_DISABLE_PACKET_CHAIN
            /* Determine if we are at the end of the current packet.  */
            if ((word_ptr >= (UCHAR *)current_packet -> nx_packet_append_ptr) &&
                (current_packet -> nx_packet_next))
            {

                /* We have crossed the packet boundary.  Move to the next packet
                   structure.  */
                current_packet =  current_packet -> nx_packet_next;

                /* Setup the new word pointer.  */
                word_ptr =  (UCHAR *)current_packet -> nx_packet_prepend_ptr;
            }
#endif
        }

        checksum = ~checksum & NX_LOWER_16_MASK;

        /* Determine if the checksum is valid.  */
        if (checksum)
        {

            /* It is not. By RFC requirements we should not accept this packet. */

#ifndef NX_DISABLE_IGMP_INFO
            /* Increment the IGMP invalid packet error.  */
            ip_ptr -> nx_ip_igmp_invalid_packets++;

            /* Increment the IGMP checksum error count.  */
            ip_ptr -> nx_ip_igmp_checksum_errors++;
#endif /* NX_DISABLE_IGMP_INFO */

            /* Toss this IGMP packet out.  */
            _nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Swap the IGMP headers to host byte order. */
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_igmp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_igmp_header_word_1);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IGMP_RECEIVE, ip_ptr, *(((ULONG *)packet_ptr -> nx_packet_prepend_ptr) - 2), packet_ptr, header_ptr -> nx_igmp_header_word_0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Determine the type of IGMP message received.  Note that an IGMPv1 host will respond
       to an IGMPv2 general query but not process the maximum response time field. */
    if ((header_ptr -> nx_igmp_header_word_0 & NX_IGMP_TYPE_MASK) == NX_IGMP_ROUTER_QUERY_TYPE)
    {


#ifndef NX_DISABLE_IGMP_INFO
        /* Increment the IGMP queries received count.  */
        ip_ptr -> nx_ip_igmp_queries_received++;
#endif

        /* Set the max response time recommended by RFC 1112 set by host in seconds. In a
           IGMPv2 network, the router may set a different max time in its IGMP membership queries. */
        max_update_time  = NX_IGMP_MAX_UPDATE_TIME;

#ifndef NX_DISABLE_IGMPV2

        /* Determine the IGMP version the sender (router) is using.  */

        /* Is the max response time non zero? */


        if ((header_ptr -> nx_igmp_header_word_0 & NX_IGMP_MAX_RESP_TIME_MASK) != NX_NULL)
        {
            /* Yes, this must be an IGMPv2 router. */
            ip_ptr -> nx_ip_igmp_router_version = NX_IGMP_HOST_VERSION_2;

            /* Parse the max response time from the IGMP header. */
            max_update_time  = (USHORT)((header_ptr -> nx_igmp_header_word_0 & NX_IGMP_MAX_RESP_TIME_MASK) >> 16) & 0x0000FF;

            /* Convert from tenths of a second to seconds. */
            max_update_time /= 10;
        }
        else
        {
            /* No; IGMPv1 requires setting this field to zero. */
            ip_ptr -> nx_ip_igmp_router_version = NX_IGMP_HOST_VERSION_1;
        }

#endif

        /* Then generate a random update time initially in timer ticks for the delay. */
        update_time =  tx_time_get() & 0xF;

        /* Check we have a valid non zero update time that does not exceed the
           maximum response time. */
        if ((update_time > max_update_time) || (update_time == 0))
        {

            /* If not, wrap the update time back to one second. */
            update_time =  1;
        }

        /* Loop through the multicast join list and assign an arbitrary timeout to
           respond between 1 and maximum response time for each group.  */
        for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
        {

            /* Is there a group address in this slot?  */
            if (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == NX_NULL)
            {

                /* No, skip doing further processing. */
                continue;
            }

            /* Does the group address in the header match our join list? */
            if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list != header_ptr -> nx_igmp_header_word_1) &&
                /* or is this a general membership query? */
                (header_ptr -> nx_igmp_header_word_1 != NX_NULL))
            {

                /* No; so no need to update the timer, skip to the next group in the host list. */
                continue;
            }

            /* Is the current host group running timer less than the max delay? */
            if (((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time < max_update_time) &&
                 (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time != NX_NULL)) ||
                (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time == NX_WAIT_FOREVER))
            {

                /* Yes; Let the current timer timeout. Skip to the next group. */
                continue;
            }

            /* Set the timeout for this multicast group. */
            ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time = update_time;

            /* Then increment the update time for the next host group so the update/expiration times
               are separated by one second. This avoids bursts of IGMP reports to the server. */
            update_time++;

            /* Check after each multicast group that we have not exceeded the maximum response time. */
            if (update_time > max_update_time)
            {

                /* We have, so wrap the update time back to one. */
                update_time =  1;
            }
        }
    }
#ifndef NX_DISABLE_IGMPV2

    /* Is this another IGMPv1 host's join request? */
    else if (((header_ptr -> nx_igmp_header_word_0 & NX_IGMP_TYPE_MASK) == NX_IGMP_HOST_RESPONSE_TYPE) ||
             /* ...Or an IGMPv2 host's join request? */
             ((header_ptr -> nx_igmp_header_word_0 & NX_IGMPV2_TYPE_MASK) == NX_IGMP_HOST_V2_JOIN_TYPE))
#else

    /* Is this another IGMPv1 host's join request? */
    else if ((header_ptr -> nx_igmp_header_word_0 & NX_IGMP_TYPE_MASK) == NX_IGMP_HOST_RESPONSE_TYPE)
#endif
    {

        /* Yes;  Loop through the host multicast join list to find a matching group.  */
        for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
        {

            /* Compare the group address in the header with the host list. Is this a match? */
            if ((ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_join_list == header_ptr -> nx_igmp_header_word_1) &&
                (ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time != NX_WAIT_FOREVER))
            {

                /* Yes; Clear the update time. This will cancel sending a join
                   request for the same multicast group.  */
                ip_ptr -> nx_ipv4_multicast_entry[i].nx_ipv4_multicast_update_time =  0;
                break;
            }
        }
    }

    /* Release the IGMP packet.  */
    _nx_packet_release(packet_ptr);
}
#endif /* !NX_DISABLE_IPV4  */

