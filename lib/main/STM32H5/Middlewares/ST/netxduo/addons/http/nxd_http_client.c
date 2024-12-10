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

#define NX_HTTP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */


#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nx_tcp.h"  
#include    "nxd_http_client.h"
#include    "stdio.h"
#include    "string.h"


/* Define global HTTP variables and strings.  */

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_create                             PORTABLE C      */ 
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
/*    _nx_http_client_create                Actual client create call     */ 
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
UINT  _nxe_http_client_create(NX_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size, UINT http_client_size)
{

NX_PACKET   *packet_ptr;
UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id == NXD_HTTP_CLIENT_ID) || 
        (pool_ptr == NX_NULL) || (http_client_size != sizeof(NX_HTTP_CLIENT)))
        return(NX_PTR_ERROR);

    /* Pickup a packet from the supplied packet pool.  */
    packet_ptr =  pool_ptr -> nx_packet_pool_available_list;

    /* Determine if the packet payload is equal to or greater than the maximum HTTP header supported.  */
    if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start) < NX_HTTP_CLIENT_MIN_PACKET_SIZE)
        return(NX_HTTP_POOL_ERROR);

    /* Call actual client create function.  */
    status =  _nx_http_client_create(client_ptr, client_name, ip_ptr, pool_ptr, window_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_create                              PORTABLE C      */ 
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
UINT  _nx_http_client_create(NX_HTTP_CLIENT *client_ptr, CHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, ULONG window_size)
{

UINT        status;


    /* Clear the HTTP Client structure.  */
    memset((void *) client_ptr, 0, sizeof(NX_HTTP_CLIENT));

    /* Create the Client's TCP socket.  */
    status =  nx_tcp_socket_create(ip_ptr, &(client_ptr -> nx_http_client_socket), client_name, 
                          NX_HTTP_TYPE_OF_SERVICE,  NX_HTTP_FRAGMENT_OPTION, NX_HTTP_TIME_TO_LIVE,
                          window_size, NX_NULL, NX_NULL);

    /* Determine if an error occurred.   */
    if (status != NX_SUCCESS)
    {

        /* Yes, return error code.  */
        return(status);
    }

    /* Save the Client name.  */
    client_ptr -> nx_http_client_name =  client_name;

    /* Save the IP pointer address.  */
    client_ptr -> nx_http_client_ip_ptr =  ip_ptr;

    /* Save the packet pool pointer.  */
    client_ptr -> nx_http_client_packet_pool_ptr =  pool_ptr;

    /* Set the client state to ready to indicate a get or put operation can be done.  */
    client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

    /* Set the Client ID to indicate the HTTP client thread is ready.  */
    client_ptr -> nx_http_client_id =  NXD_HTTP_CLIENT_ID;

    /* Set the default port the client connects to the HTTP server on (80). */
    client_ptr -> nx_http_client_connect_port = NX_HTTP_SERVER_PORT;


    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_delete                             PORTABLE C      */ 
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
/*    _nx_http_client_delete                Actual client delete call     */ 
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
UINT  _nxe_http_client_delete(NX_HTTP_CLIENT *client_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client delete function.  */
    status =  _nx_http_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_delete                              PORTABLE C      */ 
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
UINT  _nx_http_client_delete(NX_HTTP_CLIENT *client_ptr)
{

    /* Clear the client ID to indicate the HTTP client is no longer ready.  */
    client_ptr -> nx_http_client_id =  0;

    /* Determine if a GET or PUT state is present.  */
    if ((client_ptr -> nx_http_client_state == NX_HTTP_CLIENT_STATE_PUT) ||
        (client_ptr -> nx_http_client_state == NX_HTTP_CLIENT_STATE_GET))
    {

        /* Check for a saved packet.  */
        if (client_ptr -> nx_http_client_first_packet)
        {

            /* Release the packet.  */
            nx_packet_release(client_ptr -> nx_http_client_first_packet);
        }

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), NX_HTTP_CLIENT_TIMEOUT);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
    }

    /* Delete the TCP socket.  */
    nx_tcp_socket_delete(&(client_ptr -> nx_http_client_socket));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_set_connect_port                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client set connect port */
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    port                                  Port to connect to server on  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_http_client_set_connect_port      Actual set port call          */ 
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
UINT  _nxe_http_client_set_connect_port(NX_HTTP_CLIENT *client_ptr, UINT port)
{
UINT status;
  
  
    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for an invalid port.  */
    if (((ULONG)port) > (ULONG)NX_MAX_PORT)
    {
        return(NX_INVALID_PORT);
    }
    else if (port == 0) 
    {
        return(NX_INVALID_PORT);
    }

    status = _nx_http_client_set_connect_port(client_ptr, port);

    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_set_connect_port                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the HTTP Client port to connect to the server on.*/
/*    This is useful if the HTTP Client needs to connect to a server on   */
/*    another port than the default 80 port.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    port                                  Port to connect to server on  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Completion status             */ 
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
UINT  _nx_http_client_set_connect_port(NX_HTTP_CLIENT *client_ptr, UINT port)
{

    client_ptr-> nx_http_client_connect_port = port;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_get_start                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client get start call.  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IP address of HTTP Server     */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nx_http_client_get_start             Actual client get start call  */ 
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
UINT  _nxe_http_client_get_start(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, CHAR *input_ptr, 
                                 UINT input_size, CHAR *username, CHAR *password, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || (resource == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nx_http_client_get_start(client_ptr, ip_address, resource, input_ptr, input_size, 
                                        username, password, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(input_ptr);
    NX_PARAMETER_NOT_USED(input_size);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_get_start_extended                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP client get start call.  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IP address of HTTP Server     */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nx_http_client_get_start_extended    Actual client get start call  */ 
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
UINT  _nxe_http_client_get_start_extended(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, UINT resource_length, 
                                          CHAR *input_ptr, UINT input_size, CHAR *username, UINT username_length, 
                                          CHAR *password, UINT password_length, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || (resource == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nx_http_client_get_start_extended(client_ptr, ip_address, resource, resource_length, 
                                                 input_ptr, input_size, username, username_length,
                                                 password, password_length, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(resource_length);
    NX_PARAMETER_NOT_USED(input_ptr);
    NX_PARAMETER_NOT_USED(input_size);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(username_length);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(password_length);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_get_start                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is a wrapper for the actual _nxd_http_client_get_start*/ 
/*    service that enables HTTP applications running on IPv4 to access    */
/*    HTTP duo API without enabling IPv6.                                 */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nxd_http_client_get_start            Actual HTTP Client duo get    */
/*                                               start service            */
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
UINT  _nx_http_client_get_start(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, CHAR *input_ptr, 
                                UINT input_size, CHAR *username, CHAR *password, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT            status;
NXD_ADDRESS     server_ip_addr;

    /* Construct an IP address structure, and fill in IPv4 address information. */
    server_ip_addr.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip_addr.nxd_ip_address.v4 = ip_address;

    /* Call the NetX HTTP 'duo' get start service. */
    status = _nxd_http_client_get_start(client_ptr, &server_ip_addr, resource, input_ptr, input_size, 
                                        username, password, wait_option);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(input_ptr);
    NX_PARAMETER_NOT_USED(input_size);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_get_start_extended                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is a wrapper for the actual _nxd_http_client_get_start*/ 
/*    service that enables HTTP applications running on IPv4 to access    */
/*    HTTP duo API without enabling IPv6.                                 */
/*                                                                        */
/*    Note: The strings of resource, username and password must be        */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length to resource (URL)      */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nxd_http_client_get_start_extended   Actual HTTP Client duo get    */
/*                                               start service            */
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
UINT  _nx_http_client_get_start_extended(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, UINT resource_length, 
                                         CHAR *input_ptr, UINT input_size, CHAR *username, UINT username_length,
                                         CHAR *password, UINT password_length, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT            status;
NXD_ADDRESS     server_ip_addr;

    /* Construct an IP address structure, and fill in IPv4 address information. */
    server_ip_addr.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip_addr.nxd_ip_address.v4 = ip_address;

    /* Call the NetX HTTP 'duo' get start service. */
    status = _nxd_http_client_get_start_extended(client_ptr, &server_ip_addr, resource, resource_length, 
                                                 input_ptr, input_size, username, username_length, 
                                                 password, password_length, wait_option);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(resource_length);
    NX_PARAMETER_NOT_USED(input_ptr);
    NX_PARAMETER_NOT_USED(input_size);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(username_length);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(password_length);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_http_client_get_start                          PORTABLE C     */ 
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
/*    ip_address                            IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nx_http_client_get_start             Actual client get start call  */ 
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
UINT  _nxde_http_client_get_start(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *ip_address, CHAR *resource, 
                                  CHAR *input_ptr, UINT input_size, CHAR *username, CHAR *password, 
                                  ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !ip_address)
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nxd_http_client_get_start(client_ptr, ip_address, resource, input_ptr, input_size, 
                                         username, password, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_http_client_get_start_extended                 PORTABLE C     */ 
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
/*    ip_address                            IP duo address of HTTP Server */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nxd_http_client_get_start_extended   Actual client get start call  */ 
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
UINT  _nxde_http_client_get_start_extended(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *ip_address, CHAR *resource, UINT resource_length,
                                           CHAR *input_ptr, UINT input_size, CHAR *username, UINT username_length,
                                           CHAR *password, UINT password_length, ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        (resource == NX_NULL) || !ip_address)
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET start routine.  */
    status =  _nxd_http_client_get_start_extended(client_ptr, ip_address, resource, resource_length, 
                                                  input_ptr, input_size, username, username_length,
                                                  password, password_length, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_http_client_get_start                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the application GET request.  The specified */ 
/*    resource (URL) is requested from the HTTP Server at the specified   */ 
/*    IP address. If input was specified, the request will actually be    */ 
/*    sent as a POST instead of a GET.                                    */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nxd_http_client_get_start_extended   Actual client get start call  */ 
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
UINT  _nxd_http_client_get_start(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, CHAR *input_ptr, 
                                 UINT input_size, CHAR *username, CHAR *password, ULONG wait_option)
{

UINT resource_length = 0;
UINT username_length = 0;
UINT password_length = 0;
UINT status;

    /* Make sure there is enough room in the destination string.  */
    if((username) && (password))
    {
        if (_nx_utility_string_length_check(username, &username_length, NX_HTTP_MAX_NAME) ||
            _nx_utility_string_length_check(password, &password_length, NX_HTTP_MAX_PASSWORD))
        {

            /* Error, return to caller.  */
            return(NX_HTTP_PASSWORD_TOO_LONG);
        }
    }

    /* Check resource length.  */
    if (_nx_utility_string_length_check(resource, &resource_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_HTTP_ERROR);
    }

    status = _nxd_http_client_get_start_extended(client_ptr, server_ip, resource, resource_length, 
                                                 input_ptr, input_size, username, username_length,
                                                 password, password_length, wait_option);

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_http_client_get_start_extended                 PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the application GET request.  The specified */ 
/*    resource (URL) is requested from the HTTP Server at the specified   */ 
/*    IP address. If input was specified, the request will actually be    */ 
/*    sent as a POST instead of a GET.                                    */ 
/*                                                                        */
/*    Note: The strings of resource, username and password must be        */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             HTTP Server IP address        */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */ 
/*    input_ptr                             Additional input pointer      */ 
/*    input_size                            Additional input size         */ 
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
/*    _nx_http_client_base64_encode         Encode username/password      */ 
/*    _nx_http_client_calculate_content_offset Calculate content offset   */ 
/*    _nx_http_client_content_length_get    Get content length            */ 
/*    _nx_http_client_number_convert        Convert number to ASCII       */ 
/*    nx_packet_allocate                    Allocate a packet             */ 
/*    nx_packet_data_append                 Append data to packet         */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_client_socket_bind             Bind client socket to port    */ 
/*    nxd_tcp_client_socket_connect         Connect to HTTP Server        */ 
/*    nx_tcp_socket_disconnect              Disconnect client socket      */ 
/*    nx_tcp_client_socket_unbind           Unbind client socket          */ 
/*    nx_tcp_socket_receive                 Get response packet           */ 
/*    nx_tcp_socket_send                    Send request to Server        */ 
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            correted the status when    */
/*                                            received error code,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nxd_http_client_get_start_extended(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, UINT resource_length,
                                          CHAR *input_ptr, UINT input_size, CHAR *username, UINT username_length,
                                          CHAR *password, UINT password_length, ULONG wait_option)
{

UINT        status;
NX_PACKET   *packet_ptr;
UINT        length = 0, offset = 0;
NX_PACKET   *response_packet_ptr;
CHAR        string1[NX_HTTP_MAX_NAME + NX_HTTP_MAX_PASSWORD + 2];
CHAR        string2[NX_HTTP_MAX_STRING + 1];
CHAR        *buffer_ptr;
CHAR        crlf[2] = {13,10};
UINT        string1_length;
UINT        string2_length;
UINT        temp_resource_length = 0;
UINT        temp_username_length = 0;
UINT        temp_password_length = 0;


    /* Determine if the client is in a ready state.  */
    if (client_ptr -> nx_http_client_state != NX_HTTP_CLIENT_STATE_READY)
    {

        /* Client not ready, return error.  */
        return(NX_HTTP_NOT_READY);
    }

    /* Make sure there is enough room in the destination string.  */
    if((username) && (password))
    {
        if (_nx_utility_string_length_check(username, &temp_username_length, username_length) ||
            _nx_utility_string_length_check(password, &temp_password_length, password_length))
        {

            /* Error, return to caller.  */
            return(NX_HTTP_PASSWORD_TOO_LONG);
        }

        /* Validate string length. */
        if ((username_length != temp_username_length) ||
            (password_length != temp_password_length))
        {
            return(NX_HTTP_ERROR);
        }
    }

    /* Check resource length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, resource_length))
    {
        return(NX_HTTP_ERROR);
    }

    /* Validate string length. */
    if (resource_length != temp_resource_length)
    {
        return(NX_HTTP_ERROR);
    }

    /* Otherwise, attempt to bind the client socket.  */
    status =  nx_tcp_client_socket_bind(&(client_ptr -> nx_http_client_socket), NX_ANY_PORT, wait_option);

    /* Check status of the bind.  */
    if (status != NX_SUCCESS)
    {

        /* Error binding to a port, return to caller.  */
        return(status);
    }

    /* Invoke the connection call. */
    status = nxd_tcp_client_socket_connect(&(client_ptr -> nx_http_client_socket), server_ip, 
                                             client_ptr -> nx_http_client_connect_port, wait_option);

    /* Check for connection status.  */
    if (status != NX_SUCCESS)
    {

        /* Error, unbind the port and return an error.  */
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
        return(status);
    }

    /* At this point we have a connection setup with an HTTP server!  */

    /* Allocate a packet for the GET (or POST) message.  */
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(client_ptr -> nx_http_client_packet_pool_ptr, &packet_ptr, 
                                     NX_IPv4_TCP_PACKET, wait_option);
    }
    else
    {
    
        status =  nx_packet_allocate(client_ptr -> nx_http_client_packet_pool_ptr, &packet_ptr, 
                                    NX_IPv6_TCP_PACKET, wait_option);
    }

    /* Check allocation status.  */
    if (status != NX_SUCCESS)
    {

        /* Error, unbind the port and return an error.  */
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
        return(status);
    }

    /* Determine if a GET or POST is requested.  */
    if ((input_ptr) && (input_size))
    {

        /* Additional information requested, build the POST request.  */
        nx_packet_data_append(packet_ptr, "POST ", 5, client_ptr -> nx_http_client_packet_pool_ptr, 
                              NX_WAIT_FOREVER);
    }
    else
    {

        /* No additional information, build the GET request.  */
        nx_packet_data_append(packet_ptr, "GET ", 4, client_ptr -> nx_http_client_packet_pool_ptr, 
                              NX_WAIT_FOREVER);
    }

    /* Determine if the resource needs a leading "/".  */
    if (resource[0] != '/')
    {

        /* Is this another website e.g. begins with http and has a colon? */
        if  (
             ((resource[0] == 'h') || (resource[0] == 'H')) &&
             ((resource[1] == 't') || (resource[1] == 'T')) &&
             ((resource[2] == 't') || (resource[2] == 'T')) &&
             ((resource[3] == 'p') || (resource[3] == 'P'))  &&
             (resource[4] == ':')
            )
        {
          
            /* Yes, to send this string as is. */
        }
        else
        { 
          
            /* Local file URI which needs a leading '/' character.  */
            nx_packet_data_append(packet_ptr, "/", 1, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        }
    }
    /* Else URI string refers to root directory file, and has a leading '/' character already. */

    /* Place the resource in the header.  */
    nx_packet_data_append(packet_ptr, resource, resource_length, client_ptr -> nx_http_client_packet_pool_ptr,
                          NX_WAIT_FOREVER);

    /* Place the HTTP version in the header.  */
    nx_packet_data_append(packet_ptr, " HTTP/1.0", 9, client_ptr -> nx_http_client_packet_pool_ptr, 
                          NX_WAIT_FOREVER);

    /* Place the end of line character in the header.  */
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, 
                          NX_WAIT_FOREVER);

    /* Determine if basic authentication is required.  */
    if ((username) && (password))
    {

        /* Yes, attempt to build basic authentication.  */
        nx_packet_data_append(packet_ptr, "Authorization: Basic ", 21, client_ptr -> nx_http_client_packet_pool_ptr, 
                              NX_WAIT_FOREVER);

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
        nx_packet_data_append(packet_ptr, string2, string2_length, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    }
        
    /* Check to see if a Content-Length field is needed.  */
    if ((input_ptr) && (input_size))
    {

        /* Now build the content-length entry.  */
        nx_packet_data_append(packet_ptr, "Content-Length: ", 16, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        string1_length =  _nx_http_client_number_convert(input_size, string1);
        nx_packet_data_append(packet_ptr, string1, string1_length, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    }

    /* Place an extra cr/lf to signal the end of the HTTP header.  */
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Check to see if we need to append the additional user data.  */
    if ((input_ptr) && (input_size))
    {

        /* Now build the content-length entry.  */
        nx_packet_data_append(packet_ptr, input_ptr, input_size, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    }

    /* Now send the packet to the HTTP server.  */
    status =  nx_tcp_socket_send(&(client_ptr -> nx_http_client_socket), packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return an error.  */
        return(status);
    }

    /* Pickup the response from the Server.  */
    status =  nx_tcp_socket_receive(&(client_ptr -> nx_http_client_socket), &response_packet_ptr, wait_option);

    /* Check for a response from the Server.  */
    if (status == NX_SUCCESS)
    {

        /* Setup pointer to server response.  */
        buffer_ptr =  (CHAR *) response_packet_ptr -> nx_packet_prepend_ptr;

        /* Check if packet contains the HTTP status. */
        if ((buffer_ptr + 11) >= (CHAR *)response_packet_ptr -> nx_packet_append_ptr)
        {

            /* Release the packet.  */
            nx_packet_release(response_packet_ptr);

            /* Disconnect and unbind the socket.  */
            nx_tcp_socket_disconnect(&(client_ptr->nx_http_client_socket), wait_option);
            nx_tcp_client_socket_unbind(&(client_ptr->nx_http_client_socket));

            /* Return an error.  */
            return(NX_HTTP_FAILED);
        }

        /* Determine if the request was successful.  */
        if (buffer_ptr[9] == '2')
        {

            /* Determine if we need to copy the packet.  */
            if ((response_packet_ptr -> nx_packet_data_end - response_packet_ptr -> nx_packet_data_start) < NX_HTTP_CLIENT_MIN_PACKET_SIZE)
            {

                /* Copy the packet to a packet in the packet pool.  */
                nx_packet_copy(response_packet_ptr, &packet_ptr, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
                
                /* Release the original packet.  */
                nx_packet_release(response_packet_ptr);

                /* Copy the packet pointer.  */
                response_packet_ptr =  packet_ptr;
            }

            /* Pickup the content length.  */
            length =  _nx_http_client_content_length_get(response_packet_ptr);

            /* Pickup the content offset.  */
            offset =  _nx_http_client_calculate_content_offset(response_packet_ptr);

            /* Indicate a successful completion.  */
            status =  NX_SUCCESS;
        }
        /* Determine if it is an authentication error.  */
        else if ((buffer_ptr[9] == '4') && (buffer_ptr[10] == '0') && (buffer_ptr[11] == '1'))
        {

            /* Release the packet.  */
            nx_packet_release(response_packet_ptr);

            /* Inform caller of an authentication error.  */
            status =  NX_HTTP_AUTHENTICATION_ERROR;
        }
        else
        {

            /* Release the packet.  */
            nx_packet_release(response_packet_ptr);

            /* Received error code.  */
            status = NX_HTTP_REQUEST_UNSUCCESSFUL_CODE;
        }
    }

    /* Check for error processing received packet. */
    if (status != NX_SUCCESS)
    {

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        return status;
    }

    /* Check for invalid packet parameters.  */
    if ((length == 0) || (offset == 0))
    {

        /* Release the packet.  */
        nx_packet_release(response_packet_ptr);

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return an error.  */
        return(NX_HTTP_FAILED);
    }


    /* Determine if the offset is in this request.  */
    if (response_packet_ptr -> nx_packet_length > (offset + 3))
    {

        /* Adjust the pointers to skip over the response header.  */
        response_packet_ptr -> nx_packet_prepend_ptr =  response_packet_ptr -> nx_packet_prepend_ptr + offset;

        /* Reduce the length.  */
        response_packet_ptr -> nx_packet_length =  response_packet_ptr -> nx_packet_length - offset;

        /* Save the packet pointer for the get packet call.  */
        client_ptr -> nx_http_client_first_packet =  response_packet_ptr;
    }
    else
    {
    
        /* Clear the saved packet pointer.  */
        client_ptr -> nx_http_client_first_packet =  NX_NULL;

        /* This packet only contains the header, just release it!  */
        nx_packet_release(response_packet_ptr);
    }

    /* Store the total number of bytes to receive.  */
    client_ptr -> nx_http_client_total_transfer_bytes =     length;
    client_ptr -> nx_http_client_actual_bytes_transferred =  0;

    /* Enter the GET state.  */
    client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_GET;

    /* Return success to the caller.  */
    return  NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_get_packet                         PORTABLE C      */ 
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
/*    _nx_http_client_get_packet            Actual client get packet call */ 
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
UINT  _nxe_http_client_get_packet(NX_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual GET packet routine.  */
    status =  _nx_http_client_get_packet(client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_get_packet                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets a data packet associated with the resource       */ 
/*    specified by the previous GET start request.                        */ 
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
/*    nx_tcp_client_socket_unbind           Unbind client socket          */ 
/*    nx_tcp_socket_disconnect              Disconnect client socket      */ 
/*    nx_tcp_socket_receive                 Receive a resource data packet*/ 
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
UINT  _nx_http_client_get_packet(NX_HTTP_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

NX_PACKET   *data_packet_ptr;
UINT        status;


    /* Default the return packet to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Determine if the client is in a get state.  */
    if (client_ptr -> nx_http_client_state != NX_HTTP_CLIENT_STATE_GET)
    {

        /* Client not ready, return error.  */
        return(NX_HTTP_NOT_READY);
    }

    /* Determine if the GET packet operation is complete.  */
    if (client_ptr -> nx_http_client_total_transfer_bytes == client_ptr -> nx_http_client_actual_bytes_transferred)
    {

        /* Yes, we are finished.  */

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Reenter the READY state.  */
        client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

        /* Return the GET done code.  */
        return(NX_HTTP_GET_DONE);
    }

    /* Determine if there is a queued packet.  */
    if (client_ptr -> nx_http_client_first_packet)
    {

        /* Yes, just use the saved packet.  */
        data_packet_ptr =  client_ptr -> nx_http_client_first_packet;

        /* Clear the saved packet pointer.  */
        client_ptr -> nx_http_client_first_packet =  NX_NULL;
    }
    else
    {

        /* Receive a data packet from the TCP connection.  */
        status =  nx_tcp_socket_receive(&(client_ptr -> nx_http_client_socket), &data_packet_ptr, wait_option);

        /* Determine if a packet is available.  */
        if (status != NX_SUCCESS)
        {

            /* Ensure the packet pointer is NULL.  */
            data_packet_ptr =  NX_NULL;
            /* Disconnect and unbind the socket.  */
            nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
            nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
            return status;
        }
    }

    /* Check for an error condition.  */
    if (data_packet_ptr -> nx_packet_length > (client_ptr -> nx_http_client_total_transfer_bytes - client_ptr -> nx_http_client_actual_bytes_transferred))
    {

        /* Release the invalid HTTP packet. */
        nx_packet_release(data_packet_ptr);

        /* Error, break down the connection and return to the caller.  */

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Reenter the READY state.  */
        client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

        /* Return an error.  */
        return(NX_HTTP_BAD_PACKET_LENGTH);
    }

    /* Adjust the actual transfer bytes.  */
    client_ptr -> nx_http_client_actual_bytes_transferred =  client_ptr -> nx_http_client_actual_bytes_transferred +
                                                            data_packet_ptr -> nx_packet_length;

    /* Move the packet pointer into the return pointer.  */
    *packet_ptr =  data_packet_ptr;

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_put_start                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP (IPv4) Client put start */
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
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
/*    _nx_http_client_put_start             Actual IPv4 HTTP Client put   */    
/*                                                 start call             */ 
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

UINT  _nxe_http_client_put_start(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, 
                                   CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        resource == NX_NULL || !ip_address)
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_http_client_put_start(client_ptr, ip_address, resource, username, password, 
                                        total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(total_bytes);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_put_start_extended                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the HTTP (IPv4) Client put start */
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */ 
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
/*    _nx_http_client_put_start_extended    Actual IPv4 HTTP Client put   */ 
/*                                                 start call             */ 
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

UINT  _nxe_http_client_put_start_extended(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, 
                                          UINT resource_length, CHAR *username, UINT username_length, 
                                          CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        resource == NX_NULL || !ip_address)
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nx_http_client_put_start_extended(client_ptr, ip_address, resource, resource_length, 
                                                 username, username_length, password, password_length,
                                                 total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(resource_length);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(username_length);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(password_length);
    NX_PARAMETER_NOT_USED(total_bytes);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_put_start                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is a wrapper for the actual _nxd_http_client_put_start*/ 
/*    service that enables HTTP applications running on IPv4 to access    */
/*    HTTP duo API without enabling IPv6.                                 */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
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
/*    _nxd_http_client_put_start            Actual put start service for  */
/*                                               HTTP Duo Client          */
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
UINT  _nx_http_client_put_start(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, CHAR *username, 
                                CHAR *password, ULONG total_bytes, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT            status;
NXD_ADDRESS     server_ip_addr;


    /* Construct an IP address structure, and fill in IPv4 address information. */
    server_ip_addr.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip_addr.nxd_ip_address.v4 = ip_address;

    status = _nxd_http_client_put_start(client_ptr, &server_ip_addr, resource, username, 
                                       password, total_bytes, wait_option);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(total_bytes);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_put_start_extended                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is a wrapper for the actual _nxd_http_client_put_start*/ 
/*    service that enables HTTP applications running on IPv4 to access    */
/*    HTTP duo API without enabling IPv6.                                 */
/*                                                                        */
/*    Note: The strings of resource, username and password must be        */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    ip_address                            IPv4 address of HTTP Server   */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */ 
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
/*    _nxd_http_client_put_start_extended   Actual put start service for  */
/*                                               HTTP Duo Client          */
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
UINT  _nx_http_client_put_start_extended(NX_HTTP_CLIENT *client_ptr, ULONG ip_address, CHAR *resource, 
                                         UINT resource_length, CHAR *username, UINT username_length, 
                                         CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT            status;
NXD_ADDRESS     server_ip_addr;


    /* Construct an IP address structure, and fill in IPv4 address information. */
    server_ip_addr.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip_addr.nxd_ip_address.v4 = ip_address;

    status = _nxd_http_client_put_start_extended(client_ptr, &server_ip_addr, resource, resource_length, 
                                                 username, username_length, password, password_length, 
                                                 total_bytes, wait_option);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(resource_length);
    NX_PARAMETER_NOT_USED(username);
    NX_PARAMETER_NOT_USED(username_length);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(password_length);
    NX_PARAMETER_NOT_USED(total_bytes);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_http_client_put_start                         PORTABLE C      */ 
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
/*    _nx_http_client_put_start             Actual client put start call  */ 
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
UINT  _nxde_http_client_put_start(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, 
                                   CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        resource == NX_NULL || !server_ip)
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nxd_http_client_put_start(client_ptr, server_ip, resource, username, password, 
                                         total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_http_client_put_start_extended                PORTABLE C      */ 
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
/*    resource_length                       Length of resource (URL)      */ 
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
/*    _nxd_http_client_put_start_extended   Actual client put start call  */ 
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
UINT  _nxde_http_client_put_start_extended(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, 
                                           UINT resource_length, CHAR *username, UINT username_length, 
                                           CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option)
{

UINT    status; 


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        resource == NX_NULL || !server_ip)
        return(NX_PTR_ERROR);

    /* Check for invalid total bytes.  */
    if (total_bytes == 0)
        return(NX_SIZE_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PUT start routine.  */
    status =  _nxd_http_client_put_start_extended(client_ptr, server_ip, resource, resource_length, 
                                                  username, username_length, password, password_length,
                                                  total_bytes, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_http_client_put_start                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processesthe application PUT request. Transferring the*/
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
/*    _nxd_http_client_put_start_extended   Actual client put start call  */ 
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
UINT  _nxd_http_client_put_start(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, 
                                 CHAR *username, CHAR *password, ULONG total_bytes, ULONG wait_option)

{

UINT resource_length = 0;
UINT username_length = 0;
UINT password_length = 0;
UINT status;

    /* Make sure there is enough room in the destination string.  */
    if((username) && (password))
    {
        if (_nx_utility_string_length_check(username, &username_length, NX_HTTP_MAX_NAME) ||
            _nx_utility_string_length_check(password, &password_length, NX_HTTP_MAX_PASSWORD))
        {

            /* Error, return to caller.  */
            return(NX_HTTP_PASSWORD_TOO_LONG);
        }
    }

    /* Check resource length.  */
    if (_nx_utility_string_length_check(resource, &resource_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_HTTP_ERROR);
    }

    status = _nxd_http_client_put_start_extended(client_ptr, server_ip, resource, resource_length, 
                                                 username, username_length, password, password_length, 
                                                 total_bytes, wait_option);

    /* Return status to the caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_http_client_put_start_extended                 PORTABLE C      */ 
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processesthe application PUT request. Transferring the*/
/*    specified resource (URL) is started by this routine. The            */ 
/*    application must call put packet one or more times to transfer      */ 
/*    the resource contents.                                              */ 
/*                                                                        */
/*    Note: The strings of resource, username and password must be        */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list.                                     */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                            Pointer to HTTP client        */ 
/*    server_ip                             IP address of HTTP Server     */ 
/*    resource                              Pointer to resource (URL)     */ 
/*    resource_length                       Length of resource (URL)      */ 
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
/*    _nx_http_client_base64_encode         Encode username/password      */ 
/*    _nx_http_client_number_convert        Convert number to ASCII       */ 
/*    nx_http_client_type_get               Get the HTTP file type        */ 
/*    nx_packet_allocate                    Allocate a packet             */ 
/*    nx_packet_data_append                 Append data to packet         */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_client_socket_bind             Bind client socket to port    */ 
/*    nxd_tcp_client_socket_connect         Connect to HTTP  Server       */ 
/*    nx_tcp_socket_disconnect              Disconnect client socket      */ 
/*    nx_tcp_client_socket_unbind           Unbind client socket          */ 
/*    nx_tcp_socket_send                    Send request to Server        */ 
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
UINT  _nxd_http_client_put_start_extended(NX_HTTP_CLIENT *client_ptr, NXD_ADDRESS *server_ip, CHAR *resource, 
                                          UINT resource_length, CHAR *username, UINT username_length,
                                          CHAR *password, UINT password_length, ULONG total_bytes, ULONG wait_option)

{

NX_PACKET   *packet_ptr;
CHAR        string1[NX_HTTP_MAX_NAME + NX_HTTP_MAX_PASSWORD + 2];
CHAR        string2[NX_HTTP_MAX_STRING + 1];
CHAR        crlf[2] = {13,10};
UINT        status;
UINT        string1_length;
UINT        string2_length;
UINT        temp_resource_length = 0;
UINT        temp_username_length = 0;
UINT        temp_password_length = 0;



    /* Determine if the client is in a ready state.  */
    if (client_ptr -> nx_http_client_state != NX_HTTP_CLIENT_STATE_READY)
    {

        /* Client not ready, return error.  */
        return(NX_HTTP_NOT_READY);
    }

    if ((username) && (password))
    {

        /* Make sure there is enough room in the destination string.  */
        if (_nx_utility_string_length_check(username, &temp_username_length, username_length) ||
            _nx_utility_string_length_check(password, &temp_password_length, password_length))
        {

            /* Error, return to caller.  */
            return(NX_HTTP_USERNAME_TOO_LONG);
        }

        /* Validate string length. */
        if ((username_length != temp_username_length) ||
            (password_length != temp_password_length))
        {
            return(NX_HTTP_ERROR);
        }
    }

    /* Check resource length.  */
    if (_nx_utility_string_length_check(resource, &temp_resource_length, resource_length))
    {
        return(NX_HTTP_ERROR);
    }

    /* Validate string length. */
    if (resource_length != temp_resource_length)
    {
        return(NX_HTTP_ERROR);
    }

    /* Otherwise, attempt to bind the client socket.  */
    status =  nx_tcp_client_socket_bind(&(client_ptr -> nx_http_client_socket), NX_ANY_PORT, wait_option);

    /* Check status of the bind.  */
    if (status != NX_SUCCESS)
    {

        /* Error binding to a port, return to caller.  */
        return(status);
    }

    /* Connect to the HTTP server.  */

    /* Invoke the 'Duo' (supports IPv6/IPv4) connection call. */
    status = nxd_tcp_client_socket_connect(&(client_ptr -> nx_http_client_socket), server_ip,
                                             client_ptr -> nx_http_client_connect_port, wait_option);

    /* Check for connection status.  */
    if (status != NX_SUCCESS)
    {

        /* Error, unbind the port and return an error.  */
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
        return(status);
    }

    /* At this point we have a connection setup with an HTTP server!  */

    /* Allocate a packet for the PUT message.  */
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  nx_packet_allocate(client_ptr -> nx_http_client_packet_pool_ptr, &packet_ptr, 
                                     NX_IPv4_TCP_PACKET, wait_option);
    }
    else
    {
    
        status =  nx_packet_allocate(client_ptr -> nx_http_client_packet_pool_ptr, &packet_ptr, 
                                    NX_IPv6_TCP_PACKET, wait_option);
    }

    /* Check allocation status.  */
    if (status != NX_SUCCESS)
    {

        /* Error, unbind the port and return an error.  */
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));
        return(status);
    }

    /* Build the PUT request.  */
    nx_packet_data_append(packet_ptr, "PUT ", 4, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Determine if this is a root directory based URI string. */
    if (resource[0] != '/')
    {

        /* Then check if this another website e.g. begins with http and has a colon. */
        if  (
             ((resource[0] == 'h') || (resource[0] == 'H')) &&
             ((resource[1] == 't') || (resource[1] == 'T')) &&
             ((resource[2] == 't') || (resource[2] == 'T')) &&
             ((resource[3] == 'p') || (resource[3] == 'P'))  &&
             (resource[4] == ':')
            )
        {
          
            /* Yes, ok to send this string as is. */
        }
        else
        { 
          
            /* This URI is a root directory based file but it needs a leading / character.  */
            nx_packet_data_append(packet_ptr, "/", 1, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        }
    }

    /* (Else the URI begins with a '/' and is ok to send as is.) */
    
    /* Place the resource in the header.  */
    nx_packet_data_append(packet_ptr, resource, resource_length,
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Place the HTTP version in the header.  */
    nx_packet_data_append(packet_ptr, " HTTP/1.0", 9, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Place the end of line character in the header.  */
    nx_packet_data_append(packet_ptr, crlf, 2, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Determine if basic authentication is required.  */
    if ((username) && (password))
    {

        /* Yes, attempt to build basic authentication.  */
        nx_packet_data_append(packet_ptr, "Authorization: Basic ", 21, 
                              client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

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
        nx_packet_data_append(packet_ptr, string2, string2_length,
                              client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    }
        
    /* Now build the content-type entry.  */
    nx_packet_data_append(packet_ptr, "Content-Type: ", 14, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    string1_length = _nx_http_client_type_get(resource, string1);
    nx_packet_data_append(packet_ptr, string1, string1_length, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Now build the content-length entry.  */
    nx_packet_data_append(packet_ptr, "Content-Length: ", 16, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    string1_length =  _nx_http_client_number_convert(total_bytes, string1);
    nx_packet_data_append(packet_ptr, string1, string1_length, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);
    nx_packet_data_append(packet_ptr, crlf, 2, client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Place an extra cr/lf to signal the end of the HTTP header.  */
    nx_packet_data_append(packet_ptr, crlf, 2, 
                          client_ptr -> nx_http_client_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Now send the packet to the HTTP server.  */
    status =  nx_tcp_socket_send(&(client_ptr -> nx_http_client_socket), packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return an error.  */
        return(status);
    }

    /* Store the total number of bytes to send.  */
    client_ptr -> nx_http_client_total_transfer_bytes =     total_bytes;
    client_ptr -> nx_http_client_actual_bytes_transferred =  0;

    /* Enter the PUT state.  */
    client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_PUT;

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_http_client_put_packet                         PORTABLE C      */ 
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
/*    _nx_http_client_put_packet            Actual client put packet call */ 
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
UINT  _nxe_http_client_put_packet(NX_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_http_client_id != NXD_HTTP_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Ensure there is enough room for the TCP packet header.  */
    if ((packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < NX_TCP_PACKET)
    {

        /* Return an invalid packet error.  */
        return(NX_INVALID_PACKET);
    }

    /* Call actual PUT data routine.  */
    status =  _nx_http_client_put_packet(client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_put_packet                          PORTABLE C      */ 
/*                                                           6.1          */
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
/*    nx_packet_release                     Release the packet            */ 
/*    nx_tcp_client_socket_unbind           Unbind the client socket      */ 
/*    nx_tcp_socket_disconnect              Disconnect form Server        */ 
/*    nx_tcp_socket_receive                 Receive response from Server  */ 
/*    nx_tcp_socket_send                    Send resource data packet     */ 
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
UINT  _nx_http_client_put_packet(NX_HTTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

NX_PACKET   *response_packet_ptr;
CHAR        *buffer_ptr;
UINT        length;
UINT        status;


    /* First, check and see if the client instance is still in the PUT state.  */
    if (client_ptr -> nx_http_client_state != NX_HTTP_CLIENT_STATE_PUT)
    {

        /* Client not ready, return error.  */
        return(NX_HTTP_NOT_READY);
    }

    /* Next, check and see if there is a response from the Server.  */
    status =  nx_tcp_socket_receive(&(client_ptr -> nx_http_client_socket), &response_packet_ptr, NX_NO_WAIT);

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
            status =  NX_HTTP_AUTHENTICATION_ERROR;
        }
        else
        {

            /* Inform caller of general Server failure.  */
            status =  NX_HTTP_INCOMPLETE_PUT_ERROR;
        }

        /* Release the packet.  */
        nx_packet_release(response_packet_ptr);

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return to the READY state.  */
        client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

        /* Return error to caller.  */
        return(status);
    }

    /* Otherwise, determine if the packet length fits in the available bytes to send.  */
    if (packet_ptr -> nx_packet_length > 
            (client_ptr -> nx_http_client_total_transfer_bytes - client_ptr -> nx_http_client_actual_bytes_transferred))
    {

        /* Request doesn't fit into the remaining transfer window.  */
        return(NX_HTTP_BAD_PACKET_LENGTH);
    }

    /* Remember the packet length.  */
    length =  packet_ptr -> nx_packet_length;

    /* Now send the packet out.  */
    status =  nx_tcp_socket_send(&(client_ptr -> nx_http_client_socket), packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status != NX_SUCCESS)
    {

        /* No, send was not successful.  */
        
        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return to the READY state.  */
        client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

        /* Return an error.  */
        return(status);
    }

    /* Otherwise, update the actual bytes transferred.  */
    client_ptr ->  nx_http_client_actual_bytes_transferred =  client_ptr ->  nx_http_client_actual_bytes_transferred + length;

    /* Are we finished?  */
    if (client_ptr -> nx_http_client_total_transfer_bytes > client_ptr -> nx_http_client_actual_bytes_transferred)
    {

        /* No, we are not finished so just return success to the caller.  */
        return(NX_SUCCESS);        
    }

    /* We are finished sending the PUT data.  Now wait for a response from the Server.  */
    status =  nx_tcp_socket_receive(&(client_ptr -> nx_http_client_socket), &response_packet_ptr, wait_option);

    if (status != NX_SUCCESS)
    {
        /* Disconnect and unbind the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
        nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

        /* Return to the READY state.  */
        client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

        /* Return status to caller.  */
        return(status);
    }


    /* Setup pointer to server response.  */
    buffer_ptr =  (CHAR *) response_packet_ptr -> nx_packet_prepend_ptr;

    /* Determine if the request was successful.  */
    if (((buffer_ptr + 9) >= (CHAR *) response_packet_ptr -> nx_packet_append_ptr) || 
        (buffer_ptr[9] != '2'))
    {

        /* Inform caller of a successful completion.  */
        status =  NX_HTTP_REQUEST_UNSUCCESSFUL_CODE;
    }

    /* Release the packet.  */
    nx_packet_release(response_packet_ptr);

    /* Disconnect and unbind the socket.  */
    nx_tcp_socket_disconnect(&(client_ptr -> nx_http_client_socket), wait_option);
    nx_tcp_client_socket_unbind(&(client_ptr -> nx_http_client_socket));

    /* Return to the READY state.  */
    client_ptr -> nx_http_client_state =  NX_HTTP_CLIENT_STATE_READY;

    /* Return status to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_type_get                            PORTABLE C      */ 
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
/*    _nx_http_client_put_start_extended    Start the PUT process         */ 
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
UINT  _nx_http_client_type_get(CHAR *name, CHAR *http_type_string)
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
/*    _nx_http_client_content_length_get                  PORTABLE C      */ 
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
/*    length                                Length of content             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_http_client_get_start             Start the GET operation       */ 
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
UINT  _nx_http_client_content_length_get(NX_PACKET *packet_ptr)
{

UINT    length;
CHAR   *buffer_ptr;
UINT    found = NX_FALSE;


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
/*    _nx_http_client_calculate_content_offset            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculates the byte offset to the start of the        */ 
/*    HTTP request content area.  This area immediately follows the HTTP  */ 
/*    request header (which ends with a blank line).                      */ 
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
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_http_client_get_start             Start GET processing          */ 
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
UINT  _nx_http_client_calculate_content_offset(NX_PACKET *packet_ptr)
{

UINT    offset;
CHAR    *buffer_ptr;

    
    /* Default the content offset to zero.  */
    offset =  0;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Find the "cr,lf,cr,lf" token.  */
    while (((buffer_ptr+3) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the <cr,lf,cr,lf> token.  This signals a blank line, which also 
           specifies the start of the content.  */
        if ((*buffer_ptr ==      (CHAR) 13) &&
            (*(buffer_ptr+1) ==  (CHAR) 10) &&
            (*(buffer_ptr+2) ==  (CHAR) 13) &&
            (*(buffer_ptr+3) ==  (CHAR) 10))
        {

            /* Adjust the offset.  */
            offset =  offset + 4;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;

        /* Increment the offset.  */
        offset++;
    }

    /* Return the offset to the caller.  */
    return(offset);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_http_client_number_convert                      PORTABLE C      */ 
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
/*    _nx_http_client_get_start_extended    Start GET processing          */ 
/*    _nx_http_client_put_start_extended    Start PUT processing          */ 
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
UINT  _nx_http_client_number_convert(UINT number, CHAR *string)
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
