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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_driver_deferred_receive                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function places the supplied packet on the driver's deferred   */
/*    receive queue.  It will be processed later during subsequent        */
/*    execution of the IP helper thread by calling the driver's deferred  */
/*    handling routine.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Raw receive packet            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Driver Receive ISR                                      */
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
VOID  _nx_ip_driver_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

#ifdef NX_DRIVER_DEFERRED_PROCESSING
TX_INTERRUPT_SAVE_AREA

    /* Disable interrupts.  */
    TX_DISABLE

    /* Add the packet to the end of the driver queue for processing */
    packet_ptr -> nx_packet_queue_next = NX_NULL;
    if (ip_ptr -> nx_ip_driver_deferred_packet_head == NX_NULL)
    {

        /* The queue is empty, set both the first and last packet
            pointers to the new packet */
        ip_ptr -> nx_ip_driver_deferred_packet_head = packet_ptr;
        ip_ptr -> nx_ip_driver_deferred_packet_tail = packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP helper thread to process the packet.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_DRIVER_PACKET_EVENT, TX_OR);
    }
    else
    {

        /* The queue is not empty, simply add the packet to the end of the queue.  */
        (ip_ptr -> nx_ip_driver_deferred_packet_tail) -> nx_packet_queue_next = packet_ptr;
        ip_ptr -> nx_ip_driver_deferred_packet_tail = packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }

#else
    NX_PARAMETER_NOT_USED(ip_ptr);

    /* No deferred packet processing, just release the packet.  */
    _nx_packet_release(packet_ptr);
#endif
}

