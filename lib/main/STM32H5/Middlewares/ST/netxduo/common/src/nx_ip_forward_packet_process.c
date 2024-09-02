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

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_forward_packet_process                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to forward the IP packet to the destination  */
/*    IP by using the NetX send packet routine.  Note that the IP header  */
/*    is still intact prior to the packet.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to forward  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Packet release                */
/*    _nx_packet_data_adjust                Adjust the packet data to fill*/
/*                                            the specified header        */
/*    _nx_ip_driver_packet_send             Send the IP packet            */
/*    _nx_ip_fragment_forward_packet        Fragment the forward packet   */
/*    _nx_ip_packet_deferred_receive        IP deferred receive packet    */
/*                                            processing                  */
/*    _nx_ip_route_find                     Find suitable outgoing        */
/*                                            interface                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_packet_receive                 Receive IP packet             */
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
VOID  _nx_ip_forward_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_IPV4_HEADER *ip_header_ptr;
NX_INTERFACE   *outgoing_interface = NX_NULL;
ULONG           next_hop_address = 0;
ULONG           destination_ip;
ULONG           time_to_live;
ULONG           fragment_bit;
ULONG           status;
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
UINT            compute_checksum = 1;
#endif
ULONG           checksum;
ULONG           old_m;
ULONG           new_m;


    /* The NetX IP forwarding consists of simply sending the same packet out through
       the internal send routine.  Applications may choose to modify this code or
       replace the nx_ip_forward_packet_process pointer in the IP structure to point
       at an application-specific routine for forwarding.  */

    /* It's assumed that the IP header is still present in front of the packet.  Position
       backwards to access it.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr =  (NX_IPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Check for if the destination address is Class D address.  */
    if ((ip_header_ptr -> nx_ip_header_destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE)
    {

        /* Discard the packet.  */
        _nx_packet_release(packet_ptr);
        return;
    }

    /* Determine if source address or destination addrss is link-local address(169.254/16 Hexadecimal:0xA9FE0000).  */
    if (((ip_header_ptr -> nx_ip_header_source_ip & 0xFFFF0000) == 0xA9FE0000) ||
        ((ip_header_ptr -> nx_ip_header_destination_ip & 0xFFFF0000) == 0xA9FE0000))
    {

        /* Discard the packet.  */
        _nx_packet_release(packet_ptr);
        return;
    }

    /* Check whether find the correct forwarding interface.  */
    if (_nx_ip_route_find(ip_ptr, ip_header_ptr -> nx_ip_header_destination_ip, &outgoing_interface, &next_hop_address) == NX_SUCCESS)
    {

        /* If the forwarding interface is same as the receiving interface,
           According to the RFC 792 Redirect Message on page 12,
           send a redirect message to source address. */

        /* Update the forwarding interface. */
        /*lint -e{644} suppress variable might not be initialized, since "outgoing_interface" was initialized as long as return value is NX_SUCCESS. */
        packet_ptr -> nx_packet_ip_interface = outgoing_interface;

        /* Get the relevant information from original packet.  */
        destination_ip = ip_header_ptr -> nx_ip_header_destination_ip;
        time_to_live = ((ip_header_ptr -> nx_ip_header_word_2 & NX_IP_TIME_TO_LIVE_MASK) >> NX_IP_TIME_TO_LIVE_SHIFT) - 1;
        fragment_bit = (ip_header_ptr -> nx_ip_header_word_1 & NX_DONT_FRAGMENT);

        /* If the TTL is 0, discard the packet.  */
        if (!time_to_live)
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* No correct forwarding interface, toss the packet!  */
            _nx_packet_release(packet_ptr);
            return;
        }

        /* Update the TTL value.  */
        ip_header_ptr -> nx_ip_header_word_2 = (ip_header_ptr -> nx_ip_header_word_2 - 0x01000000);

        /* Update the checksum, according to the RFC1624 page2, Eqn.3.
           HC  - old checksum in header
           HC' - new checksum in header
           m   - old value of a 16-bit field
           m'  - new value of a 16-bit field
           HC' = ~(C + (-m) + m')
            = ~(~HC + ~m + m') */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        if (packet_ptr -> nx_packet_ip_interface -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
        {
            compute_checksum = 0;
        }
#endif


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        /* Check whether the destination IP address matched with one interface of IP instance.  */
        /*lint -e{613} suppress possible use of null pointer, since "outgoing_interface" was set in _nx_ip_route_find. */
        if (outgoing_interface -> nx_interface_ip_address == destination_ip)
        {


            /* Clear the capability flag.  */
            packet_ptr -> nx_packet_interface_capability_flag &= (ULONG)(~NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM);


            /* Set the computer checksum flag.  */
            compute_checksum = 1;
        }
#endif

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        /* Computer the checksum.  */
        /*lint -e{774} suppress boolean always evaluates to True, since it is necessary with NX_ENABLE_INTERFACE_CAPABILITY macro. */
        if (compute_checksum)
        {
#endif
            /* Get the old checksum (HC) in header. */
            checksum = ip_header_ptr -> nx_ip_header_word_2 & NX_LOWER_16_MASK;

            /* Get the new TTL(m') in the header. */
            new_m = (ip_header_ptr -> nx_ip_header_word_2 & 0xFFFF0000) >> 16;

            /* Get the old TTL(m). */
            old_m = new_m + 0x0100;

            /* Update the checksum, get the new checksum(HC'),
               The new_m is ULONG value, so need get the lower value after invert. */
            checksum = ((~checksum) & 0xFFFF) + ((~old_m) & 0xFFFF) + new_m;

            /* Fold a 4-byte value into a two byte value */
            checksum = (checksum >> 16) + (checksum & 0xFFFF);

            /* Do it again in case previous operation generates an overflow */
            checksum = (checksum >> 16) + (checksum & 0xFFFF);

            /* Now store the new checksum in the IP header.  */
            ip_header_ptr -> nx_ip_header_word_2 =  ((ip_header_ptr -> nx_ip_header_word_2 & 0xFFFF0000) | ((~checksum) & NX_LOWER_16_MASK));
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        }
        else
        {
            /* Set the checksum to 0.  */
            ip_header_ptr -> nx_ip_header_word_2 =  (ip_header_ptr -> nx_ip_header_word_2 & 0xFFFF0000);
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

        /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
           swap the endian of the IP header.  */
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

        /* Check whether the destination IP address matched with one interface of IP instance.  */
        /*lint -e{613} suppress possible use of null pointer, since "outgoing_interface" was set in _nx_ip_route_find. */
        if (outgoing_interface -> nx_interface_ip_address == destination_ip)
        {

            /* Forward this packet to correct interface.  */
            _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
            return;
        }
        else
        {

            /* Check if the packet can fill physical header.  */
            status = _nx_packet_data_adjust(packet_ptr, NX_PHYSICAL_HEADER);

            /* Check status.  */
            if (status)
            {

                /* Release the packet. */
                _nx_packet_release(packet_ptr);
                return;
            }

            /* Determine if fragmentation is needed.  */
            if (packet_ptr -> nx_packet_length <= packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_mtu_size)
            {

                /* Call the function to directly forward the packet.  */
                /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
                _nx_ip_driver_packet_send(ip_ptr, packet_ptr, destination_ip, fragment_bit, next_hop_address);
                return;
            }
#ifndef NX_DISABLE_FRAGMENTATION
            else
            {
                /* Check the DF bit flag.  */
                if ((ip_ptr -> nx_ip_fragment_processing) && (!(fragment_bit & NX_DONT_FRAGMENT)))
                {

                    /* Fragment and send the packet.  */
                    _nx_ip_fragment_forward_packet(ip_ptr, packet_ptr, destination_ip, NX_FRAGMENT_OKAY, next_hop_address);
                    return;
                }
            }
#endif
        }
    }

    /* No correct forwarding interface, toss the packet!  */
    _nx_packet_release(packet_ptr);
    return;
}

#endif /* NX_DISABLE_IPV4 */

