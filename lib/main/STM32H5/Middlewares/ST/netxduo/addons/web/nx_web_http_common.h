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
/*    nx_web_http_common.h                                PORTABLE C      */
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
/*    included, along with fx_api.h and fx_port.h. If HTTPS is being used */
/*    then nx_secure_tls_api.h must also be included.                     */
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

#ifndef NX_WEB_HTTP_COMMON_H
#define NX_WEB_HTTP_COMMON_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Define the HTTP version.  */
#define NX_WEB_HTTP_VERSION                     "HTTP/1.1"
    
/* Define HTTP TCP socket create options.  */

#ifndef NX_WEB_HTTP_TYPE_OF_SERVICE
#define NX_WEB_HTTP_TYPE_OF_SERVICE             NX_IP_NORMAL
#endif

#ifndef NX_WEB_HTTP_FRAGMENT_OPTION
#define NX_WEB_HTTP_FRAGMENT_OPTION             NX_DONT_FRAGMENT
#endif

#ifndef NX_WEB_HTTP_TIME_TO_LIVE
#define NX_WEB_HTTP_TIME_TO_LIVE                0x80
#endif

#ifndef NX_WEB_HTTP_MAX_RESOURCE
#define NX_WEB_HTTP_MAX_RESOURCE                40
#endif

#ifndef NX_WEB_HTTP_MAX_NAME
#define NX_WEB_HTTP_MAX_NAME                    20
#endif

#ifndef NX_WEB_HTTP_MAX_PASSWORD
#define NX_WEB_HTTP_MAX_PASSWORD                20
#endif
  
/* To enabled HTTPS, define this symbol 
#define NX_WEB_HTTPS_ENABLE
*/

/* To enable MD5 digest authentication, define this symbol 
#define NX_WEB_HTTP_DIGEST_ENABLE
*/    

#ifndef NX_PHYSICAL_TRAILER
#define NX_PHYSICAL_TRAILER                     4
#endif

/* NX_WEB_HTTP_MAX_STRING is base64 of "name:password" and plus 1 if an extra conversion is needed and plus 2 pad if needed.. */
#define NX_WEB_HTTP_MAX_STRING                  ((NX_WEB_HTTP_MAX_NAME + NX_WEB_HTTP_MAX_PASSWORD  + 1 ) * 4 / 3 + 1 + 2)

#define NX_WEB_HTTP_MAX_BINARY_MD5              16
#define NX_WEB_HTTP_MAX_ASCII_MD5               32


/* Define return code constants.  */
#define NX_WEB_HTTP_ERROR                       0x30000        /* HTTP internal error                                  */
#define NX_WEB_HTTP_TIMEOUT                     0x30001        /* HTTP timeout occurred                                */
#define NX_WEB_HTTP_FAILED                      0x30002        /* HTTP error                                           */
#define NX_WEB_HTTP_DONT_AUTHENTICATE           0x30003        /* HTTP authentication not needed                       */
#define NX_WEB_HTTP_BASIC_AUTHENTICATE          0x30004        /* HTTP basic authentication requested                  */
#define NX_WEB_HTTP_DIGEST_AUTHENTICATE         0x30005        /* HTTP digest authentication requested                 */
#define NX_WEB_HTTP_NOT_FOUND                   0x30006        /* HTTP request not found                               */
#define NX_WEB_HTTP_DATA_END                    0x30007        /* HTTP end of content area                             */
#define NX_WEB_HTTP_CALLBACK_COMPLETED          0x30008        /* HTTP user callback completed the processing          */
#define NX_WEB_HTTP_POOL_ERROR                  0x30009        /* HTTP supplied pool payload is too small              */
#define NX_WEB_HTTP_NOT_READY                   0x3000A        /* HTTP client not ready for operation                  */
#define NX_WEB_HTTP_GET_DONE                    0x3000C        /* HTTP client get is complete                          */
#define NX_WEB_HTTP_BAD_PACKET_LENGTH           0x3000D        /* Invalid packet received - length incorrect           */
#define NX_WEB_HTTP_REQUEST_UNSUCCESSFUL_CODE   0x3000E        /* Received an error code instead of 2xx from server    */
#define NX_WEB_HTTP_INCOMPLETE_PUT_ERROR        0x3000F        /* Server responds before PUT is complete               */
#define NX_WEB_HTTP_PASSWORD_TOO_LONG           0x30011        /* Password exceeded expected length                    */
#define NX_WEB_HTTP_USERNAME_TOO_LONG           0x30012        /* Username exceeded expected length                    */
#define NX_WEB_HTTP_NO_QUERY_PARSED             0x30013        /* Server unable to find query in client request        */
#define NX_WEB_HTTP_METHOD_ERROR                0x30014        /* Client method (e.g. GET, POST) was missing required information. */
#define NX_WEB_HTTP_IMPROPERLY_TERMINATED_PARAM 0x30015        /* Client request parameter not properly terminated     */
#define NX_WEB_HTTP_BOUNDARY_ALREADY_FOUND      0x30016        /* Boundary is already found.                           */
#define NX_WEB_HTTP_MISSING_CONTENT_LENGTH      0x30017        /* The Content-Length header was not found.             */
#define NX_WEB_HTTP_EXTENSION_NOT_FOUND         0x30018        /* A searched-for HTTP type extension was not found.    */
#define NX_WEB_HTTP_EXTENSION_MIME_DEFAULT      0x30019        /* No matching extension found, return default.         */
#define NX_WEB_HTTP_STATUS_CODE_CONTINUE               0x3001A        /* "100 Continue"                                       */
#define NX_WEB_HTTP_STATUS_CODE_SWITCHING_PROTOCOLS    0x3001B        /* "101 Switching Protocols"                            */
#define NX_WEB_HTTP_STATUS_CODE_CREATED                0x3001C        /* "201 Created"                                        */
#define NX_WEB_HTTP_STATUS_CODE_ACCEPTED               0x3001D        /* "202 Accepted"                                       */
#define NX_WEB_HTTP_STATUS_CODE_NON_AUTH_INFO          0x3001E        /* "203 Non-Authoritative Information"                  */
#define NX_WEB_HTTP_STATUS_CODE_NO_CONTENT             0x3001F        /* "204 No Content"                                     */
#define NX_WEB_HTTP_STATUS_CODE_RESET_CONTENT          0x30020        /* "205 Reset Content"                                  */
#define NX_WEB_HTTP_STATUS_CODE_PARTIAL_CONTENT        0x30021        /* "206 Partial Content"                                */
#define NX_WEB_HTTP_STATUS_CODE_MULTIPLE_CHOICES       0x30022        /* "300 Multiple Choices"                               */
#define NX_WEB_HTTP_STATUS_CODE_MOVED_PERMANETLY       0x30023        /* "301 Moved Permanently"                              */
#define NX_WEB_HTTP_STATUS_CODE_FOUND                  0x30024        /* "302 Found"                                          */
#define NX_WEB_HTTP_STATUS_CODE_SEE_OTHER              0x30025        /* "303 See Other"                                      */
#define NX_WEB_HTTP_STATUS_CODE_NOT_MODIFIED           0x30026        /* "304 Not Modified"                                   */
#define NX_WEB_HTTP_STATUS_CODE_USE_PROXY              0x30027        /* "305 Use Proxy"                                      */
#define NX_WEB_HTTP_STATUS_CODE_TEMPORARY_REDIRECT     0x30028        /* "307 Temporary Redirect"                             */
#define NX_WEB_HTTP_STATUS_CODE_BAD_REQUEST            0x30029        /* "400 Bad Request"                                    */
#define NX_WEB_HTTP_STATUS_CODE_UNAUTHORIZED           0x3002A        /* "401 Unauthorized"                                   */
#define NX_WEB_HTTP_STATUS_CODE_PAYMENT_REQUIRED       0x3002B        /* "402 Payment Required"                               */
#define NX_WEB_HTTP_STATUS_CODE_FORBIDDEN              0x3002C        /* "403 Forbidden"                                      */
#define NX_WEB_HTTP_STATUS_CODE_NOT_FOUND              0x3002D        /* "404 Not Found"                                      */
#define NX_WEB_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED     0x3002E        /* "405 Method Not Allowed"                             */
#define NX_WEB_HTTP_STATUS_CODE_NOT_ACCEPTABLE         0x3002F        /* "406 Not Acceptable"                                 */
#define NX_WEB_HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED    0x30030        /* "407 Proxy Authentication Required"                  */
#define NX_WEB_HTTP_STATUS_CODE_REQUEST_TIMEOUT        0x30031        /* "408 Request Time-out"                               */
#define NX_WEB_HTTP_STATUS_CODE_CONFLICT               0x30032        /* "409 Conflict"                                       */
#define NX_WEB_HTTP_STATUS_CODE_GONE                   0x30033        /* "410 Gone"                                           */
#define NX_WEB_HTTP_STATUS_CODE_LENGTH_REQUIRED        0x30034        /* "411 Length Required"                                */
#define NX_WEB_HTTP_STATUS_CODE_PRECONDITION_FAILED    0x30035        /* "412 Precondition Failed"                            */
#define NX_WEB_HTTP_STATUS_CODE_ENTITY_TOO_LARGE       0x30036        /* "413 Request Entity Too Large"                       */
#define NX_WEB_HTTP_STATUS_CODE_URL_TOO_LARGE          0x30037        /* "414 Request-URL Too Large"                          */
#define NX_WEB_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA      0x30038        /* "415 Unsupported Media Type"                         */
#define NX_WEB_HTTP_STATUS_CODE_RANGE_NOT_SATISFY      0x30039        /* "416 Requested range not satisfiable"                */
#define NX_WEB_HTTP_STATUS_CODE_EXPECTATION_FAILED     0x3003A        /* "417 Expectation Failed"                             */
#define NX_WEB_HTTP_STATUS_CODE_INTERNAL_ERROR         0x3003B        /* "500 Internal Server Error"                          */
#define NX_WEB_HTTP_STATUS_CODE_NOT_IMPLEMENTED        0x3003C        /* "501 Not Implemented"                                */
#define NX_WEB_HTTP_STATUS_CODE_BAD_GATEWAY            0x3003D        /* "502 Bad Gateway"                                    */
#define NX_WEB_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE    0x3003E        /* "503 Service Unavailable"                            */
#define NX_WEB_HTTP_STATUS_CODE_GATEWAY_TIMEOUT        0x3003F        /* "504 Gateway Time-out"                               */
#define NX_WEB_HTTP_STATUS_CODE_VERSION_ERROR          0x30040        /* "505 HTTP Version not supported"                     */
#define NX_WEB_HTTP_AUTHENTICATION_ERROR               NX_WEB_HTTP_STATUS_CODE_UNAUTHORIZED        /* HTTP client authentication failed                    */

/* Define the HTTP Server TCP port number */

#define NX_WEB_HTTP_SERVER_PORT               80        /* Port for HTTP server                                 */
#define NX_WEB_HTTPS_SERVER_PORT             443        /* Port for HTTPS server.                               */

/* Define constants for the various HTTP methods supported. */
#define NX_WEB_HTTP_METHOD_NONE             0x0
#define NX_WEB_HTTP_METHOD_GET              0x1
#define NX_WEB_HTTP_METHOD_PUT              0x2
#define NX_WEB_HTTP_METHOD_POST             0x3
#define NX_WEB_HTTP_METHOD_DELETE           0x4
#define NX_WEB_HTTP_METHOD_HEAD             0x5

/* Define status codes. */
#define NX_WEB_HTTP_STATUS_CONTINUE             "100 Continue"
#define NX_WEB_HTTP_STATUS_SWITCHING_PROTOCOLS  "101 Switching Protocols"
#define NX_WEB_HTTP_STATUS_OK                   "200 OK"
#define NX_WEB_HTTP_STATUS_CREATED              "201 Created"
#define NX_WEB_HTTP_STATUS_ACCEPTED             "202 Accepted"
#define NX_WEB_HTTP_STATUS_NON_AUTH_INFO        "203 Non-Authoritative Information"
#define NX_WEB_HTTP_STATUS_NO_CONTENT           "204 No Content"
#define NX_WEB_HTTP_STATUS_RESET_CONTENT        "205 Reset Content"
#define NX_WEB_HTTP_STATUS_PARTIAL_CONTENT      "206 Partial Content"
#define NX_WEB_HTTP_STATUS_MULTIPLE_CHOICES     "300 Multiple Choices"
#define NX_WEB_HTTP_STATUS_MOVED_PERMANETLY     "301 Moved Permanently"
#define NX_WEB_HTTP_STATUS_FOUND                "302 Found"
#define NX_WEB_HTTP_STATUS_SEE_OTHER            "303 See Other"
#define NX_WEB_HTTP_STATUS_NOT_MODIFIED         "304 Not Modified"
#define NX_WEB_HTTP_STATUS_USE_PROXY            "305 Use Proxy"
#define NX_WEB_HTTP_STATUS_TEMPORARY_REDIRECT   "307 Temporary Redirect"
#define NX_WEB_HTTP_STATUS_BAD_REQUEST          "400 Bad Request"
#define NX_WEB_HTTP_STATUS_UNAUTHORIZED         "401 Unauthorized"
#define NX_WEB_HTTP_STATUS_PAYMENT_REQUIRED     "402 Payment Required"
#define NX_WEB_HTTP_STATUS_FORBIDDEN            "403 Forbidden"
#define NX_WEB_HTTP_STATUS_NOT_FOUND            "404 Not Found"
#define NX_WEB_HTTP_STATUS_METHOD_NOT_ALLOWED   "405 Method Not Allowed"
#define NX_WEB_HTTP_STATUS_NOT_ACCEPTABLE       "406 Not Acceptable"
#define NX_WEB_HTTP_STATUS_PROXY_AUTH_REQUIRED  "407 Proxy Authentication Required"
#define NX_WEB_HTTP_STATUS_REQUEST_TIMEOUT      "408 Request Time-out"
#define NX_WEB_HTTP_STATUS_CONFLICT             "409 Conflict"
#define NX_WEB_HTTP_STATUS_GONE                 "410 Gone"
#define NX_WEB_HTTP_STATUS_LENGTH_REQUIRED      "411 Length Required"
#define NX_WEB_HTTP_STATUS_PRECONDITION_FAILED  "412 Precondition Failed"
#define NX_WEB_HTTP_STATUS_ENTITY_TOO_LARGE     "413 Request Entity Too Large"
#define NX_WEB_HTTP_STATUS_URL_TOO_LARGE        "414 Request-URL Too Large"
#define NX_WEB_HTTP_STATUS_UNSUPPORTED_MEDIA    "415 Unsupported Media Type"
#define NX_WEB_HTTP_STATUS_RANGE_NOT_SATISFY    "416 Requested range not satisfiable"
#define NX_WEB_HTTP_STATUS_EXPECTATION_FAILED   "417 Expectation Failed"
#define NX_WEB_HTTP_STATUS_INTERNAL_ERROR       "500 Internal Server Error"
#define NX_WEB_HTTP_STATUS_NOT_IMPLEMENTED      "501 Not Implemented"
#define NX_WEB_HTTP_STATUS_BAD_GATEWAY          "502 Bad Gateway"
#define NX_WEB_HTTP_STATUS_SERVICE_UNAVAILABLE  "503 Service Unavailable"
#define NX_WEB_HTTP_STATUS_GATEWAY_TIMEOUT      "504 Gateway Time-out"
#define NX_WEB_HTTP_STATUS_VERSION_ERROR        "505 HTTP Version not supported"


/* Define the max length of header field. */

#ifndef NX_WEB_HTTP_MAX_HEADER_FIELD            
#define NX_WEB_HTTP_MAX_HEADER_FIELD            256
#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NX_WEB_HTTP_COMMON_H */
