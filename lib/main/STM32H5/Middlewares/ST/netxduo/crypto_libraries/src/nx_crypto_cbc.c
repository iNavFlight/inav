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
/**   CBC Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_cbc.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_xor                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs XOR operation on the output buffer.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    plaintext                             Pointer to input plantext     */
/*    key                                   Value to be xor'ed            */
/*    ciphertext                            Output buffer                 */
/*    block_size                            Block size                    */
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
/*    _nx_crypto_cbc_encrypt                Perform CBC mode encryption   */
/*    _nx_crypto_cbc_decrypt                Perform CBC mode decryption   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static VOID _nx_crypto_cbc_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext, UCHAR block_size)
{
UINT i;

    for (i = 0; i < block_size; i++)
    {
        ciphertext[i] = plaintext[i] ^ key[i];
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_encrypt                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CBC mode encryption.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    cbc_metadata                          Pointer to CBC metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The size of the output      */
/*                                            buffer must be at least     */
/*                                            the size of input message.  */
/*    length                                Length of the input message.  */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_cbc_xor                    Perform CBC XOR operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_cbc_operation   Handle AES encrypt or decrypt */
/*    _nx_crypto_method_des_operation       Handle DES encrypt or decrypt */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            and updated constants,      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_cbc_encrypt(VOID *crypto_metadata, NX_CRYPTO_CBC *cbc_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UCHAR block_size)
{
UCHAR *last_cipher;
UINT   i;

    if (block_size == 0)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Determine if data length is multiple of block size. */
    if (length % block_size)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Determine if block size is larger than the size of save_input. */
    if (block_size > sizeof(cbc_metadata -> nx_crypto_cbc_last_block))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Pick up last cipher. */
    last_cipher = cbc_metadata -> nx_crypto_cbc_last_block;

    for (i = 0; i < length; i += block_size)
    {

        /* XOR. */
        _nx_crypto_cbc_xor(&input[i], last_cipher, output, block_size);

        /* Encrypt the block. */
        crypto_function(crypto_metadata, output, output, block_size);

        /* Remember the previous encrypt block result. */
        last_cipher = output;

        output += block_size;
    }

    /* Store the last cipher for next round. */
    NX_CRYPTO_MEMCPY(cbc_metadata -> nx_crypto_cbc_last_block, last_cipher, block_size); /* Use case of memcpy is verified. */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_decrypt                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CBC mode decryption.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    cbc_metadata                          Pointer to CBC metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The size of the output      */
/*                                            buffer must be at least     */
/*                                            the size of input message.  */
/*    length                                Length of the input message.  */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_cbc_xor                    Perform CBC XOR operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_cbc_operation   Handle AES encrypt or decrypt */
/*    _nx_crypto_method_des_operation       Handle DES encrypt or decrypt */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            and updated constants,      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_cbc_decrypt(VOID *crypto_metadata, NX_CRYPTO_CBC *cbc_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UCHAR block_size)
{
UCHAR *last_cipher;
UCHAR save_input[16];
UINT  i;

    if (block_size == 0)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Determine if data length is multiple of block size. */
    if (length % block_size)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Determine if block size is larger than the size of save_input. */
    if (block_size > sizeof(cbc_metadata -> nx_crypto_cbc_last_block))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    last_cipher = cbc_metadata -> nx_crypto_cbc_last_block;

    for (i = 0; i < length; i += block_size)
    {
        /* If input == output, the xor clobbers the input buffer so we need to save off our last ciphertext
           before doing the xor. */
        NX_CRYPTO_MEMCPY(save_input, &input[i], block_size); /* Use case of memcpy is verified. */

        /* Decrypt the block.  */
        crypto_function(crypto_metadata, &input[i], &output[i], block_size);

        /* XOR.  */
        _nx_crypto_cbc_xor(&output[i], last_cipher, &output[i], block_size);

        NX_CRYPTO_MEMCPY(last_cipher, save_input, block_size); /* Use case of memcpy is verified. */
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(save_input, 0, sizeof(save_input));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_cbc_encrypt_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CBC mode initialization.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cbc_metadata                          Pointer to CBC metadata       */
/*    iv                                    Pointer to Initial Vector     */
/*    iv_len                                Length of Initial Vector      */
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
/*    _nx_crypto_method_aes_cbc_operation   Handle AES encrypt or decrypt */
/*    _nx_crypto_method_des_operation       Handle DES encrypt or decrypt */
/*    _nx_crypto_method_3des_operation      Handle 3DES encrypt or decrypt*/
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
NX_CRYPTO_KEEP UINT _nx_crypto_cbc_encrypt_init(NX_CRYPTO_CBC *cbc_metadata, UCHAR *iv, UINT iv_len)
{

    /* Determine if IV size is larger than the size of save_input. */
    if (iv_len > sizeof(cbc_metadata -> nx_crypto_cbc_last_block))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Copy IV to last cipher. */
    NX_CRYPTO_MEMCPY(cbc_metadata -> nx_crypto_cbc_last_block, iv, iv_len); /* Use case of memcpy is verified. */

    return(NX_CRYPTO_SUCCESS);
}
