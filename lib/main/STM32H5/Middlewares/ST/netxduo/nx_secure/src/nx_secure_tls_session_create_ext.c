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
/*    _nx_secure_tls_session_create_ext                   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the new-style API for creating a new TLS session.  */
/*    Like the original function, it initializes a TLS session control    */
/*    block for later use in establishing a secure TLS session over a TCP */
/*    socket or other lower-level networking protocol. The difference is  */
/*    that this function takes an un-ordered array of cryptographic       */
/*    methods and a mapping table from ciphersuites to algorithm          */
/*    identifiers so the mapping of crypto methods to ciphersuites can be */
/*    done automatically, rather than needing to link in all possible     */
/*    cipher routines in a single table.                                  */
/*                                                                        */
/*    To calculate the necessary metadata size, the API                   */
/*    nx_secure_tls_metadata_size_calculate may be used.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session control block     */
/*    crypto_array                          crypto methods to use         */
/*    cipher_map                            Mapping table for ciphersuites*/
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
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed race condition for    */
/*                                            multithread transmission,   */
/*                                            added ECC initialization,   */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            added null pointer checking,*/
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added null pointer checking,*/
/*                                            removed unused code,        */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), and added*/
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

static UINT _find_cipher(UINT cipher_id, UINT cipher_role_id, UINT key_size, const NX_CRYPTO_METHOD **crypto_array, UINT array_size, const NX_CRYPTO_METHOD **crypto_method)
{
UINT i;

    for(i = 0; i < array_size; ++i)
    {
        if(crypto_array[i]->nx_crypto_algorithm == cipher_id)
        {
            if (cipher_role_id == NX_CRYPTO_ROLE_SYMMETRIC &&
                crypto_array[i] -> nx_crypto_key_size_in_bits != key_size << 3)
            {
                continue;
            }

            *crypto_method = crypto_array[i];
            return(NX_SUCCESS);
        }
    }

    *crypto_method = NX_NULL;
    return(NX_SECURE_TLS_UNSUPPORTED_CIPHER);
}



static UINT _map_tls_ciphersuites(NX_SECURE_TLS_SESSION *tls_session,
                                  const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                  const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size, UINT *metadata_size)
{
NX_CRYPTO_ROLE_ENTRY            current_cipher;
const NX_CRYPTO_METHOD          *cipher_method;
NX_SECURE_TLS_CIPHERSUITE_INFO  *cipher_entry;
NX_SECURE_TLS_CRYPTO *          crypto_table;
UINT                            cipher_id;
UINT                            suite;
UINT                            cipher_counter;
UINT                            remaining_size;
UCHAR                           crypto_found;
UINT                            status;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT ecdhe_found;
UINT dhe_found;
UINT rsa_found;
UINT dsa_found;
UINT ecdsa_found;
#endif

/* Bitmasks for marking found ciphers. */
const UCHAR sig_found =     0x1;
const UCHAR key_ex_found =  0x2;
const UCHAR symm_found =    0x4;
const UCHAR hash_found =    0x8;
const UCHAR prf_found =     0x10;
const UCHAR all_found =     sig_found | key_ex_found | symm_found | hash_found | prf_found;

    /* Pointer to our lookup table. */
    crypto_table = tls_session->nx_secure_tls_crypto_table;

    /* Get a pointer to our entry. */
    cipher_entry = crypto_table->nx_secure_tls_ciphersuite_lookup_table;

    /* For bookkeeping, keep track of remaining metadata space. */
    remaining_size = *metadata_size;

    /* TLS version-specific cipher mapping. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    /* TLS 1.0, 1.1 both need SHA-1 AND MD5. If both are present, we support those versions, otherwise
     * we don't support either. */

    /* Find MD5. If not found, cipher_method is set to NX_NULL so all good either way. */
    status = _find_cipher(NX_CRYPTO_HASH_MD5, NX_CRYPTO_ROLE_RAW_HASH, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_handshake_hash_md5_method = cipher_method;
    if(status != NX_SUCCESS)
    {
        /* TLS 1.0, 1.1 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_0)));
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_1)));
    }

    /* Find SHA-1 and assign (if not found, pointer is set to NX_NULL). */
    status = _find_cipher(NX_CRYPTO_HASH_SHA1, NX_CRYPTO_ROLE_RAW_HASH, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_handshake_hash_sha1_method = cipher_method;

    if(status != NX_SUCCESS)
    {
        /* TLS 1.0, 1.1 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_0)));
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_1)));
    }

    /* Find TLS PRF (for TLS 1.0 and 1.1) and assign (if not found, pointer is set to NX_NULL). */
    status = _find_cipher(NX_CRYPTO_PRF_HMAC_SHA1, NX_CRYPTO_ROLE_PRF, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_prf_1_method = cipher_method;

    if(status != NX_SUCCESS)
    {
        /* TLS 1.0, 1.1 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_0)));
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_1)));
    }

#endif

    /* TLS 1.2 - the default PRF uses SHA-256 so make sure we have at least that routine. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    status = _find_cipher(NX_CRYPTO_HASH_SHA256, NX_CRYPTO_ROLE_RAW_HASH, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_handshake_hash_sha256_method = cipher_method;

    if(status != NX_SUCCESS)
    {
        /* Version 1.2 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_2)));
    }

    /* Find TLS PRF (for TLS 1.0 and 1.1) and assign (if not found, pointer is set to NX_NULL). */
    status = _find_cipher(NX_CRYPTO_PRF_HMAC_SHA2_256, NX_CRYPTO_ROLE_PRF, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_prf_sha256_method = cipher_method;

    if(status != NX_SUCCESS)
    {
        /* TLS 1.0, 1.1 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_0)));
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_1)));
    }

#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* TLS 1.3. */
    /* TLS 1.3 supports only ECDHE and DHE key exchanges. */
    ecdhe_found = _find_cipher(NX_CRYPTO_KEY_EXCHANGE_ECDHE, NX_CRYPTO_ROLE_KEY_EXCHANGE, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_ecdhe_method = cipher_method;
    dhe_found = _find_cipher(NX_CRYPTO_KEY_EXCHANGE_DHE, NX_CRYPTO_ROLE_KEY_EXCHANGE, 0, crypto_array, crypto_array_size, &cipher_method);
    if(!(ecdhe_found || dhe_found))
    {
        /* TLS 1.3 NOT supported! */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_3)));
    }

    /* TLS 1.3 needs at least one signature authentication routine. */
    rsa_found = _find_cipher(NX_CRYPTO_DIGITAL_SIGNATURE_RSA, NX_CRYPTO_ROLE_SIGNATURE_CRYPTO, 0, crypto_array, crypto_array_size, &cipher_method);
    dsa_found = _find_cipher(NX_CRYPTO_DIGITAL_SIGNATURE_DSA, NX_CRYPTO_ROLE_SIGNATURE_CRYPTO, 0, crypto_array, crypto_array_size, &cipher_method);
    ecdsa_found = _find_cipher(NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA, NX_CRYPTO_ROLE_SIGNATURE_CRYPTO, 0, crypto_array, crypto_array_size, &cipher_method);

    if(!((rsa_found == NX_SUCCESS) || (dsa_found == NX_SUCCESS) || (ecdsa_found == NX_SUCCESS)))
    {
        /* TLS 1.3 is  NOTsupported!. */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_3)));
    }

    /* TLS 1.3 needs HKDF and HMAC methods. */
    status = _find_cipher(NX_CRYPTO_HASH_HMAC, NX_CRYPTO_ROLE_HMAC, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_hmac_method = cipher_method;
    if(status != NX_SUCCESS)
    {
        /* TLS 1.3 is  NOTsupported!. */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_3)));
    }

    status = _find_cipher(NX_CRYPTO_HKDF_METHOD, NX_CRYPTO_ROLE_PRF, 0, crypto_array, crypto_array_size, &cipher_method);
    crypto_table->nx_secure_tls_hkdf_method = cipher_method;
    if(status != NX_SUCCESS)
    {
        /* TLS 1.3 is  NOTsupported!. */
        tls_session->nx_secure_tls_supported_versions = (tls_session->nx_secure_tls_supported_versions & (USHORT)(~(NX_SECURE_TLS_BITFIELD_VERSION_1_3)));
    }
#endif


    /* Loop through cipher map, check each ciphersuite. */
    for(suite = 0; suite < cipher_map_size; ++suite)
    {
        /* Search the crypto array for each cipher in this ciphersuite map. */
        cipher_entry->nx_secure_tls_ciphersuite = cipher_map[suite]->nx_crypto_ciphersuite_id;

        /* Handle TLS mapping. */
        if(cipher_map[suite]->nx_crypto_internal_id == NX_SECURE_APPLICATION_TLS)
        {
            /* Start with no crypto methods found. */
            crypto_found = 0;

            /* Loop through the ciphers in this ciphersuite and map to a cipher method in the cipher array. */
            for(cipher_counter = 0; cipher_counter < NX_CRYPTO_MAX_CIPHER_ROLES; ++cipher_counter)
            {
                /* Get the current entry and role. */
                current_cipher = cipher_map[suite]->nx_crypto_ciphers[cipher_counter];
                cipher_id = current_cipher.nx_crypto_role_cipher_id;

                /* Check for end of expected ciphers. */
                if(current_cipher.nx_crypto_role_id == NX_CRYPTO_ROLE_NONE)
                {
                    /* Reached the end of the list - ciphersuite is good if we found the
                       expected ciphers. */
                    if(crypto_found != all_found)
                    {
                        break;
                    }

                    /* The ciphersuite is fully supported given the input
                       crypto method array. Add it to the table by advancing the pointer
                       as a ciphersuite we can use. */
                    cipher_entry = &cipher_entry[1];
                    crypto_table->nx_secure_tls_ciphersuite_lookup_table_size++;

                    /* If there is not enough space, return error. */
                    if(remaining_size < sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO))
                    {
                        return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
                    }

                    /* Update our bookkeeping. */
                    remaining_size -= sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);
                    
                    /* Exit the search loop. */
                    break;                
                }
                
                /* Cipher is not "None" so we need to see if it is available. */
                status = _find_cipher(cipher_id, current_cipher.nx_crypto_role_id, cipher_map[suite] -> nx_crypto_symmetric_key_size, crypto_array, crypto_array_size, &cipher_method);
                if(status != NX_SUCCESS)
                {
                    /* Did not find a cipher, break out of search. */
                    break;
                }

                /* Put the cipher method into the ciphersuite info structure. */
                switch(current_cipher.nx_crypto_role_id)
                {
                case NX_CRYPTO_ROLE_KEY_EXCHANGE:
                    cipher_entry->nx_secure_tls_public_cipher = cipher_method;
                    crypto_found |= key_ex_found;
                    break;
                case NX_CRYPTO_ROLE_SIGNATURE_CRYPTO:
                    cipher_entry->nx_secure_tls_public_auth = cipher_method;
                    crypto_found |= sig_found;
                    break;
                case NX_CRYPTO_ROLE_MAC_HASH:
                    cipher_entry->nx_secure_tls_hash = cipher_method;
                    cipher_entry->nx_secure_tls_hash_size = (USHORT)(cipher_method->nx_crypto_ICV_size_in_bits >> 3);
                    crypto_found |= hash_found;
                    break;
                case NX_CRYPTO_ROLE_SYMMETRIC:
                    cipher_entry->nx_secure_tls_session_cipher = cipher_method;
                    cipher_entry->nx_secure_tls_session_key_size = (UCHAR)cipher_map[suite]->nx_crypto_symmetric_key_size;
                    cipher_entry->nx_secure_tls_iv_size = (UCHAR)(cipher_method->nx_crypto_IV_size_in_bits >> 3);
                    crypto_found |= symm_found;
                    break;
                case NX_CRYPTO_ROLE_PRF:
                    cipher_entry->nx_secure_tls_prf = cipher_method;
                    crypto_found |= prf_found;
                    break;
                default:
                    /* Cipher role not supported by TLS. Ignore. */
                    break;
                }
            }
        }
    }

    /* Return the used metadata size. */
    *metadata_size = (*metadata_size - remaining_size);

    return(NX_SUCCESS);
}



static UINT _map_x509_ciphersuites(NX_SECURE_TLS_SESSION *tls_session,
                                  const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                  const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size, UINT *metadata_size)
{
NX_CRYPTO_ROLE_ENTRY            current_cipher;
const NX_CRYPTO_METHOD                *cipher_method;
NX_SECURE_X509_CRYPTO          *cert_crypto;
NX_SECURE_TLS_CRYPTO *          crypto_table;
UINT                            cipher_id;
UINT                            suite;
UINT                            cipher_counter;
UINT                            status;
UINT                            remaining_size;
UCHAR                           crypto_found;

/* Constants for marking ciphers as found. */
const UCHAR pub_key_found = 0x1;
const UCHAR hash_found =    0x2;
const UCHAR all_found =     pub_key_found | hash_found;

    /* Get a pointer to our entry. */
    crypto_table = tls_session->nx_secure_tls_crypto_table;
    cert_crypto = crypto_table->nx_secure_tls_x509_cipher_table;

    /* For bookkeeping, keep track of remaining metadata space. */
    remaining_size = *metadata_size;

    /* Loop through cipher map, check each ciphersuite. */
    for(suite = 0; suite < cipher_map_size; ++suite)
    {
        /* Search the crypto array for each cipher in this ciphersuite map. */
        cert_crypto->nx_secure_x509_crypto_identifier = cipher_map[suite]->nx_crypto_ciphersuite_id;

        /* Handle X.509 mapping. */
        if(cipher_map[suite]->nx_crypto_internal_id == NX_SECURE_APPLICATION_X509)
        {
            crypto_found = 0;

            /* Loop through the ciphers in this ciphersuite and map to a cipher method in the cipher array. */
            for(cipher_counter = 0; cipher_counter < NX_CRYPTO_MAX_CIPHER_ROLES; ++cipher_counter)
            {
                /* Get the current entry and role. */
                current_cipher = cipher_map[suite]->nx_crypto_ciphers[cipher_counter];
                cipher_id = current_cipher.nx_crypto_role_cipher_id;

                if(current_cipher.nx_crypto_role_id == NX_CRYPTO_ROLE_NONE)
                {
                    /* Reached the end of the list - ciphersuite is good if all ciphers present. */
                    if(crypto_found != all_found)
                    {
                        break;
                    }

                    /* Advance to the next entry in the table. */
                    cert_crypto = &cert_crypto[1];
                    crypto_table->nx_secure_tls_x509_cipher_table_size++;

                    /* If there is not enough space, return error. */
                    if(remaining_size < sizeof(NX_SECURE_X509_CRYPTO))
                    {
                        return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
                    }

                    /* Update our bookkeeping. */
                    remaining_size -= sizeof(NX_SECURE_X509_CRYPTO);
                    
                    /* Break out of the search loop. */
                    break;
                }
                
                /* Cipher is not "none" so see if it's available. */
                status = _find_cipher(cipher_id, current_cipher.nx_crypto_role_id, 0, crypto_array, crypto_array_size, &cipher_method);
                if(status != NX_SUCCESS)
                {
                    /* Did not find a cipher, break out of search. */
                    break;
                }

                /* Put the cipher method into the ciphersuite info structure. */
                switch(current_cipher.nx_crypto_role_id)
                {
                case NX_CRYPTO_ROLE_SIGNATURE_CRYPTO:
                    cert_crypto->nx_secure_x509_public_cipher_method = cipher_method;
                    crypto_found |= pub_key_found;
                    break;
                case NX_CRYPTO_ROLE_SIGNATURE_HASH:
                    cert_crypto->nx_secure_x509_hash_method = cipher_method;
                    crypto_found |= hash_found;
                    break;
                default:
                    /* Cipher role not supported by X.509. Ignore. */
                    break;
                }
            }
        }
    }

    /* Return the used metadata size. */
    *metadata_size = (*metadata_size - remaining_size);

    return(NX_SUCCESS);
}



UINT _nx_secure_tls_session_create_ext(NX_SECURE_TLS_SESSION *tls_session,
                                   const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                   const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                   VOID *metadata_buffer,
                                   ULONG metadata_size)
{
NX_SECURE_TLS_SESSION          *tail_ptr;
UINT                            i;
UINT                            cipher_table_bytes;
UINT                            status;

UINT                            max_public_cipher_metadata_size  = 0;
UINT                            max_public_auth_metadata_size    = 0;
UINT                            max_session_cipher_metadata_size = 0;
UINT                            max_hash_mac_metadata_size       = 0;
UINT                            max_tls_prf_metadata_size        = 0;
UINT                            max_handshake_hash_metadata_size = 0;
UINT                            max_handshake_hash_scratch_size  = 0;
ULONG                           max_total_metadata_size;
ULONG                           offset;
CHAR                           *metadata_area;

NX_SECURE_TLS_CRYPTO *          crypto_table;

NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_table;
USHORT                          ciphersuite_table_size;
NX_SECURE_X509_CRYPTO           *cert_crypto;
USHORT                          cert_crypto_size;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_CRYPTO_METHOD              **curve_crypto_list = NX_NULL;
USHORT                         *supported_groups = NX_NULL;
USHORT                          ecc_curves_count = 0;
UINT                            supported_groups_bytes;
#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
const NX_CRYPTO_METHOD *crypto_method_md5;
const NX_CRYPTO_METHOD *crypto_method_sha1;
ULONG metadata_size_md5 = 0;
ULONG metadata_size_sha1 = 0;
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
const NX_CRYPTO_METHOD *crypto_method_sha256;
ULONG metadata_size_sha256 = 0;
#endif

    /* Get a working pointer to the metadata buffer. */
    metadata_area = (CHAR*)metadata_buffer;

    /* Check and adjust metadata for four byte alignment. */
    if (((ULONG)metadata_area) & 0x3)
    {
        if (metadata_size < 4 - (((ULONG)metadata_area) & 0x3))
        {
            return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
        }

        metadata_size -= 4 - (((ULONG)metadata_area) & 0x3);
        metadata_area += 4 - (((ULONG)metadata_area) & 0x3);
    }

    if((crypto_array == NX_NULL) || (cipher_map == NX_NULL))
    {

        /* Coming from the old-style API. Don't allocate crypto table. */
        crypto_table = tls_session->nx_secure_tls_crypto_table;

        if (crypto_table == NX_NULL)
        {
            return(NX_PTR_ERROR);
        }

        /* Start by assuming all versions are enabled, remove versions without the appropriate ciphers. */
        tls_session->nx_secure_tls_supported_versions = NX_SECURE_TLS_BITFIELD_VERSIONS_ALL;
    }
    else
    {
        NX_SECURE_MEMSET(tls_session, 0, sizeof(NX_SECURE_TLS_SESSION));

        /* Start by assuming all versions are enabled, remove versions without the appropriate ciphers. */
        tls_session->nx_secure_tls_supported_versions = NX_SECURE_TLS_BITFIELD_VERSIONS_ALL;

        /* Make sure we can allocate our crypto table. */
        if(metadata_size < sizeof(NX_SECURE_TLS_CRYPTO))
        {
            return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
        }

        /* Carve out space for our dynamic crypto table. */
        tls_session->nx_secure_tls_crypto_table = (NX_SECURE_TLS_CRYPTO *)(&metadata_area[0]);

        /* Advance the metadata buffer pointer. */
        metadata_area += sizeof(NX_SECURE_TLS_CRYPTO);
        metadata_size -= sizeof(NX_SECURE_TLS_CRYPTO);

        /* Get a working pointer to our newly-allocated crypto table. */
        crypto_table = tls_session->nx_secure_tls_crypto_table;

        /* Allocate space for our ciphersuite lookup table prior to mapping crypto methods. */
        crypto_table->nx_secure_tls_ciphersuite_lookup_table = (NX_SECURE_TLS_CIPHERSUITE_INFO*)(&metadata_area[0]);
        crypto_table->nx_secure_tls_ciphersuite_lookup_table_size = 0;

        /* Map crypto methods to TLS ciphersuites. */
        cipher_table_bytes = metadata_size;
        status = _map_tls_ciphersuites(tls_session, crypto_array, crypto_array_size, cipher_map, cipher_map_size, &cipher_table_bytes);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Update metadata pointers. */
        metadata_area += cipher_table_bytes;
        metadata_size -= cipher_table_bytes;
        cipher_table_bytes = metadata_size;

        /* Carve out space for our dynamic X.509 ciphersuite table. */
        crypto_table->nx_secure_tls_x509_cipher_table = (NX_SECURE_X509_CRYPTO*)(&metadata_area[0]);
        crypto_table->nx_secure_tls_x509_cipher_table_size = 0;

        /* Map crypto methods to X.509 ciphersuites. */
        status = _map_x509_ciphersuites(tls_session, crypto_array, crypto_array_size, cipher_map, cipher_map_size, &cipher_table_bytes);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Advance the metadata area past the end of the crypto table. */
        metadata_area += cipher_table_bytes;
        metadata_size -= cipher_table_bytes;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        curve_crypto_list = (NX_CRYPTO_METHOD **)(&metadata_area[0]);

        /* Find ECC curves in the crypto array. */
        for (i = 0; i < crypto_array_size; i++)
        {
            if ((crypto_array[i] -> nx_crypto_algorithm & 0xFFFF0000) == NX_CRYPTO_EC_MASK)
            {
                if (metadata_size < sizeof(NX_CRYPTO_METHOD *))
                {
                    return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
                }
                curve_crypto_list[ecc_curves_count] = (NX_CRYPTO_METHOD *)crypto_array[i];
                ecc_curves_count++;
                metadata_size -= sizeof(NX_CRYPTO_METHOD *);
            }
        }

        if (ecc_curves_count > 0)
        {
            metadata_area += ecc_curves_count * sizeof(NX_CRYPTO_METHOD *);
            supported_groups = (USHORT *)(&metadata_area[0]);

            supported_groups_bytes = ecc_curves_count * sizeof(USHORT);

            /* Align length to 4 bytes. */
            if (supported_groups_bytes & 0x3)
            {
                supported_groups_bytes += 4 - (supported_groups_bytes & 0x3);
            }

            if (metadata_size < supported_groups_bytes)
            {
                return(NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE);
            }

            metadata_area += supported_groups_bytes;
            metadata_size -= supported_groups_bytes;

            for (i = 0; i < ecc_curves_count; i++)
            {
                supported_groups[i] = (USHORT)(curve_crypto_list[i] -> nx_crypto_algorithm & 0xFFFF);
            }

            _nx_secure_tls_ecc_initialize(tls_session, supported_groups, ecc_curves_count, (const NX_CRYPTO_METHOD **)curve_crypto_list);
        }
#endif

    }

    /* Get working pointers to our crypto methods. */
    ciphersuite_table = crypto_table -> nx_secure_tls_ciphersuite_lookup_table;
    ciphersuite_table_size = crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;

    cert_crypto = crypto_table -> nx_secure_tls_x509_cipher_table;
    cert_crypto_size = crypto_table -> nx_secure_tls_x509_cipher_table_size;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    crypto_method_md5 = crypto_table -> nx_secure_tls_handshake_hash_md5_method;
    crypto_method_sha1 = crypto_table -> nx_secure_tls_handshake_hash_sha1_method;

    if (crypto_method_md5 != NX_NULL)
    {
        metadata_size_md5 = crypto_method_md5 -> nx_crypto_metadata_area_size;

        /* Align metadata size to four bytes. */
        if (metadata_size_md5 & 0x3)
        {
            metadata_size_md5 += 4 - (metadata_size_md5 & 0x3);
        }
    }

    if (crypto_method_sha1 != NX_NULL)
    {
        metadata_size_sha1 = crypto_method_sha1 -> nx_crypto_metadata_area_size;

        if (metadata_size_sha1 & 0x3)
        {
            metadata_size_sha1 += 4 - (metadata_size_sha1 & 0x3);
        }
    }
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    crypto_method_sha256 = crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
    if (crypto_method_sha256 != NX_NULL)
    {
        metadata_size_sha256 = crypto_method_sha256 -> nx_crypto_metadata_area_size;

        if (metadata_size_sha256 & 0x3)
        {
            metadata_size_sha256 += 4 - (metadata_size_sha256 & 0x3);
        }
    }
#endif
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    tls_session -> nx_secure_tls_1_3_supported = NX_FALSE;
#endif

    /* Loop through the ciphersuite table and find the largest metadata for each type of cipher. */
    for (i = 0; i < ciphersuite_table_size; ++i)
    {
        if (max_public_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size)
        {
            max_public_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_public_auth_metadata_size < ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size;
        }

        if (max_session_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size)
        {
            max_session_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_tls_prf_metadata_size < ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size)
        {
            max_tls_prf_metadata_size = ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size;
        }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if ((ciphersuite_table[i].nx_secure_tls_ciphersuite >> 8) == 0x13)
        {

            /* Enable TLS 1.3 only if the ciphersuite required by RFC 8446 is provided. */
            tls_session->nx_secure_tls_1_3_supported = NX_TRUE;
        }
#endif
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    tls_session -> nx_secure_tls_1_3 = tls_session -> nx_secure_tls_1_3_supported;
#endif

    /* Loop through the certificate cipher table as well. */
    for (i = 0; i < cert_crypto_size; ++i)
    {
        if (max_public_auth_metadata_size < cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }

        if (max_handshake_hash_scratch_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_handshake_hash_scratch_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }
    }

    /* We also need metadata space for the TLS handshake hash, so add that into the total.
       We need some scratch space to copy the handshake hash metadata during final hash generation
       so figure out the largest metadata between SHA-1+MD5 (TLSv1.0, 1.1) and SHA256 (TLSv1.2). */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    if (tls_session -> nx_secure_tls_supported_versions & (USHORT)(NX_SECURE_TLS_BITFIELD_VERSION_1_0 | NX_SECURE_TLS_BITFIELD_VERSION_1_1))
    {
        max_handshake_hash_metadata_size += (metadata_size_md5 + metadata_size_sha1);
        if (max_handshake_hash_scratch_size < metadata_size_md5 + metadata_size_sha1)
        {
            max_handshake_hash_scratch_size = metadata_size_md5 + metadata_size_sha1;
        }

        if (crypto_table -> nx_secure_tls_prf_1_method != NX_NULL)
        {
            if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size)
            {
                max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size;
            }
        }
    }
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    max_handshake_hash_metadata_size += metadata_size_sha256;

    /* See if the scratch size from above is bigger. */
    if (max_handshake_hash_scratch_size < metadata_size_sha256)
    {
        max_handshake_hash_scratch_size = metadata_size_sha256;
    }

    if ((crypto_table -> nx_secure_tls_prf_sha256_method != NX_NULL) &&
        (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size))
    {
        max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size;
    }
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (crypto_table -> nx_secure_tls_hmac_method != NX_NULL)
    {
        max_handshake_hash_scratch_size += crypto_table -> nx_secure_tls_hmac_method -> nx_crypto_metadata_area_size;
    }

    if (crypto_table -> nx_secure_tls_hkdf_method != NX_NULL)
    {
        if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_hkdf_method -> nx_crypto_metadata_area_size)
        {
            max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_hkdf_method -> nx_crypto_metadata_area_size;
        }
    }
#endif

    /* The public cipher and authentication should never be used simultaneously, so it should be OK
         to share their metadata areas. */
    if (max_public_cipher_metadata_size < max_public_auth_metadata_size)
    {
        max_public_cipher_metadata_size = max_public_auth_metadata_size;
    }

    /* Check and adjust every metadata sizes for four byte alignment. */
    if (max_public_cipher_metadata_size & 0x3)
    {
        max_public_cipher_metadata_size += 4 - (max_public_cipher_metadata_size & 0x3);
    }
    if (max_session_cipher_metadata_size & 0x3)
    {
        max_session_cipher_metadata_size += 4 - (max_session_cipher_metadata_size & 0x3);
    }
    if (max_hash_mac_metadata_size & 0x3)
    {
        max_hash_mac_metadata_size += 4 - (max_hash_mac_metadata_size & 0x3);
    }
    if (max_tls_prf_metadata_size & 0x3)
    {
        max_tls_prf_metadata_size += 4 - (max_tls_prf_metadata_size & 0x3);
    }
    if (max_handshake_hash_scratch_size & 0x3)
    {
        max_handshake_hash_scratch_size += 4 - (max_handshake_hash_scratch_size & 0x3);
    }

    /* The Total metadata size needed is the sum of all the maximums calculated above.
       We need to keep track of two separate session cipher states, one for the server and one for the client,
       so account for that extra space. */
    max_total_metadata_size = max_public_cipher_metadata_size +
                              (2 * max_session_cipher_metadata_size) +
                              max_hash_mac_metadata_size +
                              max_tls_prf_metadata_size +
                              max_handshake_hash_metadata_size +
                              max_handshake_hash_scratch_size;

    /* Check if the caller provided enough metadata space. */
    if (max_total_metadata_size > metadata_size)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);


    /* Clear out the X509 certificate stores when we create a new TLS Session. */
    tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates = NX_NULL;
    tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_local_certificates = NX_NULL;
    tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_trusted_certificates = NX_NULL;
    tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate = NX_NULL;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    /* Initialize/reset all the other TLS state. */
    _nx_secure_tls_session_reset(tls_session);

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Now allocate cipher metadata space from our calculated numbers above. */
    offset = 0;

    /* Handshake hash metadata. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    if (tls_session -> nx_secure_tls_supported_versions & (USHORT)(NX_SECURE_TLS_BITFIELD_VERSION_1_0 | NX_SECURE_TLS_BITFIELD_VERSION_1_1))
    {
        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata = &metadata_area[offset];
        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size = metadata_size_md5;
        offset += metadata_size_md5;

        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata = &metadata_area[offset];
        tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size = metadata_size_sha1;
        offset += metadata_size_sha1;
    }
#endif

#if NX_SECURE_TLS_TLS_1_2_ENABLED
    tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata = &metadata_area[offset];
    tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size = metadata_size_sha256;
    offset += metadata_size_sha256;
#endif

    tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch = &metadata_area[offset];
    tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size = max_handshake_hash_scratch_size;
    offset += max_handshake_hash_scratch_size;

    /* Client and server session cipher metadata. */
    tls_session -> nx_secure_session_cipher_metadata_size = max_session_cipher_metadata_size;

    tls_session -> nx_secure_session_cipher_metadata_area_client = &metadata_area[offset];
    offset += max_session_cipher_metadata_size;

    tls_session -> nx_secure_session_cipher_metadata_area_server = &metadata_area[offset];
    offset += max_session_cipher_metadata_size;

    /* Public cipher metadata. */
    tls_session -> nx_secure_public_cipher_metadata_area = &metadata_area[offset];
    tls_session -> nx_secure_public_cipher_metadata_size = max_public_cipher_metadata_size;
    offset += max_public_cipher_metadata_size;

    /* Public authentication metadata. For now it shares space with the public cipher. */
    tls_session -> nx_secure_public_auth_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
    tls_session -> nx_secure_public_auth_metadata_size = max_public_cipher_metadata_size;

    /* Hash MAC metadata. */
    tls_session -> nx_secure_hash_mac_metadata_area = &metadata_area[offset];
    tls_session -> nx_secure_hash_mac_metadata_size = max_hash_mac_metadata_size;
    offset += max_hash_mac_metadata_size;

    /* TLS PRF metadata. */
    tls_session -> nx_secure_tls_prf_metadata_area = &metadata_area[offset];
    tls_session -> nx_secure_tls_prf_metadata_size = max_tls_prf_metadata_size;

    /* Place the new TLS control block on the list of created TLS. */
    if (_nx_secure_tls_created_ptr)
    {

        /* Pickup tail pointer. */
        tail_ptr = _nx_secure_tls_created_ptr -> nx_secure_tls_created_previous;

        /* Place the new TLS control block in the list. */
        _nx_secure_tls_created_ptr -> nx_secure_tls_created_previous = tls_session;
        tail_ptr -> nx_secure_tls_created_next = tls_session;

        /* Setup this TLS's created links. */
        tls_session -> nx_secure_tls_created_previous = tail_ptr;
        tls_session -> nx_secure_tls_created_next = _nx_secure_tls_created_ptr;
    }
    else
    {

        /* The created TLS list is empty. Add TLS control block to empty list. */
        _nx_secure_tls_created_ptr = tls_session;
        tls_session -> nx_secure_tls_created_previous = tls_session;
        tls_session -> nx_secure_tls_created_next = tls_session;
    }
    _nx_secure_tls_created_count++;

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* Flag to indicate when a session renegotiation is enabled. Enabled by default. */
    if (tls_session -> nx_secure_tls_1_3)
    {
        tls_session -> nx_secure_tls_renegotation_enabled = NX_FALSE;
    }
    else
#endif
    {
        tls_session -> nx_secure_tls_renegotation_enabled = NX_TRUE;
    }
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

    /* Set the secret generation functions to the default implementation. */
    tls_session -> nx_secure_generate_premaster_secret = _nx_secure_generate_premaster_secret;
    tls_session -> nx_secure_generate_master_secret = _nx_secure_generate_master_secret;
    tls_session -> nx_secure_generate_session_keys = _nx_secure_generate_session_keys;
    tls_session -> nx_secure_session_keys_set = _nx_secure_session_keys_set;
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    tls_session -> nx_secure_process_server_key_exchange = _nx_secure_process_server_key_exchange;
    tls_session -> nx_secure_generate_client_key_exchange = _nx_secure_generate_client_key_exchange;
#endif
#ifndef NX_SECURE_TLS_SERVER_DISABLED
    tls_session -> nx_secure_process_client_key_exchange = _nx_secure_process_client_key_exchange;
    tls_session -> nx_secure_generate_server_key_exchange = _nx_secure_generate_server_key_exchange;
#endif
    tls_session -> nx_secure_verify_mac = _nx_secure_verify_mac;
    tls_session -> nx_secure_remote_certificate_verify = _nx_secure_remote_certificate_verify;
    tls_session -> nx_secure_trusted_certificate_add = _nx_secure_trusted_certificate_add;

#ifdef NX_SECURE_CUSTOM_SECRET_GENERATION

    /* Customized secret generation functions can be set by the user in nx_secure_custom_secret_generation_init. */
    status = nx_secure_custom_secret_generation_init(tls_session);
    if (status != NX_SUCCESS)
    {
        return(status);
    }
#endif

    /* Set ID to check initialization status. */
    tls_session -> nx_secure_tls_id = NX_SECURE_TLS_ID;

    /* Create the mutex used for TLS session while transmitting packets. */
    tx_mutex_create(&(tls_session -> nx_secure_tls_session_transmit_mutex), "TLS transmit mutex", TX_NO_INHERIT);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SUCCESS);
}

