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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_create                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes a TLS session control block for later     */
/*    use in establishing a secure TLS session over a TCP socket or       */
/*    other lower-level networking protocol.                              */
/*                                                                        */
/*    To calculate the necessary metadata size, the API                   */
/*    nx_secure_tls_metadata_size_calculate may be used.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session control block     */
/*    crypto_table                          crypto method table           */
/*    metadata_buffer                       Encryption metadata area      */
/*    metadata_size                         Encryption metadata size      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_reset          Clear out the session         */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_create        Create DTLS session           */
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
UINT _nx_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                   const NX_SECURE_TLS_CRYPTO *crypto_table,
                                   VOID *metadata_buffer,
                                   ULONG metadata_size)
{
    UINT                            status;

    NX_SECURE_MEMSET(session_ptr, 0, sizeof(NX_SECURE_TLS_SESSION));

    /* Assign the table to the session. */
    /* Cast away "const" for new API. */
    session_ptr -> nx_secure_tls_crypto_table = (NX_SECURE_TLS_CRYPTO *)(crypto_table);

    status = _nx_secure_tls_session_create_ext(session_ptr, NX_NULL, 0, NX_NULL, 0, metadata_buffer, metadata_size);

    return(status);
}

