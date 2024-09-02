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
/**    Transport Layer Security (TLS) - Generate Session Keys             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_session_keys_set                         PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the session keys for a TLS session following the */
/*    sending or receiving of a ChangeCipherSpec message. In              */
/*    renegotiation handshakes, two separate set of session keys will be  */
/*    in use simultaneously so we need this to be able to separate which  */
/*    keys are actually in use.                                           */
/*                                                                        */
/*    Once the keys are set, this function initializes the appropriate    */
/*    session cipher with the new key set.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    tls_key_material                      TLS key material              */
/*    key_material_data_size                Length of key material        */
/*    is_client                             Indicate keys for client      */
/*    session_cipher_initialized            Whether cipher is initialized */
/*    session_cipher_metadata               Metadata for session cipher   */
/*    session_cipher_handler                Session cipher handler        */
/*    session_cipher_metadata_size          Size of session metadata      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Initialize crypto             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_session_keys_set(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, NX_SECURE_TLS_KEY_MATERIAL *tls_key_material,
                                 UINT key_material_data_size, UINT is_client, UCHAR *session_cipher_initialized,
                                 VOID *session_cipher_metadata, VOID **session_cipher_handler, ULONG session_cipher_metadata_size)
{
UINT                    status;
UCHAR                  *key_block;     /* Maximum ciphersuite key size - AES_256_CBC_SHA, 2x32 byte keys + 2x20 byte MAC secrets + 2x16 IVs. */
UINT                    key_size;
UINT                    hash_size;
UINT                    iv_size;
UINT                    key_offset;
UCHAR                  *write_key;
const NX_CRYPTO_METHOD *session_cipher_method = NX_NULL;

    /* The key material should have already been generated by nx_secure_tls_generate_keys once all
     * key generation data was available. This simply switches the appropriate key data over to the active
     * key block */

    /* Working pointers into our key material blocks. */
    key_block = tls_key_material -> nx_secure_tls_key_material_data;

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = ciphersuite -> nx_secure_tls_session_cipher;

    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup ciphersuite data for hash size - used for the MAC secret. */
    hash_size = ciphersuite -> nx_secure_tls_hash_size;

    /* Lookup initialization vector size.  */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Partition the key block into our keys and secrets following the TLS spec.*/
    key_offset = 0;

    if (((hash_size + key_size + iv_size) << 1) > key_material_data_size)
    {

        /* No space for key material data. */
        return(NX_SECURE_TLS_CRYPTO_KEYS_TOO_LARGE);
    }

    /* First, the mac secrets. Check for non-zero in the (unlikely) event we are using a NULL hash. */
    if (hash_size > 0)
    {

        /* Copy new client mac secret over if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], hash_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_client_write_mac_secret = key_block + key_offset;
        key_offset += hash_size;

        /* Copy new server mac secret if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], hash_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_server_write_mac_secret = key_block + key_offset;
        key_offset += hash_size;
    }

    /* Now the keys. Check for non-zero size in the event we are using a NULL cipher (usually for debugging). */
    if (key_size > 0)
    {
        /* Copy new client session key if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], key_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_client_write_key = key_block + key_offset;
        key_offset += key_size;

        /* Copy new server session key if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], key_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_server_write_key = key_block + key_offset;
        key_offset += key_size;
    }

    /* Finally, the IVs. Many ciphers don't use IV's so the iv_size is often zero. */
    if (iv_size > 0)
    {
        /* Copy new client IV if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], iv_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_client_iv = key_block + key_offset;
        key_offset += iv_size;

        /* Copy new server IV if setting server keys. */
        if (!is_client)
        {
            NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_key_material_data[key_offset],
                             &tls_key_material -> nx_secure_tls_new_key_material_data[key_offset], iv_size); /* Use case of memcpy is verified. */
        }
        tls_key_material -> nx_secure_tls_server_iv = key_block + key_offset;
    }

    /* Initialize the crypto method used in the session cipher. */
    if (session_cipher_method -> nx_crypto_init != NULL)
    {
        /* Set client write key. */
        if (*session_cipher_initialized && session_cipher_method -> nx_crypto_cleanup)
        {
            status = session_cipher_method -> nx_crypto_cleanup(session_cipher_metadata);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            *session_cipher_initialized = 0;
        }

        if (is_client)
        {
            write_key = tls_key_material -> nx_secure_tls_client_write_key;
        }
        else
        {
            write_key = tls_key_material -> nx_secure_tls_server_write_key;
        }

        status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                         write_key,
                                                         session_cipher_method -> nx_crypto_key_size_in_bits,
                                                         session_cipher_handler,
                                                         session_cipher_metadata,
                                                         session_cipher_metadata_size);

        *session_cipher_initialized = 1;

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SECURE_TLS_SUCCESS);
}

