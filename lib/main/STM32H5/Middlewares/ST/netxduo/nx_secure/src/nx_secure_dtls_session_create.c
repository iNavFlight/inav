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
/*    _nx_secure_dtls_session_create                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes a DTLS session control block for later    */
/*    use in establishing a secure DTLS session over a UDP socket or      */
/*    other lower-level networking protocol.                              */
/*                                                                        */
/*    To calculate the necessary metadata size, the API                   */
/*    nx_secure_tls_metadata_size_calculate may be used.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           DTLS session control block    */
/*    crypto_table                          Crypto table                  */
/*    metadata_buffer                       Encryption metadata buffer    */
/*    metadata_size                         Encryption metadata size      */
/*    packet_reassembly_buffer              DTLS reassembly buffer        */
/*    packet_reassembly_buffer_size         Size of reassembly buffer     */
/*    certs_number                          Number of certs               */
/*    remote_certificate_buffer             Remote certificate buffer     */
/*    remote_certificate_buffer_size        Remote certificate buffer size*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_create         Initialize TLS control block  */
/*    _nx_secure_tls_remote_certificate_buffer_allocate                   */
/*                                          Allocate space for remote     */
/*                                            certificate                 */
/*    _nxe_secure_tls_session_packet_buffer_set                           */
/*                                          Allocate space for packet     */
/*                                            reassembly                  */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT _nx_secure_dtls_session_create(NX_SECURE_DTLS_SESSION *session_ptr,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *metadata_buffer, ULONG metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT certs_number,
                                    UCHAR *remote_certificate_buffer, ULONG remote_certificate_buffer_size)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT                    status;
NX_SECURE_TLS_SESSION  *tls_session;
NX_SECURE_DTLS_SESSION *tail_ptr;

    NX_SECURE_MEMSET(session_ptr, 0, sizeof(NX_SECURE_DTLS_SESSION));

    /* Get a working pointer to the internal TLS control block. */
    tls_session = &session_ptr -> nx_secure_dtls_tls_session;

    /* Initialize the TLS session. Nothing specific to DTLS is needed in this function. */
    status = _nx_secure_tls_session_create(tls_session, crypto_table, metadata_buffer, metadata_size);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Don't allocate space if we don't have any certificates. Mostly for internal
       API calls when creating DTLS server sessions. */
    if(certs_number > 0)
    {
        /* Allocate buffer space for incoming certificate chains. */
        status = _nx_secure_tls_remote_certificate_buffer_allocate(tls_session, certs_number,
                                                                   remote_certificate_buffer, remote_certificate_buffer_size);

        if(status != NX_SUCCESS)
        {
            _nx_secure_tls_session_delete(tls_session);
            return(status);
        }
    }

    /* Allocate space for packet re-assembly. */
    status = _nx_secure_tls_session_packet_buffer_set(tls_session, packet_reassembly_buffer, packet_reassembly_buffer_size);

    if (status)
    {

        _nx_secure_tls_session_delete(tls_session);
        return(status);
    }

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Place the new DTLS control block on the list of created DTLS. */
    if (_nx_secure_dtls_created_ptr)
    {

        /* Pickup tail pointer. */
        tail_ptr = _nx_secure_dtls_created_ptr -> nx_secure_dtls_created_previous;

        /* Place the new DTLS control block in the list. */
        _nx_secure_dtls_created_ptr -> nx_secure_dtls_created_previous = session_ptr;
        tail_ptr -> nx_secure_dtls_created_next = session_ptr;

        /* Setup this DTLS's created links. */
        session_ptr -> nx_secure_dtls_created_previous = tail_ptr;
        session_ptr -> nx_secure_dtls_created_next = _nx_secure_dtls_created_ptr;
    }
    else
    {

        /* The created DTLS list is empty. Add DTLS control block to empty list. */
        _nx_secure_dtls_created_ptr = session_ptr;
        session_ptr -> nx_secure_dtls_created_previous = session_ptr;
        session_ptr -> nx_secure_dtls_created_next = session_ptr;
    }
    _nx_secure_dtls_created_count++;

    /* Reset the local IP address index to 0xffffffff.  */
    session_ptr -> nx_secure_dtls_local_ip_address_index = 0xffffffff;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(session_ptr);
    NX_PARAMETER_NOT_USED(crypto_table);
    NX_PARAMETER_NOT_USED(metadata_buffer);
    NX_PARAMETER_NOT_USED(metadata_size);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer);
    NX_PARAMETER_NOT_USED(packet_reassembly_buffer_size);
    NX_PARAMETER_NOT_USED(certs_number);
    NX_PARAMETER_NOT_USED(remote_certificate_buffer);
    NX_PARAMETER_NOT_USED(remote_certificate_buffer_size);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

