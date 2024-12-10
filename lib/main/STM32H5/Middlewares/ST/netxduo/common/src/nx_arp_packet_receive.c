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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"
#include "nx_packet.h"


#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_packet_receive                              PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the reception of both the ARP request and   */
/*    the ARP response.  ARP requests are filled in and sent out as ARP   */
/*    responses.  ARP responses received are used to update this IP's     */
/*    ARP cache and dequeue and send any waiting packet.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    packet_ptr                            Received ARP packet           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release the ARP request       */
/*    (nx_ip_arp_allocate)                  ARP entry allocate call       */
/*    (nx_ip_arp_gratuitous_response_handler) ARP gratuitous response     */
/*    _nx_arp_queue_send                    Send the queued packet        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_arp_queue_process                 ARP receive queue processing  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler errors,      */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_arp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

ULONG        *message_ptr;
ULONG         sender_physical_msw;
ULONG         sender_physical_lsw;
ULONG         sender_ip_address;
ULONG         target_ip_address;
ULONG         message_type;
ULONG         index;
UCHAR         consumed = NX_FALSE;
NX_ARP       *arp_ptr;
NX_IP_DRIVER  driver_request;
NX_INTERFACE *interface_ptr;


#ifndef NX_DISABLE_RX_SIZE_CHECKING
    /* Determine if the packet length is valid.  */
    if (packet_ptr -> nx_packet_length < NX_ARP_MESSAGE_SIZE)
    {

        /* Invalid ARP message.  Release the packet and return.  */

#ifndef NX_DISABLE_ARP_INFO
        /* Increment the ARP invalid messages count.  */
        ip_ptr -> nx_ip_arp_invalid_messages++;
#endif

        /* Invalid ARP message.  Just release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING  */

    /* Setup a pointer to the ARP message.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    message_ptr =  (ULONG *)packet_ptr -> nx_packet_prepend_ptr;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the ARP message.  */
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 3));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 4));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 5));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 6));

    /* Pickup the ARP message type.  */
    message_type =  (ULONG)(*(message_ptr + 1) & 0xFFFF);

    /* Determine if the ARP message type is valid.  */
    if ((message_type != NX_ARP_OPTION_REQUEST) && (message_type != NX_ARP_OPTION_RESPONSE))
    {

        /* Invalid ARP message.  Release the packet and return.  */

#ifndef NX_DISABLE_ARP_INFO
        /* Increment the ARP invalid messages count.  */
        ip_ptr -> nx_ip_arp_invalid_messages++;
#endif

        /* Invalid ARP message.  Just release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }

    /* Pick up the sender's physical address from the message.  */
    sender_physical_msw =  (*(message_ptr + 2) >> 16);
    sender_physical_lsw =  (*(message_ptr + 2) << 16) | (*(message_ptr + 3) >> 16);
    sender_ip_address =    (*(message_ptr + 3) << 16) | (*(message_ptr + 4) >> 16);
    target_ip_address =    *(message_ptr + 6);

    /* Does the packet have an interface assigned? */
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr == NX_NULL)
    {

        /* No, so default it to the primary interface. */
        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &ip_ptr -> nx_ip_interface[0];
    }

    /* Pickup the interface information from the incoming packet. */
    interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

    /* Determine if it is an IP address conflict when IP address probing.  */
    if ((interface_ptr -> nx_interface_ip_address == 0) &&
        (interface_ptr -> nx_interface_ip_probe_address != 0) &&
        ((sender_ip_address == interface_ptr -> nx_interface_ip_probe_address) ||
         ((sender_ip_address == 0) && (target_ip_address == interface_ptr -> nx_interface_ip_probe_address))))
    {

        /* Make sure the sender physical address is not ours.  */
        if ((sender_physical_msw != interface_ptr -> nx_interface_physical_address_msw) ||
            (sender_physical_lsw != interface_ptr -> nx_interface_physical_address_lsw))
        {

            /* Determine if there is a a IP address conflict notify handler.  */
            if (interface_ptr -> nx_interface_ip_conflict_notify_handler)
            {

                /* A IP address conflict is present, call the notification handler.  */
                (interface_ptr -> nx_interface_ip_conflict_notify_handler)(ip_ptr, interface_ptr -> nx_interface_index, interface_ptr -> nx_interface_ip_probe_address,
                                                                           sender_physical_msw, sender_physical_lsw);
            }
        }

        /* Release the packet. */
        _nx_packet_release(packet_ptr);

        return;
    }

    /* Determine if it is an address conflict packet after set the IP address.  */
    if ((sender_ip_address != 0) && (sender_ip_address == interface_ptr -> nx_interface_ip_address))
    {

        /* Is it sent from other devices. */
        if ((sender_physical_msw != packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_physical_address_msw) ||
            (sender_physical_lsw != packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_physical_address_lsw))
        {

            /* Yes it is.  */
            if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_arp_defend_timeout == 0)
            {

                /* Set defend timeout. */
                packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_arp_defend_timeout = NX_ARP_DEFEND_INTERVAL;

                /* Send the announcement. */
                _nx_arp_packet_send(ip_ptr, sender_ip_address, packet_ptr -> nx_packet_address.nx_packet_interface_ptr);
            }

            /* Determine if there is a a IP address conflict notify handler.  */
            if (interface_ptr -> nx_interface_ip_conflict_notify_handler)
            {

                /* A IP address conflict is present, call the notification handler.  */
                (interface_ptr -> nx_interface_ip_conflict_notify_handler)(ip_ptr, interface_ptr -> nx_interface_index, interface_ptr -> nx_interface_ip_probe_address,
                                                                           sender_physical_msw, sender_physical_lsw);
            }

            /* This is likely in response to our previous gratuitous ARP from another entity on the
               network has the same IP address.  */

            /* Determine if there is a gratuitous ARP response handler.  */
            if (ip_ptr -> nx_ip_arp_gratuitous_response_handler)
            {

                /* Yes, call the gratuitous ARP response handler. Note that it is responsible
                   for releasing the packet!  */
                (ip_ptr -> nx_ip_arp_gratuitous_response_handler)(ip_ptr, packet_ptr);

                return;
            }

#ifdef NX_ARP_DEFEND_BY_REPLY
#ifndef NX_DISABLE_ARP_INFO
            /* Increment the ARP responses sent count.  */
            ip_ptr -> nx_ip_arp_responses_sent++;
#endif

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_RESPONSE_SEND, ip_ptr, sender_ip_address, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Set the ARP message type to ARP response.  */
            *(message_ptr + 1) =  (*(message_ptr + 1) & 0xFFFF0000) | NX_ARP_OPTION_RESPONSE;

            /* Now fill in the new source and destination information for the ARP response.  */
            *(message_ptr + 2) =  (ULONG)(packet_ptr -> nx_packet_ip_interface -> nx_interface_physical_address_msw << 16) |
                (packet_ptr -> nx_packet_ip_interface -> nx_interface_physical_address_lsw >> 16);
            *(message_ptr + 3) =  (ULONG)(packet_ptr -> nx_packet_ip_interface -> nx_interface_physical_address_lsw << 16) |
                (packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_address >> 16);
            *(message_ptr + 4) =  (ULONG)(packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_address << 16);
            *(message_ptr + 5) =  (ULONG)0;
            *(message_ptr + 6) =  (ULONG)0;

            /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
               swap the endian of the ARP message.  */
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 1));
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 2));
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 3));
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 4));
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 5));
            NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 6));

            /* Make sure the packet length is set properly.  */
            packet_ptr -> nx_packet_length =  NX_ARP_MESSAGE_SIZE;

            /* Setup the append pointer, since the received ARP packet can be padded
               with unnecessary bytes.  */
            packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ARP_MESSAGE_SIZE;

            /* Send the ARP request to the driver.  */
            driver_request.nx_ip_driver_ptr                  =  ip_ptr;
            driver_request.nx_ip_driver_command              =  NX_LINK_ARP_RESPONSE_SEND;
            driver_request.nx_ip_driver_packet               =  packet_ptr;
            driver_request.nx_ip_driver_physical_address_msw =  0xFFFFUL;
            driver_request.nx_ip_driver_physical_address_lsw =  0xFFFFFFFFUL;
            driver_request.nx_ip_driver_interface            =  packet_ptr -> nx_packet_ip_interface;

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_ARP_RESPONSE_SEND, ip_ptr, packet_ptr, packet_ptr -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* No need to update packet_ptr -> nx_packet_ip_interface.  When responding to an ARP request, use the same interface where the request was received. */

            (packet_ptr -> nx_packet_ip_interface -> nx_interface_link_driver_entry)(&driver_request);

            return;
#endif /* NX_ARP_DEFEND_BY_REPLY */
        }

        /* Release the conflict packet. */
        _nx_packet_release(packet_ptr);

        return;
    }

    /* Determine what type of ARP message this is.  Note that ARP requests must
       also specify this IP instance's IP address.  */
    if ((message_type == NX_ARP_OPTION_REQUEST) && (target_ip_address == (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address)))
    {

#ifndef NX_DISABLE_ARP_INFO

        /* Increment the ARP requests received count.  */
        ip_ptr -> nx_ip_arp_requests_received++;

        /* Increment the ARP responses sent count.  */
        ip_ptr -> nx_ip_arp_responses_sent++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_REQUEST_RECEIVE, ip_ptr, sender_ip_address, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_RESPONSE_SEND, ip_ptr, sender_ip_address, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Set the ARP message type to ARP response.  */
        *(message_ptr + 1) =  (*(message_ptr + 1) & 0xFFFF0000) | NX_ARP_OPTION_RESPONSE;


        /* Now fill in the new source and destination information for the ARP response.  */
        *(message_ptr + 2) =  (ULONG)(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_physical_address_msw << 16) |
            (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_physical_address_lsw >> 16);
        *(message_ptr + 3) =  (ULONG)(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_physical_address_lsw << 16) |
            (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address >> 16);
        *(message_ptr + 4) =  (ULONG)(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address << 16) | sender_physical_msw;
        *(message_ptr + 5) =  (ULONG)sender_physical_lsw;
        *(message_ptr + 6) =  (ULONG)sender_ip_address;

        /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
           swap the endian of the ARP message.  */
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 1));
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 2));
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 3));
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 4));
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 5));
        NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 6));

        /* Make sure the packet length is set properly.  */
        packet_ptr -> nx_packet_length =  NX_ARP_MESSAGE_SIZE;

        /* Setup the append pointer, since the received ARP packet can be padded
           with unnecessary bytes.  */
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ARP_MESSAGE_SIZE;

        /* Send the ARP request to the driver.  */
        driver_request.nx_ip_driver_ptr =      ip_ptr;
        driver_request.nx_ip_driver_command =  NX_LINK_ARP_RESPONSE_SEND;
        driver_request.nx_ip_driver_packet =   packet_ptr;
        driver_request.nx_ip_driver_physical_address_msw =  sender_physical_msw;
        driver_request.nx_ip_driver_physical_address_lsw =  sender_physical_lsw;
        driver_request.nx_ip_driver_interface            =  packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_ARP_RESPONSE_SEND, ip_ptr, packet_ptr, packet_ptr -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* No need to update interface.  When responding to an ARP request, use the same interface where the request was received. */
        (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_link_driver_entry)(&driver_request);

        /* Set the consumed as NX_TRUE, do not need to release the packet.  */
        consumed = NX_TRUE;
    }
    else
    {

        /* We have a response to a previous ARP request or Gratuitous ARP from another network entity.  */

#ifndef NX_DISABLE_ARP_INFO

        /* Check for the message type to see which counter to increment.  */
        if (message_type == NX_ARP_OPTION_REQUEST)
        {

            /* Increment the ARP requests received count.  */
            ip_ptr -> nx_ip_arp_requests_received++;

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_REQUEST_RECEIVE, ip_ptr, sender_ip_address, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);
        }
        else
        {

            /* Increment the ARP responses received count.  */
            ip_ptr -> nx_ip_arp_responses_received++;

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ARP_RESPONSE_RECEIVE, ip_ptr, sender_ip_address, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);
        }
#endif /* NX_DISABLE_ARP_INFO */
    }

    /* In either case, search the ARP cache to update any entry that matches the sender's IP
       address.  */

    /* Now we need to search through the active ARP list for the IP address
       to see if there is a matching entry.  */

    /* Calculate the hash index for the sender IP address.  */
    index =  (UINT)((sender_ip_address + (sender_ip_address >> 8)) & NX_ARP_TABLE_MASK);

    /* Pickup the first ARP entry.  */
    arp_ptr = NX_NULL;

    /* Ignore anything from any ARP packet with a zero sender IP address. */
    if (sender_ip_address != 0)
    {
        /* Calculate the hash index for the sender IP address.  */
        index =  (UINT)((sender_ip_address + (sender_ip_address >> 8)) & NX_ROUTE_TABLE_MASK);

        /* Pickup the first ARP entry.  */
        arp_ptr =  ip_ptr -> nx_ip_arp_table[index];
    }

    /* Loop to look for an ARP match.  */
    while (arp_ptr)
    {

        /* Check for an IP match.  */
        if (arp_ptr -> nx_arp_ip_address == sender_ip_address)
        {


#ifdef NX_ENABLE_ARP_MAC_CHANGE_NOTIFICATION

            /* Determine if there is a ARP collision notify handler.  */
            if (ip_ptr -> nx_ip_arp_collision_notify_response_handler)
            {

                /* Now check if the machine address is stored in our ARP cache.  */
                if ((arp_ptr -> nx_arp_physical_address_msw != 0) || (arp_ptr -> nx_arp_physical_address_lsw != 0))
                {

                    /* Now check if its machine address is different from what is in our ARP cache.  */
                    if ((arp_ptr -> nx_arp_physical_address_msw != sender_physical_msw) ||
                        (arp_ptr -> nx_arp_physical_address_lsw != sender_physical_lsw))
                    {

                        /* A collision is present with the mapping in our ARP table.  Call the notification handler.
                           Note: the application must release the packet. */
                        (ip_ptr -> nx_ip_arp_collision_notify_response_handler)((void *)packet_ptr);

                        /* We're done.  NetX does not respond or do any further processing.*/
                        return;
                    }
                }
            }
#endif /* NX_ENABLE_ARP_MAC_CHANGE_NOTIFICATION */

            /* No need to update the static ARP entry. */
            if (arp_ptr -> nx_arp_route_static)
            {
                break;
            }

            /* Save the physical address found in this ARP response.  */
            arp_ptr -> nx_arp_physical_address_msw =  sender_physical_msw;
            arp_ptr -> nx_arp_physical_address_lsw =  sender_physical_lsw;

            /* Set the update rate to the expiration rate since we now have an ARP
               response.  */
            arp_ptr -> nx_arp_entry_next_update =  NX_ARP_EXPIRATION_RATE;

            /* Reset the retry counter for this ARP entry.  */
            arp_ptr -> nx_arp_retries =  0;

            /* Set the interface attached to this packet. */
            arp_ptr -> nx_arp_ip_interface = interface_ptr;

            /* Call queue send function to send the packet queued up.  */
            _nx_arp_queue_send(ip_ptr, arp_ptr);

            /* Yes, we found a match.  Get out of the loop!  */
            break;
        }

        /* Move to the next active ARP entry.  */
        arp_ptr =  arp_ptr -> nx_arp_active_next;

        /* Determine if we are at the end of the ARP list.  */
        if (arp_ptr == ip_ptr -> nx_ip_arp_table[index])
        {

            /* Clear the ARP pointer.  */
            arp_ptr =  NX_NULL;
            break;
        }
    }

    /* Determine if we have a packet to release. */
    if (consumed == NX_FALSE)
    {
        _nx_packet_release(packet_ptr);
    }

#ifndef NX_DISABLE_ARP_AUTO_ENTRY

    /* Determine if anything was found.  Ignore ARP messages with a zero IP sender address.   */
    if ((arp_ptr == NX_NULL) && (sender_ip_address != 0))
    {

        /* Calculate the hash index for the sender IP address.  */
        index =  (UINT)((sender_ip_address + (sender_ip_address >> 8)) & NX_ARP_TABLE_MASK);

        /* Allocate a new ARP entry in advance of the need to send to the IP
           address.  */
        if (((ip_ptr -> nx_ip_arp_allocate)(ip_ptr, &(ip_ptr -> nx_ip_arp_table[index]), NX_FALSE)) == NX_SUCCESS)
        {

            /* Setup a pointer to the new ARP entry.  */
            arp_ptr =  (ip_ptr -> nx_ip_arp_table[index]) -> nx_arp_active_previous;

            /* Setup the IP address and clear the physical mapping.  */
            arp_ptr -> nx_arp_ip_address =            sender_ip_address;
            arp_ptr -> nx_arp_physical_address_msw =  sender_physical_msw;
            arp_ptr -> nx_arp_physical_address_lsw =  sender_physical_lsw;
            arp_ptr -> nx_arp_entry_next_update =     NX_ARP_EXPIRATION_RATE;
            arp_ptr -> nx_arp_retries =               0;
            arp_ptr -> nx_arp_ip_interface         =  interface_ptr;
        }
    }
#endif /* NX_DISABLE_ARP_AUTO_ENTRY */
}
#endif /* !NX_DISABLE_IPV4  */

