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
/*    _nx_ip_fragment_disable                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disables IP fragment assembly processing and releases */
/*    all partial fragments being assembled.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet                */
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
UINT  _nx_ip_fragment_disable(NX_IP *ip_ptr)
{
#ifndef NX_DISABLE_FRAGMENTATION
TX_INTERRUPT_SAVE_AREA

#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
NX_PACKET *new_fragment_head;
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */
NX_PACKET *assemble_head;
NX_PACKET *next_packet;
NX_PACKET *release_packet;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_FRAGMENT_DISABLE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts temporarily.  */
    TX_DISABLE

    /* Clear the IP fragment processing routine pointer.  */
    ip_ptr -> nx_ip_fragment_processing =  NX_NULL;

    /* Clear the IP fragment assembly routine pointer.  */
    ip_ptr -> nx_ip_fragment_assembly =  NX_NULL;

    /* Clear the IP fragment timeout routine pointer.  */
    ip_ptr -> nx_ip_fragment_timeout_check =  NX_NULL;

    /* Pickup the fragment list pointer.  */
#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
    new_fragment_head = ip_ptr -> nx_ip_received_fragment_head;
#else
    NX_ASSERT(ip_ptr -> nx_ip_received_fragment_head == NX_NULL);
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */
    assemble_head =     ip_ptr -> nx_ip_fragment_assembly_head;

    /* Clear the IP structure lists.  */
    ip_ptr -> nx_ip_received_fragment_head =  NX_NULL;
    ip_ptr -> nx_ip_received_fragment_tail =  NX_NULL;
    ip_ptr -> nx_ip_fragment_assembly_head =  NX_NULL;
    ip_ptr -> nx_ip_fragment_assembly_tail =  NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
    /* Now walk through the receive and assembly lists to free the packets.  */
    next_packet =  new_fragment_head;
    while (next_packet)
    {

        /* Set the release packet to this packet.  */
        release_packet =  next_packet;

        /* Move next packet to the next in the list.  */
        next_packet =  next_packet -> nx_packet_queue_next;

        /* Release the current packet.  */
        _nx_packet_release(release_packet);
    }
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */

    /* Now walk through the assemble list and release all packets.  */
    while (assemble_head)
    {

        /* Walk through the list of packets being assembled for this packet and release them.  */
        next_packet =  assemble_head;
        assemble_head =  next_packet -> nx_packet_queue_next;
        while (next_packet)
        {

            /* Set the release packet to this packet.  */
            release_packet =  next_packet;

            /* Move next packet to the next in the list.  */
            next_packet =  next_packet -> nx_packet_union_next.nx_packet_fragment_next;

            /* Reset tcp_queue_next before releasing. */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            release_packet -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

            /* Release the current packet.  */
            _nx_packet_release(release_packet);
        }
    }

    /* Return success to the caller.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_FRAGMENTATION */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_ENABLED);

#endif /* NX_DISABLE_FRAGMENTATION */
}

