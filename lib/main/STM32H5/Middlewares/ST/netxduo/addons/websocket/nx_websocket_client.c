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
/** NetX WebSocket Component                                              */
/**                                                                       */
/**   WebSocket Protocol                                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_WEBSOCKET_CLIENT_SOURCE_CODE

/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include "tx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"
#include "nx_tcp.h"
#include "nx_websocket_client.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

#define NX_WEBSOCKET_CRLF                           "\r\n"
#define NX_WEBSOCKET_CRLF_SIZE                      2

#define NX_WEBSOCKET_HEADER_MINIMUM_LENGTH          2

#define NX_WEBSOCKET_ACCEPT_PREDEFINED_GUID         "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define NX_WEBSOCKET_ACCEPT_PREDEFINED_GUID_SIZE    (sizeof(NX_WEBSOCKET_ACCEPT_PREDEFINED_GUID) - 1)
#define NX_WEBSOCKET_ACCEPT_DIGEST_SIZE             20 /* The length of SHA-1 hash is 20 bytes */
#define NX_WEBSOCKET_ACCEPT_KEY_SIZE                28 /* The base64 encode key for 20 bytes digest requires (27 bytes name size + 1 byte pad) = 28 bytes */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_create                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket instance create    */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    client_name                           Name of this WebSocket        */
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to packet pool        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_create           Actual websocket create call  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_create(NX_WEBSOCKET_CLIENT *client_ptr, UCHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr)
{

UINT        status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id == NX_WEBSOCKET_CLIENT_ID) || 
        (pool_ptr == NX_NULL) || (pool_ptr -> nx_packet_pool_id != NX_PACKET_POOL_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual client create function.  */
    status = _nx_websocket_client_create(client_ptr, client_name, ip_ptr, pool_ptr);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_create                         PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an instance for WebSocket Client.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    client_name                           Name of this WebSocket        */
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to packet pool        */
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
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_create(NX_WEBSOCKET_CLIENT *client_ptr, UCHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr)
{

UINT status;

    /* Clear the WebSocket structure.  */
    memset((void *)client_ptr, 0, sizeof(NX_WEBSOCKET_CLIENT));

    /* Create WebSocket mutex.  */
    status = tx_mutex_create(&client_ptr -> nx_websocket_client_mutex, (CHAR *)client_name, TX_NO_INHERIT);
    if (status)
    {
        return(status);
    }

    /* Save the Client name.  */
    client_ptr -> nx_websocket_client_name = client_name;

    /* Save the IP pointer address.  */
    client_ptr -> nx_websocket_client_ip_ptr = ip_ptr;

    /* Save the packet pool pointer.  */
    client_ptr -> nx_websocket_client_packet_pool_ptr = pool_ptr;

    /* Set the Client ID to indicate the WebSocket client thread is ready.  */
    client_ptr -> nx_websocket_client_id = NX_WEBSOCKET_CLIENT_ID;

    /* Update the state.  */
    client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_delete                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket instance delete    */
/*    function call.                                                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_delete           Actual websocket delete call  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_delete(NX_WEBSOCKET_CLIENT *client_ptr)
{

UINT        status;

    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual client delete function.  */
    status = _nx_websocket_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_delete                         PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an instance for WebSocket Client.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    _nx_websocket_client_cleanup          Cleanup unused resources      */
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
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_delete(NX_WEBSOCKET_CLIENT *client_ptr)
{

    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Clean up unused resources.  */
    _nx_websocket_client_cleanup(client_ptr);

    /* Set the Client ID to a default value.  */
    client_ptr -> nx_websocket_client_id = 0;

    /* Update the state.  */
    client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_INITIALIZE;

    /* Release the mutex, delete the mutex and assign the pointer to NULL. */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
    tx_mutex_delete(&(client_ptr -> nx_websocket_client_mutex));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_connect                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket connect.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    socket_ptr                            Pointer to TCP socket         */
/*    host                                  Pointer to host               */ 
/*    host_length                           Length of host                */
/*    uri_path                              Pointer to uri path           */ 
/*    uri_path_length                       Length of uri path            */
/*    protocol                              Pointer to protocol           */ 
/*    protocol_length                       Length of protocol            */
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_connect          Actual websocket connect call */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_TCP_SOCKET *socket_ptr,
                                    UCHAR *host, UINT host_length,
                                    UCHAR *uri_path, UINT uri_path_length,
                                    UCHAR *protocol, UINT protocol_length,UINT wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID) || 
        (socket_ptr == NX_NULL) || (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID) ||
        (host == NX_NULL) || (host_length == 0) || 
        (uri_path == NX_NULL) || (uri_path_length == 0) ||
        (protocol == NX_NULL) || (protocol_length == 0))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual connect function.  */
    status = _nx_websocket_client_connect(client_ptr, socket_ptr, host, host_length, uri_path, uri_path_length, protocol, protocol_length, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_connect                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes a WebSocket connection over TCP socket to the   */
/*    server.                                                             */
/*    Note: Application must establish a TCP connection before.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    socket_ptr                            Pointer to TCP socket         */
/*    host                                  Pointer to host               */ 
/*    host_length                           Length of host                */
/*    uri_path                              Pointer to uri path           */ 
/*    uri_path_length                       Length of uri path            */
/*    protocol                              Pointer to protocol           */ 
/*    protocol_length                       Length of protocol            */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_TCP_SOCKET *socket_ptr,
                                   UCHAR *host, UINT host_length,
                                   UCHAR *resource, UINT resource_length,
                                   UCHAR *protocol, UINT protocol_length,UINT wait_option)
{

UINT status;


    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Check the state.  */
    if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTED)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_ALREADY_CONNECTED);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTING)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_CONNECTING);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_INITIALIZE)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_INVALID_STATE);
    }

    /* Save the socket pointer.  */
    client_ptr -> nx_websocket_client_socket_ptr = socket_ptr;

#ifdef NX_SECURE_ENABLE
    client_ptr -> nx_websocket_client_use_tls = NX_FALSE;
#endif /* NX_SECURE_ENABLE */

    status = _nx_websocket_client_connect_internal(client_ptr, host, host_length, resource, resource_length, protocol, protocol_length, wait_option);

    /* Release the mutex and return */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_connect_internal               PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes a WebSocket connection over TCP socket or TLS   */
/*    session to the server.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    host                                  Pointer to host               */ 
/*    host_length                           Length of host                */
/*    uri_path                              Pointer to uri path           */ 
/*    uri_path_length                       Length of uri path            */
/*    protocol                              Pointer to protocol           */ 
/*    protocol_length                       Length of protocol            */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_allocate                    Allocate a packet             */
/*    nx_packet_release                     Release the packet            */
/*    nx_secure_tls_packet_allocate         Allocate a TLS packet         */
/*    nx_packet_data_append                 Append data                   */
/*    _nx_utility_base64_encode             Base64 encode                 */
/*    _nx_websocket_client_packet_send      Send out websocket packet     */
/*    _nx_websocket_client_packet_receive   Receive a websocket packet    */
/*    _nx_websocket_client_connect_response_check                         */
/*                                          Process connect response      */
/*    _nx_websocket_client_cleanup          Cleanup resources             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_connect          Make websocket connection     */
/*    _nx_websocket_client_secure_connect   Make secure websocket         */
/*                                            connection                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_connect_internal(NX_WEBSOCKET_CLIENT *client_ptr,
                                            UCHAR *host, UINT host_length,
                                            UCHAR *uri_path, UINT uri_path_length,
                                            UCHAR *protocol, UINT protocol_length,UINT wait_option)
{

UINT i;
UINT status;
NX_PACKET *packet_ptr;


    /* To guarantee resources are cleaned-up if a re-connect happens. */
    _nx_websocket_client_cleanup(client_ptr);

    /* Generate GUID for WebSocket Key.  */
    for (i = 0; i < NX_WEBSOCKET_CLIENT_GUID_SIZE; i ++)
    {
        client_ptr -> nx_websocket_client_guid[i] = (UCHAR)(NX_RAND());
    }

    /* Encode the GUID as key.  */
    _nx_utility_base64_encode(client_ptr -> nx_websocket_client_guid, NX_WEBSOCKET_CLIENT_GUID_SIZE, 
                              client_ptr -> nx_websocket_client_key, NX_WEBSOCKET_CLIENT_KEY_SIZE,
                              &client_ptr -> nx_websocket_client_key_size);

    /* Release the mutex */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

    /* Allocate a packet.  */
#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nx_websocket_client_use_tls)
    {

        /* Use TLS packet allocate.  The TLS packet allocate is able to count for 
           TLS-related header space including crypto initial vector area. */
        status = nx_secure_tls_packet_allocate(client_ptr -> nx_websocket_client_tls_session_ptr,
                                               client_ptr -> nx_websocket_client_packet_pool_ptr,
                                               &packet_ptr, TX_WAIT_FOREVER);
    }
    else
    {
#endif /* NX_SECURE_ENABLE */

        /* Allocate packet.  */
        if (client_ptr -> nx_websocket_client_socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            status =  nx_packet_allocate(client_ptr -> nx_websocket_client_packet_pool_ptr,
                                         &packet_ptr,
                                         NX_IPv4_TCP_PACKET, wait_option);
        }
        else
        {
            status =  nx_packet_allocate(client_ptr -> nx_websocket_client_packet_pool_ptr,
                                         &packet_ptr,
                                         NX_IPv6_TCP_PACKET, wait_option);
        }
#ifdef NX_SECURE_ENABLE
    }
#endif /* NX_SECURE_ENABLE */

    /* Check status. */
    if (status)
    {

        /* Obtain the mutex again and return error status. */
        tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);
        return(status);
    }

    /* WebSocket opening handshake message format:
       GET /mqtt HTTP/1.1\r\n
       Host: example.com\r\n
       Upgrade: websocket\r\n
       connection: Upgrade\r\n
       Sec-WebSocket-Key: xxxxxxx=\r\n
       Sec-WebSocket-Protocol: mqtt\r\n
       Sec-WebSocket-Version: 13\r\n
       \r\n
    */

    /* Build the GET request: Get + Request URI + HTTP version.  */
    status =  nx_packet_data_append(packet_ptr, "GET ", sizeof("GET ") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, uri_path, uri_path_length, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, " HTTP/1.1", sizeof(" HTTP/1.1") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the Host in the header.  */
    status += nx_packet_data_append(packet_ptr, "Host: ", sizeof("Host: ") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, host, host_length, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the Upgrade in the header.  */
    status += nx_packet_data_append(packet_ptr, "Upgrade: websocket", sizeof("Upgrade: websocket") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the Connection in the header.  */
    status += nx_packet_data_append(packet_ptr, "Connection: Upgrade", sizeof("Connection: Upgrade") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the Sec-WebSocket-Key in the header.  */
    status += nx_packet_data_append(packet_ptr, "Sec-WebSocket-Key: ", sizeof("Sec-WebSocket-Key: ") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, client_ptr -> nx_websocket_client_key, client_ptr -> nx_websocket_client_key_size, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the connection in the header.  */
    status += nx_packet_data_append(packet_ptr, "Sec-WebSocket-Protocol: ", sizeof("Sec-WebSocket-Protocol: ") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, protocol, protocol_length, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Place the connection in the header.  */
    status += nx_packet_data_append(packet_ptr, "Sec-WebSocket-Version: 13", sizeof("Sec-WebSocket-Version: 13") - 1, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Fill the last \r\n.  */
    status += nx_packet_data_append(packet_ptr, NX_WEBSOCKET_CRLF, NX_WEBSOCKET_CRLF_SIZE, client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

    /* Check status.  */
    if (status)
    {

        /* Obtain the mutex, release the packet and return error status. */
        tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);
        nx_packet_release(packet_ptr);
        return(NX_WEBSOCKET_DATA_APPEND_FAILURE);
    }

    /* Send out the packet.  */
    status = _nx_websocket_client_packet_send(client_ptr, packet_ptr, wait_option);
    if (status)
    {

        /* Release the packet and return error status. */
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Obtain the mutex again. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Update the subprotocol name and length */
    client_ptr -> nx_websocket_client_subprotocol = protocol;
    client_ptr -> nx_websocket_client_subprotocol_length = protocol_length;

    /* Update the state.  */
    client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_CONNECTING;

    /* Set the frame flag to be unfragmented and corresponding opcode to be zero */
    client_ptr -> nx_websocket_client_frame_fragmented = NX_FALSE;
    client_ptr -> nx_websocket_client_frame_opcode = 0;

    /* Check if using non-blocking mode.  */
    if (wait_option == 0)
    {
        return(NX_IN_PROGRESS);
    }

    while (1)
    {

        /* Receive response.  */
        status = _nx_websocket_client_packet_receive(client_ptr, &packet_ptr, wait_option);
        if (status)
        {
            return(status);
        }

        /* Process the response.  */
        status = _nx_websocket_client_connect_response_check(client_ptr, packet_ptr, wait_option);

        /* If status is NX_IN_PROGRESS, continue to receive connect response.
           Otherwise, break the while loop.  */
        if (status != NX_IN_PROGRESS)
        {
            break;
        }
    }

    /* Return status.  */
    return(status);
}

#ifdef NX_SECURE_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_secure_connect                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket secure connect.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    tls_session                           Pointer to TLS session        */
/*    host                                  Pointer to host               */ 
/*    host_length                           Length of host                */
/*    uri_path                              Pointer to uri path           */ 
/*    uri_path_length                       Length of uri path            */
/*    protocol                              Pointer to protocol           */ 
/*    protocol_length                       Length of protocol            */
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_secure_connect   Actual websocket connect call */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_secure_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                           UCHAR *host, UINT host_length,
                                           UCHAR *uri_path, UINT uri_path_length,
                                           UCHAR *protocol, UINT protocol_length,UINT wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID) || 
        (tls_session == NX_NULL) ||
        (host == NX_NULL) || (host_length == 0) || 
        (uri_path == NX_NULL) || (uri_path_length == 0) ||
        (protocol == NX_NULL) || (protocol_length == 0))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual secure connect function.  */
    status = _nx_websocket_client_secure_connect(client_ptr, tls_session, host, host_length, uri_path, uri_path_length, protocol, protocol_length, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_secure_connect                 PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes a WebSocket connection over TLS session to the  */
/*    server.                                                             */
/*    Note: Application must establish a TLS connection before.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    tls_session                           Pointer to TLS session        */
/*    host                                  Pointer to host               */ 
/*    host_length                           Length of host                */
/*    uri_path                              Pointer to uri path           */ 
/*    uri_path_length                       Length of uri path            */
/*    protocol                              Pointer to protocol           */ 
/*    protocol_length                       Length of protocol            */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_secure_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                          UCHAR *host, UINT host_length,
                                          UCHAR *uri_path, UINT uri_path_length,
                                          UCHAR *protocol, UINT protocol_length,UINT wait_option)
{

UINT status;


    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Check the state.  */
    if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTED)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_ALREADY_CONNECTED);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTING)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_CONNECTING);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_INITIALIZE)
    {

        /* Release the mutex and return */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_INVALID_STATE);
    }

    /* Save the tls session pointer.  */
    client_ptr -> nx_websocket_client_tls_session_ptr = tls_session;
    client_ptr -> nx_websocket_client_use_tls = NX_TRUE;

    status = _nx_websocket_client_connect_internal(client_ptr, host, host_length, uri_path, uri_path_length, protocol, protocol_length, wait_option);

    /* Release the mutex and return */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
    return(status);
}
#endif /* NX_SECURE_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_name_compare                   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
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
/*    _nx_websocket_client_connect_response_process                       */
/*                                          Process connect response      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_name_compare(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length)
{
UCHAR   ch;

    /* Compare the length. */
    if(src_length != dest_length)
    {
        return(NX_WEBSOCKET_ERROR);
    }

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
        {
            return(NX_WEBSOCKET_ERROR);
        }

        /* Pickup next character. */
        src_length--;
        src++;
        dest++;
    }

    return(NX_WEBSOCKET_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_connect_response_process       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes connect response.                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_name_compare     Compare memory data           */
/*    _nx_sha1_initialize                   Initialize sha1               */
/*    _nx_sha1_update                       Update sha1                   */
/*    _nx_sha1_digest_calculate             Calculate sha1 digest         */
/*    _nx_utility_base64_encode             Base64 encode                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_connect_response_check                         */
/*                                          Check connect response        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_connect_response_process(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr)
{
UCHAR  *buffer_ptr;
UINT    offset;
UCHAR  *field_name;
UINT    field_name_length;
UCHAR  *field_value;
UINT    field_value_length;
UCHAR   digest[NX_WEBSOCKET_ACCEPT_DIGEST_SIZE];
UCHAR   key[NX_WEBSOCKET_ACCEPT_KEY_SIZE + 1];
UINT    key_size = 0;
UCHAR   upgrade_flag = NX_FALSE;
UCHAR   connection_flag = NX_FALSE;
UCHAR   protocol_cnt = 0;
UCHAR   accept_cnt = 0;

    NX_PARAMETER_NOT_USED(client_ptr);

    /* WebSocket opening handshake message format:
       HTTP/1.1 101 Switching Protocols
       Upgrade: websocket\r\n
       Server: Microsoft-HTTPAPI/2.0\r\n
       Sec-WebSocket-Protocol: mqtt\r\n
       Connection: Upgrade\r\n
       Sec-WebSocket-Accept: xxxxxxxxxx=\r\n
       Date: Mon, 06 Jun 2022 07:46:53 GMT\r\n
       \r\n
    */

    /* Setup pointer and offset.  */
    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr;
    offset = 0;

    /* Check the length.  */
    if ((packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) < 12)
    {
        return(NX_WEBSOCKET_INVALID_PACKET);
    }

    /* Check the status code. Must be "101" Switching Protocols.  */
    if ((buffer_ptr[9] != '1') || (buffer_ptr[10] != '0') || (buffer_ptr[11] != '1'))
    {
        return(NX_WEBSOCKET_INVALID_STATUS_CODE);
    }

    /* Skip over the first HTTP line (HTTP/1.1 101 Switching Protocols\r\n).  */
    offset = 12;
    buffer_ptr += 12;
    while(((buffer_ptr + 1) < packet_ptr -> nx_packet_append_ptr) &&
          (*buffer_ptr != '\r') && (*(buffer_ptr + 1) != '\n'))
    {
        buffer_ptr++;
        offset++;
    }

    /* Skip over the CR,LF. */
    buffer_ptr += 2;
    offset += 2;

    /* Loop until we find the "cr,lf,cr,lf" token.  */
    while (((buffer_ptr + 1) < packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != 0))
    {

        /* Check for the <cr,lf,cr,lf> token.  This signals a blank line, which also 
           specifies the start of the content.  */
        if ((*buffer_ptr == '\r') &&
            (*(buffer_ptr + 1) ==  '\n'))
        {

            /* Adjust the offset.  */
            offset = offset + 2;
            break;
        }

        /* We haven't seen the <cr,lf,cr,lf> so we are still processing header data.
           Extract the field name and it's value.  */
        field_name = buffer_ptr;
        field_name_length = 0;

        /* Look for the ':' that separates the field name from its value. */
        while ((buffer_ptr < packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != ':'))
        {
            buffer_ptr++;
            field_name_length++;
        }
        offset += field_name_length;

        /* Skip ':'.  */
        buffer_ptr++;
        offset++;

        /* Now skip over white space. */
        while ((buffer_ptr < packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
        {
            buffer_ptr++;
            offset++;
        }

        /* Now get the field value. */
        field_value = buffer_ptr;
        field_value_length = 0;

        /* Loop until we see a <CR, LF>. */
        while (((buffer_ptr + 1) < packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != '\r') && (*(buffer_ptr+1) != '\n'))
        {
            buffer_ptr++;
            field_value_length++;
        }
        offset += field_value_length;

        /* Skip over the CR,LF. */
        buffer_ptr += 2;
        offset += 2;

        /* Check the upgrade.  */
        if (_nx_websocket_client_name_compare((UCHAR *)field_name, field_name_length, (UCHAR *)"Upgrade", sizeof("Upgrade") - 1) == NX_SUCCESS)
        {
            if (_nx_websocket_client_name_compare((UCHAR *)field_value, field_value_length, (UCHAR *)"websocket", sizeof("websocket") - 1))
            {
                return(NX_WEBSOCKET_INVALID_PACKET);
            }

            upgrade_flag = NX_TRUE;
        }
        else if (_nx_websocket_client_name_compare((UCHAR *)field_name, field_name_length, (UCHAR *)"Connection", sizeof("Connection") - 1) == NX_SUCCESS)
        {
            if (_nx_websocket_client_name_compare((UCHAR *)field_value, field_value_length, (UCHAR *)"Upgrade", sizeof("Upgrade") - 1))
            {
                return(NX_WEBSOCKET_INVALID_PACKET);
            }

            connection_flag = NX_TRUE;
        }
        else if (_nx_websocket_client_name_compare((UCHAR *)field_name, field_name_length, (UCHAR *)"Sec-WebSocket-Protocol", sizeof("Sec-WebSocket-Protocol") - 1) == NX_SUCCESS)
        {
            if (_nx_websocket_client_name_compare((UCHAR *)field_value, field_value_length, client_ptr -> nx_websocket_client_subprotocol, client_ptr -> nx_websocket_client_subprotocol_length))
            {
                return(NX_WEBSOCKET_INVALID_PACKET);
            }

            protocol_cnt++;
        }
        else if (_nx_websocket_client_name_compare((UCHAR *)field_name, field_name_length, (UCHAR *)"Sec-WebSocket-Accept", sizeof("Sec-WebSocket-Accept") - 1) == NX_SUCCESS)
        {

            /* Calculate the SHA-1 hash of the concatenation of the client key and the Globally Unique Identifier (GUID)
               Referenced in RFC 6455, Section 1.3, Page 6 */
            _nx_sha1_initialize(&(client_ptr -> nx_websocket_client_sha1));
            _nx_sha1_update(&(client_ptr -> nx_websocket_client_sha1), client_ptr->nx_websocket_client_key, client_ptr->nx_websocket_client_key_size);
            _nx_sha1_update(&(client_ptr -> nx_websocket_client_sha1), (UCHAR*)NX_WEBSOCKET_ACCEPT_PREDEFINED_GUID, NX_WEBSOCKET_ACCEPT_PREDEFINED_GUID_SIZE);
            _nx_sha1_digest_calculate(&(client_ptr -> nx_websocket_client_sha1), digest);

            /* Encode the hash and compare it with the field value from the server.  */
            _nx_utility_base64_encode(digest, NX_WEBSOCKET_ACCEPT_DIGEST_SIZE, key, (NX_WEBSOCKET_ACCEPT_KEY_SIZE + 1), &key_size);
            if ((field_value_length != NX_WEBSOCKET_ACCEPT_KEY_SIZE) || (memcmp((void *)field_value, (void *)key, NX_WEBSOCKET_ACCEPT_KEY_SIZE))) /* Use case of memcpy is verified. */
            {
                return(NX_WEBSOCKET_INVALID_PACKET);
            }

            accept_cnt++;
        }
    }

    /* Check if the all fields are processed and found as required.  */
    if ((offset != packet_ptr -> nx_packet_length) ||
        (upgrade_flag != NX_TRUE) || (connection_flag != NX_TRUE) ||
        (protocol_cnt != 1) || (accept_cnt != 1)) /* Both sec-websocket-protocol field and sec-websocket-accept field are allowed occur once only.
                                                     Reference in RFC 6455, Section 11.3.3 and 11.3.4, Page 59-60 */
    {
        return(NX_WEBSOCKET_INVALID_PACKET);
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_disconnect                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket disconnect.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_disconnect       Actual disconnect call        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_disconnect(NX_WEBSOCKET_CLIENT *client_ptr, UINT wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual disconnect function.  */
    status = _nx_websocket_client_disconnect(client_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_disconnect                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disconnects the WebSocket connection created          */
/*    previously.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    wait_option                           Wait option                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_packet_allocate  Allocate websocket packet     */
/*    _nx_websocket_client_send             Send websocket packet         */
/*    nx_packet_release                     Release websocket packet      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_disconnect(NX_WEBSOCKET_CLIENT *client_ptr, UINT wait_option)
{

UINT        status;
NX_PACKET  *packet_ptr;


    /* Check the state.  */
    if ((client_ptr -> nx_websocket_client_state <= NX_WEBSOCKET_CLIENT_STATE_IDLE) || (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_SENT))
    {

        /* Return error status */
        return(NX_WEBSOCKET_INVALID_STATE);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTING)
    {

        /* Update the state and return */
        client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;
        return(NX_WEBSOCKET_NOT_CONNECTED);
    }

    /* Allocate a packet.  */
    status = _nx_websocket_client_packet_allocate(client_ptr, &packet_ptr, wait_option);

    /* Check the status.  */
    if (status == NX_SUCCESS)
    {

        /* Send out the packet.  */
        status = _nx_websocket_client_send(client_ptr, packet_ptr, NX_WEBSOCKET_OPCODE_CONNECTION_CLOSE, NX_TRUE, wait_option);

        /* Check the status.  */
        if (status)
        {
            nx_packet_release(packet_ptr);
        }
    }

    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Check if the CLOSE frame has already been received before */
    if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_RECEIVED)
    {
        client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;
    }
    else /* i.e. The state is CONNECTED */
    {
        client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_SENT;
    }

    /* Release the mutex and return completion status */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_send                          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket send.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet             */ 
/*    code                                  Opcode: text or binary frame  */ 
/*    is_final                              Flag: final fragment or not   */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_send             Actual websocket send call    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT code, UINT is_final, UINT wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual send function.  */
    status = _nx_websocket_client_send(client_ptr, packet_ptr, code, is_final, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_send                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends Websocket data frame to server.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet             */ 
/*    code                                  Opcode: text or binary frame  */ 
/*    is_final                              Flag: final fragment or not   */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_packet_send      Send websocket packet         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT code, UINT is_final, UINT wait_option)
{

UINT status;
UCHAR *data_ptr;
NX_PACKET *data_packet;
USHORT message;
ULONG tmp;
UCHAR masking_key[4];
UINT mask_id = 0;
UINT header_size = NX_WEBSOCKET_HEADER_NORMAL_SIZE;


    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Check the state */
    if (client_ptr -> nx_websocket_client_state < NX_WEBSOCKET_CLIENT_STATE_CONNECTED)
    {

        /* Release the mutex and return error status */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_NOT_CONNECTED);
    }
    else if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_SENT)
    {

        /* Release the mutex and return error status */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_INVALID_STATE);
    }

    /* Check the payload length.  */
    if (packet_ptr -> nx_packet_length > 65535)
    {

        /* Release the mutex and return error status */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_NOT_SUPPORTED);
    }
    else if (packet_ptr -> nx_packet_length > 125)
    {
        header_size += 2;
    }

    /* Check the packet.  */
    if ((UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < header_size)
    {

        /* Release the mutex and return error status */
        tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
        return(NX_WEBSOCKET_INVALID_PACKET);
    }

    /* Adjust the pointer for filling the WebSocket header.  */
    data_ptr = packet_ptr -> nx_packet_prepend_ptr - header_size;

    /* Fill the first byte (FIN + OPCODE) in header.  */
    if (is_final == NX_TRUE)
    {
        *data_ptr = (UCHAR)(NX_WEBSOCKET_FIN | code);
    }
    else
    {
        *data_ptr = (UCHAR)code;
    }
    data_ptr++;

    /* Fill the next byte for MASK and Payload length.  */
    *data_ptr = NX_WEBSOCKET_MASK;
    if (packet_ptr -> nx_packet_length < 125)
    {
        *data_ptr |= (UCHAR)packet_ptr -> nx_packet_length;
        data_ptr++;
    }
    else
    {
        *data_ptr |= NX_WEBSOCKET_PAYLOAD_LEN_16BITS;
        data_ptr++;

        /* Fill the next two bytes for extended payload length.  */
        message = (USHORT)packet_ptr -> nx_packet_length;
        NX_CHANGE_USHORT_ENDIAN(message);
        memcpy(data_ptr, &message, NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE); /* Use case of memcpy is verified. */
        data_ptr += NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE;
    }

    /* Fill the masking key, the masking key is a 32-bit value chosen at random, must mask websocket data from client*/
    tmp = (ULONG)NX_RAND();
    masking_key[0] = (UCHAR)(tmp >> 24);
    masking_key[1] = (UCHAR)(tmp >> 16);
    masking_key[2] = (UCHAR)(tmp >> 8);
    masking_key[3] = (UCHAR)(tmp);
    memcpy(data_ptr, &masking_key, NX_WEBSOCKET_MASKING_KEY_SIZE); /* Use case of memcpy is verified. */
    data_ptr += NX_WEBSOCKET_MASKING_KEY_SIZE;

    /* Mask all payload data.  */
    data_packet = packet_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
    while(data_packet)
    {
#endif /* NX_DISABLE_PACKET_CHAIN  */

        data_ptr = data_packet -> nx_packet_prepend_ptr;
        while(data_ptr < data_packet -> nx_packet_append_ptr)
        {
            *data_ptr ^= masking_key[mask_id % 4];
            mask_id++;
            data_ptr++;
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        data_packet = data_packet -> nx_packet_next;
    }
#endif /* NX_DISABLE_PACKET_CHAIN  */

    /* Update prepend pointer and packet length to include WebSocket header.  */
    packet_ptr -> nx_packet_prepend_ptr -= header_size;
    packet_ptr -> nx_packet_length += header_size;

    /* Release the mutex */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

    /* Send out the packet.  */
    status = _nx_websocket_client_packet_send(client_ptr, packet_ptr, wait_option);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_receive                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket receive.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    code                                  Opcode: text or binary frame  */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_receive          Actual websocket receive call */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code, UINT wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID) ||
        (packet_ptr == NX_NULL) || (code == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual process function.  */
    status = _nx_websocket_client_receive(client_ptr, packet_ptr, code, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_receive                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives Websocket data frame from server.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    code                                  Opcode: text or binary frame  */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_packet_receive   Receive websocket packet      */
/*    _nx_websocket_client_data_process     Process data frame            */
/*    _nx_websocket_client_connect_response_check                         */
/*                                          Check connect response        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code, UINT wait_option)
{

UINT        status = NX_SUCCESS;


    /* Initialize here since packet_ptr will be compared with NX_NULL in following function */
    *packet_ptr = NX_NULL;

    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    while (1) /* The while loop ensures parsing all received packets. */
    {

        /* Check the state */
        if ((client_ptr -> nx_websocket_client_state < NX_WEBSOCKET_CLIENT_STATE_CONNECTING) ||
            (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_RECEIVED))
        {

            /* Release the mutex and return error status */
            tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
            return(NX_WEBSOCKET_INVALID_STATE);
        }

        /* If the state is NX_WEBSOCKET_CLIENT_STATE_CONNECTING, the received packet should be connect response.  */
        if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_CONNECTING)
        {

            /* Receive the data packet */
            status = _nx_websocket_client_packet_receive(client_ptr, packet_ptr, wait_option);
            if (status != NX_SUCCESS)
            {

                /* Release the mutex and return status */
                tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
                return(status);
            }

            /* Process the connect response.  */
            status = _nx_websocket_client_connect_response_check(client_ptr, *packet_ptr, wait_option);

            if ((status == NX_SUCCESS) || (status == NX_IN_PROGRESS))
            {

                /* Continue to receive remaining connect response or application data.  */
                continue;
            }
            else
            {

                /* Release the mutex and return status */
                tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
                return(status);
            }
        }
        else
        {

            /* Check if there is an existing complete frame in the waiting list */
            if ((status == NX_SUCCESS) && (client_ptr -> nx_websocket_client_processing_packet != NX_NULL))
            {

                /* Parse the data packet. */
                status = _nx_websocket_client_data_process(client_ptr, packet_ptr, code);
                if (status != NX_CONTINUE)
                {

                    /* Release the mutex and return directly if a complete frame is parsed or any error status is found. */
                    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
                    return(status);
                }
            }

            /* No existing frame, go to receive the data packet */
            status = _nx_websocket_client_packet_receive(client_ptr, packet_ptr, wait_option);
            if (status != NX_SUCCESS)
            {

                /* Release the mutex and return status */
                tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
                return(status);
            }

            /* Parse with the new data packet. */
            status = _nx_websocket_client_data_process(client_ptr, packet_ptr, code);
            if (status != NX_CONTINUE)
            {

                /* Release the mutex and return directly if a complete frame is parsed or any error status is found. */
                tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));
                return(status);
            }

            /* Due to no application data found, continue to check if there is any pending data packet. */
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_data_process                   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes Websocket data frame from server.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    code                                  Opcode: text or binary frame  */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_extract_offset         Extract data from packet      */
/*    nx_packet_allocate                    Allocate a packet             */
/*    _nx_websocket_client_packet_trim      Trim data from packet         */
/*    _nx_websocket_client_cleanup          Cleanup resource              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_receive          Receive websocket data        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_data_process(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code)
{
UINT  status;
UCHAR fin_bit = NX_FALSE;
UCHAR opcode = 0;
UCHAR bytes[4];
ULONG payload_length;
ULONG offset = 0;
ULONG bytes_copied;
ULONG packet_length;
NX_PACKET *data_packet;
UCHAR *data_ptr;


    /* Is there a packet waiting for processing? */
    if (client_ptr -> nx_websocket_client_processing_packet)
    {
        if (*packet_ptr != NX_NULL)
        {

            /* Yes. Link received packet to existing one. */
            if (client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last)
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last -> nx_packet_next = *packet_ptr;
            }
            else
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_next = *packet_ptr;
            }
            if ((*packet_ptr) -> nx_packet_last)
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last = (*packet_ptr) -> nx_packet_last;
            }
            else
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last = *packet_ptr;
            }
            client_ptr -> nx_websocket_client_processing_packet -> nx_packet_length += (*packet_ptr) -> nx_packet_length;
        }

        /* Make packet_ptr point to the waiting list */
        *packet_ptr = client_ptr -> nx_websocket_client_processing_packet;
    }
    else
    {
        if (*packet_ptr == NX_NULL)
        {
            return(NX_WEBSOCKET_INVALID_PACKET);
        }

        client_ptr -> nx_websocket_client_processing_packet = *packet_ptr;
    }

    /* Check if the websocket frame header shall be parsed and found first */
    if (client_ptr -> nx_websocket_client_frame_header_found == NX_FALSE)
    {

        /* Parse the first 2 bytes. */
        if (nx_packet_data_extract_offset(*packet_ptr, offset, bytes, NX_WEBSOCKET_HEADER_MINIMUM_LENGTH, &bytes_copied)
            || (bytes_copied < NX_WEBSOCKET_HEADER_MINIMUM_LENGTH))
        {
            return(NX_CONTINUE);
        }

        /* Update the offset */
        offset += NX_WEBSOCKET_HEADER_MINIMUM_LENGTH;

        /* Obtain the fin bit and opcode */
        fin_bit = bytes[0] & NX_WEBSOCKET_FIN_MASK;
        opcode = bytes[0] & NX_WEBSOCKET_OPCODE_MASK;

        /* Parse the mask bit and payload length */
        if (bytes[1] & NX_WEBSOCKET_MASK)
        {
            client_ptr -> nx_websocket_client_frame_masked = NX_TRUE;
        }
        else
        {
            client_ptr -> nx_websocket_client_frame_masked = NX_FALSE;
        }

        payload_length = (UCHAR)(bytes[1] & NX_WEBSOCKET_PAYLOAD_LEN_MASK);
        if (payload_length < 126)
        {

            /* No extend payload length; record data payload length directly. */
        }
        else if (payload_length == 126)
        {

            /* Extract the 16-bit extended data payload length */
            if (nx_packet_data_extract_offset(*packet_ptr, offset, bytes, NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE, &bytes_copied)
                || (bytes_copied < NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE))
            {
                return(NX_CONTINUE);
            }

            /* Record 16-bit data payload length. */
            payload_length = ((((ULONG)bytes[0]) << 8) + (ULONG)bytes[1]);

            /* Add the byte count by the payload size */
            offset += NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE;
        }
        else
        {

            /* Since 64 bits extend payload length is not supported, clean up and return directly. */
            _nx_websocket_client_cleanup(client_ptr);
            return(NX_NOT_SUPPORTED);
        }

        /* Parse the masking key */
        if (client_ptr -> nx_websocket_client_frame_masked == NX_TRUE)
        {
            if (nx_packet_data_extract_offset(*packet_ptr, offset, bytes, NX_WEBSOCKET_MASKING_KEY_SIZE, &bytes_copied)
                || (bytes_copied < NX_WEBSOCKET_MASKING_KEY_SIZE))
            {
                return(NX_CONTINUE);
            }

            /* Get the masking key. */
            memcpy(client_ptr -> nx_websocket_client_frame_masking_key, bytes, NX_WEBSOCKET_MASKING_KEY_SIZE); /* Use case of memcpy is verified. */

            /* Add the byte count by the masking key size */
            offset += NX_WEBSOCKET_MASKING_KEY_SIZE;
        }

        /* Set the flag to indicate the frame header found, and re-initialize corresponding variables */
        client_ptr -> nx_websocket_client_frame_header_found = NX_TRUE;
        client_ptr -> nx_websocket_client_frame_data_received = 0;

        /* Record the payload length for judging if all payload data received */
        client_ptr -> nx_websocket_client_frame_data_length = payload_length;

        /* Check the rules apply to fragmentation corresponding to the FIN bit and opcode
        RFC 6455, Section 5.4, Page 33-35 */
        if (fin_bit == NX_WEBSOCKET_FIN) /* This is the final frame (tip: the first frame may also be the final frame) */
        {
            if (client_ptr -> nx_websocket_client_frame_fragmented == NX_FALSE) /* A single unfragmented frame shall be received */
            {
                if (opcode == 0) /* The opcode should not denotes a continuation frame for a single unfragmented frame */
                {
                    _nx_websocket_client_cleanup(client_ptr);
                    return(NX_INVALID_PACKET);
                }

                /* Update the header opcode */
                client_ptr -> nx_websocket_client_frame_opcode = opcode;
            }
            else /* This is the termination frame in overall fragmented frames */
            {
                if (opcode != 0) /* The opcode of the termination frame shall be zero */
                {
                    _nx_websocket_client_cleanup(client_ptr);
                    return(NX_INVALID_PACKET);
                }

                /* Set the header flag to be unfragmented for next time to use */
                client_ptr -> nx_websocket_client_frame_fragmented = NX_FALSE;
            }
        }
        else /* This is not the final header */
        {
            if (client_ptr -> nx_websocket_client_frame_fragmented == NX_FALSE) /* This is the beginning frame in fragmented frames */
            {

                /* The opcode of the beginning frame shall indicate the opcode of overall fragmented frames. Besides,
                since control frames cannot be fragmented, the supported frame type shall be text or binary */
                if ((opcode != NX_WEBSOCKET_OPCODE_BINARY_FRAME) && (opcode != NX_WEBSOCKET_OPCODE_TEXT_FRAME))
                {
                    _nx_websocket_client_cleanup(client_ptr);
                    return(NX_INVALID_PACKET);
                }

                /* Update the frame fragmented flag and the opcode since a beginning frame is received */
                client_ptr -> nx_websocket_client_frame_fragmented = NX_TRUE;
                client_ptr -> nx_websocket_client_frame_opcode = opcode;
            }
            else /* This is a continuation frame in overall fragmented frames */
            {
                if (opcode != 0) /* The opcode of a continuation frame shall be zero */
                {
                    _nx_websocket_client_cleanup(client_ptr);
                    return(NX_INVALID_PACKET);
                }
            }
        }

        /* Trim the WebSocket header */
        status = _nx_websocket_client_packet_trim(client_ptr, packet_ptr, offset);
        client_ptr -> nx_websocket_client_processing_packet = *packet_ptr;
        if (status)
        {
            if (status == NX_NO_PACKET)
            {

                /* Try to receive more payload data from TCP/TLS if the packet holds the WebSocket header only */
                return(NX_CONTINUE);
            }

            /* Return error status */
            return(status);
        }
    }

    /* Reset payload length and use the variable to count the data length processed by the function call this time */
    payload_length = 0;

    /* Unmask payload data if there is masking key. */
    if (client_ptr -> nx_websocket_client_frame_masked == NX_TRUE)
    {
        data_packet = (*packet_ptr);
#ifndef NX_DISABLE_PACKET_CHAIN
        while (data_packet && client_ptr -> nx_websocket_client_frame_header_found)
        {
#endif /* NX_DISABLE_PACKET_CHAIN  */

            data_ptr = (*packet_ptr) -> nx_packet_prepend_ptr;
            while (data_ptr < data_packet -> nx_packet_append_ptr)
            {

                /* Unmask payload data byte by byte */
                *data_ptr ^= client_ptr -> nx_websocket_client_frame_masking_key[client_ptr -> nx_websocket_client_frame_data_received % 4];
                data_ptr++;

                /* Increase the payload length for the usage in frame process */
                payload_length++;

                /* Check and jump out if all data payload in the frame have been processed. */
                client_ptr -> nx_websocket_client_frame_data_received++;
                if (client_ptr -> nx_websocket_client_frame_data_received >= client_ptr -> nx_websocket_client_frame_data_length)
                {

                    /* Reset the frame header flag as not found and break. */
                    client_ptr -> nx_websocket_client_frame_header_found = NX_FALSE;
                    break;
                }
            }

#ifndef NX_DISABLE_PACKET_CHAIN
            data_packet = data_packet -> nx_packet_next;
        }
#endif /* NX_DISABLE_PACKET_CHAIN  */
    }

    /* Add the payload length if no masking key */
    else
    {

        /* Check and adjust received data length for processing */
        payload_length = (*packet_ptr) -> nx_packet_length;
        if (payload_length >= (client_ptr -> nx_websocket_client_frame_data_length - client_ptr -> nx_websocket_client_frame_data_received))
        {

            /* The maximum the payload length for each frame process shall not exceed the remaining frame data to be received */
            payload_length = (client_ptr -> nx_websocket_client_frame_data_length - client_ptr -> nx_websocket_client_frame_data_received);

            /* Reset the frame header flag as not found. */
            client_ptr -> nx_websocket_client_frame_header_found = NX_FALSE;
        }

        /* Add the length to the total count */
        client_ptr -> nx_websocket_client_frame_data_received += payload_length;
    }

    /* Check the opcode for the received frame, and return corresponding status. */
    switch (opcode)
    {
        case NX_WEBSOCKET_OPCODE_CONTINUATION_FRAME:
        case NX_WEBSOCKET_OPCODE_TEXT_FRAME:
        case NX_WEBSOCKET_OPCODE_BINARY_FRAME:
        {

            /* Assign the return opcode by the pre-stored opcode */
            *code = client_ptr -> nx_websocket_client_frame_opcode;

            /* Update the offset by payload length */
            offset = payload_length;

            /* For a data frame (i.e. text/binary frame), search and find the end of the complete frame */
            data_packet = *packet_ptr;
            packet_length = (ULONG)(data_packet -> nx_packet_append_ptr - data_packet -> nx_packet_prepend_ptr);
            while (packet_length < offset)
            {
                offset -= packet_length;

                /* Move the current data packet pointer to next and compute the length of next packet */
                data_packet = data_packet -> nx_packet_next;
                packet_length = (ULONG)(data_packet -> nx_packet_append_ptr - data_packet -> nx_packet_prepend_ptr);
            }

            /* After subtracting by the offset, packet_length represents the size of remaining data in the single packet to be linked into the waiting list */
            packet_length -= offset;

            /* Check if the frame end is just in the end of one of the packet(s) */
            if (packet_length == 0)
            {

                /* Put remaining packet(s) into the waiting list */
                client_ptr -> nx_websocket_client_processing_packet = data_packet -> nx_packet_next;

                /* Check if there is data remaining and determine whether to go on to following logic. */
                if (data_packet -> nx_packet_next == NX_NULL)
                {

                    /* The packet is fully parsed, return success directly. */
                    return(NX_WEBSOCKET_SUCCESS);
                }
            }
            else /* One more frame in the packet */
            {

                /* Release the mutex. */
                tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

                /* Allocate a packet for the remaining data in the packet. */
                status = nx_packet_allocate((*packet_ptr) -> nx_packet_pool_owner, &client_ptr -> nx_websocket_client_processing_packet, NX_RECEIVE_PACKET, NX_WAIT_FOREVER);

                /* The return status may be not NX_SUCCESS only there is an unexpected issue from the packet pool (e.g. a delete operation ).
                   Assert here for the unexpected issue shall not happen in normal status */
                NX_ASSERT(status == NX_SUCCESS);

                /* Obtain the mutex again. */
                tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

                /* Copy the contents of the remaining part of current packet into the new allocated packet */
                memcpy((void *)client_ptr -> nx_websocket_client_processing_packet -> nx_packet_prepend_ptr,
                       (void *)(data_packet -> nx_packet_prepend_ptr + offset), packet_length); /* Use case of memcpy is verified. */

                /* Move the append pointer with by the extended length */
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_append_ptr += packet_length;
                
                /* Link the possibly had remaining packet(s) to the waiting list. The overall packet length will be updated outside this else branch */
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_next = data_packet -> nx_packet_next;
            }

            /* Update the last packet pointer in the waiting liast */
            if (client_ptr -> nx_websocket_client_processing_packet -> nx_packet_next)
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last = (*packet_ptr) -> nx_packet_last;
            }
            else
            {
                client_ptr -> nx_websocket_client_processing_packet -> nx_packet_last = NX_NULL;
            }

            /* Update the overall packet length for the waiting list */
            client_ptr -> nx_websocket_client_processing_packet -> nx_packet_length = (*packet_ptr) -> nx_packet_length - payload_length;

            /* Disconnect the link between the return packet and the waiting list */
            data_packet -> nx_packet_next = NX_NULL;
            data_packet -> nx_packet_append_ptr -= packet_length;
            (*packet_ptr) -> nx_packet_length = payload_length;

            /* Update the last packet pointer of the returned packet */
            if ((*packet_ptr) -> nx_packet_next)
            {
                (*packet_ptr) -> nx_packet_last = data_packet;
            }
            else
            {
                (*packet_ptr) -> nx_packet_last = NX_NULL;
            }

            /* Return success status */
            return(NX_WEBSOCKET_SUCCESS);
        }

        case NX_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
        {
            /* Make sure the complete control frame is received */
            if (client_ptr -> nx_websocket_client_frame_data_received < client_ptr -> nx_websocket_client_frame_data_length)
            {
                return(NX_CONTINUE);
            }

            /* A disconnection is informed, notify the application.  */
            if (client_ptr -> nx_websocket_client_connection_status_callback)
            {
                client_ptr -> nx_websocket_client_connection_status_callback(client_ptr, client_ptr -> nx_websocket_client_connection_context, NX_WEBSOCKET_DISCONNECTED);
            }

            /* Check the current state and update the state when the CLOSE frame is found */
            if (client_ptr -> nx_websocket_client_state == NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_SENT)
            {
                client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;
            }
            else
            {
                client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_RECEIVED;
            }

            /* Return disconnect received status only, without returning any packet data, since it is required no more data after the Close frame.
               There is no need to release the packet_ptr since it points to the same memory region as the waiting list
               Referenced in RFC 6455, Section 5.5.1, Page 36 */
            _nx_websocket_client_cleanup(client_ptr);
            return(NX_WEBSOCKET_DISCONNECTED);
        }

        case NX_WEBSOCKET_OPCODE_PING:
        case NX_WEBSOCKET_OPCODE_PONG:
        {

            /* Make sure the complete control frame is received */
            if (client_ptr -> nx_websocket_client_frame_data_received < client_ptr -> nx_websocket_client_frame_data_length)
            {
                return(NX_CONTINUE);
            }

            /* Trim payload data in the frame. */
            status = _nx_websocket_client_packet_trim(client_ptr, packet_ptr, client_ptr -> nx_websocket_client_frame_data_received);

            /* Update the waiting list */
            client_ptr -> nx_websocket_client_processing_packet = *packet_ptr;

            /* Check if error status happens */
            if ((status != NX_WEBSOCKET_SUCCESS) && (status != NX_NO_PACKET))
            {
                return(status);
            }

            /* A PING/PONG frame is parsed and found, continue to check any more data or frame received */
            return(NX_CONTINUE);
        }

        default:
        {

            /* Clean up and return invalid status */
            _nx_websocket_client_cleanup(client_ptr);
            return(NX_INVALID_PACKET);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_packet_trim                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function trims extra bytes from the data packet.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    trim_size                             Number of bytes to remove     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_release                     Release the packet            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_data_process     Process data frame            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_packet_trim(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG trim_size)
{

ULONG       packet_length;
NX_PACKET   *head_packet_ptr = *packet_ptr;
NX_PACKET   *previous_packet_ptr = NX_NULL;

    NX_PARAMETER_NOT_USED(client_ptr);

    /* The trim size shall be less than or equal to the packet length */
    if ((*packet_ptr) -> nx_packet_length < trim_size)
    {
        return(NX_PACKET_OFFSET_ERROR);
    }
    packet_length = (*packet_ptr) -> nx_packet_length - trim_size;

    /* Search and find the trim point */
    while ((ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr) <= trim_size)
    {
        trim_size -= (ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr);

        previous_packet_ptr = *packet_ptr;
        *packet_ptr = (*packet_ptr) -> nx_packet_next;

        if (*packet_ptr == NX_NULL)
        {
            break;
        }
    }

    /* Disconnect the link if the trim point is not in the head packet */
    if (previous_packet_ptr != NX_NULL)
    {
        previous_packet_ptr -> nx_packet_next = NX_NULL;

        /* Set the nx_packet_last pointer due to the packet header changed */
        if (*packet_ptr)
        {
            (*packet_ptr) -> nx_packet_last = head_packet_ptr -> nx_packet_last;
        }

        nx_packet_release(head_packet_ptr);
    }

    if (*packet_ptr)
    {

        /* Adjust current packet */
        (*packet_ptr) -> nx_packet_prepend_ptr += trim_size;
        (*packet_ptr) -> nx_packet_length = packet_length;

        return(NX_WEBSOCKET_SUCCESS);
    }

    /* Return this value to tell the caller that the whole packet is trimmed */
    return(NX_NO_PACKET);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_packet_allocate               PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket packet allocate.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_packet_allocate  Actual websocket allocate call*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_packet_allocate(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        status;


    /* Check for invalid input pointers */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID) || (packet_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual process function.  */
    status = _nx_websocket_client_packet_allocate(client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_packet_allocate                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a Websocket packet.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_allocate                    Allocate a packet             */
/*    nx_packet_release                     Release packet                */
/*    nx_secure_tls_packet_allocate         Allocate a TLS packet         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_packet_allocate(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        status;


#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nx_websocket_client_use_tls)
    {

        /* Use TLS packet allocate.  The TLS packet allocate is able to count for 
           TLS-related header space including crypto initial vector area. */
        status = nx_secure_tls_packet_allocate(client_ptr -> nx_websocket_client_tls_session_ptr,
                                               client_ptr -> nx_websocket_client_packet_pool_ptr,
                                               packet_ptr, TX_WAIT_FOREVER);
    }
    else
    {
#endif /* NX_SECURE_ENABLE */

        /* Allocate packet.  */
        if (client_ptr -> nx_websocket_client_socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            status =  nx_packet_allocate(client_ptr -> nx_websocket_client_packet_pool_ptr,
                                         packet_ptr,
                                         NX_IPv4_TCP_PACKET, wait_option);
        }
        else
        {
            status =  nx_packet_allocate(client_ptr -> nx_websocket_client_packet_pool_ptr,
                                         packet_ptr,
                                         NX_IPv6_TCP_PACKET, wait_option);
        }
#ifdef NX_SECURE_ENABLE
    }
#endif /* NX_SECURE_ENABLE */

    if (status == NX_SUCCESS)
    {

        /* Check the buffer size for the basic data header of websocket.  */
        if (((ULONG)(((*packet_ptr) -> nx_packet_data_end) - ((*packet_ptr) -> nx_packet_prepend_ptr))) < NX_WEBSOCKET_HEADER_SIZE)
        {

            /* Packet buffer is too small. */
            nx_packet_release(*packet_ptr);
            return(NX_WEBSOCKET_INVALID_PACKET);
        }

        /* Adjust the pointers.  */
        ((*packet_ptr) -> nx_packet_prepend_ptr) += NX_WEBSOCKET_HEADER_SIZE;
        ((*packet_ptr) -> nx_packet_append_ptr) += NX_WEBSOCKET_HEADER_SIZE;
    }

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_packet_send                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends Websocket packet.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet             */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_socket_send                    Send tcp packet               */
/*    nx_secure_tls_session_send            Send tls packet               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*    _nx_websocket_client_send             Send websocket data frame     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_packet_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT status;

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nx_websocket_client_use_tls)
    {
        status = nx_secure_tls_session_send(client_ptr -> nx_websocket_client_tls_session_ptr, packet_ptr, wait_option);
    }
    else
    {
        status = nx_tcp_socket_send(client_ptr -> nx_websocket_client_socket_ptr, packet_ptr, wait_option);
    }
#else
    status = nx_tcp_socket_send(client_ptr -> nx_websocket_client_socket_ptr, packet_ptr, wait_option);
#endif /* NX_SECURE_ENABLE */

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_packet_receive                 PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives Websocket packet.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet pointer     */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_socket_receive                 Receive tcp packet            */
/*    nx_secure_tls_session_receive         Receive tls packet            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*    _nx_websocket_client_receive          Receive websocket data frame  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_packet_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT status;


    /* Release the mutex first */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

#ifdef NX_SECURE_ENABLE
    if (client_ptr -> nx_websocket_client_use_tls)
    {
        status = nx_secure_tls_session_receive(client_ptr -> nx_websocket_client_tls_session_ptr, packet_ptr, wait_option);
    }
    else
    {
        status = nx_tcp_socket_receive(client_ptr -> nx_websocket_client_socket_ptr, packet_ptr, wait_option);
    }
#else
    status = nx_tcp_socket_receive(client_ptr -> nx_websocket_client_socket_ptr, packet_ptr, wait_option);
#endif /* NX_SECURE_ENABLE */

    /* Obtain the mutex again. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_connect_response_check         PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if a whole connect response is received, then  */
/*    process the response.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    packet_ptr                            Pointer to packet             */ 
/*    wait_option                           Wait option                   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append data into packet       */
/*    nx_packet_release                     Release packet                */
/*    _nx_websocket_client_connect_response_process                       */
/*                                          Process connect response      */
/*    _nx_websocket_client_cleanup          Cleanup resource              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*    _nx_websocket_client_receive          Receive websocket data frame  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_connect_response_check(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT wait_option)
{
CHAR *buffer_ptr;
UINT status = NX_SUCCESS;
UINT crlf_found = 0;
NX_PACKET *tmp_ptr;


    if (client_ptr -> nx_websocket_client_processing_packet == NX_NULL)
    {
        client_ptr -> nx_websocket_client_processing_packet = packet_ptr;
    }
    else
    {

        /* Its contents now need to be placed in the head packet.  */
        tmp_ptr = packet_ptr;

#ifndef NX_DISABLE_PACKET_CHAIN
        while (tmp_ptr)
#endif /* NX_DISABLE_PACKET_CHAIN */
        {

            /* Release the mutex */
            tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

            /* Copy the contents of the current packet into the head packet.  */
            status = nx_packet_data_append(client_ptr -> nx_websocket_client_processing_packet,
                                           (VOID *) tmp_ptr -> nx_packet_prepend_ptr,
                                           (ULONG)(tmp_ptr -> nx_packet_append_ptr - tmp_ptr -> nx_packet_prepend_ptr),
                                           client_ptr -> nx_websocket_client_packet_pool_ptr, wait_option);

            /* Obtain the mutex. */
            tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

            /* Determine if an error occurred or an unexpected packet chain happens since the connect response shall not exceed the maximum length of one packet */
            if ((status != NX_SUCCESS) || (client_ptr -> nx_websocket_client_processing_packet -> nx_packet_next))
            {

                /* Reset the state to idle */
                client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;

                /* Clean up unused resources. */
                _nx_websocket_client_cleanup(client_ptr);

                /* Release the new packet and return. */
                nx_packet_release(packet_ptr);
                return(status);
            }
#ifndef NX_DISABLE_PACKET_CHAIN
            else
            {
                tmp_ptr = tmp_ptr -> nx_packet_next;
            }
#endif /* NX_DISABLE_PACKET_CHAIN */
        }

        /* Release the new packet. */
        nx_packet_release(packet_ptr);
    }

    crlf_found = 0;
    tmp_ptr = client_ptr -> nx_websocket_client_processing_packet;

    while (1)
    {

        /* Build a pointer to the buffer area.  */
        buffer_ptr =  (CHAR *) tmp_ptr -> nx_packet_prepend_ptr;
    
        /* See if there is a blank line present in the buffer.  */
        /* Search the buffer for a cr/lf pair.  */
        while ((buffer_ptr < (CHAR *) tmp_ptr -> nx_packet_append_ptr) &&
               (crlf_found < 4))
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
    
            /* Move the buffer pointer up.  */
            buffer_ptr++;
        }

        if (crlf_found == 4)
        {

            /* Yes, we have found the end of the HTTP response header.  */
            break;
        }

#ifndef NX_DISABLE_PACKET_CHAIN

        if (tmp_ptr -> nx_packet_next != NX_NULL)
        {

            /* Get the next packet in the chain. */
            tmp_ptr  = tmp_ptr -> nx_packet_next;
        }
        else
#endif
        {
            return(NX_IN_PROGRESS);
        }
    }

    /* Process the response.  */
    status = _nx_websocket_client_connect_response_process(client_ptr, client_ptr -> nx_websocket_client_processing_packet);
    if (status == NX_SUCCESS)
    {

        /* Update the state.  */
        client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_CONNECTED;

        /* If connection is established, notify application.  */
        if (client_ptr -> nx_websocket_client_connection_status_callback)
        {
            client_ptr -> nx_websocket_client_connection_status_callback(client_ptr, client_ptr -> nx_websocket_client_connection_context, NX_SUCCESS);
        }
    }
    else
    {
        client_ptr -> nx_websocket_client_state = NX_WEBSOCKET_CLIENT_STATE_IDLE;
    }

    /* Clean up unused resources. */
    _nx_websocket_client_cleanup(client_ptr);

    return (status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_websocket_client_connection_status_callback_set                */
/*                                                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the WebSocket callback set.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    context                               Context for callback          */ 
/*    connection_status_callback            Routine to call when connect  */ 
/*                                            or disconnect occurs        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_websocket_client_connection_status_callback_set                 */
/*                                          Actual websocket callback set */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nxe_websocket_client_connection_status_callback_set(NX_WEBSOCKET_CLIENT *client_ptr, VOID *context,
                                                           VOID (*connection_status_callback)(NX_WEBSOCKET_CLIENT *, VOID *, UINT))
{
UINT        status;


    /* Check for invalid input pointers.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_websocket_client_id != NX_WEBSOCKET_CLIENT_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual process function.  */
    status = _nx_websocket_client_connection_status_callback_set(client_ptr, context, connection_status_callback);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_connection_status_callback_set PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets Websocket connection callback.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
/*    context                               Context for callback          */ 
/*    connection_status_callback            Routine to call when connect  */ 
/*                                            or disconnect occurs        */
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
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_websocket_client_connection_status_callback_set(NX_WEBSOCKET_CLIENT *client_ptr, VOID *context,
                                                          VOID (*connection_status_callback)(NX_WEBSOCKET_CLIENT *, VOID *, UINT))
{

    /* Obtain the mutex. */
    tx_mutex_get(&(client_ptr -> nx_websocket_client_mutex), NX_WAIT_FOREVER);

    /* Set the context which will be passed to connection status callback.  */
    client_ptr -> nx_websocket_client_connection_context = context;

    /* Set the connection status callback.  */
    client_ptr -> nx_websocket_client_connection_status_callback = connection_status_callback;

    /* Release the mutex */
    tx_mutex_put(&(client_ptr -> nx_websocket_client_mutex));

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_websocket_client_cleanup                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Bo Chen, Microsoft Corporation                                      */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleanups resources.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            Pointer to WebSocket Client   */
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
/*    _nx_websocket_client_delete           Delete websocket instance     */
/*    _nx_websocket_client_data_process     Process data frame            */
/*    _nx_websocket_client_connect_response_check                         */
/*                                          Check connect response        */
/*    _nx_websocket_client_connect_internal Make websocket connection     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
void  _nx_websocket_client_cleanup(NX_WEBSOCKET_CLIENT *client_ptr)
{

    /* Reset the flag for frame header found */
    client_ptr -> nx_websocket_client_frame_header_found = NX_FALSE;

    /* Release the waiting list.  */
    if (client_ptr -> nx_websocket_client_processing_packet)
    {
        nx_packet_release(client_ptr -> nx_websocket_client_processing_packet);
        client_ptr -> nx_websocket_client_processing_packet = NX_NULL;
    }
}
