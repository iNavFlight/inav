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
/**   HMAC Mode                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_hmac.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac                                     PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the HMAC.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    input_ptr                             input byte stream             */
/*    input_length                          input byte stream length      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*    digest_ptr                            generated crypto digest       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_hmac_initialize            Perform HMAC initialization   */
/*    _nx_crypto_hmac_update                Perform HMAC update           */
/*    _nx_crypto_hmac_digest_calculate      Calculate HMAC digest         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac(NX_CRYPTO_HMAC *hmac_metadata,
                                    UCHAR *input_ptr, UINT input_length,
                                    UCHAR *key_ptr, UINT key_length,
                                    UCHAR *digest_ptr, UINT digest_length)
{

    /* Initialize, update and calculate.  */
    _nx_crypto_hmac_initialize(hmac_metadata, key_ptr, key_length);
    _nx_crypto_hmac_update(hmac_metadata, input_ptr, input_length);
    _nx_crypto_hmac_digest_calculate(hmac_metadata, digest_ptr, digest_length);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_initialize                          PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC initialization.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_digest_calculate]             Calculate crypto digest       */
/*    [crypto_initialize]                   Perform crypto initialization */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
/*    _nx_crypto_method_hmac_sha512_operation Handle HMAC-SHA512 operation*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_initialize(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *key_ptr, UINT key_length)
{
UCHAR temp_key[128];
UINT  i;

    /* If key is longer than block size, reset it to key=CRYPTO(key). */
    if (key_length > hmac_metadata -> block_size)
    {

        hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

        hmac_metadata -> crypto_update(hmac_metadata -> context, key_ptr, key_length);

        hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, temp_key, hmac_metadata -> algorithm);

        key_ptr = temp_key;

        key_length = hmac_metadata -> output_length;
    }

    hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

    /* The HMAC_CRYPTO transform looks like:

       CRYPTO(K XOR opad, CRYPTO(K XOR ipad, text))

       where K is an n byte key,
       ipad is the byte 0x36 repeated block_size times,
       opad is the byte 0x5c repeated block_size times,
       and text is the data being protected.      */

    NX_CRYPTO_MEMSET(hmac_metadata -> k_ipad, 0, hmac_metadata -> block_size);

    NX_CRYPTO_MEMSET(hmac_metadata -> k_opad, 0, hmac_metadata -> block_size);

    NX_CRYPTO_MEMCPY(hmac_metadata -> k_ipad, key_ptr, key_length); /* Use case of memcpy is verified. */

    NX_CRYPTO_MEMCPY(hmac_metadata -> k_opad, key_ptr, key_length); /* Use case of memcpy is verified. */


    /* XOR key with ipad and opad values. */
    for (i = 0; i < hmac_metadata -> block_size; i++)
    {
        hmac_metadata -> k_ipad[i] ^= 0x36;
        hmac_metadata -> k_opad[i] ^= 0x5c;
    }

    /* Kick off the inner hash with our padded key. */
    hmac_metadata -> crypto_update(hmac_metadata -> context, hmac_metadata -> k_ipad, hmac_metadata -> block_size);

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(temp_key, 0, sizeof(temp_key));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_update                              PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC update.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    input_ptr                             input byte stream             */
/*    input_length                          input byte stream length      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_update(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *input_ptr, UINT input_length)
{

    /* Update inner CRYPTO. */
    hmac_metadata -> crypto_update(hmac_metadata -> context, input_ptr, input_length);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_digest_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs HMAC digest calculation.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    digest_ptr                            generated crypto digest       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [crypto_digest_calculate]             Calculate crypto digest       */
/*    [crypto_initialize]                   Perform crypto initialization */
/*    [crypto_update]                       Perform crypto update         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_hmac                       Calculate the HMAC            */
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
NX_CRYPTO_KEEP UINT _nx_crypto_hmac_digest_calculate(NX_CRYPTO_HMAC *hmac_metadata, UCHAR *digest_ptr, UINT digest_length)
{
UCHAR icv_ptr[64];

    /* Perform outer CRYPTO. */

    hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, icv_ptr, hmac_metadata -> algorithm);

    hmac_metadata -> crypto_initialize(hmac_metadata -> context, hmac_metadata -> algorithm);

    hmac_metadata -> crypto_update(hmac_metadata -> context, hmac_metadata -> k_opad, hmac_metadata -> block_size);

    hmac_metadata -> crypto_update(hmac_metadata -> context, icv_ptr, hmac_metadata -> output_length);

    hmac_metadata -> crypto_digest_calculate(hmac_metadata -> context, icv_ptr, hmac_metadata -> algorithm);

    NX_CRYPTO_MEMCPY(digest_ptr, icv_ptr,  (digest_length > hmac_metadata -> output_length ? hmac_metadata -> output_length : digest_length)); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(icv_ptr, 0, sizeof(icv_ptr));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_metadata_set                        PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets HMAC metadata.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    context                               crypto context                */
/*    k_ipad                                ipad key                      */
/*    k_opad                                opad key                      */
/*    algorithm                             algorithm                     */
/*    block_size                            block size                    */
/*    output_length                         output length                 */
/*    crypto_intitialize                    initializtion function        */
/*    crypto_update                         update function               */
/*    crypto_digest_calculate               digest calculation function   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_crypto_method_hmac_md5_operation  Handle HMAC-MD5 operation     */
/*    _nx_crypto_method_hmac_sha1_operation Handle HMAC-SHA1 operation    */
/*    _nx_crypto_method_hmac_sha256_operation Handle HMAC-SHA256 operation*/
/*    _nx_crypto_method_hmac_sha512_operation Handle HMAC-SHA512 operation*/
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
NX_CRYPTO_KEEP VOID _nx_crypto_hmac_metadata_set(NX_CRYPTO_HMAC *hmac_metadata,
                                                 VOID *context,
                                                 UINT algorithm, UINT block_size, UINT output_length,
                                                 UINT (*crypto_initialize)(VOID *, UINT),
                                                 UINT (*crypto_update)(VOID *, UCHAR *, UINT),
                                                 UINT (*crypto_digest_calculate)(VOID *, UCHAR *, UINT))
{

    hmac_metadata -> context = context;
    hmac_metadata -> algorithm = algorithm;
    hmac_metadata -> block_size = block_size;
    hmac_metadata -> output_length = output_length;
    hmac_metadata -> crypto_initialize = crypto_initialize;
    hmac_metadata -> crypto_update = crypto_update;
    hmac_metadata -> crypto_digest_calculate = crypto_digest_calculate;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_hash_initialize                     PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is a build-in wrappers for hash initialization.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               Crypto context                */
/*    algorithm                             Hash algorithm                */
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
/*    None                                                                */
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
UINT _nx_crypto_hmac_hash_initialize(VOID *context, UINT algorithm)
{
NX_CRYPTO_HMAC *hmac;
NX_CRYPTO_METHOD *hash;
UINT   metadata_size;
UINT   status;

    NX_CRYPTO_PARAMETER_NOT_USED(algorithm);

    /* Get the HMAC context. */
    hmac = (NX_CRYPTO_HMAC*)context;

    /* Get the hash method and it's metadata. */
    hash = hmac->hash_method;
    metadata_size = hash->nx_crypto_metadata_area_size;

    status = hash->nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                       NX_CRYPTO_NULL,
                                       hash,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hmac->hash_context,
                                       metadata_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);


    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_hash_update                         PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is a build-in wrappers for hash data update.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               Crypto context                */
/*    input                                 Input Stream                  */
/*    input_length                          Input Stream Length           */
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
/*    None                                                                */
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
UINT   _nx_crypto_hmac_hash_update(VOID *context, UCHAR *input, UINT input_length)
{
NX_CRYPTO_HMAC *hmac;
NX_CRYPTO_METHOD *hash;
UINT   metadata_size;
UINT   status;

    /* Get the HMAC context. */
    hmac = (NX_CRYPTO_HMAC*)context;

    /* Get the hash method and it's metadata. */
    hash = hmac->hash_method;
    metadata_size = hash->nx_crypto_metadata_area_size;

    /* Perform an update using the generic crypto API. */
    status = hash->nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                       NX_CRYPTO_NULL,
                                       hash,
                                       NX_CRYPTO_NULL,
                                       0,
                                       input,
                                       input_length,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL,
                                       0,
                                       hmac->hash_context,
                                       metadata_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_hash_digest_calculate               PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is a build-in wrappers for hash digest calculate.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               Crypto context                */
/*    digest                                Digest for output             */
/*    algorithm                             Hash algorithm                */
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
/*    None                                                                */
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
UINT   _nx_crypto_hmac_hash_digest_calculate(VOID *context, UCHAR *digest, UINT algorithm)
{
NX_CRYPTO_HMAC *hmac;
NX_CRYPTO_METHOD *hash;
UINT   metadata_size;
UINT   status;

    NX_CRYPTO_PARAMETER_NOT_USED(algorithm);

    /* Get the HMAC context. */
    hmac = (NX_CRYPTO_HMAC*)context;

    /* Get the hash method and it's metadata. */
    hash = hmac->hash_method;
    metadata_size = hash->nx_crypto_metadata_area_size;

    /* Perform a digest calculation using the generic crypto API. */
    status = hash->nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                       NX_CRYPTO_NULL,
                                       hash,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       0,
                                       NX_CRYPTO_NULL,
                                       digest,
                                       (hash->nx_crypto_ICV_size_in_bits >> 3),
                                       hmac->hash_context,
                                       metadata_size,
                                       NX_CRYPTO_NULL,
                                       NX_CRYPTO_NULL);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method initialization routine    */
/*    for the Microsoft implementation of the HMAC cryptographic          */
/*    algorithm.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to crypto method      */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    handler                               Returned crypto handler       */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hmac_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                        VOID  **handle,
                                                        VOID  *crypto_metadata,
                                                        ULONG crypto_metadata_size)
{
/*    NX_CRYPTO_HMAC       *hmac;*/

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_HMAC))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

#if 0
    hmac = (NX_CRYPTO_HMAC *)crypto_metadata;

    /* Set the method for later. */
    hmac->hash_method = method;
#endif

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_cleanup                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata for the HMAC operation. */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hmac_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_HMAC));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}





/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_operation                    PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the generic operation for HMAC. HMAC does not care */
/*    what hash is used as long as the hash size is known. Therefore, this*/
/*    method may be used with an arbitrary hash routine as long as the    */
/*    hash has an NX_CRYPTO_METHOD instance properly filled out.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hmac_metadata                         pointer to HMAC metadata      */
/*    context                               crypto context                */
/*    k_ipad                                ipad key                      */
/*    k_opad                                opad key                      */
/*    algorithm                             algorithm                     */
/*    block_size                            block size                    */
/*    output_length                         output length                 */
/*    crypto_intitialize                    initializtion function        */
/*    crypto_update                         update function               */
/*    crypto_digest_calculate               digest calculation function   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_hmac_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_HMAC       *hmac;
UINT                 status;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_HMAC))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Get our control block for HKDF. */
    hmac = (NX_CRYPTO_HMAC *)(crypto_metadata);

    /* Get the metadata for our hash routine. */
    hmac->context = hmac;
    hmac->hash_context = ((UCHAR*)(crypto_metadata)) + sizeof(NX_CRYPTO_HMAC);

    status = NX_CRYPTO_SUCCESS;
    switch (op)
    {
    case NX_CRYPTO_HMAC_SET_HASH:
        hmac->hash_method = method;

        /* Set up the HMAC metadata using the HMAC context for the hash context - this is because our built-in routines
         * (_nx_crypto_hmac_hash_initialize/update/digest_calculate) need the full HMAC context, unlike when HMAC routines
         * are built around HMAC (as opposed to when HMAC is used as a hash wrapper). */
        _nx_crypto_hmac_metadata_set(hmac, hmac,
                                     method->nx_crypto_algorithm, method->nx_crypto_block_size_in_bytes, method->nx_crypto_ICV_size_in_bits >> 3,
                                     _nx_crypto_hmac_hash_initialize,
                                     _nx_crypto_hmac_hash_update,
                                     _nx_crypto_hmac_hash_digest_calculate);

        /* Initialize the hash routine. */
        status = method->nx_crypto_init(method, NX_CRYPTO_NULL, 0, NX_CRYPTO_NULL, hmac->hash_context, method->nx_crypto_metadata_area_size);
        break;

    case NX_CRYPTO_HASH_INITIALIZE:
        /* Initialize the hash method. */
        if(key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        status = _nx_crypto_hmac_initialize(hmac, key, key_size_in_bits >> 3);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        status = _nx_crypto_hmac_update(hmac, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        if(output_length_in_byte == 0)
        {
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }
        status = _nx_crypto_hmac_digest_calculate(hmac, output,
                                                  (output_length_in_byte > (ULONG)((hmac-> hash_method -> nx_crypto_ICV_size_in_bits) >> 3) ?
                                                  ((hmac-> hash_method -> nx_crypto_ICV_size_in_bits) >> 3) : output_length_in_byte));
        break;

    default:
        /* Do entire HMAC operation in one pass (init, update, calculate). */
        if(key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        if(output_length_in_byte == 0)
        {
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }
        _nx_crypto_hmac(hmac, input, input_length_in_byte, key, (key_size_in_bits >> 3), output,
                        (output_length_in_byte > (ULONG)((hmac -> hash_method -> nx_crypto_ICV_size_in_bits) >> 3) ?
                        ((hmac -> hash_method -> nx_crypto_ICV_size_in_bits) >> 3) : output_length_in_byte));
        break;
    }

    return(status);



}

