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

#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_driver_listen                 PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers a listen request to all TCP/IP offload      */
/*    interfaces.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  TCP port number               */
/*    socket_ptr                            Server socket pointer         */
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
/*    _nx_tcp_server_socket_listen          Register a listen request     */
/*    _nx_tcp_server_socket_relisten        Register a listen request     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
UINT _nx_tcp_server_socket_driver_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr)
{
UINT          status;
UINT          i;
NX_INTERFACE *interface_ptr;
    
    /* Loop all interfaces to listen to ones support TCP/IP offload.  */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {

        /* Use a local variable for convenience.  */
        interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

        /* Check for valid interfaces.  */
        if ((interface_ptr -> nx_interface_valid == NX_FALSE) ||
            (interface_ptr -> nx_interface_link_up == NX_FALSE))
        {

            /* Skip interface not valid.  */
            continue;
        }

        /* Check for TCP/IP offload feature.  */
        if (((interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) == 0) ||
            (interface_ptr -> nx_interface_tcpip_offload_handler == NX_NULL))
        {

            /* Skip interface not support TCP/IP offload.  */
            continue;
        }

        /* Let TCP/IP offload interface listen to port.  */
        status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                                     NX_TCPIP_OFFLOAD_TCP_SERVER_SOCKET_LISTEN,
                                                                     NX_NULL, NX_NULL, NX_NULL,
                                                                     port, NX_NULL, NX_NO_WAIT);
        if (status)
        {

            /* Return an already bound error code.  */
            return(NX_TCPIP_OFFLOAD_ERROR);
        }
    }

    return(NX_SUCCESS);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */