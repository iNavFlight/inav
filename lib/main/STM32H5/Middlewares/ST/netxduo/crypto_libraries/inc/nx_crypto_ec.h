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
/**   Elliptic Curve                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ec.h                                      PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the symbols, structures and operations for        */
/*    Elliptic Curve Crypto.                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            added public key validation,*/
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            added x25519 curve,         */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            added x448 curve,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_EC_H
#define NX_CRYPTO_EC_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_huge_number.h"

#define NX_CRYPTO_EC_POINT_AFFINE     0
#define NX_CRYPTO_EC_POINT_PROJECTIVE 1

#define NX_CRYPTO_EC_FP               0
#define NX_CRYPTO_EC_F2M              1

/* Define Elliptic Curve point. */
typedef struct
{
    UINT                  nx_crypto_ec_point_type;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_x;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_y;
    NX_CRYPTO_HUGE_NUMBER nx_crypto_ec_point_z;
} NX_CRYPTO_EC_POINT;

typedef struct
{
    USHORT *nx_crypto_ec_polynomial_data;
    UINT    nx_crypto_ec_polynomial_size;
} NX_CRYPTO_EC_POLYNOMIAL;

/* Define fixed points */
typedef struct
{

    /* Window width */
    UINT nx_crypto_ec_fixed_points_window_width;

    /* Bits of curve (m). */
    UINT nx_crypto_ec_fixed_points_bits;

    /* d = (m + w - 1) / w */
    UINT nx_crypto_ec_fixed_points_d;

    /* e = (d + 1) / 2 */
    UINT nx_crypto_ec_fixed_points_e;

    /* [a(w-1),...a(0)]G */
    /* 0G and 1G are not stored. */
    /* The count of fixed points are 2 ^ w - 2. */
    NX_CRYPTO_EC_POINT *nx_crypto_ec_fixed_points_array;

    /* 2^e[a(w-1),...a(0)]G */
    /* 0G is not stored. */
    /* The count of fixed points are 2 ^ w - 1. */
    NX_CRYPTO_EC_POINT *nx_crypto_ec_fixed_points_array_2e;
} NX_CRYPTO_EC_FIXED_POINTS;

/* Define Elliptic Curve. */
typedef struct NX_CRYPTO_EC_STRUCT
{
    CHAR *nx_crypto_ec_name;
    UINT  nx_crypto_ec_id;
    UINT  nx_crypto_ec_window_width;
    UINT  nx_crypto_ec_bits;
    union
    {
        NX_CRYPTO_HUGE_NUMBER   fp;
        NX_CRYPTO_EC_POLYNOMIAL f2m;
    }                          nx_crypto_ec_field;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_a;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_b;
    NX_CRYPTO_EC_POINT         nx_crypto_ec_g;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_n;
    NX_CRYPTO_HUGE_NUMBER      nx_crypto_ec_h;
    NX_CRYPTO_EC_FIXED_POINTS *nx_crypto_ec_fixed_points;
    VOID (*nx_crypto_ec_add)(struct NX_CRYPTO_EC_STRUCT *curve,
                             NX_CRYPTO_EC_POINT *left,
                             NX_CRYPTO_EC_POINT *right,
                             HN_UBASE *scratch);
    VOID (*nx_crypto_ec_subtract)(struct NX_CRYPTO_EC_STRUCT *curve,
                                  NX_CRYPTO_EC_POINT *left,
                                  NX_CRYPTO_EC_POINT *right,
                                  HN_UBASE *scratch);
    VOID (*nx_crypto_ec_multiple)(struct NX_CRYPTO_EC_STRUCT *curve,
                                  NX_CRYPTO_EC_POINT *g,
                                  NX_CRYPTO_HUGE_NUMBER *d,
                                  NX_CRYPTO_EC_POINT *r,
                                  HN_UBASE *scratch);
    VOID (*nx_crypto_ec_reduce)(struct NX_CRYPTO_EC_STRUCT *curve,
                                NX_CRYPTO_HUGE_NUMBER *value,
                                HN_UBASE *scratch);
} NX_CRYPTO_EC;

#define NX_CRYPTO_EC_POINT_INITIALIZE(p, type, buff, size)                              \
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_x), buff, size);       \
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_y), buff, size);       \
    if ((type) == NX_CRYPTO_EC_POINT_PROJECTIVE) {                                      \
        NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&((p) -> nx_crypto_ec_point_z), buff, size); } \
    (p) -> nx_crypto_ec_point_type = (type);

#define NX_CRYPTO_EC_POINT_SETUP(p, x, x_size, y, y_size, z, z_size)         \
    _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_x), x, x_size); \
    _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_y), y, y_size); \
    if ((p) -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_PROJECTIVE) {   \
        _nx_crypto_huge_number_setup(&((p) -> nx_crypto_ec_point_z), z, z_size); }

#define NX_CRYPTO_EC_POINT_EXTRACT(p, x, x_size, x_out_size, y, y_size, y_out_size, z, z_size, z_out_size) \
    _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_x), x, x_size, x_out_size);                 \
    _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_y), y, y_size, y_out_size);                 \
    if ((p) -> nx_crypto_ec_point_type == NX_CRYPTO_EC_POINT_PROJECTIVE) {                                 \
        _nx_crypto_huge_number_extract(&((p) -> nx_crypto_ec_point_z), z, z_size, z_out_size); }

#define NX_CRYPTO_EC_MULTIPLE_DIGIT_REDUCE(curve, value, digit, result, scratch) \
    _nx_crypto_huge_number_multiply_digit(value, digit, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_MULTIPLE_REDUCE(curve, left, right, result, scratch) \
    _nx_crypto_huge_number_multiply(left, right, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_SQUARE_REDUCE(curve, value, result, scratch) \
    _nx_crypto_huge_number_square(value, result);                 \
    curve -> nx_crypto_ec_reduce(curve, result, scratch);

#define NX_CRYPTO_EC_SHIFT_LEFT_REDUCE(curve, value, shift, scratch) \
    _nx_crypto_huge_number_shift_left(value, shift);                 \
    curve -> nx_crypto_ec_reduce(curve, value, scratch);

#if (NX_CRYPTO_HUGE_NUMBER_BITS == 32)

#define NX_CRYPTO_EC_ASSIGN_REV(a, b) (a) = (b); NX_CRYPTO_CHANGE_ULONG_ENDIAN(a)
#define NX_CRYPTO_EC_CHANGE_ENDIAN(a, b, c) NX_CRYPTO_EC_ASSIGN_REV(b, c); (a) = (b)

#define NX_CRYPTO_EC_SECP192R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5)         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, c0);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, c1);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, c2);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, c3);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, c4);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, c5);    \
    (s) -> nx_crypto_huge_number_size = 6;                                      \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP224R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6)     \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, c0);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, c1);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, c2);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, c3);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, c4);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, c5);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, c6);    \
    (s) -> nx_crypto_huge_number_size = 7;                                      \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP256R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7) \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, c0);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, c1);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, c2);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, c3);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, c4);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, c5);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, c6);    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, c7);    \
    (s) -> nx_crypto_huge_number_size = 8;                                      \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP256R1_DATA_SETUP_LS1(s, b1, b2, c0, c1, c2, c3, c4, c5, c6, c7) \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c0);                                                     \
    (s) -> nx_crypto_huge_number_data[8] = (b1) >> 31;                                   \
    (s) -> nx_crypto_huge_number_size = 8 + ((b1) >> 31);                                \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c1);                                                     \
    (s) -> nx_crypto_huge_number_data[7] = ((b1) << 1) | ((b2) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c2);                                                     \
    (s) -> nx_crypto_huge_number_data[6] = ((b2) << 1) | ((b1) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c3);                                                     \
    (s) -> nx_crypto_huge_number_data[5] = ((b1) << 1) | ((b2) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c4);                                                     \
    (s) -> nx_crypto_huge_number_data[4] = ((b2) << 1) | ((b1) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c5);                                                     \
    (s) -> nx_crypto_huge_number_data[3] = ((b1) << 1) | ((b2) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c6);                                                     \
    (s) -> nx_crypto_huge_number_data[2] = ((b2) << 1) | ((b1) >> 31);                   \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c7);                                                     \
    (s) -> nx_crypto_huge_number_data[1] = ((b1) << 1) | ((b2) >> 31);                   \
    (s) -> nx_crypto_huge_number_data[0] = (b2) << 1;                                    \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP384R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[11], b, c0);                     \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[10], b, c1);                     \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[9], b, c2);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[8], b, c3);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, c4);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, c5);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, c6);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, c7);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, c8);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, c9);                      \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, c10);                     \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, c11);                     \
    (s) -> nx_crypto_huge_number_size = 12;                                                       \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP384R1_DATA_SETUP_LS1(s, b1, b2, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c0);                                                                       \
    (s) -> nx_crypto_huge_number_data[12] = (b1) >> 31;                                                    \
    (s) -> nx_crypto_huge_number_size = 12 + ((b1) >> 31);                                                 \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c1);                                                                       \
    (s) -> nx_crypto_huge_number_data[11] = ((b1) << 1) | ((b2) >> 31);                                    \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c2);                                                                       \
    (s) -> nx_crypto_huge_number_data[10] = ((b2) << 1) | ((b1) >> 31);                                    \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c3);                                                                       \
    (s) -> nx_crypto_huge_number_data[9] = ((b1) << 1) | ((b2) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c4);                                                                       \
    (s) -> nx_crypto_huge_number_data[8] = ((b2) << 1) | ((b1) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c5);                                                                       \
    (s) -> nx_crypto_huge_number_data[7] = ((b1) << 1) | ((b2) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c6);                                                                       \
    (s) -> nx_crypto_huge_number_data[6] = ((b2) << 1) | ((b1) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c7);                                                                       \
    (s) -> nx_crypto_huge_number_data[5] = ((b1) << 1) | ((b2) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c8);                                                                       \
    (s) -> nx_crypto_huge_number_data[4] = ((b2) << 1) | ((b1) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c9);                                                                       \
    (s) -> nx_crypto_huge_number_data[3] = ((b1) << 1) | ((b2) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b1, c10);                                                                      \
    (s) -> nx_crypto_huge_number_data[2] = ((b2) << 1) | ((b1) >> 31);                                     \
    NX_CRYPTO_EC_ASSIGN_REV(b2, c11);                                                                      \
    (s) -> nx_crypto_huge_number_data[1] = ((b1) << 1) | ((b2) >> 31);                                     \
    (s) -> nx_crypto_huge_number_data[0] = (b2) << 1;                                                      \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#else

#define NX_CRYPTO_EC_ASSIGN_REV(a,b) (a) = (b); NX_CRYPTO_CHANGE_USHORT_ENDIAN(a)
#define NX_CRYPTO_EC_CHANGE_ENDIAN(a, b, c) NX_CRYPTO_EC_ASSIGN_REV(b, c); (a) = (b)

#define NX_CRYPTO_EC_SECP192R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5)                         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[11], b, (HN_UBASE)(c0));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[10], b, (HN_UBASE)(c0 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[9], b, (HN_UBASE)(c1));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[8], b, (HN_UBASE)(c1 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, (HN_UBASE)(c2));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, (HN_UBASE)(c2 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, (HN_UBASE)(c3));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, (HN_UBASE)(c3 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, (HN_UBASE)(c4));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, (HN_UBASE)(c4 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, (HN_UBASE)(c5));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, (HN_UBASE)(c5 >> 16));  \
    (s) -> nx_crypto_huge_number_size = 12;                                                     \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP224R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6)                     \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[13], b, (HN_UBASE)(c0));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[12], b, (HN_UBASE)(c0 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[11], b, (HN_UBASE)(c1));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[10], b, (HN_UBASE)(c1 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[9], b, (HN_UBASE)(c2));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[8], b, (HN_UBASE)(c2 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, (HN_UBASE)(c3));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, (HN_UBASE)(c3 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, (HN_UBASE)(c4));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, (HN_UBASE)(c4 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, (HN_UBASE)(c5));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, (HN_UBASE)(c5 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, (HN_UBASE)(c6));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, (HN_UBASE)(c6 >> 16));  \
    (s) -> nx_crypto_huge_number_size = 14;                                                     \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP256R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7)                 \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[15], b, (HN_UBASE)(c0));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[14], b, (HN_UBASE)(c0 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[13], b, (HN_UBASE)(c1));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[12], b, (HN_UBASE)(c1 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[11], b, (HN_UBASE)(c2));       \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[10], b, (HN_UBASE)(c2 >> 16)); \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[9], b, (HN_UBASE)(c3));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[8], b, (HN_UBASE)(c3 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, (HN_UBASE)(c4));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, (HN_UBASE)(c4 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, (HN_UBASE)(c5));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, (HN_UBASE)(c5 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, (HN_UBASE)(c6));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, (HN_UBASE)(c6 >> 16));  \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, (HN_UBASE)(c7));        \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, (HN_UBASE)(c7 >> 16));  \
    (s) -> nx_crypto_huge_number_size = 16;                                                     \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP256R1_DATA_SETUP_LS1(s, b1, b2, c0, c1, c2, c3, c4, c5, c6, c7) \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c0));                                         \
    (s) -> nx_crypto_huge_number_data[16] = (HN_UBASE)((b1) >> 15);                      \
    (s) -> nx_crypto_huge_number_size = (UINT)(16 + (HN_UBASE)((b1) >> 15));             \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c0 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[15] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c1));                                         \
    (s) -> nx_crypto_huge_number_data[14] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c1 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[13] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c2));                                         \
    (s) -> nx_crypto_huge_number_data[12] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c2 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[11] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c3));                                         \
    (s) -> nx_crypto_huge_number_data[10] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));      \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c3 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[9] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c4));                                         \
    (s) -> nx_crypto_huge_number_data[8] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c4 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[7] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c5));                                         \
    (s) -> nx_crypto_huge_number_data[6] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c5 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[5] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c6));                                         \
    (s) -> nx_crypto_huge_number_data[4] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c6 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[3] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c7));                                         \
    (s) -> nx_crypto_huge_number_data[2] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));       \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c7 >> 16));                                   \
    (s) -> nx_crypto_huge_number_data[1] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));       \
    (s) -> nx_crypto_huge_number_data[0] = (HN_UBASE)((b2) << 1);                        \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP384R1_DATA_SETUP(s, b, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[23], b, (HN_UBASE)(c0));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[22], b, (HN_UBASE)(c0 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[21], b, (HN_UBASE)(c1));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[20], b, (HN_UBASE)(c1 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[19], b, (HN_UBASE)(c2));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[18], b, (HN_UBASE)(c2 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[17], b, (HN_UBASE)(c3));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[16], b, (HN_UBASE)(c3 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[15], b, (HN_UBASE)(c4));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[14], b, (HN_UBASE)(c4 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[13], b, (HN_UBASE)(c5));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[12], b, (HN_UBASE)(c5 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[11], b, (HN_UBASE)(c6));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[10], b, (HN_UBASE)(c6 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[9], b, (HN_UBASE)(c7));          \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[8], b, (HN_UBASE)(c7 >> 16));    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[7], b, (HN_UBASE)(c8));          \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[6], b, (HN_UBASE)(c8 >> 16));    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[5], b, (HN_UBASE)(c9));          \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[4], b, (HN_UBASE)(c9 >> 16));    \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[3], b, (HN_UBASE)(c10));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[2], b, (HN_UBASE)(c10 >> 16));   \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[1], b, (HN_UBASE)(c11));         \
    NX_CRYPTO_EC_CHANGE_ENDIAN((s) -> nx_crypto_huge_number_data[0], b, (HN_UBASE)(c11 >> 16));   \
    (s) -> nx_crypto_huge_number_size = 24;                                                       \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#define NX_CRYPTO_EC_SECP384R1_DATA_SETUP_LS1(s, b1, b2, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11) \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c0));                                                           \
    (s) -> nx_crypto_huge_number_data[24] = (HN_UBASE)((b1) >> 15);                                        \
    (s) -> nx_crypto_huge_number_size = (UINT)(24 + (HN_UBASE)((b1) >> 15));                               \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c0 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[23] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c1));                                                           \
    (s) -> nx_crypto_huge_number_data[22] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c1 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[21] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c2));                                                           \
    (s) -> nx_crypto_huge_number_data[20] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c2 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[19] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c3));                                                           \
    (s) -> nx_crypto_huge_number_data[18] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c3 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[17] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c4));                                                           \
    (s) -> nx_crypto_huge_number_data[16] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c4 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[15] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c5));                                                           \
    (s) -> nx_crypto_huge_number_data[14] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c5 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[13] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c6));                                                           \
    (s) -> nx_crypto_huge_number_data[12] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c6 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[11] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c7));                                                           \
    (s) -> nx_crypto_huge_number_data[10] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                        \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c7 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[9] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c8));                                                           \
    (s) -> nx_crypto_huge_number_data[8] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c8 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[7] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c9));                                                           \
    (s) -> nx_crypto_huge_number_data[6] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c9 >> 16));                                                     \
    (s) -> nx_crypto_huge_number_data[5] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c10));                                                          \
    (s) -> nx_crypto_huge_number_data[4] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c10 >> 16));                                                    \
    (s) -> nx_crypto_huge_number_data[3] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b1, (HN_UBASE)(c11));                                                          \
    (s) -> nx_crypto_huge_number_data[2] = (HN_UBASE)(((b2) << 1) | ((b1) >> 15));                         \
    NX_CRYPTO_EC_ASSIGN_REV(b2, (HN_UBASE)(c11 >> 16));                                                    \
    (s) -> nx_crypto_huge_number_data[1] = (HN_UBASE)(((b1) << 1) | ((b2) >> 15));                         \
    (s) -> nx_crypto_huge_number_data[0] = (HN_UBASE)((b2) << 1);                                          \
    (s) -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

#endif

extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp192r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp224r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp256r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp384r1;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_secp521r1;

#define NX_CRYPTO_EC_GET_SECP192R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp192r1
#define NX_CRYPTO_EC_GET_SECP224R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp224r1
#define NX_CRYPTO_EC_GET_SECP256R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp256r1
#define NX_CRYPTO_EC_GET_SECP384R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp384r1
#define NX_CRYPTO_EC_GET_SECP521R1(curve) curve = (NX_CRYPTO_EC *)&_nx_crypto_ec_secp521r1

UINT _nx_crypto_ec_point_is_infinite(NX_CRYPTO_EC_POINT *point);
VOID _nx_crypto_ec_point_set_infinite(NX_CRYPTO_EC_POINT *point);
UINT _nx_crypto_ec_point_setup(NX_CRYPTO_EC_POINT *point, UCHAR *byte_stream, UINT byte_stream_size);
VOID _nx_crypto_ec_point_extract_uncompressed(NX_CRYPTO_EC *curve, NX_CRYPTO_EC_POINT *point, UCHAR *byte_stream,
                                              UINT byte_stream_size, UINT *huge_number_size);

VOID _nx_crypto_ec_point_fp_affine_to_projective(NX_CRYPTO_EC_POINT *point);
VOID _nx_crypto_ec_point_fp_projective_to_affine(NX_CRYPTO_EC *curve,
                                                 NX_CRYPTO_EC_POINT *point,
                                                 HN_UBASE *scratch);

VOID _nx_crypto_ec_secp192r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp224r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp256r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp384r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_secp521r1_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_reduce(NX_CRYPTO_EC *curve,
                             NX_CRYPTO_HUGE_NUMBER *value,
                             HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_add(NX_CRYPTO_EC *curve,
                                     NX_CRYPTO_EC_POINT *projective_point,
                                     NX_CRYPTO_EC_POINT *affine_point,
                                     HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_double(NX_CRYPTO_EC *curve,
                                        NX_CRYPTO_EC_POINT *projective_point,
                                        HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_affine_add(NX_CRYPTO_EC *curve,
                                 NX_CRYPTO_EC_POINT *left,
                                 NX_CRYPTO_EC_POINT *right,
                                 HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_affine_subtract(NX_CRYPTO_EC *curve,
                                      NX_CRYPTO_EC_POINT *left,
                                      NX_CRYPTO_EC_POINT *right,
                                      HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_projective_multiple(NX_CRYPTO_EC *curve,
                                          NX_CRYPTO_EC_POINT *g,
                                          NX_CRYPTO_HUGE_NUMBER *d,
                                          NX_CRYPTO_EC_POINT *r,
                                          HN_UBASE *scratch);
VOID _nx_crypto_ec_fp_fixed_multiple(NX_CRYPTO_EC *curve,
                                     NX_CRYPTO_HUGE_NUMBER *d,
                                     NX_CRYPTO_EC_POINT *r,
                                     HN_UBASE *scratch);

VOID _nx_crypto_ec_naf_compute(NX_CRYPTO_HUGE_NUMBER *d, HN_UBASE *naf_data, UINT *naf_size);
VOID _nx_crypto_ec_add_digit_reduce(NX_CRYPTO_EC *curve,
                                    NX_CRYPTO_HUGE_NUMBER *value,
                                    HN_UBASE digit,
                                    HN_UBASE *scratch);
VOID _nx_crypto_ec_subtract_digit_reduce(NX_CRYPTO_EC *curve,
                                         NX_CRYPTO_HUGE_NUMBER *value,
                                         HN_UBASE digit,
                                         HN_UBASE *scratch);
VOID _nx_crypto_ec_add_reduce(NX_CRYPTO_EC *curve,
                              NX_CRYPTO_HUGE_NUMBER *left,
                              NX_CRYPTO_HUGE_NUMBER *right,
                              HN_UBASE *scratch);
VOID _nx_crypto_ec_subtract_reduce(NX_CRYPTO_EC *curve,
                                   NX_CRYPTO_HUGE_NUMBER *left,
                                   NX_CRYPTO_HUGE_NUMBER *right,
                                   HN_UBASE *scratch);
VOID _nx_crypto_ec_precomputation(NX_CRYPTO_EC *curve,
                                  UINT window_width,
                                  UINT bits,
                                  HN_UBASE **scratch_pptr);
VOID _nx_crypto_ec_fixed_output(NX_CRYPTO_EC *curve,
                                INT (*output)(const CHAR *format, ...),
                                const CHAR *tab,
                                const CHAR *line_ending);
UINT _nx_crypto_ec_get_named_curve(NX_CRYPTO_EC **curve, UINT curve_id);
UINT _nx_crypto_ec_key_pair_generation_extra(NX_CRYPTO_EC *curve,
                                             NX_CRYPTO_EC_POINT *g,
                                             NX_CRYPTO_HUGE_NUMBER *private_key,
                                             NX_CRYPTO_EC_POINT *public_key,
                                             HN_UBASE *scratch);
NX_CRYPTO_KEEP UINT _nx_crypto_ec_key_pair_stream_generate(NX_CRYPTO_EC *curve,
                                                           UCHAR *output,
                                                           ULONG output_length_in_byte,
                                                           ULONG *actual_output_length,
                                                           HN_UBASE *scratch);
#ifndef NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION
UINT _nx_crypto_ec_validate_public_key(NX_CRYPTO_EC_POINT *public_key,
                                       NX_CRYPTO_EC *chosen_curve,
                                       UINT partial,
                                       HN_UBASE *scratch);
#endif /* NX_CRYPTO_ECC_DISABLE_KEY_VALIDATION */

UINT _nx_crypto_method_ec_secp192r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp224r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp256r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp384r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_secp521r1_operation(UINT op,
                                              VOID *handle,
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input, ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output, ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_x25519;
extern NX_CRYPTO_CONST NX_CRYPTO_EC _nx_crypto_ec_x448;

VOID _nx_crypto_ec_cswap(UINT swap, NX_CRYPTO_HUGE_NUMBER *h1, NX_CRYPTO_HUGE_NUMBER *h2);
VOID _nx_crypto_ec_x25519_448_multiple(NX_CRYPTO_EC *curve,
                                       NX_CRYPTO_EC_POINT *u,
                                       NX_CRYPTO_HUGE_NUMBER *k,
                                       NX_CRYPTO_EC_POINT *r,
                                       HN_UBASE *scratch);
UINT _nx_crypto_method_ec_x25519_operation(UINT op,
                                           VOID *handle,
                                           struct NX_CRYPTO_METHOD_STRUCT *method,
                                           UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                           UCHAR *input, ULONG input_length_in_byte,
                                           UCHAR *iv_ptr,
                                           UCHAR *output, ULONG output_length_in_byte,
                                           VOID *crypto_metadata, ULONG crypto_metadata_size,
                                           VOID *packet_ptr,
                                           VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_method_ec_x448_operation(UINT op,
                                         VOID *handle,
                                         struct NX_CRYPTO_METHOD_STRUCT *method,
                                         UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                         UCHAR *input, ULONG input_length_in_byte,
                                         UCHAR *iv_ptr,
                                         UCHAR *output, ULONG output_length_in_byte,
                                         VOID *crypto_metadata, ULONG crypto_metadata_size,
                                         VOID *packet_ptr,
                                         VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));
UINT _nx_crypto_ec_key_pair_generation_x25519_448(NX_CRYPTO_EC *curve,
                                                  NX_CRYPTO_EC_POINT *g,
                                                  NX_CRYPTO_HUGE_NUMBER *private_key,
                                                  NX_CRYPTO_EC_POINT *public_key,
                                                  HN_UBASE *scratch);
UINT _nx_crypto_ec_extract_fixed_size_le(NX_CRYPTO_HUGE_NUMBER *number,
                                         UCHAR *byte_stream, UINT byte_stream_size);
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_EC_H */

