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
/**   SHA-512 Digest Algorithm (SHA5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sha5.h"

/* Constants used in the SHA-512 digest calculation. */
const ULONG64 _sha5_round_constants[] =
{
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817,
};


/* Define the SHA5 logic functions.  */
#define CH_FUNC(x, y, z)           (((x) & (y)) ^ ((~(x)) & (z)))
#define MAJ_FUNC(x, y, z)          (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define RIGHT_SHIFT_CIRCULAR(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define LARGE_SIGMA_0(x)           (RIGHT_SHIFT_CIRCULAR((x),  28) ^ RIGHT_SHIFT_CIRCULAR((x), 34) ^ RIGHT_SHIFT_CIRCULAR((x), 39))
#define LARGE_SIGMA_1(x)           (RIGHT_SHIFT_CIRCULAR((x),  14) ^ RIGHT_SHIFT_CIRCULAR((x), 18) ^ RIGHT_SHIFT_CIRCULAR((x), 41))
#define SMALL_SIGMA_0(x)           (RIGHT_SHIFT_CIRCULAR((x),  1) ^ RIGHT_SHIFT_CIRCULAR((x), 8) ^ ((x) >> 7))
#define SMALL_SIGMA_1(x)           (RIGHT_SHIFT_CIRCULAR((x), 19) ^ RIGHT_SHIFT_CIRCULAR((x), 61) ^ ((x) >> 6))

/* Define the padding array.  This is used to pad the message such that its length is
   64 bits shy of being a multiple of 512 bits long.  */
const UCHAR   _nx_crypto_sha512_padding[] =
{
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha512_initialize                        PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the SHA512 context. It must be called     */
/*    prior to creating a SHA512 digest.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA512 context pointer        */
/*    algorithm                             SHA384 or SHA512              */
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
/*    _nx_crypto_method_sha512_operation    Handle SHA512 operation       */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_sha512_initialize(NX_CRYPTO_SHA512 *context, UINT algorithm)
{
    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* First, clear the bit count for this context.  */
    context -> nx_sha512_bit_count[0] =  0;                   /* Clear the lower 64-bits of the count.*/
    context -> nx_sha512_bit_count[1] =  0;                   /* Clear the upper 64-bits of the count.*/

    if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512) ||
        (algorithm == NX_CRYPTO_HASH_SHA512))
    {

        /* Initialize SHA-512 state. */
        context -> nx_sha512_states[0] = 0x6a09e667f3bcc908; /* A H0 */
        context -> nx_sha512_states[1] = 0xbb67ae8584caa73b; /* B H1 */
        context -> nx_sha512_states[2] = 0x3c6ef372fe94f82b; /* C H2 */
        context -> nx_sha512_states[3] = 0xa54ff53a5f1d36f1; /* D H3 */
        context -> nx_sha512_states[4] = 0x510e527fade682d1; /* E H4 */
        context -> nx_sha512_states[5] = 0x9b05688c2b3e6c1f; /* F H5 */
        context -> nx_sha512_states[6] = 0x1f83d9abfb41bd6b; /* G H6 */
        context -> nx_sha512_states[7] = 0x5be0cd19137e2179; /* H H7 */
    }
    else if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_224) ||
             (algorithm == NX_CRYPTO_HASH_SHA512_224))
    {

        /* Initialize SHA-512/224 state. */
        context -> nx_sha512_states[0] = 0x8c3d37c819544da2; /* A H0 */
        context -> nx_sha512_states[1] = 0x73e1996689dcd4d6; /* B H1 */
        context -> nx_sha512_states[2] = 0x1dfab7ae32ff9c82; /* C H2 */
        context -> nx_sha512_states[3] = 0x679dd514582f9fcf; /* D H3 */
        context -> nx_sha512_states[4] = 0x0f6d2b697bd44da8; /* E H4 */
        context -> nx_sha512_states[5] = 0x77e36f7304c48942; /* F H5 */
        context -> nx_sha512_states[6] = 0x3f9d85a86a1d36c8; /* G H6 */
        context -> nx_sha512_states[7] = 0x1112e6ad91d692a1; /* H H7 */
    }
    else if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_256) ||
             (algorithm == NX_CRYPTO_HASH_SHA512_256))
    {

        /* Initialize SHA-512/256 state. */
        context -> nx_sha512_states[0] = 0x22312194fc2bf72c; /* A H0 */
        context -> nx_sha512_states[1] = 0x9f555fa3c84c64c2; /* B H1 */
        context -> nx_sha512_states[2] = 0x2393b86b6f53b151; /* C H2 */
        context -> nx_sha512_states[3] = 0x963877195940eabd; /* D H3 */
        context -> nx_sha512_states[4] = 0x96283ee2a88effe3; /* E H4 */
        context -> nx_sha512_states[5] = 0xbe5e1e2553863992; /* F H5 */
        context -> nx_sha512_states[6] = 0x2b0199fc2c85b8aa; /* G H6 */
        context -> nx_sha512_states[7] = 0x0eb72ddc81c52ca2; /* H H7 */
    }
    else
    {

        /* Initialize SHA-384 state. */
        context -> nx_sha512_states[0] = 0xcbbb9d5dc1059ed8; /* A H0 */
        context -> nx_sha512_states[1] = 0x629a292a367cd507; /* B H1 */
        context -> nx_sha512_states[2] = 0x9159015a3070dd17; /* C H2 */
        context -> nx_sha512_states[3] = 0x152fecd8f70e5939; /* D H3 */
        context -> nx_sha512_states[4] = 0x67332667ffc00b31; /* E H4 */
        context -> nx_sha512_states[5] = 0x8eb44a8768581511; /* F H5 */
        context -> nx_sha512_states[6] = 0xdb0c2e0d64f98fa7; /* G H6 */
        context -> nx_sha512_states[7] = 0x47b5481dbefa4fa4; /* H H7 */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha512_update                            PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the SHA512 digest with new input from the     */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA512 context pointer        */
/*    input_ptr                             Pointer to input data         */
/*    input_length                          Number of bytes in input      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha512_process_buffer      Process complete buffer       */
/*                                            using SHA512                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_sha512_digest_calculate    Calculate the SHA512 digest   */
/*    _nx_crypto_method_sha512_operation    Handle SHA512 operation       */
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
NX_CRYPTO_KEEP UINT _nx_crypto_sha512_update(NX_CRYPTO_SHA512 *context, UCHAR *input_ptr, UINT input_length)
{
ULONG64 current_bytes;
ULONG64 needed_fill_bytes;

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Determine if there is a length.  */
    if (input_length == 0)
    {
        return(NX_CRYPTO_SUCCESS);
    }

    /* Calculate the current byte count mod 128. Note the reason for the
       shift by 3 is to account for the 8 bits per byte.  */
    current_bytes =  (context -> nx_sha512_bit_count[0] >> 3) & 0x7F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_sha512_bit_count[0] += (input_length << 3);

    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_sha512_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_sha512_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_sha512_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {
        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal
           buffer of the context.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha512_buffer[current_bytes]), (void *)input_ptr, (UINT)needed_fill_bytes); /* Use case of memcpy is verified. */

        /* Process the 128-byte (1024 bit) buffer.  */
        _nx_crypto_sha512_process_buffer(context, context -> nx_sha512_buffer);

        /* Adjust the pointers and length accordingly.  */
        input_length =  (UINT)(input_length - needed_fill_bytes);
        input_ptr =     input_ptr + needed_fill_bytes;

        /* Clear the remaining bits, since the buffer was processed.  */
        current_bytes =  0;
    }

    /* Process any and all whole blocks of input.  */
    while (input_length >= NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES)
    {

        /* Process this 128-byte (1024 bit) buffer.  */
        _nx_crypto_sha512_process_buffer(context, input_ptr);

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES;
        input_ptr =     input_ptr + NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {
        /* Save the remaining bytes in the internal buffer after any remaining bytes
           so that it is processed later.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha512_buffer[current_bytes]), (void *)input_ptr, input_length); /* Use case of memcpy is verified. */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha512_digest_calculate                  PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the final SHA512 digest. It is called      */
/*    when there is no more input for the digest and returns the 32-byte  */
/*    (512-bit) SHA512 digest to the caller.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA512 context pointer        */
/*    digest                                Pointer to return buffer      */
/*    algorithm                             SHA384 or SHA512              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha512_update              Final update to the digest    */
/*                                            with padding and length     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_sha512_operation    Handle SHA512 operation       */
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
NX_CRYPTO_KEEP UINT _nx_crypto_sha512_digest_calculate(NX_CRYPTO_SHA512 *context, UCHAR *digest, UINT algorithm)
{
UCHAR bit_count_string[16];
ULONG current_byte_count;
ULONG padding_bytes;
UINT  i;
UINT  loop;


    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 56);
    bit_count_string[1] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 48);
    bit_count_string[2] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 40);
    bit_count_string[3] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 32);
    bit_count_string[4] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 24);
    bit_count_string[5] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 16);
    bit_count_string[6] =  (UCHAR)(context -> nx_sha512_bit_count[1] >> 8);
    bit_count_string[7] =  (UCHAR)(context -> nx_sha512_bit_count[1]);
    bit_count_string[8] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 56);
    bit_count_string[9] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 48);
    bit_count_string[10] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 40);
    bit_count_string[11] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 32);
    bit_count_string[12] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 24);
    bit_count_string[13] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 16);
    bit_count_string[14] =  (UCHAR)(context -> nx_sha512_bit_count[0] >> 8);
    bit_count_string[15] =  (UCHAR)(context -> nx_sha512_bit_count[0]);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_sha512_bit_count[0] >> 3) & 0x7F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 112) ? (112 - current_byte_count) : (240 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_sha512_update(context, (UCHAR *)_nx_crypto_sha512_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_sha512_update(context, bit_count_string, sizeof(bit_count_string));

    if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512) ||
        (algorithm == NX_CRYPTO_HASH_SHA512))
    {
        loop = 8;
    }
    else if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_224) ||
             (algorithm == NX_CRYPTO_HASH_SHA512_224))
    {
        loop = 3;
    }
    else if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_256) ||
             (algorithm == NX_CRYPTO_HASH_SHA512_256))
    {
        loop = 4;
    }
    else
    {
        loop = 6;
    }

    /* Now store the digest in the caller specified destination.  */
    for (i = 0; i < loop; i++)
    {
        digest[0] =  (UCHAR)(context -> nx_sha512_states[i] >> 56);
        digest[1] =  (UCHAR)(context -> nx_sha512_states[i] >> 48);
        digest[2] =  (UCHAR)(context -> nx_sha512_states[i] >> 40);
        digest[3] =  (UCHAR)(context -> nx_sha512_states[i] >> 32);
        digest[4] =  (UCHAR)(context -> nx_sha512_states[i] >> 24);
        digest[5] =  (UCHAR)(context -> nx_sha512_states[i] >> 16);
        digest[6] =  (UCHAR)(context -> nx_sha512_states[i] >> 8);
        digest[7] =  (UCHAR)(context -> nx_sha512_states[i]);
        digest += 8;
    }

    if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_224) ||
        (algorithm == NX_CRYPTO_HASH_SHA512_224))
    {

        /* The last 32 bits for SHA512/224. */
        digest[0] =  (UCHAR)(context -> nx_sha512_states[3] >> 56);
        digest[1] =  (UCHAR)(context -> nx_sha512_states[3] >> 48);
        digest[2] =  (UCHAR)(context -> nx_sha512_states[3] >> 40);
        digest[3] =  (UCHAR)(context -> nx_sha512_states[3] >> 32);
    }

#ifdef NX_SECURE_KEY_CLEAR
    NX_CRYPTO_MEMSET(bit_count_string, 0, sizeof(bit_count_string));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Return successful completion.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha512_process_buffer                    PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements the SHA512 algorithm which works on        */
/*    128-byte (1024-bit) blocks of data.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA512 context pointer        */
/*    buffer                                Pointer to 128-byte buffer    */
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
/*    _nx_crypto_sha512_update              Update the digest with padding*/
/*                                            and length of digest        */
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
NX_CRYPTO_KEEP VOID _nx_crypto_sha512_process_buffer(NX_CRYPTO_SHA512 *context, UCHAR *buffer)
{
ULONG64 *w;
UINT     t;
ULONG64  temp1, temp2;
ULONG64  a, b, c, d, e, f, g, h;


    /* Setup pointers to the word array.  */
    w =  context -> nx_sha512_word_array;

    /* Initialize the first 16 words of the word array, taking care of the
       endian issues at the same time.  */
    for (t = 0; t < 16; t++)
    {
        /* Setup each entry.  */
        w[t] =  (((ULONG64)buffer[0]) << 56) |
            (((ULONG64)buffer[1]) << 48) |
            (((ULONG64)buffer[2]) << 40) |
            (((ULONG64)buffer[3]) << 32) |
            (((ULONG64)buffer[4]) << 24) |
            (((ULONG64)buffer[5]) << 16) |
            (((ULONG64)buffer[6]) << 8) |
            ((ULONG64)buffer[7]);
        buffer += 8;
    }

    /* Setup the remaining entries of the word array.  */
    for (t = 16; t < 80; t++)
    {
        /* Setup each entry.  */
        w[t] =  SMALL_SIGMA_1(w[t - 2]) + w[t - 7] + SMALL_SIGMA_0(w[t - 15]) + w[t - 16];
    }

    /* Initialize the state variables.  */
    a =  context -> nx_sha512_states[0];
    b =  context -> nx_sha512_states[1];
    c =  context -> nx_sha512_states[2];
    d =  context -> nx_sha512_states[3];
    e =  context -> nx_sha512_states[4];
    f =  context -> nx_sha512_states[5];
    g =  context -> nx_sha512_states[6];
    h =  context -> nx_sha512_states[7];

    /* Now, perform Round operations.  */
    for (t = 0; t < 80; t++)
    {
        temp1 = h + LARGE_SIGMA_1(e) + CH_FUNC(e, f, g) + _sha5_round_constants[t] + w[t];
        temp2 = LARGE_SIGMA_0(a) + MAJ_FUNC(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    /* Save the resulting in this SHA512 context.  */
    context -> nx_sha512_states[0] +=  a;
    context -> nx_sha512_states[1] +=  b;
    context -> nx_sha512_states[2] +=  c;
    context -> nx_sha512_states[3] +=  d;
    context -> nx_sha512_states[4] +=  e;
    context -> nx_sha512_states[5] +=  f;
    context -> nx_sha512_states[6] +=  g;
    context -> nx_sha512_states[7] +=  h;

#ifdef NX_SECURE_KEY_CLEAR
    a = 0; b = 0; c = 0; d = 0;
    e = 0; f = 0; g = 0; h = 0;
    temp1 = 0; temp2 = 0;
#endif /* NX_SECURE_KEY_CLEAR  */
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha512_init                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported SHA512 cryptographic algorithm.                 */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha512_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                   UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                   VOID  **handle,
                                                   VOID  *crypto_metadata,
                                                   ULONG crypto_metadata_size)
{

    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_SHA512))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha512_cleanup                    PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha512_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_SHA512));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha512_operation                 PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the SHA512 algorithm.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    SHA512 operation              */
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
/*    _nx_crypto_sha512_initialize          Initialize the SHA512 context */
/*    _nx_crypto_sha512_update              Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_sha512_digest_calculate    Calculate the SHA512 digest   */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha512_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
UINT                status = NX_CRYPTO_NOT_SUCCESSFUL;
NX_CRYPTO_SHA512   *ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    if (method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_SHA512))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_SHA512 *)crypto_metadata;

    if (op != NX_CRYPTO_AUTHENTICATE && op != NX_CRYPTO_VERIFY && op != NX_CRYPTO_HASH_INITIALIZE &&
        op != NX_CRYPTO_HASH_UPDATE && op != NX_CRYPTO_HASH_CALCULATE)
    {
        /* Incorrect Operation. */
        return status;
    }

    if ((method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA384) &&
        (method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA512) &&
        (method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA512_224) &&
        (method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA512_256))
    {
        /* Incorrect method. */
        return status;
    }

    switch (op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        _nx_crypto_sha512_initialize((NX_CRYPTO_SHA512 *)crypto_metadata, method -> nx_crypto_algorithm);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_sha512_update((NX_CRYPTO_SHA512 *)crypto_metadata, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        if(((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512) && (output_length_in_byte < 64)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA384) && (output_length_in_byte < 48)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512_224) && (output_length_in_byte < 28)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512_256) && (output_length_in_byte < 32)))
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);


        _nx_crypto_sha512_digest_calculate((NX_CRYPTO_SHA512 *)crypto_metadata, output, method -> nx_crypto_algorithm);
        break;

    default:
        if(((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512) && (output_length_in_byte < 64)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA384) && (output_length_in_byte < 48)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512_224) && (output_length_in_byte < 28)) ||
           ((method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA512_256) && (output_length_in_byte < 32)))
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        _nx_crypto_sha512_initialize(ctx, method -> nx_crypto_algorithm);
        _nx_crypto_sha512_update(ctx, input, input_length_in_byte);
        _nx_crypto_sha512_digest_calculate(ctx, output, method -> nx_crypto_algorithm);
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

