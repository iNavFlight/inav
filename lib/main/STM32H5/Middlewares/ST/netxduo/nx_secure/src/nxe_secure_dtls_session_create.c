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


/* Include necessary system files.  */

#include "nx_secure_dtls.h"

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_dtls_session_create                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the DTLS session create call.    */
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
/*    _nx_secure_dtls_session_create        Actual DTLS session create    */
/*                                            call                        */
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
UINT _nxe_secure_dtls_session_create(NX_SECURE_DTLS_SESSION *session_ptr,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *metadata_buffer, ULONG metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT certs_number,
                                    UCHAR *remote_certificate_buffer, ULONG remote_certificate_buffer_size)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
NX_SECURE_DTLS_SESSION *created_dtls_session;
ULONG created_count;

    /* Check pointers. NOTE: Remote certificates number can be zero, so buffer can be NULL! */
    if ((session_ptr == NX_NULL) || (crypto_table == NX_NULL) ||
        (metadata_buffer == NX_NULL) ||
        ((packet_reassembly_buffer == NX_NULL) && (packet_reassembly_buffer_size != 0)))
    {
        return(NX_PTR_ERROR);
    }

    /* Loop to check for the DTLS session already created.  */
    created_dtls_session = _nx_secure_dtls_created_ptr;
    created_count = _nx_secure_dtls_created_count;
    while (created_count--)
    {

        /* Is the new DTLS already created?  */
        if (session_ptr == created_dtls_session)
        {

            /* Duplicate DTLS session created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_dtls_session = created_dtls_session -> nx_secure_dtls_created_next;
    }

    status = _nx_secure_dtls_session_create(session_ptr, crypto_table, metadata_buffer, metadata_size,
                                            packet_reassembly_buffer, packet_reassembly_buffer_size,
                                            certs_number, remote_certificate_buffer, remote_certificate_buffer_size);

    /* Return completion status.  */
    return(status);
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

