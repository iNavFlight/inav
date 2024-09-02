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
/**   XCBC MAC Mode                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_xcbc_mac.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_xcbc_xor                                 PORTABLE C      */
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
/*    _nx_crypto_xcbc_mac                   Perform XCBC MAC 96 hash      */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_xcbc_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext)
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
/*    _nx_crypto_xcbc_mac                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs XCBC MAC 96 hash, only support block of      */
/*    16 bytes.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
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
/*    _nx_crypto_xcbc_xor                   Perform XOR operation         */
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
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_xcbc_mac(VOID *crypto_metadata, UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                        UINT (*key_set_function)(VOID *, UCHAR *, UINT),
                                        UCHAR *additional_data, UINT additional_len,
                                        UCHAR *input, UCHAR *output, UINT input_length_in_byte,
                                        UCHAR *iv, UCHAR icv_len, UINT block_size)
{
UCHAR  K1[NX_CRYPTO_XCBC_MAC_BLOCK_SIZE];
UCHAR  E[NX_CRYPTO_XCBC_MAC_BLOCK_SIZE];
UCHAR  K2[NX_CRYPTO_XCBC_MAC_BLOCK_SIZE];
UCHAR  pad[NX_CRYPTO_XCBC_MAC_BLOCK_SIZE];
UCHAR *key = additional_data;
UINT   key_size_in_bits = additional_len;

    NX_CRYPTO_STATE_CHECK

    NX_CRYPTO_PARAMETER_NOT_USED(iv);

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_XCBC_MAC_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* RFC 3566, section 4. */
    /* K1 = 0x01010101010101010101010101010101 encrypted with Key K */
    NX_CRYPTO_MEMSET(K1, 1, sizeof(K1));
    key_set_function(crypto_metadata, key, key_size_in_bits >> 5);
    crypto_function(crypto_metadata, K1, K1, block_size);

    /* E = 0x00000000000000000000000000000000 */
    NX_CRYPTO_MEMSET(E, 0, sizeof(E));

    while (input_length_in_byte > block_size)
    {

        /* XOR M with E */
        _nx_crypto_xcbc_xor(E, input, E);

        /* Encrypt the result with Key K1, yielding E */
        key_set_function(crypto_metadata, K1, sizeof(K1) >> 2);
        crypto_function(crypto_metadata, E, E, block_size);

        input_length_in_byte -= block_size;
        input += block_size;
    }

    if (input_length_in_byte == block_size)
    {

        /* K2 = 0x02020202020202020202020202020202 encrypted with Key K */
        NX_CRYPTO_MEMSET(K2, 2, sizeof(K2));
    }
    else
    {

        /* K3 = 0x03030303030303030303030303030303 encrypted with Key K */
        NX_CRYPTO_MEMSET(K2, 3, sizeof(K2));

        /* Pad M with a single "1" bit, followed by "0" bits. */
        NX_CRYPTO_MEMSET(pad, 0, sizeof(pad));
        pad[input_length_in_byte] = 0x80;
    }
    key_set_function(crypto_metadata, key, key_size_in_bits >> 5);
    crypto_function(crypto_metadata, K2, K2, block_size);

    NX_CRYPTO_MEMCPY(pad, input, input_length_in_byte); /* Use case of memcpy is verified. */

    /* XOR M with E and Key K2 or K3 */
    _nx_crypto_xcbc_xor(E, pad, E);
    _nx_crypto_xcbc_xor(E, K2, E);

    /* Encrypt the result with Key K1, yielding E */
    key_set_function(crypto_metadata, K1, sizeof(K1) >> 2);
    crypto_function(crypto_metadata, E, E, block_size);

    NX_CRYPTO_MEMCPY(output, E, icv_len); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(K1, 0, sizeof(K1));
    NX_CRYPTO_MEMSET(E, 0, sizeof(E));
    NX_CRYPTO_MEMSET(K2, 0, sizeof(K2));
    NX_CRYPTO_MEMSET(pad, 0, sizeof(pad));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}

