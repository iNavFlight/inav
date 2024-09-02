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
/**   Internet Protocol version 6 (IPv6)                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_packet.h"

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
#include "nx_icmpv6.h"
#endif

#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_FRAGMENTATION)

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_fragment_process                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function breaks the supplied packet into fragments and sends   */
/*    them out through the associated IP driver.  This function uses the  */
/*    already built IP header and driver request structure for each       */
/*    packet fragment.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    driver_req_ptr                        Pointer to driver request     */
/*    mtu                                   Maximum transfer unit         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_checksum_compute        Compute checksum              */
/*    _nx_packet_allocate                   Allocate packet               */
/*    _nx_ipv6_packet_copy                  Copy packet                   */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_send_queued_packets                                      */
/*    _nx_ipv6_packet_send                                                */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    None                                                                */
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
VOID _nx_ipv6_fragment_process(struct NX_IP_DRIVER_STRUCT *driver_req_ptr, UINT mtu)
{

NX_PACKET                      *first_fragment, *source_packet, *previous_packet;
UCHAR                          *fragmentable_ptr, *last_header_location;
UINT                            packet_length, unfragmentable_size;
ULONG                           packet_id;
NX_IP_DRIVER                    driver_request;
NX_IP                          *ip_ptr;
UCHAR                           next_header;
UCHAR                           hdr_ext_len;
UINT                            fragment_offset = 0;
INT                             error = 0;
NX_IPV6_HEADER_FRAGMENT_OPTION *fragment_option;
INT                             last_fragment = 0;
NX_IPV6_HEADER                 *ipv6_header;
ULONG                           word_1;
ULONG                           val;
NX_PACKET_POOL                 *pool_ptr;


    first_fragment = NX_NULL;

    /* Setup the local driver request packet that will be used for each
       fragment.  There will be a unique packet pointer for each request, but
       otherwise all the other fields will remain constant.  */
    driver_request = *driver_req_ptr;

    /* Setup the IP pointer. */
    ip_ptr = driver_req_ptr -> nx_ip_driver_ptr;

#ifndef NX_DISABLE_IP_INFO

    /* Increment the total number of fragment requests.  */
    ip_ptr -> nx_ip_total_fragment_requests++;
#endif

    /* Source_packet points a packet (or a chain of packets) that already has IP header
       constructed.  The prepend pointer should point to the IPv6 header. */
    packet_id = ip_ptr -> nx_ip_packet_id++;

    /* Byte swap packet_id */
    NX_CHANGE_ULONG_ENDIAN(packet_id);

    /* Pickup the source packet pointer.  */
    source_packet = driver_req_ptr -> nx_ip_driver_packet;
    source_packet -> nx_packet_last = source_packet;
    source_packet -> nx_packet_ip_header = source_packet -> nx_packet_prepend_ptr;
    pool_ptr = source_packet -> nx_packet_pool_owner;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, source_packet);

#ifdef NX_ENABLE_INTERFACE_CAPABILITY

    /* Compute checksum for upper layer protocol. */
    if (source_packet -> nx_packet_interface_capability_flag)
    {
        _nx_ip_packet_checksum_compute(source_packet);
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Find out the unfragmentable part. */
    fragmentable_ptr = source_packet -> nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER);
    last_header_location  = (source_packet -> nx_packet_prepend_ptr + 6);
    next_header = *last_header_location;

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

    /* RFC 1981 Section 4 requires that we include a fragment header if we received a
       packet from a host with an MTU less than the minimum required path MTU, regardless
       if we decide to fragment the packet (which we aren't). */
    if (mtu < NX_MINIMUM_IPV6_PATH_MTU)
    {

        /* So reset the mtu to the minimum packet size. */
        mtu = NX_MINIMUM_IPV6_PATH_MTU;
    }

#endif

    /* Fragment Option header appears after Hop-by-hop and routing headers.  So we need
       to skip these headers if they are present. */
    while ((next_header == NX_PROTOCOL_NEXT_HEADER_HOP_BY_HOP) ||
           (next_header == NX_PROTOCOL_NEXT_HEADER_ROUTING))
    {

        /* Move to the next header */
        hdr_ext_len = *(fragmentable_ptr + 1);

        last_header_location = fragmentable_ptr;

        next_header = *fragmentable_ptr;

        /*lint -e{923} suppress cast between pointer and UINT.  */
        fragmentable_ptr = NX_UCHAR_POINTER_ADD(fragmentable_ptr, ((hdr_ext_len + 1) << 3));
    }

    /* If hdr_ext_len == 0, there are no optional headers in the unfragmentable region. */
    *last_header_location = NX_PROTOCOL_NEXT_HEADER_FRAGMENT;

    /* Change the very last "next_header" to
       compute the unfragmentable size which includes MAC header, IPv6 header,
       and any unfragmentable header, but not the fragment header option. */
    /*lint -e{923} suppress cast between pointer and UINT.  */
    unfragmentable_size = (UINT)((ALIGN_TYPE)fragmentable_ptr - (ALIGN_TYPE)source_packet -> nx_packet_prepend_ptr);

    /* Compute the fragmentable size. */
    packet_length = (UINT)(source_packet -> nx_packet_length - unfragmentable_size);

    /* Add the size of the fragment option header.
       This number is going to be used for finding the starting position of
       the fragmentable part. */
    /* unfragmentable_size += sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);*/

    /* Now fragmentable_ptr points to the begining of fragmentable part of 'remaining_pkt' */
    /* Also packet_prepend_ptr points to the data (fragmentable) area.  */

    /* The fragmentable pointer starts from the first packet.*/
    while (packet_length)
    {

    NX_PACKET *new_packet;
    UINT       fragment_size;
    UINT       remaining_bytes;
    UINT       nx_packet_size;

        /*
           Determine the fragment size. Take the MTU size, minus the unfragmentable
           portion, and round down to multiple of 8-bytes, then add the unfragmentable
           portion back.   This is the size of each fragment, excluding the last fragment.
         */
        fragment_size = (mtu - unfragmentable_size) & 0xFFF8;
        fragment_size -= (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);

        if (fragment_size >= packet_length)
        {
            fragment_size = packet_length;
            last_fragment = 1;
        }

        /* Compute the remaining packet length */
        packet_length -= fragment_size;

        /* Figure out the number of bytes (fragmentable part + unfragmentable part)
           of this frame. */
        remaining_bytes = fragment_size + unfragmentable_size + (UINT)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION) + NX_PHYSICAL_HEADER;

        /* find the size of each nx packet. */
        nx_packet_size = (UINT)((pool_ptr -> nx_packet_pool_payload_size) & 0xFFFC);

        if (nx_packet_size > (mtu + NX_PHYSICAL_HEADER))
        {
            nx_packet_size = (mtu + NX_PHYSICAL_HEADER);
        }

        /* Make sure we have enough packets for this fragment. */
        do
        {

            /* Allocate a packet from the default packet pool.  */
            if (_nx_packet_allocate(pool_ptr, &new_packet,
                                    0, TX_NO_WAIT))
            {
                error = 1;
                break;
            }

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, new_packet);

            /*lint -e{644} suppress variable might not be initialized, since "new_packet" was initialized in _nx_packet_allocate. */
            new_packet -> nx_packet_ip_version = NX_IP_VERSION_V6;

            if (first_fragment == NX_NULL)
            {
                first_fragment = new_packet;
                first_fragment -> nx_packet_last = new_packet;

                /* first_fragment -> nx_packet_length = fragment_size + unfragmentable_size. */
                /* May need to configure additional header information. */
            }
            else
            {
                first_fragment -> nx_packet_last -> nx_packet_next = new_packet;
                first_fragment -> nx_packet_last = new_packet;
            }
            /* Establish the "usable" size of the packet.
               The actual copy routine uses this information to figure out how many
               bytes to transer to the fragmented packets.*/

            /* The true packet size is set in the first packet. */
            if (nx_packet_size > remaining_bytes)
            {
                /* This is going to be the last packet we need. */
                new_packet -> nx_packet_length = remaining_bytes;
                remaining_bytes = 0;
            }
            else
            {
                new_packet -> nx_packet_length = nx_packet_size;
                remaining_bytes -= nx_packet_size;
            }
        } while (remaining_bytes);

        if (error)
        {
            break;
        }

        /* We have all the packets ready.  Need to copy data. */

        /* First step:  copy the unfragmentable part. */
        /* Save the state from last iteration. */
        previous_packet = source_packet -> nx_packet_last;

        source_packet -> nx_packet_last = source_packet;
        source_packet -> nx_packet_prepend_ptr = source_packet -> nx_packet_ip_header;

        first_fragment -> nx_packet_last = first_fragment;

        first_fragment -> nx_packet_prepend_ptr += NX_PHYSICAL_HEADER;
        first_fragment -> nx_packet_append_ptr += NX_PHYSICAL_HEADER;

        /* For the first packet, the prepend pointer is already at the begining of the IP header. */
        if (_nx_ipv6_packet_copy(source_packet, first_fragment, unfragmentable_size))
        {
            break;
        }

        /* Fill in the fragment header area.  Be careful here: we assume the unfragmentable part does not
           span over multiple packets. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)first_fragment -> nx_packet_last -> nx_packet_append_ptr;
        first_fragment -> nx_packet_append_ptr += sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
        first_fragment -> nx_packet_length += (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);

        fragment_option -> nx_ipv6_header_fragment_option_reserved = 0;
        fragment_option -> nx_ipv6_header_fragment_option_next_header = next_header;

        if (!last_fragment)
        {
            fragment_option -> nx_ipv6_header_fragment_option_offset_flag = (USHORT)(fragment_offset + 1);
        }
        else
        {
            fragment_option -> nx_ipv6_header_fragment_option_offset_flag = (USHORT)fragment_offset;
        }

        /* Convert to network byte order. */
        NX_CHANGE_USHORT_ENDIAN(fragment_option -> nx_ipv6_header_fragment_option_offset_flag);

        fragment_option -> nx_ipv6_header_fragment_option_packet_id = packet_id;

        /* Restore the nx_packet_last and the prepend pointer within the last packet. */
        source_packet -> nx_packet_last = previous_packet;
        source_packet -> nx_packet_last -> nx_packet_prepend_ptr = fragmentable_ptr;

        /* Copy the rest of the frame. */
        if (_nx_ipv6_packet_copy(source_packet, first_fragment, fragment_size))
        {
            break;
        }

        /*
           Set up the IP frame length.  first_fragment -> nx_packet_prepend_ptr points to the
           beginning of the IPv6 header.
         */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header = (NX_IPV6_HEADER *)first_fragment -> nx_packet_prepend_ptr;

        /* Pick up the 2nd word in the IP header. */
        val = ipv6_header -> nx_ip_header_word_1;

        /* Convert to host byte order. */
        NX_CHANGE_ULONG_ENDIAN(val);

        val = val & 0x0000FFFF;

        word_1 = (ULONG)(((fragment_size + unfragmentable_size - sizeof(NX_IPV6_HEADER)) + sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION)) << 16);
        word_1 = val | word_1;

        /* Convert to network byte order. */
        NX_CHANGE_ULONG_ENDIAN(word_1);

        ipv6_header -> nx_ip_header_word_1 = word_1;

        fragmentable_ptr = source_packet -> nx_packet_last -> nx_packet_prepend_ptr;

        fragment_offset += fragment_size;

        /* This fragment is ready to be transmitted. */
        /* Send the packet to the associated driver for output.  */
        first_fragment -> nx_packet_length = unfragmentable_size + fragment_size;
        first_fragment -> nx_packet_length += (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
        driver_request.nx_ip_driver_packet =   first_fragment;

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP fragments sent count.  */
        ip_ptr -> nx_ip_total_fragments_sent++;

        /* Increment the IP packet sent count.  */
        ip_ptr -> nx_ip_total_packets_sent++;

        /* Increment the IP bytes sent count.  */
        ip_ptr -> nx_ip_total_bytes_sent += first_fragment -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
#endif /* !NX_DISABLE_IP_INFO */

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, source_packet);

        (source_packet -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_link_driver_entry)(&driver_request);

        first_fragment = NX_NULL;
    }

    /* Release the original packet. */
    _nx_packet_transmit_release(source_packet);

    /* In case the fragmentation process fails above, frist_fragment
       still contains partial data. Free these fragments before
       exiting this function. */
    if (first_fragment)
    {
        _nx_packet_release(first_fragment);
    }

    return;
}

#endif /* FEATURE_NX_IPV6 && NX_DISABLE_FRAGMENTATION*/

