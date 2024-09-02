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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_dtls_server_create                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when creating a DTLS Server.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  Server port                   */
/*    timeout                               Timeout value                 */
/*    session_buffer                        DTLS sessions buffer          */
/*    session_buffer_size                   Size of DTLS sessions buffer  */
/*    crypto_table                          Crypto table                  */
/*    crypto_metadata_buffer                Encryption metadata buffer    */
/*    crypto_metadata_size                  Encryption metadata size      */
/*    packet_reassembly_buffer              DTLS reassembly buffer        */
/*    packet_reassembly_buffer_size         Size of reassembly buffer     */
/*    connect_notify                        Callback for new connections  */
/*    receive_notify                        Callback for received data    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_server_create         Actual function call          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_dtls_server_create(NX_SECURE_DTLS_SERVER *server_ptr, NX_IP *ip_ptr, UINT port, ULONG timeout,
                                    VOID *session_buffer, UINT session_buffer_size,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *crypto_metadata_buffer, ULONG crypto_metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT (*connect_notify)(NX_SECURE_DTLS_SESSION *dtls_session, NXD_ADDRESS *ip_address, UINT port),
                                    UINT (*receive_notify)(NX_SECURE_DTLS_SESSION *dtls_session))
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
UINT i;
UINT num_sessions;
NX_SECURE_DTLS_SESSION *current_session;
NX_SECURE_DTLS_SESSION *created_dtls_session;
NX_SECURE_DTLS_SERVER *created_dtls_server;
ULONG created_count;


    /* Figure out number of sessions. */
    num_sessions = session_buffer_size / sizeof(NX_SECURE_DTLS_SESSION);

    /* Check pointers. */
    if(server_ptr == NX_NULL || ip_ptr == NX_NULL || session_buffer == NX_NULL ||
       crypto_metadata_buffer == NX_NULL || packet_reassembly_buffer == NX_NULL ||
       connect_notify == NX_NULL || receive_notify == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Loop to check for the DTLS server already created.  */
    created_dtls_server = _nx_secure_dtls_server_created_ptr;
    created_count = _nx_secure_dtls_server_created_count;
    while (created_count--)
    {

        /* Is the new DTLS server already created?  */
        if (server_ptr == created_dtls_server)
        {

            /* Duplicate DTLS server created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_dtls_server = created_dtls_server -> nx_dtls_server_created_next;
    }

    /* Check all the sessions.  */
    for(i = 0; i < num_sessions; ++i)
    {

        /* Get the current session. */
        current_session = &((NX_SECURE_DTLS_SESSION *)session_buffer)[i];

        /* Loop to check for the DTLS session already created.  */
        created_dtls_session = _nx_secure_dtls_created_ptr;
        created_count = _nx_secure_dtls_created_count;
        while (created_count--)
        {

            /* Is the new DTLS already created?  */
            if (current_session == created_dtls_session)
            {

                /* Duplicate DTLS session created, return an error!  */
                return(NX_PTR_ERROR);
            }

            /* Move to next entry.  */
            created_dtls_session = created_dtls_session -> nx_secure_dtls_created_next;
        }
    }

    /* Call the actual function. */
    status = _nx_secure_dtls_server_create(server_ptr, ip_ptr, port, timeout,
                                           session_buffer, session_buffer_size, crypto_table,
                                           crypto_metadata_buffer, crypto_metadata_size,
                                           packet_reassembly_buffer, packet_reassembly_buffer_size,
                                           connect_notify, receive_notify);

    return(status);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(port);
    NX_PARAMETER_NOT_USED(timeout);
    NX_PARAMETER_NOT_USED(session_buffer);
    NX_PARAMETER_NOT_USED(session_buffer_size);
    NX_PARAMETER_NOT_USED(crypto_table);
    NX_PARAMETER_NOT_USED(crypto_metadata_buffer);
    NX_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer_size);
    NX_PARAMETER_NOT_USED(connect_notify);
    NX_PARAMETER_NOT_USED(receive_notify);


    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

