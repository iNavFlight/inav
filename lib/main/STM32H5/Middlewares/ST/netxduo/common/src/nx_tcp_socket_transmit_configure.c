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
/*    _nx_tcp_socket_transmit_configure                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up various parameters associated with the        */
/*    socket's transmit operation.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    max_queue_depth                       Maximum number of transmit    */
/*                                            packets that can be queued  */
/*                                            for the socket              */
/*    timeout                               Number of timer ticks for the */
/*                                            initial transmit timeout    */
/*    max_retries                           Maximum number of retries     */
/*    timeout_shift                         Factor to be applied to       */
/*                                            subsequent timeouts, a      */
/*                                            value of 0 causes identical */
/*                                            subsequent timeouts         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release a protection mutex    */
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
UINT  _nx_tcp_socket_transmit_configure(NX_TCP_SOCKET *socket_ptr, ULONG max_queue_depth,
                                        ULONG timeout, ULONG max_retries, ULONG timeout_shift)
{

NX_IP *ip_ptr;


    /* Pickup the associated IP structure.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_TRANSMIT_CONFIGURE, ip_ptr, socket_ptr, max_queue_depth, timeout, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can initiate accept processing for this socket.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup the socket with the new transmit parameters.  */
    socket_ptr -> nx_tcp_socket_timeout_rate =                    timeout;
    socket_ptr -> nx_tcp_socket_timeout_max_retries =             max_retries;
    socket_ptr -> nx_tcp_socket_timeout_shift =                   timeout_shift;
    socket_ptr -> nx_tcp_socket_transmit_queue_maximum_default =  max_queue_depth;
    socket_ptr -> nx_tcp_socket_transmit_queue_maximum =          max_queue_depth;

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return completion status.  */
    return(NX_SUCCESS);
}

