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
#include "nx_ip.h"
#include "nx_packet.h"
#include "nx_rarp.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_rarp_packet_deferred_receive                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives an RARP packet from the link driver (usually */
/*    the link driver's input ISR) and places it in the deferred receive  */
/*    RARP packet queue.  This moves the minimal receive RARP packet      */
/*    processing from the ISR to the IP helper thread.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Wakeup IP helper thread       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application I/O Driver                                              */
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
VOID  _nx_rarp_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Check to see if RARP is enabled on this IP instance.  */
    if (!ip_ptr -> nx_ip_rarp_queue_process)
    {

        /* RARP is not enabled.  */

#ifndef NX_DISABLE_RARP_INFO
        /* Increment the RARP invalid messages count...  */
        ip_ptr -> nx_ip_rarp_invalid_messages++;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

        /* Since RARP is not enabled, just release the packet.  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }

    /* Check to see if the RARP deferred processing queue is empty.  */
    if (ip_ptr -> nx_ip_rarp_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the RARP deferred queue.  */
        (ip_ptr -> nx_ip_rarp_deferred_received_packet_tail) -> nx_packet_queue_next =  packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        ip_ptr -> nx_ip_rarp_deferred_received_packet_tail =  packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Empty RARP deferred receive processing queue.  Just setup the head pointers and
           set the event flags to ensure the IP helper thread looks at the RARP deferred
           processing queue.  */
        ip_ptr -> nx_ip_rarp_deferred_received_packet_head =  packet_ptr;
        ip_ptr -> nx_ip_rarp_deferred_received_packet_tail =  packet_ptr;
        packet_ptr -> nx_packet_queue_next =                  NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP helper thread to process the RARP deferred receive.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_RARP_REC_EVENT, TX_OR);
    }
}
#endif /* !NX_DISABLE_IPV4  */

