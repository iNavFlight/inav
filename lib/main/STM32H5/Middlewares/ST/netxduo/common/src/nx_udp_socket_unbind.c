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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_udp.h"
#include "nx_packet.h"
#include "tx_thread.h"


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_driver_unbind                        PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unbinds a UDP port through TCP/IP offload interface.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_udp_socket_bind                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
static UINT _nx_udp_socket_driver_unbind(NX_UDP_SOCKET *socket_ptr)
{
UINT          i;
NX_INTERFACE *interface_ptr;
NX_IP        *ip_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* Loop all interfaces to unbind to ones support TCP/IP offload.  */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {

        /* Use a local variable for convenience.  */
        interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

        /* Check for valid interfaces.  */
        if (interface_ptr -> nx_interface_valid == NX_FALSE)
        {

            /* Skip interface not valid.  */
            continue;
        }

        /* Check for TCP/IP offload feature.  */
        if (((interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) == 0) ||
            (interface_ptr -> nx_interface_tcpip_offload_handler == NX_NULL))
        {

            /* Skip interface not support TCP/IP offload.  */
            continue;
        }

        /* Let TCP/IP offload interface unbind port.  Return value is ignored.  */
        interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr,
                                                            socket_ptr,
                                                            NX_TCPIP_OFFLOAD_UDP_SOCKET_UNBIND,
                                                            NX_NULL, NX_NULL, NX_NULL,
                                                            socket_ptr -> nx_udp_socket_port,
                                                            NX_NULL, NX_NO_WAIT);
    }

    return(NX_SUCCESS);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_unbind                               PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unbinds the UDP socket structure from the previously  */
/*    bound UDP port.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release data packet           */
/*    _nx_udp_bind_cleanup                  Remove and cleanup bind req   */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _tx_thread_system_resume              Resume suspended thread       */
/*    _tx_thread_system_preempt_check       Check for preemption          */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_udp_socket_unbind(NX_UDP_SOCKET *socket_ptr)
{
TX_INTERRUPT_SAVE_AREA

UINT           index;
UINT           port;
NX_IP         *ip_ptr;
TX_THREAD     *thread_ptr;
NX_UDP_SOCKET *new_socket_ptr;
NX_PACKET     *packet_ptr;
NX_PACKET     *next_packet_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_UNBIND, ip_ptr, socket_ptr, socket_ptr -> nx_udp_socket_port, 0, NX_TRACE_UDP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can figure out whether or not the port has already
       been bound to.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket is bound to port.  */
    if (!socket_ptr -> nx_udp_socket_bound_next)
    {

        /* Determine if there is a special condition for the socket not being in
           a bound condition...  i.e. the socket is in a pending-to-be-bound condition
           in a call from a different thread.  */
        if (socket_ptr -> nx_udp_socket_bind_in_progress)
        {

            /* Execute the bind suspension cleanup routine.  */
            _nx_udp_bind_cleanup(socket_ptr -> nx_udp_socket_bind_in_progress NX_CLEANUP_ARGUMENT);

            /* Release the protection mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return success.  */
            return(NX_SUCCESS);
        }
        else
        {

            /* Release the protection mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return a not bound error code.  */
            return(NX_NOT_BOUND);
        }
    }

    /* Otherwise, the socket is bound.  We need to remove this socket from the
       port and check for any other UDP socket bind requests that are queued.  */

    /* Pickup the port number in the UDP socket structure.  */
    port =  socket_ptr -> nx_udp_socket_port;

    /* Calculate the hash index in the UDP port array of the associated IP instance.  */
    index =  (UINT)((port + (port >> 8)) & NX_UDP_PORT_TABLE_MASK);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    _nx_udp_socket_driver_unbind(socket_ptr);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Disable interrupts while we unlink the current socket.  */
    TX_DISABLE

    /* Determine if this is the only socket bound on this port list.  */
    if (socket_ptr -> nx_udp_socket_bound_next == socket_ptr)
    {

        /* Yes, this is the only socket on the port list.  */

        /* Clear the list head pointer and the next pointer in the socket.  */
        ip_ptr -> nx_ip_udp_port_table[index] =   NX_NULL;
        socket_ptr -> nx_udp_socket_bound_next =  NX_NULL;
    }
    else
    {

        /* Relink the neighbors of this UDP socket.  */

        /* Update the links of the adjacent sockets.  */
        (socket_ptr -> nx_udp_socket_bound_next) -> nx_udp_socket_bound_previous =
            socket_ptr -> nx_udp_socket_bound_previous;
        (socket_ptr -> nx_udp_socket_bound_previous) -> nx_udp_socket_bound_next =
            socket_ptr -> nx_udp_socket_bound_next;

        /* Determine if the head of the port list points to the socket being removed.
           If so, we need to move the head pointer.  */
        if (ip_ptr -> nx_ip_udp_port_table[index] == socket_ptr)
        {

            /* Yes, we need to move the port list head pointer.  */
            ip_ptr -> nx_ip_udp_port_table[index] =  socket_ptr -> nx_udp_socket_bound_next;
        }

        /* Clear the next pointer in the socket to indicate it is no longer bound.  */
        socket_ptr -> nx_udp_socket_bound_next =  NX_NULL;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* The socket is off the bound list...  we need to check for queued packets and possible
       receive suspension.  We need to clean up either of these conditions.  */
    if (socket_ptr -> nx_udp_socket_receive_count)
    {

        /* Setup packet pointer.  */
        packet_ptr =  socket_ptr -> nx_udp_socket_receive_head;

        /* Clear the head and the tail pointers.  */
        socket_ptr -> nx_udp_socket_receive_head =  NX_NULL;
        socket_ptr -> nx_udp_socket_receive_tail =  NX_NULL;

        /* Loop to clear all the packets out.  */
        while (socket_ptr -> nx_udp_socket_receive_count)
        {

            /* Pickup the next queued packet.  */
            next_packet_ptr =  packet_ptr -> nx_packet_queue_next;

            /* Release the packet.  */
            _nx_packet_release(packet_ptr);

            /* Move to the next packet.  */
            packet_ptr =  next_packet_ptr;

            /* Decrease the queued packet count.  */
            socket_ptr -> nx_udp_socket_receive_count--;
        }
    }
    else if (socket_ptr -> nx_udp_socket_receive_suspended_count)
    {

        /* Clear out all threads suspended on this socket.  */

        /* Pickup the first suspended thread.  */
        thread_ptr =  socket_ptr -> nx_udp_socket_receive_suspension_list;

        /* Clear the thread receive suspension list.  */
        socket_ptr -> nx_udp_socket_receive_suspension_list =  NX_NULL;

        /* Walk through the queue list to resume any and all threads suspended
           on this queue.  */
        while (socket_ptr -> nx_udp_socket_receive_suspended_count)
        {

            /* Lockout interrupts.  */
            TX_DISABLE

            /* Clear the cleanup pointer, this prevents the timeout from doing
               anything.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

            /* Temporarily disable preemption again.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Set the return status in the thread to NX_SOCKET_UNBOUND.  */
            thread_ptr -> tx_thread_suspend_status =  NX_SOCKET_UNBOUND;

            /* Move the thread pointer ahead.  */
            thread_ptr =  thread_ptr -> tx_thread_suspended_next;

            /* Resume the thread.  */
            _tx_thread_system_resume(thread_ptr -> tx_thread_suspended_previous);

            /* Decrease the suspended count.  */
            socket_ptr -> nx_udp_socket_receive_suspended_count--;
        }
    }

    /* Disable interrupts again.  */
    TX_DISABLE

    /* Determine if there are any threads suspended on trying to bind to the
       same port.  */
    thread_ptr =  socket_ptr -> nx_udp_socket_bind_suspension_list;
    if (thread_ptr)
    {

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        if (thread_ptr == thread_ptr -> tx_thread_suspended_next)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            socket_ptr -> nx_udp_socket_bind_suspension_list =  NX_NULL;
        }
        else
        {

            /* At least one more thread is on the same expiration list.  */

            /* Update the list head pointer.  */
            socket_ptr -> nx_udp_socket_bind_suspension_list =  thread_ptr -> tx_thread_suspended_next;

            /* Update the links of the adjacent threads.  */
            (thread_ptr -> tx_thread_suspended_next) -> tx_thread_suspended_previous =
                thread_ptr -> tx_thread_suspended_previous;
            (thread_ptr -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                thread_ptr -> tx_thread_suspended_next;
        }

        /* Decrement the suspension count.  */
        socket_ptr -> nx_udp_socket_bind_suspended_count--;

        /* Pickup the new socket structure to link to the port list.  */
        new_socket_ptr =  (NX_UDP_SOCKET *)thread_ptr -> tx_thread_suspend_control_block;

        /* Clear the new socket's bind in progress flag.  */
        new_socket_ptr -> nx_udp_socket_bind_in_progress =  NX_NULL;

        /* Inherit the suspension list from the previously bound socket.  */
        new_socket_ptr -> nx_udp_socket_bind_suspension_list =
            socket_ptr -> nx_udp_socket_bind_suspension_list;
        socket_ptr -> nx_udp_socket_bind_suspension_list =  NX_NULL;

        /* Link the new socket to the bound list.  */
        if (ip_ptr -> nx_ip_udp_port_table[index])
        {

            /* There are already sockets on this list... just add this one
               to the end.  */
            new_socket_ptr -> nx_udp_socket_bound_next =
                ip_ptr -> nx_ip_udp_port_table[index];
            new_socket_ptr -> nx_udp_socket_bound_previous =
                (ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous;
            ((ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous) -> nx_udp_socket_bound_next =
                new_socket_ptr;
            (ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous =   new_socket_ptr;
        }
        else
        {

            /* Nothing is on the UDP port list.  Add this UDP socket to an
               empty list.  */
            new_socket_ptr -> nx_udp_socket_bound_next =      new_socket_ptr;
            new_socket_ptr -> nx_udp_socket_bound_previous =  new_socket_ptr;
            ip_ptr -> nx_ip_udp_port_table[index] =           new_socket_ptr;
        }

        /* Prepare for resumption of the first thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  NX_SUCCESS;

        /* Release the mutex protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the mutex protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return success to the caller.  */
        return(NX_SUCCESS);
    }

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return success.  */
    return(NX_SUCCESS);
}

