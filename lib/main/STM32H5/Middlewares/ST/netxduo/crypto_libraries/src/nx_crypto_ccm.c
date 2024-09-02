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
/**   CCM Mode                                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ccm.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ccm_xor                                  PORTABLE C      */
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
/*    _nx_crypto_ccm_cbc_pad                Compute CBC-MAC value with    */
/*                                            padding for CCM mode        */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_ccm_xor(UCHAR *plaintext, UCHAR *key, UCHAR *ciphertext)
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
/*    _nx_crypto_ccm_cbc_pad                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compute CBC-MAC value with padding for CCM mode.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    crypto_function                       Pointer to crypto function    */
/*    input                                 Pointer to clear text input   */
/*    output                                Pointer to encrypted output   */
/*                                            The output is the last      */
/*                                            16 bytes cipher.            */
/*    length                                Length of the input message.  */
/*    iv                                    Initial Vector                */
/*    block_size                            Block size                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ccm_xor                    Perform CCM XOR operation     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ccm_authentication         Perform CCM authentication    */
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
NX_CRYPTO_KEEP static VOID _nx_crypto_ccm_cbc_pad(VOID *crypto_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length, UCHAR *iv, UINT block_size)
{
UINT  i = 0;
UCHAR last_cipher[NX_CRYPTO_CCM_BLOCK_SIZE];

    NX_CRYPTO_MEMCPY(last_cipher, iv, block_size); /* Use case of memcpy is verified. */
    for (i = 0; i < length; i += block_size)
    {

        /* XOR */
        if ((length - i) < block_size)
        {

            /* If the length of this block is less than block size, pad it with zero.  */
            NX_CRYPTO_MEMCPY(output, input + i, length - i); /* Use case of memcpy is verified. */
            NX_CRYPTO_MEMSET(output + length - i, 0, block_size - (length - i));
            _nx_crypto_ccm_xor(output, last_cipher, output);
        }
        else
        {
            _nx_crypto_ccm_xor(input + i, last_cipher, output);
        }

        /* Encrypt the block.  */
        crypto_function(crypto_metadata, output, last_cipher, block_size);
    }

    /* Return last block of the cipher.  */
    NX_CRYPTO_MEMCPY(output, last_cipher, block_size); /* Use case of memcpy is verified. */

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(last_cipher, 0, sizeof(last_cipher));
#endif /* NX_SECURE_KEY_CLEAR  */
}

NX_CRYPTO_KEEP static VOID _nx_crypto_ccm_authentication_init(VOID *crypto_metadata,
                                                              UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                              UCHAR *a_data, UINT a_len, UINT m_len,
                                                              UCHAR *X, UCHAR *Nonce, UCHAR L,
                                                              USHORT M, UINT block_size)
{
UCHAR Flags = 0;
UCHAR B[NX_CRYPTO_CCM_BLOCK_SIZE];
UCHAR temp_len = 0;

    NX_CRYPTO_MEMSET(B, 0, NX_CRYPTO_CCM_BLOCK_SIZE);
    NX_CRYPTO_MEMSET(X, 0, NX_CRYPTO_CCM_BLOCK_SIZE);

    /* AddAuthData: Right-concatenate the l(a) with the string a,
       and pad it with zero so that the AddAuthData has length divisible by 16.  */
    /* PlaintextData: Pad string m with zero so that the PlaintextData has length divisible by 16.  */
    /* AuthData = AddAuthData||PlaintextData.  */
    /* Parse the AuthData as B(1)||B(2)||...||B(t), where the block B(i) is a 16-bytes string.  */

    /* CBC-MAC value X(i + 1) = E(Key, X(i) ^ B(i)), for i = 0, 1,..., t.  */

    /* Create Flag for B(0).  */
    if (a_len > 0)
    {
        Flags = 1 << 6;
    }
    else
    {
        Flags = 0;
    }
    Flags |= (UCHAR)(((M - 2) >> 1) << 3);
    Flags |= (UCHAR)(L - 1);

    /* B(0) = Flags||Nonce||l(m)  */
    B[0] = Flags;
    NX_CRYPTO_MEMCPY(B + 1, Nonce, (UINT)15 - L); /* Use case of memcpy is verified. */
    B[14] = (UCHAR)(m_len >> 8);
    B[15] = (UCHAR)(m_len);

    /* Get the CBC-MAC value X(1).  */
    _nx_crypto_ccm_cbc_pad(crypto_metadata, crypto_function, B, X, block_size, X, block_size);

    /* B(1) = 2 bytes l(a) + leftmost 14 bytes of string a.  */
    B[0] = (UCHAR)(a_len >> 8);
    B[1] = (UCHAR)(a_len);

    /* If the length of string a is less than 14, pad B(1) with 0.  */
    temp_len = (UCHAR)((a_len > (block_size - 2)) ? (block_size - 2) : a_len);
    NX_CRYPTO_MEMCPY(B + 2, a_data, (UINT)temp_len); /* Use case of memcpy is verified. */

    /* Get the CBC-MAC value X(2).  */
    _nx_crypto_ccm_cbc_pad(crypto_metadata, crypto_function, B, X, (UINT)(temp_len + 2), X, block_size);

    /* Get the CBC-MAC value X(i), for i = 3,...,t + 1.  */
    if (a_len > (block_size - 2))
    {
        _nx_crypto_ccm_cbc_pad(crypto_metadata, crypto_function, a_data + block_size - 2, X, a_len - (block_size - 2), X, block_size);
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(B, 0, sizeof(B));
#endif /* NX_SECURE_KEY_CLEAR  */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ccm_encrypt_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initialize the CCM mode for encryption.               */
/*                                                                        */
/*    Note, the first byte of iv represents the length of IV excluding its*/
/*    first byte. For example, 0x0401020304 indicates the length of IV is */
/*    4 bytes and the content of IV is 0x01020304.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    ccm_metadata                          Pointer to CCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    additional_data                       Pointer to additional data    */
/*    additional_len                        Length of additional data     */
/*    length                                Total length of plain/cipher  */
/*    iv                                    Pointer to Initial Vector     */
/*    icv_len                               Length of TAG                 */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ccm_authentication_init    Initialize authentication     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_ccm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_init(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                VOID *additional_data, UINT additional_len,
                                                UINT length, UCHAR *iv, USHORT icv_len, USHORT block_size)
{
UCHAR  L = (UCHAR)(15 - iv[0]);
UCHAR *Nonce = iv + 1;

UCHAR  Flags = 0;
UCHAR *A = ccm_metadata -> nx_crypto_ccm_A;

    /* Check the block size. */
    /* Accroding to RFC 3610, valid values of L range between 2 octets and 8 octets, iv[0] range between 7 octets and 13 octets. */
    if (block_size != NX_CRYPTO_CCM_BLOCK_SIZE || iv[0] > 13 || iv[0] < 7)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ccm_metadata -> nx_crypto_ccm_icv_length = icv_len;

    /* Data authentication.  */
    if (icv_len > 0)
    {

        /* Compute authentication tag T.  */
        _nx_crypto_ccm_authentication_init(crypto_metadata, crypto_function,
                                           (UCHAR *)additional_data, additional_len, length,
                                           ccm_metadata -> nx_crypto_ccm_X,
                                           Nonce, L, icv_len, block_size);
    }

    NX_CRYPTO_MEMSET(A, 0, sizeof(ccm_metadata -> nx_crypto_ccm_A));

    /* Create A(i) = Flags||Nonce||Counter i, for i = 0, 1, 2,....  */
    Flags = (UCHAR)(L - 1);
    A[0] = Flags;
    NX_CRYPTO_MEMCPY(A + 1, Nonce, (UINT)(15 - L)); /* Use case of memcpy is verified. */

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ccm_encrypt_update                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates data for GCM encryption or decryption.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    ccm_metadata                          Pointer to CCM metadata       */
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
/*    _nx_crypto_ccm_cbc_pad                Update data for CCM mode      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_ccm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_update(UINT op, VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                  UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                  UCHAR *input, UCHAR *output, UINT length, UINT block_size)
{
UCHAR *A = ccm_metadata -> nx_crypto_ccm_A;
UCHAR  X[NX_CRYPTO_CCM_BLOCK_SIZE];
UINT   i = 0, k = 0;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_CCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if (op == NX_CRYPTO_ENCRYPT_UPDATE)
    {
        
        /* Data authentication.  */
        if (ccm_metadata -> nx_crypto_ccm_icv_length > 0)
        {

            /* Compute authentication tag T.  */
            _nx_crypto_ccm_cbc_pad(crypto_metadata, crypto_function, input,
                                   ccm_metadata -> nx_crypto_ccm_X, length,
                                   ccm_metadata -> nx_crypto_ccm_X, block_size);
        }
    }

    /* Data encryption.  */
    if (length > 0)
    {

        /* Parse the plain text as M(1)||M(2)||..., where the block M(i) is a 16-byte string.  */
        /* Cipher text block: C(i) = E(Key, A(i)) ^ M(i)   */
        for (i = 0; i < length; i += block_size)
        {
            A[15] = (UCHAR)(A[15] + 1);
            crypto_function(crypto_metadata, A, X, block_size);

            for (k = 0; (k < block_size) && ((i + k) < length); k++)
            {
                output[i + k] = X[k] ^ input[i + k];
            }
        }
    }

    if (op == NX_CRYPTO_DECRYPT_UPDATE)
    {
        
        /* Data authentication.  */
        if (ccm_metadata -> nx_crypto_ccm_icv_length > 0)
        {

            /* Compute authentication tag T.  */
            _nx_crypto_ccm_cbc_pad(crypto_metadata, crypto_function, output,
                                   ccm_metadata -> nx_crypto_ccm_X, length,
                                   ccm_metadata -> nx_crypto_ccm_X, block_size);
        }
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(X, 0, sizeof(X));
#endif

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ccm_encrypt_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates TAG for CCM mode.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    icv                                   Pointer to TAG buffer         */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_ccm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ccm_encrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *icv, UINT block_size)
{
UCHAR *A = ccm_metadata -> nx_crypto_ccm_A;
UINT i;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_CCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Data authentication.  */
    if (ccm_metadata -> nx_crypto_ccm_icv_length > 0)
    {

        /* The authentication tag T is the leftmost M bytes of the CBC-MAC value X(t + 1).  */
        NX_CRYPTO_MEMCPY(icv, ccm_metadata -> nx_crypto_ccm_X, ccm_metadata -> nx_crypto_ccm_icv_length); /* Use case of memcpy is verified. */

        /* Get encryption block X.  */
        A[15] = 0;
        crypto_function(crypto_metadata, A, A, block_size);

        /* Encrypt authentication tag.  */
        for (i = 0; i < ccm_metadata -> nx_crypto_ccm_icv_length; i++)
        {
            icv[i] = A[i] ^ icv[i];
        }
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(ccm_metadata, 0, sizeof(NX_CRYPTO_CCM));
#endif

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ccm_decrypt_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the TAG for CCM mode.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Pointer to crypto metadata    */
/*    gcm_metadata                          Pointer to GCM metadata       */
/*    crypto_function                       Pointer to crypto function    */
/*    icv                                   Pointer to TAG buffer         */
/*    block_size                            Block size of crypto algorithm*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_ccm_operation   Handle AES encrypt or decrypt */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ccm_decrypt_calculate(VOID *crypto_metadata, NX_CRYPTO_CCM *ccm_metadata,
                                                     UINT (*crypto_function)(VOID *, UCHAR *, UCHAR *, UINT),
                                                     UCHAR *icv, UINT block_size)
{
UCHAR temp[NX_CRYPTO_CCM_BLOCK_SIZE];
UINT i;

    /* Check the block size.  */
    if (block_size != NX_CRYPTO_CCM_BLOCK_SIZE)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Data authentication.  */
    if (ccm_metadata -> nx_crypto_ccm_icv_length > 0)
    {

        NX_CRYPTO_MEMCPY(temp, ccm_metadata -> nx_crypto_ccm_A, block_size); /* Use case of memcpy is verified. */
        temp[15] = 0;
        crypto_function(crypto_metadata, temp, temp, block_size);

        /* Encrypt authentication tag.  */
        for (i = 0; i < ccm_metadata -> nx_crypto_ccm_icv_length; i++)
        {
            temp[i] = temp[i] ^ icv[i];
        }

        /* The authentication tag T is the leftmost M bytes of the CBC-MAC value X(t + 1).  */
        if (NX_CRYPTO_MEMCMP(temp, ccm_metadata -> nx_crypto_ccm_X, ccm_metadata -> nx_crypto_ccm_icv_length))
        {

            /* Authentication failed.  */
            return(NX_CRYPTO_AUTHENTICATION_FAILED);
        }
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(ccm_metadata, 0, sizeof(NX_CRYPTO_CCM));
#endif

    return(NX_CRYPTO_SUCCESS);
}
