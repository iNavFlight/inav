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
/** NetX Web Component                                                    */
/**                                                                       */
/**   Hypertext Transfer Protocol (HTTP)                                  */
/**   Hypertext Transfer Protocol Secure (HTTPS using TLS)                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_web_http_client.h                                PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Web Hypertext Transfer Protocol (HTTP)   */
/*    component, including all data types and external references.        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included, along with fx_api.h and fx_port.h.                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            parsing base64,             */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_WEB_HTTP_CLIENT_H
#define NX_WEB_HTTP_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include    "nx_api.h"

/* Include common HTTP definitions. */
#include "nx_web_http_common.h"

/* Define the HTTP ID.  */
#define NX_WEB_HTTP_CLIENT_ID                  0x48545450UL

#ifndef NX_WEB_HTTP_CLIENT_TIMEOUT
#define NX_WEB_HTTP_CLIENT_TIMEOUT              (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_CLIENT_MIN_PACKET_SIZE
#define NX_WEB_HTTP_CLIENT_MIN_PACKET_SIZE      600
#endif

/* Define HTTP Client states.  */

#define NX_WEB_HTTP_CLIENT_STATE_READY          1
#define NX_WEB_HTTP_CLIENT_STATE_GET            2
#define NX_WEB_HTTP_CLIENT_STATE_PUT            3
#define NX_WEB_HTTP_CLIENT_STATE_POST           4
#define NX_WEB_HTTP_CLIENT_STATE_HEAD           5
#define NX_WEB_HTTP_CLIENT_STATE_DELETE         6

#ifdef  NX_WEB_HTTP_DIGEST_ENABLE

/* Include the MD5 digest header file.  */

#include "nx_md5.h"

#endif


#ifdef NX_WEB_HTTPS_ENABLE
#include "nx_secure_tls_api.h"
#endif

/* Define the */
typedef struct NX_WEB_HTTP_CLIENT_STATUS_MAP_STRUCT
{
    CHAR           *nx_web_http_client_status_string;              /* String of status    */
    UINT           nx_web_http_client_status_code;                 /* Status code         */
} NX_WEB_HTTP_CLIENT_STATUS_MAP;

/* Define the HTTP Client data structure.  */

typedef struct NX_WEB_HTTP_CLIENT_STRUCT
{
    ULONG           nx_web_http_client_id;                              /* HTTP Server ID                       */
    CHAR           *nx_web_http_client_name;                            /* Name of this HTTP Client             */
    UINT            nx_web_http_client_state;                           /* Current state of HTTP Client         */
    UINT            nx_web_http_client_connect_port;                    /* Client port to connect to the server */
    NX_IP          *nx_web_http_client_ip_ptr;                          /* Pointer to associated IP structure   */
    NX_PACKET_POOL *nx_web_http_client_packet_pool_ptr;                 /* Pointer to HTTP Client packet pool   */
    ULONG           nx_web_http_client_total_transfer_bytes;            /* Total number of bytes to transfer    */
    ULONG           nx_web_http_client_actual_bytes_transferred;        /* Number of bytes actually transferred */
    ULONG           nx_web_http_client_total_receive_bytes;             /* Total number of bytes to receive     */
    ULONG           nx_web_http_client_actual_bytes_received;           /* Number of bytes actually received    */
    NX_TCP_SOCKET   nx_web_http_client_socket;                          /* HTTP Client TCP socket               */
    NXD_ADDRESS     nx_web_http_client_server_address;                  /* IP address of remote server          */
    NX_PACKET      *nx_web_http_client_request_packet_ptr;              /* Pointer to current request packet    */
    UINT            nx_web_http_client_method;                          /* Current method (e.g. GET, PUT)       */
#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    UCHAR           nx_web_http_client_keep_alive;                      /* Flag of keep alive                   */
#else
    UCHAR           nx_web_http_client_reserved;                        /* Reserved                             */
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */
    UCHAR           nx_web_http_client_response_header_received;        /* Flag of response header received     */
    UCHAR           nx_web_http_client_request_chunked;                 /* Flag for chunked request             */
    UCHAR           nx_web_http_client_response_chunked;                /* Flag for chunked response            */
    UINT            nx_web_http_client_chunked_response_remaining_size; /* Remaining size of the response       */
    NX_PACKET      *nx_web_http_client_response_packet;                 /* Pointer to the received response     */
    /* Pointer to application callback invoked for each field in the response header. */
    VOID          (*nx_web_http_client_response_callback)(struct NX_WEB_HTTP_CLIENT_STRUCT *client_ptr,
                                                          CHAR *field_name, UINT field_name_length,
                                                          CHAR *field_value, UINT field_value_length);
#ifdef NX_WEB_HTTPS_ENABLE
    UINT                  nx_web_http_client_is_https;                  /* If using TLS for HTTPS, set to true. */
    NX_SECURE_TLS_SESSION nx_web_http_client_tls_session;                      /* TLS session for HTTPS.               */
#endif
#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
    NX_MD5          nx_web_http_client_md5data;                         /* HTTP Client MD5 work area            */
#endif
} NX_WEB_HTTP_CLIENT;



#ifndef NX_WEB_HTTP_CLIENT_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_web_http_client_create                       _nx_web_http_client_create
#define nx_web_http_client_delete                       _nx_web_http_client_delete
#define nx_web_http_client_get_start                    _nx_web_http_client_get_start
#define nx_web_http_client_get_start_extended           _nx_web_http_client_get_start_extended
#define nx_web_http_client_put_start                    _nx_web_http_client_put_start
#define nx_web_http_client_put_start_extended           _nx_web_http_client_put_start_extended
#define nx_web_http_client_post_start                   _nx_web_http_client_post_start
#define nx_web_http_client_post_start_extended          _nx_web_http_client_post_start_extended
#define nx_web_http_client_head_start                   _nx_web_http_client_head_start
#define nx_web_http_client_head_start_extended          _nx_web_http_client_head_start_extended
#define nx_web_http_client_delete_start                 _nx_web_http_client_delete_start
#define nx_web_http_client_delete_start_extended        _nx_web_http_client_delete_start_extended
#define nx_web_http_client_get_secure_start             _nx_web_http_client_get_secure_start
#define nx_web_http_client_get_secure_start_extended    _nx_web_http_client_get_secure_start_extended
#define nx_web_http_client_put_secure_start             _nx_web_http_client_put_secure_start
#define nx_web_http_client_put_secure_start_extended    _nx_web_http_client_put_secure_start_extended
#define nx_web_http_client_post_secure_start            _nx_web_http_client_post_secure_start
#define nx_web_http_client_post_secure_start_extended   _nx_web_http_client_post_secure_start_extended
#define nx_web_http_client_head_secure_start            _nx_web_http_client_head_secure_start
#define nx_web_http_client_head_secure_start_extended   _nx_web_http_client_head_secure_start_extended
#define nx_web_http_client_delete_secure_start          _nx_web_http_client_delete_secure_start
#define nx_web_http_client_delete_secure_start_extended _nx_web_http_client_delete_secure_start_extended
#define nx_web_http_client_response_body_get            _nx_web_http_client_response_body_get
#define nx_web_http_client_put_packet                   _nx_web_http_client_put_packet
#define nx_web_http_client_response_header_callback_set _nx_web_http_client_response_header_callback_set
#define nx_web_http_client_request_initialize           _nx_web_http_client_request_initialize
#define nx_web_http_client_request_initialize_extended  _nx_web_http_client_request_initialize_extended
#define nx_web_http_client_request_send                 _nx_web_http_client_request_send
#define nx_web_http_client_request_header_add           _nx_web_http_client_request_header_add
#define nx_web_http_client_connect                      _nx_web_http_client_connect
#define nx_web_http_client_secure_connect               _nx_web_http_client_secure_connect
#define nx_web_http_client_request_packet_allocate      _nx_web_http_client_request_packet_allocate
#define nx_web_http_client_request_packet_send          _nx_web_http_client_request_packet_send
#define nx_web_http_client_request_chunked_set          _nx_web_http_client_request_chunked_set

#else

/* Services with error checking.  */

#define nx_web_http_client_create(p,n,i,pp,w)           _nxe_web_http_client_create(p,n,i,pp,w, sizeof(NX_WEB_HTTP_CLIENT))
#define nx_web_http_client_delete                       _nxe_web_http_client_delete
#define nx_web_http_client_get_start                    _nxe_web_http_client_get_start
#define nx_web_http_client_get_start_extended           _nxe_web_http_client_get_start_extended
#define nx_web_http_client_put_start                    _nxe_web_http_client_put_start
#define nx_web_http_client_put_start_extended           _nxe_web_http_client_put_start_extended
#define nx_web_http_client_post_start                   _nxe_web_http_client_post_start
#define nx_web_http_client_post_start_extended          _nx_web_http_client_post_start_extended
#define nx_web_http_client_head_start                   _nxe_web_http_client_head_start
#define nx_web_http_client_head_start_extended          _nxe_web_http_client_head_start_extended
#define nx_web_http_client_delete_start                 _nxe_web_http_client_delete_start
#define nx_web_http_client_delete_start_extended        _nxe_web_http_client_delete_start_extended
#define nx_web_http_client_get_secure_start             _nxe_web_http_client_get_secure_start
#define nx_web_http_client_get_secure_start_extended    _nxe_web_http_client_get_secure_start_extended
#define nx_web_http_client_put_secure_start             _nxe_web_http_client_put_secure_start
#define nx_web_http_client_put_secure_start_extended    _nxe_web_http_client_put_secure_start_extended
#define nx_web_http_client_post_secure_start            _nxe_web_http_client_post_secure_start
#define nx_web_http_client_post_secure_start_extended   _nxe_web_http_client_post_secure_start_extended
#define nx_web_http_client_head_secure_start            _nxe_web_http_client_head_secure_start
#define nx_web_http_client_head_secure_start_extended   _nxe_web_http_client_head_secure_start_extended
#define nx_web_http_client_delete_secure_start          _nxe_web_http_client_delete_secure_start
#define nx_web_http_client_delete_secure_start_extended _nxe_web_http_client_delete_secure_start_extended
#define nx_web_http_client_response_body_get            _nxe_web_http_client_response_body_get
#define nx_web_http_client_put_packet                   _nxe_web_http_client_put_packet
#define nx_web_http_client_response_header_callback_set _nxe_web_http_client_response_header_callback_set
#define nx_web_http_client_request_initialize           _nxe_web_http_client_request_initialize
#define nx_web_http_client_request_initialize_extended  _nxe_web_http_client_request_initialize_extended
#define nx_web_http_client_request_send                 _nxe_web_http_client_request_send
#define nx_web_http_client_request_header_add           _nxe_web_http_client_request_header_add
#define nx_web_http_client_connect                      _nxe_web_http_client_connect
#define nx_web_http_client_secure_connect               _nxe_web_http_client_secure_connect
#define nx_web_http_client_request_packet_allocate      _nxe_web_http_client_request_packet_allocate
#define nx_web_http_client_request_packet_send          _nxe_web_http_client_request_packet_send
#define nx_web_http_client_request_chunked_set          _nxe_web_http_client_request_chunked_set

#endif  /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */


#ifdef NX_DISABLE_ERROR_CHECKING
UINT        _nx_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size);
#else
UINT        _nxe_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size, UINT http_client_size);
#endif  /* NX_DISABLE_ERROR_CHECKING */


UINT        nx_web_http_client_delete(NX_WEB_HTTP_CLIENT *client_ptr);
UINT        nx_web_http_client_response_body_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        nx_web_http_client_put_packet(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        nx_web_http_client_get_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        nx_web_http_client_get_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                  CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG wait_option);
UINT        nx_web_http_client_put_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        nx_web_http_client_put_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                  CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option);
UINT        nx_web_http_client_post_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        nx_web_http_client_post_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                   CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option);
UINT        nx_web_http_client_head_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        nx_web_http_client_head_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                   CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG wait_option);
UINT        nx_web_http_client_delete_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        nx_web_http_client_delete_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                     CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG wait_option);

UINT        nx_web_http_client_response_header_callback_set(NX_WEB_HTTP_CLIENT *client_ptr,
                                                            VOID (*callback_function)(NX_WEB_HTTP_CLIENT *client_ptr,
                                                            CHAR *field_name,
                                                            UINT field_name_length,
                                                            CHAR *field_value,
                                                            UINT field_value_length));

UINT        nx_web_http_client_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option);

UINT        nx_web_http_client_request_initialize(NX_WEB_HTTP_CLIENT *client_ptr,
                                                  UINT method,
                                                  CHAR *resource,
                                                  CHAR *host,
                                                  UINT input_size,
                                                  UINT transfer_encoding_chunked,
                                                  CHAR *username,
                                                  CHAR *password,
                                                  UINT wait_option);

UINT        nx_web_http_client_request_initialize_extended(NX_WEB_HTTP_CLIENT *client_ptr,
                                                           UINT method,
                                                           CHAR *resource,
                                                           UINT resource_length,
                                                           CHAR *host,
                                                           UINT host_length,
                                                           UINT input_size,
                                                           UINT transfer_encoding_chunked,
                                                           CHAR *username,
                                                           UINT username_length,
                                                           CHAR *password,
                                                           UINT password_length,
                                                           UINT wait_option);

UINT        nx_web_http_client_request_packet_allocate(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr,
                                                       UINT wait_option);

UINT        nx_web_http_client_request_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT name_length,
                                                  CHAR *field_value, UINT value_length, UINT wait_option);

UINT        nx_web_http_client_request_send(NX_WEB_HTTP_CLIENT *client_ptr, ULONG wait_option);

UINT        nx_web_http_client_request_packet_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT more_data, ULONG wait_option);

UINT        nx_web_http_client_request_chunked_set(NX_WEB_HTTP_CLIENT *client_ptr, UINT chunk_size, NX_PACKET *packet_ptr);

#ifdef NX_WEB_HTTPS_ENABLE

UINT        nx_web_http_client_secure_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                              UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                              ULONG wait_option);

UINT        nx_web_http_client_get_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        nx_web_http_client_get_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                         CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                         CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                         UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                         ULONG wait_option);
UINT        nx_web_http_client_put_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        nx_web_http_client_put_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                         CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                         CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                         UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                         ULONG wait_option);
UINT        nx_web_http_client_post_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                 CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                 UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                 ULONG wait_option);
UINT        nx_web_http_client_post_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                          CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                          CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                          UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                          ULONG wait_option);
UINT        nx_web_http_client_head_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                 CHAR *host, CHAR *username, CHAR *password,
                                                 UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                 ULONG wait_option);
UINT        nx_web_http_client_head_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                          CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                          CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                          UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                          ULONG wait_option);
UINT        nx_web_http_client_delete_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                   CHAR *host, CHAR *username, CHAR *password,
                                                   UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                   ULONG wait_option);
UINT        nx_web_http_client_delete_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                            CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                            CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                            ULONG wait_option);

#endif
#else

/* HTTP source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size, UINT http_client_size);
UINT        _nx_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size);
UINT        _nxe_web_http_client_delete(NX_WEB_HTTP_CLIENT *client_ptr);
UINT        _nx_web_http_client_delete(NX_WEB_HTTP_CLIENT *client_ptr);
UINT        _nxe_web_http_client_response_body_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_web_http_client_response_body_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nxe_web_http_client_put_packet(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        _nx_web_http_client_put_packet(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        _nxe_web_http_client_get_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nx_web_http_client_get_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxe_web_http_client_get_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                    UINT server_port, CHAR *resource, UINT resource_length,
                                                    CHAR *host, UINT host_length, CHAR *username,
                                                    UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG wait_option);
UINT        _nx_web_http_client_get_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                   UINT server_port, CHAR *resource, UINT resource_length,
                                                   CHAR *host, UINT host_length, CHAR *username,
                                                   UINT username_length, CHAR *password,
                                                   UINT password_length, ULONG wait_option);
UINT        _nxe_web_http_client_put_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        _nx_web_http_client_put_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        _nxe_web_http_client_put_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                    UINT server_port, CHAR *resource, UINT resource_length,
                                                    CHAR *host, UINT host_length, CHAR *username,
                                                    UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG total_bytes,
                                                    ULONG wait_option);
UINT        _nx_web_http_client_put_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                   UINT server_port, CHAR *resource, UINT resource_length,
                                                   CHAR *host, UINT host_length, CHAR *username,
                                                   UINT username_length, CHAR *password,
                                                   UINT password_length, ULONG total_bytes,
                                                   ULONG wait_option);
UINT        _nxe_web_http_client_post_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        _nx_web_http_client_post_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option);
UINT        _nxe_web_http_client_post_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                     UINT server_port, CHAR *resource, UINT resource_length,
                                                     CHAR *host, UINT host_length, CHAR *username,
                                                     UINT username_length, CHAR *password,
                                                     UINT password_length, ULONG total_bytes,
                                                     ULONG wait_option);
UINT        _nx_web_http_client_post_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                    UINT server_port, CHAR *resource, UINT resource_length,
                                                    CHAR *host, UINT host_length, CHAR *username,
                                                    UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG total_bytes,
                                                    ULONG wait_option);
UINT        _nxe_web_http_client_head_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nx_web_http_client_head_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxe_web_http_client_head_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                     UINT server_port, CHAR *resource, UINT resource_length,
                                                     CHAR *host, UINT host_length, CHAR *username,
                                                     UINT username_length, CHAR *password,
                                                     UINT password_length, ULONG wait_option);
UINT        _nx_web_http_client_head_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                    UINT server_port, CHAR *resource, UINT resource_length,
                                                    CHAR *host, UINT host_length, CHAR *username,
                                                    UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG wait_option);
UINT        _nxe_web_http_client_delete_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nx_web_http_client_delete_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource, CHAR *host, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxe_web_http_client_delete_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                       UINT server_port, CHAR *resource, UINT resource_length,
                                                       CHAR *host, UINT host_length, CHAR *username,
                                                       UINT username_length, CHAR *password,
                                                       UINT password_length, ULONG wait_option);
UINT        _nx_web_http_client_delete_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                      UINT server_port, CHAR *resource, UINT resource_length,
                                                      CHAR *host, UINT host_length, CHAR *username,
                                                      UINT username_length, CHAR *password,
                                                      UINT password_length, ULONG wait_option);


UINT        _nxe_web_http_client_response_header_callback_set(NX_WEB_HTTP_CLIENT *client_ptr,
                                                              VOID (*callback_function)(NX_WEB_HTTP_CLIENT *client_ptr,
                                                              CHAR *field_name,
                                                              UINT field_name_length,
                                                              CHAR *field_value,
                                                              UINT field_value_length));
UINT        _nx_web_http_client_response_header_callback_set(NX_WEB_HTTP_CLIENT *client_ptr,
                                                             VOID (*callback_function)(NX_WEB_HTTP_CLIENT *client_ptr,
                                                             CHAR *field_name,
                                                             UINT field_name_length,
                                                             CHAR *field_value,
                                                             UINT field_value_length));
UINT        _nx_web_http_client_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option);
UINT        _nxe_web_http_client_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option);
UINT        _nx_web_http_client_request_initialize(NX_WEB_HTTP_CLIENT *client_ptr,
                                                   UINT method,
                                                   CHAR *resource,
                                                   CHAR *host,
                                                   UINT input_size,
                                                   UINT transfer_encoding_chunked,
                                                   CHAR *username,
                                                   CHAR *password,
                                                   UINT wait_option);
UINT        _nxe_web_http_client_request_initialize(NX_WEB_HTTP_CLIENT *client_ptr,
                                                    UINT method,
                                                    CHAR *resource,
                                                    CHAR *host,
                                                    UINT input_size,
                                                    UINT transfer_encoding_chunked,
                                                    CHAR *username,
                                                    CHAR *password,
                                                    UINT wait_option);
UINT        _nx_web_http_client_request_initialize_extended(NX_WEB_HTTP_CLIENT *client_ptr,
                                                            UINT method,
                                                            CHAR *resource,
                                                            UINT resource_length,
                                                            CHAR *host,
                                                            UINT host_length,
                                                            UINT input_size,
                                                            UINT transfer_encoding_chunked,
                                                            CHAR *username,
                                                            UINT username_length,
                                                            CHAR *password,
                                                            UINT password_length,
                                                            UINT wait_option);
UINT        _nxe_web_http_client_request_initialize_extended(NX_WEB_HTTP_CLIENT *client_ptr,
                                                             UINT method,
                                                             CHAR *resource,
                                                             UINT resource_length,
                                                             CHAR *host,
                                                             UINT host_length,
                                                             UINT input_size,
                                                             UINT transfer_encoding_chunked,
                                                             CHAR *username,
                                                             UINT username_length,
                                                             CHAR *password,
                                                             UINT password_length,
                                                             UINT wait_option);


UINT        _nx_web_http_client_request_send(NX_WEB_HTTP_CLIENT *client_ptr, ULONG wait_option);
UINT        _nxe_web_http_client_request_send(NX_WEB_HTTP_CLIENT *client_ptr, ULONG wait_option);
UINT        _nx_web_http_client_request_packet_allocate(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr,
                                                        UINT wait_option);
UINT        _nxe_web_http_client_request_packet_allocate(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr,
                                                         UINT wait_option);

UINT        _nx_web_http_client_request_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT name_length,
                                                   CHAR *field_value, UINT value_length, UINT wait_option);
UINT        _nxe_web_http_client_request_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT name_length,
                                                    CHAR *field_value, UINT value_length, UINT wait_option);

UINT        _nx_web_http_client_request_packet_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT more_data, ULONG wait_option);
UINT        _nxe_web_http_client_request_packet_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT more_data, ULONG wait_option);

UINT        _nx_web_http_client_request_chunked_set(NX_WEB_HTTP_CLIENT *client_ptr, UINT chunk_size, NX_PACKET *packet_ptr);
UINT        _nxe_web_http_client_request_chunked_set(NX_WEB_HTTP_CLIENT *client_ptr, UINT chunk_size, NX_PACKET *packet_ptr);

#ifdef NX_WEB_HTTPS_ENABLE
UINT        _nx_web_http_client_secure_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                               UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                               ULONG wait_option);
UINT        _nxe_web_http_client_secure_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        _nxe_web_http_client_get_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                  CHAR *host, CHAR *username, CHAR *password,
                                                  UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                  ULONG wait_option);
UINT        _nx_web_http_client_get_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                 CHAR *host, CHAR *username, CHAR *password,
                                                 UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                 ULONG wait_option);
UINT        _nxe_web_http_client_get_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                           CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                           CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                           ULONG wait_option);
UINT        _nx_web_http_client_get_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                          CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                          CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                          UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                          ULONG wait_option);
UINT        _nxe_web_http_client_put_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                  CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                  UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                  ULONG wait_option);
UINT        _nx_web_http_client_put_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                 CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                 UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                 ULONG wait_option);
UINT        _nxe_web_http_client_put_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                           CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                           CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                           ULONG wait_option);
UINT        _nx_web_http_client_put_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                          CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                          CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                          UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                          ULONG wait_option);
UINT        _nxe_web_http_client_post_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                   CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                   UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                   ULONG wait_option);
UINT        _nx_web_http_client_post_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                  CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                                  UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                  ULONG wait_option);
UINT        _nxe_web_http_client_post_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                            CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                            CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                            ULONG wait_option);
UINT        _nx_web_http_client_post_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                           CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                           CHAR *username, UINT username_length, CHAR *password, UINT password_length, ULONG total_bytes,
                                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                           ULONG wait_option);

UINT        _nxe_web_http_client_head_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        _nx_web_http_client_head_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        _nxe_web_http_client_head_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                            CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                            CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                            ULONG wait_option);
UINT        _nx_web_http_client_head_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                           CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                           CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                           ULONG wait_option);
UINT        _nxe_web_http_client_delete_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        _nx_web_http_client_delete_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                                CHAR *host, CHAR *username, CHAR *password,
                                                UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                ULONG wait_option);
UINT        _nxe_web_http_client_delete_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                              CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                              CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                              UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                              ULONG wait_option);
UINT        _nx_web_http_client_delete_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                             CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                             CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                             UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                             ULONG wait_option);



#endif

/* Define internal HTTP functions.  */

UINT        _nx_web_http_client_type_get(CHAR *name, CHAR *http_type_string);
UINT        _nx_web_http_client_content_length_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr);
UINT        _nx_web_http_client_process_header_fields(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr);
UINT        _nx_web_http_client_number_convert(UINT number, CHAR *string);
UINT        _nx_web_http_client_content_type_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *resource, ULONG wait_option);
UINT        _nx_web_http_client_content_length_header_add(NX_WEB_HTTP_CLIENT *client_ptr, ULONG total_bytes, ULONG wait_option);
VOID        _nx_web_http_client_error_exit(NX_WEB_HTTP_CLIENT *client_ptr, UINT wait_option);
UINT        _nx_web_http_client_receive(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_web_http_client_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        _nx_web_http_client_get_server_response(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
VOID        _nx_web_http_client_cleanup(NX_WEB_HTTP_CLIENT *client_ptr);
UINT        _nx_web_http_client_memicmp(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length);
UINT        _nx_web_http_client_response_read(NX_WEB_HTTP_CLIENT *client_ptr, UCHAR *data, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_client_response_byte_expect(NX_WEB_HTTP_CLIENT *client_ptr, UCHAR data, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_client_chunked_size_get(NX_WEB_HTTP_CLIENT *client_ptr, UINT *chunk_size, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_client_response_chunked_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_pptr, ULONG wait_option);

#endif  /* NX_WEB_HTTP_CLIENT_SOURCE_CODE */

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NX_WEB_HTTP_CLIENT_H */
