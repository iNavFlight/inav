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
/**                                                                       */  
/** NetX SMTP Client Component                                            */
/**                                                                       */
/**   Simple Mail Transfer Protocol (SMTP)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#define NX_SMTP_SOURCE_CODE


/* Force error checking to be disabled in this module.  */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif



#include "ctype.h"
#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv4.h"
#include "nxd_smtp_client.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif
#include "nx_tcp.h"

/* Necessary for threadx thread state macros. */
extern  TX_THREAD   *_tx_thread_current_ptr; 
extern  TX_THREAD    _tx_timer_thread; 
extern  volatile ULONG _tx_thread_system_state;

#define         NX_SMTP_BUFFER_SIZE           512
static CHAR     _nx_smtp_buffer[NX_SMTP_BUFFER_SIZE]; 


/* Define internal SMTP Client functions.  */

static VOID _nx_smtp_find_crlf(UCHAR *buffer, UINT length, UCHAR **CRLF, UINT reverse);
static UINT _nx_smtp_cmd_idle(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_idle(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_greeting(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_greeting(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_ehlo(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_ehlo(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_helo(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_helo(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_mail(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_mail(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_rcpt(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_rcpt(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_data(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_data(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_message(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_message(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_rset(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_rset(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_quit(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_quit(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_noop(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_noop(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_auth(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_auth(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_cmd_auth_challenge(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_auth_challenge(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_rsp_hello_command(NX_SMTP_CLIENT* client_ptr);
static UINT _nx_smtp_utility_read_server_code(NX_SMTP_CLIENT *client_ptr, ULONG timeout, UINT  receive_all_lines);
static UINT _nx_smtp_utility_send_to_server(NX_SMTP_CLIENT *client_ptr, CHAR *buffer_ptr, UINT buffer_length, ULONG timeout);
static UINT _nx_smtp_utility_authentication_challenge(NX_SMTP_CLIENT *client_ptr, UCHAR *buffer_ptr, UINT length);  
static UINT _nx_smtp_client_process(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_utility_send_header_to_server(NX_SMTP_CLIENT *client_ptr, ULONG timeout) ;
static VOID _nx_smtp_utility_parse_server_services(NX_SMTP_CLIENT *client_ptr);
static UINT _nx_smtp_parse_250_response(UCHAR *buffer_ptr, UINT buffer_length, UINT *is_last_code);
static UINT _nx_smtp_parse_response(NX_SMTP_CLIENT *client_ptr, UCHAR *buffer, UINT arguement_index, 
                                    UINT buffer_length, UCHAR *arguement, UINT arguement_length, 
                                    UINT include_crlf);

static NX_SMTP_CLIENT_STATES protocol_states[] =
{
    {_nx_smtp_cmd_idle               , _nx_smtp_rsp_idle},
    {_nx_smtp_cmd_greeting           , _nx_smtp_rsp_greeting},
    {_nx_smtp_cmd_ehlo               , _nx_smtp_rsp_ehlo},
    {_nx_smtp_cmd_helo               , _nx_smtp_rsp_helo},
    {_nx_smtp_cmd_mail               , _nx_smtp_rsp_mail},
    {_nx_smtp_cmd_rcpt               , _nx_smtp_rsp_rcpt},
    {_nx_smtp_cmd_data               , _nx_smtp_rsp_data},
    {_nx_smtp_cmd_message            , _nx_smtp_rsp_message},
    {_nx_smtp_cmd_rset               , _nx_smtp_rsp_rset},
    {_nx_smtp_cmd_quit               , _nx_smtp_rsp_quit},
    {_nx_smtp_cmd_noop               , _nx_smtp_rsp_noop},
    {_nx_smtp_cmd_auth               , _nx_smtp_rsp_auth},
    {_nx_smtp_cmd_auth_challenge     , _nx_smtp_rsp_auth_challenge},     
};




/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_smtp_client_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the client create service. */ 
/*                                                                        */   
/*    Note: The string lengths of username,password,from_address and      */
/*    client_domain are limited by internal buffer size.                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SMTP Client instance   */ 
/*    ip_ptr                            Pointer to IP instance            */
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    username                          Pointer to username               */ 
/*    password                          Pointer to password               */ 
/*    from_address                      Pointer to Client email address   */
/*    client_domain                     Pointer to client domain          */ 
/*    authentication_type               SMTP authentication type          */ 
/*    server_address                    Pointer to server address         */ 
/*    server_port                       SMTP server TCP port              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    NX_SMTP_INVALID_PARAM              Invalid non pointer input        */
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_smtp_client_create           Actual SMTP client create service */ 
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
UINT  _nxde_smtp_client_create(NX_SMTP_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *client_packet_pool_ptr, 
                               CHAR *username, CHAR *password, CHAR *from_address,
                               CHAR *client_domain, UINT authentication_type, 
                               NXD_ADDRESS *server_address, UINT port)
{

UINT status;


    if ((ip_ptr == NX_NULL) || (client_ptr == NX_NULL) || (client_packet_pool_ptr == NX_NULL) || 
        (from_address == NX_NULL) || (username == NX_NULL) || (password == NX_NULL) ||
        (client_domain == NX_NULL) || (server_address == NX_NULL))
    {

        /* Return error status.  */
       return(NX_PTR_ERROR);
    }

        /* Check for invalid non pointer input. */
    if (ip_ptr -> nx_ip_id != NX_IP_ID)
    {

        return NX_SMTP_INVALID_PARAM;
    }

    
    /* Make sure the IP version is set correctly. */
    if((server_address -> nxd_ip_version != NX_IP_VERSION_V4) &&
       (server_address -> nxd_ip_version != NX_IP_VERSION_V6))
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Call the actual client create service.  */
    status =  _nxd_smtp_client_create(client_ptr, ip_ptr, client_packet_pool_ptr, username, password,
                                      from_address, client_domain, authentication_type, 
                                      server_address, port);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_smtp_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a SMTP client instance for sending mail to an */
/*    SMTP Server over IPv4 or IPv6 networks. It also creates the TCP     */
/*    socket for making connections with the SMTP server, sets the        */
/*    server IP address and port, and sets the Client username and SMTP   */
/*    address (from address) used in all SMTP mail transmissions.         */ 
/*                                                                        */   
/*    Note: The string lengths of username,password,from_address and      */
/*    client_domain are limited by internal buffer size.                  */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SMTP Client instance   */ 
/*    ip_ptr                            Pointer to IP instance            */
/*    packet_pool_ptr                   Pointer to client packet pool     */
/*    username                          Pointer to username               */ 
/*    password                          Pointer to password               */ 
/*    from_address                      Pointer to Client email address   */
/*    client_domain                     Pointer to client domain          */ 
/*    authentication_type               SMTP authentication type          */ 
/*    server_address                    Pointer to server address         */ 
/*    server_port                       SMTP server TCP port              */
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
/*    memset                            Clear area of memory              */ 
/*    memcpy                            Copy data to area of memory       */
/*    nx_tcp_socket_create              Create a NetX TCP socket          */ 
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
UINT  _nxd_smtp_client_create(NX_SMTP_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *client_packet_pool_ptr, 
                              CHAR *username, CHAR *password, CHAR *from_address,
                              CHAR *client_domain, UINT authentication_type, 
                              NXD_ADDRESS *server_address, UINT port)
{                            

UINT status = NX_SUCCESS;
UINT str_length;
NX_SMTP_CLIENT_MAIL    *mail_ptr;


    /* Initialize client as not ready for SMTP yet. */
    client_ptr -> nx_smtp_client_init_status = NX_FALSE;

    mail_ptr = &client_ptr -> nx_smtp_client_mail;

    /* Clear the Client and session memory.  */
    memset(client_ptr, 0, sizeof(NX_SMTP_CLIENT));

    /* Configure client with input parameters.  */
    status = _nx_utility_string_length_check(username, &str_length, NX_SMTP_CLIENT_MAX_USERNAME);
    if (status)
    {
        return(status);
    }

    memcpy(client_ptr -> nx_smtp_username, username, str_length); /* Use case of memcpy is verified. */

    status = _nx_utility_string_length_check(password, &str_length, NX_SMTP_CLIENT_MAX_PASSWORD);
    if (status)
    {
        return(status);
    }

    memcpy(client_ptr -> nx_smtp_password, password, str_length); /* Use case of memcpy is verified. */

    status = _nx_utility_string_length_check(client_domain, &str_length, NX_SMTP_CLIENT_MAX_USERNAME);
    if (status)
    {
        return(status);
    }

    memcpy(client_ptr -> nx_smtp_client_domain, client_domain, str_length); /* Use case of memcpy is verified. */

    /* Set the mail server IP address and port number.  */

#ifdef FEATURE_NX_IPV6
    client_ptr -> nx_smtp_server_address.nxd_ip_version = server_address -> nxd_ip_version;
    client_ptr -> nx_smtp_server_address.nxd_ip_address.v6[0] = server_address -> nxd_ip_address.v6[0] ;
    client_ptr -> nx_smtp_server_address.nxd_ip_address.v6[1] = server_address -> nxd_ip_address.v6[1] ;
    client_ptr -> nx_smtp_server_address.nxd_ip_address.v6[2] = server_address -> nxd_ip_address.v6[2] ;
    client_ptr -> nx_smtp_server_address.nxd_ip_address.v6[3] = server_address -> nxd_ip_address.v6[3] ;
#else

    client_ptr -> nx_smtp_server_address.nxd_ip_version = server_address -> nxd_ip_version;
    client_ptr -> nx_smtp_server_address.nxd_ip_address.v4 = server_address -> nxd_ip_address.v4;
#endif
    client_ptr -> nx_smtp_server_port = (port & 0xFFFF);

    /* Set up the "from" address. */
    mail_ptr -> nx_smtp_client_mail_from_address = from_address;

    /* Check if authentication type is specified. */
    if ((authentication_type != NX_SMTP_CLIENT_AUTH_PLAIN) &&
        (authentication_type != NX_SMTP_CLIENT_AUTH_LOGIN) &&
        (authentication_type != NX_SMTP_CLIENT_AUTH_NONE))
    {

        /* No. Set the default authentication type. */
        authentication_type = NX_SMTP_CLIENT_AUTH_PLAIN;
    }

    client_ptr -> nx_smtp_client_authentication_type = authentication_type;

    /* Configure client IP options.  */
    client_ptr -> nx_smtp_client_ip_ptr   =  ip_ptr;
    client_ptr -> nx_smtp_client_packet_pool_ptr =  client_packet_pool_ptr;

    /* Set the Client ID to indicate the SMTP client thread is ready.  */
    client_ptr -> nx_smtp_client_id = NX_SMTP_CLIENT_ID;

    /* Create a tcp socket to send/receive SMTP data.  */
    status =  nx_tcp_socket_create(client_ptr -> nx_smtp_client_ip_ptr,
                                   &client_ptr -> nx_smtp_client_socket, "SMTP Client socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 
                                   NX_SMTP_CLIENT_TCP_WINDOW_SIZE,
                                   NX_NULL, NX_NULL);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Initialize client as ready for conducting an SMTP session. */
    client_ptr -> nx_smtp_client_init_status = NX_TRUE;

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_smtp_client_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors on the client delete service.       */ 
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                         Pointer to client struct         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                       Invalid pointer parameter        */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_smtp_client_delete            Actual SMTP client delete service */ 
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
UINT  _nxe_smtp_client_delete(NX_SMTP_CLIENT *client_ptr)
{

UINT status;


    /* Check for the validity of input parameter.  */
    if ((client_ptr == NX_NULL) || (client_ptr -> nx_smtp_client_id != NX_SMTP_CLIENT_ID))
    {

        /* Return error status.  */
        return(NX_PTR_ERROR);
    }

    /* Call the actual client create service.  */
    status =  _nx_smtp_client_delete(client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_client_delete                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created SMTP client and releases */
/*    all client resources (sockets, packet pools).                       */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to SMTP Client instance   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_smtp_session_delete           Delete the SMTP Client session    */
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
UINT  _nx_smtp_client_delete(NX_SMTP_CLIENT *client_ptr)
{

UINT status;


    if ((client_ptr  -> nx_smtp_client_socket).nx_tcp_socket_state == NX_TCP_ESTABLISHED)
    {

        nx_tcp_socket_disconnect(&client_ptr -> nx_smtp_client_socket, NX_SMTP_CLIENT_DISCONNECT_TIMEOUT);

        nx_tcp_client_socket_unbind(&client_ptr -> nx_smtp_client_socket);
    }

    status =  nx_tcp_socket_delete(&(client_ptr -> nx_smtp_client_socket));

    /* Check status.  */
    if (status)
    {        
        return status;
    }

    /* Clear client memory. */
    memset(client_ptr, 0, sizeof(NX_SMTP_CLIENT));

    /* Return success status.  */
    return(NX_SUCCESS);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_smtp_mail_send                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors on the mail send service.           */ 
/*                                                                        */   
/*    Note: The string lengths of recipient_address and subject are       */
/*    limited by internal buffer size.                                    */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                      Pointer to Client instance          */ 
/*    recipient_address               Pointer to recipient (To) address   */
/*    priority                        Mail send priority level            */ 
/*    subject                         Pointer to mail subject text        */
/*    mail_body                       Pointer to mail message text        */
/*    mail_body_length                Size of mail message text           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                    Invalid pointer parameter           */ 
/*    status                          Actual completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_smtp_mail_send              Actual SMTP mail send service       */ 
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
UINT  _nxe_smtp_mail_send(NX_SMTP_CLIENT *client_ptr, CHAR *recipient_address, UINT priority, 
                          CHAR *subject, CHAR *mail_body, UINT mail_body_length)
{

UINT  status;


    /* Check for invalid pointer input. */
    if((client_ptr == NX_NULL) || (recipient_address == NX_NULL) || 
       (mail_body == NX_NULL) || (subject == NX_NULL))

    {
        return NX_PTR_ERROR;
    }

    if (mail_body_length == 0) 
    {
        return NX_SMTP_INVALID_PARAM;
    }

    /* Check if this function is called from the appropriate thread.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_smtp_mail_send(client_ptr, recipient_address,  priority, 
                                subject, mail_body, mail_body_length);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_mail_send                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates and sends an SMTP mail item with the input    */
/*    parameters for a previously created SMTP Client. It supports IPv4   */
/*    and IPv6 connections.                                               */
/*                                                                        */ 
/*    Before calling this service, the SMTP application must initialize   */
/*    the SMTP Client (nx_smtp_client_init). Thereafter it need not       */
/*    reinitialize the SMTP Client to send subsequent mail items.         */
/*                                                                        */
/*    This service assumes the syntax of the from_address is correct.     */
/*                                                                        */   
/*    Note: The string lengths of recipient_address and subject are       */
/*    limited by internal buffer size.                                    */
/*                                                                        */
/*   INPUT                                                                */ 
/*                                                                        */
/*    client_ptr                      Pointer to Client instance          */ 
/*    recipient_address               Pointer to recipient (To) address   */
/*    priority                        Mail send priority level            */ 
/*    subject                         Pointer to mail subject text        */
/*    mail_body                       Pointer to mail message text        */
/*    mail_body_length                Size of mail message text           */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Success completion status           */ 
/*    status                          Actual completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                          Clear data in memory                */ 
/*    _nx_smtp_client_process         Conducts SMTP session for           */
/*                                      transmitting mail message         */
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
UINT  _nx_smtp_mail_send(NX_SMTP_CLIENT *client_ptr, 
                         CHAR *recipient_address, 
                         UINT priority, CHAR *subject, 
                         CHAR *mail_body, UINT mail_body_length)
{

UINT                        status;
NX_SMTP_CLIENT_MAIL         *mail_ptr;


    client_ptr -> nx_smtp_client_mute = NX_FALSE;

    /* Verify Client is properly set up and ready to conduct an SMTP session. */
    if (client_ptr -> nx_smtp_client_init_status != NX_TRUE)
    {
        return NX_SMTP_CLIENT_NOT_INTIALIZED;
    }

    /* Initialize the Client to idle. */
    client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
    client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;


    /* Set local variables for convenience. */
    mail_ptr = &client_ptr -> nx_smtp_client_mail;

    /* Did we get a valid priority type?  */
    if ((priority != NX_SMTP_MAIL_PRIORITY_LOW)    &&
        (priority != NX_SMTP_MAIL_PRIORITY_NORMAL) &&
        (priority != NX_SMTP_MAIL_PRIORITY_HIGH))
    {

        /* No, default priority to normal.  */
        mail_ptr -> nx_smtp_client_mail_priority =  NX_SMTP_MAIL_PRIORITY_NORMAL;
    }
    else
    {

        /* Yes, apply it to the session priority.  */
        mail_ptr -> nx_smtp_client_mail_priority = priority;
    }
    mail_ptr -> nx_smtp_client_mail_subject = subject;
    mail_ptr -> nx_smtp_client_mail_body = mail_body;
    mail_ptr -> nx_smtp_client_mail_body_length = mail_body_length;
    mail_ptr -> nx_smtp_client_mail_recipient_address = recipient_address;

    /* Mark mail item as unsent */
    client_ptr -> nx_smtp_client_mail_status = NX_SUCCESS;

    /* Set up the recipient. */

    /* Set up a local pointer to the session for convenience. */

    /* Set session members to initial values.  */
    client_ptr -> nx_smtp_client_authentication_state = NX_SMTP_NOT_AUTHENTICATED;
    client_ptr -> nx_smtp_client_reply_code_status = 0;

    /* Start this session by setting the state  to the first step in SMTP Protocol, greeting.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_GREETING;
    client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_GREETING;

    status = _nx_smtp_client_process(client_ptr);

    /* Return success status.  */
    return(status);

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_client_process                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function executes the SMTP client state machine to transmit a  */
/*    previously created mail item to an SMTP server. For more details,   */
/*    See nx_smtp_mail_send.                                              */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    client_ptr                      Pointer to SMTP Client instance     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Success completion status           */ 
/*    status                          Actual completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    nx_tcp_client_socket_bind       Bind NetX TCP socket to local port  */
/*    nx_tcp_client_socket_unbind     Release port from NetX TCP socket   */
/*    nx_tcp_client_socket_connect    Connect to a TCP server socket      */
/*    nx_tcp_client_socket_disconnect Disconnect from TCP server socket   */
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
static UINT _nx_smtp_client_process(NX_SMTP_CLIENT *client_ptr)
{

UINT                    status;
UINT                    close_connection = NX_FALSE;


    /* Initialize completion status to successful outcome. */
    status = NX_SUCCESS;

    /* Run the SMTP protocol state machine till session terminates.  */
    while (status == NX_SUCCESS)  
    {

        /* Check if we are starting a mail transaction. */
        if (client_ptr -> nx_smtp_client_cmd_state == NX_SMTP_CLIENT_STATE_GREETING)
        {

            /* We are so we need to set up a connection. Bind the socket to client port.  */
            status =  nx_tcp_client_socket_bind(&client_ptr -> nx_smtp_client_socket, NX_ANY_PORT, 100);

            /* Check for error.  */
            if (status != NX_SUCCESS)
            {
        
                /* Reset the Client to idle. The caller must resend the email. */
                client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
                client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;

                return status;
            }

            /* Connect to the SMTP server using our tcp socket.  */
            status =  nxd_tcp_client_socket_connect(&client_ptr -> nx_smtp_client_socket, 
                                                    &client_ptr -> nx_smtp_server_address,
                                                    client_ptr -> nx_smtp_server_port, 
                                                    NX_SMTP_CLIENT_CONNECTION_TIMEOUT);
           
            /* Check for error.  */
            if (status != NX_SUCCESS)
            {   

                nx_tcp_client_socket_unbind(&client_ptr -> nx_smtp_client_socket);

                /* Reset the Client to idle. The caller must resend the email.*/
                client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
                client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;
                return status; 
            }

            /* Successful connection set up. Now let the Client task process the GREETING command. */
        }

        /* Is the next command waiting the outcome of the previous reply handler?  */
        if ((client_ptr -> nx_smtp_client_cmd_state == NX_SMTP_CLIENT_STATE_AWAITING_REPLY) &&
            /* Do not advance the state if the client is in mute. */
            (!client_ptr -> nx_smtp_client_mute))
        {

            /* Yes, update the session state to the next command to send.  */
            client_ptr -> nx_smtp_client_cmd_state =  client_ptr -> nx_smtp_client_rsp_state;
        }

        if (!client_ptr -> nx_smtp_client_mute)
        {

            /* Execute the next command in the SMTP state machine.  */
            status =  (*protocol_states[client_ptr -> nx_smtp_client_cmd_state].cmd)(client_ptr);
    
            /* Check for internal error state in the SMTP state machine.  */
            if (status != NX_SUCCESS)
            {
    
                /* This is likely an internal error and we need to break off the connection, reset the Client state 
                   to an idle state.  */
    
                /* Set the status to close the connection down. */
                close_connection = NX_TRUE;
            }
        }

        /* Reset server reply to null. */
        client_ptr -> nx_smtp_client_reply_code_status = 0;

        /* Wait for a response unless we are closing the connection, or have 
           encountered an internal error (packet allocation error for example). */
        if (close_connection == NX_FALSE)
        {    

            /* Awaiting the server reply to the command just sent.  */
            status = (*protocol_states[client_ptr -> nx_smtp_client_rsp_state].rsp)(client_ptr);
    
            /* Check for internal error state in the SMTP state machine.  */
            if (status != NX_SUCCESS)
            {
    
                /* This is an internal error or invalid server reply of some kind. Just shut down the 
                   connection, notify the host application and set the Client to idle. */
    
                /* Set the status to close the connection down. */
                close_connection = NX_TRUE;
            }
    
            /* Are we at the end of the Client state (received a reply to our QUIT message to the server)? */
            else if (client_ptr -> nx_smtp_client_rsp_state == NX_SMTP_CLIENT_STATE_COMPLETED_NORMALLY) 
            {
    
                /* Yes, time to close it down and return to an idle state. */
                close_connection = NX_TRUE;
            }
            /* else keep the connection open... */
        }

        /* Check again if we need to break down the connection. */
        if (close_connection == NX_TRUE)
        {

            UINT timeout = 0;

            /* We do. Depending on the reason for closing the connection, set the timeout. If we experienced
               an internal error, e.g. packet allocation error, we probably won't be able to send 
               a fin or rst packet anyway, so set the timeout to zero and close down the connection. 

               RFC 2821 Section 3.9: if client must abort processing due to internal conditions or socket reset,
               it should handle as error code 451 from the server which is to abort processing immediately. 
             */   
            if (status != NX_SUCCESS)
            {
                timeout = NX_NO_WAIT;
            }
            else
            {
                timeout =  NX_SMTP_CLIENT_DISCONNECT_TIMEOUT;
            }

            /* Disconnect client socket from server.  */
            nx_tcp_socket_disconnect(&client_ptr -> nx_smtp_client_socket, timeout);
    
            /* Unbind the port from client socket.  */
            nx_tcp_client_socket_unbind(&client_ptr -> nx_smtp_client_socket);

            /* Indicate the Client is idle. */
            client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;

            /* Did the session terminate normally (e.g. no internal errors)? */
            if (status == NX_SUCCESS)
            {

                /* Yes, so set the mail transmit status as the completion status. */
                status = client_ptr -> nx_smtp_client_mail_status;
            }

            return status;  
        } 

        /* Clear the abort status. */
        close_connection = NX_FALSE;
    } 

    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_greeting                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is the start of the SMTP Client process for sending an mail    */
/*    item. It sets the client state to send a greeting command to the    */
/*    server.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_greeting(NX_SMTP_CLIENT *client_ptr)
{


    /* Set session state to wait on the outcome of the greeting response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    /* Return successful session status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_idle                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Execute the idle state of the SMTP Client (do nothing while waiting */
/*    for the next send mail request.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_idle(NX_SMTP_CLIENT *client_ptr)
{

    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_IDLE;

    /* Return successful session status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_idle                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Execute the idle state of the SMTP Client (waiting for the next     */
/*    send mail request).  The state machine has a 'response' state for   */
/*    every command state, so this is the response handler for the idle   */
/*    state.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_idle(NX_SMTP_CLIENT *client_ptr)
{

    client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_IDLE;  

    /* Return successful session status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_greeting                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This handles the server's response to the client greeting and set   */
/*    the next session state and next command to send to the server.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                    Successful completion status          */
/*    status                        Actual completion status              */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_read_server_code                                   */
/*                                  Extracts the server reply code and    */
/*                                  stores reply text to session buffer   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process        Runs the SMTP state machine          */
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
UINT  _nx_smtp_rsp_greeting(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;


    /* Get the server greeting message.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_GREETING_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return (status);
    }

    /* Process based on the reply protocol code from server.  */
    if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_GREETING_OK)       
    {

        /* Yes, set session state to next state, EHLO.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_EHLO;
    }
    else
    {
      
        /* All other codes.  */

        /* Server did not accept the greeting, exit session gracefully.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;
        
        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_GREET_REPLY_ERROR;

        
    }

    if (client_ptr -> nx_smtp_server_packet)
    {
        nx_packet_release(client_ptr -> nx_smtp_server_packet);
    }
    /* Return successful session state.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_helo                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Create the HELO command text and send to the SMTP server.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status.     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Sends data to server          */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_helo(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;
UINT     domain_length;


    memset(&_nx_smtp_buffer[0], 0, NX_SMTP_BUFFER_SIZE);

    if (_nx_utility_string_length_check(client_ptr -> nx_smtp_client_domain, &domain_length, NX_SMTP_CLIENT_MAX_USERNAME))
    {
        return(NX_SIZE_ERROR);
    }

    if (NX_SMTP_BUFFER_SIZE < (sizeof(NX_SMTP_COMMAND_HELO) + domain_length + sizeof(NX_SMTP_LINE_TERMINATOR) - 1))
    {

        /* Buffer size too small. */
        return(NX_SMTP_INTERNAL_ERROR);
    }

    /* Format the HELO command.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_HELO, sizeof(NX_SMTP_COMMAND_HELO) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_HELO) - 1;

    _nx_smtp_buffer[index++] = ' ';

    memcpy(&_nx_smtp_buffer[index],  client_ptr -> nx_smtp_client_domain, domain_length); /* Use case of memcpy is verified. */
    index += domain_length;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the HELO command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait next response from server.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_helo                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the server reply to the HELO command and set  */
/*    the next session state and next command to send to the server.  This*/
/*    is the GREETING state where we do not initiate the greeting.        */
/*    Instead we wait for the server's 'greeting' message.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                    Successful completion status          */
/*    status                        Actual completion status              */ 
/*                                                                        */
/*  CALLS                                                                 */
/*    _nx_smtp_parse_response       Parse specified argument from buffer  */
/*    _nx_smtp_rsp_hello_command    Handle HELO/EHLO reply from Server    */
/*    nx_packet_release             Release NetX receive packet           */
/*    memset                        Clear specified area of memory        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process       Runs the SMTP state machine           */
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
UINT  _nx_smtp_rsp_helo(NX_SMTP_CLIENT *client_ptr)
{

UINT        status;

    /* Get the server response to the EHLO command.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS) 
    { 

        /* This is either no packet received or an invalid server response.  */ 

        /* Return error status.  */ 
        return status; 
    } 

    /* Update session with specific services offered, if any, by server.  */
    _nx_smtp_utility_parse_server_services(client_ptr);

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

        /* if server rejected hello, set the session state to terminate */
    if (client_ptr -> nx_smtp_client_rsp_state == NX_SMTP_CLIENT_STATE_QUIT)
    {

        /* Return successful session status to execute QUIT command.  */
        return NX_SUCCESS;
    }

    /* Handle the server reply code first.  */
    _nx_smtp_rsp_hello_command(client_ptr);

    /* if server rejected hello, set the session state to terminate */
    if (client_ptr -> nx_smtp_client_rsp_state == NX_SMTP_CLIENT_STATE_QUIT)
    {

        /* Return successful session status to execute QUIT command.  */
        return NX_SUCCESS;
    }

    /* Server accepted the HELO command. Continue the SMTP session.  */

    /* Return successful completion status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_ehlo                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the EHLO command and sends it to the server.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_ehlo(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;
UINT     domain_length;

    
    memset(&_nx_smtp_buffer[0], 0, NX_SMTP_BUFFER_SIZE);

    if (_nx_utility_string_length_check(client_ptr -> nx_smtp_client_domain, &domain_length, NX_SMTP_CLIENT_MAX_USERNAME))
    {
        return(NX_SIZE_ERROR);
    }

    if (NX_SMTP_BUFFER_SIZE < (sizeof(NX_SMTP_COMMAND_EHLO) + domain_length + sizeof(NX_SMTP_LINE_TERMINATOR) - 1))
    {

        /* Buffer size too small. */
        return(NX_SMTP_INTERNAL_ERROR);
    }
    
    /* Format the EHLO command.  */
    memcpy(&_nx_smtp_buffer[0],  NX_SMTP_COMMAND_EHLO, sizeof(NX_SMTP_COMMAND_EHLO) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_EHLO) - 1;

    _nx_smtp_buffer[index++] = ' ';
    memcpy(&_nx_smtp_buffer[index],  client_ptr -> nx_smtp_client_domain, domain_length); /* Use case of memcpy is verified. */
    index += domain_length;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;
    
    /* Send the EHLO command.  */
    status = _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);
    
    /* Check for error.  */
    if (status != NX_SUCCESS)
    {
    
        return status;
    }

    /* Set session state to wait on the outcome of the EHLO response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    
    /* Return normal session status.  */
    return NX_SUCCESS;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_ehlo                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This handles the server reply to the EHLO command and determines the*/
/*    next session state and next command to send to the server.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_parse_server_services  Parser for extracting server */
/*                                                services and reply code */
/*   _nx_smtp_utility_read_server_code       Parse the server replies     */
/*                                                code and text           */
/*   _nx_smtp_rsp_hello_command              Server HELO/EHLO reply       */
/*                                                handler                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_ehlo(NX_SMTP_CLIENT *client_ptr)
{

UINT            status;


    /* Get the server response to the EHLO command.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS) 
    { 

        /* This is either no packet received or an invalid server response.  */ 

        /* Return error status.  */ 
        return status; 
    } 
 
    /* Update session with specific services offered, if any, by server.  */
    _nx_smtp_utility_parse_server_services(client_ptr);

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    if (client_ptr -> nx_smtp_client_mute)
    {
        /* We expect another response packet from the server...
           keep the same state, don't send anything. */
        return NX_SUCCESS;
    }

    /* Handle the server reply code first.  */
    _nx_smtp_rsp_hello_command(client_ptr);

    /* if server rejected hello, set the session state to terminate */
    if (client_ptr -> nx_smtp_client_rsp_state == NX_SMTP_CLIENT_STATE_QUIT)
    {

        /* Return successful session status to execute QUIT command.  */
        return NX_SUCCESS;
    }

    /* Determine if we go to authenticatio next (otherwise we go to the MAIL state).   */
    if (client_ptr -> nx_smtp_client_authentication_type != NX_SMTP_CLIENT_AUTH_NONE)
    {

        /* Yes, set session to the AUTH state before going on to MAIL.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_AUTH;                        
    }

    /* Return successful completion status.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_rsp_hello_command                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles the server response to HELO and EHLO commands */
/*    common to both command states.  If server accepts the EHLO/HELO     */
/*    command, proceed with the session                                   */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    session_ptr                   Session to send mail to SMTP Server   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                    Successful completion status          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                  Session server EHLO reply handler     */ 
/*    _nx_smtp_rsp_ehlo             Session server HELO reply handler     */ 
/*    _nx_smtp_rsp_helo                                                   */ 
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
UINT  _nx_smtp_rsp_hello_command(NX_SMTP_CLIENT* client_ptr)
{

UINT     first_digit_server_reply;

    first_digit_server_reply = client_ptr -> nx_smtp_client_reply_code_status/ 100;

    /* Process the server reply starting with the first digit.  */
    if (first_digit_server_reply == 2)
    {


        /* Did server accept the EHLO/HELO command?  */
        if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_OK_TO_CONTINUE)
        {

            /* Set session to the next state (MAIL).  */
            client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_MAIL;

            /* Return successful session status.  */
            return NX_SUCCESS;
        }
    }

    /* If we are here, an error occurred. Indicate mail cannot be sent. */
    client_ptr -> nx_smtp_client_mail_status = NX_SMTP_HELLO_REPLY_ERROR;

    /* Set session state to QUIT.  */
    client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT; 

    /* Return successful session status.  */
    return NX_SUCCESS;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_auth                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the text of the SMTP AUTH command text and    */
/*    sends it to the SMTP server. If the session is already              */ 
/*    authenticated, this function proceeds to the MAIL state (next stage */
/*    of the SMTP protocol) instead.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_cmd_mail                     Send MAIL command to server   */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memset                                Clear area of memory          */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP session         */
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
UINT  _nx_smtp_cmd_auth(NX_SMTP_CLIENT *client_ptr)
{

UINT            status = NX_SUCCESS;
UINT            index;

    memset(&_nx_smtp_buffer[0], 0, NX_SMTP_BUFFER_SIZE);

    /* Is session already successfully authenticated?  */
    if (client_ptr -> nx_smtp_client_authentication_state == NX_SMTP_AUTHENTICATION_SUCCEEDED)
    {

        /* Yes, server will reject any authentication command; skip to MAIL state.  */
        client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_MAIL;
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_MAIL;

        /* Call the session MAIL service.  */
        _nx_smtp_cmd_mail(client_ptr);

        /* Return successful session status.  */
        return NX_SUCCESS;
    }

    /* Mark the session authentication as in progress.  */
    client_ptr -> nx_smtp_client_authentication_state = NX_SMTP_AUTHENTICATION_IN_PROGRESS;

    /* Set the authentication prompt depending on the Client authentication type. */
    if (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_LOGIN)
    {

        memcpy(&_nx_smtp_buffer[0],  NX_SMTP_CLIENT_AUTH_LOGIN_TEXT, sizeof(NX_SMTP_CLIENT_AUTH_LOGIN_TEXT) - 1); /* Use case of memcpy is verified. */
        index = sizeof(NX_SMTP_CLIENT_AUTH_LOGIN_TEXT) - 1;
    }
    else if (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_CRAM_MD5)
    {

        memcpy(&_nx_smtp_buffer[0],  NX_SMTP_CLIENT_AUTH_CRAM_MD5_TEXT, sizeof(NX_SMTP_CLIENT_AUTH_CRAM_MD5_TEXT) - 1); /* Use case of memcpy is verified. */
        index = sizeof(NX_SMTP_CLIENT_AUTH_CRAM_MD5_TEXT) - 1;
    }
    else 
    {

        memcpy(&_nx_smtp_buffer[0],  NX_SMTP_CLIENT_AUTH_PLAIN_TEXT, sizeof(NX_SMTP_CLIENT_AUTH_PLAIN_TEXT) - 1); /* Use case of memcpy is verified. */
        index = sizeof(NX_SMTP_CLIENT_AUTH_PLAIN_TEXT) - 1;
    }

    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the AUTH command to the server.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait on the outcome of the EHLO response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    
    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_auth                                   PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the server reply to the AUTH command and set  */
/*    determine the next session state and next command to send to the    */
/*    server.                                                             */
/*                                                                        */
/*    During the authentication process, it attempts to find a match for  */
/*    the server authentication type and decodes each server              */
/*    authentication challenge (e.g. USERNAME, PASSWORD). It then sets the*/
/*    session authentication state depending on server acceptance of its  */
/*    replies to its challenges.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                           Successful completion status   */
/*    status                               Actual completion status       */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_read_server_code     Parser for server reply       */
/*    _nx_smtp_utility_authentication_challenge                           */
/*                                          Respond to server             */
/*                                                authentication challenge*/
/*    _nx_smtp_rsp_auth                     When called by itself,respond */
/*                                               to server authentication */
/*                                               challenge                */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    nx_packet_release                     Release NetX receive packet   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  03-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_smtp_rsp_auth(NX_SMTP_CLIENT *client_ptr)
{

UINT        status;
UINT        server_reply_first_digit; 
UCHAR       *carriage_return_linefeed_ptr = NX_NULL;
UINT        auth_length;


    /* Get the server response to the AUTH command.  */
    status   =  _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* This is an server response or no packet received.  */ 

        /* Return error status.  */
        return(status);
    }

    /* Use the first digit to group possible server replies.  */
    server_reply_first_digit = client_ptr -> nx_smtp_client_reply_code_status / 100;

    /* Process the reply code from server.  */
    if (server_reply_first_digit == 2)       
    {

        if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_AUTHENTICATION_SUCCESSFUL)
        {

            /* Advance to the MAIL state.  */
            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_MAIL;

            /* Mark the session as authenticated successfully.  */
            client_ptr -> nx_smtp_client_authentication_state = NX_SMTP_AUTHENTICATION_SUCCEEDED;
        }
        else
        {             

            /* The server returns a 2xx code. Determine if it is an error code. */
            if ((client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_CANNOT_VERIFY_RECIPIENT) ||
                (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_ACKNOWLEDGE_QUIT)) 
            {

                /* It is. Abort. */
                client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

                /* Indicate mail cannot be sent. */
                client_ptr -> nx_smtp_client_mail_status = NX_SMTP_AUTH_REPLY_ERROR;
            }
            else
            {
                /* It is not the code expected. Discard and keep the SMTP Client in the same state. */
            }

            /* We are done with the packet now. */
            nx_packet_release(client_ptr -> nx_smtp_server_packet);

            return NX_SUCCESS;
        }
     }
     else if (server_reply_first_digit == 3)
     {

        /* Check if the server accepted our authentication type. */
        if (client_ptr -> nx_smtp_client_reply_code_status != NX_SMTP_CODE_AUTHENTICATION_TYPE_ACCEPTED)
        {

            /* It did not.  Or we may be out of synch with the SMTP Server, if we get a 
               354 message for example. Abort the mail session. */
            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

            /* Indicate mail cannot be sent. */
            client_ptr -> nx_smtp_client_mail_status = NX_SMTP_AUTH_REPLY_ERROR;

            /* We are done with the packet now. */
            nx_packet_release(client_ptr -> nx_smtp_server_packet);

            return NX_SUCCESS;
        }
        /* Authentication in progress.  */

     }
     else
     {

        /* Set session to the QUIT state.  */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_AUTH_REPLY_ERROR;

        /* We are done with the packet now. */
        nx_packet_release(client_ptr -> nx_smtp_server_packet);

        return NX_SUCCESS;
    }

    /* Is authentication in progress?  */    
    if (client_ptr -> nx_smtp_client_authentication_state != NX_SMTP_AUTHENTICATION_SUCCEEDED) 
    {

        /* Set allowed authentication string length.  */
        auth_length = client_ptr -> nx_smtp_server_packet -> nx_packet_length;

        /* Find the end (e.g. CRLF) of the server reply.  */
        _nx_smtp_find_crlf(client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr, 
                           auth_length, &carriage_return_linefeed_ptr, NX_FALSE);                         

        /* Check for invalid server reply (e.g. missing CRLF).  */
        if (carriage_return_linefeed_ptr== NX_NULL)
        {

            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

            /* Indicate mail cannot be sent. */
            client_ptr -> nx_smtp_client_mail_status = NX_SMTP_SERVER_ERROR_REPLY;

            /* We are done with the packet now. */
            nx_packet_release(client_ptr -> nx_smtp_server_packet);

            /* Return completion status.  */
            return NX_SUCCESS;
        }

        /* Yes, process (decode) the server's challenge.  */ 
        status = _nx_smtp_utility_authentication_challenge(client_ptr, client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr, 
                                                           (UINT)(carriage_return_linefeed_ptr - client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr));

        /* Did the session successfully handle server challenge?  */
        if (status == NX_SUCCESS)
        {

            /* Yes, set session to respond to the next server challenge.  */
            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_AUTH_CHALLENGE;
        }
        /* Did the session have problems parsing the server challenge?  */
        else 
        {

            client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

            /* Indicate mail cannot be sent. */
            client_ptr -> nx_smtp_client_mail_status = NX_SMTP_AUTH_REPLY_ERROR;

            nx_packet_release(client_ptr -> nx_smtp_server_packet);

            /* Return completion status.  */
            return NX_SUCCESS;
        }
    }

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);
    
    /* Return successful status.  */
    return NX_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_auth_challenge                         PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a reply to the server authentication challenge*/ 
/*    It determines which challenge the server sent (e.g Username,        */
/*    Password) and encodes the session response using base64 encryption. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    _nx_utility_base64_encode             Base64 encode specified text  */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP session         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            parsing base64,             */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_smtp_cmd_auth_challenge(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;
UCHAR    auth_reply_encoded[NX_SMTP_CLIENT_AUTH_CHALLENGE_ENCODED_SIZE + 1];
UINT     auth_reply_encoded_size;
UINT     length;
UINT     password_length;

    /* Initialize the encoded reply to NULL.  */
    memset(auth_reply_encoded, 0, sizeof(auth_reply_encoded));

    /* Verify string length.  */
    if (_nx_utility_string_length_check(client_ptr -> nx_smtp_password, &password_length, NX_SMTP_CLIENT_MAX_PASSWORD))
    {
        return(NX_SIZE_ERROR);
    }

    /* Did the server send us the Username prompt?  */
    if (client_ptr -> nx_smtp_client_authentication_reply == NX_SMTP_CLIENT_REPLY_TO_USERNAME_PROMPT)
    {
        if (_nx_utility_string_length_check(client_ptr -> nx_smtp_username, &length, NX_SMTP_CLIENT_MAX_USERNAME))
        {
            return(NX_SIZE_ERROR);
        }


        if (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_PLAIN)
        {

            UCHAR plain_auth_buffer[NX_SMTP_CLIENT_AUTH_CHALLENGE_SIZE];  

            if (NX_SMTP_CLIENT_AUTH_CHALLENGE_SIZE < (length + 1 + length + 1 + password_length))
            {

                /* Buffer size too small. */
                return(NX_SMTP_INTERNAL_ERROR);
            }

            memset(&plain_auth_buffer[0], 0, NX_SMTP_CLIENT_AUTH_CHALLENGE_SIZE);

            /* Process the client name as per AUTH PLAIN syntax: authorization-id\0authentication-id\0password. */
            memcpy(&plain_auth_buffer[0], client_ptr -> nx_smtp_username,  length); /* Use case of memcpy is verified. */
            length++;
            memcpy(&plain_auth_buffer[length], client_ptr -> nx_smtp_username,  length);  /* Use case of memcpy is verified. */
            length += length;
            memcpy(&plain_auth_buffer[length], client_ptr -> nx_smtp_password, password_length); /* Use case of memcpy is verified. */
            length += password_length;
    
            /* Now encode the combined client username/password.  */
            _nx_utility_base64_encode(&plain_auth_buffer[0], length, auth_reply_encoded, sizeof(auth_reply_encoded), &auth_reply_encoded_size);

        }
        else
        {

            /* Just encode the client username.  */
            _nx_utility_base64_encode((UCHAR *)client_ptr -> nx_smtp_username, length, auth_reply_encoded, sizeof(auth_reply_encoded), &auth_reply_encoded_size);
        }

    }
    /* Or did the server send us the Password prompt?  */
    else if (client_ptr -> nx_smtp_client_authentication_reply == NX_SMTP_CLIENT_REPLY_TO_PASSWORD_PROMPT)
    {

        /* Encode the client password.  */
        _nx_utility_base64_encode((UCHAR *)client_ptr -> nx_smtp_password, password_length, auth_reply_encoded, sizeof(auth_reply_encoded), &auth_reply_encoded_size);
    }

    else 
    {

        /* Unknown prompt: Send the '*' to terminate the authentication process.  */
        memcpy(auth_reply_encoded, NX_SMTP_CANCEL_AUTHENTICATION,  sizeof(NX_SMTP_CANCEL_AUTHENTICATION)); /* Use case of memcpy is verified. */
        auth_reply_encoded_size = sizeof(NX_SMTP_CANCEL_AUTHENTICATION) - 1;
    }

    if (sizeof(_nx_smtp_buffer) < (auth_reply_encoded_size + sizeof(NX_SMTP_LINE_TERMINATOR) - 1))
    {

        /* Buffer size too small. */
        return(NX_SMTP_INTERNAL_ERROR);
    }

    /* Format the encoded response.  */
    memcpy(&_nx_smtp_buffer[0],auth_reply_encoded, auth_reply_encoded_size); /* Use case of memcpy is verified. */
    index = auth_reply_encoded_size;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the response back to the server.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for successful.  */
    if (status == NX_SUCCESS)
    {

        /* Set session state to wait on the outcome of the AUTH challenge handler.  */
        client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    }

    /* Return completion status.  */
    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_auth_challenge                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This receives the server's response to the AUTH challenge sent by   */       
/*    the session previously and actually just calls the same session     */
/*    AUTH server reply handler again.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                      SMTP Session for sending mail      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                           Actual completion status           */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_rsp_auth                 AUTH server reply handler          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process          Runs the SMTP state machine        */
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
UINT  _nx_smtp_rsp_auth_challenge(NX_SMTP_CLIENT *client_ptr)
{

UINT status;


    /* Run the session AUTH server reply handler directly.  */
    status = _nx_smtp_rsp_auth(client_ptr);

    /* Return completion status.  */
    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_mail                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the MAIL command and sets the Client state to */
/*    send it to the SMTP server.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_mail(NX_SMTP_CLIENT *client_ptr)
{

UINT index;   
UINT status;
UINT mail_from_length;
NX_SMTP_CLIENT_MAIL *mail_ptr = &client_ptr -> nx_smtp_client_mail;


    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    if (_nx_utility_string_length_check(mail_ptr -> nx_smtp_client_mail_from_address, &mail_from_length, NX_SMTP_BUFFER_SIZE))
    {
        return(NX_SIZE_ERROR);
    }

    /* Check if authentication status still left as 'in progress'.  */
    if (client_ptr -> nx_smtp_client_authentication_state == NX_SMTP_AUTHENTICATION_IN_PROGRESS)
    {

        /* For some reason, authentication did not complete. Reset status here.  */
        client_ptr -> nx_smtp_client_authentication_state = NX_SMTP_NOT_AUTHENTICATED;    
    }

    /* Validate message size. */
    if((sizeof(NX_SMTP_COMMAND_MAIL) + mail_from_length +
        sizeof(NX_SMTP_LINE_TERMINATOR) + 2) > NX_SMTP_BUFFER_SIZE)
    {
        /* Message size exceeds buffer size. */
        return(NX_SMTP_INTERNAL_ERROR);
    }

    /* Create the command text.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_MAIL, sizeof(NX_SMTP_COMMAND_MAIL) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_MAIL) - 1;

    _nx_smtp_buffer[index++] = ':';
    _nx_smtp_buffer[index++] = '<';
    memcpy(&_nx_smtp_buffer[index], mail_ptr -> nx_smtp_client_mail_from_address, mail_from_length); /* Use case of memcpy is verified. */
    index += mail_from_length; 
    _nx_smtp_buffer[index++] = '>';

    _nx_smtp_buffer[index++] = ' ';
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the session command we constructed above.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.*/
        return status;
    }

    /* Set session state to wait on the outcome of the MAIL response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    /* Return successful session status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_mail                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This receives the server's response to the MAIL command; it         */
/*    extracts the server reply code and reply text and store both to the */
/*    Client session. It will also set the next SMTP command to send to   */
/*    the server.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_read_server_code     Parse the server reply        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_mail(NX_SMTP_CLIENT *client_ptr)
{

UINT  status;

    /* Get the server response to the MAIL command.  */
    status   =  _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* This is an invalid line or corrupted transmission from the server.  */ 

        /* Return error status.  */
        return(status);
    }

    /* Did server accept the MAIL command?  */
    if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_OK_TO_CONTINUE)
    {

        /* Yes, set the session to the RCPT state.  */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_RCPT;
    }
    else
    {

        /* Set session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;
    
        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_MAIL_REPLY_ERROR;
    }

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    /* Return successful session status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_rcpt                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the RCPT command and sets the Client state to */
/*    send it to the SMTP server.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_rcpt(NX_SMTP_CLIENT *client_ptr)
{

UINT                        status;
UINT                        index;
UINT                        recipient_length;
NX_SMTP_CLIENT_MAIL         *mail_ptr;


    mail_ptr = &(client_ptr -> nx_smtp_client_mail);

    if (_nx_utility_string_length_check(mail_ptr -> nx_smtp_client_mail_recipient_address, &recipient_length, NX_SMTP_BUFFER_SIZE))
    {
        return(NX_SIZE_ERROR);
    }

    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    /* Validate message size. */
    if((sizeof(NX_SMTP_COMMAND_RCPT) + recipient_length +
        sizeof(NX_SMTP_LINE_TERMINATOR) + 2) > NX_SMTP_BUFFER_SIZE)
    {
        /* Message size exceeds buffer size. */
        return(NX_SMTP_INTERNAL_ERROR);
    }

    /* Create the command text.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_RCPT, sizeof(NX_SMTP_COMMAND_RCPT) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_RCPT) - 1; 

    _nx_smtp_buffer[index++] = ':';
    _nx_smtp_buffer[index++] = '<';
    memcpy(&_nx_smtp_buffer[index],  mail_ptr -> nx_smtp_client_mail_recipient_address, /* Use case of memcpy is verified. */
           recipient_length);
    index += recipient_length;
    _nx_smtp_buffer[index++] = '>';
    _nx_smtp_buffer[index++] = ' ';
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the RCPT command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait next response from server.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_rcpt                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This service handles the server's response to the RCPT command and  */
/*    set the next session state and next command to send to the server.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                        SMTP Session for sending mail    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                         Successful completion status     */
/*    status                             Actual completion status         */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_read_server_code   Parse the server reply           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process            Runs the SMTP state machine      */
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
UINT  _nx_smtp_rsp_rcpt(NX_SMTP_CLIENT *client_ptr)
{

UINT  status;


    /* Get server response to RCPT command.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Did server accept RCPT command?  */
    if (client_ptr -> nx_smtp_client_reply_code_status != NX_SMTP_CODE_OK_TO_CONTINUE)
    {   
        /* NO, Set session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_RCPT_REPLY_ERROR;
    }  
    else
    {
    
        /* Ok for server to send the mail message */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_DATA;  
    }

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    /* Return successful session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_data                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the DATA command text, verifies the mail to send */
/*    has a valid recipient and sends it to the  SMTP server.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send SMTP commend to server   */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_data(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;


    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    /* Format the DATA command.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_DATA, sizeof(NX_SMTP_COMMAND_DATA) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_DATA) - 1;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the DATA command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait next response from server.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_data                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the server reply to the DATA command and set  */
/*    the next SMTP Client command to send to the server.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_read_server_code      Parse the server reply        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_data(NX_SMTP_CLIENT *client_ptr)
{

UINT            status;


    /* Get the response to DATA command.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for errors.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return (status);
    }

    if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_SEND_MAIL_INPUT)
    {

        /* Yes, set session state to next step.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_MESSAGE;  
    }
    else
    {

        /* No, any other 3xy code in inappropriate. Quit the session.  */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_DATA_REPLY_ERROR;
    }

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    /* Return successful session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_message                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the mail message text handling for message    */
/*    text.  It attempts to fit as much of each message into each packet  */
/*    as possible.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           Session for sending mail      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_NOT_ENABLED                        IPv6 not enabled              */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_message(NX_SMTP_CLIENT *client_ptr)
{

UINT                        status;
ULONG                       data_left_to_transmit;
ULONG                       packet_payload;
CHAR                        *data_to_send;
NX_PACKET_POOL              *pool_ptr;
NX_SMTP_CLIENT_MAIL         *mail_ptr;


    /* Set local variable for convenience.  */
    mail_ptr = &client_ptr -> nx_smtp_client_mail;

    /* Set session state to wait next response from server.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;

    pool_ptr  = client_ptr -> nx_smtp_client_packet_pool_ptr; 

    /* Compute packet payload after IP and TCP headers are subtracted.  */
    if (client_ptr -> nx_smtp_server_address.nxd_ip_version == NX_IP_VERSION_V4)
    {

#ifndef NX_DISABLE_IPV4
        packet_payload = pool_ptr -> nx_packet_pool_payload_size - sizeof(NX_TCP_HEADER) - sizeof(NX_IPV4_HEADER);
#else
        return NX_NOT_ENABLED;
#endif /* NX_DISABLE_IPV4 */
    }
    else
    {
#ifdef FEATURE_NX_IPV6
        packet_payload = pool_ptr -> nx_packet_pool_payload_size - sizeof(NX_TCP_HEADER) - sizeof(NX_IPV6_HEADER);
#else
        return NX_NOT_ENABLED;
#endif
    }


    /* Send the Mail header */
    status = _nx_smtp_utility_send_header_to_server(client_ptr, NX_SMTP_CLIENT_SEND_TIMEOUT);
    if(status)
        return(status);

    /* Set up local variable of size of mail message. */
    data_left_to_transmit = mail_ptr -> nx_smtp_client_mail_body_length;

    /* Set a local pointer to (rest of) mail message to send. */
    data_to_send = mail_ptr -> nx_smtp_client_mail_body;

    /* Send the next chunk of mail text buffer till nothing is left.  */
    while(data_left_to_transmit)
    {

        /* Check if remaining mail text will fit in a packet.*/
        if (packet_payload >= data_left_to_transmit)
        {

            /* Yes, send it off! This will load the specified data into an packet from the SMTP client packet pool.  */
            status =  _nx_smtp_utility_send_to_server(client_ptr, data_to_send, data_left_to_transmit, 
                                                      NX_SMTP_CLIENT_SEND_TIMEOUT);

            /* Check for error.  */
            if (status != NX_SUCCESS)
            {

                /* Return error status.  */
                return status;
            }

            break;
        }

        /* No, break up mail message into multiple packets.  */
        else
        {
    
            /* Fill up the packet with as much data as it will hold and send it off! */
            status =  _nx_smtp_utility_send_to_server(client_ptr, data_to_send, packet_payload, 
                                                      NX_SMTP_CLIENT_SEND_TIMEOUT);

            /* Check for error.  */
            if (status != NX_SUCCESS)
            {

                /* Return error status.  */
                return status;
            }

            /* Update pointer to remainder of message to send, update how much is left to send. */
            data_left_to_transmit -= packet_payload;  
            data_to_send += packet_payload;
        }
    }

    /* Send end of message (EOM) to server or it will handle subsequent commands as message text.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, NX_SMTP_EOM, sizeof(NX_SMTP_EOM) - 1, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Return normal session status.  */
    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_message                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This receives the server's response to sending it mail message data.*/
/*    It extracts the server reply code and reply text and store both to  */
/*    the Client session. It then sets the next session state, sets up    */
/*    the next command to send to the server.                             */
/*                                                                        */
/*    If the server accepts the mail message, this function determines if */
/*    if there is more session mail to transact, and if so sets the next  */
/*    Client mail.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           Session for sending mail      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_read_server_code     Parse the server reply code   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_message(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;


    /* Get the server reply to receiving session mail message data.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_MESSAGE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Process server reply code by first digit.  */
    if (client_ptr -> nx_smtp_client_reply_code_status == NX_SMTP_CODE_OK_TO_CONTINUE)
    {

        /* Message accepted by server. Mark the Client mail as sent.  */
        client_ptr -> nx_smtp_client_mail_status = NX_SUCCESS;
    }
    else
    {

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_MESSAGE_REPLY_ERROR;
    }

    /* We're done with this session */
    client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    /* Return successful session status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_quit                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the QUIT command text and and sends the Quit  */
/*    command to the  SMTP server.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_quit(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;


    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    /* Format the QUIT command.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_QUIT, sizeof(NX_SMTP_COMMAND_QUIT) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_QUIT) - 1;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR)); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the QUIT command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return status;
    }

    /* Set session state to wait on the outcome of the EHLO response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    
    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_quit                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the server reply to the SMTP QUIT command and */
/*    sets the session state to the session has terminated normally.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_read_server_code      Parse the server reply        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_quit(NX_SMTP_CLIENT *client_ptr)
{

UINT    status;


    /* Get server response to QUIT command.  */
    status = _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error status.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Set the session state to normal termination. Normal is when the SMTP Client can
       receive SMTP server replies and regardless if mail can be sent, can at least
       terminate the session with the Quit command and normal TCP socket disconnect.  */
    client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_COMPLETED_NORMALLY; 

    /* We are done with the packet now. */
    nx_packet_release(client_ptr -> nx_smtp_server_packet);

    /* Return successful session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_rset                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This functions creates the RSET command for the Client task to send */
/*    it to the SMTP server.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_rset(NX_SMTP_CLIENT *client_ptr)
{

ULONG    timeout;
UINT     status;
UINT     index;


    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    /* Get TCP send timeout.  */
    timeout =  NX_SMTP_ENVELOPE_TIMEOUT;

    /* Format the RSET command.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_RSET, sizeof(NX_SMTP_COMMAND_RSET) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_RSET) - 1;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof( NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the RSET command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, timeout);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait on the outcome of the EHLO response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    
    /* Return normal session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_rset                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the RSET server reply.                        */
/*                                                                        */
/*    Resetting an SMTP session means clearing the Client mail transaction*/
/*    including recipients and mail text.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_read_server_code      Parse the server reply        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_rsp_rset(NX_SMTP_CLIENT *client_ptr)
{

UINT  status;


    /* Get server response to RSET command.  */
    status =  _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return error status.  */
        return(status);
    }

    /* Did the session receive the 250 OK from the server?  */  
    if (client_ptr -> nx_smtp_client_reply_code_status != NX_SMTP_CODE_OK_TO_CONTINUE)
    {

        /* Yes, set the session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_QUIT;            
        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_SERVER_ERROR_CODE_RECEIVED;
    }
    else
    {

        /* Yes, session accepted the RSET command with the 250 code.  
           Reset session state back to MAIL.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_MAIL;

    }

    /* Return successful session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_cmd_noop                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates the SMTP NOOP command text and sends it to the*/
/*    SMTP server.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           SMTP Session for sending mail */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_utility_send_to_server       Send data to server           */
/*    memcpy                                Copy data to area of memory   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process               Runs the SMTP state machine   */
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
UINT  _nx_smtp_cmd_noop(NX_SMTP_CLIENT *client_ptr)
{

UINT     status;
UINT     index;


    memset(_nx_smtp_buffer, 0, NX_SMTP_BUFFER_SIZE);

    /* Format the NOOP command.  */
    memcpy(&_nx_smtp_buffer[0], NX_SMTP_COMMAND_NOOP, sizeof(NX_SMTP_COMMAND_NOOP) - 1); /* Use case of memcpy is verified. */
    index = sizeof(NX_SMTP_COMMAND_NOOP) - 1;
    memcpy(&_nx_smtp_buffer[index],  NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1); /* Use case of memcpy is verified. */
    index += sizeof(NX_SMTP_LINE_TERMINATOR) - 1;

    /* Send the NOOP command.  */
    status =  _nx_smtp_utility_send_to_server(client_ptr, _nx_smtp_buffer, index, NX_SMTP_CLIENT_SEND_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        return status;
    }

    /* Set session state to wait on the outcome of the EHLO response handler.  */
    client_ptr -> nx_smtp_client_cmd_state =  NX_SMTP_CLIENT_STATE_AWAITING_REPLY;
    
    /* Return normal session status.  */
    return NX_SUCCESS;
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_rsp_noop                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the NOOP server reply. It will not            */
/*    automatically advance the session state like most other SMTP client */
/*    commands. However, an indication of server problems will set the    */ 
/*    session state to QUIT.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                       TCP connection to send mail       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                        Successful completion status      */
/*    status                            Actual completion status          */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_utility_read_server_code  Parse the server reply            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_client_process           Runs the SMTP state machine       */
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
UINT  _nx_smtp_rsp_noop(NX_SMTP_CLIENT *client_ptr)
{

UINT              status;
UINT              first_digit_server_reply;


    /* Get the response for NOOP command.  */
    status =  _nx_smtp_utility_read_server_code(client_ptr, NX_SMTP_ENVELOPE_TIMEOUT, NX_TRUE);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Return the error status.  */
        return(status);
    }

    /* Get category of server reply from the first digit.  */
    first_digit_server_reply = client_ptr -> nx_smtp_client_reply_code_status / 100;

    /* Is the server experiencing problems forcing the session to abort?  */
    if (first_digit_server_reply == 4)
    {

        /* Yes, set the session state to quit.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_SERVER_ERROR_CODE_RECEIVED;
    }

    /* Return successful session status.  */
    return NX_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_utility_read_server_code                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the 3 digit SMTP server code and stores the    */
/*    text of the server reply if there is any to the session buffer.     */
/*    Its use is intended primarily for session server reply handlers.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*    timeout                       Timeout on receiving server reply     */
/*    receive_all_lines             Indicates if SMTP Client should expect*/
/*                                    one packet or multiple packets e.g. */
/*                                    a multi packet server reply.        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                        Successful completion status      */
/*    status                            Actual completion status          */ 
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_smtp_parse_response            Parse an argument from text       */
/*   nx_tcp_socket_receive              Receive data over TCP socket      */
/*   nx_packet_release                  Release packet to packet pool     */
/*   memset                             Clear specified area of memory    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Session server reply handlers     Handle server replies to SMTP     */
/*                                            commands in the session     */
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
UINT  _nx_smtp_utility_read_server_code(NX_SMTP_CLIENT *client_ptr, ULONG timeout, UINT  receive_all_lines)     
{

UINT         status; 
NX_PACKET    *packet_ptr;
UCHAR        server_code[NX_SMTP_SERVER_REPLY_CODE_SIZE + 1];
UCHAR        *work_ptr;
UINT         buffer_length;

    NX_PARAMETER_NOT_USED(receive_all_lines);

    /* Initialize argument to NULL.  */
    memset(server_code, 0, NX_SMTP_SERVER_REPLY_CODE_SIZE + 1);

    /* Process all packets received as part of server reply. A server response may 
       have several 'lines' in it e.g. 250-EHLO\r\n250-AUTH LOGIN\r\n250 HELP, 
      and this may be spread over multiple packets. */

    /* Wait to receive the next packet.  */
    status = nx_tcp_socket_receive(&(client_ptr -> nx_smtp_client_socket), &packet_ptr, timeout);

    /* Check for errors.  */
    if (status != NX_SUCCESS)
    {

        /* Set session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;

        /* Return error status.  */
        return status;
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {
        
        /* Chained packet is not supported. */
        nx_packet_release(packet_ptr);

        /* Set session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_SERVER_ERROR_REPLY; 

        /* While there was an error with the packet, the SMTP Client is ok. So just return
           completion status and we quit the session. */
        return NX_SUCCESS;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Save the location of the server response buffer. */
    client_ptr -> nx_smtp_server_packet = packet_ptr;

    work_ptr = client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr;
    buffer_length = client_ptr -> nx_smtp_server_packet -> nx_packet_length;  

    /* Yes, parse the server code.  Note that with SMTP protocol, the server reply
       may be multiple lines/packets. To determine if we have the end of the reply
       we look for the command code followed by a space (not a hyphen). */
    status = _nx_smtp_parse_response(client_ptr, work_ptr, 1, buffer_length, &server_code[0], 
                                     NX_SMTP_SERVER_REPLY_CODE_SIZE, NX_FALSE);

    /* Was a server reply code found in the first line?  */
    if (status != NX_SUCCESS) 
    {

        /* No, it wasn't. */
        nx_packet_release(packet_ptr);

        /* Set session state to QUIT.  */
        client_ptr -> nx_smtp_client_rsp_state =  NX_SMTP_CLIENT_STATE_QUIT;

        /* Indicate mail cannot be sent. */
        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_SERVER_ERROR_REPLY; 

        /* While there was an error with the packet, the SMTP Client is ok. So just return
           completion status and we quit the session. */
        return NX_SUCCESS;
    }
    
    /* Convert server code from text to unsigned int and store to session.  */
    client_ptr -> nx_smtp_client_reply_code_status =  strtoul((const char *)server_code, NULL, 10); 

    /* Return completion status.  */
    return (NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_utility_send_to_server                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compiles the message/command to send the SMTP server. */
/*    It returns a non success status if packet allocation fails or an    */
/*    error results from creating the SMTP command packet.                */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    session_ptr                   Session to send mail to SMTP Server   */
/*    buffer_ptr                    Pointer to buffer for to send         */ 
/*    buffer_length                 Size of buffer                        */ 
/*    timeout                       Timeout for sending data out          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Successful completion               */ 
/*    status                          Actual completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   nx_packet_allocate              Allocate packet from specified pool  */ 
/*   nx_packet_data_append           Appends buffer to packet data        */
/*   nx_packet_release               Releases send packet back to pool    */ 
/*   nx_tcp_socket_send              Sends packet out over TCP socket     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_smtp_session_run            Runs the SMTP state machine          */
/*   Session server reply handlers   Handle server reply so SMTP command  */ 
/*   Session SMTP command services   Send SMTP commands to server         */ 
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
UINT  _nx_smtp_utility_send_to_server(NX_SMTP_CLIENT *client_ptr, CHAR *buffer_ptr, UINT buffer_length, ULONG timeout) 
{

UINT            status;
NX_PACKET       *packet_ptr;
UINT            packet_type;


    if (client_ptr -> nx_smtp_server_address.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(client_ptr -> nx_smtp_client_packet_pool_ptr,
                                 &packet_ptr, packet_type, NX_SMTP_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Abort the session. If we cannot allocate a packet we cannot send a QUIT message. */   
        client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;

        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_PACKET_ALLOCATE_ERROR;

        /* Return error status.  */
        return(status);
    } 

    /* Add message to packet data.  */
    status = nx_packet_data_append(packet_ptr, buffer_ptr, buffer_length, 
                                   client_ptr -> nx_smtp_client_packet_pool_ptr, 
                                   NX_SMTP_CLIENT_PACKET_TIMEOUT);
    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Abort further processing. This error is most likely a packet allocate. */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;
        client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;

        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_INTERNAL_ERROR;

        /* Return error status.  */
        return(status);
    }

    /* Send the packet out.  */
    status =  nx_tcp_socket_send(&client_ptr -> nx_smtp_client_socket, packet_ptr, timeout);

    /* Check for error.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Abort further processing. If we are unable to send, we won't be able to send a quit message either. */
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;
        client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;

        /* Return error status.  */
        return(status);
    }

    /* Return successful session status.  */
    return(NX_SUCCESS);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_utility_send_to_server                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compiles the message/command to send the SMTP server. */
/*    It returns a non success status if packet allocation fails or an    */
/*    error results from creating the SMTP command packet.                */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    session_ptr                   Session to send mail to SMTP Server   */
/*    buffer_ptr                    Pointer to buffer for to send         */ 
/*    buffer_length                 Size of buffer                        */ 
/*    timeout                       Timeout for sending data out          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                      Successful completion               */ 
/*    status                          Actual completion status            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   nx_packet_allocate              Allocate packet from specified pool  */ 
/*   nx_packet_data_append           Appends buffer to packet data        */
/*   nx_packet_release               Releases send packet back to pool    */ 
/*   nx_tcp_socket_send              Sends packet out over TCP socket     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_smtp_session_run            Runs the SMTP state machine          */
/*   Session server reply handlers   Handle server reply so SMTP command  */ 
/*   Session SMTP command services   Send SMTP commands to server         */ 
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
UINT  _nx_smtp_utility_send_header_to_server(NX_SMTP_CLIENT *client_ptr, ULONG timeout) 
{

UINT                 status;
NX_PACKET           *packet_ptr = NX_NULL;
UINT                 packet_type;
NX_SMTP_CLIENT_MAIL *mail_ptr;
NX_PACKET_POOL      *pool_ptr;
UINT                 recipient_length;
UINT                 mail_from_length;
UINT                 subject_length;


    mail_ptr = &client_ptr -> nx_smtp_client_mail;

    if (client_ptr -> nx_smtp_server_address.nxd_ip_version == NX_IP_VERSION_V6)
        packet_type = NX_IPv6_TCP_PACKET;
    else
        packet_type = NX_IPv4_TCP_PACKET;

    /* Verify string length.  */
    if (_nx_utility_string_length_check(mail_ptr -> nx_smtp_client_mail_recipient_address, &recipient_length, NX_SMTP_BUFFER_SIZE))
    {
        return(NX_SIZE_ERROR);
    }

    if (_nx_utility_string_length_check(mail_ptr -> nx_smtp_client_mail_from_address, &mail_from_length, NX_SMTP_BUFFER_SIZE))
    {
        return(NX_SIZE_ERROR);
    }

    if (_nx_utility_string_length_check(mail_ptr -> nx_smtp_client_mail_subject, &subject_length, NX_SMTP_BUFFER_SIZE))
    {
        return(NX_SIZE_ERROR);
    }


    pool_ptr = client_ptr -> nx_smtp_client_packet_pool_ptr;

    /* Allocate a packet.  */
    status =  nx_packet_allocate(pool_ptr, &packet_ptr, packet_type, NX_SMTP_CLIENT_PACKET_TIMEOUT);

    /* Check for error.  */
    if (status == NX_SUCCESS)
    {
        if (/* Add "To:  <recipient_address> CRLF" */
           (nx_packet_data_append(packet_ptr, NX_SMTP_TO_STRING, sizeof(NX_SMTP_TO_STRING) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, mail_ptr -> nx_smtp_client_mail_recipient_address,
                                  recipient_length,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           /* Add "From:  <from_address> CRLF" */
           (nx_packet_data_append(packet_ptr, NX_SMTP_FROM_STRING, sizeof(NX_SMTP_FROM_STRING) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, mail_ptr -> nx_smtp_client_mail_from_address,
                                  mail_from_length,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           /* Add "Subject:  Subject_line CRLF" */
           (nx_packet_data_append(packet_ptr, NX_SMTP_SUBJECT_STRING, sizeof(NX_SMTP_SUBJECT_STRING) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, mail_ptr -> nx_smtp_client_mail_subject,
                                  subject_length,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) &&
           (nx_packet_data_append(packet_ptr, NX_SMTP_LINE_TERMINATOR, sizeof(NX_SMTP_LINE_TERMINATOR) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS) && 
           /* Add the rest of the mail header components. */
           (nx_packet_data_append(packet_ptr, NX_SMTP_MAIL_HEADER_COMPONENTS, sizeof(NX_SMTP_MAIL_HEADER_COMPONENTS) - 1,
                                  pool_ptr, NX_SMTP_CLIENT_PACKET_TIMEOUT) == NX_SUCCESS))
        {
            /* Send the packet out.  */
            status =  nx_tcp_socket_send(&client_ptr -> nx_smtp_client_socket, packet_ptr, timeout);

            if(status)
            {
                /* Send failed. */
                status = NX_SMTP_INTERNAL_ERROR;
            }
        } 
        else
        {
            /* One of the Mail header components failed to be appended. */
            status = NX_SMTP_INTERNAL_ERROR;
        }
    }

    if(status)
    {

        /* Abort the session. If we cannot allocate a packet we cannot send a QUIT message. */   
        client_ptr -> nx_smtp_client_cmd_state = NX_SMTP_CLIENT_STATE_IDLE;
        client_ptr -> nx_smtp_client_rsp_state = NX_SMTP_CLIENT_STATE_IDLE;

        client_ptr -> nx_smtp_client_mail_status = NX_SMTP_INTERNAL_ERROR;

        if(packet_ptr)
        {
            /* Release the packet.  */
            nx_packet_release(packet_ptr);            
        }
    } 


    /* Return successful session status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_utility_authentication_challenge           PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles parsing and decoding the server challenge,    */
/*    and setting up the client response to the challenge.  It does not   */
/*    alter the session state or determine the next session command.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*    server_challenge              Buffer containing server challenge    */ 
/*    length                        Size of server challenge buffer       */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                     Successful completion status         */
/*    NX_SMTP_PARAM_ERROR            Server parsing (parameter) error     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_smtp_parse_response       Parses argument from buffer text      */
/*    _nx_utility_base64_decode     Decodes base64 text                   */
/*    memset                        Clears specified area of memory       */
/*    memcmp                        Compares data between areas of memory */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_rsp_auth             AUTH server reply handler             */ 
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
UINT  _nx_smtp_utility_authentication_challenge(NX_SMTP_CLIENT *client_ptr, UCHAR *server_challenge, UINT length)
{

UCHAR  encrypted_server_prompt[NX_SMTP_SERVER_CHALLENGE_MAX_STRING + 1];  
UCHAR  decoded_server_prompt[NX_SMTP_SERVER_CHALLENGE_MAX_STRING + 1];  
UINT   encrypted_server_prompt_size;
UINT   decoded_server_prompt_size;

    if (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_PLAIN)
    {
        /* Set the session to reply to a username challenge.  */
        client_ptr -> nx_smtp_client_authentication_reply = NX_SMTP_CLIENT_REPLY_TO_USERNAME_PROMPT;

        /* Return successful session status.  */
        return NX_SUCCESS;
    }

    /* Clear buffers for storing encrypted and decoded server challenges.  */
    memset(encrypted_server_prompt, 0, NX_SMTP_SERVER_CHALLENGE_MAX_STRING + 1);
    memset(decoded_server_prompt, 0, NX_SMTP_SERVER_CHALLENGE_MAX_STRING + 1);

    /* Parse the 2nd argument for the server challenge we need to decode.  */
    _nx_smtp_parse_response(client_ptr, &server_challenge[0], 2, length, &encrypted_server_prompt[0], 
                            NX_SMTP_SERVER_CHALLENGE_MAX_STRING, NX_FALSE);

    /* Was a valid argument parsed?  */
    if (*encrypted_server_prompt == NX_NULL)
    {

        /* No , unable to parse server prompt.  */

        /* Return error status.  */
        return NX_SMTP_INVALID_SERVER_REPLY;
    }

    /* Calculate the name length.  */
    if (_nx_utility_string_length_check((CHAR *)encrypted_server_prompt, &encrypted_server_prompt_size, NX_SMTP_SERVER_CHALLENGE_MAX_STRING))
    {
        return NX_SMTP_INVALID_SERVER_REPLY;
    }

    /* Decode the parsed server challenge.  */
    _nx_utility_base64_decode(encrypted_server_prompt, encrypted_server_prompt_size, decoded_server_prompt, sizeof(decoded_server_prompt), &decoded_server_prompt_size);

    /* Is this a username prompt?  */
    if ((decoded_server_prompt_size == 9) &&
        ((decoded_server_prompt[0] == 'U') || (decoded_server_prompt[0] == 'u')) &&
        ((decoded_server_prompt[1] == 'S') || (decoded_server_prompt[1] == 's')) &&
        ((decoded_server_prompt[2] == 'E') || (decoded_server_prompt[2] == 'e')) &&
        ((decoded_server_prompt[3] == 'R') || (decoded_server_prompt[3] == 'r')) &&
        ((decoded_server_prompt[4] == 'N') || (decoded_server_prompt[4] == 'n')) &&
        ((decoded_server_prompt[5] == 'A') || (decoded_server_prompt[5] == 'a')) &&
        ((decoded_server_prompt[6] == 'M') || (decoded_server_prompt[6] == 'm')) &&
        ((decoded_server_prompt[7] == 'E') || (decoded_server_prompt[7] == 'e')) &&
        (decoded_server_prompt[8] == ':'))
    {

        /* Yes, set the session to reply to a username challenge.  */
        client_ptr -> nx_smtp_client_authentication_reply = NX_SMTP_CLIENT_REPLY_TO_USERNAME_PROMPT;
    }
    /* Is this a password prompt?  */
    else if ((decoded_server_prompt_size == 9) &&
             ((decoded_server_prompt[0] == 'P') || (decoded_server_prompt[0] == 'p')) &&
             ((decoded_server_prompt[1] == 'A') || (decoded_server_prompt[1] == 'a')) &&
             ((decoded_server_prompt[2] == 'S') || (decoded_server_prompt[2] == 's')) &&
             ((decoded_server_prompt[3] == 'S') || (decoded_server_prompt[3] == 's')) &&
             ((decoded_server_prompt[4] == 'W') || (decoded_server_prompt[4] == 'w')) &&
             ((decoded_server_prompt[5] == 'O') || (decoded_server_prompt[5] == 'o')) &&
             ((decoded_server_prompt[6] == 'R') || (decoded_server_prompt[6] == 'r')) &&
             ((decoded_server_prompt[7] == 'D') || (decoded_server_prompt[7] == 'd')) &&
             (decoded_server_prompt[8] == ':'))
    {

        /* Yes, set the session to reply to a password challenge.  */
        client_ptr -> nx_smtp_client_authentication_reply = NX_SMTP_CLIENT_REPLY_TO_PASSWORD_PROMPT;
    }
    else
    {

        /* Indicate invalid authentication from server.  */
        return NX_SMTP_CLIENT_REPLY_TO_UNKNOWN_PROMPT;
    }

    /* Return successful session status.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_smtp_utility_parse_server_services              PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for the AUTH option in the server response. If */
/*    authentication is offered, it attempts to find a match between      */
/*    authentication options offered by the server and the client         */
/*    authentication type. If no match between client and server is found,*/
/*    or no authentication is listed in the server response, it sets the  */
/*    client authentication type to PLAIN.                                */       
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                   Session to send mail to SMTP Server   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    strstr                        Search for a string within a string   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_smtp_rsp_ehlo             EHLO server reply handler             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            improved boundary check,    */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_smtp_utility_parse_server_services(NX_SMTP_CLIENT *client_ptr)
{

UINT   plain_option = NX_FALSE;
UINT   login_option = NX_FALSE;
UCHAR  *work_ptr; 
UCHAR  *temp_ptr;
ULONG  length, new_length;
UINT   i;
UINT   found = NX_FALSE;
  

    /* Does the server list an authentication type, and  does the client want authentication? */

    /* Set local variable for convenience. */
    length = client_ptr -> nx_smtp_server_packet -> nx_packet_length;

    /* Find the location of the AUTH command. */
    work_ptr = client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr;

    /* Check length. */
    if (length <= 4)
    {
        return;
    }

    for (i = 0; i < length - 4; i++)
    {
        if (
             ((*work_ptr == 'A') || (*work_ptr == 'a')) &&
             ((*(work_ptr + 1) == 'U') || (*(work_ptr + 1) == 'u')) &&
             ((*(work_ptr + 2) == 'T') || (*(work_ptr + 2) == 't')) &&
             ((*(work_ptr + 3) == 'H') || (*(work_ptr + 3) == 'h')) 
           )
        {

            found = NX_TRUE;
            work_ptr += 4;
            break;
        }

        work_ptr++;
    }

    /* Check if this packet has the AUTH keyword. */
    if (!found )
    {

        /* It does not. So leave the Client authentication type as is. */
        return;
    }

    /* Check if the client prefers no authentication. */
    if (found && (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_NONE))
    {

        /* There is an AUTH keyword but the client prefers not to authenticate. */
        return;
    }

    /* Save the location where the search stopped. */
    temp_ptr = work_ptr;

    found = NX_FALSE;
    new_length = length - (ULONG)(temp_ptr - client_ptr -> nx_smtp_server_packet -> nx_packet_prepend_ptr);

    /* Check length. */
    if (new_length < 5)
    {
        return;
    }
    else
    {
        new_length -= 5;
    }

    /* Search for supported authentication types. */
    for (i = 0; i < new_length; i++)
    {
        if (
             ((*work_ptr == 'L') || (*work_ptr == 'l')) &&
             ((*(work_ptr + 1) == 'O') || (*(work_ptr + 1) == 'o')) &&
             ((*(work_ptr + 2) == 'G') || (*(work_ptr + 2) == 'g')) &&
             ((*(work_ptr + 3) == 'I') || (*(work_ptr + 3) == 'i')) &&
             ((*(work_ptr + 4) == 'N') || (*(work_ptr + 4) == 'n')) 
           )
        {
            found = NX_TRUE;
            break;
        }

        work_ptr++;
    }

    /* Is there a LOGIN option offered? */
    if (found)
    {
        /* Yes, set the login option flag. */
        login_option = NX_TRUE;
    }

    found = NX_FALSE;

    /* Restore the location for a new search. */
    work_ptr = temp_ptr;

    for (i = 0; i < new_length; i++)
    {
        if (
             ((*work_ptr == 'P') || (*work_ptr == 'p')) &&
             ((*(work_ptr + 1) == 'L') || (*(work_ptr + 1) == 'l')) &&
             ((*(work_ptr + 2) == 'A') || (*(work_ptr + 2) == 'a')) &&
             ((*(work_ptr + 3) == 'I') || (*(work_ptr + 3) == 'i')) &&
             ((*(work_ptr + 4) == 'N') || (*(work_ptr + 4) == 'n')) 
           )
        {
            found = NX_TRUE;
            break;
        }

        work_ptr++;
    }

    /* Is there a PLAIN option offered?  */
    if (found)
    {
        /* Yes, set the plain option flag. */
        plain_option = NX_TRUE;
    }

    /* Compare Server list of authentication types to client preferred authentication type. */

    /* Handle the case of the client prefers LOGIN authentication. */
    if (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_LOGIN)
    {

        /* Yes; Check if server supports offers LOGIN authentication. */
        if (login_option)
        {

            return;
        }
        else
        {
            /* Switch client to plain authentication. */
            client_ptr -> nx_smtp_client_authentication_type = NX_SMTP_CLIENT_AUTH_PLAIN;

            return;
        }
    }

    /* Check if server listed PLAIN authentication. */
    if (plain_option && (client_ptr -> nx_smtp_client_authentication_type == NX_SMTP_CLIENT_AUTH_PLAIN))
    {
        /* Yes, and there's a match, we're done here.  */
        return;
    }

    /* If we are here, the server offers LOGIN authentication but the Client preference is something else. */
    if (login_option)
    {

        /* Switch client to LOGIN authentication. */
        client_ptr -> nx_smtp_client_authentication_type = NX_SMTP_CLIENT_AUTH_LOGIN;

        return;
    }

    /* Handle the case of no matches between server/client.  Assume the server requires authentication
       and set Client type to plain. */
    client_ptr -> nx_smtp_client_authentication_type = NX_SMTP_CLIENT_AUTH_PLAIN;

    /* Return.  */
    return;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_parse_response                             PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function parses the argument specified by the argument index     */
/*  from the supplied buffer. The first argument is at index 1.           */
/*  If the specified argument can't be found the argument pointer at NULL.*/
/*                                                                        */
/*  Carriage return/line feeds can be treated as word word separator if   */
/*  specified by the crlf_are_word_breaks parameter. For OK TO CONTINUE   */
/*  messages (250), this function searches the whole message for the last */
/*  occurance of the server code e.g. code followed by a space.           */
/*                                                                        */
/*  Carriage return/line feeds are removed if found on the end of the     */
/*  parsed argument.                                                      */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_ptr                        Pointer to client instance        */
/*    buffer                            Pointer to buffer to parse        */
/*    argument_index                    Identify which argument to parse  */
/*    buffer_length                     Size of buffer to parse           */
/*    argument                          Argument to parse from buffer     */ 
/*    argument_length                   Size of argument buffer           */
/*    crlf_are_word_breaks              Treat \r\n's as word breaks       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     NX_SUCCESS                       Successfully parsed reply code or */ 
/*                                        authentication challenge        */
/*     NX_SMTP_INVALID_PARAM            Invalid input                     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    toupper                           Convert chars to upper case       */
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
/*  04-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            improved boundary check,    */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_smtp_parse_response(NX_SMTP_CLIENT *client_ptr, UCHAR *buffer, UINT argument_index, 
                              UINT buffer_length, UCHAR *argument, 
                              UINT argument_length, UINT crlf_are_word_breaks)
{


UINT i = 0;
UINT j = 0;
UINT argument_char_count = 0;
UCHAR *work_ptr;
UINT is_last_code;


    /* Check for invalid input parameters.  */
    if ((buffer == NX_NULL) || (buffer_length == 0) || (argument_index == 0))
    {

        return NX_SMTP_INVALID_PARAM;
    }

    work_ptr = argument;

    /* Is this the first word? 
       The first word is the reply code, not an SMTP command parameter */
    if (argument_index == 1)
    {

        /* Yes, search each character up to the end of the buffer for 
           the first separator.  */
        while (i < buffer_length)
        {

            /* Have we reached a word break?  */
            if (
                 (argument[0] != 0) &&
                 ((*buffer == ' ') || (*buffer == '-'))
               )
            {
                /* Yes, this marks the end of the first argument (word).  */
                break;
            }

            /* Are we starting a number?  */
            else if ((*buffer >= 0x30) && (*buffer <= 0x39))
            {

                if (work_ptr >= argument + argument_length)
                {
                    return(NX_SMTP_INTERNAL_ERROR);
                }

                /* Yes, copy to buffer. */
                *work_ptr = *buffer;  
                work_ptr++;
            }
            else
            {
                /* Not a number. make sure the argument buffer is reset */
                memset(&argument[0], 0, argument_length);
                work_ptr = argument;
            }

            /* Get the next character in the buffer.  */
            buffer++;
            i++;
        }
    }
    else
    {
        /*  No, we're not parsing the first argument. This is a special case for parsing
            the server authentication challenge, not just the reply code.  */

        /*  Mark the start of the argument at the separator after 
            the end of the previous argument.  */
        while ((j < argument_index) && (i < buffer_length))
        {

            /* Keep track of the number of separators in the buffer */
            /* OD0A marks the end of a line usually in SMTP */

            /* Did we hit a line terminator?  */
            if ((*buffer == 0x0D) && (*(buffer + 1) == 0x0A))
            {

                /* Yes, Update the count of separators.  */
                j++;

                /* Are line terminators as word breaks?  */
                if (crlf_are_word_breaks  == NX_FALSE)
                {

                    /* No, this is the end of the search buffer. 
                      Argument not parsed from buffer, return error.  */
                    return NX_SMTP_INVALID_SERVER_REPLY;                    
                }

                /* Get the next character after '\n' byte in the buffer.  */
                i++;
                buffer++;
            }           
            /* Did we hit a space or a dash in the first argument?  */
            else if ((*buffer == ' ') || ((*buffer == '-') && (j == 0)))
            {

                /* Yes; these are counted as separators (note that dashes found in arguments 
                   after the first argument are NOT considered separators.  */
                j++; 
            }
            else
            {

                /* No, is this the argument to parse?  */
                if (j == (argument_index - 1))
                {
                    /* Yes; did we go past the argument size limit before the next word break?  */
                    if (argument_char_count >= argument_length)
                    {

                        /* Yes, unable to finish parsing this buffer. Return error.  */
                        return NX_SMTP_INVALID_SERVER_REPLY;
                    }

                    /* No, copy the next character into the argument.  */
                    argument_char_count++;

                    /* Convert to uppercase if the caller requests.  */
                    *argument++ = *buffer;
                }
            }

            /* Get the next character in the buffer.  */
            buffer++;
            i++;
        }
    }

    /* Determine if this is the last 250 in the message (e.g. followed by a space, not hyphen) */
    work_ptr = buffer - 3;

    if ((*work_ptr == '2') && (*(work_ptr + 1) == '5') && (*(work_ptr + 2) == '0') && (*(work_ptr + 3) == '-'))
    {

        UINT status;

        /* Parse the rest of the buffer to see if this packet contains the last 250 code. */
        status = _nx_smtp_parse_250_response(buffer, buffer_length - i, &is_last_code);
        if (status)
        {
            /* Error with parsing response. */
            return status;
        }

        /* Did we parse the whole 250 response? */
        if (is_last_code != NX_TRUE)
        {
            /* NO, so we are waiting for another 250 packet. */
            client_ptr -> nx_smtp_client_mute = NX_TRUE;
        }
        else
        {

            /* YES, so we are NOT waiting for a packet with the last 250 code. */
            client_ptr -> nx_smtp_client_mute = NX_FALSE;
        }
    }
    else if ((*work_ptr == '2') && (*(work_ptr + 1) == '5') && (*(work_ptr + 2) == '0') && (*(work_ptr + 3) == ' '))
    {
        client_ptr -> nx_smtp_client_mute = NX_FALSE;
    }

    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_parse_250_response                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function is called to search the rest of the message for the last*/
/*  instance of the server code 250. The first 250 has been found but     */
/*  followed by a hyphen, indicating more 250's in the message.           */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer_ptr                        Pointer to buffer to parse        */
/*    buffer_length                     Size of buffer to parse           */
/*    is_last_code                      Indicate if last 250 found        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*     NX_SUCCESS                       Successfully parsed last code     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_smtp_parse_response          Parse (any) code in server message */ 
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
UINT _nx_smtp_parse_250_response(UCHAR *buffer_ptr, UINT buffer_length, UINT *is_last_code)
{

    *is_last_code = NX_FALSE;

    /* Are there more 250 codes in this buffer? Advance the buffer 
       pointer and search for a 250 followed by a space. */       

    while (buffer_length > 3)
    {
        /* Find next 250[sp] in the buffer */
        if ((*buffer_ptr == '2') && 
            (*(buffer_ptr + 1) == '5') && 
            (*(buffer_ptr + 2) == '0') && 
            (*(buffer_ptr + 3) == ' '))
        {
            *is_last_code = NX_TRUE;
            break;
        }
        else
        {
            buffer_ptr++;
            buffer_length--;
        }
    }

    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_smtp_find_crlf                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*  This function returns a pointer to the first carriage return/line     */
/*  feed (CRLF) sequence found in the buffer.  If no CRLF is found within */
/*  length specified, the CRLF pointer is left at NULL. If specified the  */
/*  search can start at the end of the buffer and work backwards to the   */
/*  first CRLF found.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer                          Pointer to buffer to search         */
/*    length                          Size of buffer to search            */
/*    CRLF                            Pointer to CRLF found in buffer     */ 
/*    in_reverse                      Search buffer in reverse direction  */
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
VOID  _nx_smtp_find_crlf(UCHAR *buffer, UINT length, UCHAR **CRLF, UINT in_reverse)
{

UINT i = 0;

    *CRLF = NULL;

    if ((buffer == NX_NULL) || (length == 0))
    {
        return;
    }


    /* Search for CRLF from end to start of buffer (reverse) */
    if (in_reverse == NX_TRUE)
    {

        /* Move pointer out to the end of the buffer.  */
        buffer += length - 1;

        while (i < length - 1)
        {
            if (*(buffer - 1) == 0x0D && *buffer == 0x0A)
            {
                /* Set the location of CRLF sequence.  */
                *CRLF = buffer -  1; 
                break;
            }

            /* Move the pointer back one.  */
            buffer--;

            /* Keep track how many bytes we've gone.  */
            i++;
        }
    }
    /* Search for CRLF starting at the beginning of the buffer */
    else
    {
        while( i < length - 1)
        {
            if (*buffer == 0x0D && *(buffer + 1) == 0x0A)
            {
                /* Set the location of CRLF sequence.  */
                *CRLF = buffer;  

                break;
            }
            i++;
            buffer++;
        }
    }

    return ;
}
