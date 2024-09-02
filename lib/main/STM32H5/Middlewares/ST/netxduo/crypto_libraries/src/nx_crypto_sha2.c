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
/**   SHA-256 Digest Algorithm (SHA2)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sha2.h"

/* Constants used in the SHA-256 digest calculation. */
const ULONG _sha2_round_constants[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


/* Define the SHA2 logic functions.  */
#define CH_FUNC(x, y, z)           (((x) & (y)) ^ ((~(x)) & (z)))
#define MAJ_FUNC(x, y, z)          (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define RIGHT_SHIFT_CIRCULAR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define LARGE_SIGMA_0(x)           (RIGHT_SHIFT_CIRCULAR((x),  2) ^ RIGHT_SHIFT_CIRCULAR((x), 13) ^ RIGHT_SHIFT_CIRCULAR((x), 22))
#define LARGE_SIGMA_1(x)           (RIGHT_SHIFT_CIRCULAR((x),  6) ^ RIGHT_SHIFT_CIRCULAR((x), 11) ^ RIGHT_SHIFT_CIRCULAR((x), 25))
#define SMALL_SIGMA_0(x)           (RIGHT_SHIFT_CIRCULAR((x),  7) ^ RIGHT_SHIFT_CIRCULAR((x), 18) ^ ((x) >> 3))
#define SMALL_SIGMA_1(x)           (RIGHT_SHIFT_CIRCULAR((x), 17) ^ RIGHT_SHIFT_CIRCULAR((x), 19) ^ ((x) >> 10))

#define W0(t) ((((ULONG)buffer[(t) * 4]) << 24) | (((ULONG)buffer[((t) * 4) + 1]) << 16) | (((ULONG)buffer[((t) * 4) + 2]) << 8) | ((ULONG)buffer[((t) * 4) + 3]))
#define W16(t) (SMALL_SIGMA_1(w[(t) - 2]) + w[(t) - 7] + SMALL_SIGMA_0(w[(t) - 15]) + w[(t) - 16])

/* Define the padding array.  This is used to pad the message such that its length is
   64 bits shy of being a multiple of 512 bits long.  */
const UCHAR   _nx_crypto_sha256_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_initialize                        PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the SHA256 context. It must be called     */
/*    prior to creating a SHA256 digest.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
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
/*    _nx_crypto_method_sha256_operation    Handle SHA256 operation       */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_sha256_initialize(NX_CRYPTO_SHA256 *context, UINT algorithm )
{
    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* First, clear the bit count for this context.  */
    context -> nx_sha256_bit_count[0] =  0;                   /* Clear the lower 32-bits of the count.*/
    context -> nx_sha256_bit_count[1] =  0;                   /* Clear the upper 32-bits of the count.*/

    if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256) ||
        (algorithm == NX_CRYPTO_HASH_SHA256))
    {

        /* Initialize SHA-256 state. */
        context -> nx_sha256_states[0] = 0x6a09e667; /* A H0 */
        context -> nx_sha256_states[1] = 0xbb67ae85; /* B H1 */
        context -> nx_sha256_states[2] = 0x3c6ef372; /* C H2 */
        context -> nx_sha256_states[3] = 0xa54ff53a; /* D H3 */
        context -> nx_sha256_states[4] = 0x510e527f; /* E H4 */
        context -> nx_sha256_states[5] = 0x9b05688c; /* F H5 */
        context -> nx_sha256_states[6] = 0x1f83d9ab; /* G H6 */
        context -> nx_sha256_states[7] = 0x5be0cd19; /* H H7 */
    }
    else
    {

        /* Initialize SHA-224 state. */
        context -> nx_sha256_states[0] = 0xc1059ed8; /* A H0 */
        context -> nx_sha256_states[1] = 0x367cd507; /* B H1 */
        context -> nx_sha256_states[2] = 0x3070dd17; /* C H2 */
        context -> nx_sha256_states[3] = 0xf70e5939; /* D H3 */
        context -> nx_sha256_states[4] = 0xffc00b31; /* E H4 */
        context -> nx_sha256_states[5] = 0x68581511; /* F H5 */
        context -> nx_sha256_states[6] = 0x64f98fa7; /* G H6 */
        context -> nx_sha256_states[7] = 0xbefa4fa4; /* H H7 */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_update                            PORTABLE C      */
/*                                                           6.1          */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the SHA256 digest with new input from the     */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    input_ptr                             Pointer to input data         */
/*    input_length                          Number of bytes in input      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_process_buffer      Process complete buffer       */
/*                                            using SHA256                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_sha256_digest_calculate   Calculate the SHA256 digest    */
/*    _nx_crypto_method_sha256_operation   Handle SHA256 operation        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_sha256_update(NX_CRYPTO_SHA256 *context, UCHAR *input_ptr, UINT input_length)
{
ULONG current_bytes;
ULONG needed_fill_bytes;

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

    /* Calculate the current byte count mod 64. Note the reason for the
       shift by 3 is to account for the 8 bits per byte.  */
    current_bytes =  (context -> nx_sha256_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  64 - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_sha256_bit_count[0] += (input_length << 3);

    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_sha256_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_sha256_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_sha256_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {
        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal
           buffer of the context.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha256_buffer[current_bytes]), (void *)input_ptr, needed_fill_bytes); /* Use case of memcpy is verified. */

        /* Process the 64-byte (512 bit) buffer.  */
        _nx_crypto_sha256_process_buffer(context, context -> nx_sha256_buffer);

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - needed_fill_bytes;
        input_ptr =     input_ptr + needed_fill_bytes;

        /* Clear the remaining bits, since the buffer was processed.  */
        current_bytes =  0;
    }

    /* Process any and all whole blocks of input.  */
    while (input_length >= 64)
    {

        /* Process this 64-byte (512 bit) buffer.  */
        _nx_crypto_sha256_process_buffer(context, input_ptr);

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - 64;
        input_ptr =     input_ptr + 64;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {
        /* Save the remaining bytes in the internal buffer after any remaining bytes
           so that it is processed later.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha256_buffer[current_bytes]), (void *)input_ptr, input_length); /* Use case of memcpy is verified. */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_digest_calculate                  PORTABLE C      */
/*                                                           6.1.10       */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the final SHA256 digest. It is called      */
/*    when there is no more input for the digest and returns the 32-byte  */
/*    (256-bit) SHA256 digest to the caller.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    digest                                Pointer to return buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_update              Final update to the digest    */
/*                                            with padding and length     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_sha256_operation    Handle SHA256 operation       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_sha256_digest_calculate(NX_CRYPTO_SHA256 *context, UCHAR *digest, UINT algorithm)
{
UINT bit_count_string[2];
ULONG current_byte_count;
ULONG padding_bytes;


    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] = context -> nx_sha256_bit_count[1];
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(bit_count_string[0]);
    bit_count_string[1] = context -> nx_sha256_bit_count[0];
    NX_CRYPTO_CHANGE_ULONG_ENDIAN(bit_count_string[1]);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_sha256_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_sha256_update(context, (UCHAR*)_nx_crypto_sha256_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_sha256_update(context, (UCHAR*)bit_count_string, 8);

    /* Now store the digest in the caller specified destination.  */
    digest[0] =  (UCHAR)(context -> nx_sha256_states[0] >> 24);
    digest[1] =  (UCHAR)(context -> nx_sha256_states[0] >> 16);
    digest[2] =  (UCHAR)(context -> nx_sha256_states[0] >> 8);
    digest[3] =  (UCHAR)(context -> nx_sha256_states[0]);
    digest[4] =  (UCHAR)(context -> nx_sha256_states[1] >> 24);
    digest[5] =  (UCHAR)(context -> nx_sha256_states[1] >> 16);
    digest[6] =  (UCHAR)(context -> nx_sha256_states[1] >> 8);
    digest[7] =  (UCHAR)(context -> nx_sha256_states[1]);
    digest[8] =  (UCHAR)(context -> nx_sha256_states[2] >> 24);
    digest[9] =  (UCHAR)(context -> nx_sha256_states[2] >> 16);
    digest[10] =  (UCHAR)(context -> nx_sha256_states[2] >> 8);
    digest[11] =  (UCHAR)(context -> nx_sha256_states[2]);
    digest[12] =  (UCHAR)(context -> nx_sha256_states[3] >> 24);
    digest[13] =  (UCHAR)(context -> nx_sha256_states[3] >> 16);
    digest[14] =  (UCHAR)(context -> nx_sha256_states[3] >> 8);
    digest[15] =  (UCHAR)(context -> nx_sha256_states[3]);
    digest[16] =  (UCHAR)(context -> nx_sha256_states[4] >> 24);
    digest[17] =  (UCHAR)(context -> nx_sha256_states[4] >> 16);
    digest[18] =  (UCHAR)(context -> nx_sha256_states[4] >> 8);
    digest[19] =  (UCHAR)(context -> nx_sha256_states[4]);
    digest[20] =  (UCHAR)(context -> nx_sha256_states[5] >> 24);
    digest[21] =  (UCHAR)(context -> nx_sha256_states[5] >> 16);
    digest[22] =  (UCHAR)(context -> nx_sha256_states[5] >> 8);
    digest[23] =  (UCHAR)(context -> nx_sha256_states[5]);
    digest[24] =  (UCHAR)(context -> nx_sha256_states[6] >> 24);
    digest[25] =  (UCHAR)(context -> nx_sha256_states[6] >> 16);
    digest[26] =  (UCHAR)(context -> nx_sha256_states[6] >> 8);
    digest[27] =  (UCHAR)(context -> nx_sha256_states[6]);
    if ((algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256) ||
        (algorithm == NX_CRYPTO_HASH_SHA256))
    {
        digest[28] =  (UCHAR)(context -> nx_sha256_states[7] >> 24);
        digest[29] =  (UCHAR)(context -> nx_sha256_states[7] >> 16);
        digest[30] =  (UCHAR)(context -> nx_sha256_states[7] >> 8);
        digest[31] =  (UCHAR)(context -> nx_sha256_states[7]);
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
/*    _nx_crypto_sha256_process_buffer                    PORTABLE C      */
/*                                                           6.1.10       */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements the SHA256 algorithm which works on        */
/*    64-byte (512-bit) blocks of data.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    buffer                                Pointer to 64-byte buffer     */
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
/*    _nx_crypto_sha256_update              Final update to the digest    */
/*                                            with padding and length     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_sha256_process_buffer(NX_CRYPTO_SHA256 *context, UCHAR buffer[64])
{
ULONG *w;
UINT   t;
ULONG  temp1, temp2;
ULONG  a, b, c, d, e, f, g, h;


    /* Setup pointers to the word array.  */
    w =  context -> nx_sha256_word_array;

    /* Initialize the state variables.  */
    a =  context -> nx_sha256_states[0];
    b =  context -> nx_sha256_states[1];
    c =  context -> nx_sha256_states[2];
    d =  context -> nx_sha256_states[3];
    e =  context -> nx_sha256_states[4];
    f =  context -> nx_sha256_states[5];
    g =  context -> nx_sha256_states[6];
    h =  context -> nx_sha256_states[7];

    /* Now, perform Round operations.  */
    for (t = 0; t < 16; t += 8)
    {

        /* Setup each entry.  */
        w[t] =  W0(t);
        temp1 = h + LARGE_SIGMA_1(e) + CH_FUNC(e, f, g) + _sha2_round_constants[t] + w[t];
        temp2 = LARGE_SIGMA_0(a) + MAJ_FUNC(a, b, c);
        d = d + temp1;
        h = temp1 + temp2;

        w[t + 1] =  W0(t + 1);
        temp1 = g + LARGE_SIGMA_1(d) + CH_FUNC(d, e, f) + _sha2_round_constants[t + 1] + w[t + 1];
        temp2 = LARGE_SIGMA_0(h) + MAJ_FUNC(h, a, b);
        c = c + temp1;
        g = temp1 + temp2;

        w[t + 2] =  W0(t + 2);
        temp1 = f + LARGE_SIGMA_1(c) + CH_FUNC(c, d, e) + _sha2_round_constants[t + 2] + w[t + 2];
        temp2 = LARGE_SIGMA_0(g) + MAJ_FUNC(g, h, a);
        b = b + temp1;
        f = temp1 + temp2;

        w[t + 3] =  W0(t + 3);
        temp1 = e + LARGE_SIGMA_1(b) + CH_FUNC(b, c, d) + _sha2_round_constants[t + 3] + w[t + 3];
        temp2 = LARGE_SIGMA_0(f) + MAJ_FUNC(f, g, h);
        a = a + temp1;
        e = temp1 + temp2;

        w[t + 4] =  W0(t + 4);
        temp1 = d + LARGE_SIGMA_1(a) + CH_FUNC(a, b, c) + _sha2_round_constants[t + 4] + w[t + 4];
        temp2 = LARGE_SIGMA_0(e) + MAJ_FUNC(e, f, g);
        h = h + temp1;
        d = temp1 + temp2;


        w[t + 5] =  W0(t + 5);
        temp1 = c + LARGE_SIGMA_1(h) + CH_FUNC(h, a, b) + _sha2_round_constants[t + 5] + w[t + 5];
        temp2 = LARGE_SIGMA_0(d) + MAJ_FUNC(d, e, f);
        g = g + temp1;
        c = temp1 + temp2;

        w[t + 6] =  W0(t + 6);
        temp1 = b + LARGE_SIGMA_1(g) + CH_FUNC(g, h, a) + _sha2_round_constants[t + 6] + w[t + 6];
        temp2 = LARGE_SIGMA_0(c) + MAJ_FUNC(c, d, e);
        f = f + temp1;
        b = temp1 + temp2;

        w[t + 7] =  W0(t + 7);
        temp1 = a + LARGE_SIGMA_1(f) + CH_FUNC(f, g, h) + _sha2_round_constants[t + 7] + w[t + 7];
        temp2 = LARGE_SIGMA_0(b) + MAJ_FUNC(b, c, d);
        e = e + temp1;
        a = temp1 + temp2;
    }

    for (; t < 64; t += 8)
    {

        /* Setup each entry.  */
        w[t] =  W16(t);
        temp1 = h + LARGE_SIGMA_1(e) + CH_FUNC(e, f, g) + _sha2_round_constants[t] + w[t];
        temp2 = LARGE_SIGMA_0(a) + MAJ_FUNC(a, b, c);
        d = d + temp1;
        h = temp1 + temp2;

        w[t + 1] =  W16(t + 1);
        temp1 = g + LARGE_SIGMA_1(d) + CH_FUNC(d, e, f) + _sha2_round_constants[t + 1] + w[t + 1];
        temp2 = LARGE_SIGMA_0(h) + MAJ_FUNC(h, a, b);
        c = c + temp1;
        g = temp1 + temp2;

        w[t + 2] =  W16(t + 2);
        temp1 = f + LARGE_SIGMA_1(c) + CH_FUNC(c, d, e) + _sha2_round_constants[t + 2] + w[t + 2];
        temp2 = LARGE_SIGMA_0(g) + MAJ_FUNC(g, h, a);
        b = b + temp1;
        f = temp1 + temp2;

        w[t + 3] =  W16(t + 3);
        temp1 = e + LARGE_SIGMA_1(b) + CH_FUNC(b, c, d) + _sha2_round_constants[t + 3] + w[t + 3];
        temp2 = LARGE_SIGMA_0(f) + MAJ_FUNC(f, g, h);
        a = a + temp1;
        e = temp1 + temp2;

        w[t + 4] =  W16(t + 4);
        temp1 = d + LARGE_SIGMA_1(a) + CH_FUNC(a, b, c) + _sha2_round_constants[t + 4] + w[t + 4];
        temp2 = LARGE_SIGMA_0(e) + MAJ_FUNC(e, f, g);
        h = h + temp1;
        d = temp1 + temp2;


        w[t + 5] =  W16(t + 5);
        temp1 = c + LARGE_SIGMA_1(h) + CH_FUNC(h, a, b) + _sha2_round_constants[t + 5] + w[t + 5];
        temp2 = LARGE_SIGMA_0(d) + MAJ_FUNC(d, e, f);
        g = g + temp1;
        c = temp1 + temp2;

        w[t + 6] =  W16(t + 6);
        temp1 = b + LARGE_SIGMA_1(g) + CH_FUNC(g, h, a) + _sha2_round_constants[t + 6] + w[t + 6];
        temp2 = LARGE_SIGMA_0(c) + MAJ_FUNC(c, d, e);
        f = f + temp1;
        b = temp1 + temp2;

        w[t + 7] =  W16(t + 7);
        temp1 = a + LARGE_SIGMA_1(f) + CH_FUNC(f, g, h) + _sha2_round_constants[t + 7] + w[t + 7];
        temp2 = LARGE_SIGMA_0(b) + MAJ_FUNC(b, c, d);
        e = e + temp1;
        a = temp1 + temp2;

    }

    /* Save the resulting in this SHA256 context.  */
    context -> nx_sha256_states[0] +=  a;
    context -> nx_sha256_states[1] +=  b;
    context -> nx_sha256_states[2] +=  c;
    context -> nx_sha256_states[3] +=  d;
    context -> nx_sha256_states[4] +=  e;
    context -> nx_sha256_states[5] +=  f;
    context -> nx_sha256_states[6] +=  g;
    context -> nx_sha256_states[7] +=  h;

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
/*    _nx_crypto_method_sha256_init                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported SHA256 cryptographic algorithm.                 */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha256_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_SHA256))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha256_cleanup                    PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha256_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_SHA256));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha256_operation                 PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the SHA256 algorithm.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    SHA256 operation              */
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
/*    _nx_crypto_sha256_initialize          Initialize the SHA256 context */
/*    _nx_crypto_sha256_update              Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_sha256_digest_calculate    Calculate the SHA256 digest   */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha256_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_SHA256 *ctx;

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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_SHA256))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ctx = (NX_CRYPTO_SHA256 *)crypto_metadata;

    if ((method -> nx_crypto_algorithm != NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256) &&
        (method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA224) &&
        (method -> nx_crypto_algorithm != NX_CRYPTO_HASH_SHA256))
    {
        /* Incorrect method. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    switch (op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        _nx_crypto_sha256_initialize((NX_CRYPTO_SHA256 *)crypto_metadata, method -> nx_crypto_algorithm);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_sha256_update((NX_CRYPTO_SHA256 *)crypto_metadata, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        if ((method -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256) ||
            (method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA256))
        {
            if(output_length_in_byte < 32)
                return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }
        else if(output_length_in_byte < 28)
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        _nx_crypto_sha256_digest_calculate((NX_CRYPTO_SHA256 *)crypto_metadata, output,
                                           method -> nx_crypto_algorithm);
        break;

    default:
        if ((method -> nx_crypto_algorithm == NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256) ||
            (method -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA256))
        {
            if(output_length_in_byte < 32)
                return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        }
        else if(output_length_in_byte < 28)
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);

        _nx_crypto_sha256_initialize(ctx, method -> nx_crypto_algorithm);
        _nx_crypto_sha256_update(ctx, input, input_length_in_byte);
        _nx_crypto_sha256_digest_calculate(ctx, output, method -> nx_crypto_algorithm);
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

