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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Transport Layer Security (TLS)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_crypto_tls_prf_sha384.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_sha384_init                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the PRF crypto module with SHA384.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Crypto Method Object          */
/*    key                                   Key                           */
/*    key_size_in_bits                      Size of the key, in bits      */
/*    handle                                Handle, specified by user     */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_prf_sha384_init(struct NX_CRYPTO_METHOD_STRUCT *method,
                                                       UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                       VOID **handle,
                                                       VOID *crypto_metadata,
                                                       ULONG crypto_metadata_size)
{
NX_CRYPTO_TLS_PRF_SHA384 *prf;
NX_CRYPTO_PHASH *phash;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if ((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_TLS_PRF_SHA384))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Get our control block. */
    prf = (NX_CRYPTO_TLS_PRF_SHA384 *)crypto_metadata;
    phash = &(prf -> nx_secure_tls_prf_phash_info);

    /* Set the secret using the key value. */
    phash -> nx_crypto_phash_secret = key;
    /* This is the length of secret in bytes actually */
    phash -> nx_crypto_phash_secret_length = key_size_in_bits;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_sha384_cleanup                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Crypto metadata               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMSET                      Set the memory                */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_prf_sha384_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_TLS_PRF_SHA384));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_sha384_operation             PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the PRF SHA384 algorithm.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    PRF operation                 */
/*    handle                                Crypto handle                 */
/*    method                                Cryption Method Object        */
/*    key                                   Encryption Key                */
/*    key_size_in_bits                      Key size in bits              */
/*    input                                 Input data                    */
/*    input_length_in_byte                  Input data size               */
/*    iv_ptr                                Initial vector                */
/*    output                                Output buffer                 */
/*    output_length_in_byte                 Output buffer size            */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Metadata area size            */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_phash_process              Implement the PHASH process   */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_prf_sha384_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
                                                            VOID *handle, /* Crypto handler */
                                                            struct NX_CRYPTO_METHOD_STRUCT *method,
                                                            UCHAR *key,
                                                            NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                            UCHAR *input,
                                                            ULONG input_length_in_byte,
                                                            UCHAR *iv_ptr,
                                                            UCHAR *output,
                                                            ULONG output_length_in_byte,
                                                            VOID *crypto_metadata,
                                                            ULONG crypto_metadata_size,
                                                            VOID *packet_ptr,
                                                            VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status))
{
UINT                       status;
NX_CRYPTO_TLS_PRF_SHA384 *prf;
NX_CRYPTO_PHASH *phash;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_TLS_PRF_SHA384))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* This must be a PRF operation. */
    if (op != NX_CRYPTO_PRF)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Get our control block. */
    prf = (NX_CRYPTO_TLS_PRF_SHA384 *)crypto_metadata;
    phash = &(prf -> nx_secure_tls_prf_phash_info);

    /* Install the label_seed_buffer to the phash structure as the buffer of phash seed. */
    phash -> nx_crypto_phash_seed = prf -> nx_secure_tls_prf_label_seed_buffer;
    if ((key_size_in_bits + input_length_in_byte) > sizeof(prf -> nx_secure_tls_prf_label_seed_buffer))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Concatenate label and seed. */
    NX_CRYPTO_MEMCPY(phash -> nx_crypto_phash_seed, key, key_size_in_bits); /* Use case of memcpy is verified. */
    NX_CRYPTO_MEMCPY(&phash -> nx_crypto_phash_seed[key_size_in_bits], input, input_length_in_byte); /* Use case of memcpy is verified. */
    phash -> nx_crypto_phash_seed_length = key_size_in_bits + input_length_in_byte;

    /* Install the temp_A_buffer to the phash structure. */
    phash -> nx_crypto_phash_temp_A = prf -> nx_secure_tls_prf_temp_A_buffer;
    phash -> nx_crypto_phash_temp_A_size = sizeof(prf -> nx_secure_tls_prf_temp_A_buffer);

    /* Install the hmac method to the phash structure. */
    phash -> nx_crypto_hmac_method = &crypto_method_hmac_sha384;

    /* Install metadata buffer for the hmac method. */
    phash -> nx_crypto_hmac_metadata = prf -> nx_secure_tls_prf_hmac_metadata_area;
    phash -> nx_crypto_hmac_metadata_size = sizeof(prf -> nx_secure_tls_prf_hmac_metadata_area);

    /* Install the buffer for hmac output. */
    phash -> nx_crypto_hmac_output = prf -> nx_secure_tls_prf_temp_hmac_output_buffer;
    phash -> nx_crypto_hmac_output_size = sizeof(prf -> nx_secure_tls_prf_temp_hmac_output_buffer);

    /* Clear the output buffer for the generic phash routine will show the output by XOR. */
    NX_CRYPTO_MEMSET(output, 0, output_length_in_byte);

    /* Invoke generic phash routine. */
    status = _nx_crypto_phash(phash, output, output_length_in_byte);

    return status;
}
