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
/**   HTTP Proxy Protocol                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_http_proxy_client.h                              PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the Hypertext Transfer Protocol(HTTP) Proxy       */
/*    component, including all data types and external references.        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/

#ifndef NX_HTTP_PROXY_CLIENT_H
#define NX_HTTP_PROXY_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include    "nx_api.h"

/* Define the states for HTTP Proxy connection.  */
#define NX_HTTP_PROXY_STATE_INIT                0
#define NX_HTTP_PROXY_STATE_WAITING             1
#define NX_HTTP_PROXY_STATE_CONNECTING          2
#define NX_HTTP_PROXY_STATE_CONNECTED           3

/* Define HTTP Proxy function prototypes.  */
UINT _nx_http_proxy_client_enable(NX_IP *ip_ptr, NXD_ADDRESS *proxy_server_ip, UINT proxy_server_port,
                                  UCHAR *username, UINT username_length, UCHAR *password, UINT password_length);
VOID _nx_http_proxy_client_initialize(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS **server_ip, UINT *server_port);
UINT _nx_http_proxy_client_connect(NX_TCP_SOCKET *socket_ptr);
UINT _nx_http_proxy_client_connect_response_process(NX_TCP_SOCKET *socket_ptr);
VOID _nx_http_proxy_client_cleanup(NX_TCP_SOCKET *socket_ptr);

/* Define error checking shells for API services.  These are only referenced by the
   application.  */
UINT _nxe_http_proxy_client_enable(NX_IP *ip_ptr, NXD_ADDRESS *proxy_server_ip, UINT proxy_server_port,
                                   UCHAR *username, UINT username_length, UCHAR *password, UINT password_length);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NX_HTTP_PROXY_CLIENT_H */