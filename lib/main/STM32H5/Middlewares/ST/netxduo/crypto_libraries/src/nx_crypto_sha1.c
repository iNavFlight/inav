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
/*
   Copyright (C) The Internet Society (2001).  All Rights Reserved.

   This document and translations of it may be copied and furnished to
   others, and derivative works that comment on or otherwise explain it
   or assist in its implementation may be prepared, copied, published
   and distributed, in whole or in part, without restriction of any
   kind, provided that the above copyright notice and this paragraph are
   included on all such copies and derivative works.  However, this
   document itself may not be modified in any way, such as by removing
   the copyright notice or references to the Internet Society or other
   Internet organizations, except as needed for the purpose of
   developing Internet standards in which case the procedures for
   copyrights defined in the Internet Standards process must be
   followed, or as required to translate it into languages other than
   English.

   The limited permissions granted above are perpetual and will not be
   revoked by the Internet Society or its successors or assigns.

   This document and the information contained herein is provided on an
   ""AS IS"" basis and THE INTERNET SOCIETY AND THE INTERNET ENGINEERING
   TASK FORCE DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
   BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE INFORMATION
   HEREIN WILL NOT INFRINGE ANY RIGHTS OR ANY IMPLIED WARRANTIES OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
*/
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   SHA1 Digest Algorithm (SHA1)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sha1.h"

/* Define macros for the SHA1 transform function.  */

/* Define the SHA1 basic F1, F2, F3, and F4 functions.  */

#define F1(x, y, z)               (((x) & (y)) | ((~x) & (z)))
#define F2(x, y, z)               ((x) ^ (y) ^ (z))
#define F3(x, y, z)               (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define F4(x, y, z)               ((x) ^ (y) ^ (z))


/* Define the SHA1 left shift circular function.  */

#define LEFT_SHIFT_CIRCULAR(x, n) (((x) << (n)) | ((x) >> (32 - (n))))


/* Define the padding array.  This is used to pad the message such that its length is
   64 bits shy of being a multiple of 512 bits long.  */

const UCHAR   _nx_crypto_sha1_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha1_initialize                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the SHA1 context. It must be called prior */
/*    to creating the SHA1 digest.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA1 context pointer          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    algorithm                             Algorithm type                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_sha1_operation      Handle SHA1 operation         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            updated constants,          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_sha1_initialize(NX_CRYPTO_SHA1 *context, UINT algorithm)
{
    NX_CRYPTO_PARAMETER_NOT_USED(algorithm);

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* First, clear the bit count for this context.  */
    context -> nx_sha1_bit_count[0] =  0;                   /* Clear the lower 32-bits of the count.*/
    context -> nx_sha1_bit_count[1] =  0;                   /* Clear the upper 32-bits of the count.*/

    /* Finally, setup the context states.  */
    context -> nx_sha1_states[0] =  0x67452301UL;           /* Setup state A.                       */
    context -> nx_sha1_states[1] =  0xEFCDAB89UL;           /* Setup state B.                       */
    context -> nx_sha1_states[2] =  0x98BADCFEUL;           /* Setup state C.                       */
    context -> nx_sha1_states[3] =  0x10325476UL;           /* Setup state D.                       */
    context -> nx_sha1_states[4] =  0xC3D2E1F0UL;           /* Setup state E.                       */

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha1_update                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the digest calculation with new input from    */
/*    the caller.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA1 context pointer          */
/*    input_ptr                             Pointer to byte(s) of input   */
/*    input_length                          Length of bytes of input      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha1_process_buffer        Process complete buffer       */
/*                                            using SHA1                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_sha1_digest_calculate      Calculate the SHA1 digest     */
/*    _nx_crypto_method_sha1_operation      Handle SHA1 operation         */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_sha1_update(NX_CRYPTO_SHA1 *context, UCHAR *input_ptr, UINT input_length)
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
    current_bytes =  (context -> nx_sha1_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  64 - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_sha1_bit_count[0] += (input_length << 3);

    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_sha1_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_sha1_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_sha1_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {

        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal
           buffer of the context.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha1_buffer[current_bytes]), (void *)input_ptr, needed_fill_bytes); /* Use case of memcpy is verified. */

        /* Process the 64-byte (512 bit) buffer.  */
        _nx_crypto_sha1_process_buffer(context, context -> nx_sha1_buffer);

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
        _nx_crypto_sha1_process_buffer(context, input_ptr);

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - 64;
        input_ptr =     input_ptr + 64;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {

        /* Save the remaining bytes in the internal buffer after any remaining bytes
           that it is processed later.  */
        NX_CRYPTO_MEMCPY((void *)&(context -> nx_sha1_buffer[current_bytes]), (void *)input_ptr, input_length); /* Use case of memcpy is verified. */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha1_digest_calculate                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finishes calculation of the SHA1 digest. It is called */
/*    where there is no further input needed for the digest. The resulting*/
/*    20-byte (160-bit) SHA1 digest is returned to the caller.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA1 context pointer          */
/*    digest                                Pointer to return digest in   */
/*    algorithm                             Algorithm type                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha1_update                Update the digest with padding*/
/*                                            and length of digest        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_sha1_operation      Handle SHA1 operation         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            updated constants,          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_sha1_digest_calculate(NX_CRYPTO_SHA1 *context, UCHAR digest[20], UINT algorithm)
{

UCHAR bit_count_string[8];
ULONG current_byte_count;
ULONG padding_bytes;

    NX_CRYPTO_PARAMETER_NOT_USED(algorithm);

    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] =  (UCHAR)(context -> nx_sha1_bit_count[1] >> 24);
    bit_count_string[1] =  (UCHAR)(context -> nx_sha1_bit_count[1] >> 16);
    bit_count_string[2] =  (UCHAR)(context -> nx_sha1_bit_count[1] >> 8);
    bit_count_string[3] =  (UCHAR)(context -> nx_sha1_bit_count[1]);
    bit_count_string[4] =  (UCHAR)(context -> nx_sha1_bit_count[0] >> 24);
    bit_count_string[5] =  (UCHAR)(context -> nx_sha1_bit_count[0] >> 16);
    bit_count_string[6] =  (UCHAR)(context -> nx_sha1_bit_count[0] >> 8);
    bit_count_string[7] =  (UCHAR)(context -> nx_sha1_bit_count[0]);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_sha1_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_sha1_update(context, (UCHAR*)_nx_crypto_sha1_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_sha1_update(context, bit_count_string, 8);

    /* Now store the digest in the caller specified destination.  */
    digest[0] =  (UCHAR)(context -> nx_sha1_states[0] >> 24);
    digest[1] =  (UCHAR)(context -> nx_sha1_states[0] >> 16);
    digest[2] =  (UCHAR)(context -> nx_sha1_states[0] >> 8);
    digest[3] =  (UCHAR)(context -> nx_sha1_states[0]);
    digest[4] =  (UCHAR)(context -> nx_sha1_states[1] >> 24);
    digest[5] =  (UCHAR)(context -> nx_sha1_states[1] >> 16);
    digest[6] =  (UCHAR)(context -> nx_sha1_states[1] >> 8);
    digest[7] =  (UCHAR)(context -> nx_sha1_states[1]);
    digest[8] =  (UCHAR)(context -> nx_sha1_states[2] >> 24);
    digest[9] =  (UCHAR)(context -> nx_sha1_states[2] >> 16);
    digest[10] =  (UCHAR)(context -> nx_sha1_states[2] >> 8);
    digest[11] =  (UCHAR)(context -> nx_sha1_states[2]);
    digest[12] =  (UCHAR)(context -> nx_sha1_states[3] >> 24);
    digest[13] =  (UCHAR)(context -> nx_sha1_states[3] >> 16);
    digest[14] =  (UCHAR)(context -> nx_sha1_states[3] >> 8);
    digest[15] =  (UCHAR)(context -> nx_sha1_states[3]);
    digest[16] =  (UCHAR)(context -> nx_sha1_states[4] >> 24);
    digest[17] =  (UCHAR)(context -> nx_sha1_states[4] >> 16);
    digest[18] =  (UCHAR)(context -> nx_sha1_states[4] >> 8);
    digest[19] =  (UCHAR)(context -> nx_sha1_states[4]);

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
/*    _nx_crypto_sha1_process_buffer                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function actually uses the SHA1 algorithm to process a 64-byte */
/*    (512 bit) buffer.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA1 context pointer          */
/*    buffer                                Pointer to 64-byte buffer     */
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
/*    _nx_crypto_sha1_update                Update the digest with padding*/
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
NX_CRYPTO_KEEP VOID  _nx_crypto_sha1_process_buffer(NX_CRYPTO_SHA1 *context, UCHAR buffer[64])
{

ULONG *w;
UINT   t;
ULONG  temp;
ULONG  a, b, c, d, e;


    /* Setup pointers to the word array.  */
    w =  context -> nx_sha1_word_array;

    /* Initialize the first 16 words of the word array, taking care of the
       endian issues at the same time.  */
    for (t = 0; t < 16; t++)
    {

        /* Setup each entry.  */
        w[t] =  (((ULONG)buffer[t * 4]) << 24) | (((ULONG)buffer[(t * 4) + 1]) << 16) | (((ULONG)buffer[(t * 4) + 2]) << 8) | ((ULONG)buffer[(t * 4) + 3]);
    }

    /* Setup the remaining entries of the word array.  */
    for (t = 16; t < 80; t++)
    {

        /* Setup each entry.  */
        w[t] =  LEFT_SHIFT_CIRCULAR((w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16]), 1);
    }

    /* Initialize the state variables.  */
    a =  context -> nx_sha1_states[0];
    b =  context -> nx_sha1_states[1];
    c =  context -> nx_sha1_states[2];
    d =  context -> nx_sha1_states[3];
    e =  context -> nx_sha1_states[4];

    /* Now, perform Round 1 operations.  */
    for (t = 0; t < 20; t++)
    {

        /* Compute round 1 (t = 0 through t = 19).  */
        temp =  LEFT_SHIFT_CIRCULAR(a, 5) + F1(b, c, d) + e + w[t] + 0x5A827999UL;
        e =  d;
        d =  c;
        c =  LEFT_SHIFT_CIRCULAR(b, 30);
        b =  a;
        a =  temp;
    }

    /* Now, perform Round 2 operations.  */
    for (t = 20; t < 40; t++)
    {

        /* Compute round 2 (t = 20 through t = 39).  */
        temp =  LEFT_SHIFT_CIRCULAR(a, 5) + F2(b, c, d) + e + w[t] + 0x6ED9EBA1UL;
        e =  d;
        d =  c;
        c =  LEFT_SHIFT_CIRCULAR(b, 30);
        b =  a;
        a =  temp;
    }

    /* Now, perform Round 3 operations.  */
    for (t = 40; t < 60; t++)
    {

        /* Compute round 3 (t = 40 through t = 59).  */
        temp =  LEFT_SHIFT_CIRCULAR(a, 5) + F3(b, c, d) + e + w[t] + 0x8F1BBCDCUL;
        e =  d;
        d =  c;
        c =  LEFT_SHIFT_CIRCULAR(b, 30);
        b =  a;
        a =  temp;
    }

    /* Finally, perform Round 4 operations.  */
    for (t = 60; t < 80; t++)
    {

        /* Compute round 4 (t = 60 through t = 79).  */
        temp =  LEFT_SHIFT_CIRCULAR(a, 5) + F4(b, c, d) + e + w[t] + 0xCA62C1D6UL;
        e =  d;
        d =  c;
        c =  LEFT_SHIFT_CIRCULAR(b, 30);
        b =  a;
        a =  temp;
    }

    /* Save the resulting in this SHA1 context.  */
    context -> nx_sha1_states[0] +=  a;
    context -> nx_sha1_states[1] +=  b;
    context -> nx_sha1_states[2] +=  c;
    context -> nx_sha1_states[3] +=  d;
    context -> nx_sha1_states[4] +=  e;

#ifdef NX_SECURE_KEY_CLEAR
    a = 0; b = 0; c = 0; d = 0; e = 0;
    temp = 0;
#endif /* NX_SECURE_KEY_CLEAR  */
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha1_init                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported SHA1 cryptographic algorithm.                   */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha1_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                 VOID  **handle,
                                                 VOID  *crypto_metadata,
                                                 ULONG crypto_metadata_size)
{

    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if (method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if (crypto_metadata == NX_CRYPTO_NULL)
    {

        /* metadata is not passed by IPsec. */
#ifndef NX_IPSEC_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#endif
    }
    else if (((((ULONG)crypto_metadata) & 0x3) != 0) || (crypto_metadata_size < sizeof(NX_CRYPTO_SHA1)))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha1_cleanup                      PORTABLE C      */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha1_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_SHA1));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha1_operation                   PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the SHA1 algorithm.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    SHA1 operation                */
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
/*    _nx_crypto_sha1_initialize            Initialize the SHA1 context   */
/*    _nx_crypto_sha1_update                Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_sha1_digest_calculate      Calculate the SHA1 digest     */
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
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha1_operation(UINT op,      /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_SHA1 *ctx = (NX_CRYPTO_SHA1 *)crypto_metadata;
#ifdef NX_IPSEC_ENABLE
NX_CRYPTO_SHA1  metadata;
#endif

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    if (method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if (crypto_metadata == NX_CRYPTO_NULL)
    {
#ifdef NX_IPSEC_ENABLE
        /* metadata is not passed by IPsec. */
        ctx = &metadata;
#else
        return(NX_CRYPTO_PTR_ERROR);
#endif
    }
    else if (((((ULONG)crypto_metadata) & 0x3) != 0) || (crypto_metadata_size < sizeof(NX_CRYPTO_SHA1)))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    switch (op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        _nx_crypto_sha1_initialize(ctx, method -> nx_crypto_algorithm);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_sha1_update(ctx, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        if(output_length_in_byte < 20)
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        _nx_crypto_sha1_digest_calculate(ctx, output, method -> nx_crypto_algorithm);
        break;

    default:
        if(output_length_in_byte < 20)
            return(NX_CRYPTO_INVALID_BUFFER_SIZE);
        _nx_crypto_sha1_initialize(ctx, method -> nx_crypto_algorithm);
        _nx_crypto_sha1_update(ctx, input, input_length_in_byte);
        _nx_crypto_sha1_digest_calculate(ctx, output, method -> nx_crypto_algorithm);
#if defined(NX_SECURE_KEY_CLEAR) && defined(NX_IPSEC_ENABLE)
        if (crypto_metadata == NX_CRYPTO_NULL)
        {
            memset(ctx, 0, sizeof(NX_CRYPTO_SHA1));
        }
#endif
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

