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
/**   TELNET Protocol (TELNET)                                            */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_TELNET_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nxd_telnet_server.h"
#include    "stdio.h"
#include    "string.h"


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_create                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server create call.   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    server_name                           Name of TELNET server         */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Server thread's stack pointer */ 
/*    stack_size                            Server thread's stack size    */ 
/*    new_connection                        Pointer to user's new         */ 
/*                                            connection function         */ 
/*    receive_data                          Pointer to user's receive     */ 
/*                                            data function               */ 
/*    connection_end                        Pointer to user's end of      */ 
/*                                            connection function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_create              Actual server create call     */ 
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
UINT  _nxe_telnet_server_create(NX_TELNET_SERVER *server_ptr, CHAR *server_name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, 
            void (*new_connection)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection), 
            void (*receive_data)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet_ptr),
            void (*connection_end)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection))
{

UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || 
        (server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id == NX_TELNET_SERVER_ID) || 
        (stack_ptr == NX_NULL) || (new_connection == NX_NULL) || (receive_data == NX_NULL) || (connection_end == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server create function.  */
    status =  _nx_telnet_server_create(server_ptr, server_name, ip_ptr, stack_ptr, stack_size, new_connection, receive_data, connection_end);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_create                           PORTABLE C       */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a TELNET server on the specified IP. In doing */ 
/*    so this function creates a TCP socket for subsequent TELNET         */ 
/*    transfers and a thread for the TELNET server.                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    server_name                           Name of TELNET server         */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_ptr                             Server thread's stack pointer */ 
/*    stack_size                            Server thread's stack size    */ 
/*    new_connection                        Pointer to user's new         */ 
/*                                            connection function         */ 
/*    receive_data                          Pointer to user's receive     */ 
/*                                            data function               */ 
/*    connection_end                        Pointer to user's end of      */ 
/*                                            connection function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_create                  Create sockets                */ 
/*    nx_tcp_socket_delete                  Delete sockets                */ 
/*    nx_tcp_socket_receive_notify          Register receive notify       */ 
/*                                            callback                    */ 
/*    tx_event_flags_create                 Create event flags            */ 
/*    tx_event_flags_delete                 Delete event flags            */ 
/*    tx_thread_create                      Create TELNET server thread   */ 
/*    tx_thread_delete                      Delete TELNET server thread   */ 
/*    tx_timer_create                       Create TELNET server timer    */ 
/*    tx_timer_delete                       Delete TELNET server timer    */ 
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
UINT  _nx_telnet_server_create(NX_TELNET_SERVER *server_ptr, CHAR *server_name, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, 
            void (*new_connection)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection), 
            void (*receive_data)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection, NX_PACKET *packet_ptr),
            void (*connection_end)(struct NX_TELNET_SERVER_STRUCT *telnet_server_ptr, UINT logical_connection))
{

UINT            i;
UINT            status;

    /* Clear the TELNET server structure.  */
    memset((void *) server_ptr, 0, sizeof(NX_TELNET_SERVER));

    /* Create the TELNET Server thread.  */
    status =  tx_thread_create(&(server_ptr -> nx_telnet_server_thread), "TELNET Server Thread", 
                               _nx_telnet_server_thread_entry, (ULONG) server_ptr, stack_ptr, 
                               stack_size, NX_TELNET_SERVER_PRIORITY, NX_TELNET_SERVER_PRIORITY, 
                               TX_NO_TIME_SLICE, TX_DONT_START);

    /* Determine if an error occurred creating the thread.  */
    if (status != TX_SUCCESS)
    {

        /* Error creating the server thread.  */
        return(status);
    }

    /* Create the ThreadX event flags.  These will be used to driver the TELNET server thread.  */
    status =  tx_event_flags_create(&(server_ptr -> nx_telnet_server_event_flags), "TELNET Server Thread Events");

    /* Determine if an error occurred creating the event flags.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the server thread.  */
        tx_thread_delete(&(server_ptr -> nx_telnet_server_thread));

        /* Error creating the server event flags.  */
        return(status);
    }

    /* Create the ThreadX activity timeout timer.  This will be used to periodically check to see if 
       a client connection has gone silent and needs to be terminated.  */
    status =  tx_timer_create(&(server_ptr -> nx_telnet_server_timer), "TELNET Server Timer", 
                              _nx_telnet_server_timeout, (ULONG) server_ptr, 
                              (NX_IP_PERIODIC_RATE * NX_TELNET_TIMEOUT_PERIOD), 
                              (NX_IP_PERIODIC_RATE * NX_TELNET_TIMEOUT_PERIOD), TX_NO_ACTIVATE);

    /* Determine if an error occurred creating the timer.  */
    if (status != TX_SUCCESS)
    {

        /* Delete the server thread.  */
        tx_thread_delete(&(server_ptr -> nx_telnet_server_thread));

        /* Delete the server event flags.  */
        tx_event_flags_delete(&(server_ptr -> nx_telnet_server_event_flags));

        /* Error creating the server timer.  */
        return(status);
    }

    /* Loop to create all the TELNET client sockets.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Setup the logical index for this client request structure.  */
        server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_connection =  i;

        /* Create an TELNET client socket.  */
        status +=  nx_tcp_socket_create(ip_ptr, &(server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_socket), "TELNET Server Control Socket",
                        NX_TELNET_TOS, NX_TELNET_FRAGMENT_OPTION, NX_TELNET_TIME_TO_LIVE, NX_TELNET_SERVER_WINDOW_SIZE, NX_NULL, _nx_telnet_server_disconnect_present);

        /* If no error is present, register the receive notify function.  */
        if (status == NX_SUCCESS)
        {

            /* Register the receive function.  */
            nx_tcp_socket_receive_notify(&(server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_socket), 
                                            _nx_telnet_server_data_present);
        }

        /* Make sure each socket points to the TELNET server.  */
        server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_socket.nx_tcp_socket_reserved_ptr =  server_ptr;
    }

    /* Determine if an error has occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Loop to delete any created sockets.  */
        for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
        {

            /* Delete the TELNET socket.  */
            nx_tcp_socket_delete(&(server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_socket));
        }

        /* Delete the server thread.  */
        tx_thread_delete(&(server_ptr -> nx_telnet_server_thread));

        /* Delete the event flag group.  */
        tx_event_flags_delete(&(server_ptr -> nx_telnet_server_event_flags));

        /* Delete the timer.  */
        tx_timer_delete(&(server_ptr -> nx_telnet_server_timer));

        /* Return the NetX error.  */
        return(status);
    }
    

#ifndef NX_TELNET_SERVER_OPTION_DISABLE

#ifndef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL

    /* Create the packet pool for negotiating telnet options, and check the status */
    status =  nx_packet_pool_create(&(server_ptr -> nx_telnet_server_packet_pool), "Telnet Server Options", 
                                    NX_TELNET_SERVER_PACKET_PAYLOAD, &server_ptr -> nx_telnet_server_pool_area, 
                                    NX_TELNET_SERVER_PACKET_POOL_SIZE);

    /* Determine if it was successful.  */
    if (status != NX_SUCCESS)
    {
        /* Loop to delete any created sockets.  */
        for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
        {

            /* Delete the TELNET socket.  */
            nx_tcp_socket_delete(&(server_ptr -> nx_telnet_server_client_list[i].nx_telnet_client_request_socket));
        }

        /* Delete the server thread.  */
        tx_thread_delete(&(server_ptr -> nx_telnet_server_thread));

        /* Delete the event flag group.  */
        tx_event_flags_delete(&(server_ptr -> nx_telnet_server_event_flags));

        /* Delete the timer.  */
        tx_timer_delete(&(server_ptr -> nx_telnet_server_timer));

        /* No, return error status.  */
        return(status);
    }

    server_ptr -> nx_telnet_server_packet_pool_ptr = &(server_ptr -> nx_telnet_server_packet_pool);
#endif /* NX_TELNET_SERVER_USER_CREATE_PACKET_POOL */
#endif /* NX_TELNET_SERVER_OPTION_DISABLE */

    /* Save the Server name.  */
    server_ptr -> nx_telnet_server_name =  server_name;

    /* Save the IP pointer address.  */
    server_ptr -> nx_telnet_server_ip_ptr =  ip_ptr;

    /* Save the user-supplied new connection and receive data functions.  */
    server_ptr -> nx_telnet_new_connection =   new_connection;
    server_ptr -> nx_telnet_receive_data =     receive_data;
    server_ptr -> nx_telnet_connection_end =   connection_end;

    /* Set the server ID to indicate the TELNET server thread is ready.  */
    server_ptr -> nx_telnet_server_id =  NX_TELNET_SERVER_ID;

    server_ptr -> nx_telnet_server_open_connections = 0;


    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_delete                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server delete call.   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_delete              Actual server delete call     */ 
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
UINT  _nxe_telnet_server_delete(NX_TELNET_SERVER *server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id != NX_TELNET_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_telnet_server_delete(server_ptr);

    /* Return completion status.  */
    return(status);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a previously created TELNET server on the     */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
UINT  _nx_telnet_server_delete(NX_TELNET_SERVER *server_ptr)
{

UINT                        i;
NX_TELNET_CLIENT_REQUEST   *client_request_ptr;


    /* Clear the server ID to indicate the TELNET server is no longer ready.  */
    server_ptr -> nx_telnet_server_id =  0;

    /* Suspend the TELNET server thread.  */
    tx_thread_suspend(&(server_ptr -> nx_telnet_server_thread));

    /* Terminate server thread. */
    tx_thread_terminate(&(server_ptr -> nx_telnet_server_thread));

    /* Delete server thread.  */
    tx_thread_delete(&(server_ptr -> nx_telnet_server_thread));

    /* Delete the event flag group.  */
    tx_event_flags_delete(&(server_ptr -> nx_telnet_server_event_flags));

    /* Deactivate and delete timer.  */
    tx_timer_deactivate(&(server_ptr -> nx_telnet_server_timer));
    tx_timer_delete(&(server_ptr -> nx_telnet_server_timer));

    /* If the Telnet Server is configured to send options and the packet
       pool is created by  the internally (by the Telnet server task)
       we need to delete it. */
#ifndef NX_TELNET_SERVER_OPTION_DISABLE
#ifndef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
    nx_packet_pool_delete(&server_ptr -> nx_telnet_server_packet_pool);
#endif /* NX_TELNET_SERVER_USER_CREATE_PACKET_POOL */
#endif /* NX_TELNET_SERVER_OPTION_DISABLE */

    /* Walk through the server structure to close and delete any open sockets.  */
    i =  0;
    client_request_ptr =  &(server_ptr -> nx_telnet_server_client_list[0]);
    while (i < NX_TELNET_MAX_CLIENTS)
    {

        /* Disconnect the socket.  */
        nx_tcp_socket_disconnect(&(client_request_ptr -> nx_telnet_client_request_socket), NX_NO_WAIT);

        /* Unaccept the socket.  */
        nx_tcp_server_socket_unaccept(&(client_request_ptr -> nx_telnet_client_request_socket));

        /* Delete the socket.  */
        nx_tcp_socket_delete(&(client_request_ptr -> nx_telnet_client_request_socket));

        /* Increment the pointer into the client request list.  */
        client_request_ptr++;
        i++;
    }

    /* Unlisten on the TELNET port.  */
    nx_tcp_server_socket_unlisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_disconnect                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server disconnect     */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    logical_connection                    Logical connection entry      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_disconnect          Actual server disconnect call */ 
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
UINT  _nxe_telnet_server_disconnect(NX_TELNET_SERVER *server_ptr, UINT logical_connection)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id != NX_TELNET_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for a valid logical connection.  */
    if (logical_connection >= NX_TELNET_MAX_CLIENTS)
        return(NX_OPTION_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server disconnect function.  */
    status =  _nx_telnet_server_disconnect(server_ptr, logical_connection);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_disconnect                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes server disconnect requests made by the      */ 
/*    application receive data callback function.                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    logical_connection                    Logical connection entry      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_server_socket_relisten         Relisten on Telnet port       */ 
/*    nx_tcp_server_socket_unaccept         Socket unaccept               */ 
/*    nx_tcp_socket_disconnect              Socket disconnect             */ 
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
UINT  _nx_telnet_server_disconnect(NX_TELNET_SERVER *server_ptr, UINT logical_connection)
{

UINT                        i;
UINT                        status;
NX_TELNET_CLIENT_REQUEST   *client_ptr;

    /* Set a pointer to the indicated client connection.  */
    client_ptr =  &(server_ptr -> nx_telnet_server_client_list[logical_connection]);

    /* Determine if the connection is alive.  */
    if (client_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state >= NX_TCP_ESTABLISHED)
    {

        /* Disconnect the socket.  */
        nx_tcp_socket_disconnect(&(client_ptr -> nx_telnet_client_request_socket), NX_TELNET_SERVER_TIMEOUT);

        /* Call the application's end connection callback routine.  */
        if (server_ptr -> nx_telnet_connection_end)
        {

            /* Yes, there is a connection end callback routine - call it!  */
            (server_ptr -> nx_telnet_connection_end)(server_ptr, client_ptr -> nx_telnet_client_request_connection);
        }

        /* Unaccept this socket.  */
        nx_tcp_server_socket_unaccept(&(client_ptr -> nx_telnet_client_request_socket));

        /* Update number of current open connections. */
        if (server_ptr -> nx_telnet_server_open_connections > 0)
            server_ptr -> nx_telnet_server_open_connections--;

        /* Clear the client request activity timeout.  */
        client_ptr -> nx_telnet_client_request_activity_timeout =  0;
    }
    else
    {

        /* Error, disconnecting an unconnected socket.  */
        return(NX_TELNET_NOT_CONNECTED);
    }

    /* Now look for a socket that is closed to relisten on.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Set a pointer to client request structure.  */
        client_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket is closed.  */
        if (client_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
        {

            /* Relisten on this socket.  */
            status =  nx_tcp_server_socket_relisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT, 
                                                    &(client_ptr -> nx_telnet_client_request_socket));
            /* Check for bad status.  */
            if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
            {

                /* Increment the error count and keep trying.  */
                server_ptr -> nx_telnet_server_relisten_errors++;
                continue;
            }
            
            /* Break out of loop.  */
            break;
        }
    }

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_packet_send                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server packet send    */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    logical_connection                    Logical connection entry      */ 
/*    packet_ptr                            Packet pointer to send        */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_packet_send         Actual server packet send call*/ 
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
UINT  _nxe_telnet_server_packet_send(NX_TELNET_SERVER *server_ptr, UINT logical_connection, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id != NX_TELNET_SERVER_ID) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for a valid logical connection.  */
    if (logical_connection >= NX_TELNET_MAX_CLIENTS)
        return(NX_OPTION_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server packet send function.  */
    status =  _nx_telnet_server_packet_send(server_ptr, logical_connection, packet_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_packet_send                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes packet send requests made from the          */ 
/*    application's receive data callback function.                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    logical_connection                    Logical connection entry      */ 
/*    packet_ptr                            Packet pointer to send        */ 
/*    wait_option                           Suspension option             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_socket_send                    Send packet                   */ 
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
UINT  _nx_telnet_server_packet_send(NX_TELNET_SERVER *server_ptr, UINT logical_connection, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT                        status;
NX_TELNET_CLIENT_REQUEST    *client_ptr;


    /* Derive the pointer to the appropriate client connection.  */
    client_ptr =  &(server_ptr -> nx_telnet_server_client_list[logical_connection]);

    /* Send the packet to the client.  */
    status =  nx_tcp_socket_send(&(client_ptr -> nx_telnet_client_request_socket), packet_ptr, wait_option);

    /* Determine if the send was successful.  */
    if (status)
    {

        /* Map to a generic error.  */
        status =  NX_TELNET_FAILED;
    }

    /* Return to caller.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_start                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server start call.    */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_start               Actual server start call      */ 
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
UINT  _nxe_telnet_server_start(NX_TELNET_SERVER *server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id != NX_TELNET_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Call actual server start function.  */
    status =  _nx_telnet_server_start(server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_start                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts a previously created TELNET server on the      */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_TELNET_NO_PACKET_POOL              Telnet server packet pool not */
/*                                             set yet                    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_server_socket_listen           Listen of TELNET clients      */ 
/*    tx_thread_resume                      Resume TELNET server thread   */ 
/*    tx_timer_activate                     Activate TELNET server timer  */ 
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
UINT  _nx_telnet_server_start(NX_TELNET_SERVER *server_ptr)
{

UINT    status;
ULONG   events;

#ifndef NX_TELNET_SERVER_OPTION_DISABLE

    /* Check for Telnet server packet pool we'll need to send options. */
    if (server_ptr -> nx_telnet_server_packet_pool_ptr == NX_NULL)
    {
    
        /* The Telnet server packet pool is not set. */
        return NX_TELNET_NO_PACKET_POOL;
    }
#endif

    /* Start listening on the TELNET socket.  */
    status =  nx_tcp_server_socket_listen(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT, 
                        &(server_ptr -> nx_telnet_server_client_list[0].nx_telnet_client_request_socket), 
                                    NX_TELNET_MAX_CLIENTS, _nx_telnet_server_connection_present);

    /* Determine if an error is present.  */
    if (status != NX_SUCCESS)
    {

        /* Error, return to caller.  */
        return(status);
    }

    /* Activate TELNET server timer.  */
    tx_timer_activate(&(server_ptr -> nx_telnet_server_timer));

    /* Clear stop event. */
    tx_event_flags_get(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_STOP_EVENT, TX_OR_CLEAR, &events, TX_NO_WAIT);

    /* Start the TELNET server thread.  */
    tx_thread_resume(&(server_ptr -> nx_telnet_server_thread));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_stop                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server stop call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_stop                Actual server start call      */ 
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
UINT  _nxe_telnet_server_stop(NX_TELNET_SERVER *server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_telnet_server_id != NX_TELNET_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_telnet_server_stop(server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_stop                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stops a previously started TELNET server on the       */ 
/*    specified IP.                                                       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set events for server thread  */ 
/*    tx_timer_deactivate                   Deactivate TELNET server timer*/ 
/*    nx_tcp_server_socket_unaccept         Unaccept server socket        */ 
/*    nx_tcp_server_socket_unlisten         Unlisten on server            */ 
/*    nx_tcp_socket_disconnect              Disconnect socket             */ 
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
UINT  _nx_telnet_server_stop(NX_TELNET_SERVER *server_ptr)
{
UINT                        i;
NX_TELNET_CLIENT_REQUEST   *client_request_ptr;

    /* Deactivate TELNET server timer.  */
    tx_timer_deactivate(&(server_ptr -> nx_telnet_server_timer));

    /* Suspend the TELNET server thread.  */
    tx_event_flags_set(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_STOP_EVENT, TX_OR);

    /* Walk through the server structure to close and delete any open sockets.  */
    i =  0;
    client_request_ptr =  &(server_ptr -> nx_telnet_server_client_list[0]);
    while (i < NX_TELNET_MAX_CLIENTS)
    {

        /* Disconnect the socket.  */
        nx_tcp_socket_disconnect(&(client_request_ptr -> nx_telnet_client_request_socket), NX_NO_WAIT);

        /* Unaccept the socket.  */
        nx_tcp_server_socket_unaccept(&(client_request_ptr -> nx_telnet_client_request_socket));

        /* Reset client request. */
        client_request_ptr -> nx_telnet_client_request_activity_timeout =  0;

        /* Increment the pointer into the client request list.  */
        client_request_ptr++;
        i++;
    }

    /* Unlisten on the TELNET port.  */
    nx_tcp_server_socket_unlisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_get_open_connection_count        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server get open       */
/*    connection count service.                                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    current_connections                   Pointer to # open connections */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_get_open_connection_count                         */
/*                                          Actual server get count call  */ 
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
UINT  _nxe_telnet_server_get_open_connection_count(NX_TELNET_SERVER *server_ptr, UINT *current_connections)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (current_connections == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_telnet_server_get_open_connection_count(server_ptr, current_connections);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_get_open_connection_count         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns the number of currently open connections.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    current_connections                   Pointer to # open connections */ 
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
UINT  _nx_telnet_server_get_open_connection_count(NX_TELNET_SERVER *server_ptr, UINT *current_connections)
{

    /* Retrieve server's record of open connections.  */
    *current_connections =  server_ptr -> nx_telnet_server_open_connections;

    /* Return completion status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_thread_entry                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the entry of the TELNET server.  All basic         */
/*    processing is initiated by this function.                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    telnet_server                             Pointer to TELNET server  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_telnet_server_connect_process          Process connection        */ 
/*   _nx_telnet_server_data_process             Process received data     */ 
/*   _nx_telnet_server_disconnect_process       Process disconnection     */ 
/*   _nx_telnet_server_timeout_processing       Process activity timeout  */ 
/*   tx_event_flags_get                         Get TELNET event(s)       */ 
/*   tx_thread_suspend                          Suspend TELNET thread     */ 
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
VOID  _nx_telnet_server_thread_entry(ULONG telnet_server)
{

NX_TELNET_SERVER        *server_ptr;
UINT                    status;
ULONG                   events;


    /* Setup the server pointer.  */
    server_ptr =  (NX_TELNET_SERVER *) telnet_server; 

    /* Loop to process TELNET Server requests.  */
    while(1)
    {

        /* Wait for an TELNET client activity.  */
        status =  tx_event_flags_get(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_ANY_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Check the return status.  */
        if (status)
        {

            /* If an error occurs, simply continue the loop.  */
            continue;
        }

        /* Check whether service is started. */
        if (events & NX_TELNET_STOP_EVENT)
        {

            /* Suspend thread here. */
            tx_thread_suspend(&server_ptr -> nx_telnet_server_thread);
            continue;
        }

        /* Otherwise, an event is present.  Process according to the event.  */

        /* Check for a client connection event.  */
        if (events & NX_TELNET_SERVER_CONNECT)
        {

            /* Call the connect processing.  */
            _nx_telnet_server_connect_process(server_ptr);
        }

        /* Check for a TELNET client write data event.  */
        if  (events & NX_TELNET_SERVER_DATA)
        {

            /* Call processing to handle server data.  */
            _nx_telnet_server_data_process(server_ptr);
        }

        /* Check for a client disconnect event.  */
        if  (events & NX_TELNET_SERVER_DISCONNECT)
        {

            /* Call the disconnect processing.  */
            _nx_telnet_server_disconnect_process(server_ptr);
        }

        /* Check for a client activity timeout event.  */
        if  (events & NX_TELNET_SERVER_ACTIVITY_TIMEOUT)
        {

            /* Call the activity timeout processing.  */
            _nx_telnet_server_timeout_processing(server_ptr);
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_connect_process                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles all TELNET client connections received.       */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_server_socket_accept           Accept connection on socket   */ 
/*    nx_tcp_server_socket_relisten         Relisten for connection       */ 
/*    nx_tcp_server_socket_unaccept         Unaccept connection           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_thread_entry        TELNET server thread          */ 
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
VOID  _nx_telnet_server_connect_process(NX_TELNET_SERVER *server_ptr)
{

UINT                        i;
UINT                        status;
NX_TELNET_CLIENT_REQUEST    *client_req_ptr;


    /* One of the client request sockets is in the process of connection.  */

    /* Search the connections to see which one.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket was the one that is in being connected.  */
        if ((client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state > NX_TCP_CLOSED) &&
            (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state < NX_TCP_ESTABLISHED) &&
            (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_connect_port))
        {

            /* Yes, we have found the socket being connected.  */

            /* Increment the number of connection requests.  */
            server_ptr -> nx_telnet_server_connection_requests++;

            /* Attempt to accept on this socket.  */
            status = nx_tcp_server_socket_accept(&(client_req_ptr -> nx_telnet_client_request_socket), NX_TELNET_SERVER_TIMEOUT);

            /* Determine if it is successful.  */
            if (status)
            {

                /* Not successful, simply unaccept on this socket.  */
                nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_telnet_client_request_socket));
            }
            else
            {

                /* Reset the client request activity timeout.  */
                client_req_ptr -> nx_telnet_client_request_activity_timeout =  NX_TELNET_ACTIVITY_TIMEOUT;

                /* Update number of current open connections.  */
                server_ptr -> nx_telnet_server_open_connections++;

                /* Call the application's new connection callback routine.  */
                if (server_ptr -> nx_telnet_new_connection)
                {
                    /* Yes, there is a new connection callback routine - call it!  */
                    (server_ptr -> nx_telnet_new_connection)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection);
                }

                /* Disable remote echo by default. */
                if(server_ptr -> nx_telnet_set_echo)
                    server_ptr -> nx_telnet_set_echo(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, NX_FALSE);

#ifndef NX_TELNET_SERVER_OPTION_DISABLE

                /* Yes, send out server echo option requests. */
                status = _nx_telnet_server_send_option_requests(server_ptr, client_req_ptr);
                if(status != NX_SUCCESS)
                    return;
#endif /* NX_TELNET_SERVER_OPTION_DISABLE */

            }
        }
    }

    /* Now look for a socket that is closed to relisten on.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Set a pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket is closed.  */
        if (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
        {

            /* Relisten on this socket.  */
            status =  nx_tcp_server_socket_relisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT, 
                                                    &(client_req_ptr -> nx_telnet_client_request_socket));
            /* Check for bad status.  */
            if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
            {

                /* Increment the error count and keep trying.  */
                server_ptr -> nx_telnet_server_relisten_errors++;
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
/*    _nx_telnet_server_connection_present                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles all TELNET client connections received on     */ 
/*    the socket.                                                         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Socket event occurred         */ 
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
VOID  _nx_telnet_server_connection_present(NX_TCP_SOCKET *socket_ptr, UINT port)
{

NX_TELNET_SERVER   *server_ptr;

    NX_PARAMETER_NOT_USED(port);

    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the connect event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_SERVER_CONNECT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_disconnect_present                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles all TELNET client disconnections received on  */ 
/*    the socket.                                                         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Socket event occurred         */ 
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
VOID  _nx_telnet_server_disconnect_present(NX_TCP_SOCKET *socket_ptr)
{

NX_TELNET_SERVER   *server_ptr;

    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the disconnect event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_SERVER_DISCONNECT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_disconnect_process                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes all TELNET client disconnections received   */ 
/*    on the socket.                                                      */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_server_socket_relisten         Relisten on Telnet port       */ 
/*    nx_tcp_server_socket_unaccept         Unaccept connection           */ 
/*    nx_tcp_socket_disconnect              Disconnect socket             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_thread_entry        TELNET server thread          */ 
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
VOID  _nx_telnet_server_disconnect_process(NX_TELNET_SERVER *server_ptr)
{

UINT                        i;
UINT                        status;
NX_TELNET_CLIENT_REQUEST   *client_req_ptr;
UINT                        reset_client_request;


    /* Now look for a socket that has a disconnect state.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        reset_client_request = NX_FALSE;

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

       /* Has the socket received a RST packet? If so NetX will put it in a CLOSED or LISTEN state
          and the socket activity timeout has not been reset yet.  */
       if (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state < NX_TCP_SYN_SENT)            
       { 

            if (client_req_ptr -> nx_telnet_client_request_activity_timeout > 0)        
            {

                reset_client_request = NX_TRUE;
            }
       } 
       else
       {
       
            /* Now see if this socket has entered a disconnect state.  */
            while (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state > NX_TCP_ESTABLISHED) 
            {
    
                /* Yes, a disconnect is present, which signals an end of session for TELNET request.  */
    
                /* First, cleanup this socket.  */
                nx_tcp_socket_disconnect(&(client_req_ptr -> nx_telnet_client_request_socket), NX_TELNET_SERVER_TIMEOUT);    

                reset_client_request = NX_TRUE;
            }
       }

       /* If this connection is closed, update the telnet data and notify the application of a disconnect. */
       if (reset_client_request == NX_TRUE)
       {

           /* Unaccept this socket.  */
           nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_telnet_client_request_socket));

           /* Reset the client request activity timeout.  */
           client_req_ptr -> nx_telnet_client_request_activity_timeout =  0;

           /* Update number of current open connections. */
           if (server_ptr -> nx_telnet_server_open_connections > 0)
               server_ptr -> nx_telnet_server_open_connections--;

           /* Call the application's end connection callback routine.  */
           if (server_ptr -> nx_telnet_connection_end)
           {

               /* Yes, there is a connection end callback routine - call it!  */
               (server_ptr -> nx_telnet_connection_end)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection);
           }
       }
    }

    /* Now look for a socket that is closed to relisten on.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket is closed.  */
        if (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
        {

            /* Relisten on this socket.  */
            status =  nx_tcp_server_socket_relisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT, 
                                                    &(client_req_ptr -> nx_telnet_client_request_socket));
            /* Check for bad status.  */
            if ((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
            {

                /* Increment the error count and keep trying.  */
                server_ptr -> nx_telnet_server_relisten_errors++;
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
/*    _nx_telnet_server_data_present                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function notifies the TELNET server thread of data received    */ 
/*    from a client on the socket.                                        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    socket_ptr                            Socket event occurred         */ 
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
VOID  _nx_telnet_server_data_present(NX_TCP_SOCKET *socket_ptr)
{

NX_TELNET_SERVER   *server_ptr;

    /* Pickup server pointer.  This is setup in the reserved field of the TCP socket.  */
    server_ptr =  socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_SERVER_DATA, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_data_process                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes all TELNET client data packets received on  */ 
/*    the request socket.                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */
/*    nx_tcp_socket_receive                 Receive from socket           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_thread_entry        TELNET server thread          */ 
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
VOID  _nx_telnet_server_data_process(NX_TELNET_SERVER *server_ptr)
{

UINT                        i;
UINT                        status;
NX_PACKET                   *packet_ptr;
NX_TELNET_CLIENT_REQUEST    *client_req_ptr;
#ifndef NX_TELNET_SERVER_OPTION_DISABLE
UCHAR                       data_char; 
UINT                        offset;
#endif /* NX_TELNET_SERVER_OPTION_DISABLE */

    /* Now look for a socket that has receive data.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Setup pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket has data.  If so, process all of it now!  */
        while (client_req_ptr -> nx_telnet_client_request_socket.nx_tcp_socket_receive_queue_count)
        {

            /* Reset the client request activity timeout.  */
            client_req_ptr -> nx_telnet_client_request_activity_timeout =  NX_TELNET_ACTIVITY_TIMEOUT;

            /* Attempt to read a packet from this socket.  */
            status =  nx_tcp_socket_receive(&(client_req_ptr -> nx_telnet_client_request_socket), &packet_ptr, NX_NO_WAIT);
 
            /* Check for not data present.  */
            if (status != NX_SUCCESS)
            {

                /* Break to look at the next socket.  */
                break;
            }

#ifndef NX_TELNET_SERVER_OPTION_DISABLE

            /* If the first byte of the packet data is the telnet "IAC" code, 
            this is a telnet option packet.  */
            if (*packet_ptr -> nx_packet_prepend_ptr == NX_TELNET_IAC)
            {

#ifndef NX_DISABLE_PACKET_CHAIN
                if (packet_ptr -> nx_packet_next)
                {

                    /* Chained packet is not supported. */
                    nx_packet_release(packet_ptr);
                    break;
                }
#endif /* NX_DISABLE_PACKET_CHAIN */

                /* We will use an offset to mark the beginning of each telnet option, if there is more
                than one, in the packet payload. */
                offset = 0;

                /* Validate the packet length.  */
                if (packet_ptr -> nx_packet_length == 1)
                {
                    nx_packet_release(packet_ptr);
                    break;
                }

                /* Set the work pointer to just past the telnet IAC tag. */
                data_char = *(packet_ptr -> nx_packet_prepend_ptr + 1);

                /* Verify the next byte is a valid Telnet option code. */
                if ((data_char >= NX_TELNET_WILL) && (data_char <= NX_TELNET_DONT))
                {

                    /* Process the entire packet for telnet options, ensuring we don't go off the end of the payload. */
                    while((offset < packet_ptr -> nx_packet_length) && (*(packet_ptr -> nx_packet_prepend_ptr + offset) == NX_TELNET_IAC))
                    {

                        /* Process this telnet option. The offset will be updated to the location 
                        of the next option (if there is one) on return of this function. */
                        _nx_telnet_server_process_option(server_ptr, packet_ptr, &offset, client_req_ptr);
                    }
                }

                /* Are there any data left? */
                if ((offset < packet_ptr -> nx_packet_length) && (server_ptr -> nx_telnet_receive_data))
                {

                    /* Yes. Adjust packet. */
                    packet_ptr -> nx_packet_prepend_ptr += offset;
                    packet_ptr -> nx_packet_length -= offset;

                    /* Yes, there is a process data callback routine - call it!  */
                    (server_ptr -> nx_telnet_receive_data)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, packet_ptr);
                }
                else
                {

                    /* We're done with this packet. */
                    nx_packet_release(packet_ptr);
                }

                /* Check if the echo negotiation is successful.  */
                if((client_req_ptr -> nx_telnet_client_agree_server_will_SGA_success == NX_TRUE) && 
                    (client_req_ptr -> nx_telnet_client_agree_server_will_echo_success == NX_TRUE))
                {

                    /* Enable remote echo. */
                    if(server_ptr -> nx_telnet_set_echo)
                        server_ptr -> nx_telnet_set_echo(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, NX_TRUE);
                }
            }

            /* It's not an option packet.  */
            else
            {

                /* Check server receive callback.  */
                if (server_ptr -> nx_telnet_receive_data)
                {

                    /* Yes, there is a process data callback routine - call it!  */
                    (server_ptr -> nx_telnet_receive_data)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, packet_ptr);
                }
                else
                {

                    /* Error, no application callback routine.  */

                    /* Release the packet and continue the loop.  */
                    nx_packet_release(packet_ptr);
                }
            }
#else
           if (*packet_ptr -> nx_packet_prepend_ptr == NX_TELNET_IAC)
           {
                nx_packet_release(packet_ptr);
           }
           /* Call the server receive callback.  */
           else if (server_ptr -> nx_telnet_receive_data)
           {

               /* Yes, there is a process data callback routine - call it!  */
               (server_ptr -> nx_telnet_receive_data)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, packet_ptr);
           }
           else
           {

               /* Error, no application callback routine.  */

               /* Release the packet and continue the loop.  */
               nx_packet_release(packet_ptr);
           }

#endif /* NX_TELNET_SERVER_OPTION_DISABLE */
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_timeout                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the periodic timer for this TELNET server. Its     */ 
/*    duty is to inform the TELNET server that it is time to check for    */
/*    activity timeouts.                                                  */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    telnet_server_address                 TELNET server's address       */ 
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
VOID  _nx_telnet_server_timeout(ULONG telnet_server_address)
{

NX_TELNET_SERVER   *server_ptr;

    /* Pickup server pointer.  */
    server_ptr =  (NX_TELNET_SERVER *) telnet_server_address;

    /* Set the data event flag.  */
    tx_event_flags_set(&(server_ptr -> nx_telnet_server_event_flags), NX_TELNET_SERVER_ACTIVITY_TIMEOUT, TX_OR);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_timeout_processing                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reviews all the active TELNET client connections and  */ 
/*    looks for an activity timeout. If a connection has not had any      */ 
/*    activity within NX_TELNET_ACTIVITY_TIMEOUT seconds, the connection  */ 
/*    is deleted and its resources are made available to a new connection.*/ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_tcp_server_socket_relisten         Relisten for another connect  */ 
/*    nx_tcp_server_socket_unaccept         Unaccept server connection    */ 
/*    nx_tcp_socket_disconnect              Disconnect socket             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_thread_entry        TELNET server thread          */ 
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
VOID  _nx_telnet_server_timeout_processing(NX_TELNET_SERVER *server_ptr)
{

UINT                        i;
NX_TELNET_CLIENT_REQUEST   *client_req_ptr;


    /* Now look through all the sockets.  */
    for (i = 0; i < NX_TELNET_MAX_CLIENTS; i++)
    {

        /* Set a pointer to client request structure.  */
        client_req_ptr =  &(server_ptr -> nx_telnet_server_client_list[i]);

        /* Now see if this socket has an activity timeout active.  */
        if (client_req_ptr -> nx_telnet_client_request_activity_timeout)
        {

            /* Decrement the activity timeout for this client request.  */
            if (client_req_ptr -> nx_telnet_client_request_activity_timeout > NX_TELNET_TIMEOUT_PERIOD)
                client_req_ptr -> nx_telnet_client_request_activity_timeout =  client_req_ptr -> nx_telnet_client_request_activity_timeout - NX_TELNET_TIMEOUT_PERIOD;
            else
                client_req_ptr -> nx_telnet_client_request_activity_timeout =  0;

            /* Determine if this entry has exceeded the activity timeout.  */
            if (client_req_ptr -> nx_telnet_client_request_activity_timeout == 0)
            {

                /* Yes, the activity timeout has been exceeded.  Tear down and clean up the
                   entire client request structure.  */

                /* Increment the activity timeout counter.  */
                server_ptr -> nx_telnet_server_activity_timeouts++;

                /* Now disconnect the command socket.  */
                nx_tcp_socket_disconnect(&(client_req_ptr -> nx_telnet_client_request_socket), NX_NO_WAIT);

                /* Unaccept the server socket.  */
                nx_tcp_server_socket_unaccept(&(client_req_ptr -> nx_telnet_client_request_socket));

                /* Relisten on this socket. This will probably fail, but it is needed just in case all available
                   clients were in use at the time of the last relisten.  */
                nx_tcp_server_socket_relisten(server_ptr -> nx_telnet_server_ip_ptr, NX_TELNET_SERVER_PORT, 
                                                    &(client_req_ptr -> nx_telnet_client_request_socket));

                /* Update number of current open connections. */
                if (server_ptr -> nx_telnet_server_open_connections > 0)
                    server_ptr -> nx_telnet_server_open_connections--;

                /* Call the application's end connection callback routine.  */
                if (server_ptr -> nx_telnet_connection_end)
                {

                    /* Yes, there is a connection end callback routine - call it!  */
                    (server_ptr -> nx_telnet_connection_end)(server_ptr, client_req_ptr -> nx_telnet_client_request_connection);
                }
            }
        }
    }
}

#ifndef NX_TELNET_SERVER_OPTION_DISABLE

#ifdef NX_TELNET_SERVER_USER_CREATE_PACKET_POOL
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_packet_pool_set                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TELNET server create packet  */
/*    pool call.                                                          */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    pool_ptr                              Pointer to telnet packet pool */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_telnet_server_packet_pool_set     Actual set packet pool call   */ 
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

UINT _nxe_telnet_server_packet_pool_set(NX_TELNET_SERVER *server_ptr, NX_PACKET_POOL *pool_ptr)
{

UINT status;

    /* Check for invalid pointer input. */
    if ((server_ptr == NX_NULL) || (pool_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    /* Actual set packet pool service. */
    status = _nx_telnet_server_packet_pool_set(server_ptr, pool_ptr);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_telnet_server_packet_pool_set                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the Telnet Server packet pool to the packet pool */
/*    created outside the Telnet Server domain. This permits the packet   */
/*    pool memory to be in a different location from the Telnet Server.   */ 
/*                                                                        */ 
/*    The Telnet Server only uses this packet pool for sending Telnet     */
/*    options (requires NX_TELNET_SERVER_OPTION_DISABLE not be defined).  */
/*                                                                        */ 
/*    Note: This will overwrite an existing Telnet Server packet pool if  */
/*    one was previously set.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    pool_ptr                              Pointer to telnet packet pool */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Telnet server packet pool     */
/*                                            successfully set            */
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
UINT _nx_telnet_server_packet_pool_set(NX_TELNET_SERVER *server_ptr, NX_PACKET_POOL *pool_ptr)
{


    server_ptr -> nx_telnet_server_packet_pool_ptr = pool_ptr;

    return NX_SUCCESS;
}

#endif /* NX_TELNET_SERVER_USER_CREATE_PACKET_POOL */
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_create_option_packet              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function creates option packet for specified type and id.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    option_message_type                   Option type                   */ 
/*    option_id                             Option_id                     */ 
/*    stream                                Output buffer                 */ 
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
/*    _nx_telnet_server_send_option_requests       Send telnet option     */ 
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
VOID _nx_telnet_server_create_option_packet(UCHAR option_message_type, UCHAR option_id, UCHAR *stream)
{

    *(stream++) = NX_TELNET_IAC;
    *(stream++) = option_message_type;
    *stream = option_id;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_send_option_requests              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function is called if 1) the server is configured to send out   */
/*   options without waiting to receive client option requests, or 2)after*/
/*   receiving a client's initial option requests.  This will send out    */
/*   those option requests the server did not receive from the client     */
/*   based on the server session option list.                             */
/*                                                                        */ 
/*   This function obtains mutex protection on the client session record  */
/*   to update the record with options sent out from the server.          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                           Pointer to telnet server       */ 
/*    client_req_ptr                       Pointer to client record       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                               Completion status              */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                   Allocate packet from pool      */ 
/*    nx_packet_release                    Release packet back to pool    */
/*    nx_packet_data_append                Append data to packet payload  */
/*    _nx_telnet_server_create_option_packet                              */
/*                                         Create telnet option message   */
/*   _nx_telnet_server_packet_send         Send telnet option to client   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_data_process       Top level telnet packet handler*/
/*    _nx_telnet_server_connect_process    Telnet connection handler      */ 
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
UINT _nx_telnet_server_send_option_requests(NX_TELNET_SERVER *server_ptr, NX_TELNET_CLIENT_REQUEST *client_req_ptr)
{
UINT        status;
NX_PACKET   *packet_ptr;
UINT        option_length;
UINT        packet_available;
UCHAR       option_stream[9];

    status = NX_SUCCESS;

    /* Indicate packet not available, nor needing to be released. */
    packet_available = NX_FALSE;

    /* Allocate a packet for replies to send to this telnet client. */
    status =  nx_packet_allocate(server_ptr -> nx_telnet_server_packet_pool_ptr, &packet_ptr, NX_TCP_PACKET, NX_NO_WAIT);

    if (status != NX_SUCCESS)
        return status;

    /* Now a packet is available, and if not used should be released. */
    packet_available = NX_TRUE;

    /* Initialize option data to zero bytes. */
    option_length = 0;

    client_req_ptr -> nx_telnet_client_agree_server_will_echo_success = NX_FALSE;
    client_req_ptr -> nx_telnet_client_agree_server_will_SGA_success = NX_FALSE;

    /* Yes, create will echo request (3 bytes). */
    _nx_telnet_server_create_option_packet(NX_TELNET_WILL, NX_TELNET_ECHO, &option_stream[0]);

    /* Yes, create dont echo request (3 bytes). */
    _nx_telnet_server_create_option_packet(NX_TELNET_DONT, NX_TELNET_ECHO, &option_stream[3]);

    /* Yes, create will SGA request (3 bytes). */
    _nx_telnet_server_create_option_packet(NX_TELNET_WILL, NX_TELNET_SGA, &option_stream[6]);

    /* Update the the packet payload for number of bytes for a telnet option request. */
    option_length = 9;

    /* Add to the packet payload. */
    status = nx_packet_data_append(packet_ptr, option_stream, 9, server_ptr -> nx_telnet_server_packet_pool_ptr, NX_WAIT_FOREVER);

    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Check if we have a packet started, but not sent yet.  */
    if (option_length > 0)
    {

        /* Send the telnet packet out to the client. */
        status =  _nx_telnet_server_packet_send(server_ptr, client_req_ptr -> nx_telnet_client_request_connection, packet_ptr, 100);

        /* If errors sending, we need to release the packet. */
        if (status != NX_SUCCESS)
        {
            nx_packet_release(packet_ptr);
            return status;
        }

        /* Indicate we need another packet, just sent out the last one. */
        packet_available = NX_FALSE;
    }

    /* Check for unused packet (needs to be released). */
    if (packet_available == NX_TRUE)
    {

        /* Release the packet we did not use. */
        nx_packet_release(packet_ptr);
    }
    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_telnet_server_process_option                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function examines the telnet option in the current packet and  */
/*    determines if it is a new client request, an option with            */
/*    subnegotiation data for the server, or a response to a previously   */
/*    sent server telnet option request to the client. It then forwards   */ 
/*    the telnet option information to the appropriate telnet processor.  */
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to TELNET server      */ 
/*    packet_ptr                            Packet with telnet option     */ 
/*    offset                                Option offset in packet       */
/*    client_req_ptr                        Telnet client sending packet  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_TELNET_SESSION_OPTIONS_FULL        Session option list is fill   */ 
/*    NX_SUCCESS                            Option successfully processed */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*    _nx_telnet_server_update_server_session_attributes                  */ 
/*                                          Updates server telnet features*/
/*    _nx_telnet_server_update_client_session_attributes                  */ 
/*                                          Updates client telnet features*/
/*    _nx_telnet_server_process_new_option  Add new option to session list*/
/*    _nx_telnet_server_respond_to_pending_option                         */
/*                                          Send server reply to option   */
/*    _nx_telnet_server_process_subnegotiation_data                       */
/*                                          Process received option specs */
/*    _nx_telnet_server_validate_option_change                            */
/*                                          Determine if option is dupe   */
/*                                          Process received option specs */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_telnet_server_data_process        Top level incoming telnet     */
/*                                             packet handler             */ 
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

VOID _nx_telnet_server_process_option(NX_TELNET_SERVER *server_ptr, NX_PACKET *packet_ptr, UINT *offset,
                                      NX_TELNET_CLIENT_REQUEST *client_req_ptr)
{
UCHAR   data_char;
UCHAR   *work_ptr;

    NX_PARAMETER_NOT_USED(server_ptr);

    /* Set the work pointer to see what option received.  */
    work_ptr = (UCHAR *)(packet_ptr -> nx_packet_prepend_ptr + (*offset) + 2);

    /* This option isn't complete, just move to the end of the packet.  */
    if (work_ptr >= packet_ptr -> nx_packet_append_ptr)
    {
        *offset = packet_ptr -> nx_packet_length;
        return;
    }

    data_char = *work_ptr;

    /* If it is a echo option.  */
    if(data_char == NX_TELNET_ECHO)
    {

        /* Check whether the client replies with echo negotiation of the local echo disabled.  */
        work_ptr--;
        data_char = *work_ptr;
        if(data_char == NX_TELNET_DO)
        {

            /* Server will echo what received from the telnet connection.  */
            client_req_ptr -> nx_telnet_client_agree_server_will_echo_success = NX_TRUE;

            /* Move the offset to past the option.  */
            (*offset)+=3;
        }
        else 
        {

            /* Move the offset to past the option.  */
            (*offset)+=3;
        }
    }

    /* If it is a SGA option.  */
    else if(data_char == NX_TELNET_SGA)
    {

        /* Check whether the client replies with SGA negotiation.  */
        work_ptr--;
        data_char = *work_ptr;
        if(data_char == NX_TELNET_DO)
        {

            /* Server will enable SGA option.  */
            client_req_ptr -> nx_telnet_client_agree_server_will_SGA_success = NX_TRUE;

            /* Move the offset to past the option.  */
            (*offset)+=3;
        }
        else
        {
            /* Move the offset to past the option.  */
            (*offset)+=3;
            return;
        }
    }

    /* We have not implemented this option, just return.  */
    else
    {

        /* See next three bytes.  */
        (*offset)+=3;
        return;
    }
}
#endif /* NX_TELNET_SERVER_OPTION_DISABLE */
