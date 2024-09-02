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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_delete                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created socket and unbound       */
/*    socket.  If the socket is still bound, an error is returned.        */
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
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_udp_socket_delete(NX_UDP_SOCKET *socket_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_IP *ip_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_DELETE, ip_ptr, socket_ptr, 0, 0, NX_TRACE_UDP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can process the socket delete request.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket is still bound to port.  */
    if (socket_ptr -> nx_udp_socket_bound_next)
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a still bound error code.  */
        return(NX_STILL_BOUND);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Now, remove the UDP socket from the created socket list.  */

    /* Clear the socket ID to make it invalid.  */
    socket_ptr -> nx_udp_socket_id =  0;

    /* See if the socket is the only one on the list.  */
    if (socket_ptr == socket_ptr -> nx_udp_socket_created_next)
    {

        /* Only created socket, just set the created list to NULL.  */
        ip_ptr -> nx_ip_udp_created_sockets_ptr =  NX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        (socket_ptr -> nx_udp_socket_created_next) -> nx_udp_socket_created_previous =
            socket_ptr -> nx_udp_socket_created_previous;
        (socket_ptr -> nx_udp_socket_created_previous) -> nx_udp_socket_created_next =
            socket_ptr -> nx_udp_socket_created_next;

        /* See if we have to update the created list head pointer.  */
        if (ip_ptr -> nx_ip_udp_created_sockets_ptr == socket_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            ip_ptr -> nx_ip_udp_created_sockets_ptr =  socket_ptr -> nx_udp_socket_created_next;
        }
    }

    /* Decrease the created sockets count.  */
    ip_ptr -> nx_ip_udp_created_sockets_count--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* If trace is enabled, unregister this object.  */
    NX_TRACE_OBJECT_UNREGISTER(socket_ptr);

    /* Release the IP protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return success.  */
    return(NX_SUCCESS);
}

