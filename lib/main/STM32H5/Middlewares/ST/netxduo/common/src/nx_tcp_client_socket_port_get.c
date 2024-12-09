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
/*    _nx_tcp_client_socket_port_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the bound port of the specified socket,     */
/*    which is especially useful in situations where NX_ANY_PORT was      */
/*    used to bind.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    port_ptr                              Destination for port bound to */
/*                                            this socket                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
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
UINT  _nx_tcp_client_socket_port_get(NX_TCP_SOCKET *socket_ptr, UINT *port_ptr)
{

NX_IP *ip_ptr;


    /* Setup IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_CLIENT_SOCKET_PORT_GET, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_port, 0, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if (!socket_ptr -> nx_tcp_socket_bound_next)
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a not bound error code.  */
        return(NX_NOT_BOUND);
    }

    /* Return the port number from the socket.  */
    *port_ptr =  socket_ptr -> nx_tcp_socket_port;

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion status.  */
    return(NX_SUCCESS);
}

