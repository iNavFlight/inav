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
#include "nx_ip.h"
#include "nx_udp.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_udp_socket_create                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the UDP socket create            */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    socket_ptr                            Pointer to new UDP socket     */
/*    name                                  Name of new UDP socket        */
/*    type_of_service                       Type of service for this UDP  */
/*                                            socket                      */
/*    fragment                              Flag to enable IP fragmenting */
/*    time_to_live                          Time to live value for socket */
/*    queue_maximum                         Maximum depth of receive queue*/
/*    udp_socket_size                       Size of UDP socket            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_udp_socket_create                 Actual UDP socket create      */
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
UINT  _nxe_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                             ULONG type_of_service, ULONG fragment, UINT time_to_live,
                             ULONG queue_maximum, UINT udp_socket_size)
{

UINT           status;
NX_UDP_SOCKET *created_socket;
ULONG          created_count;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (socket_ptr == NX_NULL) || (udp_socket_size != (UINT)sizeof(NX_UDP_SOCKET)))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Get protection mutex.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Pickup created count and created socket pointer.  */
    created_count =   ip_ptr -> nx_ip_udp_created_sockets_count;
    created_socket =  ip_ptr -> nx_ip_udp_created_sockets_ptr;

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
        created_socket =  created_socket -> nx_udp_socket_created_next;
    }

    /* Release protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Check to see if UDP is enabled.  */
    if (!ip_ptr -> nx_ip_udp_packet_receive)
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

    /* Call actual UDP socket create function.  */
    status =  _nx_udp_socket_create(ip_ptr, socket_ptr, name, type_of_service,
                                    fragment, time_to_live, queue_maximum);

    /* Return completion status.  */
    return(status);
}

