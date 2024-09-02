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
/**   Reverse Address Resolution Protocol (RARP)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_rarp.h"
#include "nx_packet.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_rarp_packet_receive                             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the reception of both the RARP request and  */
/*    the RARP response.  RARP responses received are used to setup the   */
/*    the IP address of this IP instance.  Once the IP address is setup,  */
/*    RARP is automatically disabled.  RARP requests are discarded        */
/*    at present time.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    packet_ptr                            Received RARP packet          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release the RARP request      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_rarp_queue_process                RARP receive queue processing */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
VOID  _nx_rarp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA

ULONG *message_ptr;
UINT   i;
VOID   (*address_change_notify)(NX_IP *, VOID *) = NX_NULL;
VOID  *additional_info = NX_NULL;
VOID   (*address_change_notify_internal)(NX_IP *, VOID *) = NX_NULL;


#ifndef NX_DISABLE_RX_SIZE_CHECKING
    /* Determine if the packet length is valid.  */
    if (packet_ptr -> nx_packet_length < NX_RARP_MESSAGE_SIZE)
    {

#ifndef NX_DISABLE_RARP_INFO
        /* Increment the RARP invalid messages count...  At least until RARP server
           logic is added.  */
        ip_ptr -> nx_ip_rarp_invalid_messages++;
#endif

        /* Just release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING  */

    /* Setup a pointer to the RARP message.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    message_ptr =  (ULONG *)packet_ptr -> nx_packet_prepend_ptr;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the RARP message.  */
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 1));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 2));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 3));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 4));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 5));
    NX_CHANGE_ULONG_ENDIAN(*(message_ptr + 6));

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_RARP_RECEIVE, ip_ptr, *(message_ptr + 6), packet_ptr, *(message_ptr + 1), NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Determine what type of RARP message this is.  Note that RARP requests must
       also specify this IP instance's IP address.  */
    if ((*(message_ptr + 1) & 0xFFFF) == NX_RARP_OPTION_REQUEST)
    {

#ifndef NX_DISABLE_RARP_INFO
        /* Increment the RARP invalid messages count...  At least until RARP server
           logic is added.  */
        ip_ptr -> nx_ip_rarp_invalid_messages++;
#endif

        /* Just release the packet.  */
        _nx_packet_release(packet_ptr);
    }
    else if ((*(message_ptr + 1) & 0xFFFF) == NX_RARP_OPTION_RESPONSE)
    {

        /* We have a response to a previous RARP request.  */

#ifndef NX_DISABLE_ARP_INFO
        /* Increment the RARP responses received count.  */
        ip_ptr -> nx_ip_rarp_responses_received++;
#endif

        /* Disable Interrupts.  */
        TX_DISABLE

        /* Determine if the target IP address is non-zero.  */
        if (*(message_ptr + 6) && (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address == 0))
        {

            /* Pickup the current notification callback and additional information pointers.  */
            address_change_notify =  ip_ptr -> nx_ip_address_change_notify;
            additional_info =        ip_ptr -> nx_ip_address_change_notify_additional_info;

            /* Pickup the internal notification callback.  */
            address_change_notify_internal = ip_ptr -> nx_ip_address_change_notify_internal;

            /* Set the IP address of this IP instance to the target
               IP address in the RARP response.  */
            packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address =  *(message_ptr + 6);

            /* Loop through all the interfaces and check whether or not to continue periodic RARP requests. */
            for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
            {

                /* Skip the invalid interface entry. */
                if (ip_ptr -> nx_ip_interface[i].nx_interface_valid == 0)
                {
                    continue;
                }

                /* Look for any interfaces still without a valid IP address. */
                if ((ip_ptr -> nx_ip_interface[i].nx_interface_ip_address == 0) &&
                    (ip_ptr -> nx_ip_interface[i].nx_interface_address_mapping_needed == NX_TRUE))
                {
                    /* At least one interface still has non-zero IP address. Continue RARP process. */
                    break;
                }
            }

            /* Do we need to continue with periodic RARP requests? */
            if (i >= NX_MAX_PHYSICAL_INTERFACES)
            {

                /* No, Disable the RARP activity now that we have a valid IP address.  */
                ip_ptr -> nx_ip_rarp_periodic_update =  NX_NULL;
                ip_ptr -> nx_ip_rarp_queue_process =    NX_NULL;
            }
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the RARP response packet.  */
        _nx_packet_release(packet_ptr);
    }
    else
    {

#ifndef NX_DISABLE_RARP_INFO
        /* Increment the RARP invalid messages count.  */
        ip_ptr -> nx_ip_rarp_invalid_messages++;
#endif

        /* Invalid RARP message.  Just release the packet.  */
        _nx_packet_release(packet_ptr);
    }

    /* Determine if the application should be notified of the IP address change. */
    if (address_change_notify != NX_NULL)
    {
        /* Yes, call the application's IP address change notify function.  */
        (address_change_notify)(ip_ptr, additional_info);
    }

    /* Determine if the internal application should be notified of the IP address change. */
    if (address_change_notify_internal != NX_NULL)
    {
        /* Yes, call the internal application's IP address change notify function.  */
        (address_change_notify_internal)(ip_ptr, NX_NULL);
    }

    return;
}
#endif /* !NX_DISABLE_IPV4  */

