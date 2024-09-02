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

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS
#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_socket_establishe_notify                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP socket establish notify  */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    tcp_establish_notify                  Routine to call when the      */
/*                                            connection handshake is     */
/*                                            completed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_establish_notify       Actual socket establish notify*/
/*                                            function                    */
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
UINT  _nxe_tcp_socket_establish_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_establish_notify)(NX_TCP_SOCKET *socket_ptr))
{
#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
UINT status;

    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID) || (tcp_establish_notify == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }


    /* Check to see if TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual socket establish notify function.  */
    status =  _nx_tcp_socket_establish_notify(socket_ptr, tcp_establish_notify);

    /* Return to caller.  */
    return(status);

#else /* !NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(tcp_establish_notify);

    return(NX_NOT_SUPPORTED);

#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */
}

