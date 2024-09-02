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
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"

#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_transmit_release                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function releases a transmitted packet chain back to the       */
/*    appropriate packet pool.  If the packet is a TCP packet, it is      */
/*    simply marked as completed.  The actual release is deferred to      */
/*    the TCP component.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer of packet to release  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet back to pool   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_packet_transmit_release(NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT status;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_TRANSMIT_RELEASE, packet_ptr, packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next, (packet_ptr -> nx_packet_pool_owner) -> nx_packet_pool_available, 0, NX_TRACE_PACKET_EVENTS, 0, 0);

    /* Disable interrupts temporarily.  */
    TX_DISABLE

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Determine if the packet is a queued TCP data packet.  Such packets cannot be released
       immediately, since they may need to be resent.  */
    /*lint -e{923} suppress cast of ULONG to pointer.  */
    if ((packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET *)NX_PACKET_ALLOCATED)) &&
        (packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET *)NX_PACKET_FREE)))
    {

        /* Yes, this is indeed a TCP packet.  Just mark this with the NX_DRIVER_TX_DONE
           value to let the TCP layer know it is no longer queued up.  */
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        packet_ptr -> nx_packet_queue_next =  (NX_PACKET *)NX_DRIVER_TX_DONE;

        /* Remove the IP header and adjust the length.  */
        packet_ptr -> nx_packet_prepend_ptr += packet_ptr -> nx_packet_ip_header_length;
        packet_ptr -> nx_packet_length -= packet_ptr -> nx_packet_ip_header_length;

        /* Reset the IP header length. */
        packet_ptr -> nx_packet_ip_header_length = 0;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        status =  NX_SUCCESS;
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call the actual packet release function.  */
        status =  _nx_packet_release(packet_ptr);
    }

    /* Return completion status.  */
    return(status);
}

