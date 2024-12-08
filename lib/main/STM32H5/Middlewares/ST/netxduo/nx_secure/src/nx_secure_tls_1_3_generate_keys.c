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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

/* Some secret generation requires a string of zeroes with a length
   equivalent to the key/hash size. This array is used for that with a memset. */
static UCHAR _nx_secure_tls_zeroes[NX_SECURE_TLS_MAX_KEY_SIZE];

static UINT _nx_secure_tls_1_3_generate_handshake_secrets(NX_SECURE_TLS_SESSION *tls_session);

static UINT _nx_secure_tls_1_3_generate_session_secrets(NX_SECURE_TLS_SESSION *tls_session);


static UINT _nx_secure_tls_hkdf_expand_label(NX_SECURE_TLS_SESSION *tls_session, UCHAR *secret, UINT secret_len,
            UCHAR *label, UINT label_len, UCHAR *context, UINT context_len, UINT length,
            UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method);

static UINT _nx_secure_tls_derive_secret(NX_SECURE_TLS_SESSION *tls_session, UCHAR *secret, UINT secret_len,
                                  UCHAR *label, UINT label_len,
                                  UCHAR *message_hash, UINT message_hash_len,
                                  UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method);

static UINT _nx_secure_tls_hkdf_extract(NX_SECURE_TLS_SESSION *tls_session, UCHAR *salt, UINT salt_len,
                                      UCHAR *ikm, UINT ikm_len, UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method);
#endif



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_generate_psk_secrets             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 to generate the secrets and keys   */
/*    needed for PSK binder generation. Since each "external" PSK needs   */
/*    a binder generated from the early secret (which is generated using  */
/*    that PSK), each PSK gets a separate "early secret" and "binder key".*/
/*    Therefore, this function will be called for *every* PSK provided by */
/*    the application, each time with a different PSK and hash method.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_generate_psk_secret(NX_SECURE_TLS_SESSION *tls_session, NX_SECURE_TLS_PSK_STORE *psk_entry, const NX_CRYPTO_METHOD *hash_method)
{
UINT status;
UINT hash_length;
UCHAR *psk_secret;
UINT   psk_secret_length;
UCHAR *label;
UINT label_length;
UINT is_resumption_psk = NX_FALSE;


    /* Get the hash length so we know how much data we are generating. */
    hash_length = (hash_method->nx_crypto_ICV_size_in_bits >> 3);

    /* The PSK is the input to the early secret. */
    psk_secret = (UCHAR *)psk_entry->nx_secure_tls_psk_data;
    psk_secret_length = psk_entry->nx_secure_tls_psk_data_size;

    NX_SECURE_MEMSET(_nx_secure_tls_zeroes, 0, sizeof(_nx_secure_tls_zeroes));

    /* Perform an HKDF-Extract to get the "early secret". */
    /* Salt: 0 string, IKM: PSK secret. */


    status = _nx_secure_tls_hkdf_extract(tls_session, _nx_secure_tls_zeroes, hash_length, psk_secret, psk_secret_length,
                                         psk_entry->nx_secure_tls_psk_early_secret, hash_length, hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    psk_entry->nx_secure_tls_psk_early_secret_size = hash_length;

    /*----- Binder key value. -----*/

    /* Get the appropriate label for our secret derivation. */
    label = (UCHAR *)((is_resumption_psk)? "res binder" : "ext binder" );
    label_length = 10;

    /* Ext/Res binder key has an empty messages context. */
    status = _nx_secure_tls_derive_secret(tls_session, psk_entry->nx_secure_tls_psk_early_secret, psk_entry->nx_secure_tls_psk_early_secret_size,
                                          label, label_length,
                                          (UCHAR *)"", 0,
                                          psk_entry->nx_secure_tls_psk_binder_key, hash_length, hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    psk_entry->nx_secure_tls_psk_binder_key_size = hash_length;

    /* Generate Finished Key. According to RFC 8446 Section 4.2.11.2, we generate the PSK binder in the same
     * fashion as the Finished message, but using the binder key as input to the HKDF expansion. Thus, generate a "finished"
     * key for this specific PSK entry to use in the binder generation. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, psk_entry->nx_secure_tls_psk_binder_key, psk_entry->nx_secure_tls_psk_binder_key_size,
                                          (UCHAR *)"finished", 8, (UCHAR *)"", 0, hash_length,
                                          psk_entry->nx_secure_tls_psk_finished_key, hash_length, hash_method);

    psk_entry->nx_secure_tls_psk_finished_key_size = hash_length;

    return(status);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_generate_handshake_keys          PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 to generate the symmetric          */
/*    encryption keys used to protect TLS handshake messages.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            cleanup for session cipher, */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_generate_handshake_keys(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;
UCHAR                                *key_block;
ULONG                                 key_block_size;
NX_SECURE_TLS_KEY_SECRETS *secrets;
const NX_CRYPTO_METHOD                     *session_cipher_method = NX_NULL;
const NX_CRYPTO_METHOD                     *hash_method = NX_NULL;
UINT                                  key_size;
UINT                                  iv_size;
UINT                                  key_offset;
UINT                                  hash_size;

    /* From RFC 8446, Section 7.3:
    The traffic keying material is generated from an input traffic secret
    value using:

    [sender]_write_key = HKDF-Expand-Label(Secret, "key", "", key_length)
    [sender]_write_iv  = HKDF-Expand-Label(Secret, "iv", "", iv_length)

    [sender] denotes the sending side.  The value of Secret for each
    record type is shown in the table below.

        +-------------------+---------------------------------------+
        | Record Type       | Secret                                |
        +-------------------+---------------------------------------+
        | 0-RTT Application | client_early_traffic_secret           |
        |                   |                                       |
        | Handshake         | [sender]_handshake_traffic_secret     |
        |                   |                                       |
        | Application Data  | [sender]_application_traffic_secret_N |
        +-------------------+---------------------------------------+
    */

    /* Generate handshake secrets. */
    status = _nx_secure_tls_1_3_generate_handshake_secrets(tls_session);

    /* Get our generated secrets. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;

    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup initialization vector size.  */
    /* IV size for AES-128-GCM is 12 bytes! */
    iv_size = 12; // session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Working pointers into our key material blocks - we need a place to store generated keys. */
    key_block = tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data;
    key_block_size = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data);


    /* Assign MAC secrets to TLS Session. */
    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret = secrets->tls_client_handshake_traffic_secret;
    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret = secrets->tls_server_handshake_traffic_secret;

    /* To generate handshake keys, we need the [sender]_handshake_traffic_secret. */

    /* Generate client traffic key. */
    key_offset = 0;
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_client_handshake_traffic_secret, secrets->tls_client_handshake_traffic_secret_len,
                                          (UCHAR *)"key", 3, (UCHAR *)"", 0, key_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key = &key_block[key_offset];

    key_offset += key_size;

    /* Generate client traffic IV. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_client_handshake_traffic_secret, secrets->tls_client_handshake_traffic_secret_len,
                                          (UCHAR *)"iv", 2, (UCHAR *)"", 0, iv_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv = &key_block[key_offset];
    
    key_offset += iv_size;

    /* Generate server-side key. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_server_handshake_traffic_secret, secrets->tls_server_handshake_traffic_secret_len,
                                          (UCHAR *)"key", 3, (UCHAR *)"", 0, key_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key = &key_block[key_offset];

    key_offset += key_size;

    /* Generate server-side IV. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_server_handshake_traffic_secret, secrets->tls_server_handshake_traffic_secret_len,
                                          (UCHAR *)"iv", 2, (UCHAR *)"", 0, iv_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv = &key_block[key_offset];

    key_offset += iv_size;

    /* We now have the session keys so we can generate the Finished keys for both client and server. */
    /*  From RFC 8446 (TLS 1.3):
        finished_key =
              HKDF-Expand-Label(BaseKey, "finished", "", Hash.length)
    */

    /* Get hash size for this ciphersuite. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Generate server-side Finished Key. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_server_handshake_traffic_secret, secrets->tls_server_handshake_traffic_secret_len,
                                          (UCHAR *)"finished", 8, (UCHAR *)"", 0, hash_size,
                                          &secrets->tls_server_finished_key[0], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    secrets->tls_server_finished_key_len = hash_size;

    /* Generate client-side Finished Key. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_client_handshake_traffic_secret, secrets->tls_client_handshake_traffic_secret_len,
                                          (UCHAR *)"finished", 8, (UCHAR *)"", 0, hash_size,
                                          &secrets->tls_client_finished_key[0], (key_block_size - key_offset), hash_method);

    secrets->tls_client_finished_key_len = hash_size;

    if(status != NX_SUCCESS)
    {
        return(status);
    }


    /* Now, we can initialize our crypto routines and turn on encryption. */
    /* Initialize the crypto method used in the session cipher. */
    if (session_cipher_method -> nx_crypto_init != NULL)
    {
        if (tls_session -> nx_secure_tls_session_cipher_client_initialized && session_cipher_method -> nx_crypto_cleanup)
        {
            status = session_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_session_cipher_metadata_area_client);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            tls_session -> nx_secure_tls_session_cipher_client_initialized = 0;
        }

        /* Set client write key. */
        status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                         tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key,
                                                         session_cipher_method -> nx_crypto_key_size_in_bits,
                                                         &tls_session -> nx_secure_session_cipher_handler_client,
                                                         tls_session -> nx_secure_session_cipher_metadata_area_client,
                                                         tls_session -> nx_secure_session_cipher_metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_session_cipher_client_initialized = 1;

        if (tls_session -> nx_secure_tls_session_cipher_server_initialized && session_cipher_method -> nx_crypto_cleanup)
        {
            status = session_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_session_cipher_metadata_area_server);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            tls_session -> nx_secure_tls_session_cipher_server_initialized = 0;
        }

        /* Set server write key. */
        status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                         tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key,
                                                         session_cipher_method -> nx_crypto_key_size_in_bits,
                                                         &tls_session -> nx_secure_session_cipher_handler_server,
                                                         tls_session -> nx_secure_session_cipher_metadata_area_server,
                                                         tls_session -> nx_secure_session_cipher_metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_session_cipher_server_initialized = 1;
    }

    return(NX_SUCCESS);
}

#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_generate_session_keys            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 to generate the symmetric          */
/*    encryption keys used to protect application data.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_generate_session_keys(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;
UCHAR                                *key_block;
ULONG                                 key_block_size;
NX_SECURE_TLS_KEY_SECRETS *secrets;
const NX_CRYPTO_METHOD                     *session_cipher_method = NX_NULL;
const NX_CRYPTO_METHOD                     *hash_method = NX_NULL;
UINT                                  key_size;
UINT                                  iv_size;
UINT                                  key_offset;

    /* From RFC 8446, Section 7.3:
    The traffic keying material is generated from an input traffic secret
    value using:

    [sender]_write_key = HKDF-Expand-Label(Secret, "key", "", key_length)
    [sender]_write_iv  = HKDF-Expand-Label(Secret, "iv", "", iv_length)

    [sender] denotes the sending side.  The value of Secret for each
    record type is shown in the table below.

        +-------------------+---------------------------------------+
        | Record Type       | Secret                                |
        +-------------------+---------------------------------------+
        | 0-RTT Application | client_early_traffic_secret           |
        |                   |                                       |
        | Handshake         | [sender]_handshake_traffic_secret     |
        |                   |                                       |
        | Application Data  | [sender]_application_traffic_secret_N |
        +-------------------+---------------------------------------+
    */

    /* Generate handshake secrets. */
    status = _nx_secure_tls_1_3_generate_session_secrets(tls_session);

    /* Get our generated secrets. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;

    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup initialization vector size.  */
//    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* IV size for AES-128-GCM is 12 bytes. */
    iv_size = 12; // session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Working pointers into our key material blocks - we need a place to store generated keys.
     * Whenever we generate session keys in TLS 1.3 we are coming from an existing encrypted
     * context so save the keys to the "on-deck" space to be enabled later. */
    key_block = tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data;
    key_block_size = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data);


    /* Assign MAC secrets to TLS Session. */
    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret = secrets->tls_client_application_traffic_secret_0;
    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret = secrets->tls_server_application_traffic_secret_0;

    /* To generate handshake keys, we need the [sender]_handshake_traffic_secret. */

    /* Generate client traffic key. */
    key_offset = 0;
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_client_application_traffic_secret_0, secrets->tls_client_application_traffic_secret_0_len,
                                          (UCHAR *)"key", 3, (UCHAR *)"", 0, key_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    /* Save the generated keys to the on-deck space (don't initialize yet). */
    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_next_write_key = &key_block[key_offset];

    key_offset += key_size;

    /* Generate client traffic IV. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_client_application_traffic_secret_0, secrets->tls_client_application_traffic_secret_0_len,
                                          (UCHAR *)"iv", 2, (UCHAR *)"", 0, iv_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_next_iv = &key_block[key_offset];

    key_offset += iv_size;

    /* Generate server-side key. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_server_application_traffic_secret_0, secrets->tls_server_application_traffic_secret_0_len,
                                          (UCHAR *)"key", 3, (UCHAR *)"", 0, key_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_next_write_key = &key_block[key_offset];

    key_offset += key_size;

    /* Generate server-side IV. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_server_application_traffic_secret_0, secrets->tls_server_application_traffic_secret_0_len,
                                          (UCHAR *)"iv", 2, (UCHAR *)"", 0, iv_size,
                                          &key_block[key_offset], (key_block_size - key_offset), hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_next_iv = &key_block[key_offset];

    key_offset += iv_size;

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    return(NX_SUCCESS);

}

#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_generate_session_psk                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 to generate a PSK for use in       */
/*    session resumptions, using the nonce provided in the                */
/*    NewSessionTicket message.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    ticket_psk                            PSK control block for output  */
/*    nonce                                 Pointer to session nonce      */
/*    nonce_len                             Length of nonce               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_session_psk_generate(NX_SECURE_TLS_SESSION *tls_session, NX_SECURE_TLS_PSK_STORE *ticket_psk, UCHAR *nonce, UINT nonce_len)
{
NX_SECURE_TLS_KEY_SECRETS *secrets;
UINT status;
UINT hash_length;
const NX_CRYPTO_METHOD *hash_method;

    /* Session PSK is generated as follows (From RFC 8446):
     *      HKDF-Expand-Label(resumption_master_secret,
     *                        "resumption", ticket_nonce, Hash.length)
     *
     */

    /* Get a pointer to our key secrets for this session. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get the hash method so we know how much data we are generating. */
    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
    hash_length = (hash_method->nx_crypto_ICV_size_in_bits >> 3);

    /* Generate the PSK by running HKDF-Expand-Label with the resumption secret and the passed-in nonce. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secrets->tls_resumption_master_secret, secrets->tls_resumption_master_secret_len,
                                          (UCHAR *)"resumption", 10, nonce, nonce_len, hash_length,
                                          ticket_psk->nx_secure_tls_psk_data,
                                          sizeof(ticket_psk->nx_secure_tls_psk_data), hash_method);

    /* Set the length of our PSK. */
    ticket_psk->nx_secure_tls_psk_data_size = hash_length;

    return(status);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_generate_handshake_secrets       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 in generating key material.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

static UINT _nx_secure_tls_1_3_generate_handshake_secrets(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;
NX_SECURE_TLS_KEY_SECRETS *secrets;
UINT hash_length;
const NX_CRYPTO_METHOD *hash_method;
UCHAR *psk_secret;
UINT   psk_secret_length;
UCHAR *label;
UINT label_length;
UINT is_resumption_psk = NX_FALSE;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get the hash method so we know how much data we are generating. */
    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
    hash_length = (hash_method->nx_crypto_ICV_size_in_bits >> 3);

    /* Get a pointer to our key secrets for this session. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;

    /* From RFC 8446, section 7.1:

       Keys are derived from two input secrets using the HKDF-Extract and
       Derive-Secret functions.  The general pattern for adding a new secret
       is to use HKDF-Extract with the Salt being the current secret state
       and the Input Keying Material (IKM) being the new secret to be added.
       In this version of TLS 1.3, the two input secrets are:

       -  PSK (a pre-shared key established externally or derived from the
          resumption_master_secret value from a previous connection)

       -  (EC)DHE shared secret (Section 7.4)

       This produces a full key derivation schedule shown in the diagram
       below.  In this diagram, the following formatting conventions apply:

       -  HKDF-Extract is drawn as taking the Salt argument from the top and
          the IKM argument from the left, with its output to the bottom and
          the name of the output on the right.

       -  Derive-Secret's Secret argument is indicated by the incoming
          arrow.  For instance, the Early Secret is the Secret for
          generating the client_early_traffic_secret.

       -  "0" indicates a string of Hash.length bytes set to zero.

                 0
                 |
                 v
       PSK ->  HKDF-Extract = Early Secret
                 |
                 +-----> Derive-Secret(., "ext binder" | "res binder", "")
                 |                     = binder_key
                 |
                 +-----> Derive-Secret(., "c e traffic", ClientHello)
                 |                     = client_early_traffic_secret
                 |
                 +-----> Derive-Secret(., "e exp master", ClientHello)
                 |                     = early_exporter_master_secret
                 v
           Derive-Secret(., "derived", "")
                 |
                 v
       (EC)DHE -> HKDF-Extract = Handshake Secret
                 |
                 +-----> Derive-Secret(., "c hs traffic",
                 |                     ClientHello...ServerHello)
                 |                     = client_handshake_traffic_secret
                 |
                 +-----> Derive-Secret(., "s hs traffic",
                 |                     ClientHello...ServerHello)
                 |                     = server_handshake_traffic_secret
                 v
           Derive-Secret(., "derived", "")
                 |
                 v
       0 -> HKDF-Extract = Master Secret
                 |
                 +-----> Derive-Secret(., "c ap traffic",
                 |                     ClientHello...server Finished)
                 |                     = client_application_traffic_secret_0
                 |
                 +-----> Derive-Secret(., "s ap traffic",
                 |                     ClientHello...server Finished)
                 |                     = server_application_traffic_secret_0
                 |
                 +-----> Derive-Secret(., "exp master",
                 |                     ClientHello...server Finished)
                 |                     = exporter_master_secret
                 |
                 +-----> Derive-Secret(., "res master",
                                       ClientHello...client Finished)
                                       = resumption_master_secret

       The general pattern here is that the secrets shown down the left side
       of the diagram are just raw entropy without context, whereas the
       secrets down the right side include Handshake Context and therefore
       can be used to derive working keys without additional context.  Note
       that the different calls to Derive-Secret may take different Messages
       arguments, even with the same secret.  In a 0-RTT exchange,
       Derive-Secret is called with four distinct transcripts; in a
       1-RTT-only exchange, it is called with three distinct transcripts.

       If a given secret is not available, then the 0-value consisting of a
       string of Hash.length bytes set to zeros is used.  Note that this
       does not mean skipping rounds, so if PSK is not in use, Early Secret
       will still be HKDF-Extract(0, 0).  For the computation of the
       binder_key, the label is "ext binder" for external PSKs (those
       provisioned outside of TLS) and "res binder" for resumption PSKs
       (those provisioned as the resumption master secret of a previous
       handshake).  The different labels prevent the substitution of one
       type of PSK for the other.

       There are multiple potential Early Secret values, depending on which
       PSK the server ultimately selects.  The client will need to compute
       one for each potential PSK; if no PSK is selected, it will then need
       to compute the Early Secret corresponding to the zero PSK.

       Once all the values which are to be derived from a given secret have
       been computed, that secret SHOULD be erased.


     */

    /* Go through all secrets, generate those that haven't been generated yet. */

    /* If available, the chosen PSK is fed into the key generation process. */
    /* If PSK is not in use, Early Secret will still be HKDF-Extract(0, 0). So set PSK as "0". */
    psk_secret = _nx_secure_tls_zeroes;
    psk_secret_length = hash_length;

    if(tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size > 0)
    {
        psk_secret = tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data;
        psk_secret_length = tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size;
    }

    NX_SECURE_MEMSET(_nx_secure_tls_zeroes, 0, sizeof(_nx_secure_tls_zeroes));
    
    if(secrets->tls_early_secret_len == 0)
    {
        /* Perform an HKDF-Extract to get the "early secret". */
        /* Salt: 0 string, IKM: PSK secret. */
        status = _nx_secure_tls_hkdf_extract(tls_session, _nx_secure_tls_zeroes, hash_length, psk_secret, psk_secret_length,
                                             secrets->tls_early_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_early_secret_len = hash_length;
    }

    /* Generate keys and secrets based on the "early secret". */
    if(secrets->tls_early_secret_len != 0)
    {
        /*----- Binder key value. -----*/

        /* Get the appropriate label for our secret derivation. */
        label = (UCHAR *)((is_resumption_psk)? "res binder" : "ext binder");
        label_length = 10;

        /* Ext/Res binder key has an empty messages context. */
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_early_secret, secrets->tls_early_secret_len, label, label_length,
                                              (UCHAR *)"", 0,
                                              secrets->tls_binder_key, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_binder_key_len = hash_length;

        /*----- Early traffic secret. -----*/
        label = (UCHAR *)"c e traffic";
        label_length = 11;

        /* Context is hash of ClientHello. */
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_early_secret, secrets->tls_early_secret_len, label, label_length,
                                              (UCHAR *)"FIXME", 5,
                                              secrets->tls_client_early_traffic_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_client_early_traffic_secret_len = hash_length;

        /*----- Early exporter master secret. -----*/
        label = (UCHAR *)"e exp master";
        label_length = 12;

        /* Context is hash of ClientHello. */
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_early_secret, secrets->tls_early_secret_len, label, label_length,
                                              (UCHAR *)"FIXME", 5,
                                              secrets->tls_early_exporter_master_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_early_exporter_master_secret_len = hash_length;
    }

    /* Handshake secret - special case! Needs a pre-master secret from the ECDHE exchange and the early secret from above. */
    if(secrets->tls_handshake_secret_len == 0 && tls_session->nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size != 0)
    {
        /* Start by deriving the salt from the early secret. Context is empty! */
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_early_secret, secrets->tls_early_secret_len, (UCHAR *)"derived", 7,
                                              (UCHAR *)"", 0,
                                              secrets->tls_handshake_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Now perform an HKDF-Extract to get the handshake secret.
           Salt: derived secret from above, IKM: ECDHE pre-master secret */
        status = _nx_secure_tls_hkdf_extract(tls_session, secrets->tls_handshake_secret, hash_length,
                                             tls_session->nx_secure_tls_key_material.nx_secure_tls_pre_master_secret,
                                             tls_session->nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size,
                                             secrets->tls_handshake_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_handshake_secret_len = hash_length;
    }

    /* Generate keys and secrets based on the "handshake secret". */
    if(secrets->tls_handshake_secret_len != 0)
    {
        /*----- Client handshake traffic secret. -----*/
        label = (UCHAR *)"c hs traffic";
        label_length = 12;

        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_handshake_secret, secrets->tls_handshake_secret_len, label, label_length,
                                              tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVERHELLO], hash_length,
                                              secrets->tls_client_handshake_traffic_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_client_handshake_traffic_secret_len = hash_length;

        /*----- Server handshake traffic secret. -----*/
        label = (UCHAR *)"s hs traffic";
        label_length = 12;

        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_handshake_secret, secrets->tls_handshake_secret_len, label, label_length,
                                              tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVERHELLO], hash_length,
                                              secrets->tls_server_handshake_traffic_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_server_handshake_traffic_secret_len = hash_length;
    }

    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_generate_session_secrets         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 in generating key material.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

static UINT _nx_secure_tls_1_3_generate_session_secrets(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;
NX_SECURE_TLS_KEY_SECRETS *secrets;
UINT hash_length;
const NX_CRYPTO_METHOD *hash_method;
UCHAR *label;
UINT label_length;
UCHAR (*transcript_hashes)[NX_SECURE_TLS_MAX_HASH_SIZE] = tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes;



    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get the hash method so we know how much data we are generating. */
    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
    hash_length = (hash_method->nx_crypto_ICV_size_in_bits >> 3);

    /* Get a pointer to our key secrets for this session. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;

    /* Make sure our zeroes string is initialized. */
    NX_SECURE_MEMSET(_nx_secure_tls_zeroes, 0, sizeof(_nx_secure_tls_zeroes));
    
    /* Application Master secret - special case! Needs a secret derived from the previous secret. */
    if(secrets->tls_master_secret_len == 0 && secrets->tls_handshake_secret_len != 0)
    {
        /* Start by deriving the salt from the handshake secret. Context is empty! */     
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_handshake_secret, secrets->tls_handshake_secret_len, (UCHAR *)"derived", 7,
                                              (UCHAR *)"", 0,
                                              secrets->tls_master_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        
        /* Now perform an HKDF-Extract to get the application master secret.
           Salt: derived secret from above, IKM: 0 string  */
        status = _nx_secure_tls_hkdf_extract(tls_session, secrets->tls_master_secret, hash_length, 
                                             _nx_secure_tls_zeroes, hash_length,
                                             secrets->tls_master_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_master_secret_len = hash_length;
    }

    /* Derive secrets based on the application master secret. */
    if(secrets->tls_master_secret_len != 0)
    {
        /*----- Client application traffic secret 0. -----*/
        label = (UCHAR *)"c ap traffic";
        label_length = 12;
        
        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_master_secret, secrets->tls_master_secret_len, label, label_length,
                                              transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED], hash_length,
                                              secrets->tls_client_application_traffic_secret_0, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_client_application_traffic_secret_0_len = hash_length;

        /*----- Server application traffic secret 0. -----*/
        label = (UCHAR *)"s ap traffic";
        label_length = 12;

        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_master_secret, secrets->tls_master_secret_len, label, label_length,
                                              transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED], hash_length,
                                              secrets->tls_server_application_traffic_secret_0, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_server_application_traffic_secret_0_len = hash_length;

        /*----- Exporter master secret. -----*/
        label = (UCHAR *)"exp master";
        label_length = 10;

        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_master_secret, secrets->tls_master_secret_len, label, label_length,
                                              transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED], hash_length,
                                              secrets->tls_exporter_master_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_early_exporter_master_secret_len = hash_length;

        /*----- Resumption master secret. -----*/
        label = (UCHAR *)"res master";
        label_length = 10;

        status = _nx_secure_tls_derive_secret(tls_session, secrets->tls_master_secret, secrets->tls_master_secret_len, label, label_length,
                                              transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENT_FINISHED], hash_length,
                                              secrets->tls_resumption_master_secret, hash_length, hash_method);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
        secrets->tls_resumption_master_secret_len = hash_length;


    }

    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_derive_secret                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 in generating key material.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

static UCHAR _nx_secure_tls_temp_hash[100];

static UINT _nx_secure_tls_derive_secret(NX_SECURE_TLS_SESSION *tls_session, UCHAR *secret, UINT secret_len,
                                  UCHAR *label, UINT label_len,
                                  UCHAR *message_hash, UINT message_hash_len,
                                  UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method)
{
UINT status;
UINT hash_length;

/* From RFC 8446, section 7.1:
        Derive-Secret(Secret, Label, Messages) =
                 HKDF-Expand-Label(Secret, Label,
                                   Transcript-Hash(Messages), Hash.length)
*/


    /* Get session hash routine. */
    hash_length = (hash_method->nx_crypto_ICV_size_in_bits >> 3);


    /* Our "messages" parameter is actually the ongoing hash of handshake
       messages stored in the TLS session context. In some contexts, the message hash will be of 0 length! */
    if(message_hash_len == 0)
    {
        /* Point the message hash at our temporary buffer. */
        message_hash = &_nx_secure_tls_temp_hash[0];
        message_hash_len = hash_length;

        /* Context has 0 length, so generate a hash on the empty string to feed into expand label call below.
         * Utilize the temporary "hash scratch" data buffer to initialize and calculate the hash. */
        if (hash_method -> nx_crypto_init)
        {
            status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                                   NX_NULL,
                                                   0,
                                                   tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                                   tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                   tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                                        (NX_CRYPTO_METHOD*)hash_method,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        NX_NULL,
                                                        0,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
           status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                                      (NX_CRYPTO_METHOD*)hash_method,
                                                      NX_NULL,
                                                      0,
                                                      (UCHAR *)"",
                                                      0,
                                                      NX_NULL,
                                                      NX_NULL,
                                                      0,
                                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                                      NX_NULL,
                                                      NX_NULL);

           if (status != NX_CRYPTO_SUCCESS)
           {
               return(status);
           }
        }



        /* Generate a hash using our temporary copy of the hash metadata, place it into the TLS Session transcript hash array. */
        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                                        (NX_CRYPTO_METHOD*)hash_method,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        0,
                                                        NX_NULL,
                                                        message_hash,
                                                        hash_length,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                                        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

        }
    }

    /* Now derive the output by calling HKDF-Expand-Label. */
    status = _nx_secure_tls_hkdf_expand_label(tls_session, secret, secret_len,
            label, label_len, message_hash, message_hash_len, hash_length,
            output, output_length, hash_method);

    return(status);
}

#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_hkdf_expand_label                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 in generating key material.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

/* Buffer for HKDF output. The HKDF temporary can technically be as big as 
   514 bytes: 2 (length) + 1 (label length byte) + 255 (label) + 1 (context length byte) + 255 (context). 
   However, 100 bytes is sufficient for the mandatory ciphersuite. */
static UCHAR _nx_secure_tls_hkdf_temp_output[100];
static UINT _nx_secure_tls_hkdf_expand_label(NX_SECURE_TLS_SESSION *tls_session, UCHAR *secret, UINT secret_len,
                                      UCHAR *label, UINT label_len, UCHAR *context, UINT context_len, UINT length,
                                      UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method)
{
UINT                                 status;
UINT                                 data_len;
const NX_CRYPTO_METHOD                     *session_hkdf_method = NX_NULL;
const NX_CRYPTO_METHOD                     *session_hmac_method = NX_NULL;

/*VOID                                 *handler = NX_NULL;*/
    /* From RFC 8446, section 7.1:
    HKDF-Expand-Label(Secret, Label, Context, Length) =
           HKDF-Expand(Secret, HkdfLabel, Length)

      Where HkdfLabel is specified as:

      struct {
          uint16 length = Length;
          opaque label<7..255> = "tls13 " + Label;
          opaque context<0..255> = Context;
      } HkdfLabel;
    */

    if (sizeof(_nx_secure_tls_hkdf_temp_output) < (10u + label_len + context_len))
    {

        /* Buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Get our HKDF method and hash routine. */
    /*session_hash_method = ciphersuite->nx_secure_tls_hash;*/
    session_hkdf_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_hkdf_method;
    session_hmac_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_hmac_method;

    /* Now build the HkdfLabel from our inputs. */
    _nx_secure_tls_hkdf_temp_output[0] = (UCHAR)((length & 0xFF00) >> 8);
    _nx_secure_tls_hkdf_temp_output[1] = (UCHAR)(length & 0x00FF);
    data_len = 2;

    /* Add the length of the label (single octet). */
    _nx_secure_tls_hkdf_temp_output[data_len] = (UCHAR)(6 + label_len);
    data_len = data_len + 1;
    
    /* Now copy in label with TLS 1.3 prefix. */
    NX_CRYPTO_MEMCPY(&_nx_secure_tls_hkdf_temp_output[data_len], "tls13 ", 6); /* Use case of memcpy is verified. */
    data_len += 6;
    NX_CRYPTO_MEMCPY(&_nx_secure_tls_hkdf_temp_output[data_len], label, label_len); /* Use case of memcpy is verified. */
    data_len += label_len;

    /* Add the length of the context (single octet). */
    _nx_secure_tls_hkdf_temp_output[data_len] = (UCHAR)(context_len);
    data_len = data_len + 1;    
    
    /* Now copy in context. */
    NX_CRYPTO_MEMCPY(&_nx_secure_tls_hkdf_temp_output[data_len], context, context_len); /* Use case of memcpy is verified. */
    data_len += context_len;


    /* Initialize the HKDF context. */
    status = session_hkdf_method->nx_crypto_init((NX_CRYPTO_METHOD*)session_hkdf_method, NX_NULL, 0, NX_NULL,
                                        tls_session -> nx_secure_tls_prf_metadata_area,
                                        tls_session -> nx_secure_tls_prf_metadata_size);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Set the hash and HMAC routines for the HKDF. */
    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_SET_HMAC, NX_NULL, (NX_CRYPTO_METHOD*)session_hmac_method,
                                             NX_NULL, 0, NX_NULL, 0, NX_NULL, NX_NULL, 0,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_SET_HASH, NX_NULL,
                                             (NX_CRYPTO_METHOD*)hash_method,
                                             NX_NULL, 0,NX_NULL, 0, NX_NULL, NX_NULL, 0,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Set the PRK for the HKDF-expand operation. */
    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_SET_PRK,
                                             NX_NULL,
                                             (NX_CRYPTO_METHOD*)session_hkdf_method,
                                             (UCHAR*)(secret),     /* Input HKDF label. */
                                             (secret_len << 3),
                                             NX_NULL,
                                             0,
                                             NX_NULL,
                                             NX_NULL,
                                             0,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Now perform the HKDF operation. */
    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_EXPAND,
                                             NX_NULL,
                                             (NX_CRYPTO_METHOD*)session_hkdf_method,
                                             (UCHAR*)(_nx_secure_tls_hkdf_temp_output), /* Input HKDF label. */
                                             (data_len << 3),
                                             NX_NULL,
                                             0,
                                             NX_NULL,
                                             (UCHAR *)output,
                                             output_length,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    return(status);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_hkdf_extract                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used by TLS 1.3 in generating key material.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    salt                                  HKDF salt parameter           */
/*    salt_len                              Length of salt                */
/*    ikm                                   HKDF input key material       */
/*    ikm_len                               Length of IKM                 */
/*    output                                Output buffer                 */
/*    output_length                         Desired output length         */
/*    hash_method                           Hash routine for HMAC         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

/*static UCHAR _nx_secure_tls_hkdf_label[524];*/

static UINT _nx_secure_tls_hkdf_extract(NX_SECURE_TLS_SESSION *tls_session, UCHAR *salt, UINT salt_len,
                                      UCHAR *ikm, UINT ikm_len, UCHAR *output, UINT output_length, const NX_CRYPTO_METHOD *hash_method)
{
UINT                                 status;
const NX_CRYPTO_METHOD                     *session_hkdf_method = NX_NULL;
const NX_CRYPTO_METHOD                     *session_hmac_method = NX_NULL;


    /* Get our HKDF method and hash routine. */
    session_hkdf_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_hkdf_method;
    session_hmac_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_hmac_method;

    /* Initialize the HKDF context with our IKM. */
    status = session_hkdf_method->nx_crypto_init((NX_CRYPTO_METHOD*)session_hkdf_method, ikm, ikm_len << 3, NX_NULL,
                                        tls_session -> nx_secure_tls_prf_metadata_area,
                                        tls_session -> nx_secure_tls_prf_metadata_size);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Set the hash and HMAC routines for the HKDF. */
    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_SET_HMAC, NX_NULL, (NX_CRYPTO_METHOD*)session_hmac_method,
                                             NX_NULL, 0, NX_NULL, 0, NX_NULL, NX_NULL, 0,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_SET_HASH, NX_NULL,
                                             (NX_CRYPTO_METHOD*)hash_method,
                                             NX_NULL, 0,NX_NULL, 0, NX_NULL, NX_NULL, 0,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    if(status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    /* Now perform the HKDF operation. */
    status = session_hkdf_method->nx_crypto_operation(NX_CRYPTO_HKDF_EXTRACT,
                                             NX_NULL,
                                             (NX_CRYPTO_METHOD*)session_hkdf_method,
                                             salt,
                                             salt_len << 3,
                                             ikm,
                                             ikm_len,
                                             NX_NULL,
                                             (UCHAR *)output,
                                             output_length,
                                             tls_session -> nx_secure_tls_prf_metadata_area,
                                             tls_session -> nx_secure_tls_prf_metadata_size,
                                             NX_NULL, NX_NULL);

    return(status);
}
#endif


