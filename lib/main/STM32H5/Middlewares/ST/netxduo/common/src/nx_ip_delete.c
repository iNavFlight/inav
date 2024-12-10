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
#include "nx_icmp.h"
#include "nx_arp.h"
#ifdef FEATURE_NX_IPV6
#include "nx_nd_cache.h"
#endif /* FEATURE_NX_IPV6 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_delete                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an Internet Protocol instance, including      */
/*    calling the associated driver with a link disable request.          */
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
/*    _nx_ip_delete_queue_clear             Clear a packet queue          */
/*    _nx_ip_raw_packet_cleanup             Cleanup raw packet suspension */
/*    _nx_icmp_cleanup                      Cleanup for ICMP packets      */
/*    _nx_ip_fragment_disable               Disable fragment processing   */
/*    tx_mutex_delete                       Delete IP protection mutex    */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    tx_thread_terminate                   Terminate IP helper thread    */
/*    tx_event_flags_delete                 Delete IP event flags         */
/*    tx_thread_delete                      Delete IP helper thread       */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    tx_timer_deactivate                   Deactivate IP-ARP timer       */
/*    tx_timer_delete                       Delete IP-ARP timer           */
/*    (ip_link_driver)                      User supplied link driver     */
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
UINT  _nx_ip_delete(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT         i;
NX_IP_DRIVER driver_request;
NX_PACKET   *raw_packet_head;
NX_PACKET   *deferred_head;
NX_PACKET   *icmp_queue_head;
NX_PACKET   *tcp_queue_head;
#ifndef NX_DISABLE_IPV4
NX_PACKET   *arp_queue_head;
NX_PACKET   *rarp_queue_head;
NX_PACKET   *igmp_queue_head;
#endif /* !NX_DISABLE_IPV4  */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_DELETE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the IP instance has any sockets bound to it.  */
    if ((ip_ptr -> nx_ip_udp_created_sockets_count) || (ip_ptr -> nx_ip_tcp_created_sockets_count))
    {

        /* Still sockets bound to this IP instance.  They must all be deleted prior
           to deleting the IP instance.  Release the mutex and return
           an error code.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_SOCKETS_BOUND);
    }

    /* Call through every link driver to disable the link.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Skip the invalid entries. */
        if (!(ip_ptr -> nx_ip_interface[i].nx_interface_valid))
        {
            continue;
        }

        driver_request.nx_ip_driver_ptr         =  ip_ptr;
        driver_request.nx_ip_driver_command     =  NX_LINK_DISABLE;
        driver_request.nx_ip_driver_interface   =  &(ip_ptr -> nx_ip_interface[i]);

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_LINK_DISABLE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Send the driver LINK DISABLE request. */
        (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);

        /* Call the link driver to uninitialize.  */
        driver_request.nx_ip_driver_ptr =      ip_ptr;
        driver_request.nx_ip_driver_command =  NX_LINK_UNINITIALIZE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_UNINITIALIZE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Send the driver UNITIALIZE request. */
        (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Remove the IP instance from the created list.  */

    /* See if the IP instance is the only one on the list.  */
    if (ip_ptr == ip_ptr -> nx_ip_created_next)
    {

        /* Only created IP instance, just set the created list to NULL.  */
        _nx_ip_created_ptr =  TX_NULL;
    }
    else
    {

        /* Otherwise, not the only created IP, link-up the neighbors.  */
        (ip_ptr -> nx_ip_created_next) -> nx_ip_created_previous =
            ip_ptr -> nx_ip_created_previous;
        (ip_ptr -> nx_ip_created_previous) -> nx_ip_created_next =
            ip_ptr -> nx_ip_created_next;

        /* See if we have to update the created list head pointer.  */
        if (_nx_ip_created_ptr == ip_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_ip_created_ptr =  ip_ptr -> nx_ip_created_next;
        }
    }

    /* Decrement the IP created counter.  */
    _nx_ip_created_count--;

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Release any raw packets queued up.  */
    raw_packet_head =  ip_ptr ->  nx_ip_raw_received_packet_head;
    ip_ptr ->  nx_ip_raw_received_packet_head =  NX_NULL;
    ip_ptr ->  nx_ip_raw_received_packet_tail =  NX_NULL;
    ip_ptr ->  nx_ip_raw_received_packet_count = 0;

    /* Release all deferred IP packets.  */
    deferred_head =  ip_ptr ->  nx_ip_deferred_received_packet_head;
    ip_ptr ->  nx_ip_deferred_received_packet_head =  NX_NULL;
    ip_ptr ->  nx_ip_deferred_received_packet_tail =  NX_NULL;

    /* Release all queued ICMP packets.  */
    icmp_queue_head =   ip_ptr ->  nx_ip_icmp_queue_head;
    ip_ptr ->  nx_ip_icmp_queue_head =  NX_NULL;

#ifndef NX_DISABLE_IPV4
    /* Release all queued IGMP packets.  */
    igmp_queue_head =   ip_ptr ->  nx_ip_igmp_queue_head;
    ip_ptr ->  nx_ip_igmp_queue_head =  NX_NULL;

    /* Release all queued ARP packets.  */
    arp_queue_head =  ip_ptr ->  nx_ip_arp_deferred_received_packet_head;
    ip_ptr ->  nx_ip_arp_deferred_received_packet_head =  NX_NULL;
    ip_ptr ->  nx_ip_arp_deferred_received_packet_tail =  NX_NULL;

    /* Release all queued RARP packets.  */
    rarp_queue_head =  ip_ptr ->  nx_ip_rarp_deferred_received_packet_head;
    ip_ptr ->  nx_ip_rarp_deferred_received_packet_head = NX_NULL;
    ip_ptr ->  nx_ip_rarp_deferred_received_packet_tail = NX_NULL;
#endif /* !NX_DISABLE_IPV4  */

    /* Release all queued TCP packets.  */
    tcp_queue_head =  ip_ptr ->  nx_ip_tcp_queue_head;
    ip_ptr ->  nx_ip_tcp_queue_head =  NX_NULL;
    ip_ptr ->  nx_ip_tcp_queue_tail =  NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Deactivate and delete the IP periodic timer.  */
    tx_timer_deactivate(&(ip_ptr -> nx_ip_periodic_timer));
    tx_timer_delete(&(ip_ptr -> nx_ip_periodic_timer));

    /* Determine if the fast timer has been created.  */
    if (ip_ptr -> nx_ip_fast_periodic_timer_created)
    {

        /* Yes. Deactivate and delete the IP fast periodic timer.  */
        tx_timer_deactivate(&(ip_ptr -> nx_ip_fast_periodic_timer));
        tx_timer_delete(&(ip_ptr -> nx_ip_fast_periodic_timer));
        ip_ptr -> nx_ip_fast_periodic_timer_created = 0;
    }

    /* Terminate the internal IP thread.  */
    tx_thread_terminate(&(ip_ptr -> nx_ip_thread));

    /* Delete the internal IP protection mutex.  */
    tx_mutex_delete(&(ip_ptr -> nx_ip_protection));

    /* Delete the internal IP event flag object.  */
    tx_event_flags_delete(&(ip_ptr -> nx_ip_events));

    /* Delete the internal IP thread for handling more processing intensive
       duties.  */
    tx_thread_delete(&(ip_ptr -> nx_ip_thread));

    /* Release any raw packets queued up.  */
    if (raw_packet_head)
    {
        _nx_ip_delete_queue_clear(raw_packet_head);
    }

    /* Release any deferred IP packets.  */
    if (deferred_head)
    {
        _nx_ip_delete_queue_clear(deferred_head);
    }

    /* Release any queued ICMP packets.  */
    if (icmp_queue_head)
    {
        _nx_ip_delete_queue_clear(icmp_queue_head);
    }

    /* Release any queued TCP packets.  */
    if (tcp_queue_head)
    {
        _nx_ip_delete_queue_clear(tcp_queue_head);
    }

#ifndef NX_DISABLE_IPV4
    /* Release any queued ARP packets.  */
    if (arp_queue_head)
    {
        _nx_ip_delete_queue_clear(arp_queue_head);
    }

    /* Release any queued RARP packets.  */
    if (rarp_queue_head)
    {
        _nx_ip_delete_queue_clear(rarp_queue_head);
    }

    /* Release any queued IGMP packets.  */
    if (igmp_queue_head)
    {
        _nx_ip_delete_queue_clear(igmp_queue_head);
    }
#endif /* !NX_DISABLE_IPV4  */

    /* Lift any suspension on RAW IP packet receives.  */
    while (ip_ptr -> nx_ip_raw_packet_suspension_list)
    {

        /* Release the suspended thread.  */
        _nx_ip_raw_packet_cleanup(ip_ptr -> nx_ip_raw_packet_suspension_list NX_CLEANUP_ARGUMENT);
    }

    /* Lift any suspension on ICMP ping requests.  */
    while (ip_ptr -> nx_ip_icmp_ping_suspension_list)
    {

        /* Release the suspended thread.  */
        _nx_icmp_cleanup(ip_ptr -> nx_ip_icmp_ping_suspension_list NX_CLEANUP_ARGUMENT);
    }

    /* Determine if fragment processing was enabled.  */
    if (ip_ptr -> nx_ip_fragment_processing)
    {

        /* Yes, disable fragment processing, which will release all outstanding
           fragmented packets.  */
        _nx_ip_fragment_disable(ip_ptr);
    }

#ifndef NX_DISABLE_IPV4
    /* Invalidate dynamic ARP entries. */
    _nx_arp_dynamic_entries_invalidate(ip_ptr);
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* Invalidate ND cache. */
    _nxd_nd_cache_invalidate(ip_ptr);
#endif /* FEATURE_NX_IPV6 */

    /* Clear the IP ID to make it invalid.  */
    ip_ptr -> nx_ip_id =  0;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Restore preemption.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}

