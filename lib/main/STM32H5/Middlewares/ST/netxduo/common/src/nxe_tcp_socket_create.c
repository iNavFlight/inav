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
#include "nx_ip.h"
#include "nx_tcp.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_socket_create                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP socket create            */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    socket_ptr                            Pointer to new TCP socket     */
/*    name                                  Name of new TCP socket        */
/*    type_of_service                       Type of service for this TCP  */
/*                                            socket                      */
/*    fragment                              Flag to enable IP fragmenting */
/*    time_to_live                          Time to live value for socket */
/*    window_size                           Size of socket's receive      */
/*                                            window                      */
/*    tcp_urgent_data_callback              Routine to call when urgent   */
/*                                            data is received            */
/*    tcp_disconnect_callback               Routine to call when a        */
/*                                            disconnect occurs           */
/*    tcp_socket_size                       Size of TCP socket            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_create                 Actual TCP socket create      */
/*                                            function                    */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nxe_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                             ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                             VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                             VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr),
                             UINT tcp_socket_size)
{

UINT           status;
NX_TCP_SOCKET *created_socket;
ULONG          created_count;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (socket_ptr == NX_NULL) ||
        (tcp_socket_size != (UINT)sizeof(NX_TCP_SOCKET)))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Get protection mutex.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Pickup created count and created socket pointer.  */
    created_count =   ip_ptr -> nx_ip_tcp_created_sockets_count;
    created_socket =  ip_ptr -> nx_ip_tcp_created_sockets_ptr;

    /* Loop to look for socket already created.  */
    while (created_count--)
    {

        /* Compare the new socket with the already created socket.  */
        if (socket_ptr == created_socket)
        {

            /* Error, socket already created!  */

            /* Release protection mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return error.  */
            return(NX_PTR_ERROR);
        }

        /* Move to next created socket.  */
        created_socket =  created_socket -> nx_tcp_socket_created_next;
    }

    /* Release protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Check to see if TCP is enabled.  */
    if (!ip_ptr -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for valid type of service.  */
    if (type_of_service & ~(NX_IP_TOS_MASK))
    {
        return(NX_OPTION_ERROR);
    }

    /* Check for valid fragment option.  */
    if ((fragment != NX_FRAGMENT_OKAY) &&
        (fragment != NX_DONT_FRAGMENT))
    {
        return(NX_OPTION_ERROR);
    }

    /* Check for valid time to live option.  */
    if (((ULONG)time_to_live) > NX_IP_TIME_TO_LIVE_MASK)
    {
        return(NX_OPTION_ERROR);
    }

    /* Check for valid window size.  */
    if (!window_size)
    {
        return(NX_OPTION_ERROR);
    }

#ifndef  NX_ENABLE_TCP_WINDOW_SCALING
    if (window_size > NX_LOWER_16_MASK)
    {
        return(NX_OPTION_ERROR);
    }
#else
    /* The maximum scale exponent is limited to 14. Section 2.2, RFC 7323. */
    if (window_size > ((1 << 30) - 1))
    {
        return(NX_OPTION_ERROR);
    }
#endif /* NX_ENABLE_TCP_WINDOW_SCALING  */

    /* Call actual TCP socket create function.  */
    status =  _nx_tcp_socket_create(ip_ptr, socket_ptr, name, type_of_service, fragment, time_to_live,
                                    window_size, tcp_urgent_data_callback, tcp_disconnect_callback);

    /* Return completion status.  */
    return(status);
}

