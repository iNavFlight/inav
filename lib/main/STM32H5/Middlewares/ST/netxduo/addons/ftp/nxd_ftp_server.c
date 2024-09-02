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
/**   File Transfer Protocol                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_FTP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* If FileX is not used in this application, define this option and define the FileX services
   declared in filex_stub.h elsewhere. 
#define      NX_FTP_NO_FILEX
*/

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_ftp_server.h"
#include    "stdio.h"
#include    "string.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/* Define FTP Error codes.  */

#define NX_FTP_CODE_RESTART_MARKER  "110"   /* Restart marker reply.
                                               In this case, the text is exact and not left to the
                                               particular implementation; it must read:
                                                  MARK yyyy = mmmm
                                               Where yyyy is User-process data stream marker, and mmmm
                                               server's equivalent marker (note the spaces between markers
                                               and "="). */
#define NX_FTP_CODE_READY_NNN        "120"  /* Service ready in nnn minutes.  */
#define NX_FTP_CODE_START_XFER       "125"  /* Data connection already open; transfer starting.  */
#define NX_FTP_CODE_OPENING          "150"  /* File status okay; about to open data connection.  */
#define NX_FTP_CODE_CMD_OK           "200"  /* Command okay.  */
#define NX_FTP_CODE_CONNECTION_OK    "220"  /* Connection okay.  */
#define NX_FTP_CODE_CLOSE            "221"  /* Service closing control connection.  */
#define NX_FTP_CODE_LOGOFF           "226"  /* Closing data connection.  */
#define NX_FTP_CODE_LOGIN            "230"  /* User logged in, proceed.  */
#define NX_FTP_CODE_COMPLETED        "250"  /* Requested file action okay, completed.  */
#define NX_FTP_CODE_MADE_DIR         "257"  /* PATHNAME created.  */
#define NX_FTP_CODE_USER_OK          "331"  /* User name okay, need password.  */
#define NX_FTP_CODE_NEED_ACCT        "332"  /* Need account for login.  */
#define NX_FTP_CODE_FILE_PEND        "350"  /* Requested file action pending further information.  */
#define NX_FTP_CODE_CMD_FAIL         "501"  /* Syntax error in parameters or arguments.  */
#define NX_FTP_CODE_NOT_IMPLEMENTED  "502"  /* Command not implemented.  */
#define NX_FTP_CODE_UNAUTHORIZED     "530"  /* Not logged in.  */
#define NX_FTP_CODE_NO_ACCT          "532"  /* Need account for storing files.  */
#define NX_FTP_CODE_BAD_TYPE         "504"  /* Invalid TYPE.  */
#define NX_FTP_CODE_BAD_FILE         "550"  /* Requested action not taken. File unavailable (e.g., file not found, no access).  */
#define NX_FTP_CODE_BAD_PAGE_TYPE    "551"  /* Requested action aborted: page type unknown.  */
#define NX_FTP_CODE_NO_SPACE         "552"  /* Requested file action aborted, no space.  */
#define NX_FTP_CODE_BAD_NAME         "553"  /* Requested action not taken, File name not allowed.  */

static VOID _nx_ftp_server_number_to_ascii(UCHAR *buffer_ptr, UINT buffer_size, UINT number, UCHAR pad);
                                   
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ftp_server_create                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the FTP server create call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    ftp_server_name                       Name of FTP server            */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    ftp_login                             Pointer to user's login       */
/*                                            function                    */
/*    ftp_logout                            Pointer to user's logout      */
/*                                            function                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_create                 Actual server create call     */
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
UINT  _nxe_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
            UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
            UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info))
{

#ifndef NX_DISABLE_IPV4
UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (ftp_server_ptr == NX_NULL) || (ftp_logout == NX_NULL) || (ftp_login == NX_NULL) ||
        (stack_ptr == NX_NULL) || (pool_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server create function.  */
    status =  _nx_ftp_server_create(ftp_server_ptr, ftp_server_name, ip_ptr, media_ptr, stack_ptr, stack_size, pool_ptr, ftp_login, ftp_logout);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(ftp_server_name);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(media_ptr);
    NX_PARAMETER_NOT_USED(stack_ptr);
    NX_PARAMETER_NOT_USED(stack_size);
    NX_PARAMETER_NOT_USED(pool_ptr);
    NX_PARAMETER_NOT_USED(ftp_login);
    NX_PARAMETER_NOT_USED(ftp_logout);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_create                              PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an FTP server on the specified IP.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    ftp_server_name                       Name of FTP server            */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    ftp_login                             Pointer to user's login       */
/*                                            function                    */
/*    ftp_logout                            Pointer to user's logout      */
/*                                            function                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_create_internal        Actual server create call     */
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
UINT  _nx_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
            UINT (*ftp_login_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
            UINT (*ftp_logout_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info))
{

#ifndef NX_DISABLE_IPV4
UINT status;

                                                   
    /* Call actual server create function but set the ftp login and logout arguments to NULL.  */
    status =  _nx_ftp_server_create_internal(ftp_server_ptr, ftp_server_name, ip_ptr, media_ptr, stack_ptr, stack_size, pool_ptr, NX_NULL, NX_NULL);
                                 
    /* Set the FTP server to accept login functions having IPv4 address arguments. */
    ftp_server_ptr -> nx_ftp_login_ipv4 = ftp_login_ipv4;
    ftp_server_ptr -> nx_ftp_logout_ipv4 = ftp_logout_ipv4;

    /* Return status.  */
    return status;        
#else
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(ftp_server_name);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(media_ptr);
    NX_PARAMETER_NOT_USED(stack_ptr);
    NX_PARAMETER_NOT_USED(stack_size);
    NX_PARAMETER_NOT_USED(pool_ptr);
    NX_PARAMETER_NOT_USED(ftp_login_ipv4);
    NX_PARAMETER_NOT_USED(ftp_logout_ipv4);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}
       

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxde_ftp_server_create                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the FTP duo (IPv4 and IPv6)      */
/*    server create call.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    ftp_server_name                       Name of FTP server            */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    ftp_login                             Pointer to user's login       */
/*                                            function                    */
/*    ftp_logout                            Pointer to user's logout      */
/*                                            function                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ftp_server_create                Actual server create call     */
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
UINT  _nxde_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
            UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
            UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info))
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (ftp_server_ptr == NX_NULL) || (ftp_server_ptr -> nx_ftp_server_id == NXD_FTP_SERVER_ID) ||
        (ftp_logout == NX_NULL) || (ftp_login == NX_NULL) ||
        (stack_ptr == NX_NULL) || (pool_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server create function.  */
    status =  _nxd_ftp_server_create(ftp_server_ptr, ftp_server_name, ip_ptr, media_ptr, stack_ptr, stack_size, pool_ptr, ftp_login, ftp_logout);

    /* Return completion status.  */
    return(status);
}        


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ftp_server_create                             PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a "duo" FTP server on the specified IP for    */
/*    IPv4 or IPv6 networks.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    ftp_server_name                       Name of FTP server            */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    ftp_login                             Pointer to user's login       */
/*                                            function                    */
/*    ftp_logout                            Pointer to user's logout      */
/*                                            function                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_create_internal        Actual server create call     */
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
UINT  _nxd_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
            UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
            UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info))
{
              

UINT status;


    /* Call actual server create function.  */
    status =  _nx_ftp_server_create_internal(ftp_server_ptr, ftp_server_name, ip_ptr, media_ptr, stack_ptr, stack_size, pool_ptr, ftp_login, ftp_logout);
                                                            
    /* Return status.  */
    return status; 
}

           
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_create_internal                     PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a FTP server on the specified IP.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    ftp_server_name                       Name of FTP server            */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    ftp_login                             Pointer to user's login       */
/*                                            function                    */
/*    ftp_logout                            Pointer to user's logout      */
/*                                            function                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    NX_FTP_INSUFFICIENT_PACKET_PAYLOAD    Packet payload too small      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_socket_create                  Create control sockets        */
/*    nx_tcp_socket_delete                  Delete control sockets        */
/*    nx_tcp_socket_receive_notify          Register receive notify       */
/*                                            callback                    */
/*    tx_event_flags_create                 Create event flags            */
/*    tx_event_flags_delete                 Delete event flags            */
/*    tx_thread_create                      Create FTP server thread      */
/*    tx_thread_delete                      Delete FTP server thread      */
/*    tx_timer_create                       Create FTP server timer       */
/*    tx_timer_delete                       Delete FTP server timer       */
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
UINT  _nx_ftp_server_create_internal(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
            UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
            UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info))
{

UINT            i;
UINT            status;


    /* Clear the FTP server structure.  */
    memset((void *) ftp_server_ptr, 0, sizeof(NX_FTP_SERVER));

    /* Check the supplied packet pool for minimum required payload length (NX_FTP_SERVER_MIN_PACKET_PAYLOAD). This configurable
       option is explained in more detail in the header file.  */    
    if (pool_ptr -> nx_packet_pool_payload_size < NX_FTP_SERVER_MIN_PACKET_PAYLOAD)
    {

        return NX_FTP_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Save the packet pool pointer.  */
    ftp_server_ptr -> nx_ftp_server_packet_pool_ptr =  pool_ptr;

    /* Create the FTP Server thread.  */
    status =  tx_thread_create(&(ftp_server_ptr -> nx_ftp_server_thread), "FTP Server Thread", _nx_ftp_server_thread_entry,
                               (ULONG) ftp_server_ptr, stack_ptr, stack_size, NX_FTP_SERVER_PRIORITY, NX_FTP_SERVER_PRIORITY,
                               NX_FTP_SERVER_TIME_SLICE, TX_DONT_START);

    /* Determine if an error occurred creating the thread.  */
    if (status)
    {

        /* Error creating the server thread.  */
        return(status);
    }

    /* Create the ThreadX event flags.  These will be used to driver the FTP server thread.  */
    status =  tx_event_flags_create(&(ftp_server_ptr -> nx_ftp_server_event_flags), 
                                    "FTP Server Thread Events");

    /* Determine if an error occurred creating the event flags.  */
    if (status)
    {


        /* Delete the server thread.  */
        tx_thread_delete(&(ftp_server_ptr -> nx_ftp_server_thread));

        /* Error creating the server event flags.  */
        return(status);
    }

    /* Create the ThreadX activity timeout timer.  This will be used to periodically check to see if
       a client connection has gone silent and needs to be terminated.  */
    status =  tx_timer_create(&(ftp_server_ptr -> nx_ftp_server_timer), "FTP Server Timer", _nx_ftp_server_timeout,
                              (ULONG) ftp_server_ptr, (NX_IP_PERIODIC_RATE * NX_FTP_TIMEOUT_PERIOD), 
                              (NX_IP_PERIODIC_RATE * NX_FTP_TIMEOUT_PERIOD), TX_NO_ACTIVATE);

    /* Determine if an error occurred creating the timer.  */
    if (status)
    {

        /* Delete the server thread.  */
        tx_thread_delete(&(ftp_server_ptr -> nx_ftp_server_thread));

        /* Delete the server event flags.  */
        tx_event_flags_delete(&(ftp_server_ptr -> nx_ftp_server_event_flags));

        /* Error creating the server timer.  */
        return(status);
    }

    /* Loop to create all the FTP client control sockets.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Create an FTP client control socket.  */
        status +=  nx_tcp_socket_create(ip_ptr, &(ftp_server_ptr -> nx_ftp_server_client_list[i].nx_ftp_client_request_control_socket), 
                                        "FTP Server Control Socket", NX_FTP_CONTROL_TOS, NX_FTP_FRAGMENT_OPTION, 
                                        NX_FTP_TIME_TO_LIVE, NX_FTP_CONTROL_WINDOW_SIZE, NX_NULL, _nx_ftp_server_control_disconnect);

        /* If no error is present, register the receive notify function.  */
        if (status == NX_SUCCESS)
        {

            /* Register the receive function.  */
            nx_tcp_socket_receive_notify(&(ftp_server_ptr -> nx_ftp_server_client_list[i].nx_ftp_client_request_control_socket),
                                            _nx_ftp_server_command_present);
        }

        /* Make sure each socket points to the FTP server.  */
        ftp_server_ptr -> nx_ftp_server_client_list[i].nx_ftp_client_request_control_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;
    }

    /* Determine if an error has occurred.  */
    if (status)
    {

        /* Loop to delete any created sockets.  */
        for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
        {

            /* Delete the FTP socket.  */
            nx_tcp_socket_delete(&(ftp_server_ptr -> nx_ftp_server_client_list[i].nx_ftp_client_request_control_socket));
        }

        /* Delete the server thread.  */
        tx_thread_delete(&(ftp_server_ptr -> nx_ftp_server_thread));

        /* Delete the event flag group.  */
        tx_event_flags_delete(&(ftp_server_ptr -> nx_ftp_server_event_flags));

        /* Delete the timer.  */
        tx_timer_delete(&(ftp_server_ptr -> nx_ftp_server_timer));

        /* Return an error.  */
        return(status);
    }

    /* Initialize the data port.  */
    ftp_server_ptr -> nx_ftp_server_data_port = NX_SEARCH_PORT_START;

    /* Save the Server name.  */
    ftp_server_ptr -> nx_ftp_server_name =  ftp_server_name;

    /* Save the IP pointer address.  */
    ftp_server_ptr -> nx_ftp_server_ip_ptr =  ip_ptr;

    /* Set the FTP media pointer address.  */
    ftp_server_ptr -> nx_ftp_server_media_ptr =  media_ptr;
                      
    /* Save the user-supplied login and logout functions.  */
    ftp_server_ptr -> nx_ftp_login = ftp_login;
    ftp_server_ptr -> nx_ftp_logout = ftp_logout; 

    /* Set the server ID to indicate the FTP server thread is ready.  */
    ftp_server_ptr -> nx_ftp_server_id = NXD_FTP_SERVER_ID;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}
                 

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ftp_server_delete                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the FTP server delete call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_delete                 Actual server delete call     */
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
UINT  _nxe_ftp_server_delete(NX_FTP_SERVER *ftp_server_ptr)
{

UINT    status;

                                
    /* Check for invalid input pointers.  */
    if ((ftp_server_ptr == NX_NULL) || (ftp_server_ptr -> nx_ftp_server_id != NXD_FTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_ftp_server_delete(ftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_delete                               PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created FTP server on the        */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_close                         Close file                    */
/*    nx_tcp_server_socket_unaccept         Unaccept server socket        */
/*    nx_tcp_server_socket_unlisten         Unlisten on server            */
/*    nx_tcp_socket_delete                  Delete socket                 */
/*    nx_tcp_socket_disconnect              Disconnect socket             */
/*    tx_event_flags_delete                 Delete event flags            */
/*    tx_thread_delete                      Delete thread                 */
/*    tx_thread_suspend                     Suspend thread                */
/*    tx_thread_terminate                   Terminate thread              */
/*    tx_timer_deactivate                   Deactivate timer              */
/*    tx_timer_delete                       Delete timer                  */
/*    _nx_ftp_server_data_socket_cleanup    Clean up data socket          */
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of clearing */
/*                                            data socket,                */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ftp_server_delete(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                        i;
NX_FTP_CLIENT_REQUEST      *client_request_ptr;


    /* Clear the server ID to indicate the FTP server is no longer ready.  */
    ftp_server_ptr -> nx_ftp_server_id =  0;

    /* Suspend the FTP server thread.  */
    tx_thread_suspend(&(ftp_server_ptr -> nx_ftp_server_thread));

    /* Terminate server thread. */
    tx_thread_terminate(&(ftp_server_ptr -> nx_ftp_server_thread));

    /* Delete server thread.  */
    tx_thread_delete(&(ftp_server_ptr -> nx_ftp_server_thread));

    /* Delete the event flag group.  */
    tx_event_flags_delete(&(ftp_server_ptr -> nx_ftp_server_event_flags));

    /* Deactivate and delete timer.  */
    tx_timer_deactivate(&(ftp_server_ptr -> nx_ftp_server_timer));
    tx_timer_delete(&(ftp_server_ptr -> nx_ftp_server_timer));

    /* Walk through the server structure to close any remaining open files.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i ++)
    {

        /* Set the client request.  */
        client_request_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* If created, cleanup the data socket.  */
        if (client_request_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
        {

            /* Clean up the data socket.  */
            _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_request_ptr);
        }

        /* Reset the transfer mode as stream mode.  */
        client_request_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

        /* Reset the block bytes.  */
        client_request_ptr -> nx_ftp_client_request_block_bytes = 0;

        /* Disconnect the control and data ports.  */
        nx_tcp_socket_disconnect(&(client_request_ptr -> nx_ftp_client_request_control_socket), NX_NO_WAIT);

        /* Unaccept the control socket.  */
        nx_tcp_server_socket_unaccept(&(client_request_ptr -> nx_ftp_client_request_control_socket));

        /* Delete both the control and data sockets.  */
        nx_tcp_socket_delete(&(client_request_ptr -> nx_ftp_client_request_control_socket));
    }

    /* Unlisten on the FTP control port.  */
    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ftp_server_start                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the FTP server start call.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_start                  Actual server start call      */
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
UINT  _nxe_ftp_server_start(NX_FTP_SERVER *ftp_server_ptr)
{

UINT    status;

                   
    /* Check for invalid input pointers.  */
    if ((ftp_server_ptr == NX_NULL) || (ftp_server_ptr -> nx_ftp_server_id != NXD_FTP_SERVER_ID))
        return(NX_PTR_ERROR);  

    /* Call actual server start function.  */
    status =  _nx_ftp_server_start(ftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_start                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a previously created FTP server on the         */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_server_socket_listen           Listen of FTP clients         */
/*    tx_thread_resume                      Resume the FTP server thread  */
/*    tx_timer_activate                     Activate FTP server timer     */
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
UINT  _nx_ftp_server_start(NX_FTP_SERVER *ftp_server_ptr)
{

UINT    status;
ULONG   events;


    /* Start listening on the FTP control socket.  */
    status =  nx_tcp_server_socket_listen(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT,
                        &(ftp_server_ptr -> nx_ftp_server_client_list[0].nx_ftp_client_request_control_socket),
                                    NX_FTP_MAX_CLIENTS, _nx_ftp_server_connection_present);

    /* Determine if an error is present.  */
    if (status)
    {

        /* Error, return to caller.  */
        return(status);
    }

    /* Activate FTP server timer.  */
    tx_timer_activate(&(ftp_server_ptr -> nx_ftp_server_timer));

    /* Clear stop event. */
    tx_event_flags_get(&(ftp_server_ptr -> nx_ftp_server_event_flags), NX_FTP_STOP_EVENT, TX_OR_CLEAR, &events, TX_NO_WAIT);

    /* Start the FTP server thread.  */
    tx_thread_resume(&(ftp_server_ptr -> nx_ftp_server_thread));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ftp_server_stop                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the FTP server stop call.        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_stop                   Actual server start call      */
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
UINT  _nxe_ftp_server_stop(NX_FTP_SERVER *ftp_server_ptr)
{

UINT    status;

                
    /* Check for invalid input pointers.  */
    if ((ftp_server_ptr == NX_NULL) || (ftp_server_ptr -> nx_ftp_server_id != NXD_FTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_ftp_server_stop(ftp_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_stop                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function stops a previously started FTP server on the          */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_suspend                     Suspend the FTP server thread */
/*    tx_timer_deactivate                   Deactivate FTP server timer   */
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
UINT  _nx_ftp_server_stop(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                        i;
NX_FTP_CLIENT_REQUEST      *client_request_ptr;

    /* Deactivate FTP server timer.  */
    tx_timer_deactivate(&(ftp_server_ptr -> nx_ftp_server_timer));

    /* Suspend the FTP server thread.  */
    tx_event_flags_set(&(ftp_server_ptr -> nx_ftp_server_event_flags), NX_FTP_STOP_EVENT, TX_OR);

    /* Walk through the server structure to close any remaining open files.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i ++)
    {

        /* Set the client request.  */
        client_request_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* If created, cleanup the data socket.  */
        if (client_request_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
        {

            /* Disconnect data socket.  */
            nx_tcp_socket_disconnect(&(client_request_ptr -> nx_ftp_client_request_data_socket), NX_NO_WAIT);

            /* Unbind/unaccept the data socket.  */
            if (client_request_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_TRUE)
            {
                nx_tcp_server_socket_unaccept(&(client_request_ptr -> nx_ftp_client_request_data_socket));
                nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, client_request_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_port);
            }
            else
            {
                nx_tcp_client_socket_unbind(&(client_request_ptr -> nx_ftp_client_request_data_socket));
            }

            /* Delete data socket.  */
            nx_tcp_socket_delete(&(client_request_ptr -> nx_ftp_client_request_data_socket));

            /* Close file.  */
            fx_file_close(&(client_request_ptr -> nx_ftp_client_request_file));
        }

        /* Clear the passive transfer enabled flag.  */
        client_request_ptr -> nx_ftp_client_request_passive_transfer_enabled = NX_FALSE;

        /* Reset the transfer mode as stream mode.  */
        client_request_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

        /* Reset the block bytes.  */
        client_request_ptr -> nx_ftp_client_request_block_bytes = 0;

        /* Disconnect the control and data ports.  */
        nx_tcp_socket_disconnect(&(client_request_ptr -> nx_ftp_client_request_control_socket), NX_NO_WAIT);

        /* Unaccept the control socket.  */
        nx_tcp_server_socket_unaccept(&(client_request_ptr -> nx_ftp_client_request_control_socket));

        /* Delete both the control and data sockets.  */
        nx_tcp_socket_delete(&(client_request_ptr -> nx_ftp_client_request_control_socket));
    }

    /* Unlisten on the FTP control port.  */
    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_response                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds an FTP Server response.                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket                                FTP socket                    */
/*    packet_ptr                            Response packet pointer       */
/*    reply_code                            Reply code string             */
/*    message                               Optional message              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_release                     Packet release                */
/*    nx_tcp_socket_send                    Send TCP packet               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX FTP Routines                                                   */
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
VOID  _nx_ftp_server_response(NX_TCP_SOCKET *socket, NX_PACKET *packet_ptr, CHAR *reply_code, CHAR *message)
{

UCHAR   *buffer_ptr;
UINT    status;


    /* Set the packet prepend pointer.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;
    packet_ptr -> nx_packet_length =  0;

    /* Now setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Copy reply code to the packet.  */
    while ((*reply_code) && (buffer_ptr < (packet_ptr -> nx_packet_data_end - NX_PHYSICAL_TRAILER)))
    {

        /* copy the next character */
        *buffer_ptr++ = (UCHAR) *reply_code++;

        /* Update the packet length.  */
        packet_ptr -> nx_packet_length++;
    }

    /* Add a separating space */
    *buffer_ptr++ =  (UCHAR) ' ';
    packet_ptr -> nx_packet_length++;

    /* Copy message to the packet */
    while ((*message) && (buffer_ptr < (packet_ptr -> nx_packet_data_end - NX_PHYSICAL_TRAILER)))
    {

        /* copy the next character */
        *buffer_ptr++ =  (UCHAR) *message++;

        /* Update the packet length.  */
        packet_ptr -> nx_packet_length++;
    }

    /* Add a trailing space, CR, LF.  */
    *buffer_ptr++ =  ' ';
    *buffer_ptr++ =  13;
    *buffer_ptr   =  10;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length +=  3;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the failed message back.  */
    status =  nx_tcp_socket_send(socket, packet_ptr, NX_FTP_SERVER_TIMEOUT);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_directory_response                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds an FTP Server directory response.              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket                                FTP socket                    */
/*    packet_ptr                            Response packet pointer       */
/*    reply_code                            Reply code string             */
/*    message                               Optional message              */
/*    directory                             Directory path                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_release                     Packet release                */
/*    nx_tcp_socket_send                    Send TCP packet               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX FTP Routines                                                   */
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
VOID  _nx_ftp_server_directory_response(NX_TCP_SOCKET *socket, NX_PACKET *packet_ptr, CHAR *reply_code, CHAR *message, CHAR *directory)
{

UCHAR   *buffer_ptr;
UINT    status;


    /* Set the packet prepend pointer.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;
    packet_ptr -> nx_packet_length = 0;

    /* Now setup pointer to the buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Copy reply code to the packet.  */
    while ((*reply_code) && (buffer_ptr < (packet_ptr -> nx_packet_data_end - NX_PHYSICAL_TRAILER)))
    {

        /* copy the next character. */
        *buffer_ptr++ = (UCHAR) *reply_code++;

        /* Update the packet length.  */
        packet_ptr -> nx_packet_length++;
    }

    /* Add a separating space and the leading ".  */
    *buffer_ptr++ =  (UCHAR) ' ';
    packet_ptr -> nx_packet_length++;

    *buffer_ptr++ =  (UCHAR) '"';
    packet_ptr -> nx_packet_length++;

    /* Is there a valid directory.  */
    if ((directory != NULL) && (*directory))
    {

        /* If the directory doesn't start with a / then make sure it does.  */
        if ((*directory != '/') && (*directory != '\\'))
        {

            /* Add / to message.  */
            *buffer_ptr++ =  (UCHAR) '/';
            packet_ptr -> nx_packet_length++;
        }

        /* Copy directory to the packet.  */
        while ((*directory) && (buffer_ptr < (packet_ptr -> nx_packet_data_end - NX_PHYSICAL_TRAILER)))
        {

            /* Copy the next character , doubling any " characters.  */
            if (*directory == '"')
            {

                /* Double the character.  */
                *buffer_ptr++ =  (UCHAR) *directory;
                packet_ptr -> nx_packet_length++;
            }

            /* Copy the directory character, converting backslashes to slashes.  */
            if (*directory == '\\')
            {

                /* Convert to slash for consistency.  */
                *buffer_ptr++ =  (UCHAR) '/';
                directory++;
            }
            else
            {

                /* Copy regular directory character.  */
                *buffer_ptr++ =  (UCHAR) *directory++;
            }

            /* Update the packet length.  */
            packet_ptr -> nx_packet_length++;
        }
    }
    else
    {

        /* No valid directory, simply assume root.  */
        *buffer_ptr++ =  (UCHAR) '/';
        packet_ptr -> nx_packet_length++;
    }

    /* Add the trailing " and a separating space.  */
    *buffer_ptr++ =  (UCHAR) '"';
    packet_ptr -> nx_packet_length++;

    *buffer_ptr++ =  (UCHAR) ' ';
    packet_ptr -> nx_packet_length++;

    /* Copy message to the packet.  */
    while ((*message) && (buffer_ptr < (packet_ptr -> nx_packet_data_end - NX_PHYSICAL_TRAILER)))
    {

        /* Copy the next character.  */
        *buffer_ptr++ =  (UCHAR) *message++;

        /* Update the packet length.  */
        packet_ptr -> nx_packet_length++;
    }

    /* Add a trailing space, CR, LF.  */
    *buffer_ptr++ =  (UCHAR) ' ';
    *buffer_ptr++ =  (UCHAR) 13;
    *buffer_ptr   =  (UCHAR) 10;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length +=  3;

    /* Setup the packet append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Send the failed message back.  */
    status =  nx_tcp_socket_send(socket, packet_ptr, NX_FTP_SERVER_TIMEOUT);

    /* Determine if the send was unsuccessful.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_thread_entry                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the entry of the FTP server.  All basic            */
/*    processing is initiated by this function.                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_address                    Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_ftp_server_command_process         Process client FTP command    */
/*   _nx_ftp_server_connect_process         Process connection requests   */
/*   _nx_ftp_server_control_disconnect_processing                         */
/*                                          Process control disconnect    */
/*   _nx_ftp_server_data_disconnect_process Process data disconnection    */
/*                                            requests                    */
/*   _nx_ftp_server_data_process            Process client write data     */
/*   _nx_ftp_server_timeout_processing      Process activity timeout      */
/*   tx_event_flags_get                     Get FTP event(s)              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX                                                             */
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
VOID  _nx_ftp_server_thread_entry(ULONG ftp_server_address)
{

NX_FTP_SERVER          *server_ptr;
UINT                    status;
ULONG                   events;


    /* Setup the server pointer.  */
    server_ptr =  (NX_FTP_SERVER *) ftp_server_address;

    /* Loop to process FTP Server requests.  */
    while(1)
    {

        /* Wait for an FTP client activity.  */
        status =  tx_event_flags_get(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_ANY_EVENT, 
                                     TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Check the return status.  */
        if (status)
        {

            /* If an error occurs, simply continue the loop.  */
            continue;
        }

        /* Check whether service is started. */
        if (events & NX_FTP_STOP_EVENT)
        {

            /* Suspend thread here. */
            tx_thread_suspend(&server_ptr -> nx_ftp_server_thread);
            continue;
        }

        /* Otherwise, an event is present.  Process according to the event.  */

        /* Check for a client connection event.  */
        if (events & NX_FTP_SERVER_CONNECT)
        {

            /* Call the connect processing.  */
            _nx_ftp_server_connect_process(server_ptr);
        }

        /* Check for an FTP client write data event.  */
        if  (events & NX_FTP_SERVER_DATA)
        {

            /* Call processing to handle client file write data.  */
            _nx_ftp_server_data_process(server_ptr);
        }

        /* Check for a FTP client command event.  */
        if  (events & NX_FTP_SERVER_COMMAND)
        {

            /* Call the command processing.  */
            _nx_ftp_server_command_process(server_ptr);
        }

        /* Check for a client disconnect event.  */
        if  (events & NX_FTP_SERVER_DATA_DISCONNECT)
        {

            /* Call the data disconnect processing.  */
            _nx_ftp_server_data_disconnect_process(server_ptr);
        }

        /* Check for a control disconnect event.  */
        if  (events & NX_FTP_SERVER_CONTROL_DISCONNECT)
        {

            /* Call the control disconnect processing.  */
            _nx_ftp_server_control_disconnect_processing(server_ptr);
        }


        /* Check for a client activity timeout event.  */
        if  (events & NX_FTP_SERVER_ACTIVITY_TIMEOUT)
        {

            /* Call the activity timeout processing.  */
            _nx_ftp_server_timeout_processing(server_ptr);
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_command_process                      PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles all FTP client commands received on all       */
/*    client connections.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_parse_command          Parse FTP client command      */
/*    _nx_ftp_server_response               Send FTP response             */
/*    _nx_ftp_server_directory_response     Send FTP response for dir     */
/*    _nx_ftp_server_number_to_ascii        Converts number to ascii text */ 
/*    fx_file_attributes_read               Read file attributes          */
/*    fx_file_close                         Close file                    */
/*    fx_file_delete                        Delete file                   */
/*    fx_file_open                          Open file                     */
/*    fx_file_read                          Read from file                */
/*    fx_file_rename                        Rename file                   */
/*    fx_file_truncate                      Truncate file                 */
/*    fx_directory_attributes_read          Read directory attributes     */
/*    fx_directory_create                   Create directory              */
/*    fx_directory_delete                   Delete directory              */
/*    fx_directory_first_entry_find         Find first directory entry    */
/*    fx_directory_local_path_restore       Restore directory path        */
/*    fx_directory_local_path_set           Set directory path            */
/*    fx_directory_next_entry_find          Find next directory entry     */
/*    fx_media_flush                        Flush cached media sectors    */
/*    nx_ftp_packet_allocate                Allocate a packet             */
/*    nx_packet_release                     Release a packet              */
/*    nx_tcp_client_socket_bind             Bind client socket            */
/*    nx_tcp_client_socket_connect          Connect client socket         */
/*    nxd_tcp_client_socket_connect         Connect client socket in IPv6 */
/*    nx_tcp_client_socket_unbind           Unbind client socket          */
/*    nx_tcp_server_socket_relisten         Relisten on server socket     */
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */
/*    nx_tcp_socket_create                  Create data socket            */
/*    nx_tcp_socket_delete                  Delete data socket            */
/*    nx_tcp_socket_disconnect              Disconnect socket             */
/*    nx_tcp_socket_receive                 Receive from command socket   */
/*    nx_tcp_socket_send                    Send packet                   */
/*    nx_tcp_socket_receive_notify          Register notification routine */
/*    nx_tcp_socket_transmit_configure      Configure data transer socket */
/*    _nx_utility_uint_to_string            Convert number to string      */
/*    _nx_ftp_server_data_socket_cleanup    Clean up data socket          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.3  */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            corrected the pad character,*/
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of clearing */
/*                                            data socket and processing  */
/*                                            disconnection event,        */
/*                                            improved the PASV response, */
/*                                            fixed the bug of processing */
/*                                            STOR in passive mode,       */
/*                                            reset the packet prepend    */
/*                                            pointer for alignment,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ftp_server_command_process(NX_FTP_SERVER *ftp_server_ptr)
{


#ifdef FEATURE_NX_IPV6  
NXD_ADDRESS             ipduo_address;
UINT                    port_size;
CHAR                    temp_buffer[10];
#endif
#ifndef NX_DISABLE_IPV4
ULONG                   connect_ip4_address = 0;
UINT                    commas;
ULONG                   ip_address;
UINT                    temp;
#endif /* NX_DISABLE_IPV4 */
UINT                    i, j;
INT                     k;
UINT                    port;
ULONG                   length;
ULONG                   remaining_length;
UINT                    status;
UINT                    ftp_command;
UINT                    single_file;
UCHAR                   *buffer_ptr;
NX_PACKET               *packet_ptr;
FX_LOCAL_PATH           temp_path;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;
UINT                    attributes;
ULONG                   size;
UINT                    year, month, day;
UINT                    hour, minute, second;
CHAR                    filename[FX_MAX_LONG_NAME_LEN];
const char              *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
UINT                    no_more_ftp_entries = NX_FALSE;
ULONG                   block_size;
 
    year = 1900;
    month = 1;
    day = 1;
    hour = 1;
    minute = 1;
    second = 1;
    attributes = 1;
    size = 1;

    /* Now look for a socket that has receive data.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

#ifndef NX_DISABLE_IPV4
        if(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            connect_ip4_address = client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4;
        }
#endif /* NX_DISABLE_IPV4 */

        /* Now see if this socket has data.  */
        if (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_receive_queue_count)
        {

            /* Reset the client request activity timeout.  */
            client_req_ptr -> nx_ftp_client_request_activity_timeout =  NX_FTP_ACTIVITY_TIMEOUT;

            /* Attempt to read a packet from this socket.  */
            status =  nx_tcp_socket_receive(&(client_req_ptr -> nx_ftp_client_request_control_socket), &packet_ptr, NX_NO_WAIT);

            /* Check for not data present.  */
            if (status != NX_SUCCESS)
            {

                /* Just continue the loop and look at the next socket.  */
                continue;
            }

#ifndef NX_DISABLE_PACKET_CHAIN
            if (packet_ptr -> nx_packet_next)
            {

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* And continue looking at other client requests.  */
                continue;
            }
#endif /* NX_DISABLE_PACKET_CHAIN */

            /* Now, parse the command in the packet.  Note that the parse command adjusts the packet pointer
               such that it is positioned at the next token in the buffer.  */
            ftp_command =  _nx_ftp_server_parse_command(packet_ptr);

            /* Check to make sure the client request is authenticated.  */
            if ((client_req_ptr -> nx_ftp_client_request_authenticated == NX_FALSE) &&
                (ftp_command != NX_FTP_USER) && (ftp_command != NX_FTP_PASS))
            {

                /* Unauthorized request.  */

                /* Increment the access error count.  */
                ftp_server_ptr -> nx_ftp_server_authentication_errors++;

                /* Now send an error response to the client.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_UNAUTHORIZED, "Not logged in");

                /* And continue looking at other client requests.  */
                continue;
            }

            /* Check to make sure the client has write access if requesting a write.  */
            else if ((client_req_ptr -> nx_ftp_client_request_read_only == NX_TRUE) &&
                ((ftp_command == NX_FTP_STOR) || (ftp_command == NX_FTP_RNFR) || (ftp_command == NX_FTP_RNTO) ||
                 (ftp_command == NX_FTP_DELE) || (ftp_command == NX_FTP_RMD ) || (ftp_command == NX_FTP_MKD )))
            {

                /* Unauthorized request.  */

                /* Increment the access error count.  */
                ftp_server_ptr -> nx_ftp_server_authentication_errors++;

                /* Now send an error response to the client.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_NO_ACCT, "Need account for storing files");

                /* And continue looking at other client requests.  */
                continue;
            }

            /* Switch on the command received.  */
            switch(ftp_command)
            {

            case NX_FTP_USER:
            {
            
                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* First, save the username in the request structure.  */
                for (j = 0; j < (NX_FTP_USERNAME_SIZE - 1) && (j < packet_ptr -> nx_packet_length); j++)
                {

                    /* Copy a character.  */
                    client_req_ptr -> nx_ftp_client_request_username[j] =  (CHAR) buffer_ptr[j];

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;
                }

                /* Ensure the username is NULL terminated.  */
                client_req_ptr -> nx_ftp_client_request_username[j] =  NX_NULL;

                /* Now send an intermediate response to the username.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_USER_OK, "Enter password");
                break;
            }

            case NX_FTP_PASS:
            {
            
                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* First, save the password in the request structure.  */
                for (j = 0; j < (NX_FTP_PASSWORD_SIZE - 1)  && (j < packet_ptr -> nx_packet_length); j++)
                {

                    /* Copy a character.  */
                    client_req_ptr -> nx_ftp_client_request_password[j] =  (CHAR) buffer_ptr[j];

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;
                }

                /* Ensure the password is NULL terminated.  */
                client_req_ptr -> nx_ftp_client_request_password[j] =  NX_NULL;

                /* Initially assume client will have read-write access.  */
                client_req_ptr -> nx_ftp_client_request_read_only =  NX_FALSE;

                /* Initialize the login status as unsuccessful. */
                status = NX_FTP_INVALID_LOGIN;

                /* Does this FTP server have an login handler?  */
                if (ftp_server_ptr -> nx_ftp_login)
                {

                    /* Now call the user's login callback routine to see if the username,password is valid.  */
                    status = (ftp_server_ptr -> nx_ftp_login)(ftp_server_ptr, &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                                        client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                                        client_req_ptr -> nx_ftp_client_request_username,
                                                        client_req_ptr -> nx_ftp_client_request_password,
                                                        &client_req_ptr -> nx_ftp_client_request_read_only);
                }
#ifndef NX_DISABLE_IPV4
                else
                {

                    /* No duo handler. Check if this is an IPv4 connection.  */
                    if (client_req_ptr -> nx_ftp_client_request_ip_type == NX_IP_VERSION_V4)
                    {

                        /* This is an IPv4 connection. */

                        /* Does this server have an IPv4 login function? */
                        if (ftp_server_ptr -> nx_ftp_login_ipv4)
                        {
    
                            /* Yes; Now call the user's login callback routine to see if the username,password is valid.  */
                            status = (ftp_server_ptr -> nx_ftp_login_ipv4)
                                            (ftp_server_ptr, 
                                            (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4),
                                             client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                             client_req_ptr -> nx_ftp_client_request_username,
                                             client_req_ptr -> nx_ftp_client_request_password,
                                             &client_req_ptr -> nx_ftp_client_request_read_only);
                        }
                    }
                }
#endif /* NX_DISABLE_IPV4 */

                /* Set the login as TRUE.  */
                client_req_ptr -> nx_ftp_client_request_login = NX_TRUE;

                if (status == NX_SUCCESS)
                {

                    /* Successful connection.  */

                    /* Mark as authenticated.  */
                    client_req_ptr -> nx_ftp_client_request_authenticated =  NX_TRUE;

                    /* Default transfer type is ASCII image. */
                    client_req_ptr -> nx_ftp_client_request_transfer_type = 'A';

                    /* Now build a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_LOGIN, "Logged in");
                }
                else
                {

                    /* Unsuccessful login.  */

                    /* Increment the number of login errors.  */
                    ftp_server_ptr -> nx_ftp_server_login_errors++;

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_UNAUTHORIZED, "Login Fail");
                }

                break;
            }

            case NX_FTP_QUIT:
            {
            
                /* Increment the number of disconnection requests.  */
                ftp_server_ptr -> nx_ftp_server_disconnection_requests++;

                /* Check if this client login.  */
                if (client_req_ptr -> nx_ftp_client_request_login)
                {

                    /* Call the logout function.  */

#ifndef NX_DISABLE_IPV4
                    /* Does this server have an IPv4 login function? */
                    if (ftp_server_ptr -> nx_ftp_logout_ipv4)
                    {

                        /* Call the logout which takes IPv4 address input. */
                        (ftp_server_ptr -> nx_ftp_logout_ipv4)(ftp_server_ptr, 
                                   client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                                   client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                   client_req_ptr -> nx_ftp_client_request_username,
                                   client_req_ptr -> nx_ftp_client_request_password, NX_NULL);
                    }
#endif /* NX_DISABLE_IPV4 */
                    if (ftp_server_ptr -> nx_ftp_logout)
                    {

                        /* Call the 'duo' logout function which takes IPv6 or IPv4 IP addresses. */
                        (ftp_server_ptr -> nx_ftp_logout)(ftp_server_ptr, &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                                          client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                                          client_req_ptr -> nx_ftp_client_request_username,
                                                          client_req_ptr -> nx_ftp_client_request_password, NX_NULL);
                    }

                    /* Set the login as FALSE.  */
                    client_req_ptr -> nx_ftp_client_request_login = NX_FALSE;
                }
                
                /* Clear authentication.  */
                client_req_ptr -> nx_ftp_client_request_authenticated =  NX_FALSE;

                /* Now send a successful response to the client.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_CLOSE, "Logging Off");

                /* If create, cleanup the associated data socket.  */
                if (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
                {

                    /* Clean up the client socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);
                }

                /* Now disconnect the command socket.  */
                nx_tcp_socket_disconnect(&(client_req_ptr -> nx_ftp_client_request_control_socket), NX_FTP_SERVER_TIMEOUT);

                /* Unaccept the server socket.  */
                nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_control_socket));

                /* Relisten on this socket. This will probably fail, but it is needed just in case all available
                   clients were in use at the time of the last relisten.  */
                nx_tcp_server_socket_relisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT,
                                                    &(client_req_ptr -> nx_ftp_client_request_control_socket));

                /* Check to see if a packet is queued up.  */
                if (client_req_ptr -> nx_ftp_client_request_packet)
                {

                    /* Yes, release it!  */
                    nx_packet_release(client_req_ptr -> nx_ftp_client_request_packet);
                }

                /* Disable the client request activity timeout.  */
                client_req_ptr -> nx_ftp_client_request_activity_timeout =  0;
                break;
            }

            case NX_FTP_RETR:
            {
            
                /* Check that the transfer type is a Binary Image.  */
                if (client_req_ptr -> nx_ftp_client_request_transfer_type != 'I')
                {

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Only Image transfer allowed");

                    /* We are done processing.  */
                    break;
                }

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "File Open Fail");
                    break;
                }

                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Attempt to open the file.  */
                status =  fx_file_open(ftp_server_ptr -> nx_ftp_server_media_ptr, 
                                       &(client_req_ptr -> nx_ftp_client_request_file), (CHAR *) buffer_ptr, 
                                       FX_OPEN_FOR_READ);

                /* Determine if the file open was successful.  */
                if (status == FX_SUCCESS)
                {

                    /* Check if passive transfer enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_TRUE)
                    {

                        /* Now wait for the data connection to connect.  */
                        status = nx_tcp_socket_state_wait(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_TCP_ESTABLISHED, NX_FTP_SERVER_TIMEOUT);

                        /* Check for connect error.  */
                        if (status)
                        {

                            /* Yes, a connect error is present. Tear everything down.  */
                            nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_port);
                            nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));
                        }
                    }
                    else
                    {

                        /* Create an FTP client data socket.  */
                        status =  nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, 
                                                       &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                                       NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, 
                                                       NX_NULL, NX_NULL);

                        /* If no error is present, register the receive notify function.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Make sure each socket points to the corresponding FTP server.  */
                            client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                            /* Bind the socket to the FTP server data port.  */
                            status =  nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                                NX_FTP_SERVER_DATA_PORT, NX_NO_WAIT);

                            /* Determine if the socket was bound.  */
                            if (status)
                            {

                                /* FTP server data port is busy, use any data port. */
                                nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_ANY_PORT, NX_NO_WAIT);
                            }

                            /* Now attempt to connect the data port to the client's data port.  */
                            status =  nxd_tcp_client_socket_connect(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                         &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                         client_req_ptr -> nx_ftp_client_request_data_port, NX_FTP_SERVER_TIMEOUT);

                            /* Check for connect error.  */
                            if (status)
                            {

                                /* Yes, a connect error is present.  Tear everything down.  */
                                nx_tcp_client_socket_unbind(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));
                            }
                            else
                            {

                                /* Setup the data port with a specific packet transmit retry logic.  */
                                nx_tcp_socket_transmit_configure(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                                    NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                                                    NX_FTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                                                    NX_FTP_SERVER_RETRY_MAX, 
                                                                    NX_FTP_SERVER_RETRY_SHIFT);
                            }
                        }
                    }
                }

                /* Now check and see if the open for read has any errors.  */
                if (status == NX_SUCCESS)
                {

                    /* The open for read command is successful!  */

                    /* Set the open for read type in the client request structure.  */
                    client_req_ptr -> nx_ftp_client_request_open_type =  NX_FTP_OPEN_FOR_READ;

                    /* Set the total bytes field to files size.  */
                    client_req_ptr -> nx_ftp_client_request_total_bytes =  (ULONG)client_req_ptr -> nx_ftp_client_request_file.fx_file_current_file_size;

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_START_XFER, "File Opened");

                    /* Determine if the block mode is enabled.  */
                    if ((client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK) &&
                        (client_req_ptr -> nx_ftp_client_request_total_bytes))
                    {

                        /* Send start block header for file size.  */
                        status = _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, 
                                                                  client_req_ptr -> nx_ftp_client_request_total_bytes);
                    }

                    /* Now read the file and send the contents to the client.  */
                    while (status == NX_SUCCESS)
                    {

                        /* Allocate a new packet.  */
                        _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                        /* Calculate the maximum read size.  */
                        length =  ((ULONG) (packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr)) - NX_PHYSICAL_TRAILER;

                        /* Determine if the length is greater than the connected MSS.  */
                        if (length > client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss)
                        {

                            /* Yes, reduce the length to match the MSS.  */
                            length =  client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss;
                        }

                        /* Read a buffer's worth of the file.  */
                        status =  fx_file_read(&(client_req_ptr -> nx_ftp_client_request_file), packet_ptr -> nx_packet_prepend_ptr, length, &length);

                        /* Determine if the file read was successful.  */
                        if (status == FX_SUCCESS)
                        {

                            /* Now send the packet on the data socket.  */

                            /* Set the packet length.  */
                            packet_ptr -> nx_packet_length =  length;

                            /* Setup the packet append pointer.  */
                            packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

                            /* Send the file data to the client.  */
                            status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                                                                        packet_ptr, NX_FTP_SERVER_TIMEOUT);

                            /* Determine if the send was unsuccessful.  */
                            if (status)
                            {
                                /* Release the packet.  */
                                nx_packet_release(packet_ptr);
                            }
                            else
                            {

                                /* Update the remaining bytes in the file.  */
                                client_req_ptr -> nx_ftp_client_request_total_bytes =  client_req_ptr -> nx_ftp_client_request_total_bytes - length;

                                /* Increment the number of bytes sent.  */
                                ftp_server_ptr -> nx_ftp_server_total_bytes_sent += length;
                            }
                        }
                        else
                        {

                            /* Release packet.  */
                            nx_packet_release(packet_ptr);
                        }
                    }

                    /* Determine if the block mode is enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
                    {

                        /* Send end block header for file size.  */
                        _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, 0);
                    }

                    /* Clean up the data socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);

                    /* Clear the open type in the client request structure.  */
                    client_req_ptr -> nx_ftp_client_request_open_type = 0;

                    /* Allocate a new packet.  */
                    _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Now determine if the read was a success.  */
                    if ((status == FX_END_OF_FILE) && (client_req_ptr -> nx_ftp_client_request_total_bytes == 0))
                    {

                        /* The read command was successful!  */
                        /* Now send a successful response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_COMPLETED, "File Sent");
                    }
                    else
                    {

                        /* Read command failed.  */
                        /* Now send an error response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_BAD_FILE, "Read Fail");
                    }
                }
                else
                {

                    /* Unsuccessful open for read or read command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "File Open Fail");
                }

                break;
            }

            case NX_FTP_STOR:
            {
            
                /* Check that the transfer type is a Binary Image.  */
                if (client_req_ptr -> nx_ftp_client_request_transfer_type != 'I')
                {

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Only Image transfer allowed");

                    /* And we are done processing.  */
                    break;
                }

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "File Open Failed");
                    break;
                }

                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Attempt to open the file.  */
                status =  fx_file_open(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_request_file), (CHAR *) buffer_ptr, FX_OPEN_FOR_WRITE);


                /* Determine if there was an error.  */
                if (status != FX_SUCCESS)
                {

                    /* Create a new file.  */
                    status = fx_file_create(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr);

                    if (status == FX_SUCCESS) 
                    {
                    
                        /* Open the new file.  */
                        status =  fx_file_open(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_request_file), (CHAR *) buffer_ptr, FX_OPEN_FOR_WRITE);
                    }
                }

                /* Truncate the file to a size of 0.  */
                status += fx_file_truncate(&(client_req_ptr -> nx_ftp_client_request_file), 0);

                /* Determine if the file create/open was successful.  */
                if (status ==  FX_SUCCESS)
                {

                    /* Check if passive transfer enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_FALSE)
                    {

                        /* Create an FTP client data socket.  */
                        status =  nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                        NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, NX_NULL, _nx_ftp_server_data_disconnect);

                        /* If no error is present, register the receive notify function.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Make sure each socket points to the corresponding FTP server.  */
                            client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                            /* Register the receive function.  */
                            nx_tcp_socket_receive_notify(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                                _nx_ftp_server_data_present);

                            /* Bind the socket to the FTP server data port.  */
                            status =  nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_FTP_SERVER_DATA_PORT, NX_NO_WAIT);

                            /* Determine if the socket was bound.  */
                            if (status)
                            {

                                /* FTP server data port is busy, use any data port. */
                                nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_ANY_PORT, NX_NO_WAIT);
                            }
                            /* Now attempt to connect the data port to the client's data port.  */
                            status =  nxd_tcp_client_socket_connect(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                         &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                         client_req_ptr -> nx_ftp_client_request_data_port, NX_FTP_SERVER_TIMEOUT);


                            /* Check for connect error.  */
                            if (status)
                            {
                                /* Yes, a connect error is present.  Tear everything down.  */
                                nx_tcp_client_socket_unbind(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));

#ifdef NX_FTP_FAULT_TOLERANT

                                /* Flush the media.  */
                                fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif
                            }
                            else
                            {
                                /* Setup the data port with a specific packet transmit retry logic.  */
                                nx_tcp_socket_transmit_configure(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                                    NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                                                    NX_FTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                                                    NX_FTP_SERVER_RETRY_MAX, 
                                                                    NX_FTP_SERVER_RETRY_SHIFT);
                            }
                        }
                    }
                }

                /* Now check and see if the open for write has any errors.  */
                if (status == NX_SUCCESS)
                {

                    /* The open for writing command is successful!  */
                    
                    /* Set the open for write type in the client request structure.  */
                    client_req_ptr -> nx_ftp_client_request_open_type =  NX_FTP_OPEN_FOR_WRITE;

                    /* Set the total bytes field to zero.  */
                    client_req_ptr -> nx_ftp_client_request_total_bytes =  0;

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_START_XFER, "File Open for Write");
                }
                else
                {
                    /* Unsuccessful open for writing command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "File Open Failed");
                }

                break;
            }

            case NX_FTP_RNFR:
            {

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Rename File not found");
                    break;
                }
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Read the file attributes to see if it is actually there.  */
                status =  fx_file_attributes_read(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr, &j);

                /* If not a file, read the directory attributes.  */
                if (status == FX_NOT_A_FILE)
                    status =   fx_directory_attributes_read(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr, &j);

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Successful start to the file rename.  */

                    /* Save the packet in the client request structure.  */
                    client_req_ptr -> nx_ftp_client_request_packet =  packet_ptr;

                    /* Allocate a new packet.  */
                    _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_FILE_PEND, "Rename File From");
                }
                else
                {

                    /* Unsuccessful first half of file rename.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Rename File not found");
                }
                break;
            }

            case NX_FTP_RNTO:
            {

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Rename failed");
                    break;
                }
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Rename the file.  */
                status =  fx_file_rename(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) (client_req_ptr -> nx_ftp_client_request_packet) -> nx_packet_prepend_ptr, (CHAR *) buffer_ptr);

                /* If not a file, rename the directory.  */
                if (status == FX_NOT_A_FILE)
                    status =   fx_directory_rename(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) (client_req_ptr -> nx_ftp_client_request_packet) -> nx_packet_prepend_ptr, (CHAR *) buffer_ptr);

#ifdef NX_FTP_FAULT_TOLERANT

                /* Flush the media.  */
                fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif

                /* Release the packet in the client request structure.  */
                nx_packet_release(client_req_ptr -> nx_ftp_client_request_packet);
                client_req_ptr -> nx_ftp_client_request_packet =  NX_NULL;

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Successful file rename.  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_COMPLETED, "File Renamed");
                }
                else
                {

                    /* Unsuccessful file rename.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Rename failed");
                }
                break;
            }

            case NX_FTP_DELE:
            {

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Delete Failed");
                    break;
                }
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Remove the specified file.  */
                status =  fx_file_delete(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr);

#ifdef NX_FTP_FAULT_TOLERANT

                /* Flush the media.  */
                fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Successful delete file.  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_COMPLETED, "File Deleted");
                }
                else
                {

                    /* Unsuccessful file delete.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Delete Failed");
                }
                break;
            }

            case NX_FTP_RMD:
            {

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Delete Directory Fail");
                    break;
                }
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Remove the specified directory.  */
                status =  fx_directory_delete(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr);

#ifdef NX_FTP_FAULT_TOLERANT

                /* Flush the media.  */
                fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Successful delete directory.  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_COMPLETED, "Directory Deleted");
                }
                else
                {

                    /* Unsuccessful directory delete.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Delete Directory Fail");
                }
                break;
            }

            case NX_FTP_MKD:
            {

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Directory Create failed");
                    break;
                }
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Ensure the name is NULL terminated.  */
                buffer_ptr[j] =  NX_NULL;

                /* Create the specified directory.  */
                status =  fx_directory_create(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr);

#ifdef NX_FTP_FAULT_TOLERANT

                /* Flush the media.  */
                fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                FX_LOCAL_PATH   temporary_path;


                    /* Successful create directory.  */

                    /* Change the path to the new directory, using a temporary directory structure */
                    status =  fx_directory_local_path_set(ftp_server_ptr -> nx_ftp_server_media_ptr, &temporary_path, (CHAR *) buffer_ptr);

                    /* Determine if it was successful.  */
                    if (status == NX_SUCCESS)
                    {

                    CHAR    *local_dir;


                        /* Successful change directory.  */

                        /* Get the actual path */
                        fx_directory_local_path_get(ftp_server_ptr -> nx_ftp_server_media_ptr, &local_dir);

                        /* Now send a successful response to the client.  */
                        _nx_ftp_server_directory_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_CMD_OK, "Directory Created", local_dir);
                    }
                    else
                    {
                        /* Unsuccessful directory change.  */

                        /* Now send an error response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_BAD_FILE, "Set New Directory Fail");
                    }

                    /* Restore the default directory of this connection.  */
                    fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));
                }
                else
                {

                    /* Unsuccessful directory create.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Directory Create failed");
                }
                break;
            }

            case NX_FTP_NLST:
            {
            
                /* Assume ASCII and relax restriction.  */
                if ((client_req_ptr -> nx_ftp_client_request_transfer_type != 'A') &&
                    (client_req_ptr -> nx_ftp_client_request_transfer_type != 'I'))
                {

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Only ASCII Listing allowed");

                    /* And we are done processing.  */
                    break;
                }

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "List bad Directory");
                    break;
                }

                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Determine if there is a directory path.  */
                if (j)
                {

                    /* Ensure the name is NULL terminated.  */
                    buffer_ptr[j] =  NX_NULL;

                    /* Set the path to the supplied directory.  */
                    status =  fx_directory_local_path_set(ftp_server_ptr -> nx_ftp_server_media_ptr, &temp_path, (CHAR *) buffer_ptr);
                }
                else
                {

                    /* Just set the buffer pointer to NULL since there isn't a string.  */
                    buffer_ptr =  NX_NULL;

                    /* Default status to success.  */
                    status =  FX_SUCCESS;
                }


                /* Determine if the path setup was successful.  */
                if (status ==  FX_SUCCESS)
                {

                    /* Check if passive transfer enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_TRUE)
                    {

                        /* Now wait for the data connection to connect.  */
                        status = nx_tcp_socket_state_wait(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_TCP_ESTABLISHED, NX_FTP_SERVER_TIMEOUT);

                        /* Check for connect error.  */
                        if (status)
                        {

                            /* Yes, a connect error is present. Tear everything down.  */
                            nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_port);
                            nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file)); 
                        }
                    }
                    else
                    {

                        /* Create an FTP client data socket.  */
                        status =  nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                        NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, NX_NULL, NX_NULL);

                        /* If no error is present, register the receive notify function.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Make sure each socket points to the corresponding FTP server.  */
                            client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                            /* Bind the socket to the FTP server data port.  */
                            status =  nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_FTP_SERVER_DATA_PORT, NX_NO_WAIT);

                            /* Determine if the socket was bound.  */
                            if (status)
                            {

                                /* FTP server data port is busy, use any data port. */
                                nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_ANY_PORT, NX_NO_WAIT);
                            }
                            /* Now attempt to connect the data port to the client's data port.  */
                            status =  nxd_tcp_client_socket_connect(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                         &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                         client_req_ptr -> nx_ftp_client_request_data_port, NX_FTP_SERVER_TIMEOUT);


                            /* Check for connect error.  */
                            if (status)
                            {

                                /* Yes, a connect error is present.  Tear everything down.  */
                                nx_tcp_client_socket_unbind(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));
                            }
                            else
                            {

                                /* Setup the data port with a specific packet transmit retry logic.  */
                                nx_tcp_socket_transmit_configure(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                                    NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                                                    NX_FTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                                                    NX_FTP_SERVER_RETRY_MAX, 
                                                                    NX_FTP_SERVER_RETRY_SHIFT);
                            }
                        }
                    }
                }

                /* Now check and see if the directory listing command has any errors.  */
                if (status == NX_SUCCESS)
                {

                    /* The directory listing is successful!  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_START_XFER, "Sending List");

                    /* Determine if the block mode is enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
                    {

                        /* Get the directory listing size.  */
                        _nx_ftp_server_block_size_get(ftp_server_ptr, ftp_command, filename, &block_size);

                        /* Send start block header for file size.  */
                        if (block_size)
                            _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, block_size);
                    }

                    /* Allocate a new packet.  */
                    status =  _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Calculate the remaining length.  */
                    remaining_length =  (ULONG)((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr) - NX_PHYSICAL_TRAILER);

                    /* Determine if the advertised MSS is even less.  */
                    if (remaining_length > client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss)
                    {

                        /* Reduce the remaining length to the MSS value.  */
                        remaining_length =  client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss;
                    }

                    /* Now generate the full directory listing and send the contents to the client.  */
                    j =  0;
                    while (status == NX_SUCCESS)
                    {

                        /* Pickup the next directory entry.  */
                        if (j == 0)
                        {


                            /* First directory entry.  */
                            status =  fx_directory_first_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                                            &attributes, &size, &year, &month, &day, &hour, &minute, &second);

                        }
                        else
                        {

                            /* Not the first entry - pickup the next!  */
                            status =  fx_directory_next_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                                            &attributes, &size, &year, &month, &day, &hour, &minute, &second);

                        }

                        /* Increment the entry count.  */
                        j++;

                        /* Determine if successful.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Setup pointer to buffer.  */
                            buffer_ptr =  packet_ptr -> nx_packet_append_ptr;

                            /* Calculate the size of the name.  */
                            length = 0;
                            do
                            {
                                if (filename[length])
                                    length++;
                                else
                                    break;
                            } while (length < FX_MAX_LONG_NAME_LEN);

                            /* Make sure there is enough space for the file name.  */
                            if ((length + 2) > remaining_length)
                            {

                                /* Send the current buffer out.  */

                                /* Send the directory data to the client.  */
                                status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                                                                        packet_ptr, NX_FTP_SERVER_TIMEOUT);

                                /* Determine if the send was unsuccessful.  */
                                if (status)
                                {
    
                                    /* Release the packet.  */
                                    nx_packet_release(packet_ptr);
                                }

                                /* Allocate a new packet.  */
                                status =  _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                                /* Determine if the packet allocate was successfull.  */
                                if (status)
                                {

                                    /* Get out of the loop!  */
                                    break;
                                }

                                /* Calculate the remaining length.  */
                                remaining_length =  (ULONG)((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr) - NX_PHYSICAL_TRAILER);

                                /* Determine if the advertised MSS is even less.  */
                                if (remaining_length > client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss)
                                {

                                    /* Reduce the remaining length to the MSS value.  */
                                    remaining_length =  client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss;
                                }

                                /* Setup pointer to buffer.  */
                                buffer_ptr =  packet_ptr -> nx_packet_append_ptr;
                            }

                            /* Put the file name and cr/lf in the buffer*/
                            memcpy(buffer_ptr, filename, length); /* Use case of memcpy is verified. */
                            buffer_ptr[length++] = '\r';
                            buffer_ptr[length++] = '\n';

                            /* Set the packet length. */
                            packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + length;

                            /* Setup the packet append pointer.  */
                            packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr + length;

                            /* Adjust the remaining length.  */
                            remaining_length =  remaining_length - length;
                        }
                    }

                    /* Now determine if the directory listing was a success.  */
                    if ((status == FX_NO_MORE_ENTRIES) && (packet_ptr) && (packet_ptr -> nx_packet_length))
                    {

                        no_more_ftp_entries = NX_TRUE;

                        /* Send the directory data to the client.  */
                        status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                                                                        packet_ptr, NX_FTP_SERVER_TIMEOUT);

                        /* Determine if the send was unsuccessful.  */
                        if (status)
                        {

                            /* Release the packet.  */
                            nx_packet_release(packet_ptr);
                        }
                    }
                    else if (packet_ptr)
                    {

                        /* Release packet just in case!  */
                        nx_packet_release(packet_ptr);
                    }

                    /* Determine if the block mode is enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
                    {

                        /* Send end block header for file size.  */
                        _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, 0);
                    }

                    /* Clean up the data socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);

                    /* Allocate a new packet.  */
                    _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Now determine if the directory listing was a success (e..g runs until no more entries found).  */
                    if (no_more_ftp_entries)
                    {

                        /* The directory listing was successful!  */

                        /* Now send a successful response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_COMPLETED, "List End");
                    }
                    else
                    {

                        /* Directory listing command failed.  */

                        /* Now send an error response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_BAD_FILE, "List fail");
                    }
                }
                else
                {
                    /* Unsuccessful directory listing command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "List bad Directory");
                }
                break;
            }

            case NX_FTP_LIST:
            {
            
                /* Assume ASCII and relax restriction.  */
                if ((client_req_ptr -> nx_ftp_client_request_transfer_type != 'A') &&
                    (client_req_ptr -> nx_ftp_client_request_transfer_type != 'I'))
                {

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Only ASCII Listing allowed");

                    /* And we are done processing.  */
                    break;
                }

                /* Check packet length.  */
                if (packet_ptr -> nx_packet_length == 0)
                {

                    /* Empty message.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Bad Directory");
                    break;
                }

                /* Change to the default directory of this connection.  */
                status = fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Find the end of the message.  */
                j =  0;
                k =  -1;
                while (j < packet_ptr -> nx_packet_length - 1)
                {

                    /* Determine if a slash or backslash is present.  */
                    if ((buffer_ptr[j] == '/') || (buffer_ptr[j] == '\\'))
                        k =  (INT)j;

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                        break;

                    /* Move to next character.  */
                    j++;
                }

                /* If specified path ends with slash or backslash, strip it.  */
                if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                {
                    j--;
                }

                /* Default the single file specified flag to false.  */
                single_file =  NX_FALSE;

                /* Determine if there is a directory path.  */
                if (j)
                {

                    /* Ensure the name is NULL terminated.  */
                    buffer_ptr[j] =  NX_NULL;

                    /* Set the path to the supplied directory.  */
                    status =  fx_directory_local_path_set(ftp_server_ptr -> nx_ftp_server_media_ptr, &temp_path, (CHAR *) buffer_ptr);

                    /* Determine if the path setup was unsuccessful.  */
                    if (status)
                    {

                        /* Pickup the information for the single file.  */
                        status =  fx_directory_information_get(ftp_server_ptr -> nx_ftp_server_media_ptr, (CHAR *) buffer_ptr,
                                                &attributes, &size, &year, &month, &day, &hour, &minute, &second);

                        /* Determine if a file is specified as the LIST parameter.  */
                        if ((status == FX_SUCCESS) && ((attributes & FX_DIRECTORY) == 0))
                        {

                            /* Yes, a file name was supplied. Set the single file flag for the processing below.  */
                            single_file =  NX_TRUE;

                            /* Advance to first character of the filename.  */
                            k++;

                            /* Copy the file name from the last slash into the filename buffer.  */
                            j =  0;
                            while ((buffer_ptr[(UINT)k + j]) && (j < FX_MAX_LONG_NAME_LEN-1))
                            {

                                /* Copy a character of the filename.  */
                                filename[j] =  (CHAR)(buffer_ptr[(UINT)k + j]);

                                /* Move to next character.  */
                                j++;
                            }

                            /* Null terminate the string.  */
                            filename[j] =  NX_NULL;
                        }
                    }
                }
                else
                {

                    /* Just set the buffer pointer to NULL since there isn't a string.  */
                    buffer_ptr =  NX_NULL;
                }

                /* Determine if the path setup was successful.  */
                if (status ==  FX_SUCCESS)
                {

                CHAR    *local_dir;


                    /* Get the actual path */
                    fx_directory_local_path_get(ftp_server_ptr -> nx_ftp_server_media_ptr, &local_dir);

                    /* Check if passive transfer enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_TRUE)
                    {

                        /* Now wait for the data connection to connect.  */
                        status = nx_tcp_socket_state_wait(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_TCP_ESTABLISHED, NX_FTP_SERVER_TIMEOUT);

                        /* Check for connect error.  */
                        if (status)
                        {

                            /* Yes, a connect error is present. Tear everything down.  */
                            nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_port);
                            nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                            fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));
                        }
                    }
                    else
                    {

                        /* Create an FTP client data socket.  */
                        status =  nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                            NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, NX_NULL, NX_NULL);

                        /* If no error is present, register the receive notify function.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Make sure each socket points to the corresponding FTP server.  */
                            client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                            /* Bind the socket to the FTP server data port.  */
                            status =  nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_FTP_SERVER_DATA_PORT, NX_NO_WAIT);

                            /* Determine if the socket was bound.  */
                            if (status)
                            {

                                /* FTP server data port is busy, use any data port. */
                                nx_tcp_client_socket_bind(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_ANY_PORT, NX_NO_WAIT);
                            }

                            /* Now attempt to connect the data port to the client's data port.  */
                            status =  nxd_tcp_client_socket_connect(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                         &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                         client_req_ptr -> nx_ftp_client_request_data_port, NX_FTP_SERVER_TIMEOUT);


                            /* Check for connect error.  */
                            if (status)
                            {

                                /* Yes, a connect error is present.  Tear everything down.  */
                                nx_tcp_client_socket_unbind(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));
                                fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));
                            }
                            else
                            {

                                /* Setup the data port with a specific packet transmit retry logic.  */
                                nx_tcp_socket_transmit_configure(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                                    NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                                                    NX_FTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                                                    NX_FTP_SERVER_RETRY_MAX, 
                                                                    NX_FTP_SERVER_RETRY_SHIFT);
                            }
                        }
                    }
                }

                /* Now check and see if the directory listing command has any errors.  */
                if (status == NX_SUCCESS)
                {

                    /* The directory listing is successful!  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_START_XFER, "Sending List");

                    /* Determine if the block mode is enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
                    {

                        /* Get the directory listing size.  */
                        _nx_ftp_server_block_size_get(ftp_server_ptr, ftp_command, filename, &block_size);

                        /* Send start block header for file size.  */
                        if (block_size)
                            _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, block_size);
                    }

                    /* Allocate a new packet.  */
                    status =  _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Calculate the remaining length.  */
                    remaining_length =  (ULONG)((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr) - NX_PHYSICAL_TRAILER);

                    /* Determine if the advertised MSS is even less.  */
                    if (remaining_length > client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss)
                    {

                        /* Reduce the remaining length to the MSS value.  */
                        remaining_length =  client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss;
                    }

                    /* Now generate the full directory listing and send the contents to the client.  */
                    j =  0;
                    while (status == NX_SUCCESS)
                    {

                        /* Determine if a single file was specified.  */
                        if (single_file == NX_FALSE)
                        {

                            /* Typical case - not a single file.  */

                            /* Pickup the next directory entry.  */
                            if (j == 0)
                            {

                                /* First directory entry.  */
                                status =  fx_directory_first_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                                                &attributes, &size, &year, &month, &day, &hour, &minute, &second);

                            }
                            else
                            {

                                /* Not the first entry - pickup the next!  */
                                status =  fx_directory_next_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                                                &attributes, &size, &year, &month, &day, &hour, &minute, &second);

                            }
                        }
                        else
                        {

                            /* The parameter to the LIST command is a single file. Simply return the information
                               already gathered above for this one file instead of traversing the entire list.  */

                            /* Is this the first pass through the loop?  */
                            if (j)
                            {

                                /* End the loop, since the single file has already been sent.  */
                                status = FX_NO_MORE_ENTRIES;
                            }
                        }

                        /* Increment the entry count.  */
                        j++;

                        /* Determine if successful.  */
                        if (status == NX_SUCCESS)
                        {

                            /* Check if the month is valid before convert it.  */
                            if ((month < 1) || (month > 12))
                                continue;

                            /* Setup pointer to buffer.  */
                            buffer_ptr =  packet_ptr -> nx_packet_append_ptr;

                            /* Calculate the size of the name.  */
                            length = 0;
                            do
                            {
                                if (filename[length])
                                    length++;
                                else
                                    break;
                            } while (length < FX_MAX_LONG_NAME_LEN);

                            /* Make sure there is enough space for the data plus the file info.
                               File Info is 10 chars for permissions, 15 chars for owner and group,
                               11 chars for size (for file size up to 4gB), 14 for date, 2 chars for cr lf.  */
                            if ((length + 52) > remaining_length)
                            {

                                /* Send the current buffer out.  */

                                /* Send the directory data to the client.  */
                                status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_data_socket), packet_ptr, NX_FTP_SERVER_TIMEOUT);

                                /* Determine if the send was unsuccessful.  */
                                if (status)
                                {
    
                                    /* Release the packet.  */
                                    nx_packet_release(packet_ptr);
                                }

                                /* Allocate a new packet.  */
                                status =  _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                                /* Determine if the packet allocate was successfull.  */
                                if (status)
                                {

                                    /* Get out of the loop!  */
                                    break;
                                }

                                /* Calculate the remaining length.  */
                                remaining_length =  (ULONG)((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr) - NX_PHYSICAL_TRAILER);

                                /* Determine if the advertised MSS is even less.  */
                                if (remaining_length > client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss)
                                {

                                    /* Reduce the remaining length to the MSS value.  */
                                    remaining_length =  client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_connect_mss;
                                }

                                /* Setup pointer to buffer.  */
                                buffer_ptr =  packet_ptr -> nx_packet_append_ptr;
                            }

                            /* Put the file information followed by the file name */
                            buffer_ptr[0] = ((attributes & FX_DIRECTORY) ? 'd' : '-');
                            if (attributes & FX_READ_ONLY)
                            {
                                memcpy(&buffer_ptr[1], "r--r--r--", 9); /* Use case of memcpy is verified. */
                            }
                            else
                            {
                                memcpy(&buffer_ptr[1], "rw-rw-rw-", 9); /* Use case of memcpy is verified. */
                            }
                            memcpy(&buffer_ptr[10], "  1 owner group ", 16); /* Use case of memcpy is verified. */
                            _nx_ftp_server_number_to_ascii(&buffer_ptr[26], 10, size, ' ');
                            buffer_ptr[36] = ' ';
                            buffer_ptr[37] = (UCHAR)months[month - 1][0];
                            buffer_ptr[38] = (UCHAR)months[month - 1][1];
                            buffer_ptr[39] = (UCHAR)months[month - 1][2];
                            buffer_ptr[40] = ' ';
                            _nx_ftp_server_number_to_ascii(&buffer_ptr[41], 2, day, '0');
                            buffer_ptr[43] = ' ';
                            _nx_ftp_server_number_to_ascii(&buffer_ptr[44], 2, hour, '0');
                            buffer_ptr[46] = ':';
                            _nx_ftp_server_number_to_ascii(&buffer_ptr[47], 2, minute, '0');
                            buffer_ptr[49] = ' ';
                            memcpy(&buffer_ptr[50], filename, length); /* Use case of memcpy is verified. */
                            length += 50;
                            buffer_ptr[length++] = '\r';
                            buffer_ptr[length++] = '\n';
                            

                            /* Set the packet length. */
                            packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + length;

                            /* Setup the packet append pointer.  */
                            packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr + length;

                            /* Adjust the remaining length.  */
                            remaining_length =  remaining_length - length;
                        }
                    }

                    /* Now determine if the directory listing was a success.  */
                    if ((status == FX_NO_MORE_ENTRIES) && (packet_ptr) && (packet_ptr -> nx_packet_length))
                    {

                        /* Send the directory data to the client.  */
                        status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_data_socket),
                                                                                        packet_ptr, NX_FTP_SERVER_TIMEOUT);

                        /* Determine if the send was unsuccessful.  */
                        if (status)
                        {

                            /* Release the packet.  */
                            nx_packet_release(packet_ptr);
                        }
                        else
                        {

                            /* Reset the status for the response processing below.  */
                            status =  FX_NO_MORE_ENTRIES;
                        }
                    }
                    else if (packet_ptr)
                    {

                        /* Release packet just in case!  */
                        nx_packet_release(packet_ptr);
                    }

                    /* Determine if the block mode is enabled.  */
                    if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
                    {

                        /* Send end block header for file size.  */
                        _nx_ftp_server_block_header_send(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, 0);
                    }

                    /* Clean up the data socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);

                    /* Allocate a new packet.  */
                    _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                    /* Now determine if the directory listing was a success.  */
                    if (status == FX_NO_MORE_ENTRIES)
                    {

                        /* The directory listing was successful!  */

                        /* Now send a successful response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_COMPLETED, "List End");
                    }
                    else
                    {

                        /* Directory listing command failed.  */

                        /* Now send an error response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_BAD_FILE, "List fail");
                    }
                }
                else
                {
                    /* Unsuccessful directory listing command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Bad Directory");
                }
                break;
            }

#ifndef NX_DISABLE_IPV4
            case NX_FTP_PORT:
            {
            

                /* Check that only IPv4 packets can use the PORT command. */
                if (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
                {
                    /* Illegal PORT command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_NOT_IMPLEMENTED, "PORT illegal in IPv6");

                    /* Bail out! */
                    break;
                }
                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* First, pickup the IP address.  */
                commas =      0;
                ip_address =  0;
                j =           0;
                temp =        0;
                status =      NX_SUCCESS;
                while (j < packet_ptr -> nx_packet_length)
                {

                    /* Is a numeric character present?  */
                    if ((buffer_ptr[j] >= '0') && (buffer_ptr[j] <= '9'))
                    {

                        /* Yes, numeric character is present.  Update the IP address.  */
                        temp =  (temp*10) + (ULONG) (buffer_ptr[j] - '0');
                    }

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                    {
                        status =  NX_FTP_INVALID_COMMAND;
                        break;
                    }

                    /* Determine if a comma is present.  */
                    if (buffer_ptr[j] == ',')
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
                            j++;
                            break;
                        }

                    }

                    /* Move to next character.  */
                    j++;
                }

                /* Now pickup the port number.  */
                port =  0;
                temp =  0;
                while (j < packet_ptr -> nx_packet_length)
                {

                    /* Is a numeric character present?  */
                    if ((buffer_ptr[j] >= '0') && (buffer_ptr[j] <= '9'))
                    {

                        /* Yes, numeric character is present.  Update the IP port.  */
                        temp =  (temp*10) + (UINT) (buffer_ptr[j] - '0');
                    }

                    /* Determine if a CR/LF is present.  */
                    if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                    {
                        /* Good condition on the port number!  */
                        break;
                    }

                    /* Determine if a comma is present.  */
                    if (buffer_ptr[j] == ',')
                    {

                        /* Increment the comma count. */
                        commas++;

                        /* Move port number up.  */
                        port =  (port << 8) & 0xFFFFFFFF;
                        port =  port | (temp & 0xFF);
                        temp =  0;

                        /* Have we finished with the IP address?  */
                        if (commas >= 6)
                        {

                            /* Error, get out of the loop.  */
                            status =  NX_FTP_INVALID_ADDRESS;
                            break;
                        }
                    }

                    /* Move to next character.  */
                    j++;
                }

                /* Move port number up.  */
                port =  (port << 8) & 0xFFFFFFFF;
                port =  port | (temp & 0xFF);
                temp =  0;

                /* Determine if an error occurred.  */
                if ((buffer_ptr[j] != 13) || (commas != 5) || (ip_address == 0) || (port == 0) ||
                    (ip_address != connect_ip4_address))
                {

                    /* Set the error status.  */
                    status =  NX_FTP_INVALID_COMMAND;
                }

                /* Save the data port.  */
                client_req_ptr -> nx_ftp_client_request_data_port =  port;

                /* Determine if the port command was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Yes, the port command is successful!  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "Port set");
                }
                else
                {

                    /* Unsuccessful port command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "Port Fail");
                }

                break;
            }

            case NX_FTP_PASV:
            { 

                /* If create, cleanup the data socket.  */
                if (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
                {

                    /* Clean up the data socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);
                }

                /* Try to find the data port. */
                status = nx_tcp_free_port_find(ftp_server_ptr -> nx_ftp_server_ip_ptr,
                                               ftp_server_ptr -> nx_ftp_server_data_port++, &port);

                /* Determine if the PASV command was successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful port find.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "PASV Fail"); 

                    /* And we are done processing.  */
                    break;
                }

                /* Create an FTP client data socket.  */
                status = nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                    NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, NX_NULL, _nx_ftp_server_data_disconnect);

                /* Determine if the listen is successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful data socket create.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "PASV Fail");

                    /* And we are done processing.  */
                    break;
                }

                /* Register the receive function.  */
                nx_tcp_socket_receive_notify(&(client_req_ptr -> nx_ftp_client_request_data_socket), _nx_ftp_server_data_present);

                /* Setup the data port with a specific packet transmit retry logic.  */
                nx_tcp_socket_transmit_configure(&(client_req_ptr -> nx_ftp_client_request_data_socket), 
                                                    NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                                    NX_FTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                                    NX_FTP_SERVER_RETRY_MAX, 
                                                    NX_FTP_SERVER_RETRY_SHIFT);

                /* Make sure each socket points to the corresponding FTP server.  */
                client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                /* Start listening on the data port.  */
                status = nx_tcp_server_socket_listen(ftp_server_ptr -> nx_ftp_server_ip_ptr, port, &(client_req_ptr -> nx_ftp_client_request_data_socket), 5, 0);

                /* Determine if the listen is successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful data socket listen.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "PASV Fail"); 

                    /* Delete data socket.  */
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* And we are done processing.  */
                    break;
                }

                /* Wait for the data connection to connect.  */
                nx_tcp_server_socket_accept(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_NO_WAIT);

                /* Pickup the IPv4 address of this IP instance.  */
                ip_address =  client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_interface -> nx_interface_ip_address;

                /* Reset the packet prepend pointer for alignment.  */
                packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Now build PASV response. "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)."  */
                
                /* Verify packet payload. The max size of this message is 54. */
                if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 54)
                {

                    /* Delete data socket.  */
                    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, port);
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                    break;
                }

                /* Build the string. "227 Entering Passive Mode "  */
                memcpy(buffer_ptr, "227 Entering Passive Mode ", 26); /* Use case of memcpy is verified. */

                /* Build the IP address and port.  */
                j = 26;
                buffer_ptr[j++] = '(';
                j += _nx_utility_uint_to_string((ip_address >> 24), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ',';
                j += _nx_utility_uint_to_string(((ip_address >> 16) & 0xFF), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ',';
                j += _nx_utility_uint_to_string(((ip_address >> 8) & 0xFF), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ',';
                j += _nx_utility_uint_to_string((ip_address & 0xFF), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ',';
                j += _nx_utility_uint_to_string((port >> 8), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ',';
                j += _nx_utility_uint_to_string((port & 0XFF), 10, (CHAR *)(buffer_ptr + j), 54 - j);
                buffer_ptr[j++] = ')';
                buffer_ptr[j++] = '.';

                /* Set the CR/LF.  */
                buffer_ptr[j++] = 13;
                buffer_ptr[j++] = 10;

                /* Set the packet length.  */
                packet_ptr -> nx_packet_length = j;

                /* Setup the packet append pointer.  */
                packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

                /* Send the PASV response message.  */
                status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr, NX_FTP_SERVER_TIMEOUT);

                /* Determine if the send was unsuccessful.  */
                if (status != NX_SUCCESS)
                {

                    /* Delete data socket.  */
                    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, port);
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                }

                /* Yes, passive transfer enabled.  */
                client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled = NX_TRUE;

                break;
            }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
            case NX_FTP_EPRT:
            {
            

                /* Check that only IPv6 packets can use the EPRT command. */
                if (client_req_ptr -> nx_ftp_client_request_ip_type == NX_IP_VERSION_V4)
                {
                    /* Illegal EPRT command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_NOT_IMPLEMENTED, "EPRT illegal in IPv4");

                    /* Bail out! */
                    break;
                }

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* First, pickup the IPv6 address.  */
                status = _nx_ftp_utility_parse_IPv6_address((CHAR *)buffer_ptr, packet_ptr -> nx_packet_length, &ipduo_address);

                if (status != NX_SUCCESS)
                {
                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "Bad IPv6 address");

                    return;
                }

                /* Now pickup the port number.  */
                status = _nx_ftp_utility_parse_port_number((CHAR *)buffer_ptr, packet_ptr -> nx_packet_length, &port);

                /* Save the data port.  */
                client_req_ptr -> nx_ftp_client_request_data_port =  port;

                /* Determine if the EPRT command was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Yes, the EPRT command is successful!  */

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "EPRT command set");
                }
                else  
                {

                    /* Unsuccessful EPRT command.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "EPRT command failed");
                }

                break;

            }
            
            case NX_FTP_EPSV:
            { 

                /* If create, cleanup the data socket.  */
                if (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
                {

                    /* Clean up the client socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);
                }

                /* Try to find the data port. */
                status = nx_tcp_free_port_find(ftp_server_ptr -> nx_ftp_server_ip_ptr,
                                               ftp_server_ptr -> nx_ftp_server_data_port++, &port);

                /* Determine if the EPSV command was successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful port find.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "EPSV Fail"); 

                    /* And we are done processing.  */
                    break;
                }

                /* Create an FTP client data socket.  */
                status = nx_tcp_socket_create(ftp_server_ptr -> nx_ftp_server_ip_ptr, &(client_req_ptr -> nx_ftp_client_request_data_socket), "FTP Server Data Socket",
                                    NX_FTP_DATA_TOS, NX_FTP_FRAGMENT_OPTION, NX_FTP_TIME_TO_LIVE, NX_FTP_DATA_WINDOW_SIZE, NX_NULL, _nx_ftp_server_data_disconnect);

                /* Determine if the listen is successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful data socket create.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "EPSV Fail");

                    /* And we are done processing.  */
                    break;
                }

                /* Make sure each socket points to the corresponding FTP server.  */
                client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_reserved_ptr =  ftp_server_ptr;

                /* Start listening on the data port.  */
                status = nx_tcp_server_socket_listen(ftp_server_ptr -> nx_ftp_server_ip_ptr, port, &(client_req_ptr -> nx_ftp_client_request_data_socket), 5, 0);

                /* Determine if the listen is successful.  */
                if (status != NX_SUCCESS)
                {

                    /* Unsuccessful data socket listen.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_FAIL, "EPSV Fail"); 

                    /* Delete data socket.  */
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* And we are done processing.  */
                    break;
                }

                /* Wait for the data connection to connect.  */
                nx_tcp_server_socket_accept(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_NO_WAIT);

                /* Reset the packet prepend pointer for alignment.  */
                packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Now build EPSV response. "229 Entering Extended Passive Mode (|||6446|)."  */

                /* Get port size.  */
                port_size = _nx_ftp_server_utility_fill_port_number(temp_buffer, port);
                
                /* Verify packet payload.  */
                if ((ULONG)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (43 + port_size))
                {

                    /* Delete data socket.  */
                    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, port);
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                    break;
                }

                /* Build the string. "229 Entering Extended Passive Mode "  */
                memcpy(buffer_ptr, "229 Entering Extended Passive Mode ", 35); /* Use case of memcpy is verified. */

                /* Build the IPv6 address and port.  */
                buffer_ptr[35] = '(';
                buffer_ptr[36] = '|';
                buffer_ptr[37] = '|';
                buffer_ptr[38] = '|';

                memcpy(&buffer_ptr[39], temp_buffer, port_size); /* Use case of memcpy is verified. */

                buffer_ptr[39 + port_size] = '|';
                buffer_ptr[40 + port_size] = ')';

                /* Set the CR/LF.  */
                buffer_ptr[41 + port_size] = 13;
                buffer_ptr[42 + port_size] = 10;

                /* Set the packet length.  */
                packet_ptr -> nx_packet_length = 43 + port_size;

                /* Setup the packet append pointer.  */
                packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

                /* Send the EPSV response message.  */
                status =  nx_tcp_socket_send(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr, NX_FTP_SERVER_TIMEOUT);

                /* Determine if the send was unsuccessful.  */
                if (status != NX_SUCCESS)
                {

                    /* Delete data socket.  */
                    nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, port);
                    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

                    /* Release the packet.  */
                    nx_packet_release(packet_ptr);
                }

                /* Yes, passive transfer enabled.  */
                client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled = NX_TRUE;

                break;
            }
#endif  /* FEATURE_NX_IPV6 */

            case NX_FTP_CDUP:
            case NX_FTP_CWD:
            {
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, 
                                                &(client_req_ptr -> nx_ftp_client_local_path));

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* If CDUP command, create the "up one directory" pathname string.  */
                if (ftp_command == NX_FTP_CDUP)
                {

                    /* Move the pointer to make sure there is enough memory to store the data.  */
                    buffer_ptr -= 3;
                    buffer_ptr[0] = '.';
                    buffer_ptr[1] = '.';
                    buffer_ptr[2] = NX_NULL;
                }

                /* Otherwise CWD command, parse the pathname string.  */
                else
                {

                    /* Check packet length.  */
                    if (packet_ptr -> nx_packet_length == 0)
                    {

                        /* Empty message.  */

                        /* Now send an error response to the client.  */
                        _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                    NX_FTP_CODE_BAD_FILE, "Change Dir Fail");
                        break;
                    }

                    /* Find the end of the message.  */
                    j =  0;
                    while (j < packet_ptr -> nx_packet_length - 1)
                    {

                        /* Determine if a CR/LF is present.  */
                        if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
                            break;

                        /* Move to next character.  */
                        j++;
                    }

                    /* If specified path ends with slash or backslash, strip it.  */
                    if ((j > 1) && ((buffer_ptr[j - 1] == '/') || (buffer_ptr[j - 1] == '\\')))
                    {
                        j--;
                    }

                    /* Ensure the name is NULL terminated.  */
                    buffer_ptr[j] =  NX_NULL;
                }

                /* Set the local path to the path specified.  */
                status =  fx_directory_local_path_set(ftp_server_ptr -> nx_ftp_server_media_ptr, 
                                                      &(client_req_ptr -> nx_ftp_client_local_path), (CHAR *) buffer_ptr);

                /* Determine if it was successful.  */
                if (status == NX_SUCCESS)
                {

                CHAR    *local_dir;


                    /* Successful change directory.  */

                    /* Get the actual path */
                    fx_directory_local_path_get(ftp_server_ptr -> nx_ftp_server_media_ptr, &local_dir);

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_directory_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_COMPLETED, "set successfully", local_dir);
                }
                else
                {

                    /* Unsuccessful directory change.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "Change Dir Fail");
                }
                break;
            }

            case NX_FTP_PWD:
            {
            
                /* Change to the default directory of this connection.  */
                fx_directory_local_path_restore(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path));

                {

                CHAR    *local_dir;

                    /* Pickup the current directory.  */
                    fx_directory_local_path_get(ftp_server_ptr -> nx_ftp_server_media_ptr, &local_dir);

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_directory_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_MADE_DIR, "is current Directory", local_dir);
                }

                break;
            }

            case NX_FTP_TYPE:
            {
            
                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Determine if a binary transfer is specified.  */
                if (buffer_ptr[0] == 'I')
                {

                    /* Yes, a binary image is specified and supported.  */
                    client_req_ptr -> nx_ftp_client_request_transfer_type = 'I';

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "Type Binary");
                }

                /* Not Binary - check if ASCII type is specified */
                else if (buffer_ptr[0] == 'A')
                {

                    /* Yes, a ASCII image is specified and supported.  */
                    client_req_ptr -> nx_ftp_client_request_transfer_type = 'A';

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "Type ASCII");
                }
                else
                {

                    /* Otherwise, a non-binary type is requested.  */

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Type Non ASCII or Binary");
                }
                break;
            }

            case NX_FTP_MODE:
            {

                /* Setup pointer to packet buffer area.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Determine if stream mode is specified.  */
                if (buffer_ptr[0] == 'S')
                {

                    /* Yes, stream mode is specified and supported.  */
                    client_req_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "Mode Stream");
                }

                /* Not stream - check if block mode is specified */
                else if (buffer_ptr[0] == 'B')
                {

                    /* Yes, stream mode is specified and supported.  */
                    client_req_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_BLOCK;

                    /* Now send a successful response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CMD_OK, "Mode Block");
                }
                else
                {

                    /* Now send an error response to the client.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_TYPE, "Mode Non Stream or Block");
                }
                break;
            }

            case NX_FTP_NOOP:

                /* Now send a successful response to the client.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_CMD_OK, "NOOP Success");
                break;

            default:

                /* Unimplemented Command.  */

                /* Increment the number of unknown commands.  */
                ftp_server_ptr -> nx_ftp_server_unknown_commands++;

                /* Now send an error response to the client.  */
                _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                            NX_FTP_CODE_NOT_IMPLEMENTED, "Not Implemented");
                break;
            }
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_connect_process                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles all FTP client connections received.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ftp_server_response               Build and send response       */
/*    fx_directory_local_path_set           Set local path                */
/*    nx_tcp_server_socket_accept           Accept connection on socket   */
/*    nx_tcp_server_socket_relisten         Relisten for connection       */
/*    nx_tcp_server_socket_unaccept         Unaccept connection           */
/*    nx_ftp_packet_allocate                Allocate a packet             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
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
VOID  _nx_ftp_server_connect_process(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                    i;
UINT                    status;
NX_PACKET               *packet_ptr;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;


    /* One of the control ports is in the process of connection.  */

    /* Search the connections to see which one.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Now see if this socket was the one that is in being connected.  */
        if ((client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_state > NX_TCP_CLOSED) &&
            (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_state < NX_TCP_ESTABLISHED) &&
            (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port))
        {

            /* Yes, we have found the socket being connected.  */

            /* Increment the number of connection requests.  */
            ftp_server_ptr -> nx_ftp_server_connection_requests++;

            /* Attempt to accept on this socket.  */
            status = nx_tcp_server_socket_accept(&(client_req_ptr -> nx_ftp_client_request_control_socket), NX_FTP_SERVER_TIMEOUT);

            /* Determine if it is successful.  */
            if (status)
            {

                /* Not successful, simply unaccept on this socket.  */
                nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_control_socket));
            }
            else
            {

                /* Set the request type.  */
                if (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
                {

                    client_req_ptr -> nx_ftp_client_request_ip_type = NX_IP_VERSION_V6;
                }
                else
                {
                    client_req_ptr -> nx_ftp_client_request_ip_type = NX_IP_VERSION_V4;
                }

                /* Reset the client request activity timeout.  */
                client_req_ptr -> nx_ftp_client_request_activity_timeout =  NX_FTP_ACTIVITY_TIMEOUT;

                /* Send a connection ACK back to the client, indicating a successful connection
                   has been established.  */

                /* Allocate a packet for sending the connection ACK.  */
                status =  _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_FTP_SERVER_TIMEOUT);

                /* Determine if the packet allocation was successful.  */
                if (status == NX_SUCCESS)
                {

                    /* Now send "220" ACK message to indicate connection is ready.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_CONNECTION_OK, "Connection Ready");

                    /* Set the local path to the root directory.  */
                    fx_directory_local_path_set(ftp_server_ptr -> nx_ftp_server_media_ptr, &(client_req_ptr -> nx_ftp_client_local_path), "\\");
                }
                else
                {

                    /* Increment allocation error count.  */
                    ftp_server_ptr -> nx_ftp_server_allocation_errors++;
                }
            }
        }
    }

    /* Now look for a socket that is closed to relisten on.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Now see if this socket is closed.  */
        if (client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
        {

            /* Relisten on this socket.  */
            status =  nx_tcp_server_socket_relisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT,
                                                    &(client_req_ptr -> nx_ftp_client_request_control_socket));
            /* Check for bad status.  */
            if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
            {

                /* Increment the error count and keep trying.  */
                ftp_server_ptr -> nx_ftp_server_relisten_errors++;
                continue;
            }

            /* Break out of loop.  */
            break;
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_command_present                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles all FTP client commands received on           */
/*    the control socket.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    control_socket_ptr                    Socket event occurred         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                  NetX receive packet callback  */
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
VOID  _nx_ftp_server_command_present(NX_TCP_SOCKET *control_socket_ptr)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  (NX_FTP_SERVER *)control_socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the command event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_COMMAND, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_connection_present                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles all FTP client connections received on        */
/*    the control socket.                                                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    control_socket_ptr                    Socket event occurred         */
/*    port                                  Port the connection occurred  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                  NetX connect callback         */
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
VOID  _nx_ftp_server_connection_present(NX_TCP_SOCKET *control_socket_ptr, UINT port)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  (NX_FTP_SERVER *)control_socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the connect event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_CONNECT, TX_OR);

    NX_PARAMETER_NOT_USED(port);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_data_disconnect                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles all FTP client disconnections received on     */
/*    the data socket.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    data_socket_ptr                       Socket event occurred         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                  NetX connect callback         */
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
VOID  _nx_ftp_server_data_disconnect(NX_TCP_SOCKET *data_socket_ptr)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr = (NX_FTP_SERVER *) data_socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the disconnect event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_DATA_DISCONNECT, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_data_disconnect_process              PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes all FTP client disconnections received on   */
/*    the data socket.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_close                         Close file                    */
/*    nx_ftp_packet_allocate                Allocate a packet             */
/*    _nx_ftp_server_data_socket_cleanup    Clean up data socket          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the issue of processing     */
/*                                            disconnection event,        */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ftp_server_data_disconnect_process(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                    i;
UINT                    block_status;
ULONG                   length;
NX_PACKET               *packet_ptr;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;


    /* Now look for a socket that has receive data.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Initialize block status as NX_TRUE.  */
        block_status = NX_TRUE;

        /* Now see if this socket has entered a disconnect state.  */
        while (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_state > NX_TCP_ESTABLISHED)
        {

            /* Yes, a disconnect is present, which signals an end of file of a client FTP write request.  */

            /* Cleanup this data socket.  */
            _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);

            /* Determine if the block mode is enabled.  */
            if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
            {

                /* Check if receive all block bytes.  */
                if (client_req_ptr -> nx_ftp_client_request_total_bytes != client_req_ptr -> nx_ftp_client_request_block_bytes)
                    block_status = NX_FALSE;

                /* Reset the transfer mode as stream mode.  */
                client_req_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

                /* Reset the block bytes.  */
                client_req_ptr -> nx_ftp_client_request_block_bytes = 0;
            }

            /* Reset the client request activity timeout.  */
            client_req_ptr -> nx_ftp_client_request_activity_timeout =  NX_FTP_ACTIVITY_TIMEOUT;

            /* Check if file is open.  */
            if (client_req_ptr -> nx_ftp_client_request_open_type == NX_FTP_OPEN_FOR_WRITE)
            {

                /* Pickup the file length.  */
                length =  (ULONG)client_req_ptr -> nx_ftp_client_request_file.fx_file_current_file_size;

                /* Allocate a packet for sending the file write response.  */
                _nx_ftp_packet_allocate(ftp_server_ptr -> nx_ftp_server_packet_pool_ptr, client_req_ptr, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

                /* Now determine if the operation was successful.  */
                if ((length == client_req_ptr -> nx_ftp_client_request_total_bytes) && (block_status == NX_TRUE))
                {

                    /* Successful client file write.  */

                    /* Now send "250" message to indicate successful file write.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_COMPLETED, "File Written");
                }
                else
                {

                    /* Unsuccessful client file write.  */

                    /* Now send "550" message to indicate unsuccessful file write.  */
                    _nx_ftp_server_response(&(client_req_ptr -> nx_ftp_client_request_control_socket), packet_ptr,
                                NX_FTP_CODE_BAD_FILE, "File Write Failed");
                }

                /* Clear the open type.  */
                client_req_ptr -> nx_ftp_client_request_open_type =  0;
            }
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_data_present                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function notifies the FTP server thread of data received       */
/*    from a client on a data socket.                                     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    data_socket_ptr                       Socket event occurred         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                  NetX connect callback         */
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
VOID  _nx_ftp_server_data_present(NX_TCP_SOCKET *data_socket_ptr)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  (NX_FTP_SERVER *)data_socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_DATA, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_data_process                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes all FTP client data packets received on     */
/*    the data socket.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_write                         Write client data to file     */
/*    nx_packet_release                     Release packet                */
/*    nx_tcp_socket_receive                 Receive from data socket      */
/*    _nx_ftp_server_block_header_retrieve  Retrieve the block header     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
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
VOID  _nx_ftp_server_data_process(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                    i;
UINT                    status;
ULONG                   length;
NX_PACKET               *packet_ptr;
NX_PACKET               *next_packet_ptr;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;


    /* Now look for a socket that has receive data.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Now see if this socket has data.  If so, process all of it now!  */
        while (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_receive_queue_count)
        {

            /* Reset the client request activity timeout.  */
            client_req_ptr -> nx_ftp_client_request_activity_timeout =  NX_FTP_ACTIVITY_TIMEOUT;

            /* Attempt to read a packet from this socket.  */
            status =  nx_tcp_socket_receive(&(client_req_ptr -> nx_ftp_client_request_data_socket), &packet_ptr, NX_NO_WAIT);

            /* Check for not data present.  */
            if (status != NX_SUCCESS)
            {

                /* Just continue the loop and look at the next socket.  */
                continue;
            }

            /* Check for open type.  */
            if (client_req_ptr -> nx_ftp_client_request_open_type != NX_FTP_OPEN_FOR_WRITE)
            {

                /* Just release the packet.  */
                nx_packet_release(packet_ptr);

                /* Continue the loop.  */
                continue;
            }

            /* Determine if the block mode is enabled.  */
            if (client_req_ptr -> nx_ftp_client_request_transfer_mode == NX_FTP_TRANSFER_MODE_BLOCK)
            {

                /* Retrieve the block header.  */
                status = _nx_ftp_server_block_header_retrieve(client_req_ptr, packet_ptr);

                /* Determine if an error occurred.  */
                if (status)
                {

                    /* Continue the loop.  */
                    continue;
                }
            }

            /* Update the total bytes for this file.  This will be checked on close to see if
               any error occurred.  */
            client_req_ptr -> nx_ftp_client_request_total_bytes +=  packet_ptr -> nx_packet_length;

            /* Write the packet to the file.  */
            length =            packet_ptr -> nx_packet_length;
            next_packet_ptr =   packet_ptr;
            do
            {

                /* Write to the already opened file.  */
                status = fx_file_write(&(client_req_ptr -> nx_ftp_client_request_file), next_packet_ptr -> nx_packet_prepend_ptr,
                                            (ULONG)((next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr)));

                /* If unsuccessful file write, ok to receive the rest of the socket
                   packets from the receive queue. */

                if (status == FX_SUCCESS) 
                {
                
                    /* Increment the number of bytes received.  */
                    ftp_server_ptr -> nx_ftp_server_total_bytes_received += 
                        (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);
                }

                /* If not successful, keep retrieving packets from the socket receive queue so we can free
                   up the packets. */

                /* Update the remaining length to write.  */
                length =  length - (ULONG) (next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);

#ifdef NX_DISABLE_PACKET_CHAIN
                next_packet_ptr =  NX_NULL;
#else
                /* Pickup next packet pointer.  */
                next_packet_ptr =  next_packet_ptr -> nx_packet_next;
#endif /* NX_DISABLE_PACKET_CHAIN */

            } while ((next_packet_ptr) && (length));

            /* Release the packet and continue the loop.  */
            nx_packet_release(packet_ptr);
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_parse_command                        PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines which FTP request is present, returns      */
/*    a code for it, and adjusts the packet pointer to the next token.    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to FTP packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FTP command code                                                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_command_process        FTP command process           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ftp_server_parse_command(NX_PACKET *packet_ptr)
{

UINT    i;
char    *buffer_ptr;


    /* Check packet length.  */
    if (packet_ptr -> nx_packet_length == 0)
    {

        /* Empty message, just return INVALID.  */
        return(NX_FTP_INVALID);
    }

    /* Setup pointer to command buffer area.  */
    buffer_ptr =  (char *) packet_ptr -> nx_packet_prepend_ptr;

    /* Search the packet to find the end of the token, converting the token to Uppercase.  */
    i =  0;
    while ((buffer_ptr[i] != ' ') && (buffer_ptr[i] != (char) 13) && (i < packet_ptr -> nx_packet_length - 1))
    {

        /* Determine if the character is lowercase.  */
        if ((buffer_ptr[i] >= 'a') && (buffer_ptr[i] <= 'z'))
        {

            /* Convert to uppercase.  */
            buffer_ptr[i] =   (char) (((UCHAR) buffer_ptr[i]) - ((UCHAR) 0x20));
        }

        /* Move to next character.  */
        i++;
    }

    /* Set the NULL termination for the command.  */
    buffer_ptr[i] =  NX_NULL;

    /* Move the packet pointer to the next token.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + (i + 1);

    /* Decrease the length of the packet.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - (i + 1);

    /* Is it the "USER" command?  */
    if (strcmp(buffer_ptr, "USER") == 0)
    {
        return(NX_FTP_USER);
    }

    /* Is the the "PASS" command?  */
    if (strcmp(buffer_ptr, "PASS") == 0)
    {
        return(NX_FTP_PASS);
    }

    /* Is it the "QUIT" command?  */
    if (strcmp(buffer_ptr, "QUIT") == 0)
    {
        return(NX_FTP_QUIT);
    }

    /* Is it the "RETR" command?  */
    if (strcmp(buffer_ptr, "RETR") == 0)
    {
        return(NX_FTP_RETR);
    }

    /* Is it the "STOR" command?  */
    if (strcmp(buffer_ptr, "STOR") == 0)
    {
        return(NX_FTP_STOR);
    }

    /* Is it the "RNFR" command?  */
    if (strcmp(buffer_ptr, "RNFR") == 0)
    {
        return(NX_FTP_RNFR);
    }

    /* Is it the "RNTO" command?  */
    if (strcmp(buffer_ptr, "RNTO") == 0)
    {
        return(NX_FTP_RNTO);
    }

    /* Is it the "DELE" command?  */
    if (strcmp(buffer_ptr, "DELE") == 0)
    {
        return(NX_FTP_DELE);
    }

    /* Is it the "RMD" or "XRMD" command?  */
    if ((strcmp(buffer_ptr, "RMD") == 0) || (strcmp(buffer_ptr, "XRMD") == 0))
    {
        return(NX_FTP_RMD);
    }

    /* Is it the "MKD" or "XMKD" command?  */
    if ((strcmp(buffer_ptr, "MKD") == 0) || (strcmp(buffer_ptr, "XMKD")== 0))
    {
        return(NX_FTP_MKD);
    }

    /* Is it the "CDUP" command?  */
    if (strcmp(buffer_ptr, "CDUP") == 0)
    {
        return(NX_FTP_CDUP);
    }

    /* Is it the "CWD" command?  */
    if (strcmp(buffer_ptr, "CWD") == 0)
    {
        return(NX_FTP_CWD);
    }

    /* Is it the "PWD" command?  */
    if (strcmp(buffer_ptr, "PWD") == 0)
    {
        return(NX_FTP_PWD);
    }

    /* Is it the "NLST" command?  */
    if (strcmp(buffer_ptr, "NLST") == 0)
    {
        return(NX_FTP_NLST);
    }

    /* Is it the "LIST" command?  */
    if (strcmp(buffer_ptr, "LIST") == 0)
    {
        return(NX_FTP_LIST);
    }

    /* Is it the "PORT" command?  */
    if (strcmp(buffer_ptr, "PORT") == 0)
    {
        return(NX_FTP_PORT);
    }

    /* Is it the "EPRT" command?  */
    if (strcmp(buffer_ptr, "EPRT") == 0)
    {
        return(NX_FTP_EPRT);
    }

    /* Is it the "TYPE" command?  */
    if (strcmp(buffer_ptr, "TYPE") == 0)
    {
        return(NX_FTP_TYPE);
    }

    /* Is it the "NOOP" command?  */
    if (strcmp(buffer_ptr, "NOOP") == 0)
    {
        return(NX_FTP_NOOP);
    }

    /* Is it the "PASV" command?  */
    if (strcmp(buffer_ptr, "PASV") == 0)
    {
        return(NX_FTP_PASV);
    }

    /* Is it the "EPSV" command?  */
    if (strcmp(buffer_ptr, "EPSV") == 0)
    {
        return(NX_FTP_EPSV);
    }

    /* Is it the "MODE" command?  */
    if (strcmp(buffer_ptr, "MODE") == 0)
    {
        return(NX_FTP_MODE);
    }

    /* Otherwise, just return INVALID.  */
    return(NX_FTP_INVALID);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_timeout                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the periodic timer for this FTP server.  Its duty  */
/*    is to inform the FTP server that it is time to check for activity   */
/*    timeouts.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_address                    FTP server's address          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX                               ThreadX timer callback        */
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
VOID  _nx_ftp_server_timeout(ULONG ftp_server_address)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  */
    server_ptr =  (NX_FTP_SERVER *) ftp_server_address;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_ACTIVITY_TIMEOUT, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_timeout_processing                   PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reviews all the active FTP client connections and     */
/*    looks for an activity timeout. If a connection has not had any      */
/*    activity within NX_FTP_ACTIVITY_TIMEOUT seconds, the connection is  */
/*    deleted and its resources are made available to a new connection.   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_close                         Close file                    */
/*    nx_packet_release                     Release saved packet          */
/*    nx_tcp_server_socket_relisten         Relisten for another connect  */
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */
/*    nx_tcp_socket_disconnect              Disconnect socket             */
/*    _nx_ftp_server_data_socket_cleanup    Clean up data socket          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of clearing */
/*                                            data socket,                */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ftp_server_timeout_processing(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                    i;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;


    /* Examine all the client request structures.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Now see if this socket has an activity timeout active.  */
        if (client_req_ptr -> nx_ftp_client_request_activity_timeout)
        {

            /* Decrement the activity timeout for this client request.  */
            if (client_req_ptr -> nx_ftp_client_request_activity_timeout > NX_FTP_TIMEOUT_PERIOD)
                client_req_ptr -> nx_ftp_client_request_activity_timeout =  client_req_ptr -> nx_ftp_client_request_activity_timeout - NX_FTP_TIMEOUT_PERIOD;
            else
                client_req_ptr -> nx_ftp_client_request_activity_timeout =  0;

            /* Determine if this entry has exceeded the activity timeout.  */
            if (client_req_ptr -> nx_ftp_client_request_activity_timeout == 0)
            {

                /* Yes, activity timeout has been exceeded.  Tear down and cleanup the
                   entire client request structure.  */

                /* Increment the activity timeout counter.  */
                ftp_server_ptr -> nx_ftp_server_activity_timeouts++;

                /* If create, cleanup the data socket.  */
                if (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
                {

                    /* Clean up the client socket.  */
                    _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);
                }

                /* Reset the transfer mode as stream mode.  */
                client_req_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM; 

                /* Reset the block bytes.  */
                client_req_ptr -> nx_ftp_client_request_block_bytes = 0;

                /* Now disconnect the command socket.  */
                nx_tcp_socket_disconnect(&(client_req_ptr -> nx_ftp_client_request_control_socket), NX_NO_WAIT);

                /* Unaccept the server socket.  */
                nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_control_socket));

                /* Check to see if a packet is queued up.  */
                if (client_req_ptr -> nx_ftp_client_request_packet)
                {

                    /* Yes, release it!  */
                    nx_packet_release(client_req_ptr -> nx_ftp_client_request_packet);
                }

                /* Relisten on this socket. This will probably fail, but it is needed just in case all available
                   clients were in use at the time of the last relisten.  */
                nx_tcp_server_socket_relisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT,
                                                    &(client_req_ptr -> nx_ftp_client_request_control_socket));
            }
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_control_disconnect                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function notifies the FTP server thread of client disconnects  */
/*    of the control socket.                                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    control_socket_ptr                    Control socket event occurred */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set events for server thread  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                  NetX connect callback         */
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
VOID  _nx_ftp_server_control_disconnect(NX_TCP_SOCKET *control_socket_ptr)
{

NX_FTP_SERVER   *server_ptr;


    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  (NX_FTP_SERVER *)control_socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_ftp_server_event_flags), NX_FTP_SERVER_CONTROL_DISCONNECT, TX_OR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_control_disconnect_processing         PORTABLE C     */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reviews all the active FTP client connections and     */
/*    looks for a client initiated disconnect activity that was done      */
/*    without a QUIT command.                                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ftp_server_ptr                        Pointer to FTP server         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    fx_file_close                         Close file                    */
/*    nx_packet_release                     Release saved packet          */
/*    nx_tcp_server_socket_relisten         Relisten for another connect  */
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */
/*    nx_tcp_socket_disconnect              Disconnect socket             */
/*    _nx_ftp_server_data_socket_cleanup    Clean up data socket          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_thread_entry           FTP server thread             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of clearing */
/*                                            data socket,                */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ftp_server_control_disconnect_processing(NX_FTP_SERVER *ftp_server_ptr)
{

UINT                    i;
NX_FTP_CLIENT_REQUEST   *client_req_ptr;


    /* Examine all the client request structures.  */
    for (i = 0; i < NX_FTP_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(ftp_server_ptr -> nx_ftp_server_client_list[i]);

        /* Determine if this socket is in a disconnect state.  This should only happen
           if the client issues a disconnect without issuing a QUIT command.  */
        if ((client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_state > NX_TCP_ESTABLISHED) ||
            ((client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_state < NX_TCP_SYN_SENT) &&
             (client_req_ptr -> nx_ftp_client_request_activity_timeout > 0)))
        {

            /* Yes, this socket needs to be torn down.  */

            /* Increment the number of disconnection requests.  */
            ftp_server_ptr -> nx_ftp_server_disconnection_requests++;

            /* Clear authentication.  */
            client_req_ptr -> nx_ftp_client_request_authenticated =  NX_FALSE;

            /* Disable the client request activity timeout.  */
            client_req_ptr -> nx_ftp_client_request_activity_timeout =  0;

            /* If create, cleanup the associated data socket.  */
            if (client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_id)
            {

                /* Clean up the client socket.  */
                _nx_ftp_server_data_socket_cleanup(ftp_server_ptr, client_req_ptr);
            }

            /* Reset the transfer mode as stream mode.  */
            client_req_ptr -> nx_ftp_client_request_transfer_mode = NX_FTP_TRANSFER_MODE_STREAM;

            /* Reset the block bytes.  */
            client_req_ptr -> nx_ftp_client_request_block_bytes = 0;

            /* Check if this client login.  */
            if (client_req_ptr -> nx_ftp_client_request_login)
            {

                /* Call the logout function.  */

#ifndef NX_DISABLE_IPV4
                /* Does this server have an IPv4 login function? */
                if (ftp_server_ptr -> nx_ftp_logout_ipv4)
                {

                    /* Call the logout which takes IPv4 address input. */
                    (ftp_server_ptr -> nx_ftp_logout_ipv4)(ftp_server_ptr, 
                                client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                                client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                client_req_ptr -> nx_ftp_client_request_username,
                                client_req_ptr -> nx_ftp_client_request_password, NX_NULL);
                }
#endif /* NX_DISABLE_IPV4 */
                if (ftp_server_ptr -> nx_ftp_logout)
                {

                    /* Call the 'duo' logout function which takes IPv6 or IPv4 IP addresses. */
                    (ftp_server_ptr -> nx_ftp_logout)(ftp_server_ptr, &(client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_ip),
                                                      client_req_ptr -> nx_ftp_client_request_control_socket.nx_tcp_socket_connect_port,
                                                      client_req_ptr -> nx_ftp_client_request_username,
                                                      client_req_ptr -> nx_ftp_client_request_password, NX_NULL);
                }

                /* Set the login as FALSE.  */
                client_req_ptr -> nx_ftp_client_request_login = NX_FALSE;
            }

            /* Now disconnect the command socket.  */
            nx_tcp_socket_disconnect(&(client_req_ptr -> nx_ftp_client_request_control_socket), NX_FTP_SERVER_TIMEOUT);

            /* Unaccept the server socket.  */
            nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_control_socket));

            /* Check to see if a packet is queued up.  */
            if (client_req_ptr -> nx_ftp_client_request_packet)
            {

                /* Yes, release it!  */
                nx_packet_release(client_req_ptr -> nx_ftp_client_request_packet);
            }

            /* Relisten on this socket. This will probably fail, but it is needed just in case all available
               clients were in use at the time of the last relisten.  */
            nx_tcp_server_socket_relisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, NX_FTP_SERVER_CONTROL_PORT,
                                                    &(client_req_ptr -> nx_ftp_client_request_control_socket));
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_packet_allocate                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function uses knowledge of the client request connection type  */
/*    to allocate the proper TCP packet type.                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Pointer to packet pool        */
/*    client_request_ptr                    Pointer to Client request     */
/*    packet_ptr                            Pointer to allocated packet   */
/*    packet_type                           Packet type, usually TCP      */
/*    wait_option                           Wait option on packet alloc   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Packet allocation status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_allocate                    NetX packet allocate service  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_command_process        Client command handler        */
/*    _nx_ftp_server_control_disconnect_processing                        */
/*                                          Client disconnect handler     */
/*    _nx_ftp_server_connect_process        Client Connect request handler*/
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

UINT _nx_ftp_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_FTP_CLIENT_REQUEST *client_request_ptr, NX_PACKET **packet_ptr, UINT packet_type, UINT wait_option)
{

UINT status;


    /* Determine type of packet to allocate based on client connection. */

    if (client_request_ptr -> nx_ftp_client_request_ip_type == NX_IP_VERSION_V6)
    {

        /*  This is an IPV6 connection. */
        status = nx_packet_allocate(pool_ptr, packet_ptr, NX_IPv6_TCP_PACKET, wait_option);
    }
    else
    {

        /* This is an IPv4 connection.  */
        status = nx_packet_allocate(pool_ptr, packet_ptr, NX_IPv4_TCP_PACKET, wait_option);
    }


    NX_PARAMETER_NOT_USED(packet_type);

    /* Return completion status. */
    return status;
}


#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_utility_parse_IPv6_address                  PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a string containing a properly formatted IPv6  */
/*    string and while it does not assume it is part of the EPRT command  */
/*    it does not expect to find any non IPv6 characters in the string    */
/*    after checking for a leading 'EPRT |2|' and before an entire IPv6   */
/*    address is parsed.  If there is a leading 'EPRT ' in the string, it */
/*    does check that it has the required '|2|' preamble to the IPv6      */
/*    address.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer_ptr                            Pointer to the string to parse*/
/*    buffer_length                         Size of the IPv6 string       */
/*    ipduo_address                         Pointer to the IPv6 address   */
/*                                            parsed from the buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Proper IPv6 address parsed    */
/*    NX_FTP_ERROR                          IPv6 address is bad           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_command_process        Parse a received FTP command  */
/*                                                packet service          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            colons count verification,  */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT _nx_ftp_utility_parse_IPv6_address(CHAR *buffer_ptr, UINT buffer_length, NXD_ADDRESS *ipduo_address)
{

UINT j, k, i, m, colons, digit; 
UINT  number_consecutive_zeros_to_add, remaining_colons, actual_colons_left;
ULONG  temp;


    /* Initialize variables. */
    temp =                  0;
    colons =                0;
    remaining_colons =      0;
    actual_colons_left =    0;
    j =                     0;
    k =                     0;
    i =                     0;

    number_consecutive_zeros_to_add = 0;

    /* Check if the buffer includes the leading EPRT command. */
    if ( (buffer_ptr[0] == 'E' || buffer_ptr[0] == 'e') && 
         (buffer_ptr[1] == 'P' || buffer_ptr[1] == 'p') &&
         (buffer_ptr[2] == 'R' || buffer_ptr[2] == 'p') && 
         (buffer_ptr[3] == 'T' || buffer_ptr[3] == 't') &&
         (buffer_ptr[4] == ' '))
    {

        /* If this is an EPRT command it must be followed by a single
           space and the |2| preamble. */
        if ((buffer_ptr[5] != '|') || (buffer_ptr[6] != '2') || (buffer_ptr[7] != '|'))
        {

            /* It does not. Return error status. */
            return NX_FTP_INVALID_COMMAND;
        }

        /* Set the start of where to resume parsing. */
        j = 8;
    }
    else
    {
        /* If the EPRT command has been removed already, check for the |2| preamble. */
        if ((buffer_ptr[0] == '|') && (buffer_ptr[1] == '2') && (buffer_ptr[2] == '|'))
        {

            /* Set the start of where to resume parsing. */
            j = 3;
        }
    }

    /* Parse the buffer till the end is reached (or a complete
       IPv6 address is parsed. */
    while (j < buffer_length)
    {

        /* Is a numeric or hex character present?  */
        if (((buffer_ptr[j] >= '0') && (buffer_ptr[j] <= '9')) ||
            ((buffer_ptr[j] >= 'A') && (buffer_ptr[j] <= 'F')) ||
            ((buffer_ptr[j] >= 'a') && (buffer_ptr[j] <= 'f')))
        {

            /* Yes, is a numeric character is present?  Update the IP address.  */
            if (buffer_ptr[j] >= '0' && buffer_ptr[j] <= '9')
            {
                temp =  (temp*16) + (ULONG) (buffer_ptr[j] - '0');
            }
            else
            {
                /* Is a hex character present? */
                switch (buffer_ptr[j] )
                {
                    case 'A':
                    case 'a':
                    digit = 10;
                    break;

                    case 'B':
                    case 'b':
                        digit = 11;
                        break;

                    case 'C':
                    case 'c':
                        digit = 12;
                        break;

                    case 'D':
                    case 'd':
                        digit = 13;
                        break;

                    case 'E':
                    case 'e':
                        digit = 14;
                        break;

                    case 'F':
                    case 'f':
                        digit = 15;
                        break;

                    default:
                        digit = 0;
                }

                /* Update the ULONG. */
                temp =  (temp*16 + digit);
            }

            /* Increase the size of the digit by one. */
            i++;

            /* Check if the size of this digit is out of range. */
            if (i > 4)
            {

                /* Yes, return an error status. */
                return NX_FTP_INVALID_ADDRESS;
            }
        }

        /* Determine if a CR/LF is present.  */
        else if ((buffer_ptr[j] == 13) || (buffer_ptr[j] == 10) || (buffer_ptr[j] == 0))
        {

            /*  Whether this is a prematurely terminated IPv6 string, abort any further parsing. */
            break;
        }

        /* Determine if a colon is present.  */
        else if (buffer_ptr[j] == ':')
        {

            /* Check if colons is valid.  */
            if (colons >= 7)
            {
                return NX_FTP_INVALID_ADDRESS;
            }

            /* Increment the colon count. */
            colons++;

            /* Are we in the middle of a ULONG? */
            if (colons % 2 == 1)
            {

                /* Yes, add this temp to the lower half.  */
                ipduo_address -> nxd_ip_address.v6[k] =  temp; 
            }
            else
            {

                /* No, shift the existing 16 bits left first. */
                ipduo_address -> nxd_ip_address.v6[k] <<= 16;

                /* Mask out the lower 16 bits. */
                ipduo_address -> nxd_ip_address.v6[k] = ipduo_address -> nxd_ip_address.v6[k] & 0xFFFF0000;

                /* Add the number we just parsed. */
                ipduo_address -> nxd_ip_address.v6[k] += temp;

                /* Move to the next ULONG in the nxd_ip_address array. */
                k++;
            }

            /* Reset our counters and temporary ULONG. */
            i = 0;
            temp =  0;

            /* Check if we have encountered a double colon. */
            if (buffer_ptr[j + 1] == ':')
            {

                /* Yes; Check if we already encountered a double colon. */
                if (remaining_colons)
                {

                    /* Yes we have. This is an improperly formatted IPv6 address. */
                    return NX_FTP_INVALID_ADDRESS;
                }

                /* Move the index to the second colon. */
                j++;

                /* Compute how many more colons should there be. */
                remaining_colons = 7 - colons;

                /* Start looking for the rest of the colons after the double. */
                m = j + 1;

                /* Count colons in the rest of the string. */
                while (m < buffer_length)
                {

                    /* Do we have another colon? */
                    if (buffer_ptr [m] == ':')
                    {

                        /* Yes, update the count. */
                        actual_colons_left++;
                    }
                    m++;
                }
                
                /* Check if colons is valid. */
                if (actual_colons_left >= remaining_colons)
                {
                    return NX_FTP_INVALID_ADDRESS;
                }

                /* Compute the number of zeros to insert into the NXD_ADDRESS IPv6 ULONG array. */
                number_consecutive_zeros_to_add = remaining_colons - actual_colons_left;
              
                /* Add the zeroes until its time to start parsing rest of the IPv6 address string. */
                while (number_consecutive_zeros_to_add)
                {

                    /* Determine where to add the 16 bits depending 
                       on odd or even number colon we're on. */

                    /* Are we in the middle of the current v6 ULONG?*/
                    if (colons % 2 == 1)
                    {

                        /* Yes, shift the bits in v6[k] left, and insert a zero at the least significant end. */
                        ipduo_address -> nxd_ip_address.v6[k] <<= 16;
                        ipduo_address -> nxd_ip_address.v6[k] = ipduo_address -> nxd_ip_address.v6[k] & 0xFFFF0000;
                        k++;
                    }
                    else
                    {

                        /* No, starting the next ULONG in v6 so just set to zero. */
                        ipduo_address -> nxd_ip_address.v6[k] = 0;
                    }

                    /* Keep track of how many colons would have been in the IPv6 string
                       without the double colon. */
                    colons++; 

                    number_consecutive_zeros_to_add--;
                }
            }

        }
        /* Have we hit the end of the IPv6 address? */
        else if (buffer_ptr[j] == '|' && k <= 3)
        {

            /* Yes, shift the existing 16 bits left first. */
            ipduo_address -> nxd_ip_address.v6[k] <<= 16;

            /* Mask out the lower 16 bits. */
            ipduo_address -> nxd_ip_address.v6[k] = ipduo_address -> nxd_ip_address.v6[k] & 0xFFFF0000;

            /* Add the number we just parsed. */
            ipduo_address -> nxd_ip_address.v6[k] += temp;

            /* Clear the temporary ULONG so we know we applied it to our IPv6 address array. */
            temp = 0;

            /* This is the end of the IP address. Bail out...*/
            break;
        }
        else  /* No hex, not a seperator or part of the EPRT command. */
        {
            /* Unknown character in the IPv6 string. */
            return NX_FTP_INVALID_ADDRESS;
        }

        j++;
    }

    /* Did we parse the full IP address?  */
    if (colons != 7 )
    {

        /* No, it is either too long or too short. */
        return  NX_FTP_INVALID_ADDRESS;
    }
    
    /* Is there a parsed number to apply to the last ULONG e.g. no terminating '|' 
       in the buffer? */
    if (temp)
    {
        /* Yes, shift the existing 16 bits left first. */
        ipduo_address -> nxd_ip_address.v6[3] <<= 16;

        /* Mask out the lower 16 bits. */
        ipduo_address -> nxd_ip_address.v6[3] = ipduo_address -> nxd_ip_address.v6[3] & 0xFFFF0000;

        /* Add the number we just parsed. */
        ipduo_address -> nxd_ip_address.v6[3] += temp;
    }

    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_utility_parse_port_number                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a port number from the input buffer. It assumes*/
/*    the buffer holds an EPRT command string and therefore looks for the */
/*    last '|' separator in the string.  If it does not find one, it      */
/*    returns an error. If it encounters anything after this separator,   */
/*    other than a digit or CRLF before reaching the end of the buffer it */
/*    returns an error.                                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer_ptr                            Pointer to the string to parse*/
/*    buffer_length                         Size of the string            */
/*    portnumber                            Pointer to the port number    */
/*                                            parsed from the buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Proper port number  parsed    */
/*    NX_FTP_ERROR                          Port number is bad            */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_command_process        Parse a received FTP command  */
/*                                                packet service          */
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

UINT _nx_ftp_utility_parse_port_number(CHAR *buffer_ptr, UINT buffer_length, UINT *portnumber)
{

UINT j;
UINT digits = 0;


    j = (buffer_length - 1);
    *portnumber = 0;

    /* Find the terminating CRLF. */
    while (j >= 1)
    {
        if (buffer_ptr[j - 1] == 13 && buffer_ptr[j] == 10)
        {
            /* Subtract for the extra LF character. */
            j--;

            /* We're ready to move on to the port number extraction. */
            break;
        }
        j--;
    }

    /* Find the port number. There may be separators between the port number
       and the CRLF. */
    while (j >= 1)
    {
        /* Skip everything except for digits. */
        if (buffer_ptr[j] >= '0' && buffer_ptr[j] <= '9')
        {
            break;
        }

        /* But we should not encounter a colon! */
        if (buffer_ptr[j] == ':')
        {

            /* Return an error status. */
            return NX_FTP_INVALID_NUMBER;
        }

        j--;
    }

    while ((INT)j >= 0)
    {

        /* Is this a space separator '|'?  */
        if (buffer_ptr[j] == '|')
        {
            /* Yes, so the port number must be adjacent. */
            j++;

            /* Now go back up the buffer. */
            while(j < buffer_length - 2)
            {

                /* Check if we have backed up into the IPv6 address? */
                if ((buffer_ptr[j] == ':'))
                {
                    /* Yes, apparently this string is missing the separator and port number. */
                    return NX_FTP_INVALID_NUMBER;
                }

                /* Is a numeric character present?  */
                if ((buffer_ptr[j] >= '0') && (buffer_ptr[j] <= '9'))
                {
        
                    /* Yes, numeric character is present.  Update the port number.  */
                    *portnumber =  (*portnumber * 10) + (ULONG) (buffer_ptr[j] - '0');
                     digits++;

                    /* Check for overflow. */
                    if (digits > 5)
                    {
                        return NX_FTP_INVALID_NUMBER;
                    }
                    /* Determine if a CR/LF or | separator is present.  This marks
                       the end of the port number. */
                    if ((buffer_ptr[j+1] == '|') || (buffer_ptr[j+1] == 13) || (buffer_ptr[j+2] == 10))
                    {
                        return  NX_SUCCESS;
                    }

                    
                }
                else
                {

                    /* This is a non numeric character, and not a CRLF either. */
                    return NX_FTP_INVALID_NUMBER;
                }
                j++;
            }

            /* If we got here we did not parse a properly formatted port number. */
            return NX_FTP_INVALID_NUMBER;
        }

        j--;
    }

    /* If we got here we did not parse a properly formatted port number. */
    return NX_FTP_INVALID_NUMBER;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ftp_server_utility_fill_port_number             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts a number to ascii text e.g. and fill the     */
/*    text port into the buffer.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                         Port number to convert to ASCII      */
/*    numstring                      Pointer to ascii portnumber string   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    size                           Size of port number                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ftp_server_command_process Processes FTP commands               */
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
UINT  _nx_ftp_server_utility_fill_port_number(CHAR *buffer_ptr, UINT port_number)
{

UINT    j;
UINT    digit;
UINT    size;


    /* Initialize counters.  */
    size = 0;

    /* Loop to convert the number to ASCII.  */
    while (size < 10)
    {

        /* Shift the current digits over one.  */
        for (j = size; j != 0; j--)
        {

            /* Move each digit over one place.  */
            buffer_ptr[j] =  buffer_ptr[j-1];
        }

        /* Compute the next decimal digit.  */
        digit = (port_number % 10);

        /* Update the input number.  */
        port_number = (port_number / 10);

        /* Store the new digit in ASCII form.  */
        buffer_ptr[0] = (CHAR) (digit + 0x30);

        /* Increment the size.  */
        size++;

        /* Determine if the number is now zero.  */
        if (port_number == 0)
            break;
    }

    /* Determine if there is an overflow error.  */
    if (port_number)
    {

        /* Error, return bad values to user.  */
        return(0);
    }

    /* Return size to caller.  */
    return(size);
}
#endif /* FEATURE_NX_IPV6 */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_server_block_size_get                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the block size for LIST and NLST in block mode.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    client_request_ptr                    Pointer to FTP client         */ 
/*    block_size                            The size of block data        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_directory_first_full_entry_find    Find first directory entry    */ 
/*    fx_directory_next_full_entry_find     Find next directory entry     */ 
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_server_command_process        Process command               */ 
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
VOID  _nx_ftp_server_block_size_get(NX_FTP_SERVER *ftp_server_ptr, UINT ftp_command, CHAR *filename, ULONG *block_size)
{

UINT        j = 0;
UINT        attributes;
ULONG       size;
UINT        year, month, day;
UINT        hour, minute, second;
UINT        length;
UINT        status;


    /* Block size for LIST and NLST.  */
    *block_size = 0;

    /* Now get the full directory listing.  */
    do
    {
        if (j == 0)
        {

            /* First directory entry.  */
            status =  fx_directory_first_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                            &attributes, &size, &year, &month, &day, &hour, &minute, &second);
            j++;
        }
        else
        {

            /* Not the first entry - pickup the next!  */
            status =  fx_directory_next_full_entry_find(ftp_server_ptr -> nx_ftp_server_media_ptr, filename,
                                            &attributes, &size, &year, &month, &day, &hour, &minute, &second);
        }

        /* Determine if successful.  */
        if (status == NX_SUCCESS)
        {

            /* Calculate the size of the name.  */
            if (_nx_utility_string_length_check(filename, &length, FX_MAX_LONG_NAME_LEN - 1))
            {
                return;
            }

            /* Check the command.  */
            if (ftp_command == NX_FTP_NLST)
            {

                /* NLST. Add extra "cr/lf" (2 bytes).  */
                (*block_size) += (length + 2);
            }
            else
            {

                /* Check if the month is valid.  */
                if ((month < 1) || (month > 12))
                    continue;

                /* LIST. Add extra file info (52 bytes).
                   File Info is 10 chars for permissions, 15 chars for owner and group,
                   11 chars for size (for file size up to 4gB), 14 for date, 2 chars for cr lf.  */ 
                (*block_size) += (length + 52);
            }
        }
    } while (status == NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_server_block_header_send                    PORTABLE C      */ 
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
/*    client_request_ptr                    Pointer to FTP client         */ 
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
/*    _nx_ftp_server_command_process        Process command               */ 
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
UINT  _nx_ftp_server_block_header_send(NX_PACKET_POOL *pool_ptr, NX_FTP_CLIENT_REQUEST *client_request_ptr, ULONG block_size)
{

UINT        status;
NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;


    /* Use block mode to transfer data.  RFC959, Section3.4.2, Page21-22.  */

    /* Allocate the packet.  */
    status = _nx_ftp_packet_allocate(pool_ptr, client_request_ptr, &packet_ptr, NX_TCP_PACKET, NX_FTP_SERVER_TIMEOUT);

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
    status =  nx_tcp_socket_send(&(client_request_ptr -> nx_ftp_client_request_data_socket), packet_ptr, NX_FTP_SERVER_TIMEOUT);

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
/*    _nx_ftp_server_block_header_retrieve                PORTABLE C      */ 
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
/*    client_request_ptr                    Pointer to FTP client         */ 
/*    packet_ptr                            Pointer to packet to write    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*   _nx_ftp_server_data_process            Process client write data     */ 
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
UINT  _nx_ftp_server_block_header_retrieve(NX_FTP_CLIENT_REQUEST *ftp_client_ptr, NX_PACKET *packet_ptr)
{

ULONG       remaining_bytes;
ULONG       delta;
UCHAR       *buffer_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET   *before_last_packet;
NX_PACKET   *last_packet;
#endif /* NX_DISABLE_PACKET_CHAIN */


    /* Check if it is the first packet.  */
    if (ftp_client_ptr -> nx_ftp_client_request_block_bytes == 0)
    {

        /* Check the packet length.  */
        if ((packet_ptr -> nx_packet_length < 3) ||
            (packet_ptr -> nx_packet_prepend_ptr + 3 > packet_ptr -> nx_packet_append_ptr))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_FTP_SERVER_INVALID_SIZE);
        }

        /* We have a packet, setup pointer to the buffer area.  */
        buffer_ptr = packet_ptr -> nx_packet_prepend_ptr;

        /* Process block header.  */
        ftp_client_ptr -> nx_ftp_client_request_block_bytes = (ULONG)((buffer_ptr[1] << 8) | buffer_ptr[2]);

        /* Skip the block header.  */
        packet_ptr -> nx_packet_prepend_ptr += 3;
        packet_ptr -> nx_packet_length -= 3;
    }

    /* Check if have remaining data.  */
    remaining_bytes = ftp_client_ptr -> nx_ftp_client_request_block_bytes - ftp_client_ptr -> nx_ftp_client_request_total_bytes;
    if (remaining_bytes == 0)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return(NX_FTP_SERVER_END_OF_BLOCK);
    }

    /* Check the data of current packet.  */
    if (remaining_bytes < packet_ptr -> nx_packet_length)
    {

        /* Remove the extra data, such as: end block header.  */

        /* Calculate the difference in the length.  */
        delta =  packet_ptr -> nx_packet_length - remaining_bytes;

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

    /* Return success to caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_server_number_to_ascii                      PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function converts a number to ascii text. Fill space at the     */
/*   beginning if possible.                                               */
/*                                                                        */ 
/*   INPUT                                                                */ 
/*                                                                        */ 
/*    buffer_ptr                     Pointer to output string buffer      */
/*    buffer_size                    Size of output buffer                */
/*    number                         Number to convert to ASCII           */
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
/*    _nx_ftp_server_command_process                                      */ 
/*                                   Process command                      */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the size check,   */
/*                                            resulting in version 6.1.7  */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            corrected the pad character,*/
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
static VOID _nx_ftp_server_number_to_ascii(UCHAR *buffer_ptr, UINT buffer_size, UINT number, UCHAR pad)
{
UINT    digit;
UINT    size;

    /* Initialize counters.  */
    size = 1;

    /* Initialize buffer with pad character. */
    memset(buffer_ptr, pad, buffer_size);

    /* Loop to convert the number to ASCII.  */
    while (size <= buffer_size)
    {

        /* Compute the next decimal digit.  */
        digit = (number % 10);

        /* Update the input number.  */
        number = (number / 10);

        /* Store the new digit in ASCII form.  */
        buffer_ptr[buffer_size - size] = (UCHAR)(digit + '0');

        /* Increment the size.  */
        size++;

        /* Determine if the number is now zero.  */
        if (number == 0)
            break;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ftp_server_data_socket_cleanup                  PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function cleans up the data socket.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ftp_server_ptr                        Pointer to FTP server         */
/*    client_req_ptr                        Pointer to client request     */
/*                                                                        */
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    fx_file_close                         Close file                    */
/*    nx_tcp_socket_disconnect              Disconnect a socket           */ 
/*    nx_tcp_client_socket_unbind           Release the data socket port  */
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server socket     */
/*    nx_tcp_socket_delete                  Delete socket                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ftp_server_command_process        Process command               */
/*    _nx_ftp_server_data_disconnect_process                              */
/*                                          Disconnect data socket        */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021     Yuxin Zhou               Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
VOID _nx_ftp_server_data_socket_cleanup(NX_FTP_SERVER *ftp_server_ptr, NX_FTP_CLIENT_REQUEST *client_req_ptr)
{

    /* First, cleanup this data socket.  */
    nx_tcp_socket_disconnect(&(client_req_ptr -> nx_ftp_client_request_data_socket), NX_FTP_SERVER_TIMEOUT);

    /* Unbind/unaccept the data socket.  */
    if (client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled == NX_TRUE)
    {
        nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_ftp_client_request_data_socket));
        nx_tcp_server_socket_unlisten(ftp_server_ptr -> nx_ftp_server_ip_ptr, client_req_ptr -> nx_ftp_client_request_data_socket.nx_tcp_socket_port);
    }
    else
    {
        nx_tcp_client_socket_unbind(&(client_req_ptr -> nx_ftp_client_request_data_socket));
    }

    /* And finally delete the data socket.  */
    nx_tcp_socket_delete(&(client_req_ptr -> nx_ftp_client_request_data_socket));

    fx_file_close(&(client_req_ptr -> nx_ftp_client_request_file));

#ifdef NX_FTP_FAULT_TOLERANT

    /* Flush the media.  */
    fx_media_flush(ftp_server_ptr -> nx_ftp_server_media_ptr);
#endif

    /* Clear the passive transfer enabled flag.  */
    client_req_ptr -> nx_ftp_client_request_passive_transfer_enabled = NX_FALSE;
}