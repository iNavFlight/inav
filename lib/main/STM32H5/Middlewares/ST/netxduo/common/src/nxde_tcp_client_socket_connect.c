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
/*    _nxde_tcp_client_socket_connect                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP client socket connect    */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    server_ip                             IP address of server          */
/*    server_port                           Port number of server         */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Acctual completion status     */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_NOT_ENABLED                        TCP not enabled               */
/*    NX_IP_ADDRESS_ERROR                   Invalid TCP server IP address */
/*    NX_INVALID_PORT                       Invalid TCP server port       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_tcp_client_socket_connect        Actual client socket connect  */
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
UINT  _nxde_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr,
                                      NXD_ADDRESS *server_ip,
                                      UINT server_port, ULONG wait_option)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Verify TCP is enabled.  */
    if (!(socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }


    /* Check for valid TCP server address. */
    if (server_ip == NX_NULL)
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check that the server IP address version is either IPv4 or IPv6. */
    if ((server_ip -> nxd_ip_version != NX_IP_VERSION_V4) &&
        (server_ip -> nxd_ip_version != NX_IP_VERSION_V6))
    {

        return(NX_IP_ADDRESS_ERROR);
    }

#ifndef NX_DISABLE_IPV4
    /* Check for a valid server IP address if the server_ip is version IPv4.  */
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        if (((server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_A_MASK) != NX_IP_CLASS_A_TYPE) &&
            ((server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_B_MASK) != NX_IP_CLASS_B_TYPE) &&
            ((server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_C_MASK) != NX_IP_CLASS_C_TYPE))
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

    /* Check for an invalid port.  */
    if (((ULONG)server_port) > (ULONG)NX_MAX_PORT)
    {
        return(NX_INVALID_PORT);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual TCP client socket connect function.  */
    status =  _nxd_tcp_client_socket_connect(socket_ptr, server_ip, server_port, wait_option);

    /* Return completion status.  */
    return(status);
}

