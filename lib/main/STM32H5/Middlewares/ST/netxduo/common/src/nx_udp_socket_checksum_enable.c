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
/*    _nx_udp_socket_checksum_enable                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the UDP checksum logic for the specified      */
/*    socket.  Note this function has no effect on UDP packets over IPv6  */
/*    network, since the UDP packet checksum must be computed for UDP     */
/*    over IPv6 network.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
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
UINT  _nx_udp_socket_checksum_enable(NX_UDP_SOCKET *socket_ptr)
{
TX_INTERRUPT_SAVE_AREA


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_CHECKSUM_ENABLE, socket_ptr -> nx_udp_socket_ip_ptr, socket_ptr, 0, 0, NX_TRACE_UDP_EVENTS, 0, 0);

    /* Lockout interrupts.  */
    TX_DISABLE

    /* Determine if the socket is currently bound.  */
    if (!socket_ptr ->  nx_udp_socket_bound_next)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Socket is not bound, return an error message.  */
        return(NX_NOT_BOUND);
    }

    /* Clear the checksum disable flag.  */
    socket_ptr -> nx_udp_socket_disable_checksum =  NX_FALSE;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return a successful status.  */
    return(NX_SUCCESS);
}

