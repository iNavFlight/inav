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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_queue_process                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the TCP receive packet queue.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_process                Process TCP packet            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread processing   */
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
VOID  _nx_tcp_queue_process(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_PACKET *queue_head;
NX_PACKET *packet_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Remove the TCP message queue from the IP structure.  */
    queue_head =  ip_ptr -> nx_ip_tcp_queue_head;
    ip_ptr -> nx_ip_tcp_queue_head =  NX_NULL;
    ip_ptr -> nx_ip_tcp_queue_tail =  NX_NULL;
    ip_ptr -> nx_ip_tcp_received_packet_count =  0;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the entire TCP message queue and process packets
       one by one.  */
    while (queue_head)
    {

        /* Pickup the first queue TCP message and remove it from the
           TCP queue.  */
        packet_ptr =  queue_head;
        queue_head =  queue_head -> nx_packet_queue_next;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

        /* Process the packet.  */
        _nx_tcp_packet_process(ip_ptr, packet_ptr);
    }
}

