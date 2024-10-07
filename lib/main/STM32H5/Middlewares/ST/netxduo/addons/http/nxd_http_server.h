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
/**   Hypertext Transfer Protocol (HTTP)                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_http_server.h                                   PORTABLE C      */
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
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
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

#include    "nx_api.h"
/* If not using FileX, define this option and define the file writing services
   declared in filex_stub.h.  */ 
/* #define      NX_HTTP_NO_FILEX  */

#ifndef      NX_HTTP_NO_FILEX
#include    "fx_api.h"
#else
#include    "filex_stub.h"
#endif

/* Define the HTTP Server ID.  */

#define NXD_HTTP_SERVER_ID                  0x48545451UL


/* Define the HTTP version.  */
#define NX_HTTP_VERSION                     "HTTP/1.0"
    
/* Enable Digest authentication. 
#define NX_HTTP_DIGEST_ENABLE
*/

/* Define HTTP TCP socket create options.  */

#ifndef NX_HTTP_TYPE_OF_SERVICE
#define NX_HTTP_TYPE_OF_SERVICE             NX_IP_NORMAL
#endif

#ifndef NX_HTTP_FRAGMENT_OPTION
#define NX_HTTP_FRAGMENT_OPTION             NX_DONT_FRAGMENT
#endif

#ifndef NX_HTTP_TIME_TO_LIVE
#define NX_HTTP_TIME_TO_LIVE                0x80
#endif

#ifndef NX_HTTP_SERVER_PRIORITY
#define NX_HTTP_SERVER_PRIORITY             16
#endif

#ifndef NX_HTTP_SERVER_WINDOW_SIZE
#define NX_HTTP_SERVER_WINDOW_SIZE          2048
#endif

#ifndef NX_HTTP_SERVER_TIMEOUT_ACCEPT
#define NX_HTTP_SERVER_TIMEOUT_ACCEPT       (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_HTTP_SERVER_TIMEOUT_RECEIVE
#define NX_HTTP_SERVER_TIMEOUT_RECEIVE      (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_HTTP_SERVER_TIMEOUT_SEND
#define NX_HTTP_SERVER_TIMEOUT_SEND         (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_HTTP_SERVER_TIMEOUT_DISCONNECT
#define NX_HTTP_SERVER_TIMEOUT_DISCONNECT   (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_HTTP_SERVER_TIMEOUT
#define NX_HTTP_SERVER_TIMEOUT              (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_HTTP_SERVER_MAX_PENDING
#define NX_HTTP_SERVER_MAX_PENDING          5
#endif

#ifndef NX_HTTP_SERVER_THREAD_TIME_SLICE
#define NX_HTTP_SERVER_THREAD_TIME_SLICE    2
#endif

#ifndef NX_HTTP_MAX_RESOURCE
#define NX_HTTP_MAX_RESOURCE                40
#endif

#ifndef NX_HTTP_MAX_NAME
#define NX_HTTP_MAX_NAME                    20
#endif

#ifndef NX_HTTP_MAX_PASSWORD
#define NX_HTTP_MAX_PASSWORD                20
#endif

#ifndef NX_PHYSICAL_TRAILER
#define NX_PHYSICAL_TRAILER                 4
#endif

#ifndef NX_HTTP_SERVER_MIN_PACKET_SIZE
#define NX_HTTP_SERVER_MIN_PACKET_SIZE      600
#endif

/* Define the HTTP server retry parameters.  */

#ifndef NX_HTTP_SERVER_RETRY_SECONDS
#define NX_HTTP_SERVER_RETRY_SECONDS        2           /* 2 second initial timeout                            */ 
#endif

#ifndef NX_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH
#define NX_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH 20          /* Maximum of 20 queued transmit packets               */ 
#endif

#ifndef NX_HTTP_SERVER_RETRY_MAX
#define NX_HTTP_SERVER_RETRY_MAX            10          /* Maximum of 10 retries per packet                    */ 
#endif

#ifndef NX_HTTP_SERVER_RETRY_SHIFT
#define NX_HTTP_SERVER_RETRY_SHIFT          1           /* Every retry is twice as long                        */
#endif

#ifndef NX_HTTP_SERVER_NONCE_SIZE
#define NX_HTTP_SERVER_NONCE_SIZE           32          /* The size of nonce for digest authtentication        */
#endif

#ifndef NX_HTTP_SERVER_NONCE_MAX
#define NX_HTTP_SERVER_NONCE_MAX            2           /* The size of nonce for digest authtentication        */
#endif

#ifndef NX_HTTP_SERVER_NONCE_TIMEOUT
#define NX_HTTP_SERVER_NONCE_TIMEOUT        (10 * NX_IP_PERIODIC_RATE)
#endif

/* Define the state of the nonce.  */

#define NX_HTTP_SERVER_NONCE_INVALID        0
#define NX_HTTP_SERVER_NONCE_VALID          1
#define NX_HTTP_SERVER_NONCE_ACCEPTED       2

/* NX_HTTP_MAX_STRING is base64 of "name:password" and plus 1 if an extra conversion is needed and plus 2 pad if needed. */
#define NX_HTTP_MAX_STRING                  ((NX_HTTP_MAX_NAME + NX_HTTP_MAX_PASSWORD + 1) * 4 / 3 + 1 + 2)

#define NX_HTTP_MAX_BINARY_MD5              16
#define NX_HTTP_MAX_ASCII_MD5               32


/* Define HTTP Server request types.  */

#define NX_HTTP_SERVER_UNKNOWN_REQUEST      0
#define NX_HTTP_SERVER_GET_REQUEST          1
#define NX_HTTP_SERVER_POST_REQUEST         2
#define NX_HTTP_SERVER_HEAD_REQUEST         3
#define NX_HTTP_SERVER_PUT_REQUEST          4
#define NX_HTTP_SERVER_DELETE_REQUEST       5


/* Define return code constants.  */

#define NX_HTTP_ERROR                       0xE0        /* HTTP internal error                                  */
#define NX_HTTP_TIMEOUT                     0xE1        /* HTTP timeout occurred                                */
#define NX_HTTP_FAILED                      0xE2        /* HTTP error                                           */
#define NX_HTTP_DONT_AUTHENTICATE           0xE3        /* HTTP authentication not needed                       */
#define NX_HTTP_BASIC_AUTHENTICATE          0xE4        /* HTTP basic authentication requested                  */
#define NX_HTTP_DIGEST_AUTHENTICATE         0xE5        /* HTTP digest authentication requested                 */
#define NX_HTTP_NOT_FOUND                   0xE6        /* HTTP request not found                               */
#define NX_HTTP_DATA_END                    0xE7        /* HTTP end of content area                             */
#define NX_HTTP_CALLBACK_COMPLETED          0xE8        /* HTTP user callback completed the processing          */
#define NX_HTTP_POOL_ERROR                  0xE9        /* HTTP supplied pool payload is too small              */
#define NX_HTTP_NOT_READY                   0xEA        /* HTTP client not ready for operation                  */
#define NX_HTTP_AUTHENTICATION_ERROR        0xEB        /* HTTP client authentication failed                    */
#define NX_HTTP_GET_DONE                    0xEC        /* HTTP client get is complete                          */
#define NX_HTTP_BAD_PACKET_LENGTH           0xED        /* Invalid packet received - length incorrect           */
#define NX_HTTP_REQUEST_UNSUCCESSFUL_CODE   0xEE        /* Received an error code instead of 2xx from server    */
#define NX_HTTP_INCOMPLETE_PUT_ERROR        0xEF        /* Server responds before PUT is complete               */
#define NX_HTTP_PASSWORD_TOO_LONG           0xF0        /* Password exceeded expected length                    */
#define NX_HTTP_USERNAME_TOO_LONG           0xF1        /* Username exceeded expected length                    */
#define NX_HTTP_NO_QUERY_PARSED             0xF2        /* Server unable to find query in client request        */
#define NX_HTTP_IMPROPERLY_TERMINATED_PARAM 0xF3        /* Client request parameter not properly terminated     */
#define NX_HTTP_BOUNDARY_ALREADY_FOUND      0xF4        /* Boundary is already found.                           */

/* Define the HTTP Server TCP port number */

#define NX_HTTP_SERVER_PORT                 80          /* Port for HTTP server                                 */

#ifdef  NX_HTTP_DIGEST_ENABLE

/* Include the MD5 digest header file.  */

#include "nx_md5.h"

#endif

/* Define the status code. */

#define NX_HTTP_STATUS_CONTINUE             "100 Continue"
#define NX_HTTP_STATUS_SWITCHING_PROTOCOLS  "101 Switching Protocols"
#define NX_HTTP_STATUS_OK                   "200 OK"
#define NX_HTTP_STATUS_CREATED              "201 Created"
#define NX_HTTP_STATUS_ACCEPTED             "202 Accepted"
#define NX_HTTP_STATUS_NON_AUTH_INFO        "203 Non-Authoritative Information"
#define NX_HTTP_STATUS_NO_CONTENT           "204 No Content"
#define NX_HTTP_STATUS_RESET_CONTENT        "205 Reset Content"
#define NX_HTTP_STATUS_PARTIAL_CONTENT      "206 Partial Content"
#define NX_HTTP_STATUS_MULTIPLE_CHOICES     "300 Multiple Choices"
#define NX_HTTP_STATUS_MOVED_PERMANETLY     "301 Moved Permanently"
#define NX_HTTP_STATUS_FOUND                "302 Found"
#define NX_HTTP_STATUS_SEE_OTHER            "303 See Other"
#define NX_HTTP_STATUS_NOT_MODIFIED         "304 Not Modified"
#define NX_HTTP_STATUS_USE_PROXY            "305 Use Proxy"
#define NX_HTTP_STATUS_TEMPORARY_REDIRECT   "307 Temporary Redirect"
#define NX_HTTP_STATUS_BAD_REQUEST          "400 Bad Request"
#define NX_HTTP_STATUS_UNAUTHORIZED         "401 Unauthorized"
#define NX_HTTP_STATUS_PAYMENT_REQUIRED     "402 Payment Required"
#define NX_HTTP_STATUS_FORBIDDEN            "403 Forbidden"
#define NX_HTTP_STATUS_NOT_FOUND            "404 Not Found"
#define NX_HTTP_STATUS_METHOD_NOT_ALLOWED   "405 Method Not Allowed"
#define NX_HTTP_STATUS_NOT_ACCEPTABLE       "406 Not Acceptable"
#define NX_HTTP_STATUS_PROXY_AUTH_REQUIRED  "407 Proxy Authentication Required"
#define NX_HTTP_STATUS_REQUEST_TIMEOUT      "408 Request Time-out"
#define NX_HTTP_STATUS_CONFLICT             "409 Conflict"
#define NX_HTTP_STATUS_GONE                 "410 Gone"
#define NX_HTTP_STATUS_LENGTH_REQUIRED      "411 Length Required"
#define NX_HTTP_STATUS_PRECONDITION_FAILED  "412 Precondition Failed"
#define NX_HTTP_STATUS_ENTITY_TOO_LARGE     "413 Request Entity Too Large"
#define NX_HTTP_STATUS_URL_TOO_LARGE        "414 Request-URL Too Large"
#define NX_HTTP_STATUS_UNSUPPORTED_MEDIA    "415 Unsupported Media Type"
#define NX_HTTP_STATUS_RANGE_NOT_SATISFY    "416 Requested range not satisfiable"
#define NX_HTTP_STATUS_EXPECTATION_FAILED   "417 Expectation Failed"
#define NX_HTTP_STATUS_INTERNAL_ERROR       "500 Internal Server Error"
#define NX_HTTP_STATUS_NOT_IMPLEMENTED      "501 Not Implemented"
#define NX_HTTP_STATUS_BAD_GATEWAY          "502 Bad Gateway"
#define NX_HTTP_STATUS_SERVICE_UNAVAILABLE  "503 Service Unavailable"
#define NX_HTTP_STATUS_GATEWAY_TIMEOUT      "504 Gateway Time-out"
#define NX_HTTP_STATUS_VERSION_ERROR        "505 HTTP Version not supported"


/* Define the max length of header field. */

#ifndef NX_HTTP_MAX_HEADER_FIELD            
#define NX_HTTP_MAX_HEADER_FIELD            256
#endif


/* Define default MIME type. */
/* Page 14, Section 5.2, RFC 2045 */
#define NX_HTTP_SERVER_DEFAULT_MIME         "text/plain"


/* Define the MIME map structure. */

typedef struct NX_HTTP_SERVER_MIME_MAP_STRUCT
{
    CHAR           *nx_http_server_mime_map_extension;              /* Extension of file    */
    CHAR           *nx_http_server_mime_map_type;                   /* MIME type of file    */
} NX_HTTP_SERVER_MIME_MAP;


/* Define the date structure. */

typedef struct NX_HTTP_SERVER_DATE_STRUCT
{
    USHORT          nx_http_server_year;                            /* Year                 */
    UCHAR           nx_http_server_month;                           /* Month                */
    UCHAR           nx_http_server_day;                             /* Day                  */
    UCHAR           nx_http_server_hour;                            /* Hour                 */
    UCHAR           nx_http_server_minute;                          /* Minute               */
    UCHAR           nx_http_server_second;                          /* Second               */
    UCHAR           nx_http_server_weekday;                         /* Weekday              */
} NX_HTTP_SERVER_DATE;


/* Define the nonce structure.  */

typedef struct NX_HTTP_SERVER_NONCE_STRUCT
{
    UINT            nonce_state;                                    /* The state of the nonce               */
    UINT            nonce_timestamp;                                /* The time when the nonce is created   */
    UCHAR           nonce_buffer[NX_HTTP_SERVER_NONCE_SIZE];        /* Nonce for digest authetication       */
} NX_HTTP_SERVER_NONCE;


/* Define the multipart context data structure.  */

typedef struct NX_HTTP_SERVER_MULTIPART_STRUCT
{

    /* Boundary string.  */
    UCHAR           nx_http_server_multipart_boundary[NX_HTTP_MAX_HEADER_FIELD + 1];

    /* Offset of available data. */
    UINT            nx_http_server_multipart_available_offset;

    /* Length of available data. */
    UINT            nx_http_server_multipart_available_length;

    /* Boundary find status. */
    volatile UINT   nx_http_server_multipart_boundary_find;

    /* The next packet to process. */
    NX_PACKET      *nx_http_server_multipart_next_packet;

    /* Offset of next available data. */
    UINT            nx_http_server_multipart_next_available_offset;

    /* The packet returned at last. */
    NX_PACKET      *nx_http_server_multipart_last_packet;

} NX_HTTP_SERVER_MULTIPART;


/* Define the HTTP Server data structure.  */

typedef struct NX_HTTP_SERVER_STRUCT
{
    ULONG           nx_http_server_id;                              /* HTTP Server ID                       */
    CHAR           *nx_http_server_name;                            /* Name of this HTTP Server             */
    NX_IP          *nx_http_server_ip_ptr;                          /* Pointer to associated IP structure   */
    CHAR            nx_http_server_request_resource[NX_HTTP_MAX_RESOURCE + 1];
                                                                    /* Uniform Resource Locator (URL)       */
    UINT            nx_http_connection_pending;                     /* Connection pending flag              */
    NX_PACKET_POOL *nx_http_server_packet_pool_ptr;                 /* Pointer to HTTP Server packet pool   */
    FX_MEDIA       *nx_http_server_media_ptr;                       /* Pointer to media control block       */
    ULONG           nx_http_server_get_requests;                    /* Number of get requests               */
    ULONG           nx_http_server_head_requests;                   /* Number of head requests              */
    ULONG           nx_http_server_put_requests;                    /* Number of put requests               */
    ULONG           nx_http_server_delete_requests;                 /* Number of delete requests            */
    ULONG           nx_http_server_post_requests;                   /* Number of post requests              */
    ULONG           nx_http_server_unknown_requests;                /* Number of unknown requests           */
    ULONG           nx_http_server_total_bytes_sent;                /* Number of total bytes sent           */
    ULONG           nx_http_server_total_bytes_received;            /* Number of total bytes received       */
    ULONG           nx_http_server_allocation_errors;               /* Number of allocation errors          */
    ULONG           nx_http_server_connection_failures;             /* Number of failed connections         */
    ULONG           nx_http_server_connection_successes;            /* Number of successful connections     */
    ULONG           nx_http_server_invalid_http_headers;            /* Number of invalid http headers       */
    FX_FILE         nx_http_server_file;                            /* HTTP file control block              */
    NX_TCP_SOCKET   nx_http_server_socket;                          /* HTTP Server TCP socket               */
    TX_THREAD       nx_http_server_thread;                          /* HTTP server thread                   */
#ifdef  NX_HTTP_DIGEST_ENABLE
    NX_MD5          nx_http_server_md5data;                         /* HTTP server MD5 work area            */
    NX_HTTP_SERVER_NONCE
                    nx_http_server_nonces[NX_HTTP_SERVER_NONCE_MAX];/* Nonce for digest authetication       */
#endif /* NX_HTTP_DIGEST_ENABLE */

#ifdef  NX_HTTP_MULTIPART_ENABLE
    NX_HTTP_SERVER_MULTIPART
                    nx_http_server_multipart;                       /* HTTP multipart area                  */
#endif /* NX_HTTP_MULTIPART_ENABLE */

    UINT            nx_http_server_request_type;                    /* HTTP request type                    */
    NX_HTTP_SERVER_MIME_MAP
                   *nx_http_server_mime_maps_additional;            /* Additional HTTP MIME maps            */
    UINT            nx_http_server_mime_maps_additional_num;        /* Number of additional HTTP MIME maps  */
    /* Define the user supplied routines that are used to inform the application of particular server requests.  */

    UINT (*nx_http_server_authentication_check)(struct NX_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);
    UINT (*nx_http_server_authentication_check_extended)(struct NX_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length);
    UINT (*nx_http_server_request_notify)(struct NX_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);
    UINT (*nx_http_server_invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type);  
    VOID (*nx_http_server_gmt_get)(NX_HTTP_SERVER_DATE *date);
    UINT (*nx_http_server_cache_info_get)(CHAR *resource, UINT *max_age, NX_HTTP_SERVER_DATE *last_modified);
#ifdef  NX_HTTP_DIGEST_ENABLE
    UINT (*nx_http_server_digest_authenticate_callback)(struct NX_HTTP_SERVER_STRUCT *server_ptr, CHAR *name_ptr,
                                                        CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                        CHAR *authorization_uri, CHAR *authorization_nc,
                                                        CHAR *authorization_cnonce);
#endif
} NX_HTTP_SERVER;


#ifndef NX_HTTP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_http_server_callback_data_send           _nx_http_server_callback_data_send
#define nx_http_server_callback_response_send       _nx_http_server_callback_response_send
#define nx_http_server_callback_response_send_extended _nx_http_server_callback_response_send_extended
#define nx_http_server_content_get                  _nx_http_server_content_get
#define nx_http_server_content_length_get           _nx_http_server_content_length_get
#define nx_http_server_create                       _nx_http_server_create
#define nx_http_server_delete                       _nx_http_server_delete
#define nx_http_server_param_get                    _nx_http_server_param_get
#define nx_http_server_query_get                    _nx_http_server_query_get
#define nx_http_server_start                        _nx_http_server_start
#define nx_http_server_stop                         _nx_http_server_stop
#define nx_http_server_content_get_extended         _nx_http_server_content_get_extended
#define nx_http_server_content_length_get_extended  _nx_http_server_content_length_get_extended
#define nx_http_server_get_entity_header            _nx_http_server_get_entity_header
#define nx_http_server_get_entity_content           _nx_http_server_get_entity_content
#define nx_http_server_callback_generate_response_header _nx_http_server_callback_generate_response_header
#define nx_http_server_callback_generate_response_header_extended _nx_http_server_callback_generate_response_header_extended
#define nx_http_server_callback_packet_send         _nx_http_server_callback_packet_send
#define nx_http_server_gmt_callback_set             _nx_http_server_gmt_callback_set
#define nx_http_server_cache_info_callback_set      _nx_http_server_cache_info_callback_set
#define nx_http_server_mime_maps_additional_set     _nx_http_server_mime_maps_additional_set
#define nx_http_server_type_get                     _nx_http_server_type_get
#define nx_http_server_type_get_extended            _nx_http_server_type_get_extended
#define nx_http_server_packet_content_find          _nx_http_server_packet_content_find
#define nx_http_server_packet_get                   _nx_http_server_packet_get
#define nx_http_server_invalid_userpassword_notify_set _nx_http_server_invalid_userpassword_notify_set
#define nx_http_server_digest_authenticate_notify_set _nx_http_server_digest_authenticate_notify_set
#define nx_http_server_authentication_check_set      _nx_http_server_authentication_check_set

#else

/* Services with error checking.  */

#define nx_http_server_callback_data_send           _nxe_http_server_callback_data_send
#define nx_http_server_callback_response_send       _nxe_http_server_callback_response_send
#define nx_http_server_callback_response_send_extended _nxe_http_server_callback_response_send_extended
#define nx_http_server_content_get                  _nxe_http_server_content_get
#define nx_http_server_content_length_get           _nxe_http_server_content_length_get
#define nx_http_server_create(p,n,i,m,sp,ss,pp,a,r) _nxe_http_server_create(p,n,i,m,sp,ss,pp,a,r,sizeof(NX_HTTP_SERVER))
#define nx_http_server_delete                       _nxe_http_server_delete
#define nx_http_server_param_get                    _nxe_http_server_param_get
#define nx_http_server_query_get                    _nxe_http_server_query_get
#define nx_http_server_start                        _nxe_http_server_start
#define nx_http_server_stop                         _nxe_http_server_stop
#define nx_http_server_content_get_extended         _nxe_http_server_content_get_extended
#define nx_http_server_content_length_get_extended  _nxe_http_server_content_length_get_extended
#define nx_http_server_get_entity_header            _nxe_http_server_get_entity_header
#define nx_http_server_get_entity_content           _nxe_http_server_get_entity_content
#define nx_http_server_callback_generate_response_header _nxe_http_server_callback_generate_response_header
#define nx_http_server_callback_generate_response_header_extended _nxe_http_server_callback_generate_response_header_extended
#define nx_http_server_callback_packet_send         _nxe_http_server_callback_packet_send
#define nx_http_server_gmt_callback_set             _nxe_http_server_gmt_callback_set
#define nx_http_server_cache_info_callback_set      _nxe_http_server_cache_info_callback_set
#define nx_http_server_mime_maps_additional_set     _nxe_http_server_mime_maps_additional_set
#define nx_http_server_type_get                     _nxe_http_server_type_get
#define nx_http_server_type_get_extended            _nxe_http_server_type_get_extended
#define nx_http_server_packet_content_find          _nxe_http_server_packet_content_find
#define nx_http_server_packet_get                   _nxe_http_server_packet_get
#define nx_http_server_invalid_userpassword_notify_set _nxe_http_server_invalid_userpassword_notify_set
#define nx_http_server_digest_authenticate_notify_set _nxe_http_server_digest_authenticate_notify_set
#define nx_http_server_authentication_check_set      _nxe_http_server_authentication_check_set

#endif

/* Define the prototypes accessible to the application software.  */


UINT        nx_http_server_callback_data_send(NX_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        nx_http_server_callback_response_send(NX_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        nx_http_server_callback_response_send_extended(NX_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT information_length, CHAR *additional_info, UINT additional_info_length);
UINT        nx_http_server_content_get(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        nx_http_server_packet_get(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        nx_http_server_packet_content_find(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);
UINT        nx_http_server_content_length_get(NX_PACKET *packet_ptr);
#ifdef NX_DISABLE_ERROR_CHECKING
UINT        _nx_http_server_create(NX_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                   UINT (*authentication_check)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                   UINT (*request_notify)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr));
#else
UINT        _nxe_http_server_create(NX_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                    UINT (*authentication_check)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                    UINT (*request_notify)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr),
                                    UINT http_server_size);
#endif /* NX_DISABLE_ERROR_CHECKING */
UINT        nx_http_server_delete(NX_HTTP_SERVER *http_server_ptr);
UINT        nx_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT max_param_size);
UINT        nx_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT max_query_size);
UINT        nx_http_server_start(NX_HTTP_SERVER *http_server_ptr);
UINT        nx_http_server_stop(NX_HTTP_SERVER *http_server_ptr);
UINT        nx_http_server_content_get_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        nx_http_server_content_length_get_extended(NX_PACKET *packet_ptr, ULONG *length);
UINT        nx_http_server_get_entity_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        nx_http_server_get_entity_content(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        nx_http_server_callback_generate_response_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, 
                                                             CHAR *status_code, UINT content_length, CHAR *content_type,
                                                             CHAR* additional_header);
UINT        nx_http_server_callback_generate_response_header_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, 
                                                                      UINT status_code_length, UINT content_length, CHAR *content_type, 
                                                                      UINT content_type_length, CHAR* additional_header, UINT additional_header_length);
UINT        nx_http_server_callback_packet_send(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        nx_http_server_gmt_callback_set(NX_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_HTTP_SERVER_DATE *));
UINT        nx_http_server_cache_info_callback_set(NX_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_HTTP_SERVER_DATE *));
UINT        nx_http_server_mime_maps_additional_set(NX_HTTP_SERVER *server_ptr, NX_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        nx_http_server_type_get(NX_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string);
UINT        nx_http_server_type_get_extended(NX_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length, CHAR *http_type_string, UINT http_type_string_max_size);
UINT        nx_http_server_invalid_userpassword_notify_set(NX_HTTP_SERVER *http_server_ptr, UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type )); 
UINT        nx_http_server_digest_authenticate_notify_set(NX_HTTP_SERVER *http_server_ptr,
                                                          UINT (*digest_authenticate_callback)(NX_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                          CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                          CHAR *authorization_uri, CHAR *authorization_nc,
                                                          CHAR *authorization_cnonce)); 
UINT        nx_http_server_authentication_check_set(NX_HTTP_SERVER *http_server_ptr,
                                                    UINT (*authentication_check_extended)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));

#else

/* HTTP source code is being compiled, do not perform any API mapping.  */

UINT        _nx_http_server_callback_data_send(NX_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        _nx_http_server_callback_response_send(NX_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        _nx_http_server_callback_response_send_extended(NX_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT infomation_length, CHAR *additional_info, UINT additional_info_length);
UINT        _nx_http_server_content_get(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nx_http_server_content_length_get(NX_PACKET *packet_ptr);
UINT        _nx_http_server_create(NX_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                UINT (*authentication_check)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                UINT (*request_notify)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr));
UINT        _nx_http_server_delete(NX_HTTP_SERVER *http_server_ptr);
UINT        _nx_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT max_param_size);
UINT        _nx_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT max_query_size);
UINT        _nx_http_server_start(NX_HTTP_SERVER *http_server_ptr);
UINT        _nx_http_server_stop(NX_HTTP_SERVER *http_server_ptr);
UINT        _nx_http_server_content_get_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nx_http_server_content_length_get_extended(NX_PACKET *packet_ptr, ULONG *length);
UINT        _nx_http_server_get_entity_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        _nx_http_server_get_entity_content(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        _nx_http_server_callback_generate_response_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, 
                                                              CHAR *status_code, UINT content_length, CHAR *content_type,
                                                              CHAR* additional_header);
UINT        _nx_http_server_callback_generate_response_header_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, 
                                                                       UINT status_code_length, UINT content_length, CHAR *content_type, 
                                                                       UINT content_type_length, CHAR* additional_header, UINT additional_header_length);
UINT        _nx_http_server_callback_packet_send(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nx_http_server_gmt_callback_set(NX_HTTP_SERVER *server_ptr, VOID (*gmt_callback)(NX_HTTP_SERVER_DATE *));
UINT        _nx_http_server_cache_info_callback_set(NX_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_HTTP_SERVER_DATE *));
UINT        _nx_http_server_mime_maps_additional_set(NX_HTTP_SERVER *server_ptr, NX_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        _nx_http_server_packet_content_find(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);
UINT        _nx_http_server_packet_get(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        _nx_http_server_invalid_userpassword_notify_set(NX_HTTP_SERVER *http_server_ptr, UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type )); 
UINT        _nx_http_server_digest_authenticate_notify_set(NX_HTTP_SERVER *http_server_ptr,
                                                           UINT (*digest_authenticate_callback)(NX_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                           CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                           CHAR *authorization_uri, CHAR *authorization_nc,
                                                           CHAR *authorization_cnonce)); 
UINT        _nx_http_server_authentication_check_set(NX_HTTP_SERVER *http_server_ptr,
                                                     UINT (*authentication_check_extended)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));



/* Define internal HTTP Server functions.  */

VOID        _nx_http_server_connection_present(NX_TCP_SOCKET *socket_ptr, UINT port);
VOID        _nx_http_server_thread_entry(ULONG http_server_address);
UINT        _nx_http_server_get_client_request(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
VOID        _nx_http_server_get_process(NX_HTTP_SERVER *server_ptr, UINT request_type, NX_PACKET *packet_ptr);
VOID        _nx_http_server_put_process(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
VOID        _nx_http_server_delete_process(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nx_http_server_response_send(NX_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT information_length, CHAR *additional_info, UINT additional_info_length);
UINT        _nx_http_server_basic_authenticate(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, CHAR *password_ptr, CHAR *realm_ptr, UINT realm_length, UINT *auth_request_present);
UINT        _nx_http_server_retrieve_basic_authorization(NX_PACKET *packet_ptr, CHAR *authorization_request_ptr);
UINT        _nx_http_server_retrieve_resource(NX_PACKET *packet_ptr, CHAR *destination, UINT max_size);
UINT        _nx_http_server_calculate_content_offset(NX_PACKET *packet_ptr);
UINT        _nx_http_server_type_get(NX_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string);
UINT        _nx_http_server_type_get_extended(NX_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length, CHAR *http_type_string, UINT http_type_string_max_size);
UINT        _nx_http_server_disconnect(NX_HTTP_SERVER *http_server_ptr, UINT wait_option);

#ifdef  NX_HTTP_DIGEST_ENABLE
UINT        _nx_http_server_digest_authenticate(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, UINT name_length, CHAR *password_ptr, UINT password_length, CHAR *realm_ptr, UINT realm_length, UINT *auth_request_present);
VOID        _nx_http_server_digest_response_calculate(NX_HTTP_SERVER *server_ptr, CHAR *username, UINT username_length, CHAR *realm, UINT realm_length, CHAR *password, UINT password_length, CHAR *nonce, CHAR *method, CHAR *uri, CHAR *nc, CHAR *cnonce, CHAR *result);
UINT        _nx_http_server_retrieve_digest_authorization(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *response, CHAR *uri, CHAR *nc, CHAR *cnonce, NX_HTTP_SERVER_NONCE **nonce_ptr);
VOID        _nx_http_server_hex_ascii_convert(CHAR *source, UINT source_length, CHAR *destination);
UINT        _nx_http_server_nonce_allocate(NX_HTTP_SERVER *server_ptr, NX_HTTP_SERVER_NONCE **nonce_ptr);
#endif

#ifdef  NX_HTTP_MULTIPART_ENABLE
UINT        _nx_http_server_boundary_find(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr);
#endif /* NX_HTTP_MULTIPART_ENABLE */

UINT        _nx_http_server_match_string(UCHAR *src_start, UCHAR *src_end, UCHAR *target, ULONG target_length, ULONG *match_count, UCHAR **match_end_ptr);
UINT        _nx_http_server_field_value_get(NX_PACKET *packet_ptr, UCHAR *field_name, ULONG name_length, UCHAR *field_value, ULONG field_value_size);
UINT        _nx_http_server_memicmp(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length);

UINT        _nx_http_server_generate_response_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, 
                                                     UINT status_code_length, UINT content_length, CHAR *content_type, 
                                                     UINT content_type_length, CHAR* additional_header, UINT additional_header_length);
UINT        _nx_http_server_date_to_string(NX_HTTP_SERVER_DATE *date, CHAR *string);
VOID        _nx_http_server_date_convert(UINT date, UINT count, CHAR *string);

UINT        _nxe_http_server_callback_data_send(NX_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length);
UINT        _nxe_http_server_callback_response_send(NX_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info);
UINT        _nxe_http_server_callback_response_send_extended(NX_HTTP_SERVER *server_ptr, CHAR *status_code, UINT status_code_length, CHAR *information, UINT infomation_length, CHAR *additional_info, UINT additional_info_length);
UINT        _nxe_http_server_content_get(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nxe_http_server_packet_content_find(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length);
UINT        _nxe_http_server_packet_get(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr);
UINT        _nxe_http_server_content_length_get(NX_PACKET *packet_ptr);
UINT        _nxe_http_server_content_get_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, 
                                                  CHAR *destination_ptr, UINT destination_size, UINT *actual_size);
UINT        _nxe_http_server_content_length_get_extended(NX_PACKET *packet_ptr, ULONG *content_length);
UINT        _nxe_http_server_create(NX_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                    UINT (*authentication_check)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                    UINT (*request_notify)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr),
                                    UINT http_server_size);
UINT        _nxe_http_server_delete(NX_HTTP_SERVER *http_server_ptr);
UINT        _nxe_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT max_param_size);
UINT        _nxe_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT max_query_size);
UINT        _nxe_http_server_start(NX_HTTP_SERVER *http_server_ptr);
UINT        _nxe_http_server_stop(NX_HTTP_SERVER *http_server_ptr);
UINT        _nxe_http_server_invalid_userpassword_notify_set(NX_HTTP_SERVER *http_server_ptr, 
                                                             UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type));
UINT        _nxe_http_server_type_get(NX_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string);
UINT        _nxe_http_server_type_get_extended(NX_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length, CHAR *http_type_string, UINT http_type_string_max_size);
UINT        _nxe_http_server_get_entity_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size);
UINT        _nxe_http_server_get_entity_content(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length);
UINT        _nxe_http_server_callback_generate_response_header(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, 
                                                               CHAR *status_code, UINT content_length, CHAR *content_type,
                                                               CHAR* additional_header);
UINT        _nxe_http_server_callback_generate_response_header_extended(NX_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, CHAR *status_code, 
                                                                        UINT status_code_length, UINT content_length, CHAR *content_type, 
                                                                        UINT content_type_length, CHAR* additional_header, UINT additional_header_length);
UINT        _nxe_http_server_callback_packet_send(NX_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
UINT        _nxe_http_server_gmt_callback_set(NX_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_HTTP_SERVER_DATE *));
UINT        _nxe_http_server_cache_info_callback_set(NX_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_HTTP_SERVER_DATE *));
UINT        _nxe_http_server_mime_maps_additional_set(NX_HTTP_SERVER *server_ptr, NX_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num);
UINT        _nxe_http_server_digest_authenticate_notify_set(NX_HTTP_SERVER *http_server_ptr,
                                                            UINT (*digest_authenticate_callback)(NX_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                            CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                            CHAR *authorization_uri, CHAR *authorization_nc,
                                                            CHAR *authorization_cnonce)); 
UINT        _nxe_http_server_authentication_check_set(NX_HTTP_SERVER *http_server_ptr,
                                                      UINT (*authentication_check_extended)(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length));

#endif /* NX_HTTP_SOURCE_CODE */

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_HTTP_SERVER_H */
