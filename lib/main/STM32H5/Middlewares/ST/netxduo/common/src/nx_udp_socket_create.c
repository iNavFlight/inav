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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_create                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a UDP socket for the specified IP instance.   */
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
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG queue_maximum)
{
TX_INTERRUPT_SAVE_AREA

NX_UDP_SOCKET *tail_ptr;


    /* Initialize the TCP control block to zero.  */
    memset((void *)socket_ptr, 0, sizeof(NX_UDP_SOCKET));

    /* Fill in the basic information in the new UDP socket structure.  */

    /* Remember the associated IP structure.  */
    socket_ptr -> nx_udp_socket_ip_ptr =  ip_ptr;

    /* Save the UDP socket's name.  */
    socket_ptr -> nx_udp_socket_name =  name;

    /* Save the type of service input parameter.  */
    socket_ptr -> nx_udp_socket_type_of_service =  type_of_service;

    /* Save the fragment input parameter.  */
    socket_ptr -> nx_udp_socket_fragment_enable =  fragment & NX_DONT_FRAGMENT;

    /* Save the time-to-live input parameter.  */
    socket_ptr -> nx_udp_socket_time_to_live =  time_to_live;

    /* By default, have UDP checksum logic enabled.  To disable checksum logic, the
       application must call the nx_udp_checksum disable function for this UDP socket.  */
    socket_ptr -> nx_udp_socket_disable_checksum =  NX_FALSE;

    /* Clear the socket bind in progress flag.  */
    socket_ptr -> nx_udp_socket_bind_in_progress =  NX_FALSE;

    /* Set various list pointers to NULL.  */
    socket_ptr -> nx_udp_socket_bound_next =            NX_NULL;
    socket_ptr -> nx_udp_socket_bound_previous =        NX_NULL;
    socket_ptr -> nx_udp_socket_bind_suspension_list =  NX_NULL;
    socket_ptr -> nx_udp_socket_bind_suspended_count =  0;

    /* Initialize the receive queue parameters.  */
    socket_ptr -> nx_udp_socket_receive_count =         0;
    socket_ptr -> nx_udp_socket_queue_maximum =         queue_maximum;
    socket_ptr -> nx_udp_socket_receive_head =          NX_NULL;
    socket_ptr -> nx_udp_socket_receive_tail =          NX_NULL;

    /* Clear the receive notify function pointer.  */
    socket_ptr -> nx_udp_receive_callback =             NX_NULL;

    /* If trace is enabled, register this object.  */
    NX_TRACE_OBJECT_REGISTER(NX_TRACE_OBJECT_TYPE_UDP_SOCKET, socket_ptr, name, type_of_service, queue_maximum);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_CREATE, ip_ptr, socket_ptr, type_of_service, queue_maximum, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can add socket to IP structure.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts while we link the new UDP socket to the IP structure.  */
    TX_DISABLE

    /* Load the UDP ID field in the UDP control block.  */
    socket_ptr -> nx_udp_socket_id =  NX_UDP_ID;

    /* Place the new UDP control block on the list of created UDP sockets for this IP.  First,
       check for an empty list.  */
    if (ip_ptr -> nx_ip_udp_created_sockets_ptr)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  (ip_ptr -> nx_ip_udp_created_sockets_ptr) -> nx_udp_socket_created_previous;

        /* Place the new UDP socket control block in the list.  */
        (ip_ptr -> nx_ip_udp_created_sockets_ptr) -> nx_udp_socket_created_previous =  socket_ptr;
        tail_ptr ->  nx_udp_socket_created_next =  socket_ptr;

        /* Setup this UDP socket's created links.  */
        socket_ptr -> nx_udp_socket_created_previous =  tail_ptr;
        socket_ptr -> nx_udp_socket_created_next =      ip_ptr -> nx_ip_udp_created_sockets_ptr;
    }
    else
    {

        /* The created UDP socket list is empty.  Add UDP socket control block to empty list.  */
        ip_ptr -> nx_ip_udp_created_sockets_ptr =       socket_ptr;
        socket_ptr -> nx_udp_socket_created_previous =  socket_ptr;
        socket_ptr -> nx_udp_socket_created_next =      socket_ptr;
    }

    /* Increment the created UDP socket counter.  */
    ip_ptr -> nx_ip_udp_created_sockets_count++;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Release the IP protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

