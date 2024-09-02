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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_icmp.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_queue_process                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the ICMP receive packet queue.              */
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
/*    _nx_icmp_packet_process               Process ICMP packet           */
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
VOID  _nx_icmp_queue_process(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_PACKET *queue_head;
NX_PACKET *packet_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Remove the ICMP message queue from the IP structure.  */
    queue_head =  ip_ptr -> nx_ip_icmp_queue_head;
    ip_ptr -> nx_ip_icmp_queue_head =  NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the entire ICMP message queue and process packets
       one by one.  */
    while (queue_head)
    {

        /* Pickup the first queued ICMP message and remove it from the
           ICMP queue.  */
        packet_ptr =  queue_head;
        queue_head =  queue_head -> nx_packet_queue_next;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Process the packet.  */
        _nx_icmp_packet_process(ip_ptr, packet_ptr);
    }
}
#endif /* !NX_DISABLE_IPV4  */

