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
#define NX_SOURCE_CODE


#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_packet.h"
#include "nx_tcp.h"
#include "nx_http_proxy_client.h"

#ifdef NX_ENABLE_HTTP_PROXY
#define NX_HTTP_CRLF       "\r\n"
#define NX_HTTP_CRLF_SIZE  2

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_http_proxy_client_enable                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP Proxy enable call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    proxy_server_ip                       IP address of proxy server    */
/*    proxy_server_port                     Port of proxy server          */
/*    username                              Pointer to username           */
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */
/*    password_length                       Length of password            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_http_proxy_client_enable          Actual HTTP Proxy enable call */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nxe_http_proxy_client_enable(NX_IP *ip_ptr, NXD_ADDRESS *proxy_server_ip, UINT proxy_server_port,
                                   UCHAR *username, UINT username_length, UCHAR *password, UINT password_length)
{
UINT        status;

    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (proxy_server_ip == NX_NULL) || (proxy_server_port == 0) ||
        ((username == NX_NULL) && (username_length != 0)) ||
        ((password == NX_NULL) && (password_length != 0)))
    {
        return(NX_PTR_ERROR);
    }

    /* Check the length of username and password.  */
    if ((username_length > NX_HTTP_PROXY_MAX_USERNAME) ||
        (password_length > NX_HTTP_PROXY_MAX_PASSWORD))
    {
        return(NX_SIZE_ERROR);
    }

    /* Check that the server IP address version is either IPv4 or IPv6. */
    if ((proxy_server_ip -> nxd_ip_version != NX_IP_VERSION_V4) &&
        (proxy_server_ip -> nxd_ip_version != NX_IP_VERSION_V6))
    {

        return(NX_IP_ADDRESS_ERROR);
    }

#ifndef NX_DISABLE_IPV4
    /* Check for a valid server IP address if the server_ip is version IPv4.  */
    if (proxy_server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        if (((proxy_server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_A_MASK) != NX_IP_CLASS_A_TYPE) &&
            ((proxy_server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_B_MASK) != NX_IP_CLASS_B_TYPE) &&
            ((proxy_server_ip -> nxd_ip_address.v4 & NX_IP_CLASS_C_MASK) != NX_IP_CLASS_C_TYPE))
        {
            return(NX_IP_ADDRESS_ERROR);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

    /* Check for an invalid port.  */
    if (((ULONG)proxy_server_port) > (ULONG)NX_MAX_PORT)
    {
        return(NX_INVALID_PORT);
    }

    /* Call actual HTTP proxy enable function.  */
    status = _nx_http_proxy_client_enable(ip_ptr, proxy_server_ip, proxy_server_port, username,
                                          username_length, password, password_length);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_http_proxy_client_enable                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the HTTP Proxy and sets the information of    */
/*    the HTTP Proxy server.                                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    proxy_server_ip                       IP address of proxy server    */
/*    proxy_server_port                     Port of proxy server          */
/*    username                              Pointer to username           */
/*    username_length                       Length of username            */
/*    password                              Pointer to password           */
/*    password_length                       Length of password            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    COPY_IPV6_ADDRESS                     Copy IPv6 address             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_http_proxy_client_enable(NX_IP *ip_ptr, NXD_ADDRESS *proxy_server_ip, UINT proxy_server_port,
                                  UCHAR *username, UINT username_length, UCHAR *password, UINT password_length)
{

UINT status;
UCHAR string[NX_HTTP_PROXY_MAX_USERNAME + NX_HTTP_PROXY_MAX_PASSWORD + 2];

    /* Set the IP address of HTTP proxy.  */
    ip_ptr -> nx_ip_http_proxy_ip_address.nxd_ip_version = proxy_server_ip -> nxd_ip_version;

#ifndef NX_DISABLE_IPV4
    if (proxy_server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Copy the IPv4 address */
        ip_ptr -> nx_ip_http_proxy_ip_address.nxd_ip_address.v4 = proxy_server_ip -> nxd_ip_address.v4;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (proxy_server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Copy the IPv6 address */
        COPY_IPV6_ADDRESS(proxy_server_ip -> nxd_ip_address.v6,
                          ip_ptr -> nx_ip_http_proxy_ip_address.nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */
    
    /* Set the port of HTTP proxy.  */
    ip_ptr -> nx_ip_http_proxy_port = (USHORT)proxy_server_port;

    /* Determine if basic authentication is required.  */
    if ((username_length) && (password_length))
    {

        /* Encode the "name:password" into authentication buffer.  */

        /* Place the name and password in a single string.  */

        /* Copy the name into the merged string.  */
        memcpy(string, username, username_length); /* Use case of memcpy is verified. */

        /* Insert the colon.  */
        string[username_length] =  ':';

        /* Copy the password into the merged string.  */
        memcpy(&string[username_length + 1], password, password_length); /* Use case of memcpy is verified. */

        /* Make combined string NULL terminated.  */
        string[username_length + password_length + 1] =  NX_NULL;

        /* Now encode the username:password string.  */
        status = _nx_utility_base64_encode((UCHAR *)string, username_length + password_length + 1,
                                           (UCHAR *)ip_ptr -> nx_ip_http_proxy_authentication,
                                           sizeof(ip_ptr -> nx_ip_http_proxy_authentication),
                                           &(ip_ptr -> nx_ip_http_proxy_authentication_length));

        /* Check status.  */
        if (status)
        {
            return(status);
        }
    }
    else
    {
        ip_ptr -> nx_ip_http_proxy_authentication_length = 0;
    }

    /* Set HTTP proxy as enabled.  */
    ip_ptr -> nx_ip_http_proxy_enable = NX_TRUE;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_http_proxy_client_initialize                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the state of HTTP Proxy and stores the    */
/*    the information of remote server.                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    server_ip                             IP address of remote server   */
/*    server_port                           Port of remote server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    COPY_IPV6_ADDRESS                     Copy IPv6 address             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_tcp_client_socket_connect        Connect TCP client socket     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
VOID _nx_http_proxy_client_initialize(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS **server_ip, UINT *server_port)
{

NX_IP *ip_ptr = socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Store the server's IP and port for sending CONNECT request later.  */
    socket_ptr -> nx_tcp_socket_original_server_ip.nxd_ip_version = (*server_ip) -> nxd_ip_version;

#ifndef NX_DISABLE_IPV4
    if ((*server_ip) -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Copy the IPv4 address */
        socket_ptr -> nx_tcp_socket_original_server_ip.nxd_ip_address.v4 = (*server_ip) -> nxd_ip_address.v4;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if ((*server_ip) -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Copy the IPv6 address */
        COPY_IPV6_ADDRESS((*server_ip) -> nxd_ip_address.v6,
                          socket_ptr -> nx_tcp_socket_original_server_ip.nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */

    socket_ptr -> nx_tcp_socket_original_server_port = *server_port;

    /* Replace the peer info with HTTP proxy's IP and port.  */
    *server_ip = &(ip_ptr -> nx_ip_http_proxy_ip_address);
    *server_port = ip_ptr -> nx_ip_http_proxy_port;

    /* Initialize the state.  */
    socket_ptr -> nx_tcp_socket_http_proxy_state = NX_HTTP_PROXY_STATE_INIT;
    socket_ptr -> nx_tcp_socket_http_proxy_header_packet = NX_NULL;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_http_proxy_client_connect                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends HTTP request to connect with HTTP Proxy server. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_disconnect             Disconnect TCP socket         */
/*    _nx_utility_uint_to_string            Convert integer to string     */
/*    _nx_utility_base64_encode             Base64 encode                 */
/*    _nx_packet_allocate                   Allocate packet               */
/*    _nx_packet_data_append                Append packet data            */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_state_syn_received     Process SYN RECEIVED state    */
/*    _nx_tcp_socket_state_syn_sent         Process SYN SENT state        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_http_proxy_client_connect(NX_TCP_SOCKET *socket_ptr)
{

UINT status;
NX_PACKET *packet_ptr;
NX_IP *ip_ptr = socket_ptr -> nx_tcp_socket_ip_ptr;
NX_PACKET_POOL *pool_ptr = ip_ptr -> nx_ip_default_packet_pool;
CHAR *ip_string_ptr;
UINT ip_string_len = 0;
CHAR *port_string_ptr;
UINT port_string_len = 0;
UINT temp_length = 0;
NXD_ADDRESS *server_ip = &(socket_ptr -> nx_tcp_socket_original_server_ip);
UINT port = socket_ptr -> nx_tcp_socket_original_server_port;


    /* Check the IP version of the server.  */
    if (server_ip -> nxd_ip_version != NX_IP_VERSION_V4)
    {

        /* Only IPv4 is supported.  */
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(NX_NOT_SUPPORTED);
    }

    /* Allocate packet.  */
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        status =  _nx_packet_allocate(pool_ptr, &packet_ptr, NX_IPv4_TCP_PACKET, NX_NO_WAIT);
    }
    else
    {
        status =  _nx_packet_allocate(pool_ptr, &packet_ptr, NX_IPv6_TCP_PACKET, NX_NO_WAIT);
    }

    /* Check status.  */
    if (status)
    {
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(status);
    }

    /* Convert IP address and port to string.  */
    /* Use the buffer before prepend pointer to store the IP and port string.  */
    ip_string_ptr = (CHAR *)packet_ptr -> nx_packet_data_start;
    temp_length = (UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start);

    /* Convert IP address to string.  */
    ip_string_len = _nx_utility_uint_to_string((UCHAR)(server_ip -> nxd_ip_address.v4 >> 24), 10, ip_string_ptr, temp_length);
    ip_string_ptr[ip_string_len++] = '.';
    ip_string_len += _nx_utility_uint_to_string((UCHAR)(server_ip -> nxd_ip_address.v4 >> 16), 10, ip_string_ptr + ip_string_len, temp_length - ip_string_len);
    ip_string_ptr[ip_string_len++] = '.';
    ip_string_len += _nx_utility_uint_to_string((UCHAR)(server_ip -> nxd_ip_address.v4 >> 8), 10, ip_string_ptr + ip_string_len, temp_length - ip_string_len);
    ip_string_ptr[ip_string_len++] = '.';
    ip_string_len += _nx_utility_uint_to_string((UCHAR)(server_ip -> nxd_ip_address.v4), 10, ip_string_ptr + ip_string_len, temp_length - ip_string_len);

    /* Convert port to string.  */
    port_string_ptr = ip_string_ptr + ip_string_len;
    port_string_len = _nx_utility_uint_to_string(port, 10, ip_string_ptr + ip_string_len, temp_length - ip_string_len);

    /* HTTP proxy opening handshake message format:
       CONNECT IP address:port HTTP/1.1
       Host: IP address
    */

    /* Build the CONNECT request: CONNECT + Request URI(IP address:port) + HTTP version.  */
    status = _nx_packet_data_append(packet_ptr, "CONNECT ", sizeof("CONNECT ") - 1, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, ip_string_ptr, ip_string_len, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, ":", sizeof(":") - 1, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, port_string_ptr, port_string_len, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, " HTTP/1.1", sizeof(" HTTP/1.1") - 1, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, NX_HTTP_CRLF, NX_HTTP_CRLF_SIZE, pool_ptr, NX_NO_WAIT);

    /* Place the Host in the header.  */
    status += _nx_packet_data_append(packet_ptr, "Host: ", sizeof("Host: ") - 1, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, ip_string_ptr, ip_string_len, pool_ptr, NX_NO_WAIT);
    status += _nx_packet_data_append(packet_ptr, NX_HTTP_CRLF, NX_HTTP_CRLF_SIZE, pool_ptr, NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {
        _nx_packet_release(packet_ptr);
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Determine if basic authentication is required.  */
    if (ip_ptr -> nx_ip_http_proxy_authentication_length)
    {

        /* Yes, attempt to build basic authentication.  */
        status = _nx_packet_data_append(packet_ptr, "Proxy-authorization: Basic ", 27, pool_ptr, NX_NO_WAIT);

        /* Check status.  */
        if (status)
        {
            _nx_packet_release(packet_ptr);
            _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
            return(status);
        }

        /* Append authentication string.  */
        status = _nx_packet_data_append(packet_ptr, ip_ptr -> nx_ip_http_proxy_authentication,
                                        ip_ptr -> nx_ip_http_proxy_authentication_length, pool_ptr, NX_NO_WAIT);
        status += _nx_packet_data_append(packet_ptr, NX_HTTP_CRLF, NX_HTTP_CRLF_SIZE, pool_ptr, NX_NO_WAIT);

        /* Check status.  */
        if (status)
        {
            _nx_packet_release(packet_ptr);
            _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* Append CRLF.  */
    status = _nx_packet_data_append(packet_ptr, NX_HTTP_CRLF, NX_HTTP_CRLF_SIZE, pool_ptr, NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {
        _nx_packet_release(packet_ptr);
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(status);
    }

    /* Send out the packet.  */
    status = _nx_tcp_socket_send(socket_ptr, packet_ptr, NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {
        _nx_packet_release(packet_ptr);
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(status);
    }

    /* Update HTTP Proxy state.  */
    socket_ptr -> nx_tcp_socket_http_proxy_state = NX_HTTP_PROXY_STATE_CONNECTING;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_http_proxy_client_connect_response_process      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the CONNECT response from HTTP Proxy server.*/
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_receive                Receive TCP socket            */
/*    _nx_tcp_socket_disconnect             Disconnect TCP socket         */
/*    _nx_tcp_socket_thread_resume          Resume the suspended thread   */
/*    _nx_packet_data_append                Append packet data            */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_packet_process         Process socket packet         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_http_proxy_client_connect_response_process(NX_TCP_SOCKET *socket_ptr)
{
NX_PACKET *head_packet_ptr = socket_ptr -> nx_tcp_socket_http_proxy_header_packet;
NX_PACKET *new_packet_ptr;
CHAR *buffer_ptr;
UINT status = NX_SUCCESS;
NX_PACKET *work_ptr;
UINT crlf_found = 0;
NX_PACKET *tmp_ptr;
NX_IP *ip_ptr = socket_ptr -> nx_tcp_socket_ip_ptr;
NX_PACKET_POOL *pool_ptr = ip_ptr -> nx_ip_default_packet_pool;


    if (head_packet_ptr == NX_NULL)
    {

        /* Wait for a response from HTTP proxy.  */
        status = _nx_tcp_socket_receive(socket_ptr, &head_packet_ptr, NX_NO_WAIT);

        /* Check the return status.  */
        if (status == NX_NO_PACKET)
        {
            return(NX_IN_PROGRESS);
        } 
        else if (status != NX_SUCCESS)
        {

            /* Return an error condition.  */
            _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
            return(status);
        }

        socket_ptr -> nx_tcp_socket_http_proxy_header_packet = head_packet_ptr;
    }

    crlf_found = 0;
    work_ptr = head_packet_ptr;
    
    /* Build a pointer to the buffer area.  */
    buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr;
    
    do
    {
    
        /* See if there is a blank line present in the buffer.  */
        /* Search the buffer for a cr/lf pair.  */
        while ((buffer_ptr < (CHAR *) work_ptr -> nx_packet_append_ptr) &&
               (crlf_found < 4))
        {
            if (!(crlf_found & 1) && (*buffer_ptr == (CHAR)13))
            {

                /* Found CR.  */
                crlf_found++;
            }
            else if((crlf_found & 1) && (*buffer_ptr == (CHAR)10))
            {

                /* Found LF.  */
                crlf_found++;
            }
            else
            {

                /* Reset the CRLF marker.  */
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

        /* Determine if the packet has already overflowed into another packet.  */

#ifndef NX_DISABLE_PACKET_CHAIN

        if (work_ptr -> nx_packet_next != NX_NULL)
        {

            /* Get the next packet in the chain.  */
            work_ptr  = work_ptr -> nx_packet_next;
            buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr; 
        }
        else
#endif
        {
            /* Receive another packet from the HTTP proxy.  */
            status = _nx_tcp_socket_receive(socket_ptr, &new_packet_ptr, NX_NO_WAIT);
            
            /* Check the return status.  */
            if (status == NX_NO_PACKET)
            {
                return(NX_IN_PROGRESS);
            }
            else if (status != NX_SUCCESS)
            {
                break;
            }

            /* Successfully received another packet. Its contents now need to be placed in the head packet.  */
            tmp_ptr = new_packet_ptr;

#ifndef NX_DISABLE_PACKET_CHAIN
            while (tmp_ptr)
#endif /* NX_DISABLE_PACKET_CHAIN */
            {

                /* Copy the contents of the current packet into the head packet.  */
                status =  _nx_packet_data_append(head_packet_ptr, (VOID *) tmp_ptr -> nx_packet_prepend_ptr,
                                                (ULONG)(tmp_ptr -> nx_packet_append_ptr - tmp_ptr -> nx_packet_prepend_ptr),
                                                pool_ptr, NX_NO_WAIT);

#ifndef NX_DISABLE_PACKET_CHAIN

                /* Determine if an error occurred.  */
                if (status != NX_SUCCESS)
                {
                    break;
                }
                else
                {
                    tmp_ptr = tmp_ptr -> nx_packet_next;
                }
#endif /* NX_DISABLE_PACKET_CHAIN */
            }

            /* Release the new packet.  */
            _nx_packet_release(new_packet_ptr);
        }

    } while (status == NX_SUCCESS);

    if (status)
    {

        /* Return an error condition.  */
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(status);
    }

    /* Check the packet length and response code.  */
    if ((head_packet_ptr -> nx_packet_append_ptr - head_packet_ptr -> nx_packet_prepend_ptr < 12) ||
        (*(head_packet_ptr -> nx_packet_prepend_ptr + 9) != '2'))
    {

        /* If error occurs, disconnect the TCP connection.  */
        _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Update the HTTP Proxy state.  */
    socket_ptr -> nx_tcp_socket_http_proxy_state = NX_HTTP_PROXY_STATE_CONNECTED;

    /* Release the packet.  */
    _nx_packet_release(head_packet_ptr);
    socket_ptr -> nx_tcp_socket_http_proxy_header_packet = NX_NULL;

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

    /* Is a connection completion callback registered with the TCP socket?  */
    if (socket_ptr -> nx_tcp_establish_notify)
    {

        /* Call the application's establish callback function.  */
        (socket_ptr -> nx_tcp_establish_notify)(socket_ptr);
    }
#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */

    /* Determine if we need to wake a thread suspended on the connection.  */
    if (socket_ptr -> nx_tcp_socket_connect_suspended_thread)
    {

        /* Resume the suspended thread.  */
        _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_connect_suspended_thread), NX_SUCCESS);
    }

    /* Return status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_http_proxy_client_cleanup                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Wenhui Xie, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the HTTP Proxy by resetting the state and   */
/*    release the processing packet.                                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_block_cleanup          Clean up the socket block     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Wenhui Xie               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
VOID _nx_http_proxy_client_cleanup(NX_TCP_SOCKET *socket_ptr)
{

    /* Clear the HTTP Proxy connection state.  */
    socket_ptr -> nx_tcp_socket_http_proxy_state = NX_HTTP_PROXY_STATE_INIT;

    /* Release HTTP Proxy header packet.  */
    if (socket_ptr -> nx_tcp_socket_http_proxy_header_packet)
    {
        _nx_packet_release(socket_ptr -> nx_tcp_socket_http_proxy_header_packet);
        socket_ptr -> nx_tcp_socket_http_proxy_header_packet = NX_NULL;
    }
}
#endif /* NX_ENABLE_HTTP_PROXY */