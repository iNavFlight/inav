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
/**  HMAC-based Extract-and-Expand Key Derivation Function (HKDF)         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_hkdf.h"
#include "nx_crypto_hmac.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hkdf_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method initialization routine    */
/*    for the Microsoft implementation of the HKDF cryptographic          */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hkdf_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                        VOID  **handle,
                                                        VOID  *crypto_metadata,
                                                        ULONG crypto_metadata_size)
{
    NX_CRYPTO_HKDF       *hkdf;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    /* We don't need a key in the HKDF init. */
    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_POINTER_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_METADATA_UNALIGNED);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_HKDF))
    {
        return(NX_CRYPTO_INVALID_BUFFER_SIZE);
    }

    hkdf = (NX_CRYPTO_HKDF *)crypto_metadata;

    /* Initialize IKM with key data. */
    hkdf->nx_crypto_hkdf_ikm = key;
    hkdf->nx_crypto_hkdf_ikm_length = (key_size_in_bits << 3);

    /* Initialize HMAC and HASH methods. */
    hkdf->nx_crypto_hmac_method = NX_CRYPTO_NULL;
    hkdf->nx_crypto_hash_method = NX_CRYPTO_NULL;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hkdf_cleanup                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata for the HKDF operation. */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hkdf_cleanup(VOID *crypto_metadata)
{
#ifdef NX_SECURE_KEY_CLEAR
NX_CRYPTO_METHOD *hmac_method;
NX_CRYPTO_HKDF *hkdf;
UINT status;
#endif
    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clear the HMAC state. */
    hkdf = (NX_CRYPTO_HKDF *)crypto_metadata;
    hmac_method = hkdf->nx_crypto_hmac_method;

    if(hmac_method)
    {
        status = hmac_method -> nx_crypto_cleanup(hmac_method);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Clean up the HKDF metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_HKDF));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hkdf_operation                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function provides the generic NetX Crypto API for the HKDF     */
/*    operation.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation Type                */
/*                                          Encrypt, Decrypt, Authenticate*/
/*    handler                               Pointer to crypto context     */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    input                                 Input Stream                  */
/*    input_length_in_byte                  Input Stream Length           */
/*    iv_ptr                                Initialized Vector            */
/*    output                                Output Stream                 */
/*    output_length_in_byte                 Output Stream Length          */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_hkdf_extract               Calculate the HKDF key        */
/*    _nx_crypto_hkdf_expand                Generate HKDF key material    */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_hkdf_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_HKDF       *hkdf;
UINT                 status;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_POINTER_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_HKDF))
    {
        return(NX_CRYPTO_INVALID_BUFFER_SIZE);
    }

    /* Get our control block for HKDF. */
    hkdf = (NX_CRYPTO_HKDF *)(crypto_metadata);

    status = NX_CRYPTO_SUCCESS;
    switch (op)
    {
    case NX_CRYPTO_HKDF_SET_HMAC:
        hkdf->nx_crypto_hmac_method = method;
        if(hkdf->nx_crypto_hash_method != NX_CRYPTO_NULL)
        {
            /* Set the HMAC hash method. */
            status = hkdf->nx_crypto_hmac_method->nx_crypto_operation(NX_CRYPTO_HMAC_SET_HASH, NX_CRYPTO_NULL,
                                                                      hkdf->nx_crypto_hash_method, NX_CRYPTO_NULL, 0, NX_CRYPTO_NULL, 0,
                                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL, 0, hkdf->nx_crypto_hmac_metadata,
                                                                      sizeof(hkdf->nx_crypto_hmac_metadata), NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        }
        break;
    case NX_CRYPTO_HKDF_SET_HASH:
        hkdf->nx_crypto_hash_method = method;
        if(hkdf->nx_crypto_hmac_method != NX_CRYPTO_NULL)
        {
            /* Set the HMAC hash method. */
            status = hkdf->nx_crypto_hmac_method->nx_crypto_operation(NX_CRYPTO_HMAC_SET_HASH, NX_CRYPTO_NULL,
                                                                      method, NX_CRYPTO_NULL, 0, NX_CRYPTO_NULL, 0,
                                                                      NX_CRYPTO_NULL, NX_CRYPTO_NULL, 0, hkdf->nx_crypto_hmac_metadata,
                                                                      sizeof(hkdf->nx_crypto_hmac_metadata), NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        }
        break;
    case NX_CRYPTO_HKDF_SET_PRK:
        if(key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_POINTER_ERROR);
        }

        if ((key_size_in_bits >> 3) > sizeof(hkdf->nx_crypto_hkdf_prk))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        /* Set the PRK and return. */
        NX_CRYPTO_MEMCPY(hkdf->nx_crypto_hkdf_prk, key, (key_size_in_bits >> 3)); /* Use case of memcpy is verified. */
        hkdf->nx_crypto_hkdf_prk_size = (key_size_in_bits >> 3);

        break;
    case NX_CRYPTO_HKDF_EXTRACT:
        if(key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_POINTER_ERROR);
        }

        if(hkdf->nx_crypto_hash_method == NX_CRYPTO_NULL || hkdf->nx_crypto_hmac_method == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_METHOD_INITIALIZATION_FAILURE);
        }

        /* Key is our "salt". The IKM should have been passed in the init function.*/
        hkdf->nx_crypto_hkdf_salt = key;
        hkdf->nx_crypto_hkdf_salt_length = (key_size_in_bits >> 3);

        /* Initialize IKM with input data. */
        hkdf->nx_crypto_hkdf_ikm = input;
        hkdf->nx_crypto_hkdf_ikm_length = input_length_in_byte;

        /* Our output size is the output size of the hash. */
        hkdf->nx_crypto_hkdf_prk_size = hkdf->nx_crypto_hmac_method->nx_crypto_block_size_in_bytes;

        status = _nx_crypto_hkdf_extract(hkdf);

        if(status == NX_CRYPTO_SUCCESS)
        {
            if (output_length_in_byte < hkdf->nx_crypto_hkdf_prk_size)
            {
                return(NX_CRYPTO_SIZE_ERROR);
            }

            /* Copy the PRK into output. */
            NX_CRYPTO_MEMCPY(output, hkdf->nx_crypto_hkdf_prk, hkdf->nx_crypto_hkdf_prk_size); /* Use case of memcpy is verified. */
        }

        break;

    case NX_CRYPTO_HKDF_EXPAND:
        if(key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_POINTER_ERROR);
        }

        if(hkdf->nx_crypto_hash_method == NX_CRYPTO_NULL || hkdf->nx_crypto_hmac_method == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_METHOD_INITIALIZATION_FAILURE);
        }

        /* Key is our "info". The PRK should have been initialized by the call to NX_CRYPTO_HKDF_EXTRACT.*/
        hkdf->nx_crypto_hkdf_info = key;
        hkdf->nx_crypto_hkdf_info_size = (key_size_in_bits >> 3);

        status = _nx_crypto_hkdf_expand(hkdf, output, output_length_in_byte);
        break;
    default:
        break;
    }

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hkdf_extract                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the HKDF-extract operation detailed in RFC   */
/*    5869. The output key is placed in the HKDF structure passed in.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hkdf                                  HKDF structure                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [hash method]                         Perform selected HMAC hash    */
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
UINT _nx_crypto_hkdf_extract(NX_CRYPTO_HKDF *hkdf)
{
UINT    status;
UINT    hash_size; /* the length of hmac output */
UCHAR   *salt;
UINT    salt_len;
UCHAR   *ikm;
UINT    ikm_len;
UCHAR   *metadata;
UINT    metadata_size;
UCHAR   *hmac_output;
VOID   *handler = NX_CRYPTO_NULL;
NX_CRYPTO_METHOD *hmac_method = hkdf -> nx_crypto_hmac_method;

    NX_CRYPTO_STATE_CHECK

    /* From RFC 5869:
     * HKDF-Extract(salt, IKM) -> PRK
     *
     * Options:
     *    Hash     a hash function; HashLen denotes the length of the
     *             hash function output in octets
     *
     * Inputs:
     *    salt     optional salt value (a non-secret random value);
     *             if not provided, it is set to a string of HashLen zeros.
     *    IKM      input keying material
     *
     * Output:
     *    PRK      a pseudorandom key (of HashLen octets)
     *
     * The output PRK is calculated as follows:
     *
     * PRK = HMAC-Hash(salt, IKM)
     *
     */


    /* Validate pointers. */
    if (hmac_method == NX_CRYPTO_NULL
        || hmac_method -> nx_crypto_operation == NX_CRYPTO_NULL
        || hmac_method -> nx_crypto_cleanup == NX_CRYPTO_NULL
        || hkdf->nx_crypto_hash_method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Initialize temporary variables. */
    salt = hkdf->nx_crypto_hkdf_salt;
    salt_len = hkdf->nx_crypto_hkdf_salt_length;
    ikm = hkdf->nx_crypto_hkdf_ikm;
    ikm_len = hkdf->nx_crypto_hkdf_ikm_length;
    hash_size = hkdf -> nx_crypto_hash_method -> nx_crypto_ICV_size_in_bits >> 3;

    metadata = hkdf->nx_crypto_hmac_metadata;
    metadata_size = sizeof(hkdf->nx_crypto_hmac_metadata);
    hmac_output = hkdf->nx_crypto_hkdf_prk;

    /* Make sure we can store our output key. */
    if (hash_size > sizeof(hkdf->nx_crypto_hkdf_prk))
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Assign the output size to our HKDF structure. */
    hkdf->nx_crypto_hkdf_prk_size = hash_size;

    /* Initialize hash method (check key sizes, etc.). */
    if (hmac_method -> nx_crypto_init)
    {
        status = hmac_method -> nx_crypto_init(hmac_method,
                                      salt,
                                      (NX_CRYPTO_KEY_SIZE)(salt_len << 3),
                                      &handler,
                                      metadata,
                                      metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Generate the output PRK. */
    status = hmac_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                handler,
                                                hmac_method,
                                                salt,
                                                (NX_CRYPTO_KEY_SIZE)(salt_len << 3),
                                                ikm,
                                                ikm_len,
                                                NX_CRYPTO_NULL,
                                                hmac_output,
                                                hash_size,
                                                metadata,
                                                metadata_size,
                                                NX_CRYPTO_NULL,
                                                NX_CRYPTO_NULL);


    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hkdf_expand                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the HKDF-expand operation detailed in RFC    */
/*    5869. The hdkf parameter contains the input key (PRK) and other     */
/*    parameters needed to generate the desired output data.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hkdf                                  HKDF structure                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [hash method]                         Perform selected HMAC hash    */
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
UINT _nx_crypto_hkdf_expand(NX_CRYPTO_HKDF *hkdf, UCHAR *output, UINT desired_length)
{
UINT    offset;
UINT    N_count;
UINT    hash_size; /* the length of hmac output */
UCHAR   *prk;
UINT    prk_len;
UCHAR   *info;
UINT    info_len;
UCHAR   *metadata;
UINT    metadata_size;
VOID   *handler = NX_CRYPTO_NULL;
UINT    output_len;
/* T(i) */
UCHAR   *temp_T;
UINT    temp_T_size, T_len, T_bytes_to_hash;
UINT    i;
UINT    status;

NX_CRYPTO_METHOD *hmac_method = hkdf -> nx_crypto_hmac_method;

    NX_CRYPTO_STATE_CHECK

    /* From RFC 5869:
     * HKDF-Expand(PRK, info, L) -> OKM
     *
     *  Options:
     *     Hash     a hash function; HashLen denotes the length of the
     *              hash function output in octets
     *  Inputs:
     *     PRK      a pseudorandom key of at least HashLen octets
     *              (usually, the output from the extract step)
     *     info     optional context and application specific information
     *              (can be a zero-length string)
     *     L        length of output keying material in octets
     *              (<= 255*HashLen)
     *
     *  Output:
     *     OKM      output keying material (of L octets)
     *
     *  The output OKM is calculated as follows:
     *
     *  N = ceil(L/HashLen)
     *  T = T(1) | T(2) | T(3) | ... | T(N)
     *  OKM = first L octets of T
     *
     *  where:
     *  T(0) = empty string (zero length)
     *  T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
     *  T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
     *  T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
     *  ...
     *
     *  (where the constant concatenated to the end of each T(n) is a
     *  single octet.)
     *
     */


    /* Validate pointers. */
    if (hmac_method == NX_CRYPTO_NULL
        || hmac_method -> nx_crypto_operation == NX_CRYPTO_NULL
        || hmac_method -> nx_crypto_cleanup == NX_CRYPTO_NULL
        || hkdf->nx_crypto_hash_method == NX_CRYPTO_NULL
        || output == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Initialize temporary variables. */
    prk = hkdf->nx_crypto_hkdf_prk;
    prk_len = hkdf->nx_crypto_hkdf_prk_size;
    info = hkdf->nx_crypto_hkdf_info;
    info_len = hkdf->nx_crypto_hkdf_info_size;
    temp_T = hkdf->nx_crypto_hkdf_temp_T;
    temp_T_size = sizeof(hkdf->nx_crypto_hkdf_temp_T);
    hash_size = hkdf-> nx_crypto_hash_method -> nx_crypto_ICV_size_in_bits >> 3;
    metadata = hkdf->nx_crypto_hmac_metadata;
    metadata_size = sizeof(hkdf->nx_crypto_hmac_metadata);

    /* Assign the output size to our HKDF structure. */
    hkdf->nx_crypto_hkdf_prk_size = hash_size;

    /* Assign T(0), the empty string. */
    NX_CRYPTO_MEMSET(temp_T, 0, temp_T_size);
    T_len = 0;

    /* Get our L count for our loop. */
    N_count = 1 + ((desired_length) / hash_size);

    /* Loop through T(i) to calculate output material (OKM).
     * NOTE: We start at 1 so the counter is correct. Add one
     * to N_count to get the full amount of data. */
    for (i = 1; i < N_count + 1; ++i)
    {
        if ((T_len + info_len + 1) > temp_T_size)
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        /* Concatenate T(i-1) (in temp_T after the hash above), info, and counter octet to feed into digest. */
        NX_CRYPTO_MEMCPY(&temp_T[T_len], info, info_len); /* Use case of memcpy is verified. */

        /* Concatenate counter octet. */
        temp_T[T_len + info_len] = (UCHAR)(i & 0xFF);

        /* Initialize hash method. */
        if (hmac_method -> nx_crypto_init)
        {
            status = hmac_method -> nx_crypto_init(hmac_method,
                                                   prk,
                                                   (NX_CRYPTO_KEY_SIZE)(prk_len << 3),
                                                   &handler,
                                                   metadata,
                                                   metadata_size);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        /* The number of bytes we want to hash is a combination of T_len (0 or <hash size>)
           the length of "info", and add 1 for the counter octet. */
        T_bytes_to_hash = T_len + info_len + 1;

        /* Calculate T(i) = HMAC(PRK, T(i-1) | info | i) */
        status = hmac_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                                    handler,
                                                    hmac_method,
                                                    prk,
                                                    (NX_CRYPTO_KEY_SIZE)(prk_len << 3),
                                                    temp_T,
                                                    T_bytes_to_hash,
                                                    NX_CRYPTO_NULL,
                                                    temp_T,
                                                    temp_T_size,
                                                    metadata,
                                                    metadata_size,
                                                    NX_CRYPTO_NULL,
                                                    NX_CRYPTO_NULL);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Updated the length of T(i) */
        T_len = hash_size;

        /* Get our output offset. */
        offset = (i - 1) * hash_size;

        /* Output block is the size of the digest unless the remaining
           desired length is smaller than the digest length. */
        if ((desired_length - offset) < hash_size)
        {
            output_len = (desired_length - offset);
        }
        else
        {
            output_len = hash_size;
        }

        /* Make sure we only copy the desired data length into the output. */
        if (hash_size > desired_length)
        {
            output_len  = desired_length;
        }        
        
        /* Copy T(i) into output. */
        NX_CRYPTO_MEMCPY(&output[offset], temp_T, output_len); /* Use case of memcpy is verified. */

    }

    return(NX_CRYPTO_SUCCESS);


}




