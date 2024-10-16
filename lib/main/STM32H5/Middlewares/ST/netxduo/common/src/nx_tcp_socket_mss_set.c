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
#include "nx_ip.h"
#include "nx_tcp.h"
#include "nx_ipv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_mss_set                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the TCP Maximum Segment Size (MSS) for the       */
/*    specified TCP socket.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP socket     */
/*    mss                                   New value for the MSS         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
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
UINT  _nx_tcp_socket_mss_set(NX_TCP_SOCKET *socket_ptr, ULONG mss)
{

NX_IP *ip_ptr;
UINT status;

    /* Setup IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    if (mss == 0)
    {

        /* Invalid MSS, return an error.  */
        return(NX_SIZE_ERROR);
    }

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (socket_ptr -> nx_tcp_socket_state < NX_TCP_SYN_SENT)
    {
        
        /* Set the socket's MSS value.  */
        socket_ptr -> nx_tcp_socket_mss =  mss;
        status = NX_SUCCESS;
    }
    else
    {

        /* MSS can not be modified when connection is started. */
        status = NX_NOT_SUCCESSFUL;
    }

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion status.  */
    return(status);
}

