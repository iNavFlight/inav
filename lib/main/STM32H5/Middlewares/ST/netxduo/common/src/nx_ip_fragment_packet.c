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
#include "nx_packet.h"


#ifndef NX_DISABLE_FRAGMENTATION
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fragment_packet                              PORTABLE C      */
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
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_checksum_compute        Compute checksum              */
/*    _nx_packet_allocate                   Allocate packet for fragment  */
/*    _nx_packet_transmit_release           Transmit packet release       */
/*    (ip_link_driver)                      User supplied link driver     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_arp_packet_receive                Received ARP packet processing*/
/*    _nx_ip_packet_send                    Send an IP packet             */
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
VOID  _nx_ip_fragment_packet(struct NX_IP_DRIVER_STRUCT *driver_req_ptr)
{

#ifndef NX_DISABLE_IPV4
UINT            status;
ULONG           checksum;
ULONG           temp;
UCHAR          *source_ptr;
ULONG           remaining_bytes;
ULONG           fragment_size;
ULONG           copy_size;
ULONG           copy_remaining_size;
ULONG           fragment_offset = 0;
NX_IP_DRIVER    driver_request;
NX_PACKET      *source_packet;
NX_PACKET      *fragment_packet;
NX_IPV4_HEADER *source_header_ptr;
NX_IPV4_HEADER *fragment_header_ptr;
NX_IP          *ip_ptr;
#if defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */

#ifdef NX_DISABLE_IP_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_IP_TX_CHECKSUM */

    /* Setup the local driver request packet that will be used for each
       fragment.  There will be a unique packet pointer for each request, but
       otherwise all the other fields will remain constant.  */
    driver_request =  *driver_req_ptr;

    /* Setup the IP pointer.  */
    ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;

#ifndef NX_DISABLE_IP_INFO

    /* Increment the total number of fragment requests.  */
    ip_ptr -> nx_ip_total_fragment_requests++;
#endif

    /* Pickup the source packet pointer.  */
    source_packet =  driver_req_ptr -> nx_ip_driver_packet;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, source_packet);

#ifdef NX_ENABLE_INTERFACE_CAPABILITY

    /* Compute checksum for upper layer protocol. */
    if (source_packet -> nx_packet_interface_capability_flag)
    {
        _nx_ip_packet_checksum_compute(source_packet);
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Build a pointer to the source IP header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    source_header_ptr =  (NX_IPV4_HEADER *)source_packet -> nx_packet_prepend_ptr;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the IP header.  */
    NX_CHANGE_ULONG_ENDIAN(source_header_ptr -> nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(source_header_ptr -> nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(source_header_ptr -> nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(source_header_ptr -> nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(source_header_ptr -> nx_ip_header_destination_ip);

    /* Pickup the length of the packet and the starting pointer.  */
    remaining_bytes =  (source_packet -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER));
    source_ptr =  source_packet -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER);

    /* Derive the fragment size.  */
    fragment_size =  source_packet -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_mtu_size - (ULONG)sizeof(NX_IPV4_HEADER);
    fragment_size =  (fragment_size / NX_IP_ALIGN_FRAGS) * NX_IP_ALIGN_FRAGS;

    /* Loop to break the source packet into fragments and send each out through
       the associated driver.  */
    while (remaining_bytes)
    {
        /* Allocate a packet from the default packet pool.  */
        status =  _nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &fragment_packet,
                                      NX_IPv4_PACKET, TX_NO_WAIT);

        /* Determine if there is a packet available.  */
        if (status)
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the fragment failure count.  */
            ip_ptr -> nx_ip_fragment_failures++;

            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;

            /* Increment the IP transmit resource error count.  */
            ip_ptr -> nx_ip_transmit_resource_errors++;
#endif

            /* Error, not enough packets to perform the fragmentation...  release the
               source packet and return.  */
            _nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
            return;
        }

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, fragment_packet);

        /* Set the proper interface. */
        /*lint -e{644} suppress variable might not be initialized, since "fragment_packet" was initialized in _nx_packet_allocate. */
        fragment_packet -> nx_packet_address.nx_packet_interface_ptr = driver_req_ptr -> nx_ip_driver_packet -> nx_packet_address.nx_packet_interface_ptr;

        /* Calculate the size of this fragment.  */
        if (remaining_bytes > fragment_size)
        {
            copy_remaining_size =  fragment_size;
            remaining_bytes -= fragment_size;
        }
        else
        {
            copy_remaining_size = remaining_bytes;
            remaining_bytes = 0;
        }

        /* Copy data.  */
        while (copy_remaining_size)
        {

            /* We need to copy the remaining bytes into the new packet and then move to the next
               packet.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            if (copy_remaining_size > (ULONG)(source_packet -> nx_packet_append_ptr - source_ptr))
            {
                copy_size = (ULONG)(source_packet -> nx_packet_append_ptr - source_ptr);
            }
            else
            {
                copy_size = copy_remaining_size;
            }

            status = _nx_packet_data_append(fragment_packet, source_ptr, copy_size, ip_ptr -> nx_ip_default_packet_pool, NX_NO_WAIT);

            /* Determine if there is a packet available.  */
            if (status)
            {

#ifndef NX_DISABLE_IP_INFO

                /* Increment the fragment failure count.  */
                ip_ptr -> nx_ip_fragment_failures++;

                /* Increment the IP send packets dropped count.  */
                ip_ptr -> nx_ip_send_packets_dropped++;

                /* Increment the IP transmit resource error count.  */
                ip_ptr -> nx_ip_transmit_resource_errors++;
#endif

                /* Error, not enough packets to perform the fragmentation...  release the
                   source packet and return.  */
                _nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
                _nx_packet_release(fragment_packet);
                return;
            }

            /* Reduce the remaining size. */
            copy_remaining_size -= copy_size;

            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            if (copy_size == (ULONG)(source_packet -> nx_packet_append_ptr - source_ptr))
            {

                /* Move to the next physical packet in the source message.  */
                /* Determine if there is a next packet.  */
                if (source_packet -> nx_packet_next)
                {

                    /* Move to the next physical packet in the source message.  */
                    source_packet =  source_packet -> nx_packet_next;

                    /* Setup new source pointer.  */
                    source_ptr =  source_packet -> nx_packet_prepend_ptr;
                }
                else if (remaining_bytes)
                {

                    /* Error, no next packet but current packet is exhausted and there are
                       remaining bytes.  */

#ifndef NX_DISABLE_IP_INFO

                    /* Increment the invalid transmit packet count.  */
                    ip_ptr -> nx_ip_invalid_transmit_packets++;

                    /* Increment the fragment failures count.  */
                    ip_ptr -> nx_ip_fragment_failures++;
#endif

                    /* Error, not enough packets to perform the fragmentation...  release the
                       source packet and return.  */
                    _nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
                    _nx_packet_release(fragment_packet);
                    return;
                }
            }
            else
            {

                /* Copy finished. */
                source_ptr += copy_size;
            }
        }

        /* Setup the fragment packet pointers.  */
        fragment_packet -> nx_packet_prepend_ptr = fragment_packet -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER);
        fragment_packet -> nx_packet_length += (ULONG)sizeof(NX_IPV4_HEADER);

        /* Setup the fragment's IP header.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        fragment_header_ptr =  (NX_IPV4_HEADER *)fragment_packet -> nx_packet_prepend_ptr;

        /* Setup the new IP header.  */
        fragment_header_ptr -> nx_ip_header_word_0 =          (source_header_ptr -> nx_ip_header_word_0 & ~NX_LOWER_16_MASK) |
            fragment_packet -> nx_packet_length;
        fragment_header_ptr -> nx_ip_header_word_1 =          source_header_ptr -> nx_ip_header_word_1 | (fragment_offset / 8);
        fragment_header_ptr -> nx_ip_header_word_2 =          source_header_ptr -> nx_ip_header_word_2 & ~NX_LOWER_16_MASK;
        fragment_header_ptr -> nx_ip_header_source_ip =       source_header_ptr -> nx_ip_header_source_ip;
        fragment_header_ptr -> nx_ip_header_destination_ip =  source_header_ptr -> nx_ip_header_destination_ip;

        /* Determine if this is the last fragment.  */
        if (remaining_bytes)
        {

            /* Not the last fragment, so set the more fragments bit.  */
            fragment_header_ptr -> nx_ip_header_word_1 =  fragment_header_ptr -> nx_ip_header_word_1 | NX_IP_MORE_FRAGMENT;
        }

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        if (fragment_packet -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
        {
            compute_checksum = 0;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#if defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
        if (compute_checksum)
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
        {

            /* Build the IP checksum for this fragment.  */
            temp =       fragment_header_ptr -> nx_ip_header_word_0;
            checksum =   (temp >> NX_SHIFT_BY_16) + (temp & NX_LOWER_16_MASK);
            temp =       fragment_header_ptr -> nx_ip_header_word_1;
            checksum +=  (temp >> NX_SHIFT_BY_16) + (temp & NX_LOWER_16_MASK);
            temp =       fragment_header_ptr -> nx_ip_header_word_2;
            checksum +=  (temp >> NX_SHIFT_BY_16);
            temp =       fragment_header_ptr -> nx_ip_header_source_ip;
            checksum +=  (temp >> NX_SHIFT_BY_16) + (temp & NX_LOWER_16_MASK);
            temp =       fragment_header_ptr -> nx_ip_header_destination_ip;
            checksum +=  (temp >> NX_SHIFT_BY_16) + (temp & NX_LOWER_16_MASK);

            /* Add in the carry bits into the checksum.  */
            checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

            /* Do it again in case previous operation generates an overflow.  */
            checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

            /* Now store the checksum in the IP fragment header.  */
            fragment_header_ptr -> nx_ip_header_word_2 =  fragment_header_ptr -> nx_ip_header_word_2 | (NX_LOWER_16_MASK & (~checksum));
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {
            fragment_packet -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

        /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
           swap the endian of the IP header.  */
        NX_CHANGE_ULONG_ENDIAN(fragment_header_ptr -> nx_ip_header_word_0);
        NX_CHANGE_ULONG_ENDIAN(fragment_header_ptr -> nx_ip_header_word_1);
        NX_CHANGE_ULONG_ENDIAN(fragment_header_ptr -> nx_ip_header_word_2);
        NX_CHANGE_ULONG_ENDIAN(fragment_header_ptr -> nx_ip_header_source_ip);
        NX_CHANGE_ULONG_ENDIAN(fragment_header_ptr -> nx_ip_header_destination_ip);

#ifndef NX_DISABLE_IP_INFO
        /* Increment the IP fragments sent count.  */
        ip_ptr -> nx_ip_total_fragments_sent++;

        /* Increment the IP packet sent count.  */
        ip_ptr -> nx_ip_total_packets_sent++;

        /* Increment the IP bytes sent count.  */
        ip_ptr -> nx_ip_total_bytes_sent += fragment_packet -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);
#endif

        /* Send the packet to the associated driver for output.  */
        driver_request.nx_ip_driver_packet =   fragment_packet;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_PACKET_SEND, ip_ptr, fragment_packet, fragment_packet -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, fragment_packet);

        (fragment_packet -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_link_driver_entry)(&driver_request);

        /* Increase offset. */
        fragment_offset += fragment_size;
    }

#ifndef NX_DISABLE_IP_INFO

    /* Increment the total number of successful fragment requests.  */
    ip_ptr -> nx_ip_successful_fragment_requests++;
#endif

    /* The original packet has been sent out in fragments... release it!  */
    _nx_packet_transmit_release(driver_req_ptr -> nx_ip_driver_packet);
#else
    NX_PARAMETER_NOT_USED(driver_req_ptr);
#endif /* NX_DISABLE_IPV4 */
}
#endif /* NX_DISABLE_FRAGMENTATION */

