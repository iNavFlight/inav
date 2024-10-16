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
#include "tx_thread.h"
#include "nx_ip.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_raw_packet_disable                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disables raw IP packet sending and receiving          */
/*    capability.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release data packet           */
/*    _nx_ip_raw_packet_cleanup             Release suspended threads     */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nx_ip_raw_packet_disable(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_PACKET *current_packet;
NX_PACKET *next_packet;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_RAW_PACKET_DISABLE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Disable raw IP packet sending/receiving.  */
    ip_ptr -> nx_ip_raw_ip_processing =  NX_NULL;

    /* Pickup the head pointer to any raw IP packets queued up.  */
    next_packet =  ip_ptr ->  nx_ip_raw_received_packet_head;

    /* Clear the head and tail pointers.  */
    ip_ptr ->  nx_ip_raw_received_packet_head =  NX_NULL;
    ip_ptr ->  nx_ip_raw_received_packet_tail =  NX_NULL;
    ip_ptr ->  nx_ip_raw_received_packet_count = 0;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Loop to release any queued up raw IP packets.  */
    while (next_packet)
    {

        /* Setup the current packet pointer.  */
        current_packet =  next_packet;

        /* Move to the next packet.  */
        next_packet =  next_packet -> nx_packet_queue_next;

        /* Release the current packet.  */
        _nx_packet_release(current_packet);
    }

    /* Lift any suspension on RAW IP packet receives.  */
    while (ip_ptr -> nx_ip_raw_packet_suspension_list)
    {

        /* Release the suspended thread.  */
        _nx_ip_raw_packet_cleanup(ip_ptr -> nx_ip_raw_packet_suspension_list NX_CLEANUP_ARGUMENT);
    }

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Disable interrupts.  */
    TX_DISABLE

    /* Restore preemption.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return a successful status!  */
    return(NX_SUCCESS);
}

