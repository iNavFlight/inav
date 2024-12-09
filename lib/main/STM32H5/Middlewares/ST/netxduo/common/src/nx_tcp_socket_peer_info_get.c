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
/*    _nx_tcp_socket_peer_info_get                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves IP address and port number of the peer      */
/*    connected to the specified TCP socket.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP sockete    */
/*    peer_ip_address                       Pointer to the IP address     */
/*                                             of the peer.               */
/*    peer_port                             Pointer to the port number    */
/*                                             of the peer.               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
/*    _nxd_tcp_socket_peer_info_get         Obtain TCP socket information */
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
UINT  _nx_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr,
                                   ULONG *peer_ip_address,
                                   ULONG *peer_port)
{
#ifndef NX_DISABLE_IPV4
UINT        status;
NXD_ADDRESS ip_address;

    status = _nxd_tcp_socket_peer_info_get(socket_ptr, &ip_address, peer_port);
    if (status == NX_SUCCESS)
    {

        /*lint -e{644} suppress variable might not be initialized, since "ip_address" was initialized as long as status is NX_SUCCESS. */
        *peer_ip_address = ip_address.nxd_ip_address.v4;
    }


    /* Return successful completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(socket_ptr);
    NX_PARAMETER_NOT_USED(peer_ip_address);
    NX_PARAMETER_NOT_USED(peer_port);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

