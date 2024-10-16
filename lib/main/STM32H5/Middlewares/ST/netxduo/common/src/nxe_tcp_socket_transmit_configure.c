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

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_socket_transmit_configure                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP socket transmit          */
/*    configure function call.                                            */
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
/*    _nx_tcp_socket_transmit_configure     Actual transmit configure     */
/*                                            service                     */
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
UINT  _nxe_tcp_socket_transmit_configure(NX_TCP_SOCKET *socket_ptr, ULONG max_queue_depth,
                                         ULONG timeout, ULONG max_retries, ULONG timeout_shift)
{

UINT status;


    /* Check for invalid input pointer.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for valid options.  */
    if (!max_queue_depth)
    {
        return(NX_OPTION_ERROR);
    }

    /* Check to see if TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual socket transmit configure service.  */
    status =  _nx_tcp_socket_transmit_configure(socket_ptr, max_queue_depth,
                                                timeout, max_retries, timeout_shift);

    /* Return completion status.  */
    return(status);
}

