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
/*    _nx_tcp_socket_receive_notify                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the receive notify function pointer to the       */
/*    function specified by the application, which is called whenever     */
/*    one or more receive packets is available for the socket.  If a      */
/*    NULL pointer is supplied, the receive notify function is disabled.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    tcp_receive_notify                    Routine to call when one or   */
/*                                            receive packets are         */
/*                                            available for the socket    */
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
UINT  _nx_tcp_socket_receive_notify(NX_TCP_SOCKET *socket_ptr,
                                    VOID (*tcp_receive_notify)(NX_TCP_SOCKET *socket_ptr))
{
TX_INTERRUPT_SAVE_AREA

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_RECEIVE_NOTIFY, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, tcp_receive_notify, 0, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Get mutex protection.  */
    tx_mutex_get(&(socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the receive notify function pointer.  */
    socket_ptr -> nx_tcp_receive_callback =  tcp_receive_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(&(socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

