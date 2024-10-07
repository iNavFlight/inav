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
/*    nx_web_http_server.h                                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Hypertext Transfer Protocol (HTTP)       */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            converting number to string,*/
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            deprecated unused macros,   */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported ECC configuration,*/
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_HTTP_SERVER_H
#define NXD_HTTP_SERVER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* If HTTPS is enabled, make sure TLS is enabled in the TCP server. */
#ifdef NX_WEB_HTTPS_ENABLE
#ifndef NX_TCPSERVER_ENABLE_TLS
#define NX_TCPSERVER_ENABLE_TLS
#endif
#endif

#include    "nx_api.h"
/* If not using FileX, define this option and define the file writing services
   declared in filex_stub.h.  */ 
/* #define      NX_WEB_HTTP_NO_FILEX  */

#ifndef      NX_WEB_HTTP_NO_FILEX
#include    "fx_api.h"
#else
#include    "filex_stub.h"
#endif

/* Include common HTTP definitions. */
#include "nx_web_http_common.h"

/* Include multiple-socket TCP/TLS support. */
#include "nx_tcpserver.h"

/* Define the HTTP Server ID.  */
#define NX_WEB_HTTP_SERVER_ID               0x48545451UL

/* Enable Digest authentication. 
#define NX_WEB_HTTP_DIGEST_ENABLE
*/

/* Define HTTP TCP socket create options.  */
#ifndef NX_WEB_HTTP_SERVER_PRIORITY
#define NX_WEB_HTTP_SERVER_PRIORITY             4
#endif

#ifndef NX_WEB_HTTP_SERVER_WINDOW_SIZE
#define NX_WEB_HTTP_SERVER_WINDOW_SIZE          8192
#endif

#ifndef NX_WEB_HTTP_SERVER_TIMEOUT_ACCEPT
#define NX_WEB_HTTP_SERVER_TIMEOUT_ACCEPT       (1 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE
#define NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE      (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_SERVER_TIMEOUT_SEND
#define NX_WEB_HTTP_SERVER_TIMEOUT_SEND         (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT
#define NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT   (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_SERVER_TIMEOUT
#define NX_WEB_HTTP_SERVER_TIMEOUT              (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_WEB_HTTP_SERVER_SESSION_MAX
#define NX_WEB_HTTP_SERVER_SESSION_MAX          2
#endif

#ifndef NX_WEB_HTTP_SERVER_SESSION_BUFFER_SIZE
#define NX_WEB_HTTP_SERVER_SESSION_BUFFER_SIZE  (sizeof(NX_TCP_SESSION) * NX_WEB_HTTP_SERVER_SESSION_MAX)
#endif

#ifndef NX_WEB_HTTP_SERVER_MAX_PENDING
#define NX_WEB_HTTP_SERVER_MAX_PENDING          (NX_WEB_HTTP_SERVER_SESSION_MAX << 1)
#endif

/* Deprecated. This symbol is defined for compatibility. */
#ifndef NX_WEB_HTTP_SERVER_THREAD_TIME_SLICE
#define NX_WEB_HTTP_SERVER_THREAD_TIME_SLICE    2
#endif

#ifndef NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE
#define NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE      600
#endif

/* Define the HTTP server retry parameters.  */

#ifndef NX_WEB_HTTP_SERVER_RETRY_SECONDS
#define NX_WEB_HTTP_SERVER_RETRY_SECONDS        2           /* 2 second initial timeout                            */ 
#endif

#ifndef NX_WEB_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH
#define NX_WEB_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH 20          /* Maximum of 20 queued transmit packets               */ 
#endif

#ifndef NX_WEB_HTTP_SERVER_RETRY_MAX
#define NX_WEB_HTTP_SERVER_RETRY_MAX            10          /* Maximum of 10 retries per packet                    */ 
#endif

#ifndef NX_WEB_HTTP_SERVER_RETRY_SHIFT
#define NX_WEB_HTTP_SERVER_RETRY_SHIFT          1           /* Every retry is twice as long                        */
#endif

#ifndef NX_WEB_HTTP_SERVER_NONCE_MAX
#define NX_WEB_HTTP_SERVER_NONCE_MAX            (NX_WEB_HTTP_SERVER_SESSION_MAX << 1)
#endif

#ifndef NX_WEB_HTTP_SERVER_NONCE_SIZE
#define NX_WEB_HTTP_SERVER_NONCE_SIZE           32
#endif

#ifndef NX_WEB_HTTP_SERVER_NONCE_TIMEOUT
#define NX_WEB_HTTP_SERVER_NONCE_TIMEOUT        (10 * NX_IP_PERIODIC_RATE)
#endif

/* Define the state of the nonce.  */

#define NX_WEB_HTTP_SERVER_NONCE_INVALID        0
#define NX_WEB_HTTP_SERVER_NONCE_VALID          1
#define NX_WEB_HTTP_SERVER_NONCE_ACCEPTED       2

/* Define HTTP Server request types.  */

#define NX_WEB_HTTP_SERVER_UNKNOWN_REQUEST      0
#define NX_WEB_HTTP_SERVER_GET_REQUEST          1
#define NX_WEB_HTTP_SERVER_POST_REQUEST         2
#define NX_WEB_HTTP_SERVER_HEAD_REQUEST         3
#define NX_WEB_HTTP_SERVER_PUT_REQUEST          4
#define NX_WEB_HTTP_SERVER_DELETE_REQUEST       5


#ifdef  NX_WEB_HTTP_DIGEST_ENABLE

/* Include the MD5 digest header file.  */

#include "nx_md5.h"
  
#endif

#ifdef NX_WEB_HTTPS_ENABLE
/* Include TLS for HTTPS support. */
#include "nx_secure_tls_api.h"
#endif

/* Define default MIME type. */
/* Page 14, Section 5.2, RFC 2045 */
#define NX_WEB_HTTP_SERVER_DEFAULT_MIME         "text/plain"


/* Define the MIME map structure. */

typedef struct NX_WEB_HTTP_SERVER_MIME_MAP_STRUCT
{
    CHAR           *nx_web_http_server_mime_map_extension;              /* Extension of file    */
    CHAR           *nx_web_http_server_mime_map_type;                   /* MIME type of file    */
} NX_WEB_HTTP_SERVER_MIME_MAP;


/* Define the date structure. */

typedef struct NX_WEB_HTTP_SERVER_DATE_STRUCT
{
    USHORT          nx_web_http_server_year;                            /* Year                 */
    UCHAR           nx_web_http_server_month;                           /* Month                */
    UCHAR           nx_web_http_server_day;                             /* Day                  */
    UCHAR           nx_web_http_server_hour;                            /* Hour                 */
    UCHAR           nx_web_http_server_minute;                          /* Minute               */
    UCHAR           nx_web_http_server_second;                          /* Second               */
    UCHAR           nx_web_http_server_weekday;                         /* Weekday              */
} NX_WEB_HTTP_SERVER_DATE;

/* Define the nonce structure.  */

typedef struct NX_WEB_HTTP_SERVER_NONCE_STRUCT
{
    UINT            nonce_state;                                        /* The state of the nonce               */
    ULONG           nonce_timestamp;                                    /* The time when the nonce is created   */
    NX_TCP_SESSION  *nonce_session_ptr;                                 /* The session accepted with this nonce */
    UCHAR           nonce_buffer[NX_WEB_HTTP_SERVER_NONCE_SIZE];        /* Nonce for digest authetication       */
} NX_WEB_HTTP_SERVER_NONCE;

/* Define the multipart context data structure.  */

typedef struct NX_WEB_HTTP_SERVER_MULTIPART_STRUCT
{

    /* Boundary string.  */
    UCHAR           nx_web_http_server_multipart_boundary[NX_WEB_HTTP_MAX_HEADER_FIELD + 1];

    /* Offset of available data. */
    UINT            nx_web_http_server_multipart_available_offset;

    /* Length of available data. */
    UINT            nx_web_http_server_multipart_available_length;

    /* Boundary find status. */
    volatile UINT   nx_web_http_server_multipart_boundary_find;

    /* The next packet to process. */
    NX_PACKET      *nx_web_http_server_multipart_next_packet;

    /* Offset of next available data. */
    UINT            nx_web_http_server_multipart_next_available_offset;

    /* The packet returned at last. */
    NX_PACKET      *nx_web_http_server_multipart_last_packet;

} NX_WEB_HTTP_SERVER_MULTIPART;


/* Define the HTTP Server data structure.  */

typedef struct NX_WEB_HTTP_SERVER_STRUCT
{
    ULONG           nx_web_http_server_id;                              /* HTTP Server ID                       */
    CHAR           *nx_web_http_server_name;                            /* Name of this HTTP Server             */
    NX_IP          *nx_web_http_server_ip_ptr;                          /* Pointer to associated IP structure   */
    CHAR            nx_web_http_server_request_resource[NX_WEB_HTTP_MAX_RESOURCE + 1];
                                                                        /* Uniform Resource Locator (URL)       */
    NX_PACKET_POOL *nx_web_http_server_packet_pool_ptr;                 /* Pointer to HTTP Server packet pool   */
    FX_MEDIA       *nx_web_http_server_media_ptr;                       /* Pointer to media control block       */
    ULONG           nx_web_http_server_get_requests;                    /* Number of get requests               */
    ULONG           nx_web_http_server_head_requests;                   /* Number of head requests              */
    ULONG           nx_web_http_server_put_requests;                    /* Number of put requests               */
    ULONG           nx_web_http_server_delete_requests;                 /* Number of delete requests            */
    ULONG           nx_web_http_server_post_requests;                   /* Number of post requests              */
    ULONG           nx_web_http_server_unknown_requests;                /* Number of unknown requests           */
    ULONG           nx_web_http_server_total_bytes_sent;                /* Number of total bytes sent           */
    ULONG           nx_web_http_server_total_bytes_received;            /* Number of total bytes received       */
    ULONG           nx_web_http_server_allocation_errors;               /* Number of allocation errors          */
    ULONG           nx_web_http_server_invalid_http_headers;            /* Number of invalid http headers       */
    FX_FILE         nx_web_http_server_file;                            /* HTTP file control block              */

    NX_TCPSERVER    nx_web_http_server_tcpserver;                       /* TCP server with multiple sessions    */
    NX_TCP_SESSION *nx_web_http_server_current_session_ptr;             /* Current session in process           */
    UCHAR           nx_web_http_server_session_buffer[NX_WEB_HTTP_SERVER_SESSION_BUFFER_SIZE];
                                                                    /* Size of session buffer               */

    NX_TCP_SOCKET   nx_web_http_server_socket;                          /* HTTP Server TCP socket               */
    UINT            nx_web_http_server_listen_port;                     /* HTTP(S) listening port.              */
#ifdef NX_WEB_HTTPS_ENABLE
    UINT            nx_web_http_is_https_server;                        /* If using TLS for HTTPS, set to true. */
#endif

#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
    NX_MD5          nx_web_http_server_md5data;                         /* HTTP server MD5 work area            */
    NX_WEB_HTTP_SERVER_NONCE
                    nx_web_http_server_nonces[NX_WEB_HTTP_SERVER_NONCE_MAX];
                                                                        /* Nonce for digest authetication       */
#endif /* NX_WEB_HTTP_DIGEST_ENABLE */

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
    NX_WEB_HTTP_SERVER_MULTIPART
                    nx_web_http_server_multipart;                       /* HTTP multipart area                  */
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */

    UINT            nx_web_http_server_request_type;                    /* HTTP request type                    */
    NX_WEB_HTTP_SERVER_MIME_MAP
                   *nx_web_http_server_mime_maps_additional;            /* Additional HTTP MIME maps            */
    UINT            nx_web_http_server_mime_maps_additional_num;        /* Number of additional HTTP MIME maps  */

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    UCHAR           nx_web_http_server_keepalive;                       /* HTTP keepalive flag                  */
    UCHAR           nx_web_http_server_reserved;                        /* Reserved                             */
#else
    UCHAR           nx_web_http_server_reserved[2];                     /* Reserved                             */
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    UCHAR           nx_web_http_server_response_chunked;                /* Flag for chunked response            */
    UCHAR           nx_web_http_server_request_chunked;                 /* Flag for chunked request             */
    UINT            nx_web_http_server_expect_transfer_bytes;           /* The bytes expected to transfer       */
    UINT            nx_web_http_server_actual_bytes_transferred;        /* The actual transferred bytes         */
    UINT            nx_web_http_server_expect_receive_bytes;            /* The bytes expected to receive        */
    UINT            nx_web_http_server_actual_bytes_received;           /* The actual received bytes            */
    UINT            nx_web_http_server_chunked_request_remaining_size;  /* Remaining size of the chunked request*/
    NX_PACKET      *nx_web_http_server_request_packet;                  /* Pointer to the received request      */

    /* Define the user supplied routines that are used to inform the application of particular server requests.  */

    UINT (*nx_web_http_server_authentication_check)(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);
    UINT (*nx_web_http_server_authentication_check_extended)(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length);
    UINT (*nx_web_http_server_request_notify)(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);
#ifdef __PRODUCT_NETXDUO__
    UINT (*nx_web_http_server_invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type);
#else
    UINT (*nx_web_http_server_invalid_username_password_callback)(CHAR *resource, ULONG client_address, UINT request_type);
#endif
    VOID (*nx_web_http_server_gmt_get)(NX_WEB_HTTP_SERVER_DATE *date);
    UINT (*nx_web_http_server_cache_info_get)(CHAR *resource, UINT *max_age, NX_WEB_HTTP_SERVER_DATE *last_modified);
#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
    UINT (*nx_web_http_server_digest_authenticate_callback)(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, CHAR *name_ptr,
                                                            CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                            CHAR *authorization_uri, CHAR *authorization_nc,
                                                            CHAR *authorization_cnonce);
#endif
} NX_WEB_HTTP_SERVER;


#ifndef NX_WEB_HTTP_SERVER_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_web_http_server_callback_data_send                _nx_web_http_server_callback_data_send
#define nx_web_http_server_callback_response_send            _nx_web_http_server_callback_response_send
#define nx_web_http_server_callback_response_send_extended   _nx_web_http_server_callback_response_send_extended
#define nx_web_http_server_content_get                       _nx_web_http_server_content_get
#define nx_web_http_server_create                            _nx_web_http_server_create
#define nx_web_http_server_delete                            _nx_web_http_server_delete
#define nx_web_http_server_param_get                         _nx_web_http_server_param_get
#define nx_web_http_server_query_get                         _nx_web_http_server_query_get
#define nx_web_http_server_start                             _nx_web_http_server_start
#define nx_web_http_server_secure_configure                  _nx_web_http_server_secure_configure
#define nx_web_http_server_secure_ecc_configure              _nx_web_http_server_secure_ecc_configure
#define nx_web_http_server_stop                              _nx_web_http_server_stop
#define nx_web_http_server_content_get_extended              _nx_web_http_server_content_get_extended
#define nx_web_http_server_content_length_get                _nx_web_http_server_content_length_get
#define nx_web_http_server_get_entity_header                 _nx_web_http_server_get_entity_header
#define nx_web_http_server_get_entity_content                _nx_web_http_server_get_entity_content
#define nx_web_http_server_callback_generate_response_header _nx_web_http_server_callback_generate_response_header
#define nx_web_http_server_callback_generate_response_header_extended _nx_web_http_server_callback_generate_response_header_extended
#define nx_web_http_server_callback_packet_send              _nx_web_http_server_callback_packet_send
#define nx_web_http_server_gmt_callback_set                  _nx_web_http_server_gmt_callback_set
#define nx_web_http_server_cache_info_callback_set           _nx_web_http_server_cache_info_callback_set
#define nx_web_http_server_mime_maps_additional_set          _nx_web_http_server_mime_maps_additional_set
#define nx_web_http_server_type_get                          _nx_web_http_server_type_get
#define nx_web_http_server_type_get_extended                 _nx_web_http_server_type_get_extended
#define nx_web_http_server_packet_content_find               _nx_web_http_server_packet_content_find
#define nx_web_http_server_packet_get                        _nx_web_http_server_packet_get
#define nx_web_http_server_invalid_userpassword_notify_set   _nx_web_http_server_invalid_userpassword_notify_set
#define nx_web_http_server_response_chunked_set              _nx_web_http_server_response_chunked_set
#define nx_web_http_server_response_packet_allocate          _nx_web_http_server_response_packet_allocate
#define nx_web_http_server_digest_authenticate_notify_set    _nx_web_http_server_digest_authenticate_notify_set
#define nx_web_http_server_authentication_check_set          _nx_web_http_server_authentication_check_set

#else

/* Services with error checking.  */

#define nx_web_http_server_callback_data_send                _nxe_web_http_server_callback_data_send
#define nx_web_http_server_callback_response_send            _nxe_web_http_server_callback_response_send
#define nx_web_http_server_callback_response_send_extended   _nxe_web_http_server_callback_response_send_extended
#define nx_web_http_server_content_get                       _nxe_web_http_server_content_get
#define nx_web_http_server_content_length_get                _nxe_web_http_server_content_length_get
#define nx_web_http_server_create(p,n,i,pn,m,sp,ss,pp,a,r)   _nxe_web_http_server_create(p,n,i,pn,m,sp,ss,pp,a,r,sizeof(NX_WEB_HTTP_SERVER))
#define nx_web_http_server_delete                            _nxe_web_http_server_delete
#define nx_web_http_server_param_get                         _nxe_web_http_server_param_get
#define nx_web_http_server_query_get                         _nxe_web_http_server_query_get
#define nx_web_http_server_start                             _nxe_web_http_server_start
#define nx_web_http_server_secure_configure                  _nxe_web_http_server_secure_configure
#define nx_web_http_server_secure_ecc_configure              _nxe_web_http_server_secure_ecc_configure
#define nx_web_http_server_stop                              _nxe_web_http_server_stop
#define nx_web_http_server_content_get_extended              _nxe_web_http_server_content_get_extended
#define nx_web_http_server_get_entity_header                 _nxe_web_http_server_get_entity_header
#define nx_web_http_server_get_entity_content                _nxe_web_http_server_get_entity_content
#define nx_web_http_server_callback_generate_response_header _nxe_web_http_server_callback_generate_response_header
#define nx_web_http_server_callback_generate_response_header_extended _nxe_web_http_server_callback_generate_response_header_extended
#define nx_web_http_server_callback_packet_send              _nxe_web_http_server_callback_packet_send
#define nx_web_http_server_gmt_callback_set                  _nxe_web_http_server_gmt_callback_set
#define nx_web_http_server_cache_info_callback_set           _nxe_web_http_server_cache_info_callback_set
#define nx_web_http_server_mime_maps_additional_set          _nxe_web_http_server_mime_maps_additional_set
#define nx_web_http_server_type_get                          _nxe_web_http_server_type_get
#define nx_web_http_server_type_get_extended                 _nxe_web_http_server_type_get_extended
#define nx_web_http_server_packet_content_find               _nxe_web_http_server_packet_content_find
#define nx_web_http_server_packet_get                        _nxe_web_http_server_packet_get
#define nx_web_http_server_invalid_userpassword_notify_set   _nxe_web_http_server_invalid_userpassword_notify_set
#define nx_web_http_server_response_chunked_set              _nxe_web_http_server_response_chunked_set
#define nx_web_http_server_response_packet_allocate          _nxe_web_http_server_response_packet_allocate
#define nx_web_http_server_digest_authenticate_notify_set    _nxe_web_http_server_digest_authenticate_notify_set
#define nx_web_http_server_authentication_check_set          _nxe_web_http_server_authentication_check_set

#endif

/* Define the prototypes accessible to the application software.  */


UINT        nx_web_http_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        nx_web_http_server_callback_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        nx_web_http_server_callback_response_send_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                                               UINT status_code_length, CHAR *information,
                                                               UINT information_length, CHAR *additional_info,
                                                               UINT additional_info_length);
UINT        nx_web_http_server_content_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        nx_web_http_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        nx_web_http_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);

#ifdef NX_DISABLE_ERROR_CHECKING
UINT        _nx_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, UINT server_port, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                   UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                   UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr));
#else
UINT        _nxe_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, UINT server_port, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                    UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                    UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr), UINT http_server_size);
#endif /* NX_DISABLE_ERROR_CHECKING */

UINT        nx_web_http_server_delete(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        nx_web_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT *param_size, UINT max_param_size);
UINT        nx_web_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT *query_size, UINT max_query_size);
UINT        nx_web_http_server_start(NX_WEB_HTTP_SERVER *http_server_ptr);
#ifdef NX_WEB_HTTPS_ENABLE
UINT        nx_web_http_server_secure_configure(NX_WEB_HTTP_SERVER *http_server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            VOID *metadata_buffer, ULONG metadata_size,
                                            UCHAR* packet_buffer, UINT packet_buffer_size,
                                            NX_SECURE_X509_CERT *identity_certificate,
                                            NX_SECURE_X509_CERT *trusted_certificates[],
                                            UINT trusted_certs_num,
                                            NX_SECURE_X509_CERT *remote_certificates[],
                                            UINT remote_certs_num,
                                            UCHAR *remote_certificate_buffer,
                                            UINT remote_cert_buffer_size);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT        nx_web_http_server_secure_ecc_configure(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                    const USHORT *supported_groups, USHORT supported_group_count,
                                                    const NX_CRYPTO_METHOD **curves);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#endif /* NX_WEB_HTTPS_ENABLE */
UINT        nx_web_http_server_stop(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        nx_web_http_server_content_get_extended(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        nx_web_http_server_content_length_get(NX_PACKET *packet_ptr, ULONG *length);
UINT        nx_web_http_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        nx_web_http_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        nx_web_http_server_callback_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                             CHAR *status_code, UINT content_length, CHAR *content_type,
                                                             CHAR* additional_header);
UINT        nx_web_http_server_callback_generate_response_header_extended(NX_WEB_HTTP_SERVER *server_ptr,
                                                                          NX_PACKET **packet_pptr, CHAR *status_code,
                                                                          UINT status_code_length, UINT content_length,
                                                                          CHAR *content_type, UINT content_type_length,
                                                                          CHAR *additional_header, UINT additional_header_length);
UINT        nx_web_http_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        nx_web_http_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *));
UINT        nx_web_http_server_cache_info_callback_set(NX_WEB_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_WEB_HTTP_SERVER_DATE *));
UINT        nx_web_http_server_mime_maps_additional_set(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        nx_web_http_server_type_get(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string, UINT *string_size);
UINT        nx_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length,
                                                 CHAR *http_type_string, UINT http_type_string_max_size, UINT *string_size);

UINT        nx_web_http_server_invalid_userpassword_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr, UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type ));

UINT        nx_web_http_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr, UINT chunk_size, NX_PACKET *packet_ptr);
UINT        nx_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, UINT wait_option);
UINT        nx_web_http_server_digest_authenticate_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                              UINT (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                              CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                              CHAR *authorization_uri, CHAR *authorization_nc,
                                                              CHAR *authorization_cnonce)); 
UINT        nx_web_http_server_authentication_check_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                        UINT (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));


#else

/* HTTP source code is being compiled, do not perform any API mapping.  */

UINT        _nx_web_http_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        _nx_web_http_server_callback_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        _nx_web_http_server_callback_response_send_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                                                UINT status_code_length, CHAR *information,
                                                                UINT information_length, CHAR *additional_info,
                                                                UINT additional_info_length);
UINT        _nx_web_http_server_content_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nx_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, UINT server_port, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr));
UINT        _nx_web_http_server_delete(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        _nx_web_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT *param_size, UINT max_param_size);
UINT        _nx_web_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT *query_size, UINT max_query_size);
UINT        _nx_web_http_server_start(NX_WEB_HTTP_SERVER *http_server_ptr);
#ifdef NX_WEB_HTTPS_ENABLE
UINT        _nx_web_http_server_secure_configure(NX_WEB_HTTP_SERVER *http_server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            VOID *metadata_buffer, ULONG metadata_size,
                                            UCHAR* packet_buffer, UINT packet_buffer_size,
                                            NX_SECURE_X509_CERT *identity_certificate,
                                            NX_SECURE_X509_CERT *trusted_certificates[],
                                            UINT trusted_certs_num,
                                            NX_SECURE_X509_CERT *remote_certificates[],
                                            UINT remote_certs_num,
                                            UCHAR *remote_certificate_buffer,
                                            UINT remote_cert_buffer_size);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT        _nx_web_http_server_secure_ecc_configure(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                     const USHORT *supported_groups, USHORT supported_group_count,
                                                     const NX_CRYPTO_METHOD **curves);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#endif /* NX_WEB_HTTPS_ENABLE */
UINT        _nx_web_http_server_stop(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        _nx_web_http_server_content_get_extended(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nx_web_http_server_content_length_get(NX_PACKET *packet_ptr, ULONG *length);
UINT        _nx_web_http_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        _nx_web_http_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        _nx_web_http_server_callback_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                              CHAR *status_code, UINT content_length, CHAR *content_type,
                                                              CHAR* additional_header);
UINT        _nx_web_http_server_callback_generate_response_header_extended(NX_WEB_HTTP_SERVER *server_ptr,
                                                                           NX_PACKET **packet_pptr, CHAR *status_code,
                                                                           UINT status_code_length, UINT content_length,
                                                                           CHAR *content_type, UINT content_type_length,
                                                                           CHAR *additional_header, UINT additional_header_length);
UINT        _nx_web_http_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nx_web_http_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr, VOID (*gmt_callback)(NX_WEB_HTTP_SERVER_DATE *));
UINT        _nx_web_http_server_cache_info_callback_set(NX_WEB_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_WEB_HTTP_SERVER_DATE *));
UINT        _nx_web_http_server_mime_maps_additional_set(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        _nx_web_http_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);
UINT        _nx_web_http_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        _nx_web_http_server_invalid_userpassword_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr, UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type ));
UINT        _nx_web_http_server_digest_authenticate_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                               UINT (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                               CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                               CHAR *authorization_uri, CHAR *authorization_nc,
                                                               CHAR *authorization_cnonce)); 
UINT        _nx_web_http_server_authentication_check_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                         UINT (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));


/* Define internal HTTP Server functions.  */

UINT        _nx_web_http_server_get_client_request(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
VOID        _nx_web_http_server_get_process(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, NX_PACKET *packet_ptr);
VOID        _nx_web_http_server_put_process(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
VOID        _nx_web_http_server_delete_process(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nx_web_http_server_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                              UINT status_code_length, CHAR *information,
                                              UINT information_length, CHAR *additional_information,
                                              UINT additional_information_length);
UINT        _nx_web_http_server_basic_authenticate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, CHAR *password_ptr, CHAR *realm_ptr, UINT *auth_request_present);
UINT        _nx_web_http_server_retrieve_basic_authorization(NX_PACKET *packet_ptr, CHAR *authorization_request_ptr);
UINT        _nx_web_http_server_retrieve_resource(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *destination, UINT max_size);
UINT        _nx_web_http_server_calculate_content_offset(NX_PACKET *packet_ptr);
UINT        _nx_web_http_server_type_get(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string, UINT *string_size);
UINT        _nx_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length,
                                                  CHAR *http_type_string, UINT http_type_string_max_size, UINT *string_size);

UINT        _nx_web_http_server_receive(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_web_http_server_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
VOID        _nx_web_http_server_connection_reset(NX_WEB_HTTP_SERVER *server_ptr, NX_TCP_SESSION *session_ptr, UINT wait_option);
VOID        _nx_web_http_server_connection_disconnect(NX_WEB_HTTP_SERVER *server_ptr, NX_TCP_SESSION *session_ptr, UINT wait_option);

#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
UINT        _nx_web_http_server_digest_authenticate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, CHAR *password_ptr, CHAR *realm_ptr, UINT *auth_request_present);
VOID        _nx_web_http_server_digest_response_calculate(NX_WEB_HTTP_SERVER *server_ptr, CHAR *username, CHAR *realm, CHAR *password, CHAR *nonce, CHAR *method, CHAR *uri, CHAR *nc, CHAR *cnonce, CHAR *result);
UINT        _nx_web_http_server_retrieve_digest_authorization(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *response, CHAR *uri, CHAR *nc, CHAR *cnonce, NX_WEB_HTTP_SERVER_NONCE **nonce_ptr);
VOID        _nx_web_http_server_hex_ascii_convert(CHAR *source, UINT source_length, CHAR *destination);
UINT        _nx_web_http_server_nonce_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_NONCE **nonce_ptr);
#endif

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
UINT        _nx_web_http_server_boundary_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr);
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */

UINT        _nx_web_http_server_match_string(UCHAR *src_start, UCHAR *src_end, UCHAR *target, ULONG target_length, ULONG *match_count, UCHAR **match_end_ptr);
UINT        _nx_web_http_server_field_value_get(NX_PACKET *packet_ptr, UCHAR *field_name, ULONG name_length, UCHAR *field_value, ULONG field_value_size);
UINT        _nx_web_http_server_memicmp(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length);

UINT        _nx_web_http_server_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                         CHAR *status_code, UINT status_code_length,
                                                         UINT content_length, CHAR *content_type,
                                                         UINT content_type_length, CHAR* additional_header,
                                                         UINT additional_header_length);
UINT        _nx_web_http_server_date_to_string(NX_WEB_HTTP_SERVER_DATE *date, CHAR *string);
VOID        _nx_web_http_server_date_convert(UINT date, UINT count, CHAR *string);

VOID        _nx_web_http_server_receive_data(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr);
VOID        _nx_web_http_server_connection_end(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr);
VOID        _nx_web_http_server_connection_timeout(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr);


#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
VOID        _nx_web_http_server_get_client_keepalive(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

UINT        _nx_web_http_server_chunked_check(NX_PACKET *packet_ptr);
UINT        _nx_web_http_server_request_read(NX_WEB_HTTP_SERVER *server_ptr, UCHAR *data, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_server_request_byte_expect(NX_WEB_HTTP_SERVER *server_ptr, UCHAR data, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_server_chunked_size_get(NX_WEB_HTTP_SERVER *server_ptr, UINT *chunk_size, ULONG wait_option, NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr);
UINT        _nx_web_http_server_request_chunked_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG wait_option);
UINT        _nx_web_http_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr, UINT chunk_size, NX_PACKET *packet_ptr);
UINT        _nx_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, UINT wait_option);
UINT        _nxe_web_http_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr, UINT chunk_size, NX_PACKET *packet_ptr);
UINT        _nxe_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, UINT wait_option);
UINT        _nxe_web_http_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        _nxe_web_http_server_callback_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        _nxe_web_http_server_callback_response_send_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                                                 UINT status_code_length, CHAR *information,
                                                                 UINT information_length, CHAR *additional_info,
                                                                 UINT additional_info_length);
UINT        _nxe_web_http_server_content_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nxe_web_http_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);
UINT        _nxe_web_http_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        _nxe_web_http_server_content_get_extended(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset,
                                                  CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nxe_web_http_server_content_length_get(NX_PACKET *packet_ptr, ULONG *content_length);
UINT        _nxe_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, UINT server_port, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                    UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                    UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr), UINT http_server_size);
UINT        _nxe_web_http_server_delete(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        _nxe_web_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT *param_size, UINT max_param_size);
UINT        _nxe_web_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT *query_size, UINT max_query_size);
UINT        _nxe_web_http_server_start(NX_WEB_HTTP_SERVER *http_server_ptr);
#ifdef NX_WEB_HTTPS_ENABLE
UINT        _nxe_web_http_server_secure_configure(NX_WEB_HTTP_SERVER *http_server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            VOID *metadata_buffer, ULONG metadata_size,
                                            UCHAR* packet_buffer, UINT packet_buffer_size,
                                            NX_SECURE_X509_CERT *identity_certificate,
                                            NX_SECURE_X509_CERT *trusted_certificates[],
                                            UINT trusted_certs_num,
                                            NX_SECURE_X509_CERT *remote_certificates[],
                                            UINT remote_certs_num,
                                            UCHAR *remote_certificate_buffer,
                                            UINT remote_cert_buffer_size);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT        _nxe_web_http_server_secure_ecc_configure(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                      const USHORT *supported_groups, USHORT supported_group_count,
                                                      const NX_CRYPTO_METHOD **curves);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#endif /* NX_WEB_HTTPS_ENABLE */
UINT        _nxe_web_http_server_stop(NX_WEB_HTTP_SERVER *http_server_ptr);
UINT        _nxe_web_http_server_invalid_userpassword_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                             UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type));
UINT        _nxe_web_http_server_type_get(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string, UINT *string_size);
UINT        _nxe_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length,
                                                   CHAR *http_type_string, UINT http_type_string_max_size, UINT *string_size);
UINT        _nxe_web_http_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        _nxe_web_http_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        _nxe_web_http_server_callback_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                                   CHAR *status_code, UINT content_length, CHAR *content_type,
                                                                   CHAR* additional_header);
UINT        _nxe_web_http_server_callback_generate_response_header_extended(NX_WEB_HTTP_SERVER *server_ptr,
                                                                            NX_PACKET **packet_pptr, CHAR *status_code,
                                                                            UINT status_code_length, UINT content_length,
                                                                            CHAR *content_type, UINT content_type_length,
                                                                            CHAR *additional_header, UINT additional_header_length);
UINT        _nxe_web_http_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nxe_web_http_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *));
UINT        _nxe_web_http_server_cache_info_callback_set(NX_WEB_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_WEB_HTTP_SERVER_DATE *));
UINT        _nxe_web_http_server_mime_maps_additional_set(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        _nxe_web_http_server_digest_authenticate_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                                UINT (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                                                                     CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                                                                     CHAR *authorization_uri, CHAR *authorization_nc,
                                                                                                     CHAR *authorization_cnonce)); 
UINT        _nxe_web_http_server_authentication_check_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                          UINT (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));

#endif /* NX_WEB_HTTP_SERVER_SOURCE_CODE */

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_HTTP_SERVER_H */
