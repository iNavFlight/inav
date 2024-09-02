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

#ifdef NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY
/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS
#endif /* NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_socket_queue_depth_notify_set              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the error handling service for the queue depth     */
/*    notify callback setting function.                                   */
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
/*    _nx_tcp_socket_queue_depth_notify_set Actual callback set function  */
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
UINT  _nxe_tcp_socket_queue_depth_notify_set(NX_TCP_SOCKET *socket_ptr,  VOID (*tcp_socket_queue_depth_notify)(NX_TCP_SOCKET *socket_ptr))
{
#ifdef   NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY

UINT status;


    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for invalid input pointers.  */
    if (tcp_socket_queue_depth_notify == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check to see if TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Setup the receive notify function pointer.  */
    status  = _nx_tcp_socket_queue_depth_notify_set(socket_ptr,  tcp_socket_queue_depth_notify);

    /* Return successful completion.  */
    return(status);

#else /* !NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY */
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(tcp_socket_queue_depth_notify);

    return(NX_NOT_SUPPORTED);

#endif /*   NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY      */
}

