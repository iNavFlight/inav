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
/**   Hypertext Transfer Protocol (HTTP) and                              */
/**   Hypertext Transfer Protocol Secure (HTTPS using TLS)                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_WEB_HTTP_CLIENT_SOURCE_CODE

/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nx_ipv6.h"

#include    "nx_web_http_client.h"

#include    "stdio.h"
#include    "string.h"


/* Define global HTTPS variables and strings.  */

/* Define status maps. */
static NX_WEB_HTTP_CLIENT_STATUS_MAP _nx_web_http_client_status_maps[] =
{
    {"200",    NX_SUCCESS},
    {"100",    NX_WEB_HTTP_STATUS_CODE_CONTINUE},
    {"101",    NX_WEB_HTTP_STATUS_CODE_SWITCHING_PROTOCOLS},
    {"201",    NX_WEB_HTTP_STATUS_CODE_CREATED},
    {"202",    NX_WEB_HTTP_STATUS_CODE_ACCEPTED},
    {"203",    NX_WEB_HTTP_STATUS_CODE_NON_AUTH_INFO},
    {"204",    NX_WEB_HTTP_STATUS_CODE_NO_CONTENT},
    {"205",    NX_WEB_HTTP_STATUS_CODE_RESET_CONTENT},
    {"206",    NX_WEB_HTTP_STATUS_CODE_PARTIAL_CONTENT},
    {"300",    NX_WEB_HTTP_STATUS_CODE_MULTIPLE_CHOICES},
    {"301",    NX_WEB_HTTP_STATUS_CODE_MOVED_PERMANETLY},
    {"302",    NX_WEB_HTTP_STATUS_CODE_FOUND},
    {"303",    NX_WEB_HTTP_STATUS_CODE_SEE_OTHER},
    {"304",    NX_WEB_HTTP_STATUS_CODE_NOT_MODIFIED},
    {"305",    NX_WEB_HTTP_STATUS_CODE_USE_PROXY},
    {"307",    NX_WEB_HTTP_STATUS_CODE_TEMPORARY_REDIRECT},
    {"400",    NX_WEB_HTTP_STATUS_CODE_BAD_REQUEST},
    {"401",    NX_WEB_HTTP_STATUS_CODE_UNAUTHORIZED},
    {"402",    NX_WEB_HTTP_STATUS_CODE_PAYMENT_REQUIRED},
    {"403",    NX_WEB_HTTP_STATUS_CODE_FORBIDDEN},
    {"404",    NX_WEB_HTTP_STATUS_CODE_NOT_FOUND},
    {"405",    NX_WEB_HTTP_STATUS_CODE_METHOD_NOT_ALLOWED},
    {"406",    NX_WEB_HTTP_STATUS_CODE_NOT_ACCEPTABLE},
    {"407",    NX_WEB_HTTP_STATUS_CODE_PROXY_AUTH_REQUIRED},
    {"408",    NX_WEB_HTTP_STATUS_CODE_REQUEST_TIMEOUT},
    {"409",    NX_WEB_HTTP_STATUS_CODE_CONFLICT},
    {"410",    NX_WEB_HTTP_STATUS_CODE_GONE},
    {"411",    NX_WEB_HTTP_STATUS_CODE_LENGTH_REQUIRED},
    {"412",    NX_WEB_HTTP_STATUS_CODE_PRECONDITION_FAILED},
    {"413",    NX_WEB_HTTP_STATUS_CODE_ENTITY_TOO_LARGE},
    {"414",    NX_WEB_HTTP_STATUS_CODE_URL_TOO_LARGE},
    {"415",    NX_WEB_HTTP_STATUS_CODE_UNSUPPORTED_MEDIA},
    {"416",    NX_WEB_HTTP_STATUS_CODE_RANGE_NOT_SATISFY},
    {"417",    NX_WEB_HTTP_STATUS_CODE_EXPECTATION_FAILED},
    {"500",    NX_WEB_HTTP_STATUS_CODE_INTERNAL_ERROR},
    {"501",    NX_WEB_HTTP_STATUS_CODE_NOT_IMPLEMENTED},
    {"502",    NX_WEB_HTTP_STATUS_CODE_BAD_GATEWAY},
    {"503",    NX_WEB_HTTP_STATUS_CODE_SERVICE_UNAVAILABLE},
    {"504",    NX_WEB_HTTP_STATUS_CODE_GATEWAY_TIMEOUT},
    {"505",    NX_WEB_HTTP_STATUS_CODE_VERSION_ERROR},
};

static UINT _nx_web_http_client_status_maps_size = sizeof(_nx_web_http_client_status_maps)/sizeof(NX_WEB_HTTP_CLIENT_STATUS_MAP);

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_create                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client create call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    client_name                           Name of HTTP client           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*    window_size                           Size of HTTP client rx window */ 
/*    http_client_size                      Size of HTTP client           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_create            Actual client create call     */
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
UINT  _nxe_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size, UINT http_client_size)
{

NX_PACKET   *packet_ptr;
UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id == NX_WEB_HTTP_CLIENT_ID) || 
        (pool_ptr == NX_NULL) || (http_client_size != sizeof(NX_WEB_HTTP_CLIENT)))
        return(NX_PTR_ERROR);

    /* Pickup a packet from the supplied packet pool.  */
    packet_ptr =  pool_ptr -> nx_packet_pool_available_list;

    /* Determine if the packet payload is equal to or greater than the maximum HTTP header supported.  */
    if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start) < NX_WEB_HTTP_CLIENT_MIN_PACKET_SIZE)
        return(NX_WEB_HTTP_POOL_ERROR);

    /* Call actual client create function.  */
    status =  _nx_web_http_client_create(client_ptr, client_name, ip_ptr, pool_ptr, window_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_create                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a HTTP client on the specified IP. In doing   */ 
/*    so this function creates an TCP socket for subsequent HTTP          */ 
/*    transfers.                                                          */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    client_name                           Name of HTTP client           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*    window_size                           Size of HTTP client rx window */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_create                  Create HTTP client socket     */ 
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
UINT  _nx_web_http_client_create(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size)
{

UINT        status;


    /* Clear the HTTP Client structure.  */
    memset((void *) client_ptr, 0, sizeof(NX_WEB_HTTP_CLIENT));

    /* Create the Client's TCP socket.  */
    status =  nx_tcp_socket_create(ip_ptr, &(client_ptr -> nx_web_http_client_socket), client_name, 
                                   NX_WEB_HTTP_TYPE_OF_SERVICE,  NX_WEB_HTTP_FRAGMENT_OPTION, NX_WEB_HTTP_TIME_TO_LIVE,
                                   window_size, NX_NULL, NX_NULL);

    /* Determine if an error occurred.   */
    if (status != NX_SUCCESS)
    {

        /* Yes, return error code.  */
        return(status);
    }

    /* Save the Client name.  */
    client_ptr -> nx_web_http_client_name =  client_name;

    /* Save the IP pointer address.  */
    client_ptr -> nx_web_http_client_ip_ptr =  ip_ptr;

    /* Save the packet pool pointer.  */
    client_ptr -> nx_web_http_client_packet_pool_ptr =  pool_ptr;

    /* Set the client state to ready to indicate a get or put operation can be done.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_READY;

    /* Set the current method to "NONE". */
    client_ptr -> nx_web_http_client_method = NX_WEB_HTTP_METHOD_NONE;

    /* Set the Client ID to indicate the HTTP client thread is ready.  */
    client_ptr -> nx_web_http_client_id =  NX_WEB_HTTP_CLIENT_ID;

    /* Set the default port the client connects to the HTTP server on (80). */
    client_ptr -> nx_web_http_client_connect_port = NX_WEB_HTTP_SERVER_PORT;


    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_delete                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client delete call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_delete            Actual client delete call     */
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
UINT  _nxe_web_http_client_delete(NX_WEB_HTTP_CLIENT *client_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client delete function.  */
    status =  _nx_web_http_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_delete                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created HTTP client on the       */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_delete                  Delete the HTTP client socket */ 
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
UINT  _nx_web_http_client_delete(NX_WEB_HTTP_CLIENT *client_ptr)
{

#ifdef NX_WEB_HTTPS_ENABLE

    /* End TLS session if using HTTPS. */
    if (client_ptr->nx_web_http_client_is_https)
    {
        nx_secure_tls_session_end(&(client_ptr -> nx_web_http_client_tls_session), NX_WAIT_FOREVER);
        nx_secure_tls_session_delete(&(client_ptr -> nx_web_http_client_tls_session));
    }
#endif

    /* Disconnect and unbind the socket.  */
    nx_tcp_socket_disconnect(&(client_ptr -> nx_web_http_client_socket), NX_WEB_HTTP_CLIENT_TIMEOUT);
    nx_tcp_client_socket_unbind(&(client_ptr -> nx_web_http_client_socket));

    /* Delete the TCP socket.  */
    nx_tcp_socket_delete(&(client_ptr -> nx_web_http_client_socket));

    /* Clear the client ID to indicate the HTTP client is no longer ready.  */
    client_ptr -> nx_web_http_client_id =  0;

    /* Release request packet.  */
    if (client_ptr -> nx_web_http_client_request_packet_ptr)
    {
        nx_packet_release(client_ptr -> nx_web_http_client_request_packet_ptr);
        client_ptr -> nx_web_http_client_request_packet_ptr = NX_NULL;
    }

    /* Release response packet.  */
    if (client_ptr -> nx_web_http_client_response_packet)
    {
        nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
        client_ptr -> nx_web_http_client_response_packet = NX_NULL;
    }

    /* Clear out the entire structure in case it is used again. */
    memset(client_ptr, 0, sizeof(NX_WEB_HTTP_CLIENT));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_content_type_header_add         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is used to add the Content-Type HTTP header  */
/*    to an outgoing request. It determines the resource type from the    */
/*    resource string and then adds the complete header to the current    */
/*    HTTP request packet allocated in the control block.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    resource                              Server resource name          */
/*    wait_option                           Timeout for called functions  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_type_get          Get resource content type     */
/*    _nx_web_http_client_request_header_add                              */
/*                                          Add header to request packet  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
UINT _nx_web_http_client_content_type_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *resource, ULONG wait_option)
{
UINT        j;
CHAR        string1[20];

    /* Now build the content-type entry.  */
    j = _nx_web_http_client_type_get(resource, string1);
    _nx_web_http_client_request_header_add(client_ptr, "Content-Type", 12, string1, j, wait_option);

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_content_length_header_add       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is used to add the Content-Length HTTP       */
/*    header to an outgoing request. It converts the passed-in length     */
/*    value to an HTTP string and then adds the complete header to the    */
/*    current HTTP request packet allocated in the control block.         */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    total_bytes                           Content length value          */
/*    wait_option                           Timeout for called functions  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_number_convert    Convert length to string      */
/*    _nx_web_http_client_request_header_add                              */
/*                                          Add header to request packet  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
UINT _nx_web_http_client_content_length_header_add(NX_WEB_HTTP_CLIENT *client_ptr, ULONG total_bytes, ULONG wait_option)
{
UINT        j;
CHAR        string1[20];

    /* Now build the content-length entry.  */
    j =  _nx_web_http_client_number_convert(total_bytes, string1);
    _nx_web_http_client_request_header_add(client_ptr, "Content-Length", 14, string1, j, wait_option);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_get_start                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP Duo client get start    */ 
/*    call.                                                               */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */ 
/*    password                              Pointer to password           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start         Actual client get start call  */
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
UINT  _nxe_web_http_client_get_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, 
                                     CHAR *resource, CHAR *host, CHAR *username, CHAR *password, 
                                     ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nx_web_http_client_get_start(client_ptr, server_ip, server_port, resource,
                                            host, username, password, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the application GET request.  The specified */ 
/*    resource (URL) is requested from the HTTP Server at the specified   */ 
/*    IP address. The response is processed by calling                    */
/*    nx_web_http_client_response_body_get.                               */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */ 
/*    password                              Pointer to password           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start_extended                              */
/*                                          Actual client get start call  */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_get_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                    CHAR *resource, CHAR *host,
                                    CHAR *username, CHAR *password, ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_get_start_extended(client_ptr, server_ip, server_port, resource,
                                                  temp_resource_length, host, temp_host_length, username,
                                                  temp_username_length, password, temp_password_length,
                                                  wait_option));

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_get_start_extended             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP Duo client get start    */ 
/*    call with string length.                                            */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start_extended                              */
/*                                          Actual client get start call  */
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
UINT  _nxe_web_http_client_get_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                              UINT server_port, CHAR *resource, UINT resource_length,
                                              CHAR *host, UINT host_length, CHAR *username,
                                              UINT username_length, CHAR *password,
                                              UINT password_length, ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nx_web_http_client_get_start_extended(client_ptr, server_ip, server_port, resource,
                                                     resource_length, host, host_length, username,
                                                     username_length, password, password_length,
                                                     wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start_extended              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the application GET request.  The specified */ 
/*    resource (URL) is requested from the HTTP Server at the specified   */ 
/*    IP address. The response is processed by calling                    */
/*    nx_web_http_client_response_body_get.                               */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_connect           Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
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
UINT  _nx_web_http_client_get_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                             UINT server_port, CHAR *resource, UINT resource_length,
                                             CHAR *host, UINT host_length, CHAR *username,
                                             UINT username_length, CHAR *password,
                                             UINT password_length, ULONG wait_option)
{

UINT        status;

    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_GET,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);
    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the GET state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_GET;

    return(status);
}

#ifdef NX_WEB_HTTPS_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_get_secure_start               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure GET request     */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_get_secure_start  Actual GET request processing */
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
UINT  _nxe_web_http_client_get_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                            CHAR *host, CHAR *username, CHAR *password,
                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                            ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nx_web_http_client_get_secure_start(client_ptr, server_ip, server_port, resource,
                                                   host, username, password, tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_get_secure_start                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application GET request. The specified   */
/*    resource (URL) is requested from the HTTP Server at the specified   */
/*    IP address. This version of the function also requires a TLS setup  */
/*    callback as the request is sent over TLS-secured HTTPS.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_get_secure_start_extended                       */
/*                                          Actual GET request processing */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_get_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                           CHAR *host, CHAR *username, CHAR *password,
                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                           ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_get_secure_start_extended(client_ptr, server_ip, server_port,
                                                         resource, temp_resource_length, host,
                                                         temp_host_length, username,
                                                         temp_username_length, password,
                                                         temp_password_length, tls_setup,
                                                         wait_option));
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_get_secure_start_extended      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure GET request     */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_get_secure_start_extended                       */
/*                                          Actual GET request processing */
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
UINT _nxe_web_http_client_get_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                    CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                    CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                    UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                    ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status = _nx_web_http_client_get_secure_start_extended(client_ptr, server_ip, server_port,
                                                           resource, resource_length, host,
                                                           host_length, username, username_length,
                                                           password, password_length, tls_setup,
                                                           wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_get_secure_start_extended       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application GET request. The specified   */
/*    resource (URL) is requested from the HTTP Server at the specified   */
/*    IP address. This version of the function also requires a TLS setup  */
/*    callback as the request is sent over TLS-secured HTTPS.             */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_secure_connect    Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
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
UINT _nx_web_http_client_get_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                   CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                   CHAR *username, UINT username_length, CHAR *password, UINT password_length,
                                                   UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                   ULONG wait_option)
{

UINT        status;

    /* Use default HTTPS port. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, tls_setup, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_GET,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the GET state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_GET;

    return(status);
}

#endif /* NX_WEB_HTTPS_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_get_server_response             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the initial response from the remote        */
/*    server. The body of the response is retrieved by the application    */
/*    by calling the GET response body API.                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to packet             */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_receive           Receive packet                */
/*    nx_packet_data_append                 Put data into return packet   */
/*    nx_packet_copy                        Copy packet data              */
/*    nx_packet_release                     Release processed packets     */
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
UINT  _nx_web_http_client_get_server_response(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

NX_PACKET   *head_packet_ptr;
NX_PACKET   *new_packet_ptr;
CHAR        *buffer_ptr;
UINT        status;
NX_PACKET   *work_ptr;
UINT        crlf_found = 0;
NX_PACKET   *tmp_ptr;


    /* Default the return packet pointer to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Wait for a response from server.  */
    status = _nx_web_http_client_receive(client_ptr, &head_packet_ptr, wait_option);

    /* Check the return status.  */
    if (status != NX_SUCCESS)
    {

        /* Return an error condition.  */
        return(status);
    }

    crlf_found = 0;
    work_ptr = head_packet_ptr;
    
    /* Build a pointer to the buffer area.  */
    buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr;
    
    do
    {
    
        /* See if there is a blank line present in the buffer.  */
        /* Search the buffer for a cr/lf pair.  */
        while (buffer_ptr < (CHAR *) work_ptr -> nx_packet_append_ptr)
        {
            if (!(crlf_found & 1) && (*buffer_ptr == (CHAR)13))
            {

                /* Found CR. */
                crlf_found++;
            }
            else if((crlf_found & 1) && (*buffer_ptr == (CHAR)10))
            {

                /* Found LF. */
                crlf_found++;
            }
            else
            {

                /* Reset the CRLF marker. */
                crlf_found = 0;
            }

            if (crlf_found == 4)
            {

                /* Yes, we have found the end of the HTTP response header.  */

                /* Set the return packet pointer.  */
                *packet_ptr =  head_packet_ptr;

                /* Return a successful completion.  */
                return(NX_SUCCESS);
            }
    
            /* Move the buffer pointer up.  */
            buffer_ptr++;
        }

        /* Determine if the packet has already overflowed into another packet.  */

#ifndef NX_DISABLE_PACKET_CHAIN

        if (work_ptr -> nx_packet_next != NX_NULL)
        {

            /* Get the next packet in the chain. */
            work_ptr  = work_ptr -> nx_packet_next;
            buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr; 
        }
        else
#endif
        {
            /* Receive another packet from the HTTP server port.  */
            status = _nx_web_http_client_receive(client_ptr, &new_packet_ptr, wait_option);
            
            /* Check the return status.  */
            if (status != NX_SUCCESS)
            {

                /* Release the current head packet.  */
                nx_packet_release(head_packet_ptr);
            
                /* Return an error condition.  */
                return(status);
            }

            /* Successfully received another packet.  Its contents now need to be placed in the head packet.  */
            tmp_ptr = new_packet_ptr;

#ifndef NX_DISABLE_PACKET_CHAIN
            while (tmp_ptr)
            {
#endif /* NX_DISABLE_PACKET_CHAIN */

                /* Copy the contents of the current packet into the head packet.  */
                status =  nx_packet_data_append(head_packet_ptr, (VOID *) tmp_ptr -> nx_packet_prepend_ptr,
                                                (ULONG)(tmp_ptr -> nx_packet_append_ptr - tmp_ptr -> nx_packet_prepend_ptr),
                                                client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);

                /* Determine if an error occurred.  */
                if (status != NX_SUCCESS)
                {

                    /* Yes, an error is present.  */

                    /* Release both packets.  */
                    nx_packet_release(head_packet_ptr);
                    nx_packet_release(new_packet_ptr);
        
                    /* Return an error condition.  */
                    return(status);
                }

#ifndef NX_DISABLE_PACKET_CHAIN
                tmp_ptr = tmp_ptr -> nx_packet_next;
            }
#endif /* NX_DISABLE_PACKET_CHAIN */

            /* Release the new packet. */
            nx_packet_release(new_packet_ptr);
        }

    } while (status == NX_SUCCESS);

    /* Release the packet.  */
    nx_packet_release(head_packet_ptr);

    return(NX_WEB_HTTP_ERROR);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_response_body_get              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client get packet call. */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    packet_ptr                            Destination for packet pointer*/ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_response_body_get Actual client get packet call */
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
UINT  _nxe_web_http_client_response_body_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET packet routine.  */
    status =  _nx_web_http_client_response_body_get(client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_response_body_get               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets a data packet associated with the resource       */ 
/*    specified by the previous GET start request. It can be called       */
/*    repeatedly to get all of the packets for larger resources.          */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    packet_ptr                            Destination for packet pointer*/ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_receive           Receive a resource data packet*/
/*    _nx_web_http_client_get_server_response                             */
/*                                          Get server response header    */
/*    _nx_web_http_client_content_length_get                              */
/*                                          Get Content Length value      */
/*    _nx_web_http_client_process_header_fields                           */
/*                                          Process HTTP header fields    */
/*   _nx_web_http_client_error_exit         Close HTTP(S) session         */
/*   _nx_web_http_client_response_chunked_get                             */
/*                                          Get chunked response packet   */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            released invalid packet,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_client_response_body_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_pptr, ULONG wait_option)
{

UINT        status = NX_SUCCESS;
UINT        length = 0, offset = 0;
NX_PACKET   *response_packet_ptr = NX_NULL;
CHAR        *buffer_ptr;
UINT        i = 0;
UINT        status_code = NX_SUCCESS;


    /* Check if the request packet is sent out.  */
    if ((client_ptr -> nx_web_http_client_state < NX_WEB_HTTP_CLIENT_STATE_GET) ||
        (client_ptr -> nx_web_http_client_state > NX_WEB_HTTP_CLIENT_STATE_DELETE))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Default the return packet to NULL.  */
    *packet_pptr =  NX_NULL;

    /* Check for a response from the Server.  */
    if (!(client_ptr -> nx_web_http_client_response_header_received))
    {

        /* Pickup the response from the Server.  */
        status =  _nx_web_http_client_get_server_response(client_ptr, &response_packet_ptr, wait_option);

        /* Validate the response packet length.  */
        if (status == NX_SUCCESS)
        {
            if (response_packet_ptr -> nx_packet_append_ptr - response_packet_ptr -> nx_packet_prepend_ptr < 12)
            {

                /* Release invalid packet.  */
                nx_packet_release(response_packet_ptr);
                status = NX_WEB_HTTP_ERROR;
            }
        }

        /* Check for error processing received packet. */
        if (status != NX_SUCCESS)
        {

            if (status != NX_NO_PACKET)
            {
                /* Disconnect and unbind the socket.  */
                _nx_web_http_client_error_exit(client_ptr, wait_option);

                /* Reenter the READY state.  */
                client_ptr->nx_web_http_client_state = NX_WEB_HTTP_CLIENT_STATE_READY;
            }

            return(status);
        }

        /* Setup pointer to server response.  */
        buffer_ptr =  (CHAR *) response_packet_ptr -> nx_packet_prepend_ptr;

        /* Determine which status code is reveived.  */
        for (i = 0; i < _nx_web_http_client_status_maps_size; i++)
        {
            if ((buffer_ptr[9] == _nx_web_http_client_status_maps[i].nx_web_http_client_status_string[0]) &&
                (buffer_ptr[10] == _nx_web_http_client_status_maps[i].nx_web_http_client_status_string[1]) &&
                (buffer_ptr[11] == _nx_web_http_client_status_maps[i].nx_web_http_client_status_string[2]))
            {
                status_code = _nx_web_http_client_status_maps[i].nx_web_http_client_status_code;
                break;
            }
        }

        /* Check the status code.  */
        if (i == _nx_web_http_client_status_maps_size)
        {

            /* Received an unknown status code.  */
            status = NX_WEB_HTTP_REQUEST_UNSUCCESSFUL_CODE;
        }
        else if (buffer_ptr[9] != '2')
        {

            /* Received an error status code.  */
            status = status_code;
        }
        /* Determine if the request was successful.  */
        else
        {

            /* Pickup the content length if it exists.  */
            length =  _nx_web_http_client_content_length_get(client_ptr, response_packet_ptr);

            /* Pickup the content offset and process other headers.  */
            offset =  _nx_web_http_client_process_header_fields(client_ptr, response_packet_ptr);

            /* Determine if the packet is valid.  */
            if (!offset || 
                (response_packet_ptr -> nx_packet_length < offset) || 
                ((response_packet_ptr -> nx_packet_length > (offset + length)) &&
                 (!client_ptr -> nx_web_http_client_response_chunked)))
            {

                /* Bad packet length.  */
                status = NX_WEB_HTTP_BAD_PACKET_LENGTH;
            }
            else
            {

                /* Set the flag of response header received.  */
                client_ptr -> nx_web_http_client_response_header_received = NX_TRUE;

                /* Store the total number of bytes to receive.
                   For HEAD request, just process the response header.  */
                if (client_ptr -> nx_web_http_client_state != NX_WEB_HTTP_CLIENT_STATE_HEAD)
                {
                    client_ptr -> nx_web_http_client_total_receive_bytes = length;
                }
                else
                {
                    client_ptr -> nx_web_http_client_total_receive_bytes = 0;
                }
                client_ptr -> nx_web_http_client_actual_bytes_received = 0;

                /* Adjust the pointers to skip over the response header.  */
                response_packet_ptr -> nx_packet_prepend_ptr = response_packet_ptr -> nx_packet_prepend_ptr + offset;

                /* Reduce the length.  */
                response_packet_ptr -> nx_packet_length = response_packet_ptr -> nx_packet_length - offset;

                /* Set for processing chunked response.  */
                if (client_ptr -> nx_web_http_client_response_chunked)
                {
                    client_ptr -> nx_web_http_client_response_packet = response_packet_ptr;
                    client_ptr -> nx_web_http_client_chunked_response_remaining_size = response_packet_ptr -> nx_packet_length;
                }
            }
        }
    }
    else if (!client_ptr -> nx_web_http_client_response_chunked)
    {

        /* Pickup the response from the Server.  */
        status =  _nx_web_http_client_receive(client_ptr, &response_packet_ptr, wait_option);

        /* Check for error processing received packet. */
        if (status != NX_SUCCESS)
        {

            if (status != NX_NO_PACKET)
            {

                /* Clear the flag of response header received.  */
                client_ptr->nx_web_http_client_response_header_received = NX_FALSE;

                /* Disconnect and unbind the socket.  */
                _nx_web_http_client_error_exit(client_ptr, wait_option);

                /* Reenter the READY state.  */
                client_ptr->nx_web_http_client_state = NX_WEB_HTTP_CLIENT_STATE_READY;
            }

            return(status);
        }

        if (response_packet_ptr -> nx_packet_length > (client_ptr -> nx_web_http_client_total_receive_bytes - client_ptr -> nx_web_http_client_actual_bytes_received))
        {

            /* Bad packet length.  */
            status = NX_WEB_HTTP_BAD_PACKET_LENGTH;
        }
    }

    if (status == NX_SUCCESS)
    {
        if (client_ptr -> nx_web_http_client_response_chunked)
        {

            /* Process the chunked response.  */
            status = _nx_web_http_client_response_chunked_get(client_ptr, &response_packet_ptr, wait_option);

            if (status == NX_NO_PACKET)
            {
                return(status);
            }
        }
        else
        {

            /* Adjust the actual received bytes.  */
            client_ptr -> nx_web_http_client_actual_bytes_received = client_ptr -> nx_web_http_client_actual_bytes_received +
                                                                     response_packet_ptr -> nx_packet_length;

            /* Determine if the GET packet operation is complete.  */
            if (client_ptr -> nx_web_http_client_total_receive_bytes == client_ptr -> nx_web_http_client_actual_bytes_received)
            {

                /* Yes, we are finished.  */
                status = NX_WEB_HTTP_GET_DONE;
            }
        }

        /* Move the packet pointer into the return pointer.  */
        *packet_pptr = response_packet_ptr;
    }

    if (status != NX_SUCCESS)
    {

        /* Error, break down the connection and return to the caller.  */
        if ((status != NX_WEB_HTTP_GET_DONE) && response_packet_ptr)
        {

            /* Release the invalid HTTP packet. */
            nx_packet_release(response_packet_ptr);
        }

        if (client_ptr -> nx_web_http_client_response_chunked)
        {

            /* If the response is chunked, reset the chunk info.  */
            if (status != NX_WEB_HTTP_GET_DONE)
            {

                /* If status is NX_WEB_HTTP_GET_DONE, return this packet.  */
                nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
            }
            client_ptr -> nx_web_http_client_response_packet = NX_NULL;
            client_ptr -> nx_web_http_client_chunked_response_remaining_size = 0;
            client_ptr -> nx_web_http_client_response_chunked = NX_FALSE;
        }

        /* Clear the flag of response header received.  */
        client_ptr -> nx_web_http_client_response_header_received = NX_FALSE;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
        if ((status != NX_WEB_HTTP_GET_DONE) || !(client_ptr -> nx_web_http_client_keep_alive))
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */
        {

            /* Disconnect and unbind the socket.  */
            _nx_web_http_client_error_exit(client_ptr, wait_option);
        }

        /* Reenter the READY state.  */
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_READY;
    }

    /* Return status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_response_read                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the next data in byte.                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    data                                  Returned data                 */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_receive           Receive another packet        */
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
UINT _nx_web_http_client_response_read(NX_WEB_HTTP_CLIENT *client_ptr, UCHAR *data, ULONG wait_option, 
                                       NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;

    /* If there is no remaining data.  */
    while (client_ptr -> nx_web_http_client_chunked_response_remaining_size == 0)
    {

        /* Release the previous received packet.  */
        if (client_ptr -> nx_web_http_client_response_packet)
        {
            nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
            client_ptr -> nx_web_http_client_response_packet = NX_NULL;
        }

        /* Receive another packet.  */
        status = _nx_web_http_client_receive(client_ptr, &(client_ptr -> nx_web_http_client_response_packet), wait_option);
        if (status)
        {
            return(status);
        }

        /* Update the packet pointer, data pointer and remaining size.  */
        (*current_packet_pptr) = client_ptr -> nx_web_http_client_response_packet;
        (*current_data_pptr) = client_ptr -> nx_web_http_client_response_packet -> nx_packet_prepend_ptr;
        client_ptr -> nx_web_http_client_chunked_response_remaining_size = client_ptr -> nx_web_http_client_response_packet -> nx_packet_length;
    }

    /* Process the packet chain.  */
    if ((*current_data_pptr) == (*current_packet_pptr) -> nx_packet_append_ptr)
    {
#ifndef NX_DISABLE_PACKET_CHAIN
        if ((*current_packet_pptr) -> nx_packet_next == NX_NULL)
        {
            return(NX_INVALID_PACKET);
        }

        (*current_packet_pptr) = (*current_packet_pptr) -> nx_packet_next;
        (*current_data_pptr) = (*current_packet_pptr) -> nx_packet_prepend_ptr;

        /* Release the processed packet in the packet chain.  */
        if (client_ptr -> nx_web_http_client_response_packet)
        {
            client_ptr -> nx_web_http_client_response_packet -> nx_packet_next = NX_NULL;
            nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
            client_ptr -> nx_web_http_client_response_packet = (*current_packet_pptr);
        }
#else
        return(NX_INVALID_PACKET);
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Set the returned data.  */
    *data = *(*current_data_pptr);

    /* Update the data pointer and remaining size.  */
    (*current_data_pptr)++;
    client_ptr -> nx_web_http_client_chunked_response_remaining_size--;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_response_byte_expect            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if next byte is the expected data.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    data                                  Expected data                 */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Compare result                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_response_read     Get next data                 */
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
UINT _nx_web_http_client_response_byte_expect(NX_WEB_HTTP_CLIENT *client_ptr, UCHAR data, ULONG wait_option, 
                                              NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;
UCHAR tmp;

    /* Get next data.  */
    status = _nx_web_http_client_response_read(client_ptr, &tmp, wait_option, current_packet_pptr, current_data_pptr);

    if (status)
    {
        return(status);
    }

    /* Return the compare result.  */
    if (tmp != data)
    {
        return(NX_NOT_FOUND);
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_chunked_size_get                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the chunk size of the response packet chunk.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    chunk_size                            Returned chunk size           */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_chunked_size_get  Get chunk size                */
/*    _nx_web_http_client_response_byte_expect                            */
/*                                          Check if next byte is expected*/
/*    nx_packet_release                     Release packet                */
/*    nx_packet_append                      Append packet data            */
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
UINT _nx_web_http_client_chunked_size_get(NX_WEB_HTTP_CLIENT *client_ptr, UINT *chunk_size, ULONG wait_option, 
                                          NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;
UINT size = 0;
UCHAR tmp;
UINT  chunk_extension = 0;

    if (client_ptr -> nx_web_http_client_actual_bytes_received < client_ptr -> nx_web_http_client_total_receive_bytes)
    {

        /* If there are bytes need to receive, set the size need to receive as chunk size.  */
        *chunk_size = client_ptr -> nx_web_http_client_total_receive_bytes - client_ptr -> nx_web_http_client_actual_bytes_received;
    }
    else
    {

        /* Get the chunk size.  */
        while (1)
        {

            /* Read next byte from request packet.  */
            status = _nx_web_http_client_response_read(client_ptr, &tmp, wait_option, current_packet_pptr, current_data_pptr);
            if (status)
            {
                return(status);
            }

            /* Skip the chunk extension.  */
            if (chunk_extension && (tmp != '\r'))
            {
                continue;
            }

            /* Calculate the size.  */
            if ((tmp >= 'a') && (tmp <= 'f'))
            {
                size = (size << 4) + 10 + (UINT)(tmp - 'a');
            }
            else if ((tmp >= 'A') && (tmp <= 'F'))
            {
                size = (size << 4) + 10 + (UINT)(tmp - 'A');
            }
            else if ((tmp >= '0') && (tmp <= '9'))
            {
                size = (size << 4) + (UINT)(tmp - '0');
            }
            else if (tmp == '\r')
            {

                /* Find the end of chunk header.  */
                break;
            }
            else if (tmp == ';')
            {

                /* Find chunk extension.  */
                chunk_extension = 1;
            }
            else
            {
                return(NX_NOT_FOUND);
            }
        }

        /* Expect '\n'.  */
        status = _nx_web_http_client_response_byte_expect(client_ptr, '\n', wait_option, current_packet_pptr, current_data_pptr);
        if (status)
        {
            return(status);
        }

        *chunk_size = size;
    }

    /* If there is no remaining data, receive another packet.  */
    while (client_ptr -> nx_web_http_client_chunked_response_remaining_size == 0)
    {
        if (client_ptr -> nx_web_http_client_response_packet)
        {
            nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
            client_ptr -> nx_web_http_client_response_packet = NX_NULL;
        }

        status = _nx_web_http_client_receive(client_ptr, &(client_ptr -> nx_web_http_client_response_packet), wait_option);
        if (status)
        {
            return(status);
        }

        /* Update the current request packet, data pointer and remaining size.  */
        (*current_packet_pptr) = client_ptr -> nx_web_http_client_response_packet;
        (*current_data_pptr) = client_ptr -> nx_web_http_client_response_packet -> nx_packet_prepend_ptr;
        client_ptr -> nx_web_http_client_chunked_response_remaining_size = client_ptr -> nx_web_http_client_response_packet -> nx_packet_length;
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_response_chunked_get            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the chunk data from chunked response.            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_pptr                           Pointer to the packet         */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_chunked_size_get  Get chunk size                */
/*    _nx_web_http_client_response_byte_expect                            */
/*                                          Check if next byte is expected*/
/*    nx_packet_allocate                    Allocate packet               */
/*    nx_packet_append                      Append packet data            */
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
UINT _nx_web_http_client_response_chunked_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_pptr, ULONG wait_option)
{
UINT       status;
UINT       chunk_size = 0, length = 0, temp_size = 0;
UINT       remaining_size = 0;
NX_PACKET *packet_ptr;
NX_PACKET *current_packet_ptr = NX_NULL;
UCHAR     *current_data_ptr = NX_NULL;

    /* Set pointer for processing packet data.  */
    current_packet_ptr = client_ptr -> nx_web_http_client_response_packet;

    if (current_packet_ptr)
    {

        /* Set current data pointer.  */
        current_data_ptr = current_packet_ptr -> nx_packet_prepend_ptr;
    }

    /* Get chunk size.  */
    status = _nx_web_http_client_chunked_size_get(client_ptr, &chunk_size, wait_option, &current_packet_ptr, &current_data_ptr);
    if (status)
    {
        return(status);
    }

    /* Check if it's the end.  */
    if (chunk_size == 0)
    {

        /* Read CRLF.  */
        status = _nx_web_http_client_response_byte_expect(client_ptr, '\r', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        status = _nx_web_http_client_response_byte_expect(client_ptr, '\n', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        /* Return an empty packet. */
        client_ptr -> nx_web_http_client_response_packet -> nx_packet_append_ptr = client_ptr -> nx_web_http_client_response_packet -> nx_packet_prepend_ptr;
        client_ptr -> nx_web_http_client_response_packet -> nx_packet_length = 0;
        *packet_pptr = client_ptr -> nx_web_http_client_response_packet;

        return(NX_WEB_HTTP_GET_DONE);
    }

    /* Set the return packet.  */
    *packet_pptr = client_ptr -> nx_web_http_client_response_packet;
    client_ptr -> nx_web_http_client_response_packet = NX_NULL;
    packet_ptr = *packet_pptr;
    packet_ptr -> nx_packet_prepend_ptr = current_data_ptr;
    remaining_size = client_ptr -> nx_web_http_client_chunked_response_remaining_size;

    /* Process the chunk data.  */
    if (chunk_size <= remaining_size)
    {

        /* One or more chunk in the remaining data.  */
        temp_size = chunk_size;

        /* Check if the chunk data is all in this packet.  */
        while (temp_size > (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr))
        {

#ifndef NX_DISABLE_PACKET_CHAIN
            if (current_packet_ptr -> nx_packet_next == NX_NULL)
            {
                return(NX_INVALID_PACKET);
            }

            temp_size -= (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr);
            current_packet_ptr = current_packet_ptr -> nx_packet_next;
            current_data_ptr = current_packet_ptr -> nx_packet_prepend_ptr;
#else
            return(NX_INVALID_PACKET);
#endif /* NX_DISABLE_PACKET_CHAIN */
        }

        /* Skip the chunk data.  */
        current_data_ptr += temp_size;
        client_ptr -> nx_web_http_client_chunked_response_remaining_size -= chunk_size;
        length = chunk_size;

        /* Read CRLF.  */
        status = _nx_web_http_client_response_byte_expect(client_ptr, '\r', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        status = _nx_web_http_client_response_byte_expect(client_ptr, '\n', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        /* Check the remaining data.  */
        if (client_ptr -> nx_web_http_client_chunked_response_remaining_size)
        {
            if (client_ptr -> nx_web_http_client_response_packet)
            {

                /* If received new packet, adjust the prepend pointer of this packet.  */
                client_ptr -> nx_web_http_client_response_packet -> nx_packet_prepend_ptr = current_data_ptr;
            }
            else
            {

                /* Copy the remaining data to a new packet.  */
                /* Allocate a packet.  */
                status = nx_packet_allocate(client_ptr -> nx_web_http_client_packet_pool_ptr, 
                                            &(client_ptr -> nx_web_http_client_response_packet), 
                                            0, wait_option);
                if (status)
                {
                    return(status);
                }

                /* Copy the remaining data in current packet to the new packet.   */
                temp_size = (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr);
                status = nx_packet_data_append(client_ptr -> nx_web_http_client_response_packet,
                                               current_data_ptr,
                                               temp_size,
                                               client_ptr -> nx_web_http_client_packet_pool_ptr,
                                               wait_option);
                if (status)
                {
                    return(status);
                }

                /* Check if any remaining data not in current packet.  */
                if (client_ptr -> nx_web_http_client_chunked_response_remaining_size > temp_size)
                {
#ifndef NX_DISABLE_PACKET_CHAIN

                    /* If there are chained packets, append the packets to the new packet.  */
                    if (current_packet_ptr -> nx_packet_next)
                    {
                        client_ptr -> nx_web_http_client_response_packet -> nx_packet_next = current_packet_ptr -> nx_packet_next;
                        client_ptr -> nx_web_http_client_response_packet -> nx_packet_last = current_packet_ptr -> nx_packet_last;
                        current_packet_ptr -> nx_packet_next = NX_NULL;
                    }
                    else
#endif /* NX_DISABLE_PACKET_CHAIN */
                    {
                        return(NX_INVALID_PACKET);
                    }
                }
            }

            /* Update the packet length.  */
            client_ptr -> nx_web_http_client_response_packet -> nx_packet_length = client_ptr -> nx_web_http_client_chunked_response_remaining_size;
        }
    }
    else
    {

        /* All the remaining data is in this chunk.  */
        client_ptr -> nx_web_http_client_chunked_response_remaining_size = 0;
        length = remaining_size;
    }

    /* Set the received bytes.  */
    client_ptr -> nx_web_http_client_total_receive_bytes = chunk_size;
    client_ptr -> nx_web_http_client_actual_bytes_received = length;

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Set length of the packet chain header.  */
    (*packet_pptr) -> nx_packet_length = length;

    /* Find the last packet.  */
    while (packet_ptr -> nx_packet_next && 
           (length > (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr)))
    {
        length -= (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
        packet_ptr = packet_ptr -> nx_packet_next;
    }

    /* Set the last packet.  */
    (*packet_pptr) -> nx_packet_last = packet_ptr;

    /* Maybe the '\r\n' is in another packet, release this packet.  */
    if (packet_ptr -> nx_packet_next)
    {
        nx_packet_release(packet_ptr -> nx_packet_next);
        packet_ptr -> nx_packet_next = NX_NULL;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set packet length and append pointer.  */
    /* If the packet is chained, set the length and append pointer of the last packet.  */
    packet_ptr -> nx_packet_length = length;
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + length;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_chunked_set            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP request chunked set     */
/*    API call.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    chunk_size                            Size of the chunk             */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_chunked_set                             */
/*                                          Actual request chunked set    */
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
UINT _nxe_web_http_client_request_chunked_set(NX_WEB_HTTP_CLIENT *client_ptr, UINT chunk_size, NX_PACKET *packet_ptr)
{
UINT        status;

    /* Make sure the client instance makes sense. */
    if ((client_ptr == NX_NULL) || (packet_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual request chunked set function.  */
    status =  _nx_web_http_client_request_chunked_set(client_ptr, chunk_size, packet_ptr);

    /* Return success to the caller.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_chunked_set             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the chunked information of chunked request.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    chunk_size                            Size of the chunk             */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append the packet data        */
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
UINT _nx_web_http_client_request_chunked_set(NX_WEB_HTTP_CLIENT *client_ptr, UINT chunk_size, NX_PACKET *packet_ptr)
{
UINT        status;
UINT        temp_size, i, j;
CHAR        temp_string[10];
CHAR        crlf[2] = {13,10};

    /* Covert the size to ASCII.  */
    temp_size = chunk_size;
    i = 0;

    while (temp_size)
    {

        for (j = i; j > 0; j--)
        {
            temp_string[j] = temp_string[j - 1];
        }

        if ((temp_size & 0x0F) < 10)
        {
            temp_string[0] = (CHAR)((temp_size & 0x0F) + '0');
        }
        else
        {
            temp_string[0] = (CHAR)((temp_size & 0x0F) - 10 + 'a');
        }
        temp_size = temp_size >> 4;
        i++;
    }

    if (i == 0)
    {
        temp_string[i++] = '0';
    }

    /* Place the chunk size in the packet.  */
    status = nx_packet_data_append(packet_ptr, temp_string, i, client_ptr -> nx_web_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(status);
    }

    /* Place the CRLF to signal the end of the chunk size.  */
    status = nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(status);
    }

    /* Set the request chunked flag.  */
    client_ptr -> nx_web_http_client_request_chunked = NX_TRUE;

    /* The total bytes need to transfer is chunk size plus chunk header length.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes = chunk_size + i + 2;
    client_ptr -> nx_web_http_client_actual_bytes_transferred = 0;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_packet_send            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP request packet send     */
/*    API call.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to the packet         */
/*    more_data                             If there are more data to send*/
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_packet_send                             */
/*                                          Actual request packet send    */
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
UINT _nxe_web_http_client_request_packet_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT more_data, ULONG wait_option)
{
UINT        status;

    /* Make sure the client instance makes sense. */
    if ((client_ptr == NX_NULL) || (packet_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Now send the packet to the HTTP server.  */
    status =  _nx_web_http_client_request_packet_send(client_ptr, packet_ptr, more_data, wait_option);

    /* Return success to the caller.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_packet_send             PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends an HTTP request packet to a remote server.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to the packet         */
/*    more_data                             If there are more data to send*/
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_send              Send data to server           */
/*    _nx_web_http_client_error_exit        Cleanup and shut down HTTPS   */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the invalid release issue,  */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_client_request_packet_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT more_data, ULONG wait_option)
{
UINT        status;
CHAR        crlf[2] = {13,10};
UINT        length = packet_ptr -> nx_packet_length;

    /* Check the packet length.  */
    if (client_ptr -> nx_web_http_client_total_transfer_bytes < (client_ptr -> nx_web_http_client_actual_bytes_transferred + length))
    {
        return(NX_INVALID_PACKET);
    }

    /* If the request is chunked, add CRLF at the end of the chunk.  */
    if (client_ptr -> nx_web_http_client_request_chunked)
    {

        if (client_ptr -> nx_web_http_client_total_transfer_bytes == (client_ptr -> nx_web_http_client_actual_bytes_transferred + length))
        {

            /* Place an extra CRLF to signal the end of the chunk.  */
            nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);

            /* If there are no more data to send, append the last chunk.  */
            if (!more_data)
            {
                nx_packet_data_append(packet_ptr, "0\r\n\r\n", 5, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
                client_ptr -> nx_web_http_client_request_chunked = NX_FALSE;
            }
        }
    }

    /* Now send the packet to the HTTP server.  */
    status =  _nx_web_http_client_send(client_ptr, packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }

    /* Update the transferred bytes.  */
    client_ptr -> nx_web_http_client_actual_bytes_transferred += length;

    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_put_start                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP Duo Client put start    */ 
/*        call.                                                           */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */ 
/*    password                              Pointer to password           */ 
/*    total_bytes                           Total bytes to send           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_start         Actual client put start call  */
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
UINT  _nxe_web_http_client_put_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                     CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_web_http_client_put_start(client_ptr, server_ip, server_port, resource, host,
                                            username, password, total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_start                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes an application PUT request. Transferring the*/
/*    specified resource (URL) is started by this routine. The            */ 
/*    application must call put packet one or more times to transfer      */ 
/*    the resource contents.                                              */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP address of HTTP Server     */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */ 
/*    password                              Pointer to password           */ 
/*    total_bytes                           Total bytes to send           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_web_http_client_put_start_extended                              */
/*                                          Actual client put start call  */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_put_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                    CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_put_start_extended(client_ptr, server_ip, server_port, resource,
                                                  temp_resource_length, host, temp_host_length, username,
                                                  temp_username_length, password, temp_password_length,
                                                  total_bytes, wait_option));
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_put_start_extended             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP Duo Client put start    */ 
/*        call.                                                           */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total bytes to send           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_start_extended                              */
/*                                          Actual client put start call  */
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
UINT  _nxe_web_http_client_put_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                              UINT server_port, CHAR *resource, UINT resource_length,
                                              CHAR *host, UINT host_length, CHAR *username,
                                              UINT username_length, CHAR *password,
                                              UINT password_length, ULONG total_bytes,
                                              ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_web_http_client_put_start_extended(client_ptr, server_ip, server_port, resource,
                                                     resource_length, host, host_length, username,
                                                     username_length, password, password_length,
                                                     total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_start_extended              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes an application PUT request. Transferring the*/
/*    specified resource (URL) is started by this routine. The            */ 
/*    application must call put packet one or more times to transfer      */ 
/*    the resource contents.                                              */ 
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total bytes to send           */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_web_http_client_connect           Connect to remote server      */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize PUT request        */
/*    _nx_web_http_client_content_type_header_add                         */
/*                                          Add content type header       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_error_exit        Shutdown HTTP(S) connection   */
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
UINT  _nx_web_http_client_put_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                             UINT server_port, CHAR *resource, UINT resource_length,
                                             CHAR *host, UINT host_length, CHAR *username,
                                             UINT username_length, CHAR *password,
                                             UINT password_length, ULONG total_bytes,
                                             ULONG wait_option)
{
UINT        status;

    /* Set the port we are connecting to. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_PUT,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             total_bytes,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Add approproate headers from input data. */
    _nx_web_http_client_content_type_header_add(client_ptr, resource, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }


    /* Store the total number of bytes to send.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes =     total_bytes;
    client_ptr -> nx_web_http_client_actual_bytes_transferred =  0;

    /* Enter the PUT state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_PUT;


    return(status);
}


#ifdef NX_WEB_HTTPS_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_put_secure_start               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure PUT request     */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to PUT  */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_client_put_secure_start Actual PUT request processing */
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
UINT  _nxe_web_http_client_put_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                            CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                            ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_web_http_client_put_secure_start(client_ptr, server_ip, server_port, resource,
                                                   host, username, password,
                                                   total_bytes, tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_put_secure_start                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application PUT request. Transferring the*/
/*    specified resource (URL) is started by this routine. The            */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.  This version of the function also requires  */
/*    a TLS setup callback as the data is sent over TLS-secured HTTPS.    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to PUT  */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_put_secure_start_extended                       */
/*                                          Actual PUT request processing */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_put_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                           CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                           UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                           ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_put_secure_start_extended(client_ptr, server_ip, server_port,
                                                         resource, temp_resource_length, host,
                                                         temp_host_length, username,
                                                         temp_username_length, password,
                                                         temp_password_length, total_bytes,
                                                         tls_setup, wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_put_secure_start_extended      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure PUT request     */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total number of bytes to PUT  */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_put_secure_start_extended                       */
/*                                          Actual PUT request processing */
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
UINT _nxe_web_http_client_put_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                    CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                    CHAR *username, UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG total_bytes,
                                                    UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                    ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status = _nx_web_http_client_put_secure_start_extended(client_ptr, server_ip, server_port,
                                                           resource, resource_length, host,
                                                           host_length, username, username_length,
                                                           password, password_length, total_bytes,
                                                           tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_put_secure_start_extended       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application PUT request. Transferring the*/
/*    specified resource (URL) is started by this routine. The            */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.  This version of the function also requires  */
/*    a TLS setup callback as the data is sent over TLS-secured HTTPS.    */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total number of bytes to PUT  */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_secure_connect    Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*   _nx_web_http_client_content_type_header_add                          */
/*                                          Add content type header       */
/*                                                                        */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_error_exit        Shutdown HTTPS session        */
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
UINT _nx_web_http_client_put_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                   CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                   CHAR *username, UINT username_length, CHAR *password,
                                                   UINT password_length, ULONG total_bytes,
                                                   UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                   ULONG wait_option)
{
UINT        status;


    /* Use default HTTPS port. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, tls_setup, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_PUT,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             total_bytes,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Add approproate headers from input data. */
    _nx_web_http_client_content_type_header_add(client_ptr, resource, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }


    /* Store the total number of bytes to send.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes =     total_bytes;
    client_ptr -> nx_web_http_client_actual_bytes_transferred =  0;

    /* Enter the PUT state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_PUT;

    return(status);
}

#endif /* NX_WEB_HTTPS_ENABLE */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_post_start                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP Client POST start       */
/*        call.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to POST */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_post_start        Actual client post start call */
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
UINT  _nxe_web_http_client_post_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                      CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_web_http_client_post_start(client_ptr, server_ip, server_port, resource, host,
                                             username, password, total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_post_start                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application POST request. Transferring   */
/*    the specified resource (URL) is started by this routine. The        */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.                                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to POST */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_post_start_extended                             */
/*                                          Actual client put start call  */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_post_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                     CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_post_start_extended(client_ptr, server_ip, server_port, resource,
                                                   temp_resource_length, host, temp_host_length, username,
                                                   temp_username_length, password, temp_password_length,
                                                   total_bytes, wait_option));
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_client_post_start_extended            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP Client POST start       */
/*    call.                                                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */
/*    password_length                       Length of password            */
/*    total_bytes                           Total number of bytes to POST */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_post_start_extended                             */
/*                                          Actual client post start call */
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
UINT  _nxe_web_http_client_post_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                               UINT server_port, CHAR *resource, UINT resource_length,
                                               CHAR *host, UINT host_length, CHAR *username,
                                               UINT username_length, CHAR *password,
                                               UINT password_length, ULONG total_bytes,
                                               ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual POST start routine.  */
    status =  _nx_web_http_client_post_start_extended(client_ptr, server_ip, server_port, resource,
                                                      resource_length, host, host_length, username,
                                                      username_length, password, password_length,
                                                      total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_post_start_extended             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application POST request. Transferring   */
/*    the specified resource (URL) is started by this routine. The        */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.                                              */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */
/*    password_length                       Length of password            */
/*    total_bytes                           Total bytes to send           */ 
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_connect           Connect to remote server      */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize PUT request        */
/*    _nx_web_http_client_content_type_header_add                         */
/*                                          Add content type header       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_error_exit        Shutdown HTTP(S) connection   */
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
UINT  _nx_web_http_client_post_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                              UINT server_port, CHAR *resource, UINT resource_length,
                                              CHAR *host, UINT host_length, CHAR *username,
                                              UINT username_length, CHAR *password,
                                              UINT password_length, ULONG total_bytes,
                                              ULONG wait_option)
{
UINT        status;


    /* Set the port we are connecting to. */
    client_ptr -> nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, client_ptr -> nx_web_http_client_connect_port, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_POST,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             total_bytes,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Add approproate headers from input data. */
    _nx_web_http_client_content_type_header_add(client_ptr, resource, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }


    /* Store the total number of bytes to send.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes = total_bytes;
    client_ptr -> nx_web_http_client_actual_bytes_transferred = 0;

    /* Enter the POST state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_POST;


    return(status);
}


#ifdef NX_WEB_HTTPS_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_post_secure_start              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure POST request    */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to POST */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_client_post_secure_start                              */
/*                                          Actual POST processing        */
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
UINT  _nxe_web_http_client_post_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                             CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                             UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                             ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual POST start routine.  */
    status =  _nx_web_http_client_post_secure_start(client_ptr, server_ip, server_port, resource, host,
                                                    username, password, total_bytes, tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_post_secure_start               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application POST request. Transferring   */
/*    the specified resource (URL) is started by this routine. The        */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.  This version of the function also requires  */
/*    a TLS setup callback as the data is sent over TLS-secured HTTPS.    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    server_port                           HTTP Server TCP port          */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    total_bytes                           Total number of bytes to POST */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_post_secure_start_extended                      */
/*                                          Actual POST request processing*/
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
UINT  _nx_web_http_client_post_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                            CHAR *host, CHAR *username, CHAR *password, ULONG total_bytes,
                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                            ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_post_secure_start_extended(client_ptr, server_ip, server_port,
                                                          resource, temp_resource_length, host,
                                                          temp_host_length, username,
                                                          temp_username_length, password,
                                                          temp_password_length, total_bytes,
                                                          tls_setup, wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_post_secure_start_extended     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure POST request    */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total number of bytes to POST */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_post_secure_start_extended                      */
/*                                          Actual POST request processing*/
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
UINT _nxe_web_http_client_post_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                     CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                     CHAR *username, UINT username_length, CHAR *password,
                                                     UINT password_length, ULONG total_bytes,
                                                     UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                     ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual POST start routine.  */
    status = _nx_web_http_client_post_secure_start_extended(client_ptr, server_ip, server_port,
                                                            resource, resource_length, host,
                                                            host_length, username, username_length,
                                                            password, password_length, total_bytes,
                                                            tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_post_secure_start_extended      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application POST request. Transferring   */
/*    the specified resource (URL) is started by this routine. The        */
/*    application must call put packet one or more times to transfer      */
/*    the resource contents.  This version of the function also requires  */
/*    a TLS setup callback as the data is sent over TLS-secured HTTPS.    */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    total_bytes                           Total number of bytes to POST */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_secure_connect    Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_content_type_header_add                         */
/*                                          Add content type header       */
/*    _nx_web_http_client_error_exit        Close HTTPS session           */
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
UINT _nx_web_http_client_post_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                    CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                    CHAR *username, UINT username_length, CHAR *password,
                                                    UINT password_length, ULONG total_bytes,
                                                    UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                    ULONG wait_option)
{
UINT        status;


    /* Use default HTTPS port. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, tls_setup, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_POST,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             total_bytes,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Add approproate headers from input data. */
    _nx_web_http_client_content_type_header_add(client_ptr, resource, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }


    /* Store the total number of bytes to send.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes = total_bytes;
    client_ptr -> nx_web_http_client_actual_bytes_transferred = 0;

    /* Enter the POST state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_POST;

    return(status);
}

#endif /* NX_WEB_HTTPS_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_head_start                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to start a HEAD request.*/
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_start        Actual HEAD request call      */
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
UINT  _nxe_web_http_client_head_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                      CHAR *resource, CHAR *host, CHAR *username, CHAR *password,
                                      ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL)   || !server_ip || (host == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual HEAD start routine.  */
    status =  _nx_web_http_client_head_start(client_ptr, server_ip, server_port, resource,
                                             host, username, password, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_head_start                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application HEAD request. HEAD is        */
/*    identical to GET for the HTTP client, so the server response is     */
/*    processed by calling nx_web_http_client_response_body_get.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_start_extended                             */
/*                                          Actual HEAD request call      */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_head_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                     CHAR *host, CHAR *username, CHAR *password, ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_head_start_extended(client_ptr, server_ip, server_port, resource,
                                                   temp_resource_length, host, temp_host_length, username,
                                                   temp_username_length, password, temp_password_length,
                                                   wait_option));

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_head_start_extended            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to start a HEAD request.*/
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_start_extended                             */
/*                                          Actual HEAD request call      */
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
UINT  _nxe_web_http_client_head_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                               UINT server_port, CHAR *resource, UINT resource_length,
                                               CHAR *host, UINT host_length, CHAR *username,
                                               UINT username_length, CHAR *password,
                                               UINT password_length, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL)   || !server_ip || (host == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual HEAD start routine.  */
    status =  _nx_web_http_client_head_start_extended(client_ptr, server_ip, server_port, resource,
                                                      resource_length, host, host_length, username,
                                                      username_length, password, password_length,
                                                      wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_head_start_extended             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application HEAD request. HEAD is        */
/*    identical to GET for the HTTP client, so the server response is     */
/*    processed by calling nx_web_http_client_response_body_get.          */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_connect           Connect to remote server      */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HEAD request       */
/*    _nx_web_http_client_content_type_header_add                         */
/*                                          Add content type header       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_error_exit        Shutdown HTTP(S) connection   */
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
UINT  _nx_web_http_client_head_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                              UINT server_port, CHAR *resource, UINT resource_length,
                                              CHAR *host, UINT host_length, CHAR *username,
                                              UINT username_length, CHAR *password,
                                              UINT password_length, ULONG wait_option)
{
UINT        status;

    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_HEAD,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the HEAD state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_HEAD;

    return(status);
}

#ifdef NX_WEB_HTTPS_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_head_secure_start              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure HEAD request    */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_secure_start                               */
/*                                          Actual HEAD request processing*/
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
UINT  _nxe_web_http_client_head_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                             CHAR *host, CHAR *username, CHAR *password,
                                             UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                             ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual HEAD start routine.  */
    status =  _nx_web_http_client_head_secure_start(client_ptr, server_ip, server_port, resource,
                                                    host, username, password, tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_head_secure_start               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application HEAD request. The HTTP       */
/*    header for the specified resource (URL) is requested from the HTTP  */
/*    Server at the specified IP address and port. This version of the    */
/*    function also requires a TLS setup  callback as the request is sent */
/*    over TLS-secured HTTPS.                                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_secure_start_extended                      */
/*                                          Actual HEAD request processing*/
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_head_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                            CHAR *host, CHAR *username, CHAR *password,
                                            UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                            ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_head_secure_start_extended(client_ptr, server_ip, server_port,
                                                          resource, temp_resource_length, host,
                                                          temp_host_length, username,
                                                          temp_username_length, password,
                                                          temp_password_length, tls_setup,
                                                          wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_head_secure_start_extended     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure HEAD request    */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_head_secure_start_extended                      */
/*                                          Actual HEAD request processing*/
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
UINT _nxe_web_http_client_head_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                     CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                     CHAR *username, UINT username_length, CHAR *password,
                                                     UINT password_length,
                                                     UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                     ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual HEAD start routine.  */
    status = _nx_web_http_client_head_secure_start_extended(client_ptr, server_ip, server_port,
                                                            resource, resource_length, host,
                                                            host_length, username, username_length,
                                                            password, password_length,
                                                            tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_head_secure_start_extended      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application HEAD request. The HTTP       */
/*    header for the specified resource (URL) is requested from the HTTP  */
/*    Server at the specified IP address and port. This version of the    */
/*    function also requires a TLS setup  callback as the request is sent */
/*    over TLS-secured HTTPS.                                             */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_secure_connect    Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
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
UINT _nx_web_http_client_head_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                    CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                    CHAR *username, UINT username_length, CHAR *password,
                                                    UINT password_length,
                                                    UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                    ULONG wait_option)
{

UINT        status;

    /* Use default HTTPS port. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, tls_setup, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_HEAD,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the HEAD state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_HEAD;

    return(status);
}

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_delete_start                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to start a DELETE       */
/*    request.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_start      Actual DELETE request call    */
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
UINT  _nxe_web_http_client_delete_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                        CHAR *resource, CHAR *host, CHAR *username, CHAR *password,
                                        ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL)   || !server_ip || (host == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DELETE start routine.  */
    status =  _nx_web_http_client_delete_start(client_ptr, server_ip, server_port, resource,
                                               host, username, password, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_delete_start                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application DELETE request. DELETE only  */
/*    receives a simple response which can be processed with              */
/*    nx_web_http_client_response_body_get.                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             IP address of HTTP Server     */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_start_extended                           */
/*                                          Actual DELETE request call    */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_delete_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                       CHAR *host, CHAR *username, CHAR *password, ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_delete_start_extended(client_ptr, server_ip, server_port, resource,
                                                     temp_resource_length, host, temp_host_length, username,
                                                     temp_username_length, password, temp_password_length,
                                                     wait_option));

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_delete_start_extended          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to start a DELETE       */
/*    request.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_start      Actual DELETE request call    */
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
UINT  _nxe_web_http_client_delete_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                 UINT server_port, CHAR *resource, UINT resource_length,
                                                 CHAR *host, UINT host_length, CHAR *username,
                                                 UINT username_length, CHAR *password,
                                                 UINT password_length, ULONG wait_option)
{

UINT    status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL)   || !server_ip || (host == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DELETE start routine.  */
    status =  _nx_web_http_client_delete_start_extended(client_ptr, server_ip, server_port, resource,
                                                        resource_length, host, host_length, username,
                                                        username_length, password, password_length,
                                                        wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_delete_start_extended           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application DELETE request. DELETE only  */
/*    receives a simple response which can be processed with              */
/*    nx_web_http_client_response_body_get.                               */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_connect           Connect to remote server      */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize DELETE request     */
/*    _nx_web_http_client_content_type_header_add                         */
/*                                          Add content type header       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
/*    _nx_web_http_client_error_exit        Shutdown HTTP(S) connection   */
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
UINT  _nx_web_http_client_delete_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip,
                                                UINT server_port, CHAR *resource, UINT resource_length,
                                                CHAR *host, UINT host_length, CHAR *username,
                                                UINT username_length, CHAR *password,
                                                UINT password_length, ULONG wait_option)
{

UINT        status;

    client_ptr -> nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, client_ptr -> nx_web_http_client_connect_port, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_DELETE,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the DELETE state.  */
    client_ptr -> nx_web_http_client_state = NX_WEB_HTTP_CLIENT_STATE_DELETE;

    return(status);
}

#ifdef NX_WEB_HTTPS_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_delete_secure_start            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure DELETE request  */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_secure_start                             */
/*                                          Actual DELETE request call    */
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
UINT  _nxe_web_http_client_delete_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                               CHAR *host, CHAR *username, CHAR *password,
                                               UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                               ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DELETE start routine.  */
    status =  _nx_web_http_client_delete_secure_start(client_ptr, server_ip, server_port, resource,
                                                      host, username, password, tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_delete_secure_start             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application DELETE request. DELETE only  */
/*    receives a simple response which can be processed with              */
/*    nx_web_http_client_response_body_get. This version of the  function */
/*    also requires a TLS setup  callback as the request is sent over     */
/*    TLS-secured HTTPS.                                                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             HTTP Server IP address        */
/*    resource                              Pointer to resource (URL)     */
/*    host                                  Pointer to Host               */
/*    username                              Pointer to username           */
/*    password                              Pointer to password           */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_secure_start_extended                    */
/*                                          Actual DELETE processing      */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_client_delete_secure_start(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, CHAR *resource,
                                              CHAR *host, CHAR *username, CHAR *password,
                                              UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                              ULONG wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_delete_secure_start_extended(client_ptr, server_ip, server_port,
                                                            resource, temp_resource_length, host,
                                                            temp_host_length, username,
                                                            temp_username_length, password,
                                                            temp_password_length, tls_setup,
                                                            wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_delete_secure_start_extended   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS secure DELETE request  */
/*    processing call.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_delete_secure_start_extended                    */
/*                                          Actual DELETE processing      */
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
UINT _nxe_web_http_client_delete_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                       CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                       CHAR *username, UINT username_length, CHAR *password,
                                                       UINT password_length,
                                                       UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                       ULONG wait_option)
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) ||
        (resource == NX_NULL) || !server_ip || !tls_setup || (host == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DELETE start routine.  */
    status = _nx_web_http_client_delete_secure_start_extended(client_ptr, server_ip, server_port,
                                                              resource, resource_length, host,
                                                              host_length, username, username_length,
                                                              password, password_length,
                                                              tls_setup, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_delete_secure_start_extended    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an application DELETE request. DELETE only  */
/*    receives a simple response which can be processed with              */
/*    nx_web_http_client_response_body_get. This version of the  function */
/*    also requires a TLS setup  callback as the request is sent over     */
/*    TLS-secured HTTPS.                                                  */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       String length of resource     */ 
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    username                              Pointer to username           */ 
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */ 
/*    password_length                       Length of password            */
/*    tls_setup                             TLS setup callback function   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_secure_connect    Connect to HTTPS server       */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Initialize HTTP request       */
/*    _nx_web_http_client_request_send      Send HTTP request to server   */
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
UINT _nx_web_http_client_delete_secure_start_extended(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                                      CHAR *resource, UINT resource_length, CHAR *host, UINT host_length,
                                                      CHAR *username, UINT username_length, CHAR *password,
                                                      UINT password_length,
                                                      UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                                      ULONG wait_option)
{

UINT        status;

    /* Use default HTTPS port. */
    client_ptr->nx_web_http_client_connect_port = server_port;

    /* Connect to the server. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, client_ptr->nx_web_http_client_connect_port, tls_setup, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Initialize HTTP request. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr,
                                                             NX_WEB_HTTP_METHOD_DELETE,
                                                             resource,
                                                             resource_length,
                                                             host,
                                                             host_length,
                                                             0,
                                                             NX_FALSE,   /* If true, input_size is ignored. */
                                                             username,
                                                             username_length,
                                                             password,
                                                             password_length,
                                                             wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Send the HTTP request we just built. */
    status = _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Enter the DELETE state.  */
    client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_DELETE;

    return(status);
}

#endif




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_put_packet                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client put packet call. */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    packet_ptr                            Resource data packet pointer  */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_packet        Actual client put packet call */
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
UINT  _nxe_web_http_client_put_packet(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT         status;
NXD_ADDRESS *server_ip;
UINT         header_size;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_web_http_client_id != NX_WEB_HTTP_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Get worker variables for use. */
    server_ip = &(client_ptr -> nx_web_http_client_server_address);

#ifndef NX_DISABLE_IPV4
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        header_size = NX_IPv4_TCP_PACKET;
    }
    else
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        header_size = NX_IPv6_TCP_PACKET;
    }
    else
#endif /* FEATURE_NX_IPV6 */
    {
        return(NX_NOT_CONNECTED);
    }

    /* Ensure there is enough room for the TCP packet header.  */
    if ((UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < header_size)
    {

        /* Return an invalid packet error.  */
        return(NX_INVALID_PACKET);
    }

    /* Call actual PUT data routine.  */
    status =  _nx_web_http_client_put_packet(client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_packet                      PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes a packet of resource data associated with   */ 
/*    the previous PUT request.                                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    packet_ptr                            Resource data packet pointer  */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_web_http_client_receive           Check for server response     */
/*    nx_packet_release                     Release the packet            */
/*    _nx_web_http_client_error_exit        Shutdown connection           */
/*    _nx_web_http_client_send              Send data to server           */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the invalid release issue,  */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_client_put_packet(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

NX_PACKET   *response_packet_ptr;
CHAR        *buffer_ptr;
UINT        length;
UINT        status;


    /* First, check and see if the client instance is still in the PUT state.  */
    if (client_ptr -> nx_web_http_client_state != NX_WEB_HTTP_CLIENT_STATE_PUT &&
        client_ptr -> nx_web_http_client_state != NX_WEB_HTTP_CLIENT_STATE_POST)
    {

        /* Client not ready, return error.  */
        return(NX_WEB_HTTP_NOT_READY);
    }

    /* Next, check and see if there is a response from the Server.  */
    status =  _nx_web_http_client_receive(client_ptr, &response_packet_ptr, NX_NO_WAIT);

    /* Check for an early response from the Server.  */
    if (status == NX_SUCCESS)
    {

        /* This is an error condition since the Server should not respond until the PUT is complete.  */

        /* Setup pointer to server response.  */
        buffer_ptr =  (CHAR *) response_packet_ptr -> nx_packet_prepend_ptr;

        /* Determine if it is an authentication error.  */
        if (((buffer_ptr + 11) < (CHAR *)response_packet_ptr -> nx_packet_append_ptr) &&
            (buffer_ptr[9] == '4') && (buffer_ptr[10] == '0') && (buffer_ptr[11] == '1'))
        {

            /* Inform caller of an authentication error.  */
            status =  NX_WEB_HTTP_AUTHENTICATION_ERROR;
        }
        else
        {

            /* Inform caller of general Server failure.  */
            status =  NX_WEB_HTTP_INCOMPLETE_PUT_ERROR;
        }

        /* Release the packet.  */
        nx_packet_release(response_packet_ptr);

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return to the READY state.  */
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_READY;

        /* Return error to caller.  */
        return(status);
    }

    /* Otherwise, determine if the packet length fits in the available bytes to send.  */
    if (packet_ptr -> nx_packet_length > 
            (client_ptr -> nx_web_http_client_total_transfer_bytes - client_ptr -> nx_web_http_client_actual_bytes_transferred))
    {

        /* Request doesn't fit into the remaining transfer window.  */
        return(NX_WEB_HTTP_BAD_PACKET_LENGTH);
    }

    /* Remember the packet length.  */
    length =  packet_ptr -> nx_packet_length;

    /* Now send the packet out.  */
    status =  _nx_web_http_client_send(client_ptr, packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return to the READY state.  */
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_READY;

        /* Return an error.  */
        return(status);
    }

    /* Otherwise, update the actual bytes transferred.  */
    client_ptr ->  nx_web_http_client_actual_bytes_transferred =  client_ptr ->  nx_web_http_client_actual_bytes_transferred + length;

    /* Return status to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_type_get                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function derives the type of the resource.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    name                                  Name string                   */ 
/*    http_type_string                      Destination HTTP type string  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Size                                  Number of bytes in string     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_client_put_start         Start the PUT process         */
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
UINT  _nx_web_http_client_type_get(CHAR *name, CHAR *http_type_string)
{

UINT    i;

    
    /* First find the end of the string.  */
    i =  0;
    while (name[i] != (CHAR) NX_NULL)
    {
        i++;
    }

    /* Now look backwards to find the last period that signals the 
       file extension.  */
    while ((i) && (name[i] != '.'))
    {
        i--;
    }

    /* Position forward again, past the period.  */
    i++;

    /* Now see what HTTP file type to return.  */

    /* Check for .txt file extension.  */
    if (((name[i] ==   't') || (name[i] ==   'T')) &&
        ((name[i+1] == 'x') || (name[i+1] == 'X')) &&
        ((name[i+2] == 't') || (name[i+2] == 'T')))
    {

        /* Yes, we have a plain text file.  */
        http_type_string[0] =  't';
        http_type_string[1] =  'e';
        http_type_string[2] =  'x';
        http_type_string[3] =  't';
        http_type_string[4] =  '/';
        http_type_string[5] =  'p';
        http_type_string[6] =  'l';
        http_type_string[7] =  'a';
        http_type_string[8] =  'i';
        http_type_string[9] =  'n';

        /* Return the size of the HTTP ASCII type string.  */
        return(10);
    }

    /* Check for .htm[l] file extension.  */
    else if (((name[i] ==   'h') || (name[i] ==   'H')) &&
             ((name[i+1] == 't') || (name[i+1] == 'T')) &&
             ((name[i+2] == 'm') || (name[i+2] == 'M')))
    {

        /* Yes, we have an HTML text file.  */
        http_type_string[0] =  't';
        http_type_string[1] =  'e';
        http_type_string[2] =  'x';
        http_type_string[3] =  't';
        http_type_string[4] =  '/';
        http_type_string[5] =  'h';
        http_type_string[6] =  't';
        http_type_string[7] =  'm';
        http_type_string[8] =  'l';

        /* Return the size of the HTTP ASCII type string.  */
        return(9);
    }

    /* Check for .gif file extension.  */
    else if (((name[i] ==   'g') || (name[i] ==   'G')) &&
             ((name[i+1] == 'i') || (name[i+1] == 'I')) &&
             ((name[i+2] == 'f') || (name[i+2] == 'F')))
    {

        /* Yes, we have a GIF image file.  */
        http_type_string[0] =  'i';
        http_type_string[1] =  'm';
        http_type_string[2] =  'a';
        http_type_string[3] =  'g';
        http_type_string[4] =  'e';
        http_type_string[5] =  '/';
        http_type_string[6] =  'g';
        http_type_string[7] =  'i';
        http_type_string[8] =  'f';

        /* Return the size of the HTTP ASCII type string.  */
        return(9);
    }

    /* Check for .xbm file extension.  */
    else if (((name[i] ==   'x') || (name[i] ==   'X')) &&
             ((name[i+1] == 'b') || (name[i+1] == 'B')) &&
             ((name[i+2] == 'm') || (name[i+2] == 'M')))
    {

        /* Yes, we have a x-xbitmap image file.  */
        http_type_string[0] =  'i';
        http_type_string[1] =  'm';
        http_type_string[2] =  'a';
        http_type_string[3] =  'g';
        http_type_string[4] =  'e';
        http_type_string[5] =  '/';
        http_type_string[6] =  'x';
        http_type_string[7] =  '-';
        http_type_string[8] =  'x';
        http_type_string[9] =  'b';
        http_type_string[10] = 'i';
        http_type_string[11] = 't';
        http_type_string[12] = 'm';
        http_type_string[13] = 'a';
        http_type_string[14] = 'p';

        /* Return the size of the HTTP ASCII type string.  */
        return(15);
    }

    /* Default to plain text.  */
    else 
    {

        /* Default to plain text.  */
        http_type_string[0] =  't';
        http_type_string[1] =  'e';
        http_type_string[2] =  'x';
        http_type_string[3] =  't';
        http_type_string[4] =  '/';
        http_type_string[5] =  'p';
        http_type_string[6] =  'l';
        http_type_string[7] =  'a';
        http_type_string[8] =  'i';
        http_type_string[9] =  'n';

        /* Return the size of the HTTP ASCII type string.  */
        return(10);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_content_length_get              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns the content length of the supplied HTTP       */ 
/*    response packet.  If the packet is no content or the packet is      */ 
/*    invalid, a zero is returned.                                        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to HTTP request packet*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                            HTTP client control block     */
/*    length                                Length of content             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start         Start the GET operation       */
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
UINT  _nx_web_http_client_content_length_get(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{

UINT    length;
CHAR   *buffer_ptr;
UINT    found = NX_FALSE;


    NX_PARAMETER_NOT_USED(client_ptr);

    /* Default the content length to an invalid value.  */
    length =  0;

    /* Setup pointer to buffer.  */
    buffer_ptr = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;

    /* Find the "Content-length:" token first.  */
    while ((buffer_ptr+14) < (CHAR *)packet_ptr -> nx_packet_append_ptr)
    {

        /* Check for the Content-length token.  */
        if (((*buffer_ptr ==      'c') || (*buffer_ptr ==      'C')) &&
            ((*(buffer_ptr+1) ==  'o') || (*(buffer_ptr+1) ==  'O')) &&
            ((*(buffer_ptr+2) ==  'n') || (*(buffer_ptr+2) ==  'N')) &&
            ((*(buffer_ptr+3) ==  't') || (*(buffer_ptr+3) ==  'T')) &&
            ((*(buffer_ptr+4) ==  'e') || (*(buffer_ptr+4) ==  'E')) &&
            ((*(buffer_ptr+5) ==  'n') || (*(buffer_ptr+5) ==  'N')) &&
            ((*(buffer_ptr+6) ==  't') || (*(buffer_ptr+6) ==  'T')) &&
            (*(buffer_ptr+7) ==  '-') &&
            ((*(buffer_ptr+8) ==  'l') || (*(buffer_ptr+8) ==  'L')) &&
            ((*(buffer_ptr+9) ==  'e') || (*(buffer_ptr+9) ==  'E')) &&
            ((*(buffer_ptr+10) == 'n') || (*(buffer_ptr+10) == 'N')) &&
            ((*(buffer_ptr+11) == 'g') || (*(buffer_ptr+11) == 'G')) &&
            ((*(buffer_ptr+12) == 't') || (*(buffer_ptr+12) == 'T')) &&
            ((*(buffer_ptr+13) == 'h') || (*(buffer_ptr+13) == 'H')) &&
            (*(buffer_ptr+14) == ':'))
        {

            /* Yes, found content-length token.  */
            found = NX_TRUE;

            /* Move past the Content-Length: field. Exit the loop. */
            buffer_ptr += 15;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Check if found the content-length token.  */
    if (found != NX_TRUE)
    {

        /* No, return an invalid length indicating a bad HTTP packet. */
        return(length);
    }

    /* Now skip over white space. */
    while ((buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
    {
        buffer_ptr++;
    }

    /* Now convert the length into a numeric value.  */
    while ((buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr >= '0') && (*buffer_ptr <= '9'))
    {

        /* Update the content length.  */
        length =  length * 10;
        length =  length + (((UINT) (*buffer_ptr)) - 0x30);

        /* Move the buffer pointer forward.  */
        buffer_ptr++;
    }

    /* Determine if the content length was picked up properly.  */
    if ((buffer_ptr >= (CHAR *)packet_ptr -> nx_packet_append_ptr) ||
        ((*buffer_ptr != ' ') && (*buffer_ptr != (CHAR)13)))
    {

        /* Error, set the length to zero.  */
        length =  0;
    }

    /* Return the length to the caller.  */
    return(length);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_memicmp                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compares two pieces of memory case insensitive.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    src                                   Pointer to source             */ 
/*    src_length                            Length of source              */ 
/*    dest                                  Pointer to destination        */ 
/*    dest_length                           Length of destination         */ 
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
/*    _nx_web_http_client_process_header_fields                           */
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
UINT  _nx_web_http_client_memicmp(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length)
{
UCHAR   ch;

    /* Compare the length. */
    if(src_length != dest_length)
        return NX_WEB_HTTP_FAILED;

    while(src_length)
    {

        /* Is src lowercase? */
        if((*src >= 'a') && (*src <= 'z'))
            ch = (UCHAR)(*src - 'a' + 'A');

        /* Is src uppercase? */
        else if((*src >= 'A') && (*src <= 'Z'))
            ch = (UCHAR)(*src - 'A' + 'a');
        else
            ch = *src;

        /* Compare case insensitive. */
        if((*src != *dest) && (ch != *dest))
            return NX_WEB_HTTP_FAILED;

        /* Pickup next character. */
        src_length--;
        src++;
        dest++;
    }

    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_process_header_fields           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the HTTP headers received from the remote   */
/*    server. It also calculates the byte offset to the start of the      */
/*    HTTP response content area.  This area immediately follows the HTTP */
/*    response header (which ends with a blank line).                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to request packet     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Byte Offset                           (0 implies no content)        */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_client_memicmp           Compile two strings           */
/*    [nx_web_http_client_response_callback]                              */
/*                                          Application header callback   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start         Start GET processing          */
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
UINT  _nx_web_http_client_process_header_fields(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{

UINT    offset;
CHAR    *buffer_ptr;
CHAR    *field_name;
UINT     field_name_length;
CHAR    *field_value;
UINT     field_value_length;
#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
UINT    version = 0;
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */


    /* Default the content offset to zero.  */
    offset =  0;
    client_ptr -> nx_web_http_client_response_chunked = NX_FALSE;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    if ((buffer_ptr + 7) < (CHAR *) packet_ptr -> nx_packet_append_ptr)
    {
        version = (UCHAR)((buffer_ptr[5] - '0') << 4) | (UCHAR)(buffer_ptr[7] - '0');
    }

    /* Is a valid HTTP version found?  */
    if(version == 0)
    {
        client_ptr -> nx_web_http_client_keep_alive = NX_FALSE;
        return 0;
    }

    /* Initialize the keepalive flag.  */
    if(version > 0x10)
        client_ptr -> nx_web_http_client_keep_alive = NX_TRUE;
    else
        client_ptr -> nx_web_http_client_keep_alive = NX_FALSE;
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* Skip over the first HTTP line (e.g. HTTP/1.1 200 OK).  */
    while(((buffer_ptr+1) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 13) && (*(buffer_ptr + 1) !=  (CHAR) 10))
    {
        buffer_ptr++;
        offset++;
    }

    /* Skip over the CR,LF. */
    buffer_ptr += 2;
    offset += 2;

    /* Loop until we find the "cr,lf,cr,lf" token.  */
    while (((buffer_ptr+1) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the <cr,lf,cr,lf> token.  This signals a blank line, which also 
           specifies the start of the content.  */
        if ((*buffer_ptr ==      (CHAR) 13) &&
            (*(buffer_ptr+1) ==  (CHAR) 10))
        {

            /* Adjust the offset.  */
            offset =  offset + 2;
            break;
        }


        /* We haven't seen the <cr,lf,cr,lf> so we are still processing header data.
         * Extract the field name and it's value.
         */
        field_name = buffer_ptr;
        field_name_length = 0;

        /* Look for the ':' that separates the field name from its value. */
        while(*buffer_ptr != ':')
        {
            buffer_ptr++;
            field_name_length++;
        }
        offset += field_name_length;

        /* Skip ':'.  */
        buffer_ptr++;
        offset++;

        /* Now skip over white space. */
        while ((buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
        {
            buffer_ptr++;
            offset++;
        }

        /* Now get the field value. */
        field_value = buffer_ptr;
        field_value_length = 0;

        /* Loop until we see a <CR, LF>. */
        while(((buffer_ptr+1) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 13) && (*(buffer_ptr+1) !=  (CHAR) 10))
        {
            buffer_ptr++;
            field_value_length++;
        }
        offset += field_value_length;

        /* Skip over the CR,LF. */
        buffer_ptr += 2;
        offset += 2;

        /* Check if the response packet is chunked.  */
        if (_nx_web_http_client_memicmp((UCHAR *)field_name, field_name_length, (UCHAR *)"Transfer-Encoding", 17) == 0)
        {
            if (_nx_web_http_client_memicmp((UCHAR *)field_value, field_value_length, (UCHAR *)"chunked", 7) == 0)
            {
                client_ptr -> nx_web_http_client_response_chunked = NX_TRUE;
            }
        }

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE

        /* If the "connection" field exist, and the value is "keep-alive", set the keep-alive flag to TRUE. */
        if (_nx_web_http_client_memicmp((UCHAR *)"connection", 10, (UCHAR *)field_name, field_name_length) == NX_SUCCESS)
        {
            if (_nx_web_http_client_memicmp((UCHAR *)"keep-alive", 10, (UCHAR *)field_value, field_value_length) == NX_SUCCESS)
            {
                client_ptr -> nx_web_http_client_keep_alive = NX_TRUE;
            }
            else if (_nx_web_http_client_memicmp((UCHAR *)"close", 5, (UCHAR *)field_value, field_value_length) == NX_SUCCESS)
            {
                client_ptr -> nx_web_http_client_keep_alive = NX_FALSE;
            }
        }
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

        /* Invoke the header callback if present. */
        /* If it was set, invoke the application response callback. */
        if(client_ptr -> nx_web_http_client_response_callback)
        {
            client_ptr -> nx_web_http_client_response_callback(client_ptr, field_name, field_name_length, field_value, field_value_length);
        }
    }

    /* Not find the "cr,lf,cr,lf" token.  */
    if (((buffer_ptr+1) >= (CHAR *) packet_ptr -> nx_packet_append_ptr) || (*buffer_ptr == (CHAR) 0))
    {
        offset = 0;
    }

    /* Return the offset to the caller.  */
    return(offset);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_client_number_convert                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a number into an ASCII string.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    number                                Unsigned integer number       */ 
/*    string                                Destination string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Size                                  Number of bytes in string     */ 
/*                                           (0 implies an error)         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_client_get_start         Start GET processing          */
/*    _nx_web_http_client_put_start         Start PUT processing          */
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
UINT  _nx_web_http_client_number_convert(UINT number, CHAR *string)
{

UINT    j;
UINT    digit;
UINT    size;


    /* Default string to return '0'.  */
    string[0] = '0';

    /* Initialize counters.  */
    size =  0;

    /* Loop to convert the number to ASCII.  */
    while ((size < 10) && (number))
    {

        /* Shift the current digits over one.  */
        for (j = size; j != 0; j--)
        {

            /* Move each digit over one place.  */
            string[j] =  string[j-1];
        }

        /* Compute the next decimal digit.  */
        digit =  number % 10;

        /* Update the input number.  */
        number =  number / 10;

        /* Store the new digit in ASCII form.  */
        string[0] =  (CHAR) (digit + 0x30);

        /* Increment the size.  */
        size++;
    }

    /* Make the string NULL terminated.  */
    string[size] =  (CHAR) NX_NULL;

    /* Determine if there is an overflow error.  */
    if (number)
    {

        /* Error, return bad values to user.  */
        size =  0;
        string[0] = '0';
    }

    /* Return size to caller.  */
    return(size);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_https_client_connect                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to connect the HTTP     */
/*    client to a remote HTTP server.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             Address of remote server      */
/*    server_port                           HTTPS port on remote server   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_connect           Actual HTTP connect call      */
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
UINT  _nxe_web_http_client_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option)

{
UINT status;

    /* Check pointers. */
    if(client_ptr == NX_NULL || server_ip == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual function. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, server_port, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_https_client_connect                        PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function connects a HTTP client on the specified IP. The TCP   */
/*    socket is left open following this call, allowing multiple          */
/*    operations (e.g. GET, PUT, etc.) on this client instance before     */
/*    closing the HTTP connection.                                        */
/*                                                                        */
/*    Note that multiple operations implies that the HTTP keepalive       */
/*    header is present and supported by the remote host. Performing a    */
/*    single operation and then disconnecting the socket is equivalent to */
/*    a standard non-keepalive HTTP session.                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             Address of remote server      */
/*    server_port                           HTTPS port on remote server   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_cleanup           Rest the state                */
/*    nx_tcp_client_socket_bind             Bind TCP client socket        */
/*    nxd_tcp_client_socket_connect         Connect to remote server      */
/*    nx_tcp_client_socket_unbind           Unbind socket on error        */
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
/*  03-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            supported non-blocking mode,*/
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_client_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option)

{

UINT        status;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE

    /* If keep-alive is true, the client is already connected to the server.  */
    if (client_ptr -> nx_web_http_client_keep_alive)
    {

        /* Clean up and reset the state.  */
        _nx_web_http_client_cleanup(client_ptr);

        if (nx_tcp_socket_state_wait(&(client_ptr -> nx_web_http_client_socket), NX_TCP_ESTABLISHED, 0) == NX_SUCCESS)
        {

            /* No need to reconnect.  */
            return(NX_SUCCESS);
        }
        else
        {

            /* Server disconnected the connection, need to reconnect.  */
            _nx_web_http_client_error_exit(client_ptr, wait_option);
        }
    }
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* Determine if the client is in a ready state.  */
    if (client_ptr -> nx_web_http_client_state != NX_WEB_HTTP_CLIENT_STATE_READY)
    {

        /* Reset the state.  */
        _nx_web_http_client_cleanup(client_ptr);
    }

    /* Attempt to bind the client socket.  */
    status =  nx_tcp_client_socket_bind(&(client_ptr -> nx_web_http_client_socket), NX_ANY_PORT, wait_option);

    /* Check status of the bind.  */
    if ((status != NX_SUCCESS) && (status != NX_ALREADY_BOUND))
    {

        /* Error binding to a port, return to caller.  */
        return(status);
    }

    /* Set the server IP and port. */
#ifdef FEATURE_NX_IPV6
    COPY_NXD_ADDRESS(server_ip, &(client_ptr -> nx_web_http_client_server_address));
#else
    client_ptr -> nx_web_http_client_server_address.nxd_ip_version = NX_IP_VERSION_V4;
    client_ptr -> nx_web_http_client_server_address.nxd_ip_address.v4 = server_ip -> nxd_ip_address.v4;
#endif

    client_ptr -> nx_web_http_client_connect_port = server_port;

    /* Connect to the HTTPS server with TCP.  */
    status = nxd_tcp_client_socket_connect(&(client_ptr -> nx_web_http_client_socket), server_ip,
                                             client_ptr -> nx_web_http_client_connect_port, wait_option);

    /* Check for connection status.  */
    if ((status != NX_SUCCESS) && (status != NX_IN_PROGRESS))
    {

        /* Error, unbind the port and return an error.  */
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_web_http_client_socket));
    }

    return(status);
}

#ifdef NX_WEB_HTTPS_ENABLE

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_https_client_secure_connect                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the secure HTTPS connect call.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             Address of remote server      */
/*    server_port                           HTTPS port on remote server   */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_https_client_secure_connect  Actual secure connect call    */
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
UINT  _nxe_web_http_client_secure_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                         UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                         ULONG wait_option)
{
UINT        status;


    /* Check pointers. */
    if(client_ptr == NX_NULL || server_ip == NX_NULL || tls_setup == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual function. */
    status = _nx_web_http_client_secure_connect(client_ptr, server_ip, server_port, tls_setup, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_https_client_secure_connect                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function connects a HTTPS client on the specified IP. The TLS  */
/*    session in the NX_WEB_HTTP_CLIENT structure is initialized using    */
/*    application-supplied callback.                                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    server_ip                             Address of remote server      */
/*    server_port                           HTTPS port on remote server   */
/*    tls_setup                             User-supplied callback        */
/*                                            function to set up TLS      */
/*                                            parameters.                 */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_connect           Connect to server with TCP    */
/*    _nx_secure_tls_session_start          Connect to server with TLS    */
/*    _nx_web_http_client_error_exit        Cleanup and shut down HTTPS   */
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
UINT  _nx_web_http_client_secure_connect(NX_WEB_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                         UINT (*tls_setup)(NX_WEB_HTTP_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *),
                                         ULONG wait_option)
{
UINT        status;

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE

    /* If keep-alive is true, the client is already connected to the server.  */
    if (client_ptr -> nx_web_http_client_keep_alive && 
        (nx_tcp_socket_state_wait(&(client_ptr -> nx_web_http_client_socket), NX_TCP_ESTABLISHED, 0) == NX_SUCCESS))
    {

        /* Reset the state.  */
        _nx_web_http_client_cleanup(client_ptr);
        return(NX_SUCCESS);
    }
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* We have an HTTPS client, mark it as such. */
    client_ptr -> nx_web_http_client_is_https = NX_TRUE;

    /* Attempt to connect to the HTTPS server using just TCP - we need to establish the TCP socket
       before we attempt to perform a TLS handshake for HTTPS. */
    status = _nx_web_http_client_connect(client_ptr, server_ip, server_port, wait_option);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Call the TLS setup callback to initialize TLS configuration. */
    status = (*tls_setup)(client_ptr, &(client_ptr->nx_web_http_client_tls_session));

    if(status != NX_SUCCESS)
    {

        /* Error, unbind the port and return an error.  */
        _nx_tcp_socket_disconnect(&(client_ptr -> nx_web_http_client_socket), wait_option);
        _nx_tcp_client_socket_unbind(&(client_ptr -> nx_web_http_client_socket));
        return(status);
    }

    /* Start TLS session after connecting TCP socket. */
    status = _nx_secure_tls_session_start(&(client_ptr -> nx_web_http_client_tls_session), &(client_ptr -> nx_web_http_client_socket), wait_option);

    if (status != NX_SUCCESS)
    {
        /* Error, unbind the port and return an error.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);
        return(status);
    }

    /* At this point we have a connection setup with an HTTPS server!  */

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

#endif /* NX_WEB_HTTPS_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_initialize             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP request initialize call.*/
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    method                                HTTP request type (e.g. GET)  */
/*    resource                              HTTP resource (e.g. URL)      */
/*    host                                  Pointer to Host               */
/*    input_size                            Size of input for PUT/POST    */
/*    transfer_encoding_chunked             Chunked encoding flag         */
/*    username                              Username for server access    */
/*    password                              Password for server access    */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_initialize                              */
/*                                          Actual request create call    */
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
UINT _nxe_web_http_client_request_initialize(NX_WEB_HTTP_CLIENT *client_ptr,
                                             UINT method, /* GET, PUT, DELETE, POST, HEAD */
                                             CHAR *resource,
                                             CHAR *host,
                                             UINT input_size, /* Used by PUT and POST */
                                             UINT transfer_encoding_chunked, /* If true, input_size is ignored. */
                                             CHAR *username,
                                             CHAR *password,
                                             UINT wait_option)
{
UINT        status;

    /* Check pointers. We need a control block and a resource but
       username and password can be NULL. */
    if(client_ptr == NX_NULL || resource == NX_NULL || host == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    status = NX_SUCCESS;

    /* Check methods. */
    switch(method)
    {
        case NX_WEB_HTTP_METHOD_POST:
        case NX_WEB_HTTP_METHOD_PUT:
            /* PUT and POST need an input size. */
            if (!input_size && !transfer_encoding_chunked)
            {
                status = NX_WEB_HTTP_METHOD_ERROR;
            }
            break;
        case NX_WEB_HTTP_METHOD_GET:
        case NX_WEB_HTTP_METHOD_DELETE:
        case NX_WEB_HTTP_METHOD_HEAD:
            /* These are good as-is. */
            break;
        case NX_WEB_HTTP_METHOD_NONE:
        default:
            /* Improper or missing method. */
            status = NX_WEB_HTTP_METHOD_ERROR;
    }

    /* Check status from method creation. */
    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Call actual function. */
    status = _nx_web_http_client_request_initialize(client_ptr, method, resource, host, input_size,
                                                    transfer_encoding_chunked, username, password,
                                                    wait_option);


    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_initialize              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes an HTTP request using information         */
/*    supplied by the caller. When a request is initialized, a packet is  */
/*    allocated internally which can then be modified before sending the  */
/*    final request to the server.                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    method                                HTTP request type (e.g. GET)  */
/*    resource                              HTTP resource (e.g. URL)      */
/*    host                                  Pointer to Host               */
/*    input_size                            Size of input for PUT/POST    */
/*    transfer_encoding_chunked             Chunked encoding flag         */
/*    username                              Username for server access    */
/*    password                              Password for server access    */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Actual request create call    */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT _nx_web_http_client_request_initialize(NX_WEB_HTTP_CLIENT *client_ptr,
                                            UINT method, /* GET, PUT, DELETE, POST, HEAD */
                                            CHAR *resource,
                                            CHAR *host,
                                            UINT input_size, /* Used by PUT and POST */
                                            UINT transfer_encoding_chunked, /* If true, input_size is ignored. */
                                            CHAR *username,
                                            CHAR *password,
                                            UINT wait_option)
{
UINT temp_resource_length;
UINT temp_host_length;
UINT temp_username_length = 0;
UINT temp_password_length = 0;

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, NX_WEB_HTTP_MAX_NAME) || 
            _nx_utility_string_length_check(password, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    return(_nx_web_http_client_request_initialize_extended(client_ptr, method, resource,
                                                           temp_resource_length, host, temp_host_length,
                                                           input_size, transfer_encoding_chunked,
                                                           username, temp_username_length,
                                                           password, temp_password_length,
                                                           wait_option));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_initialize_extended    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP request initialize call.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    method                                HTTP request type (e.g. GET)  */
/*    resource                              HTTP resource (e.g. URL)      */
/*    resource_length                       Length of resource            */
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    input_size                            Size of input for PUT/POST    */
/*    transfer_encoding_chunked             Chunked encoding flag         */
/*    username                              Username for server access    */
/*    username_length                       Length of username            */
/*    password                              Password for server access    */
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_initialize_extended                     */
/*                                          Actual request create call    */
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
UINT _nxe_web_http_client_request_initialize_extended(NX_WEB_HTTP_CLIENT *client_ptr,
                                                      UINT method, /* GET, PUT, DELETE, POST, HEAD */
                                                      CHAR *resource,
                                                      UINT resource_length,
                                                      CHAR *host,
                                                      UINT host_length,
                                                      UINT input_size, /* Used by PUT and POST */
                                                      UINT transfer_encoding_chunked, /* If true, input_size is ignored. */
                                                      CHAR *username,
                                                      UINT username_length,
                                                      CHAR *password,
                                                      UINT password_length,
                                                      UINT wait_option)
{
UINT        status;

    /* Check pointers. We need a control block and a resource but
       username and password can be NULL. */
    if(client_ptr == NX_NULL || resource == NX_NULL || host == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    status = NX_SUCCESS;

    /* Check methods. */
    switch(method)
    {
        case NX_WEB_HTTP_METHOD_POST:
        case NX_WEB_HTTP_METHOD_PUT:
            /* PUT and POST need an input size. */
            if (!input_size && !transfer_encoding_chunked)
            {
                status = NX_WEB_HTTP_METHOD_ERROR;
            }
            break;
        case NX_WEB_HTTP_METHOD_GET:
        case NX_WEB_HTTP_METHOD_DELETE:
        case NX_WEB_HTTP_METHOD_HEAD:
            /* These are good as-is. */
            break;
        case NX_WEB_HTTP_METHOD_NONE:
        default:
            /* Improper or missing method. */
            status = NX_WEB_HTTP_METHOD_ERROR;
    }

    /* Check status from method creation. */
    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Call actual function. */
    status = _nx_web_http_client_request_initialize_extended(client_ptr, method, resource, resource_length,
                                                             host, host_length, input_size,
                                                             transfer_encoding_chunked, username,
                                                             username_length, password, password_length,
                                                             wait_option);


    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_initialize_extended     PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes an HTTP request using information         */
/*    supplied by the caller. When a request is initialized, a packet is  */
/*    allocated internally which can then be modified before sending the  */
/*    final request to the server.                                        */
/*                                                                        */
/*    Note: The strings of resource, host, username and password must be  */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    method                                HTTP request type (e.g. GET)  */
/*    resource                              HTTP resource (e.g. URL)      */
/*    resource_length                       Length of resource            */
/*    host                                  Pointer to Host               */
/*    host_length                           Length of host                */
/*    input_size                            Size of input for PUT/POST    */
/*    transfer_encoding_chunked             Chunked encoding flag         */
/*    username                              Username for server access    */
/*    username_length                       Length of username            */
/*    password                              Password for server access    */
/*    password_length                       Length of password            */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_packet_allocate                         */
/*                                          Allocate a request packet     */
/*    _nx_web_http_client_error_exit        Cleanup and shut down HTTPS   */
/*    nx_packet_data_append                 Append packet data            */
/*    _nx_utility_string_length_check       Check string length           */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            parsing base64,             */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_client_request_initialize_extended(NX_WEB_HTTP_CLIENT *client_ptr,
                                                     UINT method, /* GET, PUT, DELETE, POST, HEAD */
                                                     CHAR *resource,
                                                     UINT resource_length,
                                                     CHAR *host,
                                                     UINT host_length,
                                                     UINT input_size, /* Used by PUT and POST */
                                                     UINT transfer_encoding_chunked, /* If true, input_size is ignored. */
                                                     CHAR *username,
                                                     UINT username_length,
                                                     CHAR *password,
                                                     UINT password_length,
                                                     UINT wait_option)
{
UINT        status;
NX_PACKET   *packet_ptr;
CHAR        string1[NX_WEB_HTTP_MAX_NAME + NX_WEB_HTTP_MAX_PASSWORD + 2];
CHAR        string2[NX_WEB_HTTP_MAX_STRING + 1];
CHAR        crlf[2] = {13,10};
UINT        temp_resource_length;
UINT        temp_host_length;
UINT        temp_username_length = 0;
UINT        temp_password_length = 0;
UINT        string1_length;
UINT        string2_length;

    /* If there is old request packet, just release it.  */
    if (client_ptr -> nx_web_http_client_request_packet_ptr)
    {
        nx_packet_release(client_ptr -> nx_web_http_client_request_packet_ptr);
        client_ptr -> nx_web_http_client_request_packet_ptr = NX_NULL;
    }

    if ((username) && (password))
    {

        /* Check username and password length.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, username_length) || 
            _nx_utility_string_length_check(password, &temp_password_length, password_length))
        {
            return(NX_WEB_HTTP_ERROR);
        }

        /* Validate string length. */
        if ((username_length != temp_username_length) ||
            (password_length != temp_password_length))
        {
            return(NX_WEB_HTTP_ERROR);
        }
    }

    /* Check resource and host length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(host, &temp_host_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Validate string length. */
    if ((resource_length != temp_resource_length) ||
        (host_length != temp_host_length))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Allocate a packet for the request message.  */
    status = _nx_web_http_client_request_packet_allocate(client_ptr, &(client_ptr -> nx_web_http_client_request_packet_ptr), wait_option);

    /* Check allocation status.  */
    if (status != NX_SUCCESS)
    {
        /* Error, unbind the port and return an error.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        return(status);
    }

    /* Get a working pointer to our newly-allocated packet. */
    packet_ptr = client_ptr -> nx_web_http_client_request_packet_ptr;

    /* Determine the requested method.  */
    status = NX_SUCCESS;
    client_ptr -> nx_web_http_client_method = method;
    switch(method)
    {
    case NX_WEB_HTTP_METHOD_GET:
        /* Build the GET request.  */
        nx_packet_data_append(packet_ptr, "GET ", 4, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);
        break;
    case NX_WEB_HTTP_METHOD_PUT:
        /* Additional information requested, build the POST request.  */
        nx_packet_data_append(packet_ptr, "PUT ", 4, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);

        /* Store the total number of bytes to send.  */
        client_ptr -> nx_web_http_client_total_transfer_bytes =     input_size;
        client_ptr -> nx_web_http_client_actual_bytes_transferred =  0;

        break;
    case NX_WEB_HTTP_METHOD_POST:
        /* Additional information requested, build the POST request.  */
        nx_packet_data_append(packet_ptr, "POST ", 5, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);

        /* Store the total number of bytes to send.  */
        client_ptr -> nx_web_http_client_total_transfer_bytes =     input_size;
        client_ptr -> nx_web_http_client_actual_bytes_transferred =  0;

        break;
    case NX_WEB_HTTP_METHOD_DELETE:
        /* Build the DELETE request.  */
        nx_packet_data_append(packet_ptr, "DELETE ", 7, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);
        break;
    case NX_WEB_HTTP_METHOD_HEAD:
        /* Build the HEAD request.  */
        nx_packet_data_append(packet_ptr, "HEAD ", 5, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);
        break;
    case NX_WEB_HTTP_METHOD_NONE:
    default:
        status = NX_WEB_HTTP_METHOD_ERROR;
        break;
    }

    /* Check status from method creation. */
    if(status != NX_SUCCESS)
    {

        /* Release the request packet.  */
        nx_packet_release(packet_ptr);
        client_ptr -> nx_web_http_client_request_packet_ptr = NX_NULL;
        return(status);
    }

    /* Determine if the resource needs a leading "/".  */
    if (resource[0] != '/')
    {

        /* Is this another website e.g. begins with http or https and has a colon? */
        if  (
             ((resource[0] == 'h') || (resource[0] == 'H')) &&
             ((resource[1] == 't') || (resource[1] == 'T')) &&
             ((resource[2] == 't') || (resource[2] == 'T')) &&
             ((resource[3] == 'p') || (resource[3] == 'P'))  &&
             ( (resource[4] == ':') ||
             (((resource[4] == 's') || (resource[4] == 'S')) && (resource[5] == ':')))
            )
        {

            /* Yes, to send this string as is. */
        }
        else
        {

            /* Local file URI which needs a leading '/' character.  */
            nx_packet_data_append(packet_ptr, "/", 1, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
        }
    }
    /* Else URI string refers to root directory file, and has a leading '/' character already. */

    /* Place the resource in the header.  */
    nx_packet_data_append(packet_ptr, resource, resource_length, client_ptr -> nx_web_http_client_packet_pool_ptr,
                          wait_option);

    /* Place the HTTP version in the header.  */
    nx_packet_data_append(packet_ptr, " HTTP/1.1", 9, client_ptr -> nx_web_http_client_packet_pool_ptr,
                          wait_option);

    /* Place the end of line character in the header.  */
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr,
                          wait_option);

    /* Place the Host in the header.  */
    nx_packet_data_append(packet_ptr, "Host: ", 6, client_ptr -> nx_web_http_client_packet_pool_ptr,
                          wait_option);

    if (host_length)
    {
        nx_packet_data_append(packet_ptr, host, host_length, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);
    }
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr,
                          wait_option);

    /* Determine if basic authentication is required.  */
    if ((username) && (password))
    {

        /* Yes, attempt to build basic authentication.  */
        nx_packet_data_append(packet_ptr, "Authorization: Basic ", 21, client_ptr -> nx_web_http_client_packet_pool_ptr,
                              wait_option);

        /* Encode and append the "name:password" into next.  */

        /* Place the name and password in a single string.  */

        /* Copy the name into the merged string.  */
        memcpy(string1, username, username_length); /* Use case of memcpy is verified. */

        /* Insert the colon.  */
        string1[username_length] =  ':';

        /* Copy the password into the merged string.  */
        memcpy(&string1[username_length + 1], password, password_length); /* Use case of memcpy is verified. */

        /* Make combined string NULL terminated.  */
        string1[username_length + password_length + 1] =  NX_NULL;

        /* Now encode the username:password string.  */
        _nx_utility_base64_encode((UCHAR *)string1, username_length + password_length + 1, (UCHAR *)string2, sizeof(string2), &string2_length);
        nx_packet_data_append(packet_ptr, string2, string2_length, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
    }

    /* Check to see if a Content-Length field is needed.  */
    if ((input_size) && (!transfer_encoding_chunked))
    {

        /* Now build the content-length entry.  */
        nx_packet_data_append(packet_ptr, "Content-Length: ", 16, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
        string1_length =  _nx_web_http_client_number_convert(input_size, string1);
        nx_packet_data_append(packet_ptr, string1, string1_length, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
    }

    if (transfer_encoding_chunked)
    {

        /* Place the Transfer Encoding in the header.  */
        nx_packet_data_append(packet_ptr, "Transfer-Encoding: chunked", 26, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_header_add             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when calling the custom header      */
/*    addition routine.                                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    field_name                            HTTP header field name        */
/*    name_length                           Length of field name string   */
/*    field_value                           Value of header field         */
/*    value_length                          Length of value string        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_request_header_add                              */
/*                                          Actual header add call        */
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
UINT _nxe_web_http_client_request_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT name_length,
                                            CHAR *field_value, UINT value_length, UINT wait_option)
{
UINT status;

    /* Check pointers. */
    if(client_ptr == NX_NULL || field_name == NX_NULL || field_value == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call the actual function. */
    status = _nx_web_http_client_request_header_add(client_ptr, field_name, name_length,
                                                    field_value, value_length, wait_option);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_header_add              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a custom header to an HTTP(S) request that was   */
/*    previously initialized with nx_web_http_client_request_initialize.  */
/*    The field name and value are placed directly into the HTTP header   */
/*    so it is up to the caller to assure that they are valid values.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    field_name                            HTTP header field name        */
/*    name_length                           Length of field name string   */
/*    field_value                           Value of header field         */
/*    value_length                          Length of value string        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append data to request packet */
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
UINT _nx_web_http_client_request_header_add(NX_WEB_HTTP_CLIENT *client_ptr, CHAR *field_name, UINT name_length,
                                            CHAR *field_value, UINT value_length, UINT wait_option)
{
NX_PACKET   *packet_ptr;
UINT        status;
CHAR        crlf[2] = {13,10};


    /* Make sure a request packet was allocated. */
    if(client_ptr -> nx_web_http_client_request_packet_ptr == NX_NULL)
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Get a working pointer to our newly-allocated packet. */
    packet_ptr = client_ptr -> nx_web_http_client_request_packet_ptr;


    /* Add custom header fields to request packet.  */
    status = nx_packet_data_append(packet_ptr, field_name, name_length, client_ptr -> nx_web_http_client_packet_pool_ptr,
                                   wait_option);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    status = nx_packet_data_append(packet_ptr, ": ", 2, client_ptr -> nx_web_http_client_packet_pool_ptr,
                                   wait_option);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    status = nx_packet_data_append(packet_ptr, field_value, value_length, client_ptr -> nx_web_http_client_packet_pool_ptr,
                                   wait_option);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    status = nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_send                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP request send API call.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_client_request_send     Actual request send call      */
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
UINT _nxe_web_http_client_request_send(NX_WEB_HTTP_CLIENT *client_ptr, ULONG wait_option)
{
UINT        status;

    /* Make sure the client instance makes sense. */
    if(client_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Now send the packet to the HTTP server.  */
    status =  _nx_web_http_client_request_send(client_ptr, wait_option);

    /* Return success to the caller.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_send                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends an HTTP request to a remote server. The request */
/*    being sent must have been initialized previously with a call to     */
/*    nx_web_http_client_request_initialize.                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append packet data            */
/*    nx_packet_release                     release packet                */
/*    _nx_web_http_client_error_exit        Cleanup and shut down HTTPS   */
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
UINT _nx_web_http_client_request_send(NX_WEB_HTTP_CLIENT *client_ptr, ULONG wait_option)
{
UINT        status;
NX_PACKET   *packet_ptr;
CHAR        crlf[2] = {13,10};


    /* Make sure a request packet was allocated. */
    if(client_ptr -> nx_web_http_client_request_packet_ptr == NX_NULL)
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Get a pointer to our current request packet. */
    packet_ptr = client_ptr -> nx_web_http_client_request_packet_ptr;

    /* Clear the request packet pointer.  */
    client_ptr -> nx_web_http_client_request_packet_ptr = NX_NULL;

    /* Place an extra cr/lf to signal the end of the HTTP header.  */
    nx_packet_data_append(packet_ptr, crlf, 2,
                          client_ptr -> nx_web_http_client_packet_pool_ptr, wait_option);

    /* Now send the packet to the HTTP server.  */
    status =  _nx_web_http_client_send(client_ptr, packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Disconnect and unbind the socket.  */
        _nx_web_http_client_error_exit(client_ptr, wait_option);

        /* Return an error.  */
        return(status);
    }

    /* Set the appropriate state. */
    switch(client_ptr -> nx_web_http_client_method)
    {
    case NX_WEB_HTTP_METHOD_GET:
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_GET;
        break;
    case NX_WEB_HTTP_METHOD_PUT:
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_PUT;
        break;
    case NX_WEB_HTTP_METHOD_POST:
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_POST;
        break;
    case NX_WEB_HTTP_METHOD_DELETE:
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_DELETE;
        break;
    case NX_WEB_HTTP_METHOD_HEAD:
        client_ptr -> nx_web_http_client_state =  NX_WEB_HTTP_CLIENT_STATE_HEAD;
        break;
    case NX_WEB_HTTP_METHOD_NONE:
    default:
        status = NX_WEB_HTTP_METHOD_ERROR;
        break;
    }


    /* Return success to the caller.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_request_packet_allocate        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to allocate packets for */
/*    an HTTP(S) client session, as used for PUT and POST operations.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to allocated packet   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_client_request_packet_allocate                        */
/*                                          Actual packet allocation call */
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
UINT _nxe_web_http_client_request_packet_allocate(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr,
                                                  UINT wait_option)
{
UINT status;

    /* Check our pointers. */
    if(client_ptr == NX_NULL || packet_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call the actual function. */
    status = _nx_web_http_client_request_packet_allocate(client_ptr, packet_ptr, wait_option);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_request_packet_allocate         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for Client HTTP(S) requests so     */
/*    that clients needing to send large amounts of data (such as for     */
/*    multi-part file upload) can allocated the needed packets and send   */
/*    them with nx_web_http_client_request_packet_send. Note that the     */
/*    appropriate headers must have been sent in the initial request.     */
/*    Headers can be added to the HTTP request with                       */
/*    _nx_web_http_client_request_header_add.                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to allocated packet   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_packet_allocate         Allocate a TLS packet         */
/*    nx_packet_allocate                    Allocate a TCP packet         */
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
UINT _nx_web_http_client_request_packet_allocate(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT wait_option)
{
UINT status;
NXD_ADDRESS *server_ip;

    /* Get worker variables for use. */
    server_ip = &(client_ptr->nx_web_http_client_server_address);

#ifdef NX_WEB_HTTPS_ENABLE
    /* Allocate a packet for the GET (or POST) message.  */
    if(client_ptr->nx_web_http_client_is_https)
    {
        status = nx_secure_tls_packet_allocate(&(client_ptr->nx_web_http_client_tls_session), client_ptr -> nx_web_http_client_packet_pool_ptr,
                                               packet_ptr, wait_option);
    }
    else
#endif   
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(client_ptr -> nx_web_http_client_packet_pool_ptr,
                                     packet_ptr,
                                     NX_IPv4_TCP_PACKET, wait_option);
    }
    else
    {

        status =  nx_packet_allocate(client_ptr -> nx_web_http_client_packet_pool_ptr,
                                     packet_ptr,
                                     NX_IPv6_TCP_PACKET, wait_option);
    }


    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_client_response_header_callback_set   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the set response callback        */
/*    routine,                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    callback_function                     Pointer to callback routine   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_response_header_callback_set                    */
/*                                          Actual set routine            */
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
UINT _nxe_web_http_client_response_header_callback_set(NX_WEB_HTTP_CLIENT *client_ptr,
                                                       VOID (*callback_function)(NX_WEB_HTTP_CLIENT *client_ptr,
                                                                                 CHAR *field_name,
                                                                                 UINT field_name_length,
                                                                                 CHAR *field_value,
                                                                                 UINT field_value_length))
{
UINT status;


    /* Check for NULL pointers. */
    if(client_ptr == NX_NULL || callback_function == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Invoke the real function. */
    status = _nx_web_http_client_response_header_callback_set(client_ptr, callback_function);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_response_header_callback_set    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets a callback routine to be invoked when the HTTP   */
/*    client receives a response from the remote server. The callback     */
/*    allows the application to process the HTTP headers beyond the       */
/*    normal processing done by the NetX Web HTTP library.                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    callback_function                     Pointer to callback routine   */
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
UINT _nx_web_http_client_response_header_callback_set(NX_WEB_HTTP_CLIENT *client_ptr,
                                                      VOID (*callback_function)(NX_WEB_HTTP_CLIENT *client_ptr,
                                                                                CHAR *field_name,
                                                                                UINT field_name_length,
                                                                                CHAR *field_value,
                                                                                UINT field_value_length))
{

    /* Assign the function pointer to the HTTP client. */
    client_ptr -> nx_web_http_client_response_callback = callback_function;

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_send                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends data to the remote server using either the TCP  */
/*    socket for plain HTTP or the TLS session for HTTPS.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to packet to send.    */
/*    wait_option                           Indicates behavior if packet  */
/*                                             cannot be sent.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_send            Send data using TLS session   */
/*    nx_secure_tcp_socket_send             Send data using TCP socket    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_client_put_packet                                      */
/*    _nx_web_http_client_request_send                                    */
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
UINT  _nx_web_http_client_send(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT        status;

#ifdef NX_WEB_HTTPS_ENABLE
    /* End TLS session if using HTTPS. */
    if(client_ptr -> nx_web_http_client_is_https)
    {
        status = nx_secure_tls_session_send(&(client_ptr -> nx_web_http_client_tls_session), packet_ptr, wait_option);
    }
    else
#endif
    {
        status = nx_tcp_socket_send(&(client_ptr -> nx_web_http_client_socket), packet_ptr, wait_option);
    }

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_client_receive                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives data from the remote server using either the */
/*    TCP socket for plain HTTP or the TLS session for HTTPS.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to packet to send.    */
/*    wait_option                           Indicates behavior if packet  */
/*                                             cannot be received         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_receive            Receive data using TLS     */
/*    nx_secure_tcp_socket_receive             Receive data using TCP     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
UINT  _nx_web_http_client_receive(NX_WEB_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        status;

#ifdef NX_WEB_HTTPS_ENABLE
    /* End TLS session if using HTTPS. */
    if(client_ptr -> nx_web_http_client_is_https)
    {
        status = nx_secure_tls_session_receive(&(client_ptr -> nx_web_http_client_tls_session), packet_ptr, wait_option);
    }
    else
#endif
    {
        status = nx_tcp_socket_receive(&(client_ptr -> nx_web_http_client_socket), packet_ptr, wait_option);
    }

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_https_client_error_exit                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used whenever there is an error in the HTTP client */
/*    and the port needs to be unbound. If the client is using TLS for    */
/*    HTTPS, then it also needs to end the TLS session.                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    error_number                          Error status in caller        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_client_socket_unbind           Unbind TCP port               */
/*    nx_secure_tls_session_delete          Delete TLS session            */
/*    nx_secure_tls_session_end             End TLS session               */
/*    nx_tcp_socket_disconnect              Close TCP socket              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
VOID _nx_web_http_client_error_exit(NX_WEB_HTTP_CLIENT *client_ptr, UINT wait_option)
{
#ifdef NX_WEB_HTTPS_ENABLE
    /* End TLS session if using HTTPS. */
    if(client_ptr -> nx_web_http_client_is_https)
    {
        nx_secure_tls_session_end(&(client_ptr -> nx_web_http_client_tls_session), NX_WAIT_FOREVER);
        nx_secure_tls_session_delete(&(client_ptr -> nx_web_http_client_tls_session));
    }
#endif

    nx_tcp_socket_disconnect(&(client_ptr -> nx_web_http_client_socket), wait_option);
    nx_tcp_client_socket_unbind(&(client_ptr -> nx_web_http_client_socket));

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_https_client_cleanup                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used to clean up the temporary variables and reset */
/*    the state.                                                          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to HTTP client        */
/*    error_number                          Error status in caller        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_client_receive           Receive a packet from server  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
VOID _nx_web_http_client_cleanup(NX_WEB_HTTP_CLIENT *client_ptr)
{
UINT status;
NX_PACKET *response_packet_ptr;

    /* Receive the early response from server.  */
    while(1)
    {
        status =  _nx_web_http_client_receive(client_ptr, &response_packet_ptr, NX_NO_WAIT);

        if (status)
        {
            break;
        }
        else
        {
            nx_packet_release(response_packet_ptr);
        }
    }

    /* Clean up the temporary variables.  */
    client_ptr -> nx_web_http_client_total_transfer_bytes = 0;

    client_ptr -> nx_web_http_client_actual_bytes_transferred = 0;

    client_ptr -> nx_web_http_client_total_receive_bytes = 0;

    client_ptr -> nx_web_http_client_actual_bytes_received = 0;

    client_ptr -> nx_web_http_client_response_header_received = NX_FALSE;

    /* Release the request packet.  */
    if (client_ptr -> nx_web_http_client_request_packet_ptr)
    {
        nx_packet_release(client_ptr -> nx_web_http_client_request_packet_ptr);
        client_ptr -> nx_web_http_client_request_packet_ptr = NX_NULL;
    }

    client_ptr -> nx_web_http_client_request_chunked = NX_FALSE;

    client_ptr -> nx_web_http_client_response_chunked = NX_FALSE;

    client_ptr -> nx_web_http_client_chunked_response_remaining_size = 0;

    /* Release the response packet.  */
    if (client_ptr -> nx_web_http_client_response_packet)
    {
        nx_packet_release(client_ptr -> nx_web_http_client_response_packet);
        client_ptr -> nx_web_http_client_response_packet = NX_NULL;
    }

    /* Reset the state to ready.  */
    client_ptr -> nx_web_http_client_state = NX_WEB_HTTP_CLIENT_STATE_READY;
}

