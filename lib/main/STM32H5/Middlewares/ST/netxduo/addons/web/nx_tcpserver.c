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
/**   Multiple TCP Socket/TLS Session support module                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_TCPSERVER_SOURCE_CODE

#include "nx_api.h"
#include "nx_tcpserver.h"


/* Define internal function prototypes. */
static UINT _nx_tcpserver_session_allocate(NX_TCPSERVER *server_ptr, NX_TCP_SESSION **session_pptr);
static UINT _nx_tcpserver_relisten(NX_TCPSERVER *server_ptr);
static VOID _nx_tcpserver_connect_present(NX_TCP_SOCKET *socket_ptr, UINT port);
static VOID _nx_tcpserver_data_present(NX_TCP_SOCKET *socket_ptr);
static VOID _nx_tcpserver_disconnect_present(NX_TCP_SOCKET *socket_ptr);
static VOID _nx_tcpserver_timeout(ULONG tcpserver_address);
static VOID _nx_tcpserver_connect_process(NX_TCPSERVER *server_ptr);
static VOID _nx_tcpserver_data_process(NX_TCPSERVER *server_ptr);
static VOID _nx_tcpserver_disconnect_process(NX_TCPSERVER *server_ptr);
static VOID _nx_tcpserver_timeout_process(NX_TCPSERVER *server_ptr);
static VOID _nx_tcpserver_thread_entry(ULONG tcpserver_address);


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_session_allocate                       PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function allocates a free socket (and TLS session, if */
/*    TLS is enabled) for an incoming client request.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to server structure   */
/*    session_pptr                          Pointer to allocated socket   */
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
/*    _nx_tcpserver_relisten                                              */
/*    _nx_tcpserver_start                                                 */
/*    _nx_tcpserver_connect_process                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed packet leak issue,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_tcpserver_session_allocate(NX_TCPSERVER *server_ptr, NX_TCP_SESSION **session_pptr)
{
UINT            i;
NX_TCP_SOCKET  *socket_ptr;

    /* Reset. */
    *session_pptr = NX_NULL;

    /* Loop to find unused session. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {

        /* Skip the listen session. */
        if(&server_ptr -> nx_tcpserver_sessions[i] == server_ptr -> nx_tcpserver_listen_session)
            continue;

        socket_ptr = &(server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket);

        /* A closed or listening socket is available. */
        if((socket_ptr -> nx_tcp_socket_state == NX_TCP_CLOSED) &&
           (socket_ptr -> nx_tcp_socket_bound_next == NX_NULL) &&
           (socket_ptr -> nx_tcp_socket_bind_in_progress == NX_NULL))
        {

            /* Reset expiration to zero. */
            server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_expiration = 0;

            /* Set connection flag to false. */
            server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_connected = NX_FALSE;

            /* Return the socket. */
            *session_pptr = &server_ptr -> nx_tcpserver_sessions[i];
            return NX_SUCCESS;
        }
    }

    return NX_TCPSERVER_FAIL;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_relisten                               PORTABLE C     */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked whenever a connection is closed   */
/*    to re-enable listening on that closed socket/session.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to server structure   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcpserver_session_allocate        Allocate socket for listening */
/*    nx_tcp_server_socket_relisten         Re-listen on free socket      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcpserver_connect_process                                       */
/*   _nx_tcpserver_disconnect_process                                     */
/*   _nx_tcpserver_timeout_process                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            removed debug output,       */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
static UINT _nx_tcpserver_relisten(NX_TCPSERVER *server_ptr)
{
UINT            status;
NX_TCP_SESSION *session_ptr;

    /* Is any socket listening? */
    if(server_ptr -> nx_tcpserver_listen_session == NX_NULL)
    {

        /* Get a socket to listen. */
        status = _nx_tcpserver_session_allocate(server_ptr, &session_ptr);
        if(status)
        {
            return NX_TCPSERVER_FAIL;
        }

        status = nx_tcp_server_socket_relisten(server_ptr -> nx_tcpserver_ip, 
                                               server_ptr -> nx_tcpserver_listen_port, 
                                               &session_ptr -> nx_tcp_session_socket);
        if((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
        {
            return NX_TCPSERVER_FAIL;
        }

        /* Store listen socket. */
        server_ptr -> nx_tcpserver_listen_session = session_ptr;
    }
    else if(server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
    {

        status = nx_tcp_server_socket_relisten(server_ptr -> nx_tcpserver_ip, 
                                               server_ptr -> nx_tcpserver_listen_port, 
                                               &server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket);
        if((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
        {
            return NX_TCPSERVER_FAIL;
        }
    }

    return NX_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_tcpserver_create                                  PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a socket server instance to handle multiple   */
/*    incoming TCP or TLS client connections. The server utilizes an      */
/*    internal thread and manages a queue of TCP sockets (and TLS         */
/*    sessions if enabled) that are used to handle incoming requests.     */
/*    The callback routines are used to handle individual requests. Each  */
/*    socket (and TLS session) receives identical state so that each      */
/*    incoming request is handled consistently.                           */
/*                                                                        */
/*    The number of available sockets and TLS sessions is determined by   */
/*    the size of the sessions buffer and can be found by dividing the    */
/*    buffer size by the size of the NX_TCP_SESSION structure.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance for this server   */
/*    server_ptr                            Pointer to server structure   */
/*    name                                  Name string for this server   */
/*    type_of_service                       Type of service for TCP       */
/*                                            sockets                     */
/*    fragment                              Flag to enable IP fragmenting */
/*    time_to_live                          Time to live value for socket */
/*    window_size                           Size of socket's receive      */
/*                                            window                      */
/*    new_connection                        Callback invoked for new      */
/*                                            client connections          */
/*    receive_data                          Callback invoked when data is */
/*                                            received from the client    */
/*    connection_end                        Callback invoked when a       */
/*                                            connection is closed        */
/*    connection_timeout                    Callback invoked when a       */
/*                                            connection times out        */
/*    timeout                               Timeout value for all sockets */
/*    stack_ptr                             Stack buffer for internal     */
/*                                            server thread               */
/*    stack_size                            Size of thread stack buffer   */
/*    sessions_buffer                       Buffer for per-session data   */
/*    buffer_size                           Size of per-session buffer,   */
/*                                            determines session number   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_create                     Create server thread           */
/*    tx_event_flags_create                Create thread event flags      */
/*    tx_timer_create                      Create timeout timer           */
/*    nx_tcp_socket_create                 Create TCP sockets             */
/*    nx_tcp_socket_receive_notify         Set TCP notification callback  */
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
UINT _nx_tcpserver_create(NX_IP *ip_ptr, NX_TCPSERVER *server_ptr, CHAR *name, 
                          ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                          VOID (*new_connection)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*receive_data)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*connection_end)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*connection_timeout)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          ULONG timeout, VOID *stack_ptr, UINT stack_size,
                          VOID *sessions_buffer, UINT buffer_size, UINT thread_priority, ULONG accept_wait_option)
{
UINT            i;
UINT            status;

    /* Initialize server struct. */
    server_ptr -> nx_tcpserver_ip = ip_ptr;
    server_ptr -> nx_tcpserver_sessions = sessions_buffer;
    server_ptr -> nx_tcpserver_sessions_count = buffer_size / sizeof(NX_TCP_SESSION);
    server_ptr -> nx_tcpserver_listen_port = 0;
    server_ptr -> nx_tcpserver_listen_session = NX_NULL;
    server_ptr -> nx_tcpserver_new_connection = new_connection;
    server_ptr -> nx_tcpserver_receive_data = receive_data;
    server_ptr -> nx_tcpserver_connection_end = connection_end;
    server_ptr -> nx_tcpserver_connection_timeout = connection_timeout;
    server_ptr -> nx_tcpserver_timeout = timeout;
    server_ptr -> nx_tcpserver_accept_wait_option = accept_wait_option;

    /* Create the tcpserver thread. */
    status = tx_thread_create(&server_ptr -> nx_tcpserver_thread, "TCPSERVER Thread",
                              _nx_tcpserver_thread_entry, (ULONG)server_ptr, stack_ptr,
                              stack_size, thread_priority, thread_priority, 
                              TX_NO_TIME_SLICE, TX_DONT_START);

    /* Create the tcpserver event flags. */
    status += tx_event_flags_create(&server_ptr -> nx_tcpserver_event_flags, "TCPSERVER Events");

    /* Create the timeout timer. */
    status += tx_timer_create(&server_ptr -> nx_tcpserver_timer, "TCPSERVER Timer",
                              _nx_tcpserver_timeout, (ULONG)server_ptr,
                              (NX_IP_PERIODIC_RATE * NX_TCPSERVER_TIMEOUT_PERIOD),
                              (NX_IP_PERIODIC_RATE * NX_TCPSERVER_TIMEOUT_PERIOD), TX_NO_ACTIVATE);

    /* Initialize buffer. */
    memset(sessions_buffer, 0, buffer_size);

    /* Initialize TCP sockets. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {
        status += nx_tcp_socket_create(ip_ptr, &server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket, name, type_of_service, fragment, time_to_live, 
                                       window_size, NX_NULL, _nx_tcpserver_disconnect_present);

        status += nx_tcp_socket_receive_notify(&server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket, _nx_tcpserver_data_present);

        server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket.nx_tcp_socket_reserved_ptr = server_ptr;
    }

    return status;
}


#ifdef NX_TCPSERVER_ENABLE_TLS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_tls_setup                              PORTABLE C     */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures a previously created NetX socket server    */
/*    instance to use TLS. The parameters are used to configure all the   */
/*    TLS sessions in the server with identical state so that each        */
/*    incoming TLS client experiences consistent behavior. The number of  */
/*    TLS sessions and TCP sockets is determined by the size of the       */
/*    sessions buffer passed into nx_tcpserver_create.                    */
/*                                                                        */
/*    The cryptographic routine table (ciphersuite table) is shared       */
/*    between all TLS sessions as it just contains function pointers.     */
/*                                                                        */
/*    The metadata buffer and packet reassembly buffer are divided        */
/*    equally between all TLS sessions. If the buffer size is not evenly  */
/*    divisible by the number of sessions the remainder will be unused.   */
/*                                                                        */
/*    The passed-in identity certificate is used by all sessions. During  */
/*    TLS operation the server identity certificate is only read from so  */
/*    copies are not needed for each session.                             */
/*                                                                        */
/*    The trusted certificates are added to the trusted store for each    */
/*    TLS session in the server. This is used for client certificate      */
/*    verification which is enabled if remote certificates are provided.  */
/*                                                                        */
/*    The remote certificate array and buffer is shared by default        */
/*    between all TLS sessions. This does mean that some sessions may     */
/*    block during certificate validation.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to control block      */
/*    crypto_table                          TLS cryptographic routines    */
/*    metadata_buffer                       Cryptographic metadata buffer */
/*    metadata_size                         Size of metadata buffer       */
/*    packet_buffer                         TLS packet buffer             */
/*    packet_size                           Size of packet buffer         */
/*    identity_certificate                  TLS server certificate        */
/*    trusted_certificates                  TLS trusted certificates      */
/*    trusted_certs_num                     Number of trusted certs       */
/*    remote_certificates                   Remote certificates array     */
/*    remote_certs_num                      Number of remote certificates */
/*    remote_certificate_buffer             Buffer for remote certs       */
/*    remote_cert_buffer_size               Size of remote cert buffer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_create          Create TLS Session            */
/*                                                                        */
/*    nx_secure_tls_session_packet_buffer_set                             */
/*                                          Set TLS packet buffer         */
/*                                                                        */
/*    nx_secure_tls_local_certificate_add                                 */
/*                                          Add local TLS identity cert   */
/*                                                                        */
/*    nx_secure_tls_session_client_verify_enable                          */
/*                                          Enable client cert verify     */
/*                                                                        */
/*    nx_secure_tls_trusted_certificate_add Add a trusted TLS cert        */
/*                                                                        */
/*    nx_secure_tls_remote_certificate_allocate                           */
/*                                          Allocate space for incoming   */
/*                                          client certificates           */
/*                                                                        */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the index of the  */
/*                                            remote certificate list,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

/* Setup TLS state per server session. */
UINT _nx_tcpserver_tls_setup(NX_TCPSERVER *server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                             VOID *metadata_buffer, ULONG metadata_size, UCHAR* packet_buffer, UINT packet_buffer_size, NX_SECURE_X509_CERT *identity_certificate,
                             NX_SECURE_X509_CERT *trusted_certificates[], UINT trusted_certs_num, NX_SECURE_X509_CERT *remote_certificates[], UINT remote_certs_num, UCHAR *remote_certificate_buffer, UINT remote_cert_buffer_size)
{
NX_SECURE_TLS_SESSION *tls_session;
ULONG session_metadata_size;
UCHAR *session_metadata;
UCHAR *session_cert_buffer;
UINT session_cert_buffer_size = 0;
UINT session_pkt_buffer_size;
UINT num_sessions;
UINT i;
UINT cert_count;
UINT status;

    /* Get the number of sessions in the TCP server. */
    num_sessions = server_ptr -> nx_tcpserver_sessions_count;

    /* Get our per-session packet buffer size. */
    session_pkt_buffer_size = packet_buffer_size / num_sessions;
    
    /* Get our per-session metadata. */
    session_metadata = metadata_buffer;
    session_metadata_size = metadata_size / num_sessions;

    /* It's OK to have zero remote certs allocated. */
    if(remote_certs_num > 0)
    {
        /* Divide the remote certificate buffer size by the total number of
           remote certificates to get per-certificate size. */
        session_cert_buffer_size = remote_cert_buffer_size / remote_certs_num;
    }
    
    /* Loop through all sessions, initialize each one. */
    for(i = 0; i < num_sessions; ++i)
    {
        /* This server instance is using TLS so mark as such. */
        server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_using_tls = NX_TRUE;
      
        /* Setup per-session data. Get the session from the server structure, then initialize it. */
        tls_session =  &(server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_tls_session);
        status = nx_secure_tls_session_create(tls_session, crypto_table, session_metadata, session_metadata_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Allocate space for packet reassembly. */
        status = nx_secure_tls_session_packet_buffer_set(tls_session, &packet_buffer[i * session_pkt_buffer_size], 
                                                         session_pkt_buffer_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        
        /* Add identity certificate. */
        status = nx_secure_tls_local_certificate_add(tls_session, identity_certificate);

        if(status != NX_SUCCESS)
        {
            return(status);
        }       
        
        /* See if remote certificates are provided. */
        if(remote_certs_num > 0)
        {
            /* We have remote certificates to allocate so enable client certificate authentication. */
            nx_secure_tls_session_client_verify_enable(tls_session);

            /* Add trusted certificates to each TLS session. */
            for(cert_count = 0; cert_count < trusted_certs_num; ++cert_count)
            {
                status = nx_secure_tls_trusted_certificate_add(tls_session, trusted_certificates[cert_count]);

                if(status != NX_SUCCESS)
                {
                    return(status);
                }
            }

            /* Allocate remote certificates (if remote_certs_num > 0). The remote certificate
               buffers are shared between all TLS Sessions. This is possible since they are only
               needed for the certificate verification process and are freed immediately after. */
            session_cert_buffer = remote_certificate_buffer;

            for(cert_count = 0; cert_count < remote_certs_num; ++cert_count)
            {
                /* Allocate a remote certificate from the provided array. */
                status = nx_secure_tls_remote_certificate_allocate(tls_session, remote_certificates[cert_count],
                                                                   session_cert_buffer, session_cert_buffer_size);

                if(status != NX_SUCCESS)
                {
                    return(status);
                }

                /* Now get the next certificate and its buffer. */
                session_cert_buffer += session_cert_buffer_size;
            }
        }

        /* Advance metadata buffer. */
        session_metadata = &session_metadata[session_metadata_size];
    }
    return(NX_SUCCESS);
}

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_tls_ecc_setup                         PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures supported curve lists for NetX socket      */
/*    server instance using TLS.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to control block      */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_ecc_initialize         Initializes curve lists for TLS*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
UINT _nx_tcpserver_tls_ecc_setup(NX_TCPSERVER *server_ptr,
                                 const USHORT *supported_groups, USHORT supported_group_count,
                                 const NX_CRYPTO_METHOD **curves)
{
UINT i;
UINT status;
NX_SECURE_TLS_SESSION* tls_session;

    /* Loop through all sessions, initialize each one. */
    for (i = 0; i < server_ptr -> nx_tcpserver_sessions_count; ++i)
    {

        tls_session = &(server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_tls_session);
        status = nx_secure_tls_ecc_initialize(tls_session, supported_groups, supported_group_count, curves);
        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_start                                  PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by an application to start a socket server  */
/*    that was previously created and configured. This routine starts the */
/*    internal server thread and timer.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to server structure   */
/*    port                                  Port for server to listen on  */
/*    listen_queue_size                     Maximum number of TCP         */
/*                                            connections queued          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcpserver_session_allocate        Allocate socket for listening */
/*    nx_tcp_server_socket_listen           Listen on free TCP socket     */
/*    tx_timer_activate                     Start timeout timer           */
/*    tx_thread_resume                      Start server thread           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application code                                                     */
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
UINT _nx_tcpserver_start(NX_TCPSERVER *server_ptr, UINT port, UINT listen_queue_size)
{
UINT            status;
NX_TCP_SESSION *session_ptr;

    /* Check whether listen is done. */
    if(server_ptr -> nx_tcpserver_listen_port)
        return NX_TCPSERVER_FAIL;

    /* Get a socket to listen. */
    status = _nx_tcpserver_session_allocate(server_ptr, &session_ptr);
    if(status)
        return status;

    /* Listen on port. */
    status = nx_tcp_server_socket_listen(server_ptr -> nx_tcpserver_ip, port, &session_ptr -> nx_tcp_session_socket, listen_queue_size, _nx_tcpserver_connect_present);
    if(status)
        return status;

    /* Store socket and port. */
    server_ptr -> nx_tcpserver_listen_session = session_ptr;
    server_ptr -> nx_tcpserver_listen_port = port;

    /* Activate timer. */
    tx_timer_activate(&server_ptr -> nx_tcpserver_timer);

    /* Start thread. */
    tx_thread_resume(&server_ptr -> nx_tcpserver_thread);

    return NX_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_connect_present                        PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is used for the TCP listen callback passed   */
/*    into nx_tcp_server_socket_listen and is used to determine when an   */
/*    incoming request has been received and is ready for processing.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*    port                                  Port of incoming request      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set thread event flag         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   TCP stack                                                            */
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
static VOID _nx_tcpserver_connect_present(NX_TCP_SOCKET *socket_ptr, UINT port)
{
NX_TCPSERVER *server_ptr = socket_ptr -> nx_tcp_socket_reserved_ptr;


    NX_PARAMETER_NOT_USED(port);

    /* Set the connect event flag. */
    tx_event_flags_set(&server_ptr -> nx_tcpserver_event_flags, NX_TCPSERVER_CONNECT, TX_OR);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_data_present                           PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is used for the TCP receive callback passed  */
/*    into nx_tcp_socket_receive_notify and is used to determine when     */
/*    data has been received and is ready for processing.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set thread event flag         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   TCP stack                                                            */
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
static VOID _nx_tcpserver_data_present(NX_TCP_SOCKET *socket_ptr)
{
NX_TCPSERVER *server_ptr = socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the data event flag. */
    tx_event_flags_set(&server_ptr -> nx_tcpserver_event_flags, NX_TCPSERVER_DATA, TX_OR);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_disconnect_present                     PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is used for the TCP disconnect callback      */
/*    passed into nx_tcp_socket_create and is used to determine when      */
/*    a socket has been disconnected.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set thread event flag         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   TCP stack                                                            */
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
static VOID _nx_tcpserver_disconnect_present(NX_TCP_SOCKET *socket_ptr)
{
NX_TCPSERVER *server_ptr = socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the disconnect event flag. */
    tx_event_flags_set(&server_ptr -> nx_tcpserver_event_flags, NX_TCPSERVER_DISCONNECT, TX_OR);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_timeout                                PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked whenever the internal timeout     */
/*    timer expires, and is passed into tx_timer_create as the callback.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcpserver_address                     Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_set                    Set thread event flag         */
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
static VOID _nx_tcpserver_timeout(ULONG tcpserver_address)
{
NX_TCPSERVER *server_ptr = (NX_TCPSERVER *)tcpserver_address;

    /* Set the timeout event flag. */
    tx_event_flags_set(&server_ptr -> nx_tcpserver_event_flags, NX_TCPSERVER_TIMEOUT, TX_OR);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_connect_process                        PORTABLE C     */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked by the server thread whenever an  */
/*    incoming connection request is received. It establishes the TCP     */
/*    connection and if TLS is enabled, it begins the TLS handshake.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcpserver_session_allocate        Get socket for listening      */
/*    nx_tcp_server_socket_relisten         Relisten on existing socket   */
/*    nx_tcp_server_socket_accept           Accept incoming TCP request   */
/*    nx_secure_tls_session_start           Start TLS handshake           */
/*    nx_tcp_server_socket_unaccept         Clear accepted socket         */
/*    _nx_tcpserver_relisten                Re-listen on all sockets      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcpserver_thread_entry                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed packet leak issue,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed TLS connection        */
/*                                            deadlock issue,             */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the wait option   */
/*                                            of TLS connection,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static VOID _nx_tcpserver_connect_process(NX_TCPSERVER *server_ptr)
{
UINT            status;
NX_TCP_SESSION *session_ptr = NX_NULL;

    /* Is any socket listening? */
    if(server_ptr -> nx_tcpserver_listen_session == NX_NULL)
    {

        /* No. Check whether listen is done. */
        if(server_ptr -> nx_tcpserver_listen_port == 0)
        {
            return;
        }

        /* Get a socket to listen. */
        status = _nx_tcpserver_session_allocate(server_ptr, &session_ptr);
        if(status)
        {
            return;
        }

        status = nx_tcp_server_socket_relisten(server_ptr -> nx_tcpserver_ip, 
                                               server_ptr -> nx_tcpserver_listen_port, 
                                               &session_ptr -> nx_tcp_session_socket);
        if((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
        {
            return;
        }

        /* Store listen socket. */
        server_ptr -> nx_tcpserver_listen_session = session_ptr;
    }
    else if(server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
    {

        status = nx_tcp_server_socket_relisten(server_ptr -> nx_tcpserver_ip, 
                                               server_ptr -> nx_tcpserver_listen_port, 
                                               &session_ptr -> nx_tcp_session_socket);
        if((status != NX_SUCCESS) && (status != NX_CONNECTION_PENDING))
        {
            return;
        }
    }

    /* If session is connected, just return. */
    if (server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_connected)
    {
        return;
    }

    /* Accept connection. */
    status = nx_tcp_server_socket_accept(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket, server_ptr -> nx_tcpserver_accept_wait_option);

    if(status == NX_SUCCESS)
    {
        
#ifdef NX_TCPSERVER_ENABLE_TLS
        /* If TLS, start the TLS handshake. */
        if(server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_using_tls == NX_TRUE)
        {
            status = nx_secure_tls_session_start(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_tls_session, 
                                                 &server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket, server_ptr -> nx_tcpserver_timeout * NX_IP_PERIODIC_RATE);
            
            if(status != NX_SUCCESS)
            {
                nx_secure_tls_session_end(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_tls_session, NX_WAIT_FOREVER);
                nx_tcp_socket_disconnect(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket, NX_NO_WAIT);
                nx_tcp_server_socket_unaccept(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket);
            }
        }

        if (status == NX_SUCCESS)
#endif
        {

            /* Set default expiration. */
            server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_expiration = server_ptr -> nx_tcpserver_timeout;

            if(server_ptr -> nx_tcpserver_new_connection)
            {

                /* Invoke new connection callback. */
                server_ptr -> nx_tcpserver_new_connection(server_ptr, server_ptr -> nx_tcpserver_listen_session);
            }

            /* Set connection flag to true. */
            server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_connected = NX_TRUE;

            /* Clear listen socket. */
            server_ptr -> nx_tcpserver_listen_session = NX_NULL;
        }
    }
    else
    {
        nx_tcp_server_socket_unaccept(&server_ptr -> nx_tcpserver_listen_session -> nx_tcp_session_socket);
    }

    /* Relisten */
    _nx_tcpserver_relisten(server_ptr);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_data_process                           PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked by the server thread whenever     */
/*    data is received from the remote client. If the application passes  */
/*    a receive callback to _nx_tcpserver_create, it will be invoked here.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_receive_data             Callback to process data      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcpserver_thread_entry                                          */
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
static VOID _nx_tcpserver_data_process(NX_TCPSERVER *server_ptr)
{
UINT            i;
NX_TCP_SOCKET  *socket_ptr;

    /* Do nothing if callback is not set. */
    if(server_ptr -> nx_tcpserver_receive_data == NX_NULL)
    {
        return;
    }

    /* Initialize TCP sockets. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {
        socket_ptr = &(server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket);

        if(socket_ptr -> nx_tcp_socket_receive_queue_count)
        {

            /* Reset default expiration. */
            server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_expiration = server_ptr -> nx_tcpserver_timeout;

            /* Invoke receive data callback. */
            server_ptr -> nx_tcpserver_receive_data(server_ptr, &server_ptr -> nx_tcpserver_sessions[i]);

            /* Relisten */
            _nx_tcpserver_relisten(server_ptr);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_disconnect_process                     PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked by the server thread whenever     */
/*    a socket is disconnected. If the application passes  a disconnect   */
/*    callback to _nx_tcpserver_create, it will be invoked here.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_connection_end           Callback for disconnections   */
/*    _nx_tcpserver_relisten                Re-listen on free sockets     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcpserver_thread_entry                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed packet leak issue,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_tcpserver_disconnect_process(NX_TCPSERVER *server_ptr)
{
UINT            i;
NX_TCP_SOCKET  *socket_ptr;

    /* Do nothing if callback is not set. */
    if(server_ptr -> nx_tcpserver_connection_end == NX_NULL)
    {
        return;
    }
    
    /* Initialize TCP sockets. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {
        socket_ptr = &(server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket);

        if((socket_ptr -> nx_tcp_socket_state > NX_TCP_ESTABLISHED) ||
           ((socket_ptr -> nx_tcp_socket_state < NX_TCP_SYN_SENT) && 
             server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_expiration))
        {

            /* Invoke disconnect callback. */
            server_ptr -> nx_tcpserver_connection_end(server_ptr, &server_ptr -> nx_tcpserver_sessions[i]);

            /* Reset epiration of session. */
            server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_expiration = 0; 

            /* Set connection flag to false. */
            server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_connected = NX_FALSE;

            /* Relisten */
            _nx_tcpserver_relisten(server_ptr);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_timeout_process                        PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is invoked by the server thread whenever     */
/*    a timeout event is encountered (triggered by the timeout timer).    */
/*    If the application passes  a disconnect callback to                 */
/*    _nx_tcpserver_create, it will be invoked here.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_connection_timeout       Callback to process timeout   */
/*    _nx_tcpserver_relisten                Re-listen on free sockets     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcpserver_thread_entry                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed packet leak issue,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static VOID _nx_tcpserver_timeout_process(NX_TCPSERVER *server_ptr)
{
UINT            i;
NX_TCP_SESSION *session_ptr;

    /* Do nothing if callback is not set. */
    if(server_ptr -> nx_tcpserver_connection_timeout == NX_NULL)
    {
        return;
    }
    
    /* Initialize TCP sockets. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {
        session_ptr = &server_ptr -> nx_tcpserver_sessions[i];

        /* Skip socket that is not used. */
        if(session_ptr -> nx_tcp_session_socket.nx_tcp_socket_state == NX_TCP_CLOSED)
        {
            continue;
        }

        /* Skip socket that is already timeout. */
        if(session_ptr -> nx_tcp_session_expiration == 0)
        {
            continue;
        }

        /* Is the session timeout? */
        if(session_ptr -> nx_tcp_session_expiration > NX_TCPSERVER_TIMEOUT_PERIOD)
            session_ptr -> nx_tcp_session_expiration -= NX_TCPSERVER_TIMEOUT_PERIOD;
        else 
        {

            session_ptr -> nx_tcp_session_expiration = 0;

            /* Invoke timeout callback. */
            server_ptr -> nx_tcpserver_connection_timeout(server_ptr, &server_ptr -> nx_tcpserver_sessions[i]);

            /* Set connection flag to false. */
            session_ptr -> nx_tcp_session_connected = NX_FALSE;

            /* Relisten */
            _nx_tcpserver_relisten(server_ptr);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_thread_entry                           PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function is the entry point for the socket server     */
/*    thread. It continually loops, checking the event flags and          */
/*    dispatching processing when events are encountered.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcpserver_address                     Pointer to socket server      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcpserver_connect_process         Process new connection        */
/*    _nx_tcpserver_data_process            Process received data         */
/*    _nx_tcpserver_disconnect_process      Process disconnection event   */
/*    _nx_tcpserver_timeout_process         Process server timeout        */
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
static VOID _nx_tcpserver_thread_entry(ULONG tcpserver_address)
{
ULONG           events;
UINT            status;
NX_TCPSERVER   *server_ptr = (NX_TCPSERVER *)tcpserver_address;


    /* Loop to process events. */
    while(1)
    {
        status = tx_event_flags_get(&server_ptr -> nx_tcpserver_event_flags, NX_TCPSERVER_ANY_EVENT, TX_OR_CLEAR, &events, TX_WAIT_FOREVER);

        /* Check the return status. */
        if(status)
        {

            /* If an error occurs, simply continue the loop. */
            continue;
        }

        /* Otherwise, an event is present. Process according to the event. */

        /* Check for a connect event. */
        if(events & NX_TCPSERVER_CONNECT)
        {

            /* Call the connect processing. */
            _nx_tcpserver_connect_process(server_ptr);
        }


        /* Check for a data event. */
        if(events & NX_TCPSERVER_DATA)
        {

            /* Call the receive data processing. */
            _nx_tcpserver_data_process(server_ptr);
        }


        /* Check for a disconnect event. */
        if(events & NX_TCPSERVER_DISCONNECT)
        {

            /* Call the disconnect processing. */
            _nx_tcpserver_disconnect_process(server_ptr);
        }


        /* Check for a timeout event. */
        if(events & NX_TCPSERVER_TIMEOUT)
        {

            /* Call the disconnect processing. */
            _nx_tcpserver_timeout_process(server_ptr);
        }

    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_stop                                   PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by an application to stop a currently       */
/*    running socket server. It shuts down the server thread and stops    */
/*    listening on all internal TCP sockets.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to server structure   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_suspend                     Stop server thread            */
/*    tx_timer_deactivate                   Stop server timeout timer     */
/*    nx_tcp_server_socket_unlisten         Stop listening on TCP socket  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application code                                                     */
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
UINT _nx_tcpserver_stop(NX_TCPSERVER *server_ptr)
{
UINT status;

    /* Check whether listen is done. */
    if(server_ptr -> nx_tcpserver_listen_port == 0)
        return NX_TCPSERVER_FAIL;

    /* Suspend thread. */
    tx_thread_suspend(&server_ptr -> nx_tcpserver_thread);

    /* Deactivate timer. */
    tx_timer_deactivate(&server_ptr -> nx_tcpserver_timer);

    /* Unlisten. */
    status = nx_tcp_server_socket_unlisten(server_ptr -> nx_tcpserver_ip, server_ptr -> nx_tcpserver_listen_port);

    if(status == NX_SUCCESS)
    {

        /* Clear. */ 
        server_ptr -> nx_tcpserver_listen_port = 0;
        server_ptr -> nx_tcpserver_listen_session = NX_NULL;
    }

    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcpserver_delete                                 PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by an application to clean up a previously  */
/*    created socket server. It cleans up the server thread and closes    */
/*    deletes all internal TCP sockets and TLS sessions.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to server structure   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_terminate                   Stop server thread            */
/*    tx_thread_delete                      Stop server timeout timer     */
/*    tx_event_flags_delete                 Delete server event flags     */
/*    tx_timer_deactivate                   Deactivate timeout timer      */
/*    tx_timer_delete                       Delete timeout timer          */
/*    nx_tcp_socket_disconnect              Disconnect TCP sockets        */
/*    nx_tcp_server_socket_unaccept         Stop listening on TCP sockets */
/*    nx_tcp_socket_delete                  Delete TCP socket             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*   Application code                                                     */
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
UINT _nx_tcpserver_delete(NX_TCPSERVER *server_ptr)
{
UINT            i;
NX_TCP_SOCKET  *socket_ptr;

    /* Terminate server thread. */
    tx_thread_terminate(&server_ptr -> nx_tcpserver_thread);

    /* Delete server thread. */
    tx_thread_delete(&server_ptr -> nx_tcpserver_thread);

    /* Delete the event flag group */
    tx_event_flags_delete(&server_ptr -> nx_tcpserver_event_flags);

    /* Deactivate and delete timer. */
    tx_timer_deactivate(&server_ptr -> nx_tcpserver_timer);
    tx_timer_delete(&server_ptr -> nx_tcpserver_timer);

    /* Delete all opened sockets. */
    for(i = 0; i < server_ptr -> nx_tcpserver_sessions_count; i++)
    {

        /* Get socket pointer. */
        socket_ptr = &server_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_socket;

        /* Disconnect the socket. */
        nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);

        /* unaccept the socket. */
        nx_tcp_server_socket_unaccept(socket_ptr);

        /* Delete the socket. */
        nx_tcp_socket_delete(socket_ptr);
    }

    /* Return successful compeletion. */
    return NX_SUCCESS;
}
