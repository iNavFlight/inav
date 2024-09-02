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
/** NetX Duo Component                                                    */
/**                                                                       */
/**   TELNET Client Protocol                                              */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nxd_telnet_client.h                                 PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Duo TELNET Protocol for Clients,         */ 
/*    including all data types and external references.                   */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included.                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_TELNET_CLIENT_H
#define NXD_TELNET_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the TELNET ID.  */

#define NX_TELNET_CLIENT_ID                        0x54454C4DUL



/* Define TELNET TCP socket create options.  */

#ifndef NX_TELNET_TOS
#define NX_TELNET_TOS                       NX_IP_NORMAL
#endif

#ifndef NX_TELNET_FRAGMENT_OPTION
#define NX_TELNET_FRAGMENT_OPTION           NX_DONT_FRAGMENT
#endif  


#ifndef NX_TELNET_TIME_TO_LIVE
#define NX_TELNET_TIME_TO_LIVE              0x80
#endif


/* Define the TELNET Server TCP port number.  */
#ifndef  NX_TELNET_SERVER_PORT
#define NX_TELNET_SERVER_PORT               23          /* Default Port for TELNET server                       */
#endif

/* Define return code constants.  */

#define NX_TELNET_ERROR                     0xF0        /* TELNET internal error                                */ 
#define NX_TELNET_TIMEOUT                   0xF1        /* TELNET timeout occurred                              */ 
#define NX_TELNET_FAILED                    0xF2        /* TELNET error                                         */ 
#define NX_TELNET_NOT_CONNECTED             0xF3        /* TELNET not connected error                           */ 
#define NX_TELNET_NOT_DISCONNECTED          0xF4        /* TELNET not disconnected error                        */ 
#define NX_TELNET_INVALID_PARAMETER         0xF5        /* Invalid non pointer input to Telnet function         */


/* Define the TELNET Client structure.  */

typedef struct NX_TELNET_CLIENT_STRUCT 
{
    ULONG           nx_telnet_client_id;                               /* TELNET Client ID                      */
    CHAR           *nx_telnet_client_name;                             /* Name of this TELNET client            */
    NX_IP          *nx_telnet_client_ip_ptr;                           /* Pointer to associated IP structure    */ 
    NX_TCP_SOCKET   nx_telnet_client_socket;                           /* Client TELNET socket                  */
} NX_TELNET_CLIENT;


#ifndef NX_TELNET_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nxd_telnet_client_connect                   _nxd_telnet_client_connect
#define nx_telnet_client_connect                    _nx_telnet_client_connect
#define nx_telnet_client_create                     _nx_telnet_client_create
#define nx_telnet_client_delete                     _nx_telnet_client_delete
#define nx_telnet_client_disconnect                 _nx_telnet_client_disconnect
#define nx_telnet_client_packet_receive             _nx_telnet_client_packet_receive
#define nx_telnet_client_packet_send                _nx_telnet_client_packet_send

#else

/* Services with error checking.  */

#define nxd_telnet_client_connect                   _nxde_telnet_client_connect
#define nx_telnet_client_connect                    _nxe_telnet_client_connect
#define nx_telnet_client_create                     _nxe_telnet_client_create
#define nx_telnet_client_delete                     _nxe_telnet_client_delete
#define nx_telnet_client_disconnect                 _nxe_telnet_client_disconnect
#define nx_telnet_client_packet_receive             _nxe_telnet_client_packet_receive
#define nx_telnet_client_packet_send                _nxe_telnet_client_packet_send
#define nx_telnet_server_create                     _nxe_telnet_server_create

#endif

/* Define the prototypes accessible to the application software.  */
UINT    nxd_telnet_client_connect(NX_TELNET_CLIENT *my_client, NXD_ADDRESS *server_ip_address, UINT server_port, ULONG wait_option);
UINT    nx_telnet_client_connect(NX_TELNET_CLIENT *client_ptr, ULONG server_ip_address, UINT server_port, ULONG wait_option);
UINT    nx_telnet_client_create(NX_TELNET_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, ULONG window_size);
UINT    nx_telnet_client_delete(NX_TELNET_CLIENT *client_ptr);
UINT    nx_telnet_client_disconnect(NX_TELNET_CLIENT *client_ptr, ULONG wait_option);
UINT    nx_telnet_client_packet_receive(NX_TELNET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT    nx_telnet_client_packet_send(NX_TELNET_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);


#else

/* TELNET source code is being compiled, do not perform any API mapping.  */
UINT    _nxde_telnet_client_connect(NX_TELNET_CLIENT *my_client, NXD_ADDRESS *server_ip_address, UINT server_port, ULONG wait_option);
UINT    _nxd_telnet_client_connect(NX_TELNET_CLIENT *my_client, NXD_ADDRESS *server_ip_address, UINT server_port, ULONG wait_option);

UINT    _nxe_telnet_client_connect(NX_TELNET_CLIENT *client_ptr, ULONG server_ip_address, UINT server_port, ULONG wait_option);
UINT    _nx_telnet_client_connect(NX_TELNET_CLIENT *client_ptr, ULONG server_ip_address, UINT server_port, ULONG wait_option);
UINT    _nxe_telnet_client_create(NX_TELNET_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, ULONG window_size);
UINT    _nx_telnet_client_create(NX_TELNET_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, ULONG window_size);
UINT    _nxe_telnet_client_delete(NX_TELNET_CLIENT *client_ptr);
UINT    _nx_telnet_client_delete(NX_TELNET_CLIENT *client_ptr);
UINT    _nxe_telnet_client_disconnect(NX_TELNET_CLIENT *client_ptr, ULONG wait_option);
UINT    _nx_telnet_client_disconnect(NX_TELNET_CLIENT *client_ptr, ULONG wait_option);
UINT    _nxe_telnet_client_packet_receive(NX_TELNET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT    _nx_telnet_client_packet_receive(NX_TELNET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT    _nxe_telnet_client_packet_send(NX_TELNET_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT    _nx_telnet_client_packet_send(NX_TELNET_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);


#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_TELNET_CLIENT_H */
