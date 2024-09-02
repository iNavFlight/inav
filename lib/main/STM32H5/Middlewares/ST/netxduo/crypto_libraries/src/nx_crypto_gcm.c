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
/**   GCM Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_gcm.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_xor                                  PORTABLE C      */
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
/*    _nx_crypto_gcm_multi                  Compute multilication in GF   */
/*    _nx_crypto_gcm_ghash_update           Compute GHASH                 */
/*    _nx_crypto_gcm_gctr                   Perform GCTR operation        */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_gcm_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext)
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
/*    _nx_crypto_gcm_inc32                                PORTABLE C      */
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
/*    counter_block                         Pointer to counter block      */
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
/*    _nx_crypto_gcm_gctr                   Perform GCTR operation        */
/*    _nx_crypto_gcm_encrypt                Perform GCM encrypt/decrypt   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_gcm_inc32(UCHAR *counter_block)
{
USHORT result;

    /* Add one for last byte. */
    result = (USHORT)(counter_block[15] + 1);
    counter_block[15] = (UCHAR)(result & 0xFF);

    /* Handle carry. */
    result = (USHORT)((result >> 8) + counter_block[14]);
    counter_block[14] = (UCHAR)(result & 0xFF);
    result = (USHORT)((result >> 8) + counter_block[13]);
    counter_block[13] = (UCHAR)(result & 0xFF);
    result = (USHORT)((result >> 8) + counter_block[12]);
    counter_block[12] = (UCHAR)(result & 0xFF);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_multi                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs multiplication in GF(2^128).                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Pointer to X block            */
/*    y                                     Pointer to Y block            */
/*    output                                Pointer to result block       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_xor                    Perform XOR operation         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_gcm_ghash_update           Compute GHASH                 */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_gcm_multi(UCHAR *x, UCHAR *y, UCHAR *output)
{
UINT i;
INT j;
UCHAR v[NX_CRYPTO_GCM_BLOCK_SIZE];
UCHAR lsb;
UCHAR mask;

    NX_CRYPTO_MEMSET(output, 0, NX_CRYPTO_GCM_BLOCK_SIZE);
    NX_CRYPTO_MEMCPY(v, y, NX_CRYPTO_GCM_BLOCK_SIZE); /* Use case of memcpy is verified. */

    mask = 0x80;
    for (i = 0; i < NX_CRYPTO_GCM_BLOCK_SIZE_BITS; i++)
    {

        /* output = output xor v when the ith bit of x is set. */
        if (*x & mask)
        {
            _nx_crypto_gcm_xor(output, v, output);
        }

        /* Store the LSB before shift right. */
        j = NX_CRYPTO_GCM_BLOCK_SIZE - 1;
        lsb = v[j];

        /* v = v >> 1 */
        for (; j > 0; j--)
        {
            v[j] = (UCHAR)((v[j] >> 1) | (v[j - 1] << 7));
        }
        v[0] = v[0] >> 1;

        /* v = v xor R when LSB of v is set. */
        if (lsb & 1)
        {
            v[0] = v[0] ^ 0xe1;
        }

        mask = mask >> 1;
        if (!mask)
        {
            mask = 0x80;
            x++;
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_ghash_update                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates GHASH with new input from the caller. The     */
/*    input is padded so that the length is a multiple of the block size. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hkey                                  Pointer to hash key           */
/*    input                                 Pointer to bytes of input     */
/*    input_length                          Length of bytes of input      */
/*    output                                Pointer to updated hash       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_xor                    Perform XOR operation         */
/*    _nx_crypto_gcm_multi                  Perform multiplication in GF  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_gcm_encrypt                Perform GCM encrypt/decrypt   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_gcm_ghash_update(UCHAR *hkey, UCHAR *input, UINT input_length, UCHAR *output)
{
UCHAR tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE];
UINT i, n;

    n = input_length >> NX_CRYPTO_GCM_BLOCK_SIZE_SHIFT;
    for (i = 0; i < n; i++)
    {

        /* output = (output xor input) multi hkey */
        _nx_crypto_gcm_xor(output, input, tmp_block);
        _nx_crypto_gcm_multi(tmp_block, hkey, output);
        input += NX_CRYPTO_GCM_BLOCK_SIZE;
    }

    input_length -= n << NX_CRYPTO_GCM_BLOCK_SIZE_SHIFT;
    if (input_length > 0)
    {

        /* Pad the block with zeros when the input length is not
            multiple of the block size. */
        NX_CRYPTO_MEMCPY(tmp_block, input, input_length); /* Use case of memcpy is verified. */
        NX_CRYPTO_MEMSET(&tmp_block[input_length], 0, sizeof(tmp_block) - input_length);
        _nx_crypto_gcm_xor(output, tmp_block, tmp_block);
        _nx_crypto_gcm_multi(tmp_block, hkey, output);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_gctr                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs GCTR mode encryption and decryption. The     */
/*    counter block is updated after calling this function.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to bytes of input     */
/*    output                                Pointer to output buffer      */
/*    length                                Length of bytes of input      */
/*    counter_block                         Pointer to counter block      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_xor                    Perform XOR operation         */
/*    _nx_crypto_gcm_inc32                  Increase the counter by one   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_gcm_encrypt                Perform GCM encrypt/decrypt   */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_gcm_gctr(VOID *crypto_metadata,
                                               UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                               UCHAR *input, UCHAR *output, UINT length, UCHAR *counter_block)
{
UCHAR aes_output[NX_CRYPTO_GCM_BLOCK_SIZE];
UINT i, n;

    n = length >> NX_CRYPTO_GCM_BLOCK_SIZE_SHIFT;

    for (i = 0; i < n; i++)
    {

        /* Encrypt the counter. */
        crypto_function(crypto_metadata, counter_block, aes_output, NX_CRYPTO_GCM_BLOCK_SIZE);

        /* XOR the input with encrypted counter. */
        _nx_crypto_gcm_xor(input, aes_output, output);

        /* Increase the counter block. */
        _nx_crypto_gcm_inc32(counter_block);

        input += NX_CRYPTO_GCM_BLOCK_SIZE;
        output += NX_CRYPTO_GCM_BLOCK_SIZE;
    }

    length -= n << NX_CRYPTO_GCM_BLOCK_SIZE_SHIFT;
    if (length > 0)
    {
        crypto_function(crypto_metadata, counter_block, aes_output, NX_CRYPTO_GCM_BLOCK_SIZE);

        /* Perform XOR operation on local buffer when
            remaining input length is smaller than block size. */
        _nx_crypto_gcm_xor(input, aes_output, aes_output);
        NX_CRYPTO_MEMCPY(output, aes_output, length); /* Use case of memcpy is verified. */
    }

}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_encrypt_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initialize the GCM mode for encryption.               */
/*                                                                        */
/*    Note, the first byte of iv represents the length of IV excluding its*/
/*    first byte. For example, 0x0401020304 indicates the length of IV is */
/*    4 bytes and the content of IV is 0x01020304.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    additional_data                       Pointer to additional data    */
/*    additional_len                        Length of additional data     */
/*    iv                                    Pointer to Initial Vector     */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_ghash_update           Update GHASH                  */
/*    _nx_crypto_gcm_inc32                  Increase the counter by one   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_init(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                VOID *additional_data, UINT additional_len,
                                                UCHAR *iv, UINT block_size)
{
UCHAR *hkey = gcm_metadata -> nx_crypto_gcm_hkey;
UCHAR *j0 = gcm_metadata -> nx_crypto_gcm_j0;
UCHAR *s = gcm_metadata -> nx_crypto_gcm_s;
UCHAR *counter = gcm_metadata -> nx_crypto_gcm_counter;
UCHAR tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE];
UCHAR iv_len;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_GCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Generate hash key by encrypt the zero block. */
    NX_CRYPTO_MEMSET(hkey, 0, NX_CRYPTO_GCM_BLOCK_SIZE);
    crypto_function(crypto_metadata, hkey, hkey, NX_CRYPTO_GCM_BLOCK_SIZE);

    /* Generate the pre-counter block j0. */
    iv_len = iv[0];
    iv = iv + 1;
    if (iv_len == NX_CRYPTO_GCM_BLOCK_SIZE - 4)
    {

        /* When the length of IV is 12 then 1 is appended to IV to form j0. */
        /* j0 in increased before GCTR. */
        NX_CRYPTO_MEMCPY(j0, iv, iv_len); /* Use case of memcpy is verified. */
        j0[12] = 0;
        j0[13] = 0;
        j0[14] = 0;
        j0[15] = 1;
    }
    else
    {

        /* When the length of IV is not 12 then apply GHASH to the IV. */
        NX_CRYPTO_MEMSET(j0, 0, NX_CRYPTO_GCM_BLOCK_SIZE);
        _nx_crypto_gcm_ghash_update(hkey, iv, iv_len, j0);

        /* Apply GHASH to the length of IV to form j0.*/
        NX_CRYPTO_MEMSET(tmp_block, 0, NX_CRYPTO_GCM_BLOCK_SIZE);
        tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE - 2] = (UCHAR)(((iv_len << 3) & 0xFF00) >> 8);
        tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE - 1] = (UCHAR)((iv_len << 3) & 0x00FF);
        _nx_crypto_gcm_ghash_update(hkey, tmp_block, NX_CRYPTO_GCM_BLOCK_SIZE, j0);
    }

    /* Apply GHASH to the additional authenticated data. */
    NX_CRYPTO_MEMSET(s, 0, NX_CRYPTO_GCM_BLOCK_SIZE);
    _nx_crypto_gcm_ghash_update(hkey, additional_data, additional_len, s);

    /* Initial counter block for GCTR is j0 + 1. */
    NX_CRYPTO_MEMCPY(counter, j0, NX_CRYPTO_GCM_BLOCK_SIZE); /* Use case of memcpy is verified. */
    _nx_crypto_gcm_inc32(counter);

    gcm_metadata -> nx_crypto_gcm_additional_data_len = additional_len;
    gcm_metadata -> nx_crypto_gcm_input_total_length = 0;

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(tmp_block, 0, sizeof(tmp_block));
#endif

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_encrypt_update                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates data for GCM encryption.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to bytes of input     */
/*    output                                Pointer to output buffer      */
/*    length                                Length of bytes of input      */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_gctr                   Update data for GCM mode      */
/*    _nx_crypto_gcm_ghash_update           Update GHASH                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_update(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length,
                                                  UINT block_size)
{
UCHAR *hkey = gcm_metadata -> nx_crypto_gcm_hkey;
UCHAR *s = gcm_metadata -> nx_crypto_gcm_s;
UCHAR *counter = gcm_metadata -> nx_crypto_gcm_counter;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_GCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Invoke GCTR function to encrypt or decrypt the input message. */
    _nx_crypto_gcm_gctr(crypto_metadata, crypto_function, input, output, length, counter);

    /* Apply GHASH to the cipher text. */
    _nx_crypto_gcm_ghash_update(hkey, output, length, s);

    gcm_metadata -> nx_crypto_gcm_input_total_length += length;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_encrypt_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates TAG for GCM mode.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    output                                Pointer to output buffer      */
/*    icv_len                               Length of TAG                 */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_gctr                   Update data for GCM mode      */
/*    _nx_crypto_gcm_ghash_update           Update GHASH                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_gcm_encrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *output, UINT icv_len, UINT block_size)
{
UCHAR *hkey = gcm_metadata -> nx_crypto_gcm_hkey;
UCHAR *j0 = gcm_metadata -> nx_crypto_gcm_j0;
UCHAR *s = gcm_metadata -> nx_crypto_gcm_s;
UCHAR tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE];
UINT additional_len = gcm_metadata -> nx_crypto_gcm_additional_data_len;
UINT length;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_GCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Apply GHASH to the length of additional authenticated data and the length of cipher text. */
    length = gcm_metadata -> nx_crypto_gcm_input_total_length;
    tmp_block[0] = 0;
    tmp_block[1] = 0;
    tmp_block[2] = 0;
    tmp_block[3] = 0;
    tmp_block[4] = (UCHAR)(((additional_len << 3) & 0xFF000000) >> 24);
    tmp_block[5] = (UCHAR)(((additional_len << 3) & 0x00FF0000) >> 16);
    tmp_block[6] = (UCHAR)(((additional_len << 3) & 0x0000FF00) >> 8);
    tmp_block[7] = (UCHAR)((additional_len << 3) & 0x000000FF);
    tmp_block[8] = 0;
    tmp_block[9] = 0;
    tmp_block[10] = 0;
    tmp_block[11] = 0;
    tmp_block[12] = (UCHAR)(((length << 3) & 0xFF000000) >> 24);
    tmp_block[13] = (UCHAR)(((length << 3) & 0x00FF0000) >> 16);
    tmp_block[14] = (UCHAR)(((length << 3) & 0x0000FF00) >> 8);
    tmp_block[15] = (UCHAR)((length << 3) & 0x000000FF);
    _nx_crypto_gcm_ghash_update(hkey, tmp_block, NX_CRYPTO_GCM_BLOCK_SIZE, s);

    /* Encrypt the GHASH result using GCTR with j0 as initial counter block.
        The result is the authentication tag. */
    _nx_crypto_gcm_gctr(crypto_metadata, crypto_function, s, s, NX_CRYPTO_GCM_BLOCK_SIZE, j0);

    /* Append authentication tag to the end of the cipher text. */
    NX_CRYPTO_MEMCPY(output, s, icv_len); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(tmp_block, 0, sizeof(tmp_block));
#endif

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_decrypt_update                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates data for GCM decryption.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to bytes of input     */
/*    output                                Pointer to output buffer      */
/*    length                                Length of bytes of input      */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_gctr                   Update data for GCM mode      */
/*    _nx_crypto_gcm_ghash_update           Update GHASH                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_gcm_decrypt_update(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length,
                                                  UINT block_size)
{
UCHAR *hkey = gcm_metadata -> nx_crypto_gcm_hkey;
UCHAR *s = gcm_metadata -> nx_crypto_gcm_s;
UCHAR *counter = gcm_metadata -> nx_crypto_gcm_counter;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_GCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Apply GHASH to the cipher text. */
    _nx_crypto_gcm_ghash_update(hkey, input, length, s);

    /* Invoke GCTR function to encrypt or decrypt the input message. */
    _nx_crypto_gcm_gctr(crypto_metadata, crypto_function, input, output, length, counter);

    gcm_metadata -> nx_crypto_gcm_input_total_length += length;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_gcm_decrypt_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the TAG for GCM mode.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to TAG buffer         */
/*    icv_len                               Length of TAG                 */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_gcm_gctr                   Update data for GCM mode      */
/*    _nx_crypto_gcm_ghash_update           Update GHASH                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_gcm_decrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_GCM *gcm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *input, UINT icv_len, UINT block_size)
{
UCHAR *hkey = gcm_metadata -> nx_crypto_gcm_hkey;
UCHAR *j0 = gcm_metadata -> nx_crypto_gcm_j0;
UCHAR *s = gcm_metadata -> nx_crypto_gcm_s;
UCHAR tmp_block[NX_CRYPTO_GCM_BLOCK_SIZE];
UINT additional_len = gcm_metadata -> nx_crypto_gcm_additional_data_len;
UINT length;
UINT i;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_GCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Apply GHASH to the length of additional authenticated data and the length of cipher text. */
    length = gcm_metadata -> nx_crypto_gcm_input_total_length;
    tmp_block[0] = 0;
    tmp_block[1] = 0;
    tmp_block[2] = 0;
    tmp_block[3] = 0;
    tmp_block[4] = (UCHAR)(((additional_len << 3) & 0xFF000000) >> 24);
    tmp_block[5] = (UCHAR)(((additional_len << 3) & 0x00FF0000) >> 16);
    tmp_block[6] = (UCHAR)(((additional_len << 3) & 0x0000FF00) >> 8);
    tmp_block[7] = (UCHAR)((additional_len << 3) & 0x000000FF);
    tmp_block[8] = 0;
    tmp_block[9] = 0;
    tmp_block[10] = 0;
    tmp_block[11] = 0;
    tmp_block[12] = (UCHAR)(((length << 3) & 0xFF000000) >> 24);
    tmp_block[13] = (UCHAR)(((length << 3) & 0x00FF0000) >> 16);
    tmp_block[14] = (UCHAR)(((length << 3) & 0x0000FF00) >> 8);
    tmp_block[15] = (UCHAR)((length << 3) & 0x000000FF);
    _nx_crypto_gcm_ghash_update(hkey, tmp_block, NX_CRYPTO_GCM_BLOCK_SIZE, s);

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(tmp_block, 0, sizeof(tmp_block));
#endif

    /* Encrypt the GHASH result using GCTR with j0 as initial counter block.
        The result is the authentication tag. */
    _nx_crypto_gcm_gctr(crypto_metadata, crypto_function, s, s, NX_CRYPTO_GCM_BLOCK_SIZE, j0);

    for (i = 0; i < icv_len; i++)
    {
        if (input[i] != s[i])
        {

            /* Authentication failed. */
            return(NX_CRYPTO_AUTHENTICATION_FAILED);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}
