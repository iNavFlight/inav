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
/**   3DES Encryption Standard (Triple DES)                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_3des.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_3des_key_set                             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up the encryption keys for the 3DES algorithm.   */
/*    It must be called before either _nx_crypto_3des_encrypt or          */
/*    _nx_crypto_3des_decrypt can be called.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               3DES context pointer          */
/*    key                                   24 bytes key                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_des_key_set                Set the key for DES           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_3des_init           Init the method for 3DES      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_3des_key_set(NX_CRYPTO_3DES *context, UCHAR key[24])
{

    _nx_crypto_des_key_set(&context -> des_1, key); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_key_set(&context -> des_2, key + 8); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_key_set(&context -> des_3, key + 16); /* lgtm[cpp/weak-cryptographic-algorithm] */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_3des_encrypt                             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function uses the 3DES algorithm to encrypt 8-bytes (64-bits). */
/*    The result is 8 encrypted bytes. Note that the caller must make     */
/*    sure the source and destination are 8-bytes in size!                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               3DES context pointer          */
/*    source                                8-byte source                 */
/*    destination                           8-byte destination            */
/*    length                                The length of output buffer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_des_encrypt                Perform DES mode encryption   */
/*    _nx_crypto_des_decrypt                Perform DES mode decryption   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_3des_encrypt(NX_CRYPTO_3DES *context, UCHAR source[8],
                                             UCHAR destination[8], UINT length)
{

    /* ciphertext = EK3(DK2(EK1(plaintext)))
       I.e., DES encrypt with K1, DES decrypt with K2, then DES encrypt with K3. */
    _nx_crypto_des_encrypt(&context -> des_1, source, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_decrypt(&context -> des_2, destination, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_encrypt(&context -> des_3, destination, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */

    /* Return successful completion.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_3des_decrypt                             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function uses the 3DES algorithm to decrypt 8-bytes (64-bits). */
/*    The result is 8 original source bytes. Note that the caller must    */
/*    make sure the source and destination are 8-bytes in size!           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               3DES context pointer          */
/*    source                                8-byte source                 */
/*    destination                           8-byte destination            */
/*    length                                The length of output buffer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_des_encrypt                Perform DES mode encryption   */
/*    _nx_crypto_des_decrypt                Perform DES mode decryption   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_3des_decrypt(NX_CRYPTO_3DES *context, UCHAR source[8],
                                             UCHAR destination[8], UINT length)
{


    /*
       plaintext = DK1(EK2(DK3(ciphertext)))
       I.e., decrypt with K3, encrypt with K2, then decrypt with K1.
     */
    _nx_crypto_des_decrypt(&context -> des_3, source, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_encrypt(&context -> des_2, destination, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */

    _nx_crypto_des_decrypt(&context -> des_1, destination, destination, length); /* lgtm[cpp/weak-cryptographic-algorithm] */


    /* Return successful completion.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_3des_init                         PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported 3DES cryptographic algorithm.                   */
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
/*    _nx_crypto_3des_key_set               Set the key for 3DES          */
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
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_3des_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                 VOID  **handle,
                                                 VOID  *crypto_metadata,
                                                 ULONG crypto_metadata_size)
{
NX_CRYPTO_3DES *triple_des_context_ptr;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    /* Validate input parameters. */
    if ((method == NX_CRYPTO_NULL) || (key == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_3DES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }


    if (key_size_in_bits != NX_CRYPTO_3DES_KEY_LEN_IN_BITS) /* lgtm[cpp/weak-cryptographic-algorithm] */
    {
        return(NX_CRYPTO_UNSUPPORTED_KEY_SIZE);
    }

    triple_des_context_ptr = (NX_CRYPTO_3DES *)(crypto_metadata);


    _nx_crypto_3des_key_set(triple_des_context_ptr, key); /* lgtm[cpp/weak-cryptographic-algorithm] */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_3des_cleanup                      PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_3des_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_3DES));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_3des_operation                    PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles 3des encrypt or decrypt operations.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation Type                */
/*                                          Encrypt, Decrypt, Authenticate*/
/*    handler                               Pointer to crypto context     */
/*    method                                Pointer to crypto method      */
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
/*    _nx_crypto_cbc_encrypt                Perform CBC mode encryption   */
/*    _nx_crypto_cbc_decrypt                Perform CBC mode decryption   */
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
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_3des_operation(UINT op,       /* Encrypt, Decrypt, Authenticate */
                                                      VOID *handler, /* Crypto handler */
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
UINT  status = NX_CRYPTO_NOT_SUCCESSFUL;
NX_CRYPTO_3DES *context;

    NX_CRYPTO_PARAMETER_NOT_USED(handler);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    if (op == NX_CRYPTO_AUTHENTICATE)
    {
        /* Incorrect Operation. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_3DES))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if (method -> nx_crypto_algorithm != NX_CRYPTO_ENCRYPTION_3DES_CBC) /* lgtm[cpp/weak-cryptographic-algorithm] */
    {
        /* Incorrect method. */
        return(NX_CRYPTO_INVALID_ALGORITHM);
    }

    context = (NX_CRYPTO_3DES *)crypto_metadata;

    switch (op)
    {
        case NX_CRYPTO_DECRYPT:
        {
            status = _nx_crypto_cbc_decrypt_init(&(context -> nx_crypto_cbc_context),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
            if (status)
            {
                break;
            }

            status = _nx_crypto_cbc_decrypt(context, &(context -> nx_crypto_cbc_context),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_3des_decrypt,
                                            input, output, input_length_in_byte,
                                            (NX_CRYPTO_3DES_BLOCK_SIZE_IN_BITS >> 3));  /* lgtm[cpp/weak-cryptographic-algorithm] */
        } break;

        case NX_CRYPTO_ENCRYPT:
        {
            status = _nx_crypto_cbc_encrypt_init(&(context -> nx_crypto_cbc_context),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
            if (status)
            {
                break;
            }

            status = _nx_crypto_cbc_encrypt(context, &(context -> nx_crypto_cbc_context),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_3des_encrypt,
                                            input, output, input_length_in_byte,
                                            (NX_CRYPTO_3DES_BLOCK_SIZE_IN_BITS >> 3)); /* lgtm[cpp/weak-cryptographic-algorithm] */
        } break;

        case NX_CRYPTO_DECRYPT_INITIALIZE:
        {
            status = _nx_crypto_cbc_decrypt_init(&(context -> nx_crypto_cbc_context),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
        } break;

        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        {
            status = _nx_crypto_cbc_encrypt_init(&(context -> nx_crypto_cbc_context),
                                                 iv_ptr, method -> nx_crypto_IV_size_in_bits >> 3);
        } break;

        case NX_CRYPTO_DECRYPT_UPDATE:
        {
            status = _nx_crypto_cbc_decrypt(context, &(context -> nx_crypto_cbc_context),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_3des_decrypt,
                                            input, output, input_length_in_byte,
                                            (NX_CRYPTO_3DES_BLOCK_SIZE_IN_BITS >> 3)); /* lgtm[cpp/weak-cryptographic-algorithm] */
        } break;

        case NX_CRYPTO_ENCRYPT_UPDATE:
        {
            status = _nx_crypto_cbc_encrypt(context, &(context -> nx_crypto_cbc_context),
                                            (UINT (*)(VOID *, UCHAR *, UCHAR *, UINT))_nx_crypto_3des_encrypt,
                                            input, output, input_length_in_byte,
                                            (NX_CRYPTO_3DES_BLOCK_SIZE_IN_BITS >> 3)); /* lgtm[cpp/weak-cryptographic-algorithm] */
        } break;

        case NX_CRYPTO_ENCRYPT_CALCULATE:
        /* fallthrough */
        case NX_CRYPTO_DECRYPT_CALCULATE:
        {

            /* Nothing to do. */
            status = NX_CRYPTO_SUCCESS;
        } break;

        default:
        {
            status = NX_CRYPTO_INVALID_ALGORITHM;
        } break;
    }

    return status;
}

