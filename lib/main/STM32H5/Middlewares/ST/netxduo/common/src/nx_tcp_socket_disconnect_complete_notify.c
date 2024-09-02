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
/*    _nx_tcp_socket_disconnect_complete_notify           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the disconnect complete notify function pointer  */
/*    with the function specified by the application, which is called     */
/*    when disconnect operation is complete. If a NULL pointer is         */
/*    supplied, the disconnect complete notify feature is disabled.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    tcp_disconnect_complete_notify        Routine to call when the      */
/*                                            disconnect is complete      */
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
UINT  _nx_tcp_socket_disconnect_complete_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_disconnect_complete_notify)(NX_TCP_SOCKET *socket_ptr))
{
#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the establish notify function pointer.  */
    socket_ptr -> nx_tcp_disconnect_complete_notify =  tcp_disconnect_complete_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return successful completion.  */
    return(NX_SUCCESS);

#else /* !NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(tcp_disconnect_complete_notify);

    return(NX_NOT_SUPPORTED);

#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */
}

