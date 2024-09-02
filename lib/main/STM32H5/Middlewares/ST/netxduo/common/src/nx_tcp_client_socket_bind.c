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
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_client_socket_bind                          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function binds the TCP socket structure to a specific TCP      */
/*    port.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    port                                  16-bit TCP port number        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_free_port_find                Find free TCP port            */
/*    _nx_tcp_socket_thread_suspend         Suspend thread                */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the random value, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_client_socket_bind(NX_TCP_SOCKET *socket_ptr, UINT port, ULONG wait_option)
{
TX_INTERRUPT_SAVE_AREA

UINT           index;
#ifdef NX_NAT_ENABLE
UINT           bound;
#endif /* NX_NAT_ENABLE */
NX_IP         *ip_ptr;
NX_TCP_SOCKET *search_ptr;
NX_TCP_SOCKET *end_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_CLIENT_SOCKET_BIND, ip_ptr, socket_ptr, port, wait_option, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can figure out whether or not the port has already
       been bound to.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if ((socket_ptr -> nx_tcp_socket_bound_next) ||
        (socket_ptr -> nx_tcp_socket_bind_in_progress))
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an already bound error code.  */
        return(NX_ALREADY_BOUND);
    }

    /* Determine if the port needs to be allocated.  */
    if (port == NX_ANY_PORT)
    {

        /* Call the find routine to allocate a TCP port.  */
        port = NX_SEARCH_PORT_START + (UINT)(((ULONG)NX_RAND()) % ((NX_MAX_PORT + 1) - NX_SEARCH_PORT_START));
        if (_nx_tcp_free_port_find(ip_ptr, port, &port) != NX_SUCCESS)
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

            /* Yes, so check the port by NAT handler. If NAT does not use this port, allow NetX to use it.  */
            bound = (ip_ptr -> nx_ip_nat_port_verify)(ip_ptr, NX_PROTOCOL_TCP, port);

            /* Check to see if the port has been used by NAT.  */
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

    /* Save the port number in the TCP socket structure.  */
    socket_ptr -> nx_tcp_socket_port =  port;

    /* Calculate the hash index in the TCP port array of the associated IP instance.  */
    index =  (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

    /* Pickup the head of the TCP ports bound list.  */
    search_ptr =  ip_ptr -> nx_ip_tcp_port_table[index];

    /* Determine if we need to perform a list search.  */
    if (search_ptr)
    {

        /* Walk through the circular list of TCP sockets that are already
           bound.  */
        end_ptr = search_ptr;
        do
        {

            /* Determine if this entry is the same as the requested port.  */
            if (search_ptr -> nx_tcp_socket_port == port)
            {

                /* Yes, the port has already been allocated.  */
                break;
            }

            /* Move to the next entry in the list.  */
            search_ptr =  search_ptr -> nx_tcp_socket_bound_next;
        } while (search_ptr != end_ptr);
    }

    /* Now determine if the port is available.  */
    if ((search_ptr == NX_NULL) || (search_ptr -> nx_tcp_socket_port != port))
    {

        /* Place this TCP socket structure on the list of bound ports.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Determine if the list is NULL.  */
        if (search_ptr)
        {

            /* There are already sockets on this list... just add this one
               to the end.  */
            socket_ptr -> nx_tcp_socket_bound_next =
                ip_ptr -> nx_ip_tcp_port_table[index];
            socket_ptr -> nx_tcp_socket_bound_previous =
                (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous;
            ((ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous) -> nx_tcp_socket_bound_next =
                socket_ptr;
            (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous =   socket_ptr;
        }
        else
        {

            /* Nothing is on the TCP port list.  Add this TCP socket to an
               empty list.  */
            socket_ptr -> nx_tcp_socket_bound_next =      socket_ptr;
            socket_ptr -> nx_tcp_socket_bound_previous =  socket_ptr;
            ip_ptr -> nx_ip_tcp_port_table[index] =       socket_ptr;
        }

        /* Restore interrupts.  */
        TX_RESTORE

        /* Release the mutex protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return success to the caller.  */
        return(NX_SUCCESS);
    }
    else if (wait_option)
    {

        /* Prepare for suspension of this thread.  */

        /* Increment the suspended thread count.  */
        search_ptr -> nx_tcp_socket_bind_suspended_count++;

        /* Set the socket bind in progress flag (thread pointer).  */
        socket_ptr -> nx_tcp_socket_bind_in_progress =  _tx_thread_current_ptr;

        /* Also remember the socket that has bound to the port, since the thread
           is going to be suspended on that socket.  */
        socket_ptr -> nx_tcp_socket_bound_previous =  search_ptr;

        /* Suspend the thread on this socket's connection attempt.  */
        _nx_tcp_socket_thread_suspend(&(search_ptr -> nx_tcp_socket_bind_suspension_list), _nx_tcp_client_bind_cleanup, socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);

        /* Return the completion status.  */
        return(_tx_thread_current_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return the port unavailable error.  */
        return(NX_PORT_UNAVAILABLE);
    }
}

