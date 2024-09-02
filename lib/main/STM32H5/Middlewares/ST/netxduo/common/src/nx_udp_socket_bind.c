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
#include "tx_thread.h"


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_driver_bind                          PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function binds to a UDP port through TCP/IP offload interface. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    port                                  16-bit UDP port number        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_udp_socket_unbind                 Unbind UDP port               */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
static UINT _nx_udp_socket_driver_bind(NX_UDP_SOCKET *socket_ptr, UINT port, ULONG wait_option)
{
UINT          status = NX_SUCCESS;
UINT          i;
NX_INTERFACE *interface_ptr;
NX_IP        *ip_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* Obtain the IP mutex.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Loop all interfaces to bind to ones support TCP/IP offload.  */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {

        /* Use a local variable for convenience.  */
        interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

        /* Check for valid interfaces.  */
        if ((interface_ptr -> nx_interface_valid == NX_FALSE) ||
            (interface_ptr -> nx_interface_link_up == NX_FALSE))
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

        /* Let TCP/IP offload interface bind to port.  */
        status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr,
                                                                     socket_ptr,
                                                                     NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND,
                                                                     NX_NULL, NX_NULL, NX_NULL, port,
                                                                     NX_NULL, wait_option);
        if (status)
        {

            /* At least one of the interface fails to bind to port.  */
            _nx_udp_socket_unbind(socket_ptr);
            status = NX_TCPIP_OFFLOAD_ERROR;
            break;
        }
    }

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return(status);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_bind                                 PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function binds the UDP socket a specific UDP port.  If the     */
/*    requested port number is currently bound to another socket,         */
/*    this function waits for specified period of time (wait_option)      */
/*    for the other socket to unbind the port number.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    port                                  16-bit UDP port number        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_udp_free_port_find                Find a free UDP port          */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _tx_thread_system_suspend             Suspend thread                */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_udp_socket_bind(NX_UDP_SOCKET *socket_ptr, UINT  port, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA
UINT           index;
#ifdef NX_NAT_ENABLE
UINT           bound;
#endif /* NX_NAT_ENABLE */
NX_IP         *ip_ptr;
TX_THREAD     *thread_ptr;
NX_UDP_SOCKET *search_ptr;
NX_UDP_SOCKET *end_ptr;
UINT           status = NX_SUCCESS;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_BIND, ip_ptr, socket_ptr, port, wait_option, NX_TRACE_UDP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can figure out whether or not the port has already
       been bound to.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if ((socket_ptr -> nx_udp_socket_bound_next) ||
        (socket_ptr -> nx_udp_socket_bind_in_progress))
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an already bound error code.  */
        return(NX_ALREADY_BOUND);
    }

    /* Determine if the port needs to be allocated.  */
    if (port == NX_ANY_PORT)
    {

        /* Call the find routine to allocate a UDP port.  */
        port = NX_SEARCH_PORT_START + (UINT)(((ULONG)NX_RAND()) % ((NX_MAX_PORT + 1) - NX_SEARCH_PORT_START));
        if (_nx_udp_free_port_find(ip_ptr, port, &port) != NX_SUCCESS)
        {

            /* Release the protection mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* There was no free port, return an error code.  */
            return(NX_NO_FREE_PORTS);
        }
    }
#ifdef NX_NAT_ENABLE
    else
    {

        /* Check if this IP interface has a NAT service. */
        if (ip_ptr -> nx_ip_nat_port_verify)
        {

            /* Yes, so check the port by NAT handler.  */
            bound = (ip_ptr -> nx_ip_nat_port_verify)(ip_ptr, NX_PROTOCOL_UDP, port);

            /* Check to see if the port has been bound by NAT.  */
            if (bound == NX_TRUE)
            {

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Return the port unavailable error.  */
                return(NX_PORT_UNAVAILABLE);
            }
        }
    }
#endif

    /* Save the port number in the UDP socket structure.  */
    socket_ptr -> nx_udp_socket_port =  port;

    /* Calculate the hash index in the UDP port array of the associated IP instance.  */
    index =  (UINT)((port + (port >> 8)) & NX_UDP_PORT_TABLE_MASK);

    /* Pickup the head of the UDP ports bound list.  */
    search_ptr =  ip_ptr -> nx_ip_udp_port_table[index];

    /* Determine if we need to perform a list search.  */
    if (search_ptr)
    {

        /* Walk through the circular list of UDP sockets that are already
           bound.  */
        end_ptr = search_ptr;
        do
        {

            /* Determine if this entry is the same as the requested port.  */
            if (search_ptr -> nx_udp_socket_port == port)
            {

                /* Yes, the port has already been allocated.  */
                break;
            }

            /* Move to the next entry in the list.  */
            search_ptr =  search_ptr -> nx_udp_socket_bound_next;
        } while (search_ptr != end_ptr);
    }

    /* Now determine if the port is available.  */
    if ((search_ptr == NX_NULL) || (search_ptr -> nx_udp_socket_port != port))
    {

        /* Place this UDP socket structure on the list of bound ports.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Determine if the list is NULL.  */
        if (search_ptr)
        {

            /* There are already sockets on this list... just add this one
               to the end.  */
            socket_ptr -> nx_udp_socket_bound_next =       ip_ptr -> nx_ip_udp_port_table[index];
            socket_ptr -> nx_udp_socket_bound_previous =   (ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous;
            ((ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous) -> nx_udp_socket_bound_next = socket_ptr;
            (ip_ptr -> nx_ip_udp_port_table[index]) -> nx_udp_socket_bound_previous =   socket_ptr;
        }
        else
        {

            /* Nothing is on the UDP port list.  Add this UDP socket to an
               empty list.  */
            socket_ptr -> nx_udp_socket_bound_next =      socket_ptr;
            socket_ptr -> nx_udp_socket_bound_previous =  socket_ptr;
            ip_ptr -> nx_ip_udp_port_table[index] =       socket_ptr;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the mutex protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
    }
    else if (wait_option)
    {

        /* Prepare for suspension of this thread.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Pickup thread pointer.  */
        thread_ptr =  _tx_thread_current_ptr;

        /* Setup cleanup routine pointer.  */
        thread_ptr -> tx_thread_suspend_cleanup =  _nx_udp_bind_cleanup;

        /* Setup cleanup information, i.e. this socket control
           block.  */
        thread_ptr -> tx_thread_suspend_control_block =  (void *)socket_ptr;

        /* Also remember the socket that has bound to the port, since the thread
           is going to be suspended on that socket.  */
        socket_ptr -> nx_udp_socket_bound_previous =  search_ptr;

        /* Set the socket bind in progress flag (thread pointer).  */
        socket_ptr -> nx_udp_socket_bind_in_progress =  thread_ptr;

        /* Setup suspension list.  */
        if (search_ptr -> nx_udp_socket_bind_suspension_list)
        {

            /* This list is not NULL, add current thread to the end. */
            thread_ptr -> tx_thread_suspended_next =       search_ptr -> nx_udp_socket_bind_suspension_list;
            thread_ptr -> tx_thread_suspended_previous =  (search_ptr -> nx_udp_socket_bind_suspension_list) -> tx_thread_suspended_previous;
            ((search_ptr -> nx_udp_socket_bind_suspension_list) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =  thread_ptr;
            (search_ptr -> nx_udp_socket_bind_suspension_list) -> tx_thread_suspended_previous =   thread_ptr;
        }
        else
        {

            /* No other threads are suspended.  Setup the head pointer and
               just setup this threads pointers to itself.  */
            search_ptr -> nx_udp_socket_bind_suspension_list =         thread_ptr;
            thread_ptr -> tx_thread_suspended_next =                   thread_ptr;
            thread_ptr -> tx_thread_suspended_previous =               thread_ptr;
        }

        /* Increment the suspended thread count.  */
        search_ptr -> nx_udp_socket_bind_suspended_count++;

        /* Set the state to suspended.  */
        thread_ptr -> tx_thread_state =  TX_TCP_IP;

        /* Set the suspending flag.  */
        thread_ptr -> tx_thread_suspending =  TX_TRUE;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Save the timeout value.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the mutex protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Call actual thread suspension routine.  */
        _tx_thread_system_suspend(thread_ptr);

        /* Return the completion status.  */
        status = thread_ptr -> tx_thread_suspend_status;
    }
    else
    {

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return the port unavailable error.  */
        status = NX_PORT_UNAVAILABLE;
    }

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    if (status == NX_SUCCESS)
    {

        /* Bind to TCP/IP offload interface.  */
        status = _nx_udp_socket_driver_bind(socket_ptr, port, wait_option);
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Return success to the caller.  */
    return(status);
}

