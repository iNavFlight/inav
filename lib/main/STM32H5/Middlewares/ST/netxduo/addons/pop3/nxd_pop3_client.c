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
/** NetX POP3 Client Component                                            */
/**                                                                       */
/**   Post Office Protocol Version 3 (POP3)                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_pop3_client.c                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Post Office Protocol (POP3)              */
/*    Client component, including all data types and external references. */
/*    It is assumed that tx_api.h, tx_port.h, nx_api.h, and nx_port.h,    */
/*    have already been included.                                         */
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

#define NX_POP3_CLIENT_SOURCE_CODE

/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING    
#endif


#include    "nx_api.h"
#include    "nx_md5.h"
#include    "nx_ip.h"
#include    "nxd_pop3_client.h"
#include    "nx_ipv6.h"
#include    <ctype.h>

/* The following array of state handlers is indexed by the state.  */

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the create POP3 client     */ 
/*    service (IPv4 only).                                                */
/*                                                                        */ 
/*    Note: The string lengths of client_name and client_password are     */
/*    limited by internal buffer size.                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    APOP_authentication               1=enable APOP; 0=disable APOP     */
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    server_ip_address                 POP3 server IP address            */
/*    server_port                       POP3 server port                  */
/*    client_name                       Client POP3 user name             */
/*    client_password                   Client POP3 password              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_create             Creates the POP3 client          */ 
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
UINT  _nxe_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, 
                              NX_PACKET_POOL *packet_pool_ptr, ULONG server_ip_address, ULONG server_port, CHAR *client_name, 
                              CHAR *client_password)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Check for valid input pointers.  */
    if ((ip_ptr == NX_NULL) || (client_ptr == NX_NULL) || (packet_pool_ptr == NX_NULL) || (client_name == NX_NULL) ||
        (client_password == NX_NULL))   
    {

       /* Return pointer error.  */
       return(NX_PTR_ERROR);
    }

    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || (server_ip_address == 0) || (server_port == 0))
    {
    
        return NX_POP3_PARAM_ERROR;
    }

    /* Call the actual client create service.  */
    status =  _nx_pop3_client_create(client_ptr, 
                                     APOP_authentication,  
                                     ip_ptr, packet_pool_ptr,
                                     server_ip_address, server_port, client_name, 
                                     client_password);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(APOP_authentication);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);
    NX_PARAMETER_NOT_USED(server_ip_address);
    NX_PARAMETER_NOT_USED(server_port);
    NX_PARAMETER_NOT_USED(client_name);
    NX_PARAMETER_NOT_USED(client_password);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_create                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a POP3 Client on the specified IP instance.   */
/*                                                                        */ 
/*    Note: The string lengths of client_name and client_password are     */
/*    limited by internal buffer size.                                    */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    APOP_authentication               1=enable APOP; 0=disable APOP     */
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    server_ip_address                 POP3 server IP address            */
/*    server_port                       POP3 server port                  */
/*    client_name                       Client POP3 user name             */
/*    client_password                   Client POP3 password              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_pop3_client_create           Duo Client POP3 create service    */ 
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
UINT  _nx_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, 
                              NX_PACKET_POOL *packet_pool_ptr, ULONG server_ip_address, ULONG server_port, CHAR *client_name, 
                              CHAR *client_password)
{

#ifndef NX_DISABLE_IPV4
UINT status;
NXD_ADDRESS server_address;

    server_address.nxd_ip_address.v4 = server_ip_address;
    server_address.nxd_ip_version = NX_IP_VERSION_V4;

    status =  _nxd_pop3_client_create(client_ptr, 
                                     APOP_authentication,  
                                     ip_ptr, packet_pool_ptr,
                                     &server_address, server_port, client_name, 
                                     client_password);

    return status;
#else
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(APOP_authentication);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(packet_pool_ptr);
    NX_PARAMETER_NOT_USED(server_ip_address);
    NX_PARAMETER_NOT_USED(server_port);
    NX_PARAMETER_NOT_USED(client_name);
    NX_PARAMETER_NOT_USED(client_password);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_pop3_client_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the create POP3 client     */ 
/*    service (supports IPv6 and IPv4).                                   */
/*                                                                        */
/*    Note: The string lengths of client_name and client_password are     */
/*    limited by internal buffer size.                                    */
/*                                                                        */  
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    APOP_authentication               1=enable APOP; 0=disable APOP     */
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    server_duo_address                POP3 server IPv6/IPv4 address     */
/*    server_port                       POP3 server port                  */
/*    client_name                       Client POP3 user name             */
/*    client_password                   Client POP3 password              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_pop3_client_create            Creates the POP3 client          */ 
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
UINT  _nxde_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, 
                              NX_PACKET_POOL *packet_pool_ptr, NXD_ADDRESS *server_ip_address, ULONG server_port, CHAR *client_name, 
                              CHAR *client_password)
{

UINT status;


    /* Check for valid input pointers.  */
    if ((ip_ptr == NX_NULL) || (client_ptr == NX_NULL) || (packet_pool_ptr == NX_NULL) || (server_ip_address == NX_NULL) ||
        (client_name == NX_NULL) || (client_password == NX_NULL))   
    {

       /* Return pointer error.  */
       return(NX_PTR_ERROR);
    }

    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || (server_port == 0))
    {
    
        return NX_POP3_PARAM_ERROR;
    }

    /* Call the actual client create service.  */
    status =  _nxd_pop3_client_create(client_ptr, 
                                     APOP_authentication,  
                                     ip_ptr, packet_pool_ptr,
                                     server_ip_address, server_port, client_name, 
                                     client_password);

    /* Return completion status.  */
    return(status);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_pop3_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a POP3 Client on the specified IP instance.   */
/*    This supports both IPv6 and IPv4 POP3 server connections.           */
/*                                                                        */   
/*    Note: The string lengths of client_name and client_password are     */
/*    limited by internal buffer size.                                    */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    APOP_authentication               1=enable APOP; 0=disable APOP     */
/*    ip_ptr                            Pointer to client IP instance     */ 
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    server_ip_address                 POP3 server IP address            */
/*    server_port                       POP3 server port                  */
/*    client_name                       Client POP3 user name             */
/*    client_password                   Client POP3 password              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_create              NetX TCP socket create service    */ 
/*    _nxd_pop3_client_connect          Connect POP3 Client to Server     */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nxd_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, 
                              NX_PACKET_POOL *packet_pool_ptr, NXD_ADDRESS *server_ip_address, ULONG server_port, CHAR *client_name, 
                              CHAR *client_password)

{

UINT status;
UINT client_name_length;
UINT client_password_length;


    client_ptr -> nx_pop3_client_ready_to_download = NX_FALSE;

    if (_nx_utility_string_length_check(client_name, &client_name_length, NX_POP3_MAX_USERNAME) ||
        _nx_utility_string_length_check(client_password, &client_password_length, NX_POP3_MAX_PASSWORD))
    {

        return NX_POP3_PARAM_ERROR;
    }
        
    /* Null the members of NX_POP3_CLIENT.  */
    memset(client_ptr, 0, sizeof(NX_POP3_CLIENT));

    /* Configure Client identification.  */
    memset(client_ptr -> nx_pop3_client_name, 0, NX_POP3_MAX_USERNAME);
    memcpy(client_ptr -> nx_pop3_client_name, client_name, client_name_length); /* Use case of memcpy is verified. */
    memset(client_ptr -> nx_pop3_client_password, 0, NX_POP3_MAX_PASSWORD);
    memcpy(client_ptr -> nx_pop3_client_password, client_password, client_password_length); /* Use case of memcpy is verified. */

   /* Configure Client POP3 authentication options.  */
    client_ptr -> nx_pop3_client_enable_APOP_authentication = APOP_authentication;

    /* Create a tcp socket to send/receive POP3 data.  */
    status =  nx_tcp_socket_create(ip_ptr, &client_ptr -> nx_pop3_client_tcp_socket, "POP3 Client socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 
                                   NX_POP3_CLIENT_TCP_WINDOW_SIZE,
                                   NX_NULL, NX_NULL);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Configure Client NetX and TCP/IP options.  */
    client_ptr -> nx_pop3_client_packet_pool_ptr =  packet_pool_ptr;

    status = _nxd_pop3_client_connect(client_ptr, server_ip_address, server_port);

    /* Return successful completion status.  */
    return(status);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the POP3 Client delete     */ 
/*    service.                                                            */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*    status                            Actual completion status          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_delete            Actual Client delete service      */ 
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
UINT  _nxe_pop3_client_delete(NX_POP3_CLIENT *client_ptr)
{

UINT status;


    /* Check for valid input parameter.  */
    if (client_ptr == NX_NULL)
    {

        /* Return error status.  */
        return(NX_PTR_ERROR);
    }

    /* Call client create function.  */
    status =  _nx_pop3_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_delete                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified POP3 Client.                    */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    status                            Actual completion status          */
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_disconnect          Closes the TCP connection         */
/*    nx_tcp_client_socket_unbind       Releases (unbinds) the TCP port   */
/*    nx_tcp_socket_delete              Deletes unbound TCP socket        */
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
UINT  _nx_pop3_client_delete(NX_POP3_CLIENT *client_ptr)
{

    if (client_ptr -> nx_pop3_client_message_ptr)
    {
        nx_packet_release(client_ptr -> nx_pop3_client_message_ptr);
    }

    /* Yes, so disconnect client socket from server.  */
    nx_tcp_socket_disconnect(&client_ptr -> nx_pop3_client_tcp_socket, NX_POP3_CLIENT_DISCONNECT_TIMEOUT);   

    /* Unbind the port from client socket.  */
    nx_tcp_client_socket_unbind(&client_ptr -> nx_pop3_client_tcp_socket);

    /* Release client socket.  */
    nx_tcp_socket_delete(&client_ptr -> nx_pop3_client_tcp_socket);        

    /* Clear the members of NX_POP3_CLIENT.  */
    memset(client_ptr, 0, sizeof(NX_POP3_CLIENT));

    /* Return completion status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_mail_items_get                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the get mail items in     */
/*    client mailbox service.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    number_mail_items                 Pointer to items in mailbox       */
/*    maildrop_total_size               Pointer to total mailbox size     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_pop3_client_mail_items_get   Actual get mail items service     */ 
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
UINT _nxe_pop3_client_mail_items_get(NX_POP3_CLIENT *client_ptr, UINT *number_mail_items, ULONG *maildrop_total_size)
{

UINT status;


        /* Check for invalid pointer inpu. */
        if ((client_ptr == NX_NULL) || (number_mail_items == NX_NULL) || (maildrop_total_size == NX_NULL))
        {

            return NX_PTR_ERROR;
        }

        status = _nx_pop3_client_mail_items_get(client_ptr, number_mail_items, maildrop_total_size);

        return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_items_get                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the number of items and total size in bytes */
/*    of mail data in the Client's mailbox.                               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    number_mail_items                 Pointer to items in mailbox       */
/*    maildrop_total_size               Pointer to total mailbox size     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                       Packet too small for command     */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_mail_items_get(NX_POP3_CLIENT *client_ptr, UINT *number_mail_items, ULONG *maildrop_total_size)
{

UINT         status;
NX_PACKET    *packet_ptr, *recv_packet_ptr;
CHAR         *buffer;
CHAR         argument[10];  
UINT         packet_type;

    /* Determine type of packet to allocate. */
    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    /* Check for sufficient packet buffer size for command. */
    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_STAT) - 1) + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Send the STAT command. */
    buffer = (CHAR *)(packet_ptr -> nx_packet_prepend_ptr);

    memcpy(buffer, NX_POP3_COMMAND_STAT, (sizeof(NX_POP3_COMMAND_STAT) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length = (sizeof(NX_POP3_COMMAND_STAT) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_STAT) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_STAT) - 1);

    memcpy(buffer, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Clear existing Client maildrop data.  */
    client_ptr -> nx_pop3_client_total_message_size = 0;
    client_ptr -> nx_pop3_client_maildrop_items = 0;

    /* Receive server reply.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to server error received.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* Yes. Clear memory for parsing the mail item count.  */
        memset(argument, 0, 5);

        /* Get the number of messages.  */
        _nx_pop3_parse_response(buffer, 2, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

        /* Check if argument parsed successfully.  */
        if ((argument[0] >= 0x30) && (argument[0] <= 0x39))
        {

            /* It was; update session maildrop items.  */
            client_ptr -> nx_pop3_client_maildrop_items = strtoul(argument, NULL, 10); 

            /* Get total mail message data next.  */
            memset(argument, 0, 5);

            /* Get the size in bytes of message data.  */
            _nx_pop3_parse_response(buffer, 3, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

            /* Check if argument parsed successfully.   */
            if ((argument[0] >= 0x30) && (argument[0] <= 0x39))
            {
                        
                /* It was; update total message data. */
                client_ptr -> nx_pop3_client_total_message_size = strtoul(argument, NULL, 10); 

                status = NX_SUCCESS;
            }
        }
    }

    *maildrop_total_size = client_ptr -> nx_pop3_client_total_message_size;
    *number_mail_items = client_ptr -> nx_pop3_client_maildrop_items;

    nx_packet_release(recv_packet_ptr);

    /* Return completion status.  */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_get_mail_item                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the get mail item size    */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_item                         Index into POP3 mailbox           */
/*    item_size                         Pointer to mail size              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INVALID_MAIL_ITEM         Invalid mail index input          */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_get_mail_size     Actual get mail size service      */ 
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
UINT _nxe_pop3_client_mail_item_size_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *size)
{

UINT status;


    /* Check for invalid pointer input. */
    if ((client_ptr == NX_NULL) || (size == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    /* Check for an invalid index. */
    if (mail_item == 0)
    {
        return NX_POP3_CLIENT_INVALID_INDEX;
    }

    status = _nx_pop3_client_mail_item_size_get(client_ptr, mail_item, size);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_size_get                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a LIST command for the specified mail item and  */
/*    processes the server response for size of the requested item. Note  */
/*    that there is considerable discrepancy between reported size and    */
/*    actual size, with the reported size usually 15% or more larger than */
/*    actual.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_item                         Index into POP3 mailbox           */
/*    size                              Pointer to mail item size         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INVALID_MAIL_ITEM         Invalid mail index input          */
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                      Packet too small for command      */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
/*    _nx_pop3_server_number_convert    Convert integer to ascii          */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_mail_item_size_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *size)
{

NX_PACKET *packet_ptr, *recv_packet_ptr; 
CHAR      *buffer_ptr;
UINT      num_size;
UINT      status;
CHAR      argument[10]; 
UINT      packet_type;


    /* Initialize mail box to having zero items. */
    *size = 0;

    /* Verify mail_item is valid index (less than total number items in mailbox. */
    if (mail_item > client_ptr -> nx_pop3_client_maildrop_items)
    {
        return NX_POP3_INVALID_MAIL_ITEM;
    }

    /* Determine which packet type to allocate. */
    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    num_size = _nx_pop3_server_number_convert(mail_item, &argument[0]);

    /* Determine if the packet payload is large enough for LIST command. */
    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_LIST) - 1) + 1 + 
         num_size + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    buffer_ptr = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;

    /* Send LIST mail_item query to server */
    memcpy(buffer_ptr, NX_POP3_COMMAND_LIST, (sizeof(NX_POP3_COMMAND_LIST) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length = (sizeof(NX_POP3_COMMAND_LIST) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_LIST) - 1);
    buffer_ptr += (sizeof(NX_POP3_COMMAND_LIST) - 1);

    memcpy(buffer_ptr, " ", 1); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length++;
    packet_ptr -> nx_packet_append_ptr++;
    buffer_ptr++;

    memcpy(buffer_ptr,  &argument[0], num_size); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += num_size;
    packet_ptr -> nx_packet_append_ptr += num_size;
    buffer_ptr += num_size;

    memcpy(buffer_ptr, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    buffer_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply over session socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer_ptr = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Initialize status to bad reply error condition.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer_ptr, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* Clear memory for parsing the mail item count.  */
        memset(argument, 0, 5);

        /* Extact the message size.  */
        _nx_pop3_parse_response(buffer_ptr, 2, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

        /* Check if argument parsed successfully.  */
        if ((argument[0] >= 0x30) && (argument[0] <= 0x39))
        {

            /* It was; verify it matches the input mail index.  */
            UINT server_maildrop_items = strtoul(argument, NULL, 10); 

            if (mail_item == server_maildrop_items)
            {

                /* It does. Parse the message size.  */
                _nx_pop3_parse_response(buffer_ptr, 3, recv_packet_ptr -> nx_packet_length, 
                                        (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

                if ((argument[0] >= 0x30) && (argument[0] <= 0x39))
                {

                    *size = strtoul(argument, NULL, 10); 

                    status = NX_SUCCESS;
                }
            }
        }
    }

    /* Done with the server response, delete the packet. */
    nx_packet_release(recv_packet_ptr);
    
    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_mail_item_get                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the get mail item         */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_item                         Index into POP3 mailbox           */
/*    item_size                         Pointer to mail size              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_get     Actual get mail service           */ 
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
UINT _nxe_pop3_client_mail_item_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *item_size)
{

UINT status;

    /* Check for invalid pointer input. */
    if ((client_ptr == NX_NULL) || (item_size == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    /* Check for an invalid index. */
    if (mail_item == 0)
    {
        return NX_POP3_CLIENT_INVALID_INDEX;
    }

    status = _nx_pop3_client_mail_item_get(client_ptr, mail_item, item_size);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_get                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends the RETR command to the server for the specified*/
/*    mail item, and returns the server response. If accepted, it will    */
/*    parse the mail item size as well.                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_item                         Index into POP3 mailbox           */
/*    item_size                         Pointer to mail size              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INVALID_MAIL_ITEM         Invalid mail index input          */
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                      Packet too small for command      */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
/*    _nx_pop3_server_number_convert    Convert integer to ascii          */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_mail_item_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *item_size)
{

UINT         status;
UINT         size;
NX_PACKET    *packet_ptr, *recv_packet_ptr;
CHAR         *buffer;
CHAR         argument[10];
UINT         packet_type;


     client_ptr -> nx_pop3_client_ready_to_download = NX_FALSE;

     /* Verify mail_item is valid index (less than total number items in mailbox. */
     if (mail_item > client_ptr -> nx_pop3_client_maildrop_items)
     {
         return NX_POP3_INVALID_MAIL_ITEM;
     }

     /* Allocate a packet.  */
     if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
         packet_type = NX_IPv6_TCP_PACKET;
     else
         packet_type = NX_IPv4_TCP_PACKET;

     /* Allocate a packet.  */
     status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                  &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    /* Check packet payload can hold the Client message. . */
    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_RETR) - 1) + 1) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    buffer = (CHAR *)(packet_ptr -> nx_packet_prepend_ptr);

    /* Send a RETR command to the server. */ 
    memcpy(packet_ptr -> nx_packet_prepend_ptr, NX_POP3_COMMAND_RETR, (sizeof(NX_POP3_COMMAND_RETR) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_RETR) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_RETR) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_RETR) - 1);

    memcpy(buffer, " ", 1); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length++;
    packet_ptr -> nx_packet_append_ptr++;
    buffer++;

    /* Convert the mail item index to ascii. */
    size = _nx_pop3_server_number_convert(mail_item, &argument[0]);
    if ((buffer + size + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)) >= (CHAR *)packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    memcpy(buffer,  &argument[0], size); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += size;
    packet_ptr -> nx_packet_append_ptr += size;
    buffer += size;

    memcpy(buffer, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply over Client socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument,
                            sizeof(argument), NX_FALSE, NX_FALSE);

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        client_ptr -> nx_pop3_client_ready_to_download = NX_TRUE;
        *item_size = 0;

        /* Yes. Clear memory for parsing the mail item size.  */
        memset(argument, 0, sizeof(argument));

        /* Get the number of szie of the message. .  */
        _nx_pop3_parse_response(buffer, 2, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument,
                                sizeof(argument) - 1, NX_FALSE, NX_FALSE);

        /* Check if argument parsed successfully.  */
        if ((argument[0] >= 0x30) && (argument[0] <= 0x39))
        {

            /* It was; set the size of th e mail item.  */
            *item_size = strtoul(argument, NULL, 10);
        }

        while (((ULONG)buffer < ((ULONG)recv_packet_ptr -> nx_packet_append_ptr - 1)) &&
               ((*buffer != 0x0D) || (*(buffer + 1) != 0x0A)))
        {
            buffer++;
        }

        buffer += 2;

        if ((UCHAR *)buffer == recv_packet_ptr -> nx_packet_append_ptr)
        {
            nx_packet_release(recv_packet_ptr);
        }
        else
        {
            client_ptr -> nx_pop3_client_message_ptr = recv_packet_ptr;
            recv_packet_ptr -> nx_packet_length -=
                (ULONG)buffer - (ULONG)recv_packet_ptr -> nx_packet_prepend_ptr;
            recv_packet_ptr -> nx_packet_prepend_ptr = (UCHAR *)buffer;
        }

        return NX_SUCCESS;
    }
    else
    {
        nx_packet_release(recv_packet_ptr);
        return NX_POP3_SERVER_ERROR_STATUS;
    }

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_mail_item_message_get              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the get message data      */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    recv_packet_ptr                   Pointer to received PP3 packet    */
/*    bytes_retrieved                   Size of message data in packet    */
/*    final_packet                      Indicates if last packet          */
/*                                        NX_FALSE = not the last packet  */
/*                                        NX_TRUE  = is the last packet   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_message_data_get                          */
/*                                      Actual get message data service   */ 
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
UINT _nxe_pop3_client_mail_item_message_get(NX_POP3_CLIENT *client_ptr, NX_PACKET **recv_packet_ptr, ULONG *bytes_retrieved, UINT *final_packet)
{

UINT status;


    if ((client_ptr == NX_NULL) || (recv_packet_ptr == NX_NULL) || (bytes_retrieved == NX_NULL) || (final_packet == NX_NULL))
    {

        return NX_PTR_ERROR;
    }

    status = _nx_pop3_client_mail_item_message_get(client_ptr, recv_packet_ptr, bytes_retrieved, final_packet);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_message_get               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves message packets from the POP3 Client TCP    */
/*    socket and returns them to the caller without further processing.   */
/*    It is up to the caller to strip off the trailing end of message tag */
/*    \r\n.\r\n if one is appended to the message. Note the end of        */
/*    message is included in the packet length.                           */
/*                                                                        */ 
/*    When packets are received with only the end of message tag (EOM),   */
/*    this function returns zero bytes retrieved and sets final_packet to */
/*    true.                                                               */
/*                                                                        */ 
/*    If the status return is NX_SUCCESS, the caller MUST release the     */
/*    packet, even if there bytes_retrieved is set to zero.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    recv_packet_ptr                   Pointer to received packet        */
/*    bytes_retrieved                   Received packet length            */
/*    final_packet                      If last packet of message         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    status                             Actual completion status         */ 
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
UINT _nx_pop3_client_mail_item_message_get(NX_POP3_CLIENT *client_ptr, NX_PACKET **recv_packet_ptr, ULONG *bytes_retrieved, UINT *final_packet)
{
UINT    status;
UINT    index; 
CHAR    *buffer_ptr;

    /* Initialize results to no data retrieved, or if the received packet is the last in the message. */
    *final_packet = NX_FALSE;
    *bytes_retrieved = 0;

    /* Verify client is ready to download mail messages. */
    if (client_ptr ->  nx_pop3_client_ready_to_download == NX_FALSE)
    {

        return NX_POP3_CLIENT_INVALID_STATE;
    }

    if (client_ptr -> nx_pop3_client_message_ptr)
    {
        status = NX_SUCCESS;
        *recv_packet_ptr = client_ptr -> nx_pop3_client_message_ptr;
        client_ptr -> nx_pop3_client_message_ptr = NX_NULL;
    }
    else
    {

        /* Retrieve the next data packet. */
        status = nx_tcp_socket_receive(&client_ptr -> nx_pop3_client_tcp_socket, recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);
    }

    if (status == NX_SUCCESS)
    {

        /* Update the bytes_retrieved with amount of data in the packet. */
        *bytes_retrieved = (*recv_packet_ptr) -> nx_packet_length;

        /* Check for end of message tag in this packet. */
        if (*bytes_retrieved > (sizeof(NX_POP3_END_OF_MESSAGE) - 1))
        {
            buffer_ptr = (CHAR *)(*recv_packet_ptr) -> nx_packet_prepend_ptr;
    
            index = *bytes_retrieved - (sizeof(NX_POP3_END_OF_MESSAGE) - 1);
    
            /* Determine if the end of the data contains the terminating \r\n.\r\n marker. */
            if (memcmp((buffer_ptr + index), NX_POP3_END_OF_MESSAGE, (sizeof(NX_POP3_END_OF_MESSAGE) - 1)) == 0)
            {
  
                /* It does; indicate this is the end of the mail download. */
                *final_packet = NX_TRUE;

                /* Client cannot download any data for this particular mail item. */
                client_ptr -> nx_pop3_client_ready_to_download = NX_FALSE;
            }
        }
        else 
        {
          
           if (
                 (memcmp((*recv_packet_ptr) -> nx_packet_prepend_ptr, NX_POP3_END_OF_MESSAGE, (sizeof(NX_POP3_END_OF_MESSAGE) - 1))== 0) ||
                 (memcmp((*recv_packet_ptr) -> nx_packet_prepend_ptr, NX_POP3_END_OF_MESSAGE_TAG, (sizeof(NX_POP3_END_OF_MESSAGE_TAG) - 1))== 0)
              )
           {

               /* Yes, but this is not considered part of the message. Indicate this with the bytes retrieved. */
              *bytes_retrieved = 0;

              /* It does; indicate this is the end of the mail download. */
              *final_packet = NX_TRUE;

              /* Client cannot download any data for this particular mail item. */
              client_ptr -> nx_pop3_client_ready_to_download = NX_FALSE;
           }  
        }
    }

    return status;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_mail_item_delete                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the delete mail item      */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_index                        Index of mail item to delete      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INVALID_MAIL_ITEM         Invalid mail index input          */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_delete  Actual delete mail service        */ 
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
UINT _nxe_pop3_client_mail_item_delete(NX_POP3_CLIENT *client_ptr, UINT mail_index)
{

UINT status;


    /* Check for invalid pointer input. */
    if (client_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* Check for invalid mail item input. */
    if (mail_index == 0)
    {

        return NX_POP3_CLIENT_INVALID_INDEX;
    }

    status = _nx_pop3_client_mail_item_delete(client_ptr, mail_index);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_mail_item_delete                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends the DELE command for the specified mail item    */
/*    and verifies the server will delete the item. Note that for some    */
/*    servers, items marked for deletion are not deleted immediately, in  */
/*    some cases only if they receive the QUIT command.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    mail_item                         Index into POP3 mailbox           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                      Invalid pointer parameter         */ 
/*    NX_POP3_INVALID_MAIL_ITEM         Invalid mail index input          */
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                       Packet too small for command     */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
/*    _nx_pop3_server_number_convert    Convert integer to ascii          */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_mail_item_delete(NX_POP3_CLIENT *client_ptr, UINT mail_index)
{

UINT         status;
UINT         size;
NX_PACKET    *packet_ptr, *recv_packet_ptr;
CHAR         *buffer_ptr;
CHAR         argument[10];
UINT         packet_type;


    /* Allocate a packet.  */
    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    buffer_ptr = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;

    /* Convert the ascii word to a number. */
    size = _nx_pop3_server_number_convert(mail_index, &argument[0]);

    /* Check packet payload is large enough for DEL request.  */
    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_DELE) - 1) + 1 + size +
         (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Send the DELE command. */
    memcpy(buffer_ptr, NX_POP3_COMMAND_DELE, (sizeof(NX_POP3_COMMAND_DELE) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_DELE) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_DELE) - 1);
    buffer_ptr += (sizeof(NX_POP3_COMMAND_DELE) - 1);


    memcpy(buffer_ptr, " ", 1); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length++;
    packet_ptr -> nx_packet_append_ptr++;  
    buffer_ptr++;

    memcpy(buffer_ptr,  &argument[0], size); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += size;
    packet_ptr -> nx_packet_append_ptr += size;
    buffer_ptr += size;

    memcpy(buffer_ptr, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1); 
    buffer_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the DELE command out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply to DELE command.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer_ptr = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer_ptr, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to server error status received.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* Yes, set status to successful completion. */
        status = NX_SUCCESS;
    }

    nx_packet_release(recv_packet_ptr);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_pop3_client_quit                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    The service performs error checking for the send QUIT service.      */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to POP3 Client             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_PTR_ERROR                     Invalid pointer input              */
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Application thread                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_pop3_client_quit             Actual send QUIT command service   */
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
UINT _nxe_pop3_client_quit(NX_POP3_CLIENT *client_ptr)
{

UINT status;

    if (client_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    status = _nx_pop3_client_quit(client_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_quit                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    The service sends a QUIT command to the Client POP3 server.  The    */
/*    QUIT command takes no arguments and can be called at any time during*/
/*    the Client session.                                                 */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                       Pointer to POP3 Client             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                       Successful completion status       */ 
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                     Packet too small for command       */
/*    status                           Actual completion status           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   Application thread                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_pop3_client_quit(NX_POP3_CLIENT *client_ptr)
{

UINT         status;
NX_PACKET    *packet_ptr, *recv_packet_ptr;
CHAR         *buffer_ptr;
CHAR         argument[10];
UINT         packet_type;


    /* Allocate a packet.  */
    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    buffer_ptr = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;

    /* Check packet payload will hold the QUIT request. */
    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_QUIT) - 1) + 1 + 
         (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    memcpy(buffer_ptr, NX_POP3_COMMAND_QUIT, (sizeof(NX_POP3_COMMAND_QUIT) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_QUIT) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_QUIT) - 1);
    buffer_ptr += (sizeof(NX_POP3_COMMAND_QUIT) - 1);

    memcpy(buffer_ptr, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1); 
    buffer_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply over session socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer_ptr = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer_ptr, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to bad reply error condition.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* Yes, set status to successful completion. */
        status = NX_SUCCESS;
    }

    nx_packet_release(recv_packet_ptr);

    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pop3_digest_authenticate                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an MD5 digest based on input string which per */
/*    POP3 protocol is the process ID and secret (e.g. Client password).  */
/*    This digest is used in APOP commands for the Client to authenticate */
/*    itself without clear texting it's password.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    client_ptr                            POP3 Client pointer           */
/*    process_ID_ptr                        Pointer to process_ID string  */
/*    process_ID_length                     Process ID string length      */
/*    result                                Pointer to MD5 digest         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_md5_initialize                     Initialize MD5 algorithm      */
/*   _nx_md5_update                         Update MD5 digest             */
/*   _nx_md5_digest_calculate               Complete the MD5 algorithm    */
/*   _nx_pop3_hex_ascii_convert             Convert digest to ascii       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_pop3_client_apop                  Send APOP cmd to Server       */
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
UINT  _nx_pop3_digest_authenticate(NX_POP3_CLIENT *client_ptr, CHAR *process_ID_ptr, UINT process_ID_length, CHAR *result)
{

CHAR    md5_binary[NX_POP3_MAX_BINARY_MD5];

    /* Initialize the Client session MD5 data.  */
    _nx_md5_initialize(&client_ptr -> nx_pop3_client_md5data);


    _nx_md5_update(&client_ptr -> nx_pop3_client_md5data, (unsigned char *)process_ID_ptr, process_ID_length);

     /* Finish calculation of the MD5 digest.  */
    _nx_md5_digest_calculate(&client_ptr -> nx_pop3_client_md5data, (unsigned char *)&md5_binary[0]);

     /* Convert digest to ASCII Hex representation.  */
    _nx_pop3_hex_ascii_convert(&md5_binary[0], NX_POP3_MAX_BINARY_MD5, result);

    /* Return successful completion */
    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_parse_process_id                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the process ID in the greeting from server.  */
/*    If found the Server is indicating we may use APOP to secure (open)  */
/*    the mailbox. The process ID is saved to the POP3 Client instance. If*/
/*    no process ID is found, this field is set to NULL.                  */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                      Pointer to Client                   */ 
/*    buffer                          Pointer to server reply buffer      */
/*    buffer_length                   Size of server reply buffer         */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                          Actual completion status            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nxd_pop3_client_connect        Connect and authenticate with server*/ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
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
VOID _nx_pop3_parse_process_id(NX_POP3_CLIENT *client_ptr, CHAR *buffer, UINT buffer_length)
{

CHAR c;
UINT index;
UINT pid_index;


   /* Set a pointer to the start of the buffer to search. */
    index = 0;

    /* Clear memory for parsing the server process ID.  */
    memset(&client_ptr -> nx_pop3_server_process_id[0], 0, NX_POP3_SERVER_PROCESS_ID_SIZE);    

    while(index < buffer_length)
    {
        c = *buffer;
        if (c == '<')
        {

            /* Found the start of the process ID. Now save it to the session instance including the angle brackets. */
            pid_index = 0;

            /* Check that we don't go over the size of the process ID buffer or off the end of the packet data. */
            while ((pid_index < NX_POP3_SERVER_PROCESS_ID_SIZE) && (index < buffer_length))
            {

                /* Copy the next character and advance the counters and buffer pointer. */
                client_ptr -> nx_pop3_server_process_id[pid_index] = *buffer;

                /*  Check if this is the end of the time stamp which is included in the process id string. . */
                if (*buffer == '>')
                {

                    /* This is the enclosing bracket. We're done. */
                    return;
                }

                index++;
                pid_index++;
                buffer++;
            }
            
        }
        index++;
        buffer++;
    }

    /* If we got here, we did not find any process IDs. */
    return; 
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pop3_parse_response                             PORTABLE C      */
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service parses the specified argument from the input buffer    */
/*    and returns a pointer to the argument if found, else the a NULL     */
/*    pointer.  Dashes are handled as word separators.                    */
/*                                                                        */
/*    crlf_are_word_breaks indicates if CR LF's should be handled as word */
/*    separators. convert_to_uppercase indicates if the argument should be*/
/*    converted to uppercase.                                             */
/*                                                                        */
/*    This service removes CR LF's at the end of the message.  It does not*/
/*    allocate or clear argument memory, but does.                        */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer                           Pointer to buffer to parse         */
/*    argument_index                   Index of argument to parse         */
/*    buffer_length                    Buffer size                        */
/*    argument                         Pointer to argument parsed         */
/*    argument_length                  Argument buffer size               */
/*    convert_to_uppercase             Convert argument to uppercase      */
/*    crlf_are_word_breaks             Handle CR LF's as word breaks      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/
void  _nx_pop3_parse_response(CHAR *buffer, UINT argument_index, UINT buffer_length, CHAR *argument, UINT argument_length, 
                              UINT convert_to_uppercase, UINT crlf_are_word_breaks)
{

UINT i = 0;
UINT j = 0;
UINT argument_char_count;


    /* Check for invalid input.  */
    if ((buffer == NX_NULL)|| (buffer[0] == 0x0) || (argument  == NX_NULL) || (buffer_length == 0) || 
        (argument_length == 0) || (argument_index == 0))
    {

        /* Return with argument not found status.  */
        return;
    }

    /* Initialize the argument to not found with NULL character.  */
    *argument = (CHAR)0x0;
    argument_char_count = 0;

    /* Is this the first argument?  */
    if (argument_index == 1)
    {

        /* Yes, search each character up to the end of the buffer for the first separator.  */
        while (i < buffer_length)
        {

            /* Did we find it?  */
            if (*buffer == ' ')
            {

                /* Yes, we're done with this loop! */
                break;
            }

            /* Treat a hyphen as word breaks if it is between words, not if it is the first char.  */
            if (*buffer == '-' && i > 0)
            {

                /* Yes, we're done with this loop! */
                break;
            }

            /* Did we go past the argument size limit?  */
            if (argument_char_count >= argument_length)
            {

                /* Yes, no argument found.  */
                break;
            }

            /* Copy the next character into the argument, converting
               to uppercase if the caller requested this.  */
            *argument++ = convert_to_uppercase ? (CHAR)toupper((INT)(*buffer)) : *buffer;

            argument_char_count++;

            /* Move to the next character.  */
            i++;
            buffer++;
        }

        /* Are we at the end of the buffer?  */
        if ((i == buffer_length) && (buffer_length >= 2))
        {       

            /* Yes, is there a line terminator?  */
            if (*(argument - 2) == 0x0D && *(argument - 1) == 0x0A)
            {

                /* Yes, remove it with a null character */
                *(argument - 2) = (CHAR) 0x0;
            }
        }
    }
    else
    {
        /*  No, we're not parsing the first argument.  */

        /*  Mark the start of the argument at the separator after the end of the previous argument. */
        while (j < argument_index && i < buffer_length)
        {

            /* Keep track of the number of separators in the buffer */

            /* Did we hit a line terminator?  */
            if ((*buffer == 0x0D) && (*(buffer + 1) == 0x0A))
            {

                /* Yes, Update the count of separators.  */
                j++;

                /* Are line terminators as word breaks?  */
                if (!crlf_are_word_breaks)
                {

                    /* No, treat as the end of the search string buffer.  */
                    break;                    
                }

                buffer++;
            }      

            /* Did we hit a space or a dash in the first argument?  */
            else if (*buffer == ' ' || 
                    (*buffer == '-' && j == 0)) 
            {

                /* These are counted as separators (note that
                   dashes found in arguments after the first arg
                   are NOT considered separators */
                j++; 
            }
            else
            {

                if (j == argument_index - 1)
                {

                    /* Have we exceeded the limit on the argument size?  */
                    if (argument_char_count < argument_length)
                    {

                        /* No, copy the next character into the argument.  */
                        argument_char_count++;

                        /* Convert to uppercase if the caller requests.  */
                        *argument++ = convert_to_uppercase ? (CHAR)toupper((INT)*buffer) : *buffer;
                    }
                }
            }

            /* Get the next character */
            i++;
            buffer++;
        }
    }

    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pop3_hex_ascii_convert                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts hexadecimal characters into an ASCII string. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source                                Source hex string             */
/*    source_length                         Length of source string       */
/*    destination                           Pointer to destination string */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_pop3_utility_digest_authenticate  Create digest for             */
/*                                              authentication            */
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
VOID  _nx_pop3_hex_ascii_convert(CHAR *source, UINT source_length, CHAR *destination)
{

UINT    i,j;
CHAR    digit;


    /* Setup destination index.  */
    j =  0;

    /* Loop to process the entire source string.  */
    for (i = 0; i < source_length; i++)
    {

        /* Pickup the first nibble.  */
        digit =  (source[i] >> 4) & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'a' - 10);

        /* Pickup the second nibble.  */
        digit =  source[i] & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'a' - 10);
    }

    /* Place a NULL in the destination string.  */
    destination[j] =  (CHAR) NX_NULL;

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_pop3_server_number_convert                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts a number into an ASCII string and returns the*/
/*    size of the string holding the number.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Unsigned integer number       */
/*    string_to_convert                     Destination string            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Size                                  Number of bytes in string     */
/*                                           (0 implies an error)         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_pop3_client_mail_item_get         Get mail item size            */
/*    _nx_pop3_client_mail_item_delete      Delete mail item              */
/*                                                                        */
/*  CALLED BY                                                             */
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
UINT  _nx_pop3_server_number_convert(UINT number, CHAR *string_to_convert)
{

UINT    j;
UINT    digit;
UINT    size;


    /* Initialize counters.  */
    size =  0;

    /* Loop to convert the number to ASCII.  */
    while (size < 9)
    {

        /* Shift the current digits over one.  */
        for (j = size; j != 0; j--)
        {

            /* Move each digit over one place.  */
            string_to_convert[j] =  string_to_convert[j - 1];
        }

        /* Compute the next decimal digit.  */
        digit =  number % 10;

        /* Update the input number.  */
        number =  number / 10;

        /* Store the new digit in ASCII form.  */
        string_to_convert[0] =  (CHAR) (digit + 0x30);

        /* Increment the size.  */
        size++;

        /* Determine if the number is now zero.  */
        if (number == 0)
            break;
    }

    /* Make the string NULL terminated.  */
    string_to_convert[size] =  (CHAR) NX_NULL;

    /* Determine if there is an overflow error.  */
    if (number)
    {

        /* Error, return bad values to user.  */
        size =  0;
        string_to_convert[0] = '0';
    }

    /* Return size to caller.  */
    return(size);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_pop3_client_connect                            PORTABLE C      */ 
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function binds the Client socket to the POP3 port and connects */
/*    to the POP3 server. If the connection is made, this function        */
/*    processes the server greeting, and initiates the APOP/USER          */
/*    authentication with the sever.                                      */
/*                                                                        */ 
/*    This supports both IPv6 and IPv4 POP3 server connections.           */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*    server_ip_address                 Pointer to POP3 server IP address */
/*    server_port                       POP3 server port                  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_POP3_PARAM_ERROR               Invalid Client user or password   */
/*    NX_POP3_CANNOT_PARSE_REPLY        Unable to parse server reply      */
/*    status                            TCP service completion status     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nxd_pop3_client_create            Create POP3 Client and socket    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_client_socket_bind          NetX TCP socket bind service     */ 
/*    nxd_tcp_client_socket_connect      NetX TCP socket connect service  */
/*    nx_tcp_socket_receive              NetX TCP socket receive service  */
/*    _nx_pop3_parse_response            Parse Server reply code          */
/*    _nx_pop3_parse_process_id          Parse server ID for APOP digest  */
/*    _nx_pop3_client_apop               Send Authenticated user/pass     */
/*    _nx_pop3_client_user_pass          Send User Pass in clear text     */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the client port,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nxd_pop3_client_connect(NX_POP3_CLIENT *client_ptr, NXD_ADDRESS *server_ip_address, ULONG server_port)
{

UINT status;
NX_PACKET *recv_packet_ptr;
CHAR      *buffer_ptr;
CHAR      argument[10];


    /* Check for client name/password/shared secret too long for allotted buffer.  */
    if ((_nx_utility_string_length_check(&client_ptr -> nx_pop3_client_name[0], NX_NULL, NX_POP3_MAX_USERNAME)) || 
        (_nx_utility_string_length_check(&client_ptr -> nx_pop3_client_password[0], NX_NULL, NX_POP3_MAX_PASSWORD)))
    {

        return NX_POP3_PARAM_ERROR;
    }

    status =  nx_tcp_client_socket_bind(&client_ptr -> nx_pop3_client_tcp_socket, NX_ANY_PORT, NX_IP_PERIODIC_RATE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {
        return status;
    }

    status =  nxd_tcp_client_socket_connect(&client_ptr -> nx_pop3_client_tcp_socket, 
                                            server_ip_address,
                                            server_port, 
                                            NX_POP3_CLIENT_CONNECTION_TIMEOUT);

    if (status != NX_SUCCESS)
    {
        return status;
    }

    /* Receive server reply over session socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer_ptr = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer_ptr, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to bad reply error condition.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        if (client_ptr -> nx_pop3_client_enable_APOP_authentication)
        {

            /* Attempt to extract the server process ID in the greeting.  */
            _nx_pop3_parse_process_id(client_ptr, buffer_ptr, recv_packet_ptr -> nx_packet_length);

            /* Did we find a process ID? */
            if (client_ptr -> nx_pop3_server_process_id[0] != 0)
            {

                /* Do APOP authentication.  */
                status = _nx_pop3_client_apop(client_ptr);
        
                if (status == NX_SUCCESS)
                {
                    return status;
                }
                else if (status != NX_POP3_SERVER_ERROR_STATUS)
                {

                    /* Another error encountered (packet pool depletion, broken connection, etc*/
                    return status;
                }
            }
            else
            {
                /* Possibly the server doesn't like APOP. Try User Pass */
                client_ptr -> nx_pop3_client_enable_APOP_authentication = NX_FALSE;
            }
        }

        /* Try USER PASS authentication.  */
        status = _nx_pop3_client_user_pass(client_ptr);
    }

    nx_packet_release(recv_packet_ptr);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_apop                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to authenticate the POP3 client with the POP3*/
/*    server.  If successful returns NX_SUCCESS.                          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Authentication accepted by server */
/*    NX_POP3_APOP_FAILED_MD5_DIGEST    Error creating digest             */
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                      Packet too small for command      */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
/*    _nx_pop3_digest_authenticate      Create authentication string      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nxd_pop3_client_connect           Connect with POP3 server          */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_apop(NX_POP3_CLIENT *client_ptr)
{

UINT         status;
NX_PACKET    *packet_ptr, *recv_packet_ptr;
CHAR         *buffer;
CHAR         argument[10];  
CHAR         md5_digest_buffer[NX_POP3_MAX_ASCII_MD5 + 1]; 
UINT         packet_type;
UINT         index;
CHAR         userid_buffer[100];
UINT         server_process_id_length;
UINT         client_password_length;
UINT         client_name_length;
UINT         md5_digest_buffer_length;


    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    /* Create the APOP digest. */
    memset(&userid_buffer[0],0,sizeof(userid_buffer));

    /* Validate copy size. */
    if (_nx_utility_string_length_check(client_ptr -> nx_pop3_server_process_id, &server_process_id_length, NX_POP3_SERVER_PROCESS_ID_SIZE) ||
        _nx_utility_string_length_check(client_ptr -> nx_pop3_client_password, &client_password_length, NX_POP3_MAX_PASSWORD))
    {
        nx_packet_release(packet_ptr);
        return(NX_POP3_INSUFFICIENT_PACKET_PAYLOAD);
    }
    if((server_process_id_length + client_password_length) > sizeof(userid_buffer)) 
    {
        nx_packet_release(packet_ptr);
        return(NX_POP3_INSUFFICIENT_PACKET_PAYLOAD);
    }
        
    memcpy(&userid_buffer[0], &client_ptr -> nx_pop3_server_process_id[0], server_process_id_length); /* Use case of memcpy is verified. */
    index = server_process_id_length;
    memcpy(&userid_buffer[index], &client_ptr -> nx_pop3_client_password[0], client_password_length); /* Use case of memcpy is verified. */
    status = _nx_pop3_digest_authenticate(client_ptr, &userid_buffer[0], (server_process_id_length + client_password_length), &md5_digest_buffer[0]);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        nx_packet_release(packet_ptr);

        /* Return failed APOP attempt error condition.  */
        return NX_POP3_APOP_FAILED_MD5_DIGEST;
    }

    /* Verify the packet payload will hold the APOP command. */
    if (_nx_utility_string_length_check(client_ptr -> nx_pop3_client_name, &client_name_length, NX_POP3_MAX_USERNAME) ||
        _nx_utility_string_length_check(md5_digest_buffer, &md5_digest_buffer_length, NX_POP3_MAX_ASCII_MD5))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_APOP) - 1)
         + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1) + 2
         + client_name_length
         + md5_digest_buffer_length) >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    buffer = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;

    /* Create the APOP command. */
    memcpy(buffer, NX_POP3_COMMAND_APOP, (sizeof(NX_POP3_COMMAND_APOP) - 1)); /* Use case of memcpy is verified. */
    buffer += (sizeof(NX_POP3_COMMAND_APOP) - 1);
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_APOP) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_APOP) - 1);

    memcpy(buffer, " ", 1); /* Use case of memcpy is verified. */
    buffer++;
    packet_ptr -> nx_packet_append_ptr++;
    packet_ptr -> nx_packet_length++;

    memcpy(buffer,  client_ptr -> nx_pop3_client_name, client_name_length); /* Use case of memcpy is verified. */
    buffer += client_name_length; 
    packet_ptr -> nx_packet_length += client_name_length; 
    packet_ptr -> nx_packet_append_ptr += client_name_length; 

    memcpy(buffer, " ", 1); /* Use case of memcpy is verified. */
    buffer++;
    packet_ptr -> nx_packet_append_ptr++;
    packet_ptr -> nx_packet_length++;

    memcpy(buffer,  &md5_digest_buffer[0], md5_digest_buffer_length); /* Use case of memcpy is verified. */
    buffer += md5_digest_buffer_length;
    packet_ptr -> nx_packet_length += md5_digest_buffer_length;
    packet_ptr -> nx_packet_append_ptr += md5_digest_buffer_length;

    memcpy(buffer,  NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply over session socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to bad reply error condition.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* APOP command accepted. */
        status = NX_SUCCESS;
    }

    nx_packet_release(recv_packet_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_pop3_client_user_pass                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function attempts to login using client sername and password   */
/*    with the POP3 server.  If successful returns NX_SUCCESS.            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client struct          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Login accepted by server          */
/*    NX_POP3_INSUFFICIENT_PACKET_PAYLOAD                                 */
/*                                      Packet too small for command      */
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                Allocate packet from packet pool  */ 
/*    nx_packet_release                 Release packet back to pool       */
/*    nx_tcp_socket_send                Send packet out TCP socket        */
/*    nx_tcp_socket_receive             Retrieve packet from TCP socket   */
/*    _nx_pop3_parse_response           Extract word from server response */
/*    _nx_pop3_digest_authenticate      Create authentication string      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nxd_pop3_client_connect           Connect with POP3 server          */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_pop3_client_user_pass(NX_POP3_CLIENT *client_ptr)
{

UINT         status;
NX_PACKET    *packet_ptr, *recv_packet_ptr, *next_recv_packet_ptr;
CHAR         *buffer;
CHAR         argument[10];  
UINT         packet_type;
UINT         client_name_length;
UINT         client_password_length;


    if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                 &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error condition.  */
        return(status);
    }

    /* Verify the packet payload will hold the user command message. */
    if (_nx_utility_string_length_check(client_ptr -> nx_pop3_client_name, &client_name_length, NX_POP3_MAX_USERNAME))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_USER) - 1) + 
         1 + client_name_length + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1))
             >= packet_ptr -> nx_packet_data_end)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
    }

    buffer = (CHAR *)(packet_ptr -> nx_packet_prepend_ptr);


    memcpy(buffer, NX_POP3_COMMAND_USER, (sizeof(NX_POP3_COMMAND_USER) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length = (sizeof(NX_POP3_COMMAND_USER) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_USER) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_USER) - 1);

    memcpy(buffer, " ", 1); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length++;
    packet_ptr -> nx_packet_append_ptr++;
    buffer++;

    memcpy(buffer, client_ptr -> nx_pop3_client_name, client_name_length); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += client_name_length; 
    packet_ptr -> nx_packet_append_ptr += client_name_length;
    buffer += client_name_length; 

    memcpy(buffer, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
    packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
    buffer += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return the error condition.  */
        return(status);
    }

    /* Receive server reply to Client username over session socket.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error condition.  */
        return(status);        
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (recv_packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(recv_packet_ptr);
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set a pointer to the packet data.  */
    buffer = (CHAR *)(recv_packet_ptr -> nx_packet_prepend_ptr);

    /* Parse the first argument of the server reply.  */
    _nx_pop3_parse_response(buffer, 1, recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

    /* Initialize status to bad reply error condition.  */
    status =  NX_POP3_SERVER_ERROR_STATUS;

    /* Did the server accept the Client command?  */
    if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
    {

        /* We are done with this packet. */
        nx_packet_release(recv_packet_ptr);

        if (client_ptr -> nx_pop3_client_tcp_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
            packet_type = NX_IPv6_TCP_PACKET;
        else
            packet_type = NX_IPv4_TCP_PACKET;
    
        /* Allocate another packet.  */
        status =  nx_packet_allocate(client_ptr -> nx_pop3_client_packet_pool_ptr,  
                                     &packet_ptr, packet_type, NX_POP3_CLIENT_PACKET_TIMEOUT);

        /* Check for error.  */
        if (status != NX_SUCCESS)
        {

            /* Return the error condition.  */
            return(status);
        }

        /* Verify the packet payload will hold the password request. */
        if (_nx_utility_string_length_check(client_ptr -> nx_pop3_client_password, &client_password_length, NX_POP3_MAX_PASSWORD))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
        }

        if ((packet_ptr -> nx_packet_prepend_ptr + (sizeof(NX_POP3_COMMAND_PASS) - 1) + 
             1 + client_password_length + (sizeof(NX_POP3_COMMAND_TERMINATION) - 1))
            >= packet_ptr -> nx_packet_data_end)
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            return NX_POP3_INSUFFICIENT_PACKET_PAYLOAD;
        }

        buffer = (CHAR *)packet_ptr -> nx_packet_prepend_ptr;
        memcpy(buffer, NX_POP3_COMMAND_PASS, (sizeof(NX_POP3_COMMAND_PASS) - 1)); /* Use case of memcpy is verified. */
        buffer += (sizeof(NX_POP3_COMMAND_PASS) - 1);
        packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_PASS) - 1);
        packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_PASS) - 1);

        memcpy(buffer, " ", 1); /* Use case of memcpy is verified. */
        buffer++;
        packet_ptr -> nx_packet_length++;
        packet_ptr -> nx_packet_append_ptr++;

        memcpy(buffer, client_ptr -> nx_pop3_client_password, client_password_length); /* Use case of memcpy is verified. */
        buffer += client_password_length;
        packet_ptr -> nx_packet_length += client_password_length;
        packet_ptr -> nx_packet_append_ptr += client_password_length;

        memcpy(buffer, NX_POP3_COMMAND_TERMINATION, (sizeof(NX_POP3_COMMAND_TERMINATION) - 1)); /* Use case of memcpy is verified. */
        buffer += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
        packet_ptr -> nx_packet_length += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);
        packet_ptr -> nx_packet_append_ptr += (sizeof(NX_POP3_COMMAND_TERMINATION) - 1);

        /* Send the next Client message out.  */
        status =  nx_tcp_socket_send(&client_ptr -> nx_pop3_client_tcp_socket, packet_ptr, NX_POP3_TCP_SOCKET_SEND_WAIT);

        /* Check for error.  */
        if (status != NX_SUCCESS)
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Return the error condition.  */
            return(status);
        }

        /* Receive server reply to Client password over session socket.  */
        status = nx_tcp_socket_receive(&(client_ptr -> nx_pop3_client_tcp_socket), &next_recv_packet_ptr, NX_POP3_SERVER_REPLY_TIMEOUT);

        /* Check for error.  */
        if (status != NX_SUCCESS)
        {

            /* Return error condition.  */
            return(status);        
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        if (next_recv_packet_ptr -> nx_packet_next)
        {

            /* Chained packet is not supported. */
            nx_packet_release(next_recv_packet_ptr);
            return(NX_INVALID_PACKET);
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Set a pointer to the packet data.  */
        buffer = (CHAR *)(next_recv_packet_ptr -> nx_packet_prepend_ptr);

        /* Parse the first argument of the server reply.  */
        _nx_pop3_parse_response(buffer, 1, next_recv_packet_ptr -> nx_packet_length, (CHAR *)&argument, 10, NX_FALSE, NX_FALSE);

        /* Did the server accept the Client command?  */
        if (memcmp(argument, NX_POP3_POSITIVE_STATUS, (sizeof(NX_POP3_POSITIVE_STATUS) - 1)) == 0x0)
        {

            status = NX_SUCCESS;
        }

        /* We are done with this packet. */
        nx_packet_release(next_recv_packet_ptr);
    }
    else
    {

        /* Server rejected the Client message. */
        nx_packet_release(recv_packet_ptr);
    }

    return status;
}

