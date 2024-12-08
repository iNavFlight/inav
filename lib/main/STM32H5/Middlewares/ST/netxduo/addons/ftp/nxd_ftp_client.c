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
/** NetX Duo Component                                                    */
/**                                                                       */
/**   File Transfer Protocol (FTP)                                        */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_FTP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* If FileX is not used in your application, define this option.  Then define the 
   services declared in filex_stub.h elsewhere.  
#define      NX_FTP_NO_FILEX    
*/

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#ifdef FEATURE_NX_IPV6
#include    "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6  */
#include    "nxd_ftp_client.h"
#include    "stdio.h"
#include    "string.h"



/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_connect                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client connect call.     */ 
/*                                                                        */
/*    Note: The string lengths of username and password are limited by    */
/*    the packet payload size.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    server_ip                             FTP server IPv4 address       */ 
/*    username                              Pointer to login username     */ 
/*    password                              Pointer to login password     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_ftp_client_connect               Actual client connect call    */ 
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
UINT  _nxe_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, ULONG server_ip, CHAR *username, 
                              CHAR *password, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status;

                                 
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for an invalid server IP address.  */
    if (server_ip == 0)
        return(NX_IP_ADDRESS_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client connect function.  */
    status =  _nx_ftp_client_connect(ftp_client_ptr, server_ip, username, password, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(ftp_client_ptr);
    NX_PARAMETER_NOT_USED(server_ip);
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
/*    _nx_ftp_client_connect                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function connects a previously created FTP instance with the   */ 
/*    FTP server at the specified IP address.                             */ 
/*                                                                        */
/*    Note: The string lengths of username and password are limited by    */
/*    the packet payload size.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    server_ip                             FTP server IPv4 address       */ 
/*    username                              Pointer to login username     */ 
/*    password                              Pointer to login password     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_connect_internal       Connect to FTP server using   */
/*                                                IP address              */
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
UINT  _nx_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, ULONG server_ip, CHAR *username, 
                             CHAR *password, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS server_ipduo;

    /* Construct an IP address structure, and fill in IPv4 address information. */
    server_ipduo.nxd_ip_version = NX_IP_VERSION_V4;
    server_ipduo.nxd_ip_address.v4 = server_ip;


    /* Invoke the real connection call. */
    return (_nx_ftp_client_connect_internal(ftp_client_ptr, &server_ipduo, username, password, wait_option));
#else
    NX_PARAMETER_NOT_USED(ftp_client_ptr);
    NX_PARAMETER_NOT_USED(server_ip);
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
/*    _nxde_ftp_client_connect                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client connect call.     */ 
/*                                                                        */
/*    Note: The string lengths of username and password are limited by    */
/*    the packet payload size.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    server_ip                             FTP server IP address         */ 
/*    username                              Pointer to login username     */ 
/*    password                              Pointer to login password     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_ftp_client_connect               Actual client connect call    */ 
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
UINT  _nxde_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ipduo, CHAR *username, 
                              CHAR *password, ULONG wait_option)
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for an invalid server IP address.  */
    if (!server_ipduo)
        return(NX_IP_ADDRESS_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client connect function.  */
    status =  _nxd_ftp_client_connect(ftp_client_ptr, server_ipduo, username, password, wait_option);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_ftp_client_connect                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function connects a previously created FTP instance with the   */ 
/*    FTP server at the specified IP address.                             */ 
/*                                                                        */
/*    Note: The string lengths of username and password are limited by    */
/*    the packet payload size.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    server_ip                             FTP server IP duo address     */ 
/*    username                              Pointer to login username     */ 
/*    password                              Pointer to login password     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_connect_internal       Acutal connect service        */  
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
UINT  _nxd_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ipduo, CHAR *username, 
                              CHAR *password, ULONG wait_option)
{
              
    /* Invoke the real connection call. */
    return (_nx_ftp_client_connect_internal(ftp_client_ptr, server_ipduo, username, password, wait_option));   
}     

        
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_connect_internal                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function connects a previously created FTP instance with the   */ 
/*    FTP server at the specified IP address.                             */ 
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    server_ip                             FTP server IP address         */ 
/*    username                              Pointer to login username     */ 
/*    password                              Pointer to login password     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_client_socket_bind             Bind client socket to port    */ 
/*    nxd_tcp_client_socket_connect         Connect to FTP server         */
/*    nx_tcp_client_socket_unbind           Unbind client socket from port*/ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive server response       */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
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
UINT  _nx_ftp_client_connect_internal(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ip, CHAR *username, 
                                      CHAR *password, ULONG wait_option)
{

NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;
UINT        length;
UINT        i;
UINT        status;
                

    /* Determine if the client is still in a not connected state.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_NOT_CONNECTED)
    {

        /* Already connected, return an error.  */
        return(NX_FTP_NOT_DISCONNECTED);
    }

    /* Bind the client control socket.  */
    status =  nx_tcp_client_socket_bind(&(ftp_client_ptr -> nx_ftp_client_control_socket), NX_FTP_CLIENT_SOURCE_PORT, wait_option);

    /* Check for an error.  */
    if ((status != NX_SUCCESS) && (status != NX_ALREADY_BOUND))
    {

        /* Unable to bind socket to port. */
        return(status);
    }

    /* Connect the socket to the FTP server.  */
    status =  nxd_tcp_client_socket_connect(&(ftp_client_ptr -> nx_ftp_client_control_socket), server_ip, NX_FTP_SERVER_CONTROL_PORT, wait_option); 


    /* Check for an error.  */
    if (status != NX_SUCCESS)
    {

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* Now wait for the "220 " response from the FTP server to indicate the connection has been
       established.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);

    /* Check to see if no packet was received.  */
    if (status)
    {
               
        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Return.  */
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;
    length = packet_ptr -> nx_packet_length;

    /* Check for 220 message.  */
    if ((length < 3) || (buffer_ptr[0] != '2') || (buffer_ptr[1] != '2'))
    {

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(NX_FTP_EXPECTED_22X_CODE);
    }

    memset(buffer_ptr, 0, length);

    /* Check if out of boundary.  */
    if (((!username) && ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 11)) ||
        ((username) && ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 7)))
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* Now build the user name message.  */
    buffer_ptr[0] =  'U';
    buffer_ptr[1] =  'S';
    buffer_ptr[2] =  'E';
    buffer_ptr[3] =  'R';
    buffer_ptr[4] =  ' ';

    /* Determine if a user name was supplied.  */
    if (username == NX_NULL) 
    {    

        /* No username specified, use "USER".  */
        buffer_ptr[5] =  'U';
        buffer_ptr[6] =  'S';
        buffer_ptr[7] =  'E';
        buffer_ptr[8] =  'R';
        buffer_ptr[9] =  13;
        buffer_ptr[10]=  10;

        /* Set the length of the packet.  */
        packet_ptr -> nx_packet_length =  11;
    }
    else 
    {

        /* A username was supplied, copy it into the buffer.  */
        for(i = 0; username[i]; i++)
        {

            /* Check if out of boundary.  */
            if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
            {

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* Close the socket. */
                nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

                /* Unbind the socket.  */
                nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

                /* Set the error status.  */
                return(NX_FTP_FAILED);
            }

            /* Copy byte of the username.  */
            buffer_ptr[5+i] =  (UCHAR)username[i];
        }

        /* Now insert the CR/LF.  */
        buffer_ptr[5+i] =  13;
        buffer_ptr[5+i+1] =  10;

        /* Setup the length of the packet.  */
        packet_ptr -> nx_packet_length =  i + 7;
    }

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the username to the FTP server.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Check for unsuccessful send.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* There may be multiple 220 responses from FTP server after establishing connection.
       Flush mutiple 220 respones and wait for 331 response from the FTP server.  */
    while(1)
    {

        /* Wait for response from the FTP server.  */
        status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

        /* Determine if a packet was not received.  */
        if (status)
        {

            /* Close the socket. */
            nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

            /* Unbind the socket.  */
            nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

            /* Unable to connect socket to server FTP control port. */
            return(status);
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        if (packet_ptr -> nx_packet_next)
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Close the socket. */
            nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

            /* Unbind the socket.  */
            nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

            /* Return.  */
            return(NX_INVALID_PACKET);
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* We have a packet, setup pointer to the buffer area.  */
        buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Check for 220 message.  */
        if ((packet_ptr -> nx_packet_length >= 3) && (buffer_ptr[0] == '2') && (buffer_ptr[1] == '2'))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            continue;
        }

        /* Check to make sure the response is proper.  */

        /* Check for 331 message.  */
        if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '3') || (buffer_ptr[1] != '3'))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Close the socket. */
            nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

            /* Unbind the socket.  */
            nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

            /* Unable to connect socket to server FTP control port. */
            return(NX_FTP_EXPECTED_33X_CODE);
        }
        else
        {
            break;
        }
    }

    memset(buffer_ptr, 0, length);

    /* Check if out of boundary.  */
    if (((!password) && ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 11)) ||
        ((password) && ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 7)))
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* Now build the password message.  */
    buffer_ptr[0] =  'P';
    buffer_ptr[1] =  'A';
    buffer_ptr[2] =  'S';
    buffer_ptr[3] =  'S';
    buffer_ptr[4] =  ' ';

    /* Determine if a password was specified.  */
    if (password == NX_NULL) 
    {    

        /* No password was specified, use "PASS".  */
        buffer_ptr[5] =  'P';
        buffer_ptr[6] =  'A';
        buffer_ptr[7] =  'S';
        buffer_ptr[8] =  'S';
        buffer_ptr[9] =  13;
        buffer_ptr[10]=  10;

        /* Setup the packet length.  */
        packet_ptr -> nx_packet_length =  11;
    }
    else 
    {

        /* Password was specified, copy it into the buffer.  */
        for(i = 0; password[i]; i++)
        {

            /* Check if out of boundary.  */
            if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
            {

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* Close the socket. */
                nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

                /* Unbind the socket.  */
                nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

                /* Set the error status.  */
                return(NX_FTP_FAILED);
            }

            /* Copy byte of password.  */
            buffer_ptr[5+i] =  (UCHAR)password[i];
        }

        /* Now insert the CR/LF.  */
        buffer_ptr[5+i] =  13;
        buffer_ptr[5+i+1] = 10;

        /* Setup the length of the packet.  */
        packet_ptr -> nx_packet_length =  i + 7;
    }

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the password to the FTP server.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Check for successful send.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 230 message, signaling successful login.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2') || (buffer_ptr[1] != '3'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Close the socket. */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(NX_FTP_EXPECTED_23X_CODE);
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* If we get here, we have a successful connect with the FTP server. */
    ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_CONNECTED;

    ftp_client_ptr -> nx_ftp_client_data_port = ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_port;

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_create                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client create call.      */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    ftp_client_name                       Name of this FTP client       */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    window_size                           Size of TCP receive window    */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_create                 Actual client create call     */ 
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
UINT  _nxe_ftp_client_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *ftp_client_name, NX_IP *ip_ptr, 
                             ULONG window_size, NX_PACKET_POOL *pool_ptr)
{

UINT    status;

                        
    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id == NXD_FTP_CLIENT_ID) || 
        (pool_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual client create function.  */
    status =  _nx_ftp_client_create(ftp_client_ptr, ftp_client_name, ip_ptr, window_size, pool_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_create                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates an FTP client instance.                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    ftp_client_name                       Name of this FTP client       */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    window_size                           Size of TCP receive window    */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_create                  Create TCP socket             */ 
/*    nx_tcp_socket_delete                  Delete TCP socket             */ 
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
UINT  _nx_ftp_client_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *ftp_client_name, NX_IP *ip_ptr, 
                            ULONG window_size, NX_PACKET_POOL *pool_ptr)
{

UINT    status;


    /* Clear the client FTP control block.  */
    memset((void *) ftp_client_ptr, 0, sizeof(NX_FTP_CLIENT));

    /* Create the TCP control socket.  */
    status =  nx_tcp_socket_create(ip_ptr, &(ftp_client_ptr -> nx_ftp_client_control_socket), ftp_client_name, 
                                        NX_FTP_CONTROL_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, window_size,
                                        NX_NULL, NX_NULL);

    /* Check for an error.  */
    if (status)
    {

        /* Return an error.  */
        return(status);
    }
                                         
    /* Create the TCP data socket.  */
    status =  nx_tcp_socket_create(ip_ptr, &(ftp_client_ptr -> nx_ftp_client_data_socket), ftp_client_name, 
                                        NX_FTP_CONTROL_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, window_size,
                                        NX_NULL, _nx_ftp_client_data_disconnect);

    /* Check for an error.  */
    if (status)
    {

        /* Delete the control socket.  */
        nx_tcp_socket_delete(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Return an error.  */
        return(status);
    }

    /* Save off the remaining information.  */

    /* Save the client name.  */
    ftp_client_ptr -> nx_ftp_client_name =  ftp_client_name;

    /* Save the IP pointer.  */
    ftp_client_ptr -> nx_ftp_client_ip_ptr =  ip_ptr;

    /* Save the packet pool pointer.  */
    ftp_client_ptr -> nx_ftp_client_packet_pool_ptr =  pool_ptr;

    /* Set the initial client state.  */
    ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_NOT_CONNECTED;
                            
    /* Set the FTP client id.  */
    ftp_client_ptr -> nx_ftp_client_id =  NXD_FTP_CLIENT_ID;

    /* Default to active transfer mode. */
    ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled = NX_FALSE;

    /* Default to stream mode. */
    ftp_client_ptr -> nx_ftp_client_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_data_disconnect                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles the disconnect from the server.               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data_socket_ptr                       Pointer to FTP client data    */ 
/*                                            socket                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_disconnect              Disconnect TCP socket         */ 
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
void  _nx_ftp_client_data_disconnect(NX_TCP_SOCKET *data_socket_ptr)
{

    /* Disconnect FTP client data socket.  */
    nx_tcp_socket_disconnect(data_socket_ptr, NX_NO_WAIT);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_delete                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client delete call.      */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_delete                 Actual client delete call     */ 
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
UINT  _nxe_ftp_client_delete(NX_FTP_CLIENT *ftp_client_ptr)
{

UINT    status;

                                             
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client delete function.  */
    status =  _nx_ftp_client_delete(ftp_client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_delete                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created FTP client instance.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_create                  Create TCP socket             */ 
/*    nx_tcp_socket_delete                  Delete TCP socket             */ 
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
UINT  _nx_ftp_client_delete(NX_FTP_CLIENT *ftp_client_ptr)
{

    /* Determine if the client is still in a not connected state.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_NOT_CONNECTED)
    {

        /* Already connected, return an error.  */
        return(NX_FTP_NOT_DISCONNECTED);
    }

    /* Delete the control and data sockets.  */
    nx_tcp_socket_delete(&(ftp_client_ptr -> nx_ftp_client_control_socket));
    nx_tcp_socket_delete(&(ftp_client_ptr -> nx_ftp_client_data_socket));

    /* Return success to caller.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_directory_create                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client directory         */ 
/*    create call.                                                        */ 
/*                                                                        */
/*    Note: The string length of directory_name is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_name                        New directory name            */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_create       Actual client directory       */ 
/*                                            create call                 */ 
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
UINT  _nxe_ftp_client_directory_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option)
{

UINT    status;

                           
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (directory_name == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client directory create function.  */
    status =  _nx_ftp_client_directory_create(ftp_client_ptr, directory_name, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_create                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the specified directory name on the FTP       */ 
/*    server.                                                             */ 
/*                                                                        */
/*    Note: The string length of directory_name is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_name                        New directory name            */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_directory_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;

        /* Ensure the client is the proper state for directory create request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Allocate a packet for sending the directory create command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 6)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "MKD" message to indicate directory create.  */
    buffer_ptr[0] =  'M';
    buffer_ptr[1] =  'K';
    buffer_ptr[2] =  'D';
    buffer_ptr[3] =  ' ';

    /* Copy the new directory name into the message.  */
    for(i = 0; directory_name[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 6) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the directory name.  */
        buffer_ptr[4+i] =  (UCHAR)directory_name[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+4] =  13;
    buffer_ptr[i+5] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+6;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the MKD message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* MKD error. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "MKD" was processed successfully.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Create directory error. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Yes, the directory was created.  */

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}
 

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_directory_default_set               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client directory         */ 
/*    default set.                                                        */ 
/*                                                                        */
/*    Note: The string length of directory_path is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_path                        New default directory path    */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_default_set  Actual client directory       */ 
/*                                            default set call            */ 
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
UINT  _nxe_ftp_client_directory_default_set(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, ULONG wait_option)
{

UINT    status;

                       
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client directory default set function.  */
    status =  _nx_ftp_client_directory_default_set(ftp_client_ptr, directory_path, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_default_set                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function changes the default working directory on the FTP      */ 
/*    server.                                                             */ 
/*                                                                        */
/*    Note: The string length of directory_path is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_path                        New default directory path    */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_directory_default_set(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Ensure the client is the proper state for a default directory request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Allocate a packet for sending the change directory command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 6)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "CWD" message to indicate default directory change.  */
    buffer_ptr[0] =  'C';
    buffer_ptr[1] =  'W';
    buffer_ptr[2] =  'D';
    buffer_ptr[3] =  ' ';

    /* Copy the new default directory into the message.  */
    for(i = 0; directory_path[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 6) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the directory path.  */
        buffer_ptr[4+i] =  (UCHAR)directory_path[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+4] =  13;
    buffer_ptr[i+5] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+6;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the CWD message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* RNFR file error. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "CWD" was processed successfully.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Change directory error. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Yes, the default directory was changed.  */

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_directory_delete                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client directory         */ 
/*    delete.                                                             */ 
/*                                                                        */
/*    Note: The string length of directory_name is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_name                        Directory name to delete      */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_delete       Actual client directory       */ 
/*                                            delete call                 */ 
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
UINT  _nxe_ftp_client_directory_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option)
{

UINT    status;

                    
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client directory delete function.  */
    status =  _nx_ftp_client_directory_delete(ftp_client_ptr, directory_name, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_delete                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified directory on the FTP            */ 
/*    server.                                                             */ 
/*                                                                        */
/*    Note: The string length of directory_name is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_name                        Directory name to delete      */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_directory_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Ensure the client is the proper state for directory delete request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Allocate a packet for sending the directory delete command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 6)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "RMD" message to indicate directory delete.  */
    buffer_ptr[0] =  'R';
    buffer_ptr[1] =  'M';
    buffer_ptr[2] =  'D';
    buffer_ptr[3] =  ' ';

    /* Copy the directory name into the message.  */
    for(i = 0; directory_name[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 6) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the directory name.  */
        buffer_ptr[4+i] =  (UCHAR)directory_name[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+4] =  13;
    buffer_ptr[i+5] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+6;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the RMD message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* DELE error. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "RMD" was processed successfully.  */
    if ((packet_ptr -> nx_packet_length < 3) ||(buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Delete directory error. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Yes, the directory was deleted.  */

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_directory_listing_get               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client directory         */ 
/*    listing get.                                                        */ 
/*                                                                        */
/*    Note: The string length of directory_path is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_path                        Directory to get listing for  */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*                                            that contains the listing   */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_listing_get  Actual client directory       */ 
/*                                            listing get call            */ 
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
UINT  _nxe_ftp_client_directory_listing_get(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, 
                                            NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;

                        
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client directory listing get function.  */
    status =  _nx_ftp_client_directory_listing_get(ftp_client_ptr, directory_path, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_listing_get                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets a listing for the specified directory on the     */ 
/*    FTP server.                                                         */ 
/*                                                                        */
/*    Note: The string length of directory_path is limited by the packet  */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    directory_path                        Directory to get listing for  */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*                                            that contains the listing   */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ftp_client_packet_allocate        Allocate packet               */ 
/*    _nx_ftp_client_active_transfer_setup  Setup active transfer         */ 
/*    _nx_ftp_client_passive_transfer_setup Setup passive transfer        */ 
/*    _nx_ftp_client_block_mode_send        Send block mode command       */ 
/*    _nx_ftp_client_data_socket_cleanup    Cleanup data socket           */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_server_socket_accept           Connect data socket to server */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
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
UINT  _nx_ftp_client_directory_listing_get(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, 
                                           NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *new_packet_ptr;
UINT        status;


    /* Set packet pointer to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Ensure the client is the proper state for directory listing request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Divert to passive mode operation if passive mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled)
    {

        /* Transfer the data in passive transfer mode.  */
        status = _nx_ftp_client_passive_transfer_setup(ftp_client_ptr,  wait_option);
    }
    else
    {

        /* Transfer the data in active transfer mode. */
        status = _nx_ftp_client_active_transfer_setup(ftp_client_ptr, wait_option);
    }

    /* Determine if set up successful.  */
    if (status)
    {
        return(status);
    }

    /* Check if enable block mode.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
    {

        /* Send MODE B command to FTP server.  */
        status = _nx_ftp_client_block_mode_send(ftp_client_ptr, wait_option);

        /* Determine if the send was unsuccessful.  */
        if (status != NX_SUCCESS)
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);
            return(status);
        }
    }

    /* Allocate a packet for sending the NLST command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &new_packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_prepend_ptr) < 7)
     {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Release the packet.  */
        nx_packet_release(new_packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  new_packet_ptr -> nx_packet_prepend_ptr;

    /* Now build the actual NLST request.  */
    buffer_ptr[0] =  'N';
    buffer_ptr[1] =  'L';
    buffer_ptr[2] =  'S';
    buffer_ptr[3] =  'T';
    buffer_ptr[4] =  ' ';

    /* Copy the directory path into the buffer.  */    
    for(i = 0; directory_path[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 7) >= (UINT)(new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

            /* Release the packet.  */
            nx_packet_release(new_packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of directory path.  */
        buffer_ptr[5+i] =  (UCHAR)directory_path[i];
    }

    /* Insert the CR/LF.  */
    buffer_ptr[5+i] =   13;
    buffer_ptr[5+i+1] = 10;

    /* Setup the length of the packet.  */
    new_packet_ptr -> nx_packet_length =  i + 7;

    /* Setup the packet append pointer.  */
    new_packet_ptr -> nx_packet_append_ptr =  new_packet_ptr -> nx_packet_prepend_ptr + new_packet_ptr -> nx_packet_length;

    /* Send the NLST message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket),  new_packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Release the packet.  */
        nx_packet_release(new_packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Check if enable passive mode.  */
    if (ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled == NX_FALSE)
    {

        /* Now wait for the data connection to connect.  */
        status =  nx_tcp_server_socket_accept(&(ftp_client_ptr -> nx_ftp_client_data_socket), wait_option);

        /* Determine if the accept was unsuccessful.  */
        if (status)
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

            /* Return error.  */
            return(status);
        }
    }

    /* Now wait for response from the FTP server control port.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &new_packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Error in NLST request to FTP server. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  new_packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 1xx message, signaling the data port was connected properly and ready for 
       transfer.  */
    if ((new_packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '1'))
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Release the packet.  */
        nx_packet_release(new_packet_ptr);

        /* Error in NLST request to FTP server. */
        return(NX_FTP_EXPECTED_1XX_CODE);
    }

    /* Release the last packet.  */
    nx_packet_release(new_packet_ptr);

    /* Now read a listing packet from the data socket.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_data_socket), packet_ptr, wait_option);

    /* Determine if an error occurred.  */
    if (status)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Map all unsuccessful status to error.  */
        return(status); 
    }

    /* Determine if the block mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
    {

        /* Retrieve the block header.  */
        status = _nx_ftp_client_block_header_retrieve(ftp_client_ptr, (*packet_ptr));

        /* Determine if an error occurred.  */
        if (status)
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);
        }
    }

    /* Return staus to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_directory_listing_continue          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client directory         */ 
/*    listing continue.                                                   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*                                            that contains the listing   */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_listing_continue  Actual client directory  */ 
/*                                                 listing continue call  */ 
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
UINT  _nxe_ftp_client_directory_listing_continue(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;

                          
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client directory listing continue function.  */
    status =  _nx_ftp_client_directory_listing_continue(ftp_client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_directory_listing_continue           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the next listing buffer.  It is assumed that     */ 
/*    a successful _nx_ftp_client_directory_listing_get immediately       */ 
/*    preceded this call.                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*                                            that contains the listing   */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_data_socket_cleanup    Cleanup data socket           */ 
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server port       */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
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
UINT  _nx_ftp_client_directory_listing_continue(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        status;
NX_PACKET   *response_ptr;
UCHAR       *buffer_ptr;


    /* Set packet pointer to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Ensure the client is the proper state for directory listing request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Now read a listing packet from the data socket.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_data_socket), packet_ptr, wait_option);

    /* Determine if the block mode is enabled.  */
    if ((status == NX_SUCCESS) &&
        (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK))
    {

        /* Retrieve the block header.  */
        status = _nx_ftp_client_block_header_retrieve(ftp_client_ptr, (*packet_ptr));
    }

    /* Determine if an error occurred.  */
    if (status)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Wait for response from the FTP server on the control socket.  */
        status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &response_ptr, wait_option);

        /* Determine if a packet was not received.  */
        if (status)
        {

            /* Directory listing error. */
            return(status);
        }

        /* We have a packet, setup pointer to the buffer area.  */
        buffer_ptr =  response_ptr -> nx_packet_prepend_ptr;

        /* Check for 2xx message, signaling the "NLST" was processed successfully.  */
        if ((response_ptr -> nx_packet_length >= 3) && (buffer_ptr[0] == '2'))
        {

            /* Release the packet.  */
            nx_packet_release(response_ptr);

            /* Appropriate end of listing error. */
            return(NX_FTP_END_OF_LISTING);
        }
        else
        {

            /* Release the packet.  */
            nx_packet_release(response_ptr);

            /* Map all other unsuccessful status to error.  */
            return(NX_FTP_EXPECTED_2XX_CODE); 
        }
    }

    /* Return success to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_disconnect                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client disconnect.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_disconnect             Actual client disconnect call */ 
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
UINT  _nxe_ftp_client_disconnect(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

UINT    status;

                   
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client disconnect function.  */
    status =  _nx_ftp_client_disconnect(ftp_client_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_disconnect                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function disconnects a previously established FTP connection.  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_client_socket_unbind           Unbind a socket               */ 
/*    nx_tcp_socket_disconnect              Disconnect a socket           */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
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
UINT  _nx_ftp_client_disconnect(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;
UINT        status;


    /* Determine if the client is in a not connected state.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
    {

        /* Already connected, return an error.  */
        return(NX_FTP_NOT_CONNECTED);
    }

    /* Enter the not connected state.  */
    ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_NOT_CONNECTED;

    /* Allocate a packet for sending the port and IP address.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Clean up connection socket.  */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 6)
     {

        /* Clean up connection socket.  */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Build QUIT message to end the FTP connection.  */
    buffer_ptr[0] =  'Q';
    buffer_ptr[1] =  'U';
    buffer_ptr[2] =  'I';
    buffer_ptr[3] =  'T';
    buffer_ptr[4] =  13;
    buffer_ptr[5] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  6;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the QUIT message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Clean up connection socket.  */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* Clean up connection socket.  */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the disconnect was processed properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Clean up connection socket.  */
        nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Success.  Disconnect and unbind the control socket.  */
    nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_control_socket), wait_option);
    nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_control_socket));

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_close                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file close.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_close             Actual client file close call */ 
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
UINT  _nxe_ftp_client_file_close(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

UINT    status;

                        
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);   


    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file close function.  */
    status =  _nx_ftp_client_file_close(ftp_client_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_close                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function closes a previously open client FTP file.             */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_data_socket_cleanup    Cleanup data socket           */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server socket     */ 
/*    nx_tcp_socket_disconnect              Disconnect a socket           */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
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
UINT  _nx_ftp_client_file_close(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;
UINT        status;


    /* Ensure the client is in the proper state for closing the socket.  */
    if (ftp_client_ptr -> nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);
    else if ((ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_OPEN) &&
             (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN))
        return(NX_FTP_NOT_OPEN);

    /* Cleanup data socket.  */
    _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

    /* Set the state to connected.  */
    ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_CONNECTED;

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* Unable to connect socket to server FTP control port. */
        return(status);
    }

    /* We have a packet, setup pointer to the packet payload area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the close was processed properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_delete                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file delete.      */ 
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_name                             File name to delete           */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_delete            Actual client file delete call*/ 
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
UINT  _nxe_ftp_client_file_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, ULONG wait_option)
{

UINT    status;

                   
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (file_name == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file delete function.  */
    status =  _nx_ftp_client_file_delete(ftp_client_ptr, file_name, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_delete                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified file on the FTP                 */ 
/*    server.                                                             */ 
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_name                             File name to delete           */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_file_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Ensure the client is the proper state for file delete request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Allocate a packet for sending the file delete command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 7)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "DELE" message to indicate file delete.  */
    buffer_ptr[0] =  'D';
    buffer_ptr[1] =  'E';
    buffer_ptr[2] =  'L';
    buffer_ptr[3] =  'E';
    buffer_ptr[4] =  ' ';

    /* Copy the file name into the message.  */
    for(i = 0; file_name[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the file name.  */
        buffer_ptr[5+i] =  (UCHAR)file_name[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+5] =  13;
    buffer_ptr[i+6] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+7;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the DELE message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* DELE error. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "DELE" was processed successfully.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* File delete error. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Yes, the file was deleted.  */

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_passive_mode_set                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the request to passive mode. Once this is set    */
/*    the Client will send a PASV command preceding any FTP Client command*/
/*    involving transferring data on the data socket.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    passive_mode_enabled                  True to enable                */
/*                                          False to disable              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*     None                                                               */ 
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
UINT  _nxe_ftp_client_passive_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT passive_mode_enabled)
{

UINT status;

    if (ftp_client_ptr == NX_NULL) 
    {
        return NX_PTR_ERROR;
    }

    if ((passive_mode_enabled != NX_TRUE) && (passive_mode_enabled != NX_FALSE)) 
    {
        return NX_INVALID_PARAMETERS;
    }

    status = _nx_ftp_client_passive_mode_set(ftp_client_ptr, passive_mode_enabled);

    return status;

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_passive_mode_set                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the request to passive mode. Once this is set    */
/*    the Client will send a PASV command preceding any FTP Client command*/
/*    for accessing a server data port.  If passive mode is not set,      */
/*    the FTP Client will send and receive data via active transfer mode. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    passive_mode_enabled                  True to enable passive mode   */
/*                                          False to disable passive mode */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*     None                                                               */ 
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
UINT  _nx_ftp_client_passive_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT passive_mode_enabled)
{

    /* Set the transfer status according to the passive mode enabled input.  */
    ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled = passive_mode_enabled;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_transfer_mode_set                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP transfer mode set.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    transfer_mode                         Transfer mode                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_transfer_mode_set      Actual transfer mode set      */ 
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
UINT  _nxe_ftp_client_transfer_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT transfer_mode)
{

UINT status;

    /* Check for valid pointer.  */
    if (ftp_client_ptr == NX_NULL) 
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual transfer mode set function.  */
    status = _nx_ftp_client_transfer_mode_set(ftp_client_ptr, transfer_mode);

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_transfer_mode_set                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the transfer mode.                               */
/*    Note: just support stream mode and block mode yet.                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    transfer_mode                         Transfer mode                 */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*     None                                                               */ 
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
UINT  _nx_ftp_client_transfer_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT transfer_mode)
{

    /* Check for transfer mode.  */
    if ((transfer_mode != NX_FTP_TRANSFER_MODE_STREAM) && 
        (transfer_mode != NX_FTP_TRANSFER_MODE_BLOCK)) 
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Set the transfer mode.  */
    ftp_client_ptr -> nx_ftp_client_transfer_mode = transfer_mode;

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_open                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file open.        */ 
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_name                             Client file name              */ 
/*    open_type                             Open for read or open for     */ 
/*                                            write                       */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_open              Actual client file open call  */ 
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
UINT  _nxe_ftp_client_file_open(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, UINT open_type, ULONG wait_option)
{

UINT    status;

                 
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (file_name == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for illegal open option type. */
    if ((open_type != NX_FTP_OPEN_FOR_READ) && (open_type != NX_FTP_OPEN_FOR_WRITE))
        return(NX_OPTION_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file open function.  */
    status =  _nx_ftp_client_file_open(ftp_client_ptr, file_name, open_type, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_open                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a client FTP file.                              */ 
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_name                             Client file name              */ 
/*    open_type                             Open for read or open for     */ 
/*                                            write                       */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_packet_allocate        Allocate packet               */ 
/*    _nx_ftp_client_active_transfer_setup  Setup active transfer         */ 
/*    _nx_ftp_client_passive_transfer_setup Setup passive transfer        */ 
/*    _nx_ftp_client_block_mode_send        Send block mode command       */ 
/*    _nx_ftp_client_data_socket_cleanup    Cleanup data socket           */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_server_socket_accept           Connect data socket to server */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
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
UINT  _nx_ftp_client_file_open(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, UINT open_type, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Ensure the client is the proper state for an open request.  */
    if (ftp_client_ptr -> nx_ftp_client_state == NX_FTP_STATE_NOT_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);
    else if (ftp_client_ptr -> nx_ftp_client_state == NX_FTP_STATE_OPEN)
        return(NX_FTP_NOT_CLOSED);
    else if (ftp_client_ptr -> nx_ftp_client_state == NX_FTP_STATE_WRITE_OPEN)
        return(NX_FTP_NOT_CLOSED);

    /* Flush the control socket queue for any old messages before starting a new file open request. */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, 1);

    /* Determine if a packet was received.  */
    if (status == NX_SUCCESS)
    {

        /* Just discard it. */
        nx_packet_release(packet_ptr);
    }

    /* Allocate a packet for sending the TYPE command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 8)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_INVALID_PACKET);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "TYPE I" message to indicate binary file transfer.  */
    buffer_ptr[0] =  'T';
    buffer_ptr[1] =  'Y';
    buffer_ptr[2] =  'P';
    buffer_ptr[3] =  'E';
    buffer_ptr[4] =  ' ';
    buffer_ptr[5] =  'I';

    /* Set the CR/LF.  */
    buffer_ptr[6] =  13;
    buffer_ptr[7] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  8;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the TYPE message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status != NX_SUCCESS)
    {

        /* Unable to open file with FTP server. */
        return(status);
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "TYPE I" was received properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to open file with FTP server. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Divert to passive mode operation if passive mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled)
    {

        /* Set up the passive transfer mode:
           Send PASV request,
           Get server IP and port to connect to,
           Open data socket based on server info in PASV response.  */
        status = _nx_ftp_client_passive_transfer_setup(ftp_client_ptr,  wait_option);
    }
    else
    {

        /* Open in active transfer mode.  */
        status = _nx_ftp_client_active_transfer_setup(ftp_client_ptr, wait_option);
    }

    /* Determine if set up successful.  */
    if (status)
    {
        return(status);
    }

    /* Check if enable block mode.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
    {

        /* Send MODE B command to FTP server.  */
        status = _nx_ftp_client_block_mode_send(ftp_client_ptr, wait_option);

        /* Determine if the send was unsuccessful.  */
        if (status != NX_SUCCESS)
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);
            return(status);
        }
    }

    /* Allocate a packet for sending the TYPE command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Return error.  */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build the actual open request.  */
    if (open_type == NX_FTP_OPEN_FOR_WRITE)
    {

        /* File write request, STOR is the command for writing a file.  */
        buffer_ptr[0] =  'S';
        buffer_ptr[1] =  'T';
        buffer_ptr[2] =  'O';
        buffer_ptr[3] =  'R';
        buffer_ptr[4] =  ' ';
    }
    else 
    {

        /* File read request, RETR is the command for reading a file.  */
        buffer_ptr[0] =  'R';
        buffer_ptr[1] =  'E';
        buffer_ptr[2] =  'T';
        buffer_ptr[3] =  'R';
        buffer_ptr[4] =  ' ';
    }

    /* Copy the file name into the buffer.  */    
    for(i = 0; file_name[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of file name.  */
        buffer_ptr[5+i] =  (UCHAR)file_name[i];
    }

    /* Insert the CR/LF.  */
    buffer_ptr[5+i] =   13;
    buffer_ptr[5+i+1] = 10;

    /* Setup the length of the packet.  */
    packet_ptr -> nx_packet_length =  i + 7;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the STOR/RETR message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status != NX_SUCCESS)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Check if enable passive mode.  */
    if (ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled == NX_FALSE)
    {

        /* Active mode. Now wait for the data connection to connect.  */
        status =  nx_tcp_server_socket_accept(&(ftp_client_ptr -> nx_ftp_client_data_socket), wait_option);

        /* Determine if the accept was unsuccessful.  */
        if (status != NX_SUCCESS)
        {

            /* Cleanup data socket.  */
            _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

            /* Return error.  */
            return(status);
        }
    }

    /* Now wait for response from the FTP server control port.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);

    /* Determine if a packet was not received.  */
    if (status != NX_SUCCESS)
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Unable to open file with FTP server. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 1xx message, signaling the data port was connected properly and ready for transfer.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '1'))
    {

        /* Cleanup data socket.  */
        _nx_ftp_client_data_socket_cleanup(ftp_client_ptr, wait_option);

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to open file with FTP server. */
        return(NX_FTP_EXPECTED_1XX_CODE);
    }

    /* Release the last packet.  */
    nx_packet_release(packet_ptr);

    /* At this point, the client has successfully opened a FTP connection.  */
    if (open_type == NX_FTP_OPEN_FOR_WRITE)
    {

        /* Set the client state to open for write.  */
        ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_WRITE_OPEN;
    }
    else
    {

        /* Set the client state to open for read.  */
        ftp_client_ptr -> nx_ftp_client_state =  NX_FTP_STATE_OPEN;
    }

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_read                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file read.        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_read              Actual client file read call  */ 
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
UINT  _nxe_ftp_client_file_read(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;

                     
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file read function.  */
    status =  _nx_ftp_client_file_read(ftp_client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_read                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reads a packet of the file from the data connection   */ 
/*    with the FTP server.                                                */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Destination of for the        */ 
/*                                            received packet pointer     */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
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
UINT  _nx_ftp_client_file_read(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT    status;


    /* Determine if the file is open for writing.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_OPEN)
        return(NX_FTP_NOT_OPEN);
   
    /* Default the packet pointer to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Read the packet from the data connection.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_data_socket), packet_ptr, wait_option);

    /* Determine if an error occurred.  */
    if (status == NX_NOT_CONNECTED)
    {

        /* Map not-connected status to end of file.  */
        status =  NX_FTP_END_OF_FILE; 
    }

    /* Determine if the block mode is enabled.  */
    if ((status == NX_SUCCESS) &&
        (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK))
    {

        /* Retrieve the block header.  */
        status = _nx_ftp_client_block_header_retrieve(ftp_client_ptr, (*packet_ptr));
    }

    /* Return status to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_rename                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file rename.      */ 
/*                                                                        */
/*    Note: The string lengths of filename and new_filename are limited   */
/*    by the packet payload size.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    filename                              Current file name             */ 
/*    new_filename                          New file name                 */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_rename           Actual client file rename call*/ 
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
UINT  _nxe_ftp_client_file_rename(NX_FTP_CLIENT *ftp_client_ptr, CHAR *filename, CHAR *new_filename, ULONG wait_option)
{

UINT    status;

                         
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || 
        (filename == NX_NULL) || (new_filename == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file rename function.  */
    status =  _nx_ftp_client_file_rename(ftp_client_ptr, filename, new_filename, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_rename                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function renames the specified file with the supplied          */ 
/*    filename.                                                           */ 
/*                                                                        */
/*    Note: The string lengths of filename and new_filename are limited   */
/*    by the packet payload size.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    filename                              Current file name             */ 
/*    new_filename                          New file name                 */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_receive                 Receive packet from server    */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_file_rename(NX_FTP_CLIENT *ftp_client_ptr, CHAR *filename, CHAR *new_filename, ULONG wait_option)
{

UINT        i;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Ensure the client is the proper state for a rename file request.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_CONNECTED)
        return(NX_FTP_NOT_CONNECTED);

    /* Allocate a packet for sending the file rename command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 7)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build "RNFR" message to indicate file rename request.  */
    buffer_ptr[0] =  'R';
    buffer_ptr[1] =  'N';
    buffer_ptr[2] =  'F';
    buffer_ptr[3] =  'R';
    buffer_ptr[4] =  ' ';

    /* Copy the old file name into the message.  */
    for(i = 0; filename[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the file name.  */
        buffer_ptr[5+i] =  (UCHAR)filename[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+5] =  13;
    buffer_ptr[i+6] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+7;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the RNFR message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket),  packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* RNFR file error. */
        return(status);
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_next)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return(NX_INVALID_PACKET);
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 3xx message, signaling the "RNFR" was received properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '3'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to rename file with FTP server. */
        return(NX_FTP_EXPECTED_3XX_CODE);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 7)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* Now build "RNTO" message to specify new file name.  */
    buffer_ptr[0] =  'R';
    buffer_ptr[1] =  'N';
    buffer_ptr[2] =  'T';
    buffer_ptr[3] =  'O';
    buffer_ptr[4] =  ' ';

    /* Copy the old file name into the message.  */
    for(i = 0; new_filename[i]; i++)
    {

        /* Check if out of boundary.  */
        if ((i + 7) >= (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_FTP_FAILED);
        }

        /* Copy character of the new file name.  */
        buffer_ptr[5+i] =  (UCHAR)new_filename[i];
    }

    /* Set the CR/LF.  */
    buffer_ptr[i+5] =  13;
    buffer_ptr[i+6] =  10;

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  i+7;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the RNTO message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status)
    {

        /* RNTO file error. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the "RNTO" was processed properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to rename with FTP server. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Release packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_size_set                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file size set.    */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_size                             The total size of file        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_size_set          Actual file size set call     */ 
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
UINT  _nxe_ftp_client_file_size_set(NX_FTP_CLIENT *ftp_client_ptr, ULONG file_size)
{

UINT    status;

                  
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file size set function.  */
    status =  _nx_ftp_client_file_size_set(ftp_client_ptr, file_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_size_set                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the file size for block mode before send packets */ 
/*    to the file on the data connection with the FTP server.             */ 
/*                                                                        */ 
/*    Note:a. This API is only used for block mode,                       */ 
/*         b. Must call this API before call nx_ftp_client_file_write.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    file_size                             The total size of file        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_file_size_set(NX_FTP_CLIENT *ftp_client_ptr, ULONG file_size)
{

UINT        status;


    /* Check for invalid file size.  */
    if (file_size == 0)
        return(NX_FTP_CLIENT_INVALID_SIZE);

    /* Check if already set the file size.  */
    if (ftp_client_ptr -> nx_ftp_client_block_total_size != 0)
        return(NX_FTP_CLIENT_FILE_SIZE_ALREADY_SET);

    /* Determine if the block mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode != NX_FTP_TRANSFER_MODE_BLOCK)
        return(NX_FTP_CLIENT_NOT_BLOCK_MODE);

    /* Determine if the file is open for writing.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN)
        return(NX_FTP_NOT_OPEN);

    /* Send start block header for file size.  */
    status = _nx_ftp_client_block_header_send(ftp_client_ptr, file_size);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* Store the file size for writing.  */
    ftp_client_ptr -> nx_ftp_client_block_total_size = file_size; 
    ftp_client_ptr -> nx_ftp_client_block_remaining_size = file_size;

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ftp_client_file_write                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the FTP client file write.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Pointer to packet to write    */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_write             Actual client file write call */ 
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
UINT  _nxe_ftp_client_file_write(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT    status;

                  
    /* Check for invalid input pointers.  */
    if ((ftp_client_ptr == NX_NULL) || (ftp_client_ptr -> nx_ftp_client_id != NXD_FTP_CLIENT_ID) || 
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file write function.  */
    status =  _nx_ftp_client_file_write(ftp_client_ptr, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_write                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes a packet to the file on the data connection    */ 
/*    with the FTP server.                                                */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Pointer to packet to write    */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
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
UINT  _nx_ftp_client_file_write(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT    status;
ULONG   file_size = 0;


    /* Determine if the file is open for writing.  */
    if (ftp_client_ptr -> nx_ftp_client_state != NX_FTP_STATE_WRITE_OPEN)
        return(NX_FTP_NOT_OPEN);

    /* Determine if the block mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
    {

        /* Record the file size.  */
        file_size = packet_ptr -> nx_packet_length;

        /* Check the file size.  */
        if (ftp_client_ptr -> nx_ftp_client_block_remaining_size < packet_ptr -> nx_packet_length)
            return(NX_FTP_CLIENT_INVALID_SIZE);
    }

    /* Write packet payload to the file.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_data_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Determine if the block mode is enabled.  */
    if (ftp_client_ptr -> nx_ftp_client_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
    {

        /* Update the file size.  */
        ftp_client_ptr -> nx_ftp_client_block_remaining_size -= file_size;

        /* Check if the file is already sent.  */
        if (ftp_client_ptr -> nx_ftp_client_block_remaining_size == 0)
        {

            /* Send end block header for file size.  */
            status = _nx_ftp_client_block_header_send(ftp_client_ptr, 0);
        }
    }

    /* Return success to caller.  */
    return(status);
}


#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_utility_convert_IPv6_to_ascii               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts an IP address to ascii e.g. for insertion    */
/*     into FTP EPRT or PORT command.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_client_ptr                   Pointer to FTP client              */
/*    buffer                           Pointer to ascii port number string*/
/*    buffer_length                    Size of buffer                     */
/*    size                             IPv6 address string size           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                       Successful conversion              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_utility_convert_number_ascii                                */
/*                                     Converts a single number to ascii  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_client_file_open         FTP Client file open service       */
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

UINT _nx_ftp_utility_convert_IPv6_to_ascii(NX_TCP_SOCKET *socket_ptr, CHAR *buffer, UINT buffer_length, UINT *size)
{


CHAR *temp;
CHAR number_buf[10]; /* 0x20010000 -> "2001:0000:" */


    /* Set a pointer to the start of the buffer. */
    temp = buffer;

    /* Clear the buffer. */
    memset(temp, 0, buffer_length);

    /* Clear the scratch buffer. */
    memset(number_buf, 0, 10);

    _nx_ftp_utility_convert_number_ascii(socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address[0], number_buf);

    /* Append to string buffer. */
    memcpy(temp, number_buf, 10); /* Use case of memcpy is verified. */

    /* Move the pointer past this number. */
    temp += 10;

    /* Clear the scratch buffer. */
    memset(number_buf, 0, 10);

     _nx_ftp_utility_convert_number_ascii(socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address[1], number_buf);

    /* Append to string buffer. */
    memcpy(temp, number_buf, 10); /* Use case of memcpy is verified. */

    /* Move the pointer past this number. */
    temp += 10;

    /* Clear the scratch buffer. */
    memset(number_buf, 0, 10);

    _nx_ftp_utility_convert_number_ascii(socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address[2], number_buf);

    /* Append to string buffer. */
    memcpy(temp, number_buf, 10); /* Use case of memcpy is verified. */

    /* Move the pointer past this number. */
    temp += 10;

    /* Clear the scratch buffer. */
    memset(number_buf, 0, 10);

     _nx_ftp_utility_convert_number_ascii(socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address[3], number_buf);

    /* Append to string buffer. */
    memcpy(temp, number_buf, 10); /* Use case of memcpy is verified. */

    /* Move the pointer past this number. */
    temp += 10;

    /* Remove the trailing ':'. */
    *(temp - 1) = 0x0;
    *size = 39;

    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_utility_convert_number_ascii                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts a number to ascii text for inserting IP      */
/*    address and port numbers into FTP EPRT and PORT commands.  It is up */
/*    to the caller to provide a buffer large enough to hold the ascii    */
/*    conversion of the input number.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                           Number to convert to ASCII         */
/*    numstring                        Pointer to ascii string            */
/*                                                                        */
/*  OUTPUT                                                                */
/*    NX_SUCCESS                       Successful completion              */
/*                                                                        */
/*  CALLS                                                                 */
/*     None                                                               */
/*                                                                        */
/*  CALLED BY                                                             */
/*     _nx_ftp_utility_convert_IPv6_to_ascii                              */
/*                                    Convert IPv6 address to ASCII       */
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

UINT  _nx_ftp_utility_convert_number_ascii(ULONG number, CHAR *numstring)
{

UINT    j; 
UCHAR   c;

    /* Go through each bit of the ULONG to convert. */
    for (j = 0; j <=7 ; j++)
    {

         /* Save the bit off the most significant end. */
         c = (UCHAR)((number & 0xF0000000) >> 28);

         /* Make it the most significant byte. */
         number = number << 4;

         /* Convert the digit to an ascii character. */
         if (c < 10)
         {
             *numstring = (CHAR)('0' + c);
         }
         else /* Handle HEX digits... */
         {
             *numstring = (CHAR)('A' + (c - 10));
         }

         /* Move past the digit. */
         numstring++;

         /* Determine if we need to add a colon. */
         if (j == 3 || j == 7)
         {
             /* Yes, append the colon and move the pointer past it. */
             *numstring = ':';
             numstring++;
         }
    }

    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_utility_convert_portnumber_ascii            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts a number to ascii text e.g. used by          */
/*    _nx_nat_utility_convert_portnumber_ULONG_to_ascii for inserting     */
/*    changed port numbers back into FTP PORT command .                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                         Port number to convert to ASCII      */
/*    numstring                      Pointer to ascii portnumber string   */
/*    numstring_length               Portnumber string length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_FTP_ERROR                   Unable to convert e.g. overflow error*/
/*    NX_SUCCESS                     Successful conversion                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_client_file_open        Processes the FTP file open service */
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

UINT  _nx_ftp_utility_convert_portnumber_ascii(UINT number, CHAR *numstring, UINT *numstring_length)
{

UINT    j;
UINT    digit;


    /* Initialize counters.  */
    (*numstring_length) =  0;

    /* Loop to convert the number to ASCII.  */
    while ((*numstring_length) < 10)
    {

        /* Shift the current digits over one.  */
        for (j = (*numstring_length); j != 0; j--)
        {

            /* Move each digit over one place.  */
            numstring[j] =  numstring[j-1];
        }

        /* Compute the next decimal digit.  */
        digit =  number % 10;

        /* Update the input number.  */
        number =  number / 10;

        /* Store the new digit in ASCII form.  */
        numstring[0] =  (CHAR) (digit + 0x30);

        /* Increment the size.  */
        (*numstring_length)++;

        /* Determine if the number is now zero.  */
        if (number == 0)
            break;
    }

    /* Make the string NULL terminated.  */
    numstring[(*numstring_length)] =  (CHAR) NX_NULL;

    /* Determine if there is an overflow error.  */
    if (number)
    {

        /* Error, return bad values to user.  */
        (*numstring_length) =  0;
        numstring[0] = '0';
        return NX_FTP_INVALID_NUMBER;    
    }

    /* Return size to caller.  */
    return NX_SUCCESS;
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_packet_allocate                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function determines which IP TCP packet to allocate based on   */
/*    the FTP connection address IP type and then allocates the packet.   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Allocated packet to return    */ 
/*    wait_option                           Packet allocate wait option   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_NOT_ENABLED                        IPv6 not enabled              */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
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
UINT _nx_ftp_client_packet_allocate(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT packet_type;
UINT status;


    /* Default to IPv4 TCP packet to allocate. */
    packet_type = NX_IPv4_TCP_PACKET;

    /* Determine IP version by FTP connection type. */
    if (ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {

#ifndef FEATURE_NX_IPV6

        return NX_NOT_ENABLED;
#else

        /* Allocate a TCP IPv6 packet. */
        packet_type = NX_IPv6_TCP_PACKET;
#endif
    }


    /* Allocate the actual packet. */
    status =  nx_packet_allocate(ftp_client_ptr -> nx_ftp_client_packet_pool_ptr, packet_ptr, packet_type, wait_option);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_active_transfer_setup                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the Client data socket for active transfer    */
/*    where the Client data socket is the TCP client.                     */
/*                                                                        */ 
/*    Find free port and listen on it, sends the PORT/EPRT command to     */ 
/*    server, then wait for a connection from server.                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_packet_allocate        Allocate packet               */ 
/*    _nx_ftp_utility_convert_IPv6_to_ascii Convert IPv6 address to ASCII */
/*    _nx_ftp_utility_convert_portnumber_ascii Convert port to ASCII      */
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_free_port_find                 Find a free port              */ 
/*    nx_tcp_server_socket_listen           Listen for TCP data socket    */ 
/*                                            connection from server      */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server socket     */ 
/*    nx_tcp_socket_receive                 Receive response from server  */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_open              Open a file for read or write */ 
/*    _nx_ftp_client_directory_listing_get  Get directory list            */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            fixed packet leak,          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ftp_client_active_transfer_setup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

UINT        status;
UINT        data_port;
#ifndef NX_DISABLE_IPV4
ULONG       ip_address;
#endif /* NX_DISABLE_IPV4 */
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
#ifdef FEATURE_NX_IPV6
UINT        ipduo_size;
#endif


    /* Pickup the next free port for the data socket.  */
    status = nx_tcp_free_port_find(ftp_client_ptr -> nx_ftp_client_ip_ptr, 
                                   ftp_client_ptr -> nx_ftp_client_data_port + 1, &data_port);

    /* Determine if the port find was successful.  */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Save the data port to the client record. */
    ftp_client_ptr -> nx_ftp_client_data_port = data_port;

    /* Start listening on the data port.  */
    status = nx_tcp_server_socket_listen(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port, &(ftp_client_ptr -> nx_ftp_client_data_socket), 5, NX_NULL);

    /* Determine if the listen is successful.  */
    if (status != NX_SUCCESS)
    {

        /* Return error.  */
        return(status);
    }

    /* Allocate a packet for sending the PORT/EPRT command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Stop listening on the data port.  */
        nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

        /* Return error.  */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build the IP and port number FTP message.  */
    if (ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
#ifndef FEATURE_NX_IPV6
        return NX_NOT_ENABLED;
#else
    CHAR        ipduo_buffer[NX_FTP_IPV6_ADDRESS_BUFSIZE];
    UINT        index;

        /* EPRT command: EPRT |2|1080::8:800:200C:417A|5282|  
           RFC2428, Section2, Page3. */
    
        /* Check if out of boundary.
           Max EPRT command sring: 8 ("EPRT |2|") + 39 (IPv6 address string) + 1 ('|') + 5 (Max port string) + 1 ('|') + 2 ("\r\n") = 56.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 56)
        {

            /* Stop listening on the data port.  */
            nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_INVALID_PACKET);
        }

        buffer_ptr[0] =  'E';
        buffer_ptr[1] =  'P';
        buffer_ptr[2] =  'R';
        buffer_ptr[3] =  'T';
        buffer_ptr[4] =  ' ';
        buffer_ptr[5] =  '|'; 
        buffer_ptr[6] =  '2'; 
        buffer_ptr[7] =  '|'; 
    
        /* Convert the PORT command ipv6 address and port number format to ascii text. */
    
        /* Clear our scratch buffer. */
        memset(&ipduo_buffer[0], 0, NX_FTP_IPV6_ADDRESS_BUFSIZE);
    
        /* Convert the IPv6 address to text. */
        status = _nx_ftp_utility_convert_IPv6_to_ascii(&(ftp_client_ptr -> nx_ftp_client_control_socket), 
                                                       &ipduo_buffer[0], NX_FTP_IPV6_ADDRESS_BUFSIZE, &ipduo_size);
    
        /* Append to the packet buffer . */
        memcpy(buffer_ptr + 8, ipduo_buffer, ipduo_size); /* Use case of memcpy is verified. */
    
        /* Update the index past the IPv6 address. */
        index = 8 + ipduo_size;
    
        /* Add the required space character. */
        buffer_ptr[index++] = '|';
    
        /* Clear the scratch buffer again. */
        memset(&ipduo_buffer[0], 0, NX_FTP_IPV6_ADDRESS_BUFSIZE);
    
        /* Convert the preferred port to ascii. */
        status = _nx_ftp_utility_convert_portnumber_ascii(data_port, &ipduo_buffer[0], &ipduo_size);
    
        /* Append to the packet buffer. */
        memcpy(buffer_ptr + index, ipduo_buffer, ipduo_size); /* Use case of memcpy is verified. */
    
        /* Update the index past the port. */
        index += ipduo_size;
                                 
        /* Add the required space character. */
        buffer_ptr[index++] = '|';

        /* Set the CR/LF.  */
        buffer_ptr[index++] = 13;
        buffer_ptr[index++] = 10;
    
        /* Set the packet length.  */
        packet_ptr -> nx_packet_length = index;
#endif /*  FEATURE_NX_IPV6 */
    }
    else
    {
        /* IPv4 FTP connection */
#ifdef NX_DISABLE_IPV4
        return NX_NOT_ENABLED;
#else

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 30)
        {

            /* Stop listening on the data port.  */
            nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Set the error status.  */
            return(NX_INVALID_PACKET);
        }

        /* Pickup the IPv4 address of this IP instance.  */
        ip_address =  ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_interface -> nx_interface_ip_address ;

        /* Now build the IP and port number FTP message.  */
        buffer_ptr[0] =  'P';
        buffer_ptr[1] =  'O';
        buffer_ptr[2] =  'R';
        buffer_ptr[3] =  'T';
        buffer_ptr[4] =  ' ';
    
        buffer_ptr[5] =  (UCHAR)('0' + (ip_address >> 24)/100);
        buffer_ptr[6] =  (UCHAR)('0' + ((ip_address >> 24)/10)%10);
        buffer_ptr[7] =  (UCHAR)('0' + (ip_address >> 24)%10);
        buffer_ptr[8] =  ',';
    
        buffer_ptr[9]  = (UCHAR)('0' + ((ip_address >> 16) & 0xFF)/100);
        buffer_ptr[10] = (UCHAR)('0' + (((ip_address >> 16) & 0xFF)/10)%10);
        buffer_ptr[11] = (UCHAR)('0' + ((ip_address >> 16) & 0xFF)%10);
        buffer_ptr[12] = ',';
    
        buffer_ptr[13] = (UCHAR)('0' + ((ip_address >> 8) & 0xFF)/100);
        buffer_ptr[14] = (UCHAR)('0' + (((ip_address >> 8) & 0xFF)/10)%10);
        buffer_ptr[15] = (UCHAR)('0' + ((ip_address >> 8) & 0xFF)%10);
        buffer_ptr[16] = ',';

        buffer_ptr[17] = (UCHAR)('0' + (ip_address & 0xFF)/100);
        buffer_ptr[18] = (UCHAR)('0' + ((ip_address & 0xFF)/10)%10);
        buffer_ptr[19] = (UCHAR)('0' + (ip_address & 0xFF)%10);
        buffer_ptr[20] = ',';
    
        buffer_ptr[21] = (UCHAR)('0' + (data_port >> 8)/100);
        buffer_ptr[22] = (UCHAR)('0' + ((data_port >> 8)/10)%10);
        buffer_ptr[23] = (UCHAR)('0' + ((data_port >> 8)%10));
        buffer_ptr[24] = ',';
    
        buffer_ptr[25] = (UCHAR)('0' + (data_port & 255)/100);
        buffer_ptr[26] = (UCHAR)('0' + ((data_port & 255)/10)%10);
        buffer_ptr[27] = (UCHAR)('0' + ((data_port & 255)%10));
    
        /* Set the CR/LF.  */
        buffer_ptr[28] = 13;
        buffer_ptr[29] = 10;
    
        /* Set the packet length.  */
        packet_ptr -> nx_packet_length =  30;
#endif /* NX_DISABLE_IPV4 */
    }

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the PORT/EPRT message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status != NX_SUCCESS)
    {

        /* Stop listening on the data port.  */
        nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status != NX_SUCCESS)
    {

        /* Stop listening on the data port.  */
        nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

        /* Unable to open file with FTP server. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the IP/port was received properly.  */
    if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2'))
    {

        /* Stop listening on the data port.  */
        nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, data_port);

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to open file with FTP server. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_passive_transfer_setup               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the Client data socket for passive transfer   */
/*    where the Client data socket is the TCP client.                     */
/*                                                                        */ 
/*    It sends the PASV command and if the server sends an OK status, it  */
/*    extracts the server connection info from the server response and    */
/*    opens a connection on the data socket.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    wait_option                           Specifies how long to wait    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ftp_client_packet_allocate        Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_client_socket_bind             Bind the data socket to a port*/
/*    nxd_tcp_client_socket_connect         Connect to server data socket */
/*    nx_tcp_socket_receive                 Receive response from server  */ 
/*    nx_tcp_socket_send                    Send request to server        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */
/*    _nx_ftp_client_file_open              Open a file for read or write */ 
/*    _nx_ftp_client_directory_listing_get  Get directory list            */ 
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
UINT  _nx_ftp_client_passive_transfer_setup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        status;
UINT        i;
UINT        data_port;
#ifndef NX_DISABLE_IPV4
UINT        commas =      0;
ULONG       ip_address =  0;
ULONG       temp =        0;
#endif /* NX_DISABLE_IPV4  */


    /* Send the PASV command to switch to passive mode. */

    /* Allocate a packet for sending the TYPE command.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, wait_option);

    /* Determine if the packet allocation was successful.  */
    if (status != NX_SUCCESS)
    {

        /* Return error.  */
        return(status);
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 6)
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Set the error status.  */
        return(NX_FTP_FAILED);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    if (ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
#ifndef FEATURE_NX_IPV6
        return(NX_NOT_ENABLED);
#else

        /* Now build "EPSV" message. RFC2428.  */
        buffer_ptr[0] =  'E';
        buffer_ptr[1] =  'P';
        buffer_ptr[2] =  'S';
        buffer_ptr[3] =  'V';
#endif /*  FEATURE_NX_IPV6 */
    }
    else
    {
#ifdef NX_DISABLE_IPV4
        return(NX_NOT_ENABLED);
#else

        /* Now build "PASV" message .  */
        buffer_ptr[0] =  'P';
        buffer_ptr[1] =  'A';
        buffer_ptr[2] =  'S';
        buffer_ptr[3] =  'V';
#endif /* NX_DISABLE_IPV4 */
    }

    /* Set the CR/LF.  */
    buffer_ptr[4] =  13;
    buffer_ptr[5] =  10; 

    /* Set the packet length.  */
    packet_ptr -> nx_packet_length =  6;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    
    /* Send the PASV or EPSV message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Wait for response from the FTP server.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);    

    /* Determine if a packet was not received.  */
    if (status != NX_SUCCESS)
    {

        /* Unable to open file with FTP server. */
        return(status);
    }

    /* Setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    if (ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
#ifndef FEATURE_NX_IPV6
        return(NX_NOT_ENABLED);
#else

        /* EPSV command, RFC2428.  */

        /* Check for 2xx message, signaling the "EPSV" was received properly.  */
        if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2') || (buffer_ptr[1] != '2') || (buffer_ptr[2] != '9'))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Unable to open file with FTP server. */
            return(NX_FTP_EXPECTED_2XX_CODE);
        }

        /* The text returned in response to the EPSV command MUST be:
             <text indicating server is entering extended passive mode> \
             (<d><d><d><tcp-port><d>)  */

        /* The netowrk address used to establish the data connnection will be the same
           network address used for the control connection.
           An example response string follows:
             Entering Extended Passive Mode (|||6446|)  */

        /* Check the length. command + <sp> + passive key word "(|||"*/
        if (packet_ptr -> nx_packet_length < 8)
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_FTP_INVALID_COMMAND);
        }

        /* Set the index.  */
        i = 7;

        /* Skip the text to locate the key word "(|||". */
        while (i < packet_ptr -> nx_packet_length)
        {

            /* Check if match the key word.  */
            if ((buffer_ptr[i - 3] == '(') &&
                (buffer_ptr[i - 2] == '|') &&
                (buffer_ptr[i - 1] == '|') &&
                (buffer_ptr[i] == '|'))
            {
                break;
            }

            /* Move to next character.  */
            i++;
        }

        /* Check for properly formated address command from server. */
        if (i >= packet_ptr -> nx_packet_length) 
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_FTP_INVALID_COMMAND);
        }

        /* Update the index to pickup the port number.  */
        data_port =  0;
        i++;
        while ((i < packet_ptr -> nx_packet_length) && (buffer_ptr[i] != '|'))
        {

            /* Is a numeric character present?  */
            if ((buffer_ptr[i] < '0') || (buffer_ptr[i] > '9'))
            {

                /* Invalid port number.  */
                break;
            }

            /* Yes, numeric character is present. Update the IP port.  */
            data_port = (data_port * 10) + (UINT) (buffer_ptr[i] - '0');

            /* Move to next character.  */
            i++;
        }

        /* At this point we are done with the packet. */
        nx_packet_release(packet_ptr);

        /* Determine if an error occurred.  */
        if ((buffer_ptr[i] != '|') || (data_port == 0))
        {

            /* Set the error status.  */
            return(NX_FTP_INVALID_COMMAND);
        }

        /* Save the passive IP address and  data port.  */
        COPY_NXD_ADDRESS(&(ftp_client_ptr -> nx_ftp_client_control_socket.nx_tcp_socket_connect_ip), &(ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_ip));
        ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_port = data_port;
#endif /*  FEATURE_NX_IPV6 */
    }
    else
    {
#ifdef NX_DISABLE_IPV4
        return(NX_NOT_ENABLED);
#else

        /* Check for 2xx message, signaling the "PASV" was received properly.  */
        if ((packet_ptr -> nx_packet_length < 3) || (buffer_ptr[0] != '2') || (buffer_ptr[1] != '2') || (buffer_ptr[2] != '7'))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Unable to open file with FTP server. */
            return(NX_FTP_EXPECTED_2XX_CODE);
        }

        /* Now parse the address and port for the Client to connect to on the data socket. 
           IP address and port are comma delimited and port number is the last two numbers,
           PASV port = (p1 * 256) + p2.  */

        /* Skip non numeric text and whitespace.  */
        i = 3;
        while (((buffer_ptr[i] > '9') || (buffer_ptr[i] < '0')) && (i < packet_ptr -> nx_packet_length))
        {
            i++;
        }

        /* Check for properly formated command from server.  */
        if (i >= packet_ptr -> nx_packet_length) 
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return NX_FTP_INVALID_COMMAND;
        }

        /* First, pickup the IP address.  */
        commas =      0;
        ip_address =  0;
        temp =        0;
        status =      NX_SUCCESS;

        while (i < packet_ptr -> nx_packet_length)
        {

            /* Is a numeric character present?  */
            if ((buffer_ptr[i] >= '0') && (buffer_ptr[i] <= '9'))
            {

                /* Yes, numeric character is present.  Update the IP address.  */
                temp =  (temp*10) + (ULONG) (buffer_ptr[i] - '0');
            }

            /* Determine if a CR/LF is present.  */
            if ((buffer_ptr[i] == 0x0D) || (buffer_ptr[i] == 0x0A) || (buffer_ptr[i] == 0))
            {
                status =  NX_FTP_INVALID_COMMAND;
                break;
            }

            /* Determine if a comma is present.  */
            if (buffer_ptr[i] == ',')
            {

                /* Increment the comma count. */
                commas++;

                /* Setup next byte of IP address.  */
                ip_address =  (ip_address << 8) & 0xFFFFFFFF;
                ip_address =  ip_address | (temp & 0xFF);
                temp =  0;

                /* Have we finished with the IP address?  */
                if (commas == 4)
                {

                    /* Finished with IP address.  */
                    i++;
                    break;
                }
            }

            /* Move to next character.  */
            i++;
        }

        /* Now pickup the port number.  */
        data_port =  0;
        temp =  0;
        while (i < packet_ptr -> nx_packet_length)
        {

            /* Is a numeric character present?  */
            if ((buffer_ptr[i] >= '0') && (buffer_ptr[i] <= '9'))
            {

                /* Yes, numeric character is present.  Update the IP port.  */
                temp =  (temp*10) + (UINT) (buffer_ptr[i] - '0');
            }

            /* Determine if a CR/LF is present.  */
            if ((buffer_ptr[i] == 0x0D) || (buffer_ptr[i] == 0x0A) || (buffer_ptr[i] == 0))
            {
                /* Good condition on the port number!  */
                break;
            }

            /* Determine if a comma is present.  */
            if (buffer_ptr[i] == ',')
            {

                /* Increment the comma count. */
                commas++;

                /* Move the data port number up.  */
                data_port =  (data_port << 8) & 0xFFFFFFFF;
                data_port =  data_port | (temp & 0xFF);
                temp =  0;

                /* Have we finished with the port?  */
                if (commas >= 6)
                {

                    /* Error, get out of the loop.  */
                    status =  NX_FTP_INVALID_ADDRESS;
                    break;
                }
            }

            /* Move to next character.  */
            i++;
        }

        /* Move port number up.  */
        data_port =  (data_port << 8) & 0xFFFFFFFF;
        data_port =  data_port | (temp & 0xFF);

        /* At this point we are done with the packet. */
        nx_packet_release(packet_ptr);

        /* Determine if an error occurred.  */
        if ((status != NX_SUCCESS) || (buffer_ptr[i] != 13) || (commas != 5) || (ip_address == 0) || (data_port == 0))
        {

            /* Set the error status.  */
            return NX_FTP_INVALID_COMMAND;
        }

        /* Save the passive IP address and  data port.  */
        ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V4;
        ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4 = ip_address;
        ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_port =  data_port;

#endif /* NX_DISABLE_IPV4 */
    }

    /* Bind the client data socket to any port.  */
    status =  nx_tcp_client_socket_bind((&ftp_client_ptr -> nx_ftp_client_data_socket), NX_ANY_PORT, wait_option);

    /* Check for an error.  */
    if (status != NX_SUCCESS) 
    {

        /* Unable to bind socket to port. */
        return(status);
    }

    /* Now connect to the IP address and port the server asked us to.  */
    status =  nxd_tcp_client_socket_connect(&(ftp_client_ptr -> nx_ftp_client_data_socket),
                                            &(ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_connect_ip),
                                            data_port, wait_option);

    /* Check for an error.  */
    if (status != NX_SUCCESS)
    {

        /* Unbind the socket.  */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_data_socket));
    }

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_data_socket_cleanup                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function cleans up the data socket.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_disconnect              Disconnect a socket           */ 
/*    nx_tcp_client_socket_unbind           Release the data socket port  */
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server socket     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_open              Open a file for read or write */ 
/*    _nx_ftp_client_file_close             Actual client file close call */ 
/*    _nx_ftp_client_directory_listing_get  Get directory list            */ 
/*    _nx_ftp_client_directory_listing_continue                           */ 
/*                                          Continue to get directory list*/ 
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
VOID  _nx_ftp_client_data_socket_cleanup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{


    /* Disconnect the data socket.  */
    nx_tcp_socket_disconnect(&(ftp_client_ptr -> nx_ftp_client_data_socket), wait_option);

    /* Check if enable passive mode.  */
    if (ftp_client_ptr -> nx_ftp_client_passive_transfer_enabled == NX_FALSE)
    {

        /* Unaccept the data socket.  */
        nx_tcp_server_socket_unaccept(&(ftp_client_ptr -> nx_ftp_client_data_socket));

        /* Stop listening on the data port.  */
        nx_tcp_server_socket_unlisten(ftp_client_ptr -> nx_ftp_client_ip_ptr, 
                                      ftp_client_ptr -> nx_ftp_client_data_socket.nx_tcp_socket_port);
    }
    else
    {

        /* Release the socket port. */
        nx_tcp_client_socket_unbind(&(ftp_client_ptr -> nx_ftp_client_data_socket));
    }

    /* Clear the block size.  */
    ftp_client_ptr -> nx_ftp_client_block_total_size = 0;
    ftp_client_ptr -> nx_ftp_client_block_remaining_size = 0;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_block_mode_send                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends MODE B command to FTP server for block mode.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_open              Open a file for read or write */ 
/*    _nx_ftp_client_directory_listing_get  Get directory list            */ 
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
UINT  _nx_ftp_client_block_mode_send(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option)
{

UINT        status;
NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;


    /* Send MODE B command.  */

    /* Allocate the packet.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, NX_NO_WAIT);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Now build the actual MODE B request.  */
    buffer_ptr[0] = 'M';
    buffer_ptr[1] = 'O';
    buffer_ptr[2] = 'D';
    buffer_ptr[3] = 'E';
    buffer_ptr[4] = ' ';
    buffer_ptr[5] = 'B';

    /* Insert the CR/LF.  */
    buffer_ptr[6] = 13;
    buffer_ptr[7] = 10;

    /* Setup the length of the packet.  */
    packet_ptr -> nx_packet_length = 8;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the MODE B message.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_control_socket), packet_ptr, wait_option);

    /* Determine if the send was unsuccessful.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error.  */
        return(status);
    }

    /* Now wait for response from the FTP server control port.  */
    status =  nx_tcp_socket_receive(&(ftp_client_ptr -> nx_ftp_client_control_socket), &packet_ptr, wait_option);

    /* Determine if a packet was not received.  */
    if (status != NX_SUCCESS)
    {

        /* Unable to open file with FTP server. */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Check for 2xx message, signaling the data port was connected properly and ready for transfer.  */
    if (buffer_ptr[0] != '2')
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Unable to open file with FTP server. */
        return(NX_FTP_EXPECTED_2XX_CODE);
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Return success to caller. */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_block_header_send                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends the block header for block mode.                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    block_size                            The size of block data        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_size_set          Set the file size for writing */ 
/*    _nx_ftp_client_file_write             Send data packet to server    */ 
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
UINT  _nx_ftp_client_block_header_send(NX_FTP_CLIENT *ftp_client_ptr, ULONG block_size)
{

UINT        status;
NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;


    /* Use block mode to transfer data.  RFC959, Section3.4.2, Page21-22.  */

    /* Allocate the packet.  */
    status = _nx_ftp_client_packet_allocate(ftp_client_ptr, &packet_ptr, NX_NO_WAIT);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Return error.  */
        return(status);
    }

    /* We have a packet, setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Set the block header.  */
    if (block_size)
    {

        /* Descriptor.  */
        buffer_ptr[0] = 0;

        /* Byte count.  */
        buffer_ptr[1] = (UCHAR)(block_size >> 8);
        buffer_ptr[2] = (UCHAR)(block_size);
    }
    else
    {

        /* Descriptor.  */
        buffer_ptr[0] = 64;

        /* Byte count.  */
        buffer_ptr[1] = 0;
        buffer_ptr[2] = 0;
    }

    /* Setup the length of the packet.  */
    packet_ptr -> nx_packet_length = 3;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Write packet payload to the file.  */
    status =  nx_tcp_socket_send(&(ftp_client_ptr -> nx_ftp_client_data_socket), packet_ptr, NX_NO_WAIT);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

    /* Return success to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_client_block_header_retrieve                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the block header for block mode.            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_client_ptr                        Pointer to FTP client         */ 
/*    packet_ptr                            Pointer to packet to write    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_tcp_socket_send                    Send data packet to server    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_client_file_read              Read data from server         */ 
/*    _nx_ftp_client_directory_listing_get  Get directory list            */ 
/*    _nx_ftp_client_directory_listing_continue                           */ 
/*                                          Continue to get directory list*/ 
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
UINT  _nx_ftp_client_block_header_retrieve(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr)
{

ULONG       delta;
UCHAR       *buffer_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET   *before_last_packet;
NX_PACKET   *last_packet;
#endif /* NX_DISABLE_PACKET_CHAIN */


    /* Check if it is the first packet.  */
    if (ftp_client_ptr -> nx_ftp_client_block_total_size == 0)
    {

        /* Check the packet length.  */
        if ((packet_ptr -> nx_packet_length < 3) ||
            (packet_ptr -> nx_packet_prepend_ptr + 3 > packet_ptr -> nx_packet_append_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_FTP_CLIENT_INVALID_SIZE);
        }

        /* We have a packet, setup pointer to the buffer area.  */
        buffer_ptr = packet_ptr -> nx_packet_prepend_ptr;

        /* Process block header.  */
        ftp_client_ptr -> nx_ftp_client_block_total_size = (ULONG)((buffer_ptr[1] << 8) | buffer_ptr[2]);
        ftp_client_ptr -> nx_ftp_client_block_remaining_size = ftp_client_ptr -> nx_ftp_client_block_total_size;

        /* Skip the block header.  */
        packet_ptr -> nx_packet_prepend_ptr += 3;
        packet_ptr -> nx_packet_length -= 3;
    }

    /* Check if have remaining data.  */
    if (ftp_client_ptr -> nx_ftp_client_block_remaining_size == 0)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return(NX_FTP_CLIENT_END_OF_BLOCK);
    }

    /* Check the data of current packet.  */
    if (ftp_client_ptr -> nx_ftp_client_block_remaining_size < packet_ptr -> nx_packet_length)
    {

        /* Remove the extra data, such as: end block header.  */

        /* Calculate the difference in the length.  */
        delta =  packet_ptr -> nx_packet_length - ftp_client_ptr -> nx_ftp_client_block_remaining_size;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - delta;

        /* Adjust the append pointer.  */

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Loop to process adjustment that spans multiple packets.  */
        while (delta)
        {

            /* Determine if the packet is chained (or still chained after the adjustment).  */
            if (packet_ptr -> nx_packet_last == NX_NULL)
            {

                /* No, packet is not chained, simply adjust the append pointer in the packet.  */
                packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr - delta;

                /* Break out of the loop, since the adjustment is complete.  */
                break;
            }

            /* Pickup the pointer to the last packet.  */
            last_packet =  packet_ptr -> nx_packet_last;

            /* Determine if the amount to adjust is less than the payload in the last packet.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            if (((ULONG)(last_packet -> nx_packet_append_ptr - last_packet -> nx_packet_prepend_ptr)) > delta)
            {

                /* Yes, simply adjust the append pointer of the last packet in the chain.  */
                /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                last_packet -> nx_packet_append_ptr =  last_packet -> nx_packet_append_ptr - delta;

                /* Get out of the loop, since the adjustment is complete.  */
                break;
            }
            else
            {

                /* Adjust the delta by the amount in the last packet.  */
                delta =  delta - ((ULONG)(last_packet -> nx_packet_append_ptr - last_packet -> nx_packet_prepend_ptr));

                /* Find the packet before the last packet.  */
                before_last_packet =  packet_ptr;
                while (before_last_packet -> nx_packet_next != last_packet)
                {

                    /* Move to the next packet in the chain.  */
                    before_last_packet =  before_last_packet -> nx_packet_next;
                }

                /* At this point, we need to release the last packet and adjust the other packet
                    pointers.  */

                /* Ensure the next packet pointer is NULL in what is now the last packet.  */
                before_last_packet -> nx_packet_next =  NX_NULL;

                /* Determine if the packet is still chained.  */
                if (packet_ptr != before_last_packet)
                {

                    /* Yes, the packet is still chained, setup the last packet pointer.  */
                    packet_ptr -> nx_packet_last =  before_last_packet;
                }
                else
                {

                    /* The packet is no longer chained, set the last packet pointer to NULL.  */
                    packet_ptr -> nx_packet_last =  NX_NULL;
                }

                /* Release the last packet.   */
                _nx_packet_release(last_packet);
            }
        }
#else

        /* Simply adjust the append pointer in the packet.  */
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr - delta;
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Update the file size.  */
    ftp_client_ptr -> nx_ftp_client_block_remaining_size -= packet_ptr -> nx_packet_length;

    /* Return success to caller.  */
    return(NX_SUCCESS);
}
