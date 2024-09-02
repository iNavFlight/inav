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
/**   CTR Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ctr.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ctr_xor                                  PORTABLE C      */
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
/*    ciphertext                            Output buffer of 16 bytes     */
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
/*    _nx_crypto_ctr_encrypt                Perform CTR mode encryption   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), disabled */
/*                                            unaligned access by default,*/
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP static VOID _nx_crypto_ctr_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext)
{
#ifdef NX_CRYPTO_ENABLE_UNALIGNED_ACCESS
UINT *p = (UINT *)plaintext;
UINT *c = (UINT *)ciphertext;
UINT *k = (UINT *)key;

    c[0] = p[0] ^ k[0];
    c[1] = p[1] ^ k[1];
    c[2] = p[2] ^ k[2];
    c[3] = p[3] ^ k[3];
#else
    ciphertext[0] = plaintext[0] ^ key[0];
    ciphertext[1] = plaintext[1] ^ key[1];
    ciphertext[2] = plaintext[2] ^ key[2];
    ciphertext[3] = plaintext[3] ^ key[3];
    ciphertext[4] = plaintext[4] ^ key[4];
    ciphertext[5] = plaintext[5] ^ key[5];
    ciphertext[6] = plaintext[6] ^ key[6];
    ciphertext[7] = plaintext[7] ^ key[7];
    ciphertext[8] = plaintext[8] ^ key[8];
    ciphertext[9] = plaintext[9] ^ key[9];
    ciphertext[10] = plaintext[10] ^ key[10];
    ciphertext[11] = plaintext[11] ^ key[11];
    ciphertext[12] = plaintext[12] ^ key[12];
    ciphertext[13] = plaintext[13] ^ key[13];
    ciphertext[14] = plaintext[14] ^ key[14];
    ciphertext[15] = plaintext[15] ^ key[15];
#endif
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ctr_add_one                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds one for the last byte.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    control_block                         Pointer to control block      */
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
/*    _nx_crypto_ctr_encrypt                Perform CTR mode encryption   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_ctr_add_one(UCHAR *control_block)
{
USHORT result;

    /* Add one for last byte. */
    result = (USHORT)(control_block[15] + 1);
    control_block[15] = (UCHAR)(result & 0xFF);

    /* Handle carry. */
    result = (USHORT)((result >> 8) + control_block[14]);
    control_block[14] = (UCHAR)(result & 0xFF);
    result = (USHORT)((result >> 8) + control_block[13]);
    control_block[13] = (UCHAR)(result & 0xFF);
    result = (USHORT)((result >> 8) + control_block[12]);
    control_block[12] = (UCHAR)(result & 0xFF);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ctr_encrypt                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CTR mode encryption, only support block of   */
/*    16 bytes.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    ctr_metadata                          Pointer to CTR metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    key_set_function                      Pointer to key set function   */
/*    additional_data                       Pointer to the additional data*/
/*    additional_len                        Length of additional data     */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The size of the output      */
/*                                            buffer must be at least     */
/*                                            the size of input message.  */
/*    length                                Length of the input message.  */
/*    iv                                    Nonce length + Nonce          */
/*    icv_len                               ICV length                    */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ctr_xor                    Perform XOR operation         */
/*    _nx_crypto_ctr_add_one                Perform add one operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_ctr_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ctr_encrypt(VOID *crypto_metadata, NX_CRYPTO_CTR *ctr_metadata,
                                           UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                           UCHAR *input, UCHAR *output, UINT length, UINT block_size)
{
UCHAR  *control_block = ctr_metadata -> nx_crypto_ctr_counter_block;
UCHAR  aes_output[NX_CRYPTO_CTR_BLOCK_SIZE];
UINT   i;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_CTR_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    for (i = 0; i < length; i += block_size)
    {
        if (length - i < block_size)
        {
            break;
        }
        crypto_function(crypto_metadata, control_block, aes_output, block_size);
        _nx_crypto_ctr_xor(&input[i], aes_output, &output[i]);
        _nx_crypto_ctr_add_one(control_block);
    }

    /* If the input is not an even multiple of 16 bytes, we need to truncate and xor the remainder. */
    if (length - i != 0)
    {
        crypto_function(crypto_metadata, control_block, aes_output, block_size);
        _nx_crypto_ctr_xor(&input[i], aes_output, aes_output);
        NX_CRYPTO_MEMCPY(&output[i], aes_output, length - i); /* Use case of memcpy is verified. */
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(aes_output, 0, sizeof(aes_output));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ctr_encrypt_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs CTR mode initialization.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ctr_metadata                          Pointer to CTR metadata       */
/*    iv                                    Pointer to Initial Vector     */
/*    iv_len                                Length of IV. Must be 8       */
/*    nonce                                 Pointer to Nonce              */
/*    nonce_len                             Length of Nonce. Must be 4    */
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
/*    _nx_crypto_method_aes_ctr_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ctr_encrypt_init(NX_CRYPTO_CTR *ctr_metadata, UCHAR *iv, UINT iv_len,
                                                UCHAR *nonce, UINT nonce_len)
{
UCHAR  *control_block = ctr_metadata -> nx_crypto_ctr_counter_block;

    /* Check IV length and Nonce length. */
    if ((iv_len != 8) || (nonce_len != 4))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Control block consists of the following data:
     * Bits: |   0-31 (32 bits)  |        32-95 (64 bits)     |  96-127 (32 bits) |
     *       |    All set to 1   | Initialization Vector (IV) |       Nonce       |
     */
    NX_CRYPTO_MEMSET(control_block, 0x0, 16);
    control_block[15] = 1;
    NX_CRYPTO_MEMCPY(&control_block[4], iv, 8); /* Use case of memcpy is verified. */
    NX_CRYPTO_MEMCPY(&control_block[0], nonce, 4); /* Use case of memcpy is verified. */

    return(NX_CRYPTO_SUCCESS);
}

