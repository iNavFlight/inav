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
/*    _nx_secure_tls_session_iv_size_get                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the size in bytes needed by the current       */
/*    TLS session cipher (e.g. AES) if the encrypted session is active.   */
/*    If the session is not encrypted (e.g. during initial handshake) or  */
/*    the cipher does not have an inlined IV, 0 is returned.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    iv_size                               Size of IV in bytes           */
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
/*    _nx_secure_dtls_packet_allocate       Allocate internal DTLS packet */
/*    _nx_secure_dtls_send_record           Send the DTLS record          */
/*    _nx_secure_tls_packet_allocate        Allocate internal TLS packet  */
/*    _nx_secure_tls_send_record            Send TLS records              */
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
UINT _nx_secure_tls_session_iv_size_get(NX_SECURE_TLS_SESSION *tls_session, USHORT *iv_size)
{
const NX_CRYPTO_METHOD               *session_cipher_method;
UINT                                  algorithm;


    /* If TLS session is active, allocate space for the IV that precedes the data in
       certain ciphersuites. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
        if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {

            /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
            return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
        }

        /* Select the encryption algorithm based on the ciphersuite. If the ciphersuite needs extra space
           for an IV or other data before the payload, this will return the number of bytes needed. */
        session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;
        algorithm = session_cipher_method -> nx_crypto_algorithm;

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
        /* For customer AEAD algorithms, convert it to AES_GCM. */
        if (NX_SECURE_AEAD_CIPHER_CHECK(algorithm))
        {
            algorithm = NX_CRYPTO_ENCRYPTION_AES_GCM_16;
        }
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

        /* Check the crypto algorithm for any special processing. */
        switch (algorithm)
        {
        case NX_CRYPTO_ENCRYPTION_AES_CBC:
            /* TLS 1.0 does not use an explicit IV in CBC-mode ciphers, so don't include it
               in the record. */
            if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0)
            {
                /* Return size of cipher method IV. */
                *iv_size = (session_cipher_method -> nx_crypto_IV_size_in_bits >> 3);
            }
            else
            {
                *iv_size = 0;
            }
            break;
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
        case NX_CRYPTO_ENCRYPTION_AES_CCM_8:
        /* fallthrough */
        case NX_CRYPTO_ENCRYPTION_AES_CCM_12:
        /* fallthrough */
        case NX_CRYPTO_ENCRYPTION_AES_CCM_16:
        /* fallthrough */
        case NX_CRYPTO_ENCRYPTION_AES_GCM_16:
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3)
            {
                *iv_size = 0;
            }
            else
#endif
            {
                *iv_size = 8;
            }
            break;
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */
        default:
            /* Default, do nothing - only allocate space for ciphers that need it. */
            *iv_size = 0;
            break;
        }
    }
    else
    {
        /* Session is not active so we don't need to allocate space for an IV. */
        *iv_size = 0;
    }

    return(NX_SUCCESS);
}

