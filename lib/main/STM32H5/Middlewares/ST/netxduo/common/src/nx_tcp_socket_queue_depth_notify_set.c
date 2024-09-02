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
/*    _nx_tcp_socket_queue_depth_notify_set               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the transmit queue depth update callback to the  */
/*    function specified by the application, which is called whenever     */
/*    the specified socket determines that it has released packets from   */
/*    the transmit queue such that the queue depth is no longer exceeded. */
/*                                                                        */
/*    If a NULL pointer is supplied, the queue depth notify function is   */
/*    disabled.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    tcp_queue_depth_notify                Routine to call when NetX     */
/*                                            queue depth decreases below */
/*                                            the maximum transmit depth  */
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
UINT  _nx_tcp_socket_queue_depth_notify_set(NX_TCP_SOCKET *socket_ptr,  VOID (*tcp_socket_queue_depth_notify)(NX_TCP_SOCKET *socket_ptr))
{
#ifdef   NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY
TX_INTERRUPT_SAVE_AREA

    /* Get mutex protection.  */
    tx_mutex_get(&(socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the receive notify function pointer.  */
    socket_ptr -> nx_tcp_socket_queue_depth_notify =  tcp_socket_queue_depth_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(&(socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);

#else /* !NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY */
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(tcp_socket_queue_depth_notify);

    return(NX_NOT_SUPPORTED);

#endif /*   NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY      */
}

