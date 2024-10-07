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
/**   Huge Number                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_huge_number.h                             PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto huge number module.                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed number initialization,*/
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), and      */
/*                                            used ULONG64_DEFINED macro, */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_HUGE_NUMBER_H
#define NX_CRYPTO_HUGE_NUMBER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_crypto.h"

/* Return values for _nx_crypto_huge_number_compare */
#define NX_CRYPTO_HUGE_NUMBER_EQUAL   (0x0)
#define NX_CRYPTO_HUGE_NUMBER_LESS    (0x1)
#define NX_CRYPTO_HUGE_NUMBER_GREATER (0x2)
#define NX_CRYPTO_HUGE_NUMBER_ERROR   (0x3)


/* Define the base exponent of 2 for huge number.
 * Only 16 and 32 are supported. */
#ifndef NX_CRYPTO_HUGE_NUMBER_BITS
#define NX_CRYPTO_HUGE_NUMBER_BITS    32
#endif /* NX_CRYPTO_HUGE_NUMBER_BITS */

#if (NX_CRYPTO_HUGE_NUMBER_BITS == 32)
#ifndef ULONG64_DEFINED
#define ULONG64_DEFINED
#define ULONG64                       unsigned long long
#define LONG64                        long long
#endif
#define HN_BASE                       LONG
#define HN_BASE2                      LONG64
#define HN_UBASE                      ULONG
#define HN_UBASE2                     ULONG64
#define HN_MASK                       0xFFFFFFFF
#define HN_RADIX                      0x100000000
#define HN_SHIFT                      (sizeof(HN_BASE) << 3)
#define HN_SIZE_ROUND                 (sizeof(HN_BASE) - 1)
#define HN_SIZE_SHIFT                 2
#define HN_ULONG_TO_UBASE(v)          v
#elif (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
#define HN_BASE                       SHORT
#define HN_BASE2                      LONG
#define HN_UBASE                      USHORT
#define HN_UBASE2                     ULONG
#define HN_MASK                       0xFFFF
#define HN_RADIX                      0x10000
#define HN_SHIFT                      (sizeof(HN_BASE) << 3)
#define HN_SIZE_ROUND                 (sizeof(HN_BASE) - 1)
#define HN_SIZE_SHIFT                 1
#define HN_ULONG_TO_UBASE(v)          (v) & HN_MASK, (v) >> HN_SHIFT
#else
#error "NX_CRYPTO_HUGE_NUMBER_BITS supports 16 and 32 only!"
#endif


/* Huge number structure - contains data pointer and size. */
typedef struct NX_CRYPTO_HUGE_NUMBER_STRUCT
{
    /* Stores a pointer to the number data which may be of arbitrary size. */
    HN_UBASE *nx_crypto_huge_number_data;

    /* The size of the data stored in the buffer, in number of digit. */
    UINT nx_crypto_huge_number_size;

    /* The size of the buffer itself, in number of bytes. */
    UINT nx_crypto_huge_buffer_size;

    /* Flag to indicate positive or negative value. */
    UINT nx_crypto_huge_number_is_negative;
} NX_CRYPTO_HUGE_NUMBER;


/* Misc macros for huge number. */

/* Initialize the buffer of huge number. */
#define NX_CRYPTO_HUGE_NUMBER_INITIALIZE(hn, buff, size)     \
    (hn) -> nx_crypto_huge_number_data = (HN_UBASE *)(buff); \
    (hn) -> nx_crypto_huge_number_size = 0;                  \
    (hn) -> nx_crypto_huge_buffer_size = (((size) + HN_SIZE_ROUND) >> HN_SIZE_SHIFT) << HN_SIZE_SHIFT;\
    (hn) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;    \
    (buff) = (buff) + (((size) + HN_SIZE_ROUND) >> HN_SIZE_SHIFT);

/* Is it an even huge number? */
#define NX_CRYPTO_HUGE_NUMBER_IS_EVEN(hn) \
    !((hn) -> nx_crypto_huge_number_data[0] & 1)

/* Set value of huge number between 0 and (HN_RADIX - 1). */
#define NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(hn, val)          \
    (hn) -> nx_crypto_huge_number_data[0] = (val);        \
    (hn) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE; \
    (hn) -> nx_crypto_huge_number_size = 1;

/* Initialize the buffer of huge number to variable between 0 and (HN_RADIX - 1). */
#define NX_CRYPTO_HUGE_NUMBER_INITIALIZE_DIGIT(hn, buff, val) \
    (hn) -> nx_crypto_huge_number_data = (HN_UBASE *)(buff);  \
    (hn) -> nx_crypto_huge_buffer_size = sizeof(HN_UBASE);        \
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(hn, val)

/* Copy huge number from src to dst. */
#define NX_CRYPTO_HUGE_NUMBER_COPY(dst, src)                                                 \
    (dst) -> nx_crypto_huge_number_size = (src) -> nx_crypto_huge_number_size;               \
    (dst) -> nx_crypto_huge_number_is_negative = (src) -> nx_crypto_huge_number_is_negative; \
    NX_CRYPTO_MEMCPY((dst) -> nx_crypto_huge_number_data,                                              \
           (src) -> nx_crypto_huge_number_data,                                              \
           (src) -> nx_crypto_huge_number_size << HN_SIZE_SHIFT);


/* Function prototypes */

VOID _nx_crypto_huge_number_adjust_size(NX_CRYPTO_HUGE_NUMBER *val);
UINT _nx_crypto_huge_number_setup(NX_CRYPTO_HUGE_NUMBER *number, const UCHAR *byte_stream, UINT size);
UINT _nx_crypto_huge_number_extract(NX_CRYPTO_HUGE_NUMBER *number, UCHAR *byte_stream,
                                    UINT byte_stream_size, UINT *huge_number_size);
UINT _nx_crypto_huge_number_extract_fixed_size(NX_CRYPTO_HUGE_NUMBER *number,
                                               UCHAR *byte_stream, UINT byte_stream_size);
UINT _nx_crypto_huge_number_rbg(UINT bits, UCHAR *result);
UINT _nx_crypto_huge_number_compare(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right);
UINT _nx_crypto_huge_number_compare_unsigned(NX_CRYPTO_HUGE_NUMBER *left,
                                             NX_CRYPTO_HUGE_NUMBER *right);
UINT _nx_crypto_huge_number_is_zero(NX_CRYPTO_HUGE_NUMBER *x);
VOID _nx_crypto_huge_number_add(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right);
VOID _nx_crypto_huge_number_add_unsigned(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right);
VOID _nx_crypto_huge_number_subtract(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right);
VOID _nx_crypto_huge_number_subtract_unsigned(NX_CRYPTO_HUGE_NUMBER *left,
                                              NX_CRYPTO_HUGE_NUMBER *right,
                                              NX_CRYPTO_HUGE_NUMBER *result);
VOID _nx_crypto_huge_number_add_digit(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit);
VOID _nx_crypto_huge_number_subtract_digit(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit);
VOID _nx_crypto_huge_number_add_digit_unsigned(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit);
VOID _nx_crypto_huge_number_subtract_digit_unsigned(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit);
VOID _nx_crypto_huge_number_multiply(NX_CRYPTO_HUGE_NUMBER *left,
                                     NX_CRYPTO_HUGE_NUMBER *right,
                                     NX_CRYPTO_HUGE_NUMBER *result);
VOID _nx_crypto_huge_number_multiply_digit(NX_CRYPTO_HUGE_NUMBER *value,
                                           HN_UBASE digit,
                                           NX_CRYPTO_HUGE_NUMBER *result);
VOID _nx_crypto_huge_number_square(NX_CRYPTO_HUGE_NUMBER *value, NX_CRYPTO_HUGE_NUMBER *result);
VOID _nx_crypto_huge_number_modulus(NX_CRYPTO_HUGE_NUMBER *dividend, NX_CRYPTO_HUGE_NUMBER *divisor);
VOID _nx_crypto_huge_number_shift_left(NX_CRYPTO_HUGE_NUMBER *x, UINT shift);
VOID _nx_crypto_huge_number_shift_right(NX_CRYPTO_HUGE_NUMBER *x, UINT shift);
UINT _nx_crypto_huge_number_inverse_modulus_prime(NX_CRYPTO_HUGE_NUMBER *a,
                                                  NX_CRYPTO_HUGE_NUMBER *p,
                                                  NX_CRYPTO_HUGE_NUMBER *r,
                                                  HN_UBASE *scratch);
UINT _nx_crypto_huge_number_inverse_modulus(NX_CRYPTO_HUGE_NUMBER *a,
                                            NX_CRYPTO_HUGE_NUMBER *m,
                                            NX_CRYPTO_HUGE_NUMBER *r,
                                            HN_UBASE *scratch);
VOID _nx_crypto_huge_number_mont(NX_CRYPTO_HUGE_NUMBER *m, UINT mi,
                                 NX_CRYPTO_HUGE_NUMBER *x,
                                 NX_CRYPTO_HUGE_NUMBER *y,
                                 NX_CRYPTO_HUGE_NUMBER *result);
VOID _nx_crypto_huge_number_power_modulus(NX_CRYPTO_HUGE_NUMBER *number,
                                          NX_CRYPTO_HUGE_NUMBER *exponent,
                                          NX_CRYPTO_HUGE_NUMBER *modulus,
                                          NX_CRYPTO_HUGE_NUMBER *result,
                                          HN_UBASE *scratch);
VOID _nx_crypto_huge_number_mont_power_modulus(NX_CRYPTO_HUGE_NUMBER *x,
                                               NX_CRYPTO_HUGE_NUMBER *e,
                                               NX_CRYPTO_HUGE_NUMBER *m,
                                               NX_CRYPTO_HUGE_NUMBER *result,
                                               HN_UBASE *scratch);
VOID _nx_crypto_huge_number_crt_power_modulus(NX_CRYPTO_HUGE_NUMBER *x,
                                              NX_CRYPTO_HUGE_NUMBER *e,
                                              NX_CRYPTO_HUGE_NUMBER *p,
                                              NX_CRYPTO_HUGE_NUMBER *q,
                                              NX_CRYPTO_HUGE_NUMBER *m,
                                              NX_CRYPTO_HUGE_NUMBER *result,
                                              HN_UBASE *scratch);


#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_HUGE_NUMBER_H */

