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
  Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
  rights reserved.

  License to copy and use this software is granted provided that it
  is identified as the ""RSA Data Security, Inc. MD5 Message-Digest
  Algorithm"" in all material mentioning or referencing this software
  or this function.

  License is also granted to make and use derivative works provided
  that such works are identified as ""derived from the RSA Data
  Security, Inc. MD5 Message-Digest Algorithm"" in all material
  mentioning or referencing the derived work.

  RSA Data Security, Inc. makes no representations concerning either
  the merchantability of this software or the suitability of this
  software for any particular purpose. It is provided ""as is""
  without express or implied warranty of any kind.

  These notices must be retained in any copies of any part of this
  documentation and/or software.
*/
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** NetX Component                                                        */
/**                                                                       */
/**   MD5 Digest Algorithm (MD5)                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_api.h"
#include "nx_md5.h"

/* Define macros for the MD5 transform function.  */

/* Define the MD5 basic F, G, H and I functions.  */

#define F(x, y, z)                      (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z)                      (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z)                      ((x) ^ (y) ^ (z))
#define I(x, y, z)                      ((y) ^ ((x) | (~z)))


/* Define the MD5 left shift circular function.  */

#define LEFT_SHIFT_CIRCULAR(x, n)       (((x) << (n)) | ((x) >> (32-(n))))


/* Define the MD5 complex FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.  */

#define FF(a, b, c, d, x, s, ac)        { \
                                            (a) += F ((b), (c), (d)) + (x) + (ULONG)(ac); \
                                            (a) = LEFT_SHIFT_CIRCULAR ((a), (s)); \
                                            (a) += (b); \
                                        }
#define GG(a, b, c, d, x, s, ac)        { \
                                            (a) += G ((b), (c), (d)) + (x) + (ULONG)(ac); \
                                            (a) = LEFT_SHIFT_CIRCULAR ((a), (s)); \
                                            (a) += (b); \
                                        }
#define HH(a, b, c, d, x, s, ac)        { \
                                            (a) += H ((b), (c), (d)) + (x) + (ULONG)(ac); \
                                            (a) = LEFT_SHIFT_CIRCULAR ((a), (s)); \
                                            (a) += (b); \
                                        }
#define II(a, b, c, d, x, s, ac)        { \
                                            (a) += I ((b), (c), (d)) + (x) + (ULONG)(ac); \
                                            (a) = LEFT_SHIFT_CIRCULAR ((a), (s)); \
                                            (a) += (b); \
                                        }


/* Define the padding array.  This is used to pad the message such that its length is 
   64 bits shy of being a multiple of 512 bits long.  */

UCHAR   _nx_md5_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_md5_initialize                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the MD5 context. It must be called prior  */ 
/*    to creating the MD5 digest.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               MD5 context pointer           */ 
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
/*    NetX Applications                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_md5_initialize(NX_MD5 *context)
{

    /* Determine if the context is non-null.  */
    if (context == NX_NULL)
        return(NX_PTR_ERROR);

    /* First, clear the bit count for this context.  */
    context -> nx_md5_bit_count[0] =  0;                    /* Clear the lower 32-bits of the count */
    context -> nx_md5_bit_count[1] =  0;                    /* Clear the upper 32-bits of the count */ 

    /* Finally, setup the context states.  */
    context -> nx_md5_states[0] =  0x67452301UL;            /* Setup state A                        */ 
    context -> nx_md5_states[1] =  0xEFCDAB89UL;            /* Setup state B                        */ 
    context -> nx_md5_states[2] =  0x98BADCFEUL;            /* Setup state C                        */ 
    context -> nx_md5_states[3] =  0x10325476UL;            /* Setup state D                        */ 

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_md5_update                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the digest calculation with new input from    */ 
/*    the caller.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               MD5 context pointer           */ 
/*    input_ptr                             Pointer to byte(s) of input   */ 
/*    input_length                          Length of bytes of input      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_md5_process_buffer                Process complete buffer,      */ 
/*                                            which is 64-bytes in size   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_md5_update(NX_MD5 *context, UCHAR *input_ptr, UINT input_length)
{

ULONG   current_bytes;
ULONG   needed_fill_bytes;


    /* Determine if the context is non-null.  */
    if (context == NX_NULL)
        return(NX_PTR_ERROR);

    /* Determine if there is a length.  */
    if (input_length == 0)
        return(NX_SUCCESS);

    /* Calculate the current byte count mod 64. Note the reason for the 
       shift by 3 is to account for the 8 bits per byte.  */
    current_bytes =  (context -> nx_md5_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  64 - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_md5_bit_count[0] += (input_length << 3);
    
    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_md5_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_md5_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_md5_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {

        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal 
           buffer of the context.  */
        memcpy((void *) &(context -> nx_md5_buffer[current_bytes]), (void *) input_ptr, needed_fill_bytes); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */

        /* Process the 64-byte (512 bit) buffer.  */
        _nx_md5_process_buffer(context, context -> nx_md5_buffer);

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
        _nx_md5_process_buffer(context, input_ptr);

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - 64;
        input_ptr =     input_ptr + 64;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {

        /* Save the remaining bytes in the internal buffer after any remaining bytes
           that it is processed later.  */
        memcpy((void *) &(context -> nx_md5_buffer[current_bytes]), (void *) input_ptr, input_length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
    }

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_md5_digest_calculate                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finishes calculation of the MD5 digest. It is called  */ 
/*    where there is no further input needed for the digest. The resulting*/ 
/*    16-byte (128-bit) MD5 digest is returned to the caller.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               MD5 context pointer           */ 
/*    digest                                16-byte (128-bit) digest      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_md5_update                        Update the digest with padding*/ 
/*                                            and length of digest        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX Applications                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_md5_digest_calculate(NX_MD5 *context, UCHAR digest[16])
{

UCHAR   bit_count_string[8];
ULONG   current_byte_count;
ULONG   padding_bytes;


    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] =  (UCHAR) context -> nx_md5_bit_count[0];
    bit_count_string[1] =  (UCHAR) (context -> nx_md5_bit_count[0] >> 8);
    bit_count_string[2] =  (UCHAR) (context -> nx_md5_bit_count[0] >> 16);
    bit_count_string[3] =  (UCHAR) (context -> nx_md5_bit_count[0] >> 24);
    bit_count_string[4] =  (UCHAR) context -> nx_md5_bit_count[1];
    bit_count_string[5] =  (UCHAR) (context -> nx_md5_bit_count[1] >> 8);
    bit_count_string[6] =  (UCHAR) (context -> nx_md5_bit_count[1] >> 16);
    bit_count_string[7] =  (UCHAR) (context -> nx_md5_bit_count[1] >> 24);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_md5_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_md5_update(context, _nx_md5_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_md5_update(context, bit_count_string, 8);

    /* Now store the digest in the caller specified destination.  */
    digest[ 0] =  (UCHAR) context -> nx_md5_states[0];
    digest[ 1] =  (UCHAR) (context -> nx_md5_states[0] >> 8);
    digest[ 2] =  (UCHAR) (context -> nx_md5_states[0] >> 16);
    digest[ 3] =  (UCHAR) (context -> nx_md5_states[0] >> 24);
    digest[ 4] =  (UCHAR) context -> nx_md5_states[1];
    digest[ 5] =  (UCHAR) (context -> nx_md5_states[1] >> 8);
    digest[ 6] =  (UCHAR) (context -> nx_md5_states[1] >> 16);
    digest[ 7] =  (UCHAR) (context -> nx_md5_states[1] >> 24);
    digest[ 8] =  (UCHAR) context -> nx_md5_states[2];
    digest[ 9] =  (UCHAR) (context -> nx_md5_states[2] >> 8);
    digest[10] =  (UCHAR) (context -> nx_md5_states[2] >> 16);
    digest[11] =  (UCHAR) (context -> nx_md5_states[2] >> 24);
    digest[12] =  (UCHAR) context -> nx_md5_states[3];
    digest[13] =  (UCHAR) (context -> nx_md5_states[3] >> 8);
    digest[14] =  (UCHAR) (context -> nx_md5_states[3] >> 16);
    digest[15] =  (UCHAR) (context -> nx_md5_states[3] >> 24);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_md5_process_buffer                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function actually uses the MD5 algorithm to process a 64-byte  */ 
/*    (512 bit) buffer.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               MD5 context pointer           */ 
/*    buffer                                64-byte buffer                */ 
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
/*    NetX Applications                                                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_md5_process_buffer(NX_MD5 *context, UCHAR buffer[64])
{

UINT    i, j;
ULONG   a, b, c, d;
ULONG   x[16];


    /* Initialize the state variables.  */
    a =  context -> nx_md5_states[0];
    b =  context -> nx_md5_states[1];
    c =  context -> nx_md5_states[2];
    d =  context -> nx_md5_states[3];

    /* Now, setup the x array of ULONGs for fast processing.  */
    j =  0;
    for (i = 0; i < 16; i++)
    {

        /* Convert 4 bytes into one 32-bit word.  */
        x[i] =  ((ULONG) buffer[j]) | (((ULONG) buffer[j+1]) << 8) | (((ULONG) buffer[j+2]) << 16) | (((ULONG) buffer[j+3]) << 24);

        /* Move to next position in source array.  */
        j =  j + 4;
    }

    /* Process Round 1 of MD5 calculation.  */
    FF(a, b, c, d, x[ 0], 7,  0xd76aa478UL);
    FF(d, a, b, c, x[ 1], 12, 0xe8c7b756UL);
    FF(c, d, a, b, x[ 2], 17, 0x242070dbUL);
    FF(b, c, d, a, x[ 3], 22, 0xc1bdceeeUL);
    FF(a, b, c, d, x[ 4], 7,  0xf57c0fafUL);
    FF(d, a, b, c, x[ 5], 12, 0x4787c62aUL);
    FF(c, d, a, b, x[ 6], 17, 0xa8304613UL);
    FF(b, c, d, a, x[ 7], 22, 0xfd469501UL);
    FF(a, b, c, d, x[ 8], 7,  0x698098d8UL);
    FF(d, a, b, c, x[ 9], 12, 0x8b44f7afUL);
    FF(c, d, a, b, x[10], 17, 0xffff5bb1UL);
    FF(b, c, d, a, x[11], 22, 0x895cd7beUL);
    FF(a, b, c, d, x[12], 7,  0x6b901122UL);
    FF(d, a, b, c, x[13], 12, 0xfd987193UL);
    FF(c, d, a, b, x[14], 17, 0xa679438eUL);
    FF(b, c, d, a, x[15], 22, 0x49b40821UL);

    /* Process Round 2 of MD5 calculation.  */
    GG(a, b, c, d, x[ 1], 5,  0xf61e2562UL);
    GG(d, a, b, c, x[ 6], 9,  0xc040b340UL);
    GG(c, d, a, b, x[11], 14, 0x265e5a51UL);
    GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aaUL);
    GG(a, b, c, d, x[ 5], 5,  0xd62f105dUL);
    GG(d, a, b, c, x[10], 9,  0x02441453UL);
    GG(c, d, a, b, x[15], 14, 0xd8a1e681UL);
    GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8UL);
    GG(a, b, c, d, x[ 9], 5,  0x21e1cde6UL);
    GG(d, a, b, c, x[14], 9,  0xc33707d6UL);
    GG(c, d, a, b, x[ 3], 14, 0xf4d50d87UL);
    GG(b, c, d, a, x[ 8], 20, 0x455a14edUL);
    GG(a, b, c, d, x[13], 5,  0xa9e3e905UL);
    GG(d, a, b, c, x[ 2], 9,  0xfcefa3f8UL);
    GG(c, d, a, b, x[ 7], 14, 0x676f02d9UL);
    GG(b, c, d, a, x[12], 20, 0x8d2a4c8aUL);

    /* Process Round 3 of MD5 calculation.  */
    HH(a, b, c, d, x[ 5], 4,  0xfffa3942UL);
    HH(d, a, b, c, x[ 8], 11, 0x8771f681UL);
    HH(c, d, a, b, x[11], 16, 0x6d9d6122UL);
    HH(b, c, d, a, x[14], 23, 0xfde5380cUL);
    HH(a, b, c, d, x[ 1], 4,  0xa4beea44UL);
    HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9UL);
    HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60UL);
    HH(b, c, d, a, x[10], 23, 0xbebfbc70UL);
    HH(a, b, c, d, x[13], 4,  0x289b7ec6UL);
    HH(d, a, b, c, x[ 0], 11, 0xeaa127faUL);
    HH(c, d, a, b, x[ 3], 16, 0xd4ef3085UL);
    HH(b, c, d, a, x[ 6], 23, 0x04881d05UL);
    HH(a, b, c, d, x[ 9], 4,  0xd9d4d039UL);
    HH(d, a, b, c, x[12], 11, 0xe6db99e5UL);
    HH(c, d, a, b, x[15], 16, 0x1fa27cf8UL);
    HH(b, c, d, a, x[ 2], 23, 0xc4ac5665UL);

    /* Process Round 4 of MD5 calculation.  */
    II(a, b, c, d, x[ 0], 6,  0xf4292244UL);
    II(d, a, b, c, x[ 7], 10, 0x432aff97UL);
    II(c, d, a, b, x[14], 15, 0xab9423a7UL);
    II(b, c, d, a, x[ 5], 21, 0xfc93a039UL);
    II(a, b, c, d, x[12], 6,  0x655b59c3UL);
    II(d, a, b, c, x[ 3], 10, 0x8f0ccc92UL);
    II(c, d, a, b, x[10], 15, 0xffeff47dUL);
    II(b, c, d, a, x[ 1], 21, 0x85845dd1UL);
    II(a, b, c, d, x[ 8], 6,  0x6fa87e4fUL);
    II(d, a, b, c, x[15], 10, 0xfe2ce6e0UL);
    II(c, d, a, b, x[ 6], 15, 0xa3014314UL);
    II(b, c, d, a, x[13], 21, 0x4e0811a1UL);
    II(a, b, c, d, x[ 4], 6,  0xf7537e82UL);
    II(d, a, b, c, x[11], 10, 0xbd3af235UL);
    II(c, d, a, b, x[ 2], 15, 0x2ad7d2bbUL);
    II(b, c, d, a, x[ 9], 21, 0xeb86d391UL);

    /* Save the resulting in this MD5 context.  */
    context -> nx_md5_states[0] +=  a;
    context -> nx_md5_states[1] +=  b;
    context -> nx_md5_states[2] +=  c;
    context -> nx_md5_states[3] +=  d;
}

