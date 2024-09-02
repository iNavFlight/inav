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

#define NX_CRYPTO_SOURCE_CODE


/* Include necessary system files.  */
#include "stdio.h"
#include "nx_crypto.h"
#include "nx_crypto_huge_number.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_is_zero                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns whether number x is zero or not.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Huge number x                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                NX_CRYPTO_TRUE: x is zero     */
/*                                          NX_CRYPTO_FALSE: x is nonzero */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_point_is_infinite       Check if the point is infinite*/
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_is_zero(NX_CRYPTO_HUGE_NUMBER *x)
{
UINT i;

    for (i = 0; i < x -> nx_crypto_huge_number_size; i++)
    {
        if (x -> nx_crypto_huge_number_data[i])
        {

            /* Found one value that is not zero. */
            return(NX_CRYPTO_FALSE);
        }
    }

    /* All values are zero. */
    return(NX_CRYPTO_TRUE);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_add                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates addition for huge numbers.                 */
/*                                                                        */
/*    Note: Result is stored in left.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  Huge number left              */
/*    right                                 Huge number right             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_add_unsigned   Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_add(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right)
{
UINT cmp;

    cmp = _nx_crypto_huge_number_compare_unsigned(left, right);

    if (left -> nx_crypto_huge_number_is_negative == right -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_add_unsigned(left, right);
    }
    else
    {

        if (cmp == NX_CRYPTO_HUGE_NUMBER_EQUAL)
        {
            NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(left, 0);
        }
        else if (cmp == NX_CRYPTO_HUGE_NUMBER_LESS)
        {
            _nx_crypto_huge_number_subtract_unsigned(right, left, left);
            left -> nx_crypto_huge_number_is_negative = right -> nx_crypto_huge_number_is_negative;
        }
        else
        {
            _nx_crypto_huge_number_subtract_unsigned(left, right, left);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_subtract                     PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates subtraction for huge numbers.              */
/*                                                                        */
/*    Note: Result is stored in left.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  Huge number left              */
/*    right                                 Huge number right             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_add_unsigned   Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_huge_number_mont           Perform Montgomery reduction  */
/*                                            for multiplication          */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved performance,       */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_subtract(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right)
{
UINT cmp;

    cmp = _nx_crypto_huge_number_compare_unsigned(left, right);

    if (left -> nx_crypto_huge_number_is_negative != right -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_add_unsigned(left, right);
    }
    else
    {

        if (cmp == NX_CRYPTO_HUGE_NUMBER_EQUAL)
        {
            NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(left, 0);
        }
        else if (cmp == NX_CRYPTO_HUGE_NUMBER_LESS)
        {
            _nx_crypto_huge_number_subtract_unsigned(right, left, left);
            left -> nx_crypto_huge_number_is_negative = !(right -> nx_crypto_huge_number_is_negative);
        }
        else
        {
            _nx_crypto_huge_number_subtract_unsigned(left, right, left);
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_add_unsigned                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates addition for unsigned huge numbers.        */
/*                                                                        */
/*    Note: Result is stored in left.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  Huge number left              */
/*    right                                 Huge number right             */
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
/*    _nx_crypto_ec_add_reduce              Perform addition between      */
/*                                            two huge numbers            */
/*    _nx_crypto_ec_subtract_digit_reduce   Perform subtraction between   */
/*                                            huge number and digit number*/
/*    _nx_crypto_ec_subtract_reduce         Perform subtraction between   */
/*                                            two huge numbers            */
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_add_unsigned(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right)
{
UINT      i;
HN_UBASE2 product;
HN_UBASE *left_buffer;
HN_UBASE *right_buffer;
HN_UBASE *result_buffer = left -> nx_crypto_huge_number_data;
UINT      left_size;
UINT      right_size;

    product = 0;

    /* Make sure the size of left is no less than the size of right. */
    if (left -> nx_crypto_huge_number_size < right -> nx_crypto_huge_number_size)
    {
        left_buffer = right -> nx_crypto_huge_number_data;
        right_buffer = left -> nx_crypto_huge_number_data;
        left_size = right -> nx_crypto_huge_number_size;
        right_size = left -> nx_crypto_huge_number_size;
    }
    else
    {
        left_buffer = left -> nx_crypto_huge_number_data;
        right_buffer = right -> nx_crypto_huge_number_data;
        left_size = left -> nx_crypto_huge_number_size;
        right_size = right -> nx_crypto_huge_number_size;
    }

    /* Calculate the result for common length. */
    for (i = 0; i < right_size; i++)
    {
        product = (product >> HN_SHIFT) + left_buffer[i] + right_buffer[i];
        result_buffer[i] = product & HN_MASK;
    }

    /* Calculate the carry. */
    for (; i < left_size; i++)
    {
        product = (product >> HN_SHIFT) + left_buffer[i];
        result_buffer[i] = product & HN_MASK;
    }

    /* Save or drop the carry? */
    if ((product >> HN_SHIFT) &&
        ((i << HN_SIZE_SHIFT) < left -> nx_crypto_huge_buffer_size))
    {

        /* Save the carry. */
        result_buffer[i] = 1;
        left -> nx_crypto_huge_number_size = i + 1;
    }
    else
    {
        left -> nx_crypto_huge_number_size = i;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_subtract_unsigned            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates subtraction for unsigned huge numbers.     */
/*                                                                        */
/*    Note: Left must be no less than right.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  Huge number left              */
/*    right                                 Huge number right             */
/*    result                                Huge number result            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_subtract_unsigned(NX_CRYPTO_HUGE_NUMBER *left,
                                                             NX_CRYPTO_HUGE_NUMBER *right,
                                                             NX_CRYPTO_HUGE_NUMBER *result)
{
UINT      i;
HN_UBASE2 product;
HN_UBASE *left_buffer = left -> nx_crypto_huge_number_data;
HN_UBASE *right_buffer = right -> nx_crypto_huge_number_data;
HN_UBASE *result_buffer = result -> nx_crypto_huge_number_data;

    /* Note, in following loops,
     * borrow = 1 - (product >> HN_SHIFT) */
    product = HN_RADIX;

    /* Calculate the result for common length. */
    for (i = 0; i < right -> nx_crypto_huge_number_size; i++)
    {
        product >>= HN_SHIFT;
        product += (HN_UBASE2)((HN_RADIX - 1) + left_buffer[i] - right_buffer[i]);
        result_buffer[i] = (product & HN_MASK);
    }

    /* Calculate the borrow. */
    for (; i < left -> nx_crypto_huge_number_size; i++)
    {
        product >>= HN_SHIFT;
        product += (HN_UBASE2)((HN_RADIX - 1) + left_buffer[i]);
        result_buffer[i] = (product & HN_MASK);
    }

    result -> nx_crypto_huge_number_size = left -> nx_crypto_huge_number_size;
    _nx_crypto_huge_number_adjust_size(result);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_add_digit_unsigned           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates addition for unsigned huge number and      */
/*    digit.                                                              */
/*                                                                        */
/*    Note: Result is stored in value.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    value                                 Huge number number            */
/*    digit                                 Digit value                   */
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
/*    _nx_crypto_huge_number_add_digit                                    */
/*    _nx_crypto_huge_number_subtract_digit                               */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_add_digit_unsigned(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit)
{
UINT      i;
HN_UBASE2 product;
HN_UBASE *buffer = value -> nx_crypto_huge_number_data;
UINT      size = value -> nx_crypto_huge_number_size;

    product = (HN_UBASE2)buffer[0] + digit;
    buffer[0] = product & HN_MASK;

    /* Process carry. */
    for (i = 1; (i < size) && ((product >> HN_SHIFT) != 0); i++)
    {
        product = (HN_UBASE)buffer[i] + 1;
        buffer[i] = product & HN_MASK;
    }

    /* Save or drop the carry? */
    if ((product >> HN_SHIFT) && (i == size))
    {
        buffer[i] = 1;
        value -> nx_crypto_huge_number_size = i + 1;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_subtract_digit_unsigned      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates subtraction for unsigned huge number and   */
/*    digit.                                                              */
/*                                                                        */
/*    Note: Result is stored in value. Left must be no less than right.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    value                                 Huge number number            */
/*    digit                                 Digit value                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_subtract_digit_reduce   Perform subtraction between   */
/*                                            huge number and digit number*/
/*    _nx_crypto_huge_number_add_digit      Calculate addition for        */
/*                                            huge number and digit       */
/*    _nx_crypto_huge_number_subtract_digit Calculate subtraction for     */
/*                                            huge number and digit       */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_subtract_digit_unsigned(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit)
{
UINT      i;
HN_UBASE2 product;
HN_UBASE *buffer = value -> nx_crypto_huge_number_data;
UINT      size = value -> nx_crypto_huge_number_size;

    product = (HN_UBASE2)(HN_RADIX + buffer[0] - digit);
    buffer[0] = product & HN_MASK;

    /* Process borrow. */
    for (i = 1; (i < size) && ((product >> HN_SHIFT) == 0); i++)
    {
        product = (HN_UBASE2)(HN_RADIX + (HN_UBASE)buffer[i] - 1);
        buffer[i] = product & HN_MASK;
    }

    _nx_crypto_huge_number_adjust_size(value);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_left                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shifts left N bits. The shift must be less than       */
/*    the value of HN_SHIFT.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Huge number x                 */
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
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_shift_left(NX_CRYPTO_HUGE_NUMBER *x, UINT shift)
{
UINT      i;
HN_UBASE *x_buffer = x -> nx_crypto_huge_number_data;

    /* Left shift. */
    i = x -> nx_crypto_huge_number_size;
    x_buffer[i] = x_buffer[i - 1] >> (HN_SHIFT - shift);
    for (i--; i > 0; i--)
    {
        x_buffer[i] = (x_buffer[i - 1] >> (HN_SHIFT - shift)) | (x_buffer[i] << shift);
    }
    x_buffer[0] <<= shift;

    if (x_buffer[x -> nx_crypto_huge_number_size] != 0)
    {
        x -> nx_crypto_huge_number_size++;
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_shift_right                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function shifts right N bits. The shift must be less than      */
/*    the value of HN_SHIFT.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Huge number x                 */
/*    shift                                 Bits to shift right           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_shift_right(NX_CRYPTO_HUGE_NUMBER *x, UINT shift)
{
UINT      i;
HN_UBASE *x_buffer = x -> nx_crypto_huge_number_data;

    /* Right shift. */
    for (i = 0; i < x -> nx_crypto_huge_number_size - 1; i++)
    {
        x_buffer[i] = (x_buffer[i + 1] << (HN_SHIFT - shift)) | (x_buffer[i] >> shift);
    }
    x_buffer[i] >>= shift;

    _nx_crypto_huge_number_adjust_size(x);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adjusts the size of a huge number to include only     */
/*    those bytes which are actually significant to the value -           */
/*    essentially removing leading zero bytes.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    val                                   Pointer to huge number        */
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
/*    _nx_crypto_huge_number_setup          Setup huge number             */
/*    _nx_crypto_huge_number_subtract_unsigned                            */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_digit_unsigned                      */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_shift_right    Shift right for huge number   */
/*    _nx_crypto_huge_number_multiply       Multiply two huge numbers     */
/*    _nx_crypto_huge_number_square         Compute the square of a value */
/*    _nx_crypto_huge_number_mont           Perform Montgomery reduction  */
/*                                            for multiplication          */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_adjust_size(NX_CRYPTO_HUGE_NUMBER *val)
{

INT size;

    /* Start at the most significant "digit" and work backwards. */
    for (size = (INT)(val -> nx_crypto_huge_number_size - 1); size >= 0; size--)
    {
        if (val -> nx_crypto_huge_number_data[size] != 0)
        {

            /* Size is now the index of the last non-zero byte,
               so the size is one more than that. */
            val -> nx_crypto_huge_number_size = (UINT)(size + 1);
            return;
        }
    }

    /* The result is zero. */
    val -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    val -> nx_crypto_huge_number_size = 1;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_compare                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares two huge numbers and returns equal, less-    */
/*    than, or greater-than.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  First operand                 */
/*    right                                 Second operand                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Result                                Equal, less-than, greater     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_point_is_infinite       Check if the point is infinite*/
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_huge_number_mont           Perform Montgomery reduction  */
/*                                            for multiplication          */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_compare(NX_CRYPTO_HUGE_NUMBER *left, NX_CRYPTO_HUGE_NUMBER *right)
{
    if (left -> nx_crypto_huge_number_is_negative != right -> nx_crypto_huge_number_is_negative)
    {
        if (left -> nx_crypto_huge_number_is_negative)
        {
            return(NX_CRYPTO_HUGE_NUMBER_LESS);
        }
        else
        {
            return(NX_CRYPTO_HUGE_NUMBER_GREATER);
        }
    }

    if (left -> nx_crypto_huge_number_is_negative)
    {
        return(_nx_crypto_huge_number_compare_unsigned(right, left));
    }
    else
    {
        return(_nx_crypto_huge_number_compare_unsigned(left, right));
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_compare_unsigned             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares two huge numbers and returns equal, less-    */
/*    than, or greater-than. Sign is ignored. Results are in relation to  */
/*    the first operand (e.g. left > right returns greater-than). The     */
/*    algorithm properly handles leading zeroes so operands with different*/
/*    buffer sizes are handled properly.                                  */
/*                                                                        */
/*    The shift component is a specialty optimization for the fast        */
/*    modulus routine (_nx_crypto_huge_number_modulus).                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  First operand                 */
/*    right                                 Second operand                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Result                                Equal, less-than, greater     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_add_digit_reduce        Perform addition between huge */
/*                                            number and digit number     */
/*    _nx_crypto_ec_add_reduce              Perform addition between      */
/*                                            two huge numbers            */
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_compare_unsigned(NX_CRYPTO_HUGE_NUMBER *left,
                                                            NX_CRYPTO_HUGE_NUMBER *right)
{

INT  i;
UINT left_size;
UINT right_size;

    left_size = left -> nx_crypto_huge_number_size;
    right_size = right -> nx_crypto_huge_number_size;

    /* Adjust the size for zero at the front. */
    while (left_size && (left -> nx_crypto_huge_number_data[left_size - 1] == 0))
    {
        left_size--;
    }
    while (right_size && (right -> nx_crypto_huge_number_data[right_size - 1] == 0))
    {
        right_size--;
    }

    if (left_size > right_size)
    {
        return(NX_CRYPTO_HUGE_NUMBER_GREATER);
    }
    if (right_size > left_size)
    {
        return(NX_CRYPTO_HUGE_NUMBER_LESS);
    }

    /* Keep comparing bytes while they are equal and we have indexes left. */
    for (i = (INT)left_size - 1; i >= 0; i--)
    {
        if (left -> nx_crypto_huge_number_data[i] > right -> nx_crypto_huge_number_data[i])
        {
            return(NX_CRYPTO_HUGE_NUMBER_GREATER);
        }
        else if (right -> nx_crypto_huge_number_data[i] > left -> nx_crypto_huge_number_data[i])
        {
            return(NX_CRYPTO_HUGE_NUMBER_LESS);
        }
    }

    /* If we reach here, the numbers are equal. */
    return(NX_CRYPTO_HUGE_NUMBER_EQUAL);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_multiply                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function multiplies two huge numbers and places the result     */
/*    in the result buffer which must be large enough to hold the         */
/*    resulting value.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    left                                  First operand                 */
/*    right                                 Second operand                */
/*    result                                Result                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecjpake_public_key_generate                              */
/*                                          Perform public key generation */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_huge_number_power_modulus  Raise a huge number           */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_multiply(NX_CRYPTO_HUGE_NUMBER *left,
                                                    NX_CRYPTO_HUGE_NUMBER *right,
                                                    NX_CRYPTO_HUGE_NUMBER *result)
{

UINT      index, right_index; /* Loop variables */
HN_UBASE *left_buffer, *right_buffer;
HN_UBASE2 product;
UINT      left_size, right_size;
HN_UBASE *result_buffer;
HN_UBASE *temp_ptr;

    left_size = left -> nx_crypto_huge_number_size;
    right_size = right -> nx_crypto_huge_number_size;
    left_buffer = left -> nx_crypto_huge_number_data;
    right_buffer = right -> nx_crypto_huge_number_data;

    /* The temp buffer serves as "buckets".  During each itegration, the products of two digits are accumucated into
       this buckets.  Since each "digit" is 16-bit, by using 32-bit buckets, the carry can be accumulated in to the
       top 16-bits of the bucket.  Therefore, we don't need to worry about "carry" bit during each iteration.  */

    result_buffer = result -> nx_crypto_huge_number_data;
    result -> nx_crypto_huge_number_size = (left_size + right_size);

    /* Zero out the buckets since we are going to accumulate "digits" there. */
    NX_CRYPTO_MEMSET(result_buffer, 0, (left_size + right_size) << HN_SIZE_SHIFT);

    /* Loop over the operands, grabbing a "digit" at a time and calculating the product accounting for carry. */
    for (index = 0; index < left_size; ++index)
    {

        /* If the "digit" on the left is zero, there is no need to loop through the "digits" on the right. */
        if (left_buffer[index] == 0)
        {
            /* Yes the digit ont he left is zero.  Continue to the next iteration. */
            continue;
        }

        product = 0;
        temp_ptr = result_buffer + index;
        for (right_index = 0; right_index < right_size; ++right_index, ++temp_ptr)
        {
            /* Multiple "digit" from the left with the one the left. */
            product >>= HN_SHIFT;
            product += (HN_UBASE2)left_buffer[index] * (HN_UBASE2)right_buffer[right_index] + *temp_ptr;

            /* The result (in product) could be two parts: the top digit the the lower digit. */
            /* Accumulate the lower digit into the "bucket" and the top digit to the next "bucket". */
            *temp_ptr = (product & HN_MASK);
        }
        *temp_ptr = (HN_UBASE)((product >> HN_SHIFT));
    }

    /* Set is_negative. */
    if (left -> nx_crypto_huge_number_is_negative == right -> nx_crypto_huge_number_is_negative)
    {
        result -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    }
    else
    {
        result -> nx_crypto_huge_number_is_negative = NX_CRYPTO_TRUE;
    }

    _nx_crypto_huge_number_adjust_size(result);

    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_multiply_digit               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function multiplies huge number with digit and replace the     */
/*    result in the result buffer which must be large enough to hold the  */
/*    resulting value.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    value                                 Huge number number            */
/*    digit                                 Digit value                   */
/*    result                                Result                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_multiply_digit(NX_CRYPTO_HUGE_NUMBER *value,
                                                          HN_UBASE digit,
                                                          NX_CRYPTO_HUGE_NUMBER *result)
{

UINT      i;
UINT      size;
HN_UBASE2 product;
HN_UBASE *value_buffer;
HN_UBASE *result_buffer;

    if (digit == 0)
    {
        result -> nx_crypto_huge_number_data[0] = 0;
        result -> nx_crypto_huge_number_size = 1;
        result -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
        return;
    }

    size = value -> nx_crypto_huge_number_size;
    value_buffer = value -> nx_crypto_huge_number_data;
    result_buffer = result -> nx_crypto_huge_number_data;

    /* Loop over the operands, grabbing a "digit" at a time and calculating the product accounting for carry. */
    product = 0;
    for (i = 0; i < size; i++)
    {
        product >>= HN_SHIFT;
        product += (HN_UBASE2)value_buffer[i] * digit;
        result_buffer[i] = (product & HN_MASK);
    }
    if ((product >> HN_SHIFT) != 0)
    {
        result -> nx_crypto_huge_number_size = value -> nx_crypto_huge_number_size + 1;
        result_buffer[i] = (HN_UBASE)(product >> HN_SHIFT);
    }
    else
    {
        result -> nx_crypto_huge_number_size = value -> nx_crypto_huge_number_size;
    }
    result -> nx_crypto_huge_number_is_negative = value -> nx_crypto_huge_number_is_negative;

    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_square                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the square of a value, and places the        */
/*    result in the result buffer. The value buffer must be large enough  */
/*    to hold the computed result.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    value                                 Value buffer                  */
/*    result                                Result buffer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_huge_number_power_modulus  Raise a huge number           */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_square(NX_CRYPTO_HUGE_NUMBER *value, NX_CRYPTO_HUGE_NUMBER *result)
{
HN_UBASE2 product;
UINT      value_size;
UINT      result_size;
HN_UBASE *value_buffer;
HN_UBASE *result_buffer;
UINT      i, j;

    /* Using Yang et al.'s squaring algorithm. */
    value_size = value -> nx_crypto_huge_number_size;
    result_size = (value_size << 1);
    result -> nx_crypto_huge_number_size = result_size;
    value_buffer = value -> nx_crypto_huge_number_data;
    result_buffer = result -> nx_crypto_huge_number_data;

    NX_CRYPTO_MEMSET(result_buffer, 0, result_size << HN_SIZE_SHIFT);

    for (i = 0; i < value_size; i++)
    {
        product = 0;
        for (j = i + 1; j < value_size; j++)
        {
            product >>= HN_SHIFT;
            product += result_buffer[i + j] + (HN_UBASE2)value_buffer[i] * value_buffer[j];
            result_buffer[i + j] = product & HN_MASK;
        }
        result_buffer[i + j] = (HN_UBASE)(product >> HN_SHIFT);
    }

    for (i = result_size - 1; i > 0; i--)
    {
        result_buffer[i] = (result_buffer[i] << 1) | (result_buffer[i - 1] >> (HN_SHIFT - 1));
    }
    result_buffer[0] <<= 1;

    product = 0;
    for (i = 0; i < value_size; i++)
    {
        product >>= HN_SHIFT;
        product += result_buffer[i << 1] + (HN_UBASE2)value_buffer[i] * value_buffer[i];
        result_buffer[i << 1] = product & HN_MASK;
        product >>= HN_SHIFT;
        product += result_buffer[(i << 1) + 1];
        result_buffer[(i << 1) + 1] = product & HN_MASK;
    }

    result -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    _nx_crypto_huge_number_adjust_size(result);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_modulus                      PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs a modulus operation on the dividend and      */
/*    divisor operands and places the remainder in the dividend buffer.   */
/*                                                                        */
/*    The algorithm uses a digit-based division, with a 16-bit short      */
/*    as a "digit".                                                       */
/*                                                                        */
/*    NORE: The dividend operand is destroyed during the operation.       */
/*          The divisor is preserved.  No additional scratch buffer       */
/*          required during this operation.                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dividend                              First operand                 */
/*    divisor                               Second operand                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_compare_unsigned                             */
/*                                          Compare two unsigned          */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_reduce               Reduce huge number for        */
/*                                            common prime field          */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
/*    _nx_crypto_ecjpake_key_exchange_process                             */
/*                                          Process key exchange message  */
/*    _nx_crypto_ecjpake_public_key_generate                              */
/*                                          Perform public key generation */
/*    _nx_crypto_ecjpake_schnorr_zkp_generate                             */
/*                                          Perform Schnorr ZKP generation*/
/*    _nx_crypto_ecjpake_schnorr_zkp_hash   Perform Schnorr ZKP hash      */
/*                                            calculation                 */
/*    _nx_crypto_huge_number_power_modulus  Raise a huge number           */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed variable type issue,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed division by zero bug, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_modulus(NX_CRYPTO_HUGE_NUMBER *dividend, NX_CRYPTO_HUGE_NUMBER *divisor)
{
UINT                   result_length, divisor_length; /* In number of USHORT words. */
UINT                   compare_value;
UINT                   i;
UINT                   shift; /* In number of USHORT words. */
HN_UBASE              *result_buffer, *divisor_buffer;
HN_UBASE2              scale;
HN_UBASE               divisor_msb;
HN_UBASE               dividend_msb;
HN_UBASE2              product;
HN_UBASE2              value, carry;
HN_UBASE               borrow;
UINT                   index;
NX_CRYPTO_HUGE_NUMBER *result;


    /* The algorithm used here is an implementation of the traditional long division.
       One 16-bit unsigned integer is treated as a "digit".  It starts with the 2 most-significant digits
       of the dividend and the most significant digit of the divisor to come up with an estimate of a scale value.  After
       subtracting the product of the scale and divisor.

       To prevent the scale value from being too big, we also look into the second digit of the divisor.
       If the 2nd digit is greater or equal to 0x80, the scale value is reduced.

               scale
              ____________________________
       BBbbxxxx  )AAAAxxxxxxxxxxxxxxxxxx
             / CCCCxxxxxx

             In the figure above, AAAA (dividend_msb) is the top 2 digits picked up from the dividend.
             BB isthe top digit picked up from the divisor.  To prevent overflow, the next digit bb is
             examined.  If bb equals or is greater than 0x80, BB is incremented by 1, effectively reducing
             the scale.

             Then dividend subtracts the product of divisor and scale (with a left "shift").

             keep going through this operation till the remainder is less than the divisor.
     */

    /* Copy dividend into result */
    result = dividend;

    /* If the dividend is smaller than the divisor, the remainder is simply the dividend so we are done. */
    compare_value = _nx_crypto_huge_number_compare_unsigned(result, divisor);
    if (compare_value == NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        if (result -> nx_crypto_huge_number_is_negative)
        {
            _nx_crypto_huge_number_add(result, divisor);
        }
        return;
    }

    /* Get divisor_length and result_legnth, which are converted to number of USHORT */
    divisor_length = divisor -> nx_crypto_huge_number_size - 1;
    result_length = result -> nx_crypto_huge_number_size  - 1;

    /* Get local pointers into our buffers for easier reading. */
    result_buffer = result -> nx_crypto_huge_number_data;
    divisor_buffer = divisor -> nx_crypto_huge_number_data;

    // divisor_buffer[divisor_length + 1]  = 0;

    /* First, obtain our divisor_msb value. */
    while (divisor_buffer[divisor_length] == 0)
    {
        divisor_length--;
    }
    divisor_msb = divisor_buffer[divisor_length] >> (HN_SHIFT >> 1);


    divisor_msb++;

    /* Going through the scale/subtraction loop till the dividend is less than
       divisor.  At this point the value in the dividend buffer is the remainder. */

    /* Obtain reuslt_length */
    result_length = result -> nx_crypto_huge_number_size - 1;
    while (compare_value != NX_CRYPTO_HUGE_NUMBER_LESS)
    {
        dividend_msb = result_buffer[result_length] >> (HN_SHIFT >> 1);
        shift = result_length - divisor_length;

        if (shift && (dividend_msb < divisor_msb))
        {
            dividend_msb = result_buffer[result_length];
            shift--;

            if (dividend_msb < divisor_msb)
            {
                dividend_msb = (result_buffer[result_length] << (HN_SHIFT >> 1)) |
                                (result_buffer[result_length - 1] >> (HN_SHIFT >> 1));
                scale = dividend_msb / divisor_msb;
            }
            else
            {
                scale = ((HN_UBASE2)(dividend_msb / divisor_msb)) << (HN_SHIFT >> 1);
            }
        }
        else if (shift)
        {
            scale = dividend_msb / divisor_msb;
        }
        else
        {
            scale = result_buffer[result_length] / ((HN_UBASE2)divisor_buffer[divisor_length] + 1);
            if (scale == 0)
            {
                scale = 1;
            }
        }


        carry = 0;
        borrow = 0;
        i = 0;

        for (index = shift; index < result_length; index++)
        {
            product = divisor_buffer[i] * scale;
            product += carry;

            carry = product >> HN_SHIFT; /* The carry bits used for the next itegration. */

            value = result_buffer[index] + (HN_UBASE2)(HN_BASE)borrow;
            value = value - (product & HN_MASK);

            borrow = (HN_UBASE)(value >> HN_SHIFT); /* The borrow bit used for the next itegration. */

            result_buffer[index] = (HN_UBASE)(value & HN_MASK);

            i++;
        }

        if (i <= divisor_length)
        {
            product = divisor_buffer[i] * scale + carry;
        }
        else
        {
            product = carry;
        }

        value = result_buffer[index] + (HN_UBASE2)(HN_BASE)borrow;
        value = value - (product & HN_MASK);

        borrow = (HN_UBASE)(value >> HN_SHIFT); /* The borrow bit used for the next itegration. */

        result_buffer[index] = (HN_UBASE)(value & HN_MASK);

        while ((index > 0) && (result_buffer[index] == 0))
        {
            index--;
        }
        result_length = index;

        result -> nx_crypto_huge_number_size = index + 1;
        if (result_length <= divisor_length)
        {

            compare_value = _nx_crypto_huge_number_compare_unsigned(result, divisor);
        }
    }

    if (result -> nx_crypto_huge_number_is_negative)
    {
        _nx_crypto_huge_number_add(result, divisor);
    }
    return;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_setup                         PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function raises a huge number to the power of a second huge    */
/*    number using a third huge number as a modulus. The result is placed */
/*    in a fourth huge number.                                            */
/*                                                                        */
/*    NOTE: This function makes use of the Huge Number scratch buffers.   */
/*          Each operand and the result must have the                     */
/*          nx_crypto_huge_number_scratch member set to point to a buffer */
/*          of size equal to the number data buffer and that size stored  */
/*          in the nx_crypto_huge_buffer_size member.                     */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Number being exponentiated    */
/*    exponent                              Exponent number               */
/*    modulus                               Modulus number                */
/*    result                                Result buffer                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_dh_compute_secret          Computes the Diffie-Hellman   */
/*                                            shared secret               */
/*    _nx_crypto_ec_point_setup             Set up point from byte steam  */
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_ecjpake_hello_process      Process hello message         */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
/*    _nx_crypto_ecjpake_key_exchange_process                             */
/*                                          Process key exchange message  */
/*    _nx_crypto_ecjpake_schnorr_zkp_hash   Perform Schnorr ZKP hash      */
/*                                            calculation                 */
/*    _nx_crypto_rsa_operation              Perform an RSA operation      */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_setup(NX_CRYPTO_HUGE_NUMBER *number, const UCHAR *byte_stream, UINT size)
{

UINT      i = 0;
HN_UBASE *destination;
HN_UBASE  word;
UINT      num_words;

    /* Remove leading zeros in the byte stream. */
    while (size > 0 && *byte_stream == 0)
    {
        size--;
        byte_stream++;
    }

    /* Handle the case when the huge number is zero. */
    if (size == 0)
    {
        if (number -> nx_crypto_huge_buffer_size < sizeof(HN_UBASE))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }

        number -> nx_crypto_huge_number_data[0] = 0;
        number -> nx_crypto_huge_number_size = 1;
        number -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

        return(NX_CRYPTO_SUCCESS);
    }

    num_words = (size + HN_SIZE_ROUND) >> HN_SIZE_SHIFT;

    /* Make sure the huge number buffer size is large enough to hold the value. */
    if (number -> nx_crypto_huge_buffer_size < (num_words << HN_SIZE_SHIFT))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    destination = number -> nx_crypto_huge_number_data + num_words - 1;


    if (size & HN_SIZE_ROUND)
    {
#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
        word = byte_stream[0];
        i++;
#else /* NX_CRYPTO_HUGE_NUMBER_BITS == 32 */
        if ((size & HN_SIZE_ROUND) == 1)
        {
            word = byte_stream[0];
            i++;
        }
        else if ((size & HN_SIZE_ROUND) == 2)
        {
            word = (HN_UBASE)((byte_stream[0] << 8) | byte_stream[1]);
            i += 2;
        }
        else
        {
            word = (HN_UBASE)((byte_stream[0] << 16) | (byte_stream[1] << 8) | byte_stream[2]);
            i += 3;
        }
#endif
        *destination = word;
        destination--;
    }

    for (; i < size; i += sizeof(HN_UBASE))
    {
#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
        word = (HN_UBASE)((byte_stream[i] << 8) | (byte_stream[i + 1]));
#else /* NX_CRYPTO_HUGE_NUMBER_BITS == 32 */
        word = (HN_UBASE)((byte_stream[i] << 24) |
                          (byte_stream[i + 1] << 16) |
                          (byte_stream[i + 2] << 8) |
                          (byte_stream[i + 3]));
#endif
        *destination = word;
        destination--;
    }

    number -> nx_crypto_huge_number_size = num_words;
    number -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_rbg                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates random huge number with specified bits.     */
/*    The result buffer must be large enough to hold random huge number.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    bits                                  Random number bits            */
/*    result                                Pointer to random number      */
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
/*    Application                                                         */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_rbg(UINT bits, UCHAR *result)
{
UCHAR *ptr = result;
UINT random_number;
UINT temp;
UINT mask;

    while (bits >= 32)
    {

        /* Generate random number for each 32 bits. */
        random_number = (UINT)NX_CRYPTO_RAND();
        ptr[0] = (UCHAR)(random_number & 0xFF);
        ptr[1] = (UCHAR)((random_number >> 8) & 0xFF);
        ptr[2] = (UCHAR)((random_number >> 16) & 0xFF);
        ptr[3] = (UCHAR)((random_number >> 24) & 0xFF);
        bits -= 32;
        ptr += 4;
    }

    if (bits == 0)
    {

        /* Every bits are generated. */
        return(NX_CRYPTO_SUCCESS);
    }

    /* Process left bits between 1 and 31. */
    random_number = (UINT)NX_CRYPTO_RAND();

    /* Get remaining bits. */
    temp = (bits + 7) >> 3;
    switch (temp)
    {
    case 4:
        *ptr++ = (UCHAR)(random_number & 0xFF);
        random_number >>= 8;
        /* fallthrough */
    case 3:
        *ptr++ = (UCHAR)(random_number & 0xFF);
        random_number >>= 8;
        /* fallthrough */
    case 2:
        *ptr++ = (UCHAR)(random_number & 0xFF);
        random_number >>= 8;
        /* fallthrough */
    case 1:
        *ptr++ = (UCHAR)(random_number & 0xFF);
        random_number >>= 8;
        /* fallthrough */
    default:
        break;
    }

    /* Zero out extra bits generated. */
    bits = bits & 7;
    if (bits)
    {
        mask = (UINT)((1 << bits) - 1);
        result[0] = (UCHAR)(result[0] & mask);
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_extract                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts huge number to buffer.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Huge number                   */
/*    byte_stream                           Output buffer                 */
/*    byte_stream_size                      Size of output buffer         */
/*    huge_number_size                      Size of huge number in bytes  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_dh_compute_secret          Computes the Diffie-Hellman   */
/*                                            shared secret               */
/*    _nx_crypto_dh_setup                   Set up Diffie-Hellman context */
/*    _nx_crypto_ec_point_extract_uncompressed                            */
/*                                          Extract point to byte stream  */
/*                                            in uncompressed format      */
/*    _nx_crypto_ec_secp192r1_reduce        Reduce the value of curve     */
/*                                            secp192r1                   */
/*    _nx_crypto_ec_secp224r1_reduce        Reduce the value of curve     */
/*                                            secp224r1                   */
/*    _nx_crypto_ec_secp256r1_reduce        Reduce the value of curve     */
/*                                            secp256r1                   */
/*    _nx_crypto_ec_secp384r1_reduce        Reduce the value of curve     */
/*                                            secp384r1                   */
/*    _nx_crypto_ec_secp521r1_reduce        Reduce the value of curve     */
/*                                            secp521r1                   */
/*    _nx_crypto_ecjpake_hello_generate     Generate hello message        */
/*    _nx_crypto_ecjpake_key_exchange_generate                            */
/*                                          Generate key exchange message */
/*    _nx_crypto_ecjpake_pre_master_secret_generate                       */
/*                                          Generate pre master secret    */
/*    _nx_crypto_rsa_operation              Perform an RSA operation      */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_extract(NX_CRYPTO_HUGE_NUMBER *number, UCHAR *byte_stream,
                                                   UINT byte_stream_size, UINT *huge_number_size)
{
INT       i = 0;
HN_UBASE *word;
HN_UBASE  value;
UCHAR    *bytes;


    if ((number -> nx_crypto_huge_number_size << HN_SIZE_SHIFT) > byte_stream_size)
    {
        /* User byte stream buffer too small. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    bytes = byte_stream;

    word = number -> nx_crypto_huge_number_data;
    for (i = (INT)(number -> nx_crypto_huge_number_size - 1); i >= 0; i--)
    {
        value = word[i];
#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
        *bytes = value >> 8;
        *(bytes + 1) = value & 0xFF;
#else /* NX_CRYPTO_HUGE_NUMBER_BITS == 32 */
        *bytes = (UCHAR)(value >> 24);
        *(bytes + 1) = (value >> 16) & 0xFF;
        *(bytes + 2) = (value >> 8) & 0xFF;
        *(bytes + 3) = value & 0xFF;
#endif
        bytes += sizeof(HN_UBASE);
    }
    *huge_number_size = (number -> nx_crypto_huge_number_size << HN_SIZE_SHIFT);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_extract_fixed_size           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts huge number to a buffer with fixed size.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Huge number                   */
/*    byte_stream                           Output buffer                 */
/*    byte_stream_size                      Size of output buffer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ecdsa_sign                 Sign hash data using ECDSA.   */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_extract_fixed_size(NX_CRYPTO_HUGE_NUMBER *number,
                                                              UCHAR *byte_stream, UINT byte_stream_size)
{
INT       i = 0;
HN_UBASE *word;
HN_UBASE  value;
UCHAR    *bytes;
UINT      leading_count = 0;
UINT      num_size_adjusted;

    word = number -> nx_crypto_huge_number_data;

    num_size_adjusted = number -> nx_crypto_huge_number_size;
    do
    {
        num_size_adjusted--;
        value = word[num_size_adjusted];
    } while (value == 0 && num_size_adjusted > 0);
    num_size_adjusted++;

#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
    if ((value & 0xFF00) == 0)
    {
        leading_count = 1;
    }
#else
    if ((value & 0xFFFFFF00) == 0)
    {
        leading_count = 3;
    }
    else if ((value & 0xFFFF0000) == 0)
    {
        leading_count = 2;
    }
    else if ((value & 0xFF000000) == 0)
    {
        leading_count = 1;
    }
#endif

    if ((num_size_adjusted << HN_SIZE_SHIFT) - leading_count > byte_stream_size)
    {
        /* User byte stream buffer too small. */
        return(NX_CRYPTO_SIZE_ERROR);
    }

    byte_stream_size -= (num_size_adjusted << HN_SIZE_SHIFT) - leading_count;

    if (byte_stream_size > 0)
    {
        NX_CRYPTO_MEMSET(byte_stream, 0, byte_stream_size);
    }

    bytes = byte_stream + byte_stream_size;

#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
    if (leading_count == 0)
    {
        *bytes = (UCHAR)(value >> 8);
        bytes++;
    }
#else
    if (leading_count == 0)
    {
        *bytes = (UCHAR)(value >> 24);
        bytes++;
    }
    if (leading_count <= 1)
    {
        *bytes = (UCHAR)((value >> 16) & 0xFF);
        bytes++;
    }
    if (leading_count <= 2)
    {
        *bytes = (UCHAR)((value >> 8) & 0xFF);
        bytes++;
    }
#endif

    *bytes = (UCHAR)(value & 0xFF);
    bytes++;

    for (i = (INT)(num_size_adjusted - 2); i >= 0; i--)
    {
        value = word[i];
#if (NX_CRYPTO_HUGE_NUMBER_BITS == 16)
        *bytes = value >> 8;
        *(bytes + 1) = value & 0xFF;
#else /* NX_CRYPTO_HUGE_NUMBER_BITS == 32 */
        *bytes = (UCHAR)(value >> 24);
        *(bytes + 1) = (value >> 16) & 0xFF;
        *(bytes + 2) = (value >> 8) & 0xFF;
        *(bytes + 3) = value & 0xFF;
#endif
        bytes += sizeof(HN_UBASE);
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an inverse modulus operation. The size of    */
/*    scratch is required to be larger than 4 * (max(len(p), len(a)) + 1) */
/*    times of size of radix. p must be prime number.                     */
/*                  r = a ^ (-1) mod p                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    a                                     Huge number a                 */
/*    p                                     Huge number modulo p          */
/*    r                                     Huge number r                 */
/*    scratch                               Buffer used to hold           */
/*                                            intermediate data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_is_zero        Check if number is zero or not*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_huge_number_shift_right    Shift right for huge number   */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_ec_fp_affine_add           Perform addition for points of*/
/*                                            affine                      */
/*    _nx_crypto_ec_point_fp_projective_to_affine                         */
/*                                          Convert point from projective */
/*                                            to affine                   */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
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
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_inverse_modulus_prime(NX_CRYPTO_HUGE_NUMBER *a,
                                                                 NX_CRYPTO_HUGE_NUMBER *p,
                                                                 NX_CRYPTO_HUGE_NUMBER *r,
                                                                 HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER u, v, A, C;
UINT                  buffer_size;

    if (p -> nx_crypto_huge_number_size > a -> nx_crypto_huge_number_size)
    {
        buffer_size = (p -> nx_crypto_huge_number_size + 1) << HN_SIZE_SHIFT;
    }
    else
    {
        buffer_size = (a -> nx_crypto_huge_number_size + 1) << HN_SIZE_SHIFT;
    }

    /* Buffer usage: 6 * buffer_size */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&u, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&v, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&A, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&C, scratch, buffer_size);

    NX_CRYPTO_HUGE_NUMBER_COPY(&u, a);
    NX_CRYPTO_HUGE_NUMBER_COPY(&v, p);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&A, 1);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&C, 0);

    while (!_nx_crypto_huge_number_is_zero(&u))
    {
        while (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&u))
        {
            _nx_crypto_huge_number_shift_right(&u, 1);
            if (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&A))
            {
                _nx_crypto_huge_number_shift_right(&A, 1);
            }
            else
            {
                _nx_crypto_huge_number_add(&A, p);
                _nx_crypto_huge_number_shift_right(&A, 1);
            }
        }

        while (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&v))
        {
            _nx_crypto_huge_number_shift_right(&v, 1);
            if (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&C))
            {
                _nx_crypto_huge_number_shift_right(&C, 1);
            }
            else
            {
                _nx_crypto_huge_number_add(&C, p);
                _nx_crypto_huge_number_shift_right(&C, 1);
            }
        }

        if (_nx_crypto_huge_number_compare(&u, &v) != NX_CRYPTO_HUGE_NUMBER_LESS)
        {
            _nx_crypto_huge_number_subtract(&u, &v);
            _nx_crypto_huge_number_subtract(&A, &C);
        }
        else
        {
            _nx_crypto_huge_number_subtract(&v, &u);
            _nx_crypto_huge_number_subtract(&C, &A);
        }
    }

    _nx_crypto_huge_number_modulus(&C, p);
    NX_CRYPTO_HUGE_NUMBER_COPY(r, &C);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_inverse_modulus              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an inverse modulus operation. The size of    */
/*    scratch is required to be larger than 6 * (max(len(m), len(a)) + 1) */
/*    times of size of radix.                                             */
/*                  r = a ^ (-1) mod m                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    a                                     Huge number a                 */
/*    m                                     Huge number modulo            */
/*    r                                     Huge number r                 */
/*    scratch                               Buffer used to hold           */
/*                                            intermediate data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_is_zero        Check if number is zero or not*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_huge_number_shift_right    Shift right for huge number   */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed input validation,     */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_huge_number_inverse_modulus(NX_CRYPTO_HUGE_NUMBER *a,
                                                           NX_CRYPTO_HUGE_NUMBER *m,
                                                           NX_CRYPTO_HUGE_NUMBER *r,
                                                           HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER u, v, A, B, C, D;
UINT                  buffer_size;

    if (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(m) && NX_CRYPTO_HUGE_NUMBER_IS_EVEN(a))
    {

        /* gcd(m, a) <> 1.
         * a is not invertible modulo m. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if(_nx_crypto_huge_number_is_zero(a))
    {

        /* zero is not invertible */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (m -> nx_crypto_huge_number_size > a -> nx_crypto_huge_number_size)
    {
        buffer_size = (m -> nx_crypto_huge_number_size + 1) << HN_SIZE_SHIFT;
    }
    else
    {
        buffer_size = (a -> nx_crypto_huge_number_size + 1) << HN_SIZE_SHIFT;
    }

    /* Buffer usage: 6 * buffer_size */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&u, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&v, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&A, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&B, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&C, scratch, buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&D, scratch, buffer_size);

    NX_CRYPTO_HUGE_NUMBER_COPY(&u, m);
    NX_CRYPTO_HUGE_NUMBER_COPY(&v, a);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&A, 1);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&B, 0);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&C, 0);
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(&D, 1);

    for (;;)
    {
        while (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&u))
        {
            _nx_crypto_huge_number_shift_right(&u, 1);

            if (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&A) && NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&B))
            {
                _nx_crypto_huge_number_shift_right(&A, 1);
                _nx_crypto_huge_number_shift_right(&B, 1);
            }
            else
            {
                _nx_crypto_huge_number_add(&A, a);
                _nx_crypto_huge_number_shift_right(&A, 1);
                _nx_crypto_huge_number_subtract(&B, m);
                _nx_crypto_huge_number_shift_right(&B, 1);
            }
        }

        while (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&v))
        {
            _nx_crypto_huge_number_shift_right(&v, 1);
            if (NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&C) && NX_CRYPTO_HUGE_NUMBER_IS_EVEN(&D))
            {
                _nx_crypto_huge_number_shift_right(&C, 1);
                _nx_crypto_huge_number_shift_right(&D, 1);
            }
            else
            {
                _nx_crypto_huge_number_add(&C, a);
                _nx_crypto_huge_number_shift_right(&C, 1);
                _nx_crypto_huge_number_subtract(&D, m);
                _nx_crypto_huge_number_shift_right(&D, 1);
            }
        }

        if (_nx_crypto_huge_number_compare(&u, &v) != NX_CRYPTO_HUGE_NUMBER_LESS)
        {
            _nx_crypto_huge_number_subtract(&u, &v);
            _nx_crypto_huge_number_subtract(&A, &C);
            _nx_crypto_huge_number_subtract(&B, &D);
        }
        else
        {
            _nx_crypto_huge_number_subtract(&v, &u);
            _nx_crypto_huge_number_subtract(&C, &A);
            _nx_crypto_huge_number_subtract(&D, &B);
        }

        if (_nx_crypto_huge_number_is_zero(&u))
        {
            _nx_crypto_huge_number_modulus(&D, m);
            NX_CRYPTO_HUGE_NUMBER_COPY(r, &D);
            break;
        }
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_mont                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs Montgomery reduction for multiplication.     */
/*                  r = (x * y) * R ^ (-1) mod m                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    m                                     Huge number m                 */
/*    mi                                    mi = -m ^ (-1) mod radix      */
/*    x                                     Huge number x                 */
/*    y                                     Huge number y                 */
/*    result                                Huge number r                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*    _nx_crypto_huge_number_compare        Compare two huge numbers      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_mont(NX_CRYPTO_HUGE_NUMBER *m, UINT mi,
                                                NX_CRYPTO_HUGE_NUMBER *x,
                                                NX_CRYPTO_HUGE_NUMBER *y,
                                                NX_CRYPTO_HUGE_NUMBER *result)
{
UINT      i, j;
HN_UBASE  u;
HN_UBASE  xi;
HN_UBASE2 product;
UINT      m_len = m -> nx_crypto_huge_number_size;
UINT      x_len = x -> nx_crypto_huge_number_size;
UINT      y_len = y -> nx_crypto_huge_number_size;
HN_UBASE *m_buffer = m -> nx_crypto_huge_number_data;
HN_UBASE *x_buffer = x -> nx_crypto_huge_number_data;
HN_UBASE *y_buffer = y -> nx_crypto_huge_number_data;
HN_UBASE *result_buffer = result -> nx_crypto_huge_number_data;

    NX_CRYPTO_MEMSET(result -> nx_crypto_huge_number_data, 0, (m_len + 1) * sizeof(HN_UBASE));

    for (i = 0; i < x_len; i++)
    {

        xi = x_buffer[i];

        /* r = (r + x[i] * y + u * m) / radix */
        product = 0;
        for (j = 0; j < y_len; j++)
        {
            product >>= HN_SHIFT;
            product += result_buffer[j] + (HN_UBASE2)xi * y_buffer[j];
            result_buffer[j] = (product & HN_MASK);
        }
        for (; j < (m_len + 1); j++)
        {
            product >>= HN_SHIFT;
            product += result_buffer[j];
            result_buffer[j] = (product & HN_MASK);
        }

        /* u = (r[0] + x[i] * y[0]) * mi mod radix */
        u = result_buffer[0] * mi;

        product = result_buffer[0] + (HN_UBASE2)u * m_buffer[0];
        for (j = 1; j < m_len; j++)
        {
            product >>= HN_SHIFT;
            product += result_buffer[j] + (HN_UBASE2)u * m_buffer[j];
            result_buffer[j - 1] = (product & HN_MASK);
        }
        product >>= HN_SHIFT;
        product += result_buffer[j];
        result_buffer[j - 1] = (product & HN_MASK);
        result_buffer[j] = (HN_UBASE)(product >> HN_SHIFT);
    }

    for (; i < m_len; i++)
    {

        /* u = (r[0] + x[i] * y[0]) * mi mod radix */
        u = ((result_buffer[0] * mi) & HN_MASK);

        /* r = (r + x[i] * y + u * m) / radix */
        product = result_buffer[0] + (HN_UBASE2)u * m_buffer[0];
        for (j = 1; j < m_len; j++)
        {
            product >>= HN_SHIFT;
            product += result_buffer[j] + (HN_UBASE2)u * m_buffer[j];
            result_buffer[j - 1] = (product & HN_MASK);
        }
        product >>= HN_SHIFT;
        product += result_buffer[j];
        result_buffer[j - 1] = (product & HN_MASK);
        result_buffer[j] = (HN_UBASE)(product >> HN_SHIFT);
    }

    /* Set result size. */
    result -> nx_crypto_huge_number_size = m_len + 1;
    _nx_crypto_huge_number_adjust_size(result);

    if (_nx_crypto_huge_number_compare(result, m) != NX_CRYPTO_HUGE_NUMBER_LESS)
    {

        /* r = r - m. */
        _nx_crypto_huge_number_subtract(result, m);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_mont_power_modulus           PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function raises a huge number to the power of a second huge    */
/*    number using a third huge number as a modulus. The result is placed */
/*    in a fourth huge number. Montgomery reduction is used.              */
/*    scratch is required to be larger than twice of buffer size of m     */
/*    plus 8 bytes.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Number being exponentiated    */
/*    e                                     Exponent number               */
/*    m                                     Modulus number                */
/*    result                                Result buffer                 */
/*    scratch                               Buffer used to hold           */
/*                                            intermediate data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Operation result status       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_adjust_size    Adjust the size of a huge     */
/*                                            number to remove leading    */
/*                                            zeroes                      */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_inverse_modulus                              */
/*                                          Perform an inverse modulus    */
/*                                            operation                   */
/*    _nx_crypto_huge_number_mont           Perform Montgomery reduction  */
/*                                            for multiplication          */
/*    _nx_crypto_huge_number_square         Compute the square of a value */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_dh_compute_secret          Computes the Diffie-Hellman   */
/*                                            shared secret               */
/*    _nx_crypto_dh_setup                   Set up Diffie-Hellman context */
/*    _nx_crypto_huge_number_crt_power_modulus                            */
/*                                          Raise a huge number for CRT   */
/*    _nx_crypto_rsa_operation              Perform an RSA operation      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_mont_power_modulus(NX_CRYPTO_HUGE_NUMBER *x,
                                                              NX_CRYPTO_HUGE_NUMBER *e,
                                                              NX_CRYPTO_HUGE_NUMBER *m,
                                                              NX_CRYPTO_HUGE_NUMBER *result,
                                                              HN_UBASE *scratch)
{
UINT                   m_len;
NX_CRYPTO_HUGE_NUMBER  xx;
NX_CRYPTO_HUGE_NUMBER  temp;
NX_CRYPTO_HUGE_NUMBER  digit;
NX_CRYPTO_HUGE_NUMBER  radix;
NX_CRYPTO_HUGE_NUMBER  mi;
NX_CRYPTO_HUGE_NUMBER  m0;
HN_UBASE               digit_value;
HN_UBASE              *val;
HN_UBASE               radix_buffer[2] = {0, 1};
HN_UBASE               mm_buffer[2];
HN_UBASE               cur_block;
UINT                   bit, exp_size;
NX_CRYPTO_HUGE_NUMBER *operand, *temp_result, *temp_swap;

    /* Adjust sizes before performing the calculation. */
    _nx_crypto_huge_number_adjust_size(x);
    _nx_crypto_huge_number_adjust_size(e);
    _nx_crypto_huge_number_adjust_size(m);

    /* Initialize const huge numbers. */
    radix.nx_crypto_huge_number_data = radix_buffer;
    radix.nx_crypto_huge_number_size = 2;
    radix.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    radix.nx_crypto_huge_buffer_size = sizeof(radix_buffer);
    m0.nx_crypto_huge_number_data = m -> nx_crypto_huge_number_data;
    m0.nx_crypto_huge_number_size = 1;
    m0.nx_crypto_huge_buffer_size = 4;
    m0.nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
    mi.nx_crypto_huge_number_data = mm_buffer;
    mi.nx_crypto_huge_buffer_size = sizeof(mm_buffer);

    /* mi = -m^(-1) mod radix */
    _nx_crypto_huge_number_inverse_modulus(&m0, &radix, &mi, scratch);
    mm_buffer[0] = (HN_UBASE)(HN_RADIX - mm_buffer[0]);

    /* Set buffers. */
    /* Buffer usage: 2 * buffer_size of m + 8 */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&xx, scratch, m -> nx_crypto_huge_buffer_size + sizeof(HN_UBASE));
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch, m -> nx_crypto_huge_buffer_size + sizeof(HN_UBASE));
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE_DIGIT(&digit, &digit_value, 1);


    /* x' = radix ^ m_len mod m */
    m_len = m -> nx_crypto_huge_number_size;
    val = temp.nx_crypto_huge_number_data;
    NX_CRYPTO_MEMSET(val, 0, (m_len << HN_SIZE_SHIFT));
    temp.nx_crypto_huge_number_size = m_len + 1;
    val[m_len] = 1;
    _nx_crypto_huge_number_modulus(&temp, m);

    /* xx = mont(x, radix ^ (2 * m_len) mod m)*/
    _nx_crypto_huge_number_square(&temp, result);
    _nx_crypto_huge_number_modulus(result, m);
    _nx_crypto_huge_number_mont(m, mm_buffer[0], x, result, &xx);

    /* result = x' */
    NX_CRYPTO_HUGE_NUMBER_COPY(result, &temp);

    /* Clear out the result since we need it to start with a simple value. */

    operand = result;
    temp_result = &temp;

    exp_size = e -> nx_crypto_huge_number_size;
    val = e -> nx_crypto_huge_number_data + (exp_size - 1);

    /* Loop through the bits of the exponent. For each bit set, multiply the result by the running square. */
    for (; (ULONG)val >= (ULONG)(e -> nx_crypto_huge_number_data); val--)
    {
        /* Current byte in the exponent determines whether we multiply or not. */
        cur_block = *val;

        /* Loop over the bits in the current byte to see whether we add or not. */
        for (bit = 0; bit < HN_SHIFT; bit++)
        {

            /* A non-zero bit means we need to multiply. */
            if (cur_block & (1u << (NX_CRYPTO_HUGE_NUMBER_BITS - (1 + bit))))
            {

                /* result = mont(result, result) */
                _nx_crypto_huge_number_mont(m, mm_buffer[0], operand, operand, temp_result);

                /* result = mont(result, xx) */
                _nx_crypto_huge_number_mont(m, mm_buffer[0], temp_result, &xx, operand);
            }
            else
            {

                /* result = mont(result, result) */
                _nx_crypto_huge_number_mont(m, mm_buffer[0], operand, operand, temp_result);
                temp_swap = temp_result;
                temp_result = operand;
                operand = temp_swap;
            }
        }
    }

    /* result = mont(result, 1) */
    _nx_crypto_huge_number_mont(m, mm_buffer[0], &digit, operand, temp_result);
    if (temp_result != result)
    {
        NX_CRYPTO_HUGE_NUMBER_COPY(result, &temp);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_crt_power_modulus            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function raises a huge number to the power of a second huge    */
/*    number using a fifth huge number as a modulus. The result is placed */
/*    in a sixth huge number. Montgomery reduction and Chinese Remainder  */
/*    Theorem are used.                                                   */
/*                                                                        */
/*    Requirement:                                                        */
/*      1. m = p * q                                                      */
/*      2. p and q are primes                                             */
/*      3. scratch is required to be no less than 4 times of buffer size  */
/*    of m plus 24 bytes.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    x                                     Number being exponentiated    */
/*    e                                     Exponent number               */
/*    p                                     Prime number p                */
/*    q                                     Prime number q                */
/*    m                                     Modulus number                */
/*    result                                Result buffer                 */
/*    scratch                               Buffer used to hold           */
/*                                            intermediate data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_HUGE_NUMBER_COPY            Copy huge number              */
/*    NX_CRYPTO_HUGE_NUMBER_INITIALIZE      Initialize the buffer of      */
/*                                            huge number                 */
/*    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT       Set value of huge number      */
/*                                            between 0 and (HN_RADIX - 1)*/
/*    _nx_crypto_huge_number_add            Calculate addition for        */
/*                                            huge numbers                */
/*    _nx_crypto_huge_number_subtract       Calculate subtraction for     */
/*                                             huge numbers               */
/*    _nx_crypto_huge_number_multiply       Multiply two huge numbers     */
/*    _nx_crypto_huge_number_modulus        Perform a modulus operation   */
/*    _nx_crypto_huge_number_inverse_modulus_prime                        */
/*                                          Perform an inverse modulus    */
/*                                            operation for prime number  */
/*    _nx_crypto_huge_number_mont_power_modulus                           */
/*                                          Raise a huge number for       */
/*                                            montgomery reduction        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_rsa_operation              Perform an RSA operation      */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_crt_power_modulus(NX_CRYPTO_HUGE_NUMBER *x,
                                                             NX_CRYPTO_HUGE_NUMBER *e,
                                                             NX_CRYPTO_HUGE_NUMBER *p,
                                                             NX_CRYPTO_HUGE_NUMBER *q,
                                                             NX_CRYPTO_HUGE_NUMBER *m,
                                                             NX_CRYPTO_HUGE_NUMBER *result,
                                                             HN_UBASE *scratch)
{
NX_CRYPTO_HUGE_NUMBER *ep, *eq, *xp, *xq, *m1, *m2;
NX_CRYPTO_HUGE_NUMBER  pi, qi;
NX_CRYPTO_HUGE_NUMBER  temp1, temp2, temp3;
NX_CRYPTO_HUGE_NUMBER  digit;
HN_UBASE               digit_value;

    /*****************************************
     * ep = e mod (p - 1)
     * xp = x mod p
     * m1 = xp ^ ep mod p
     *
     * eq = e mod (q - 1)
     * xq = x mod q
     * m2 = xq ^ eq mod q
     *
     * pi = p ^ (-1) mod q
     * qi = q ^ (-1) mod p
     *
     * r = (m1 * qi * q + m2 * pi * p) mod m
     ****************************************/

    /* Buffer usage: 1 * buffer size of m */
    /* temp1 is used to hold the result of pi * p * m2. So twice of m buffer size is used.  */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&pi, scratch, p -> nx_crypto_huge_buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&qi, scratch, q -> nx_crypto_huge_buffer_size);

    /* qi = q ^ (-1) mod p */
    /* pi = p ^ (-1) mod q */
    /* Buffer usage: 2 * buffer_size of m + 24 bytes */
    _nx_crypto_huge_number_inverse_modulus_prime(q, p, &qi, scratch);
    _nx_crypto_huge_number_inverse_modulus_prime(p, q, &pi, scratch);

    /* Buffer usage: 2 * buffer size of m */
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp1, scratch, m -> nx_crypto_huge_buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp2, scratch, p -> nx_crypto_huge_buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp3, scratch, p -> nx_crypto_huge_buffer_size);
    NX_CRYPTO_HUGE_NUMBER_INITIALIZE_DIGIT(&digit, &digit_value, 1);

    /* In the following calculation, buffer of temp3 is used by temp2. */
    /* ep = e mod (p - 1) */
    ep = &temp2;
    NX_CRYPTO_HUGE_NUMBER_COPY(ep, e);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp1, p);
    _nx_crypto_huge_number_subtract(&temp1, &digit);
    _nx_crypto_huge_number_modulus(ep, &temp1);

    /* xp = x mod p */
    xp = &temp3;
    NX_CRYPTO_HUGE_NUMBER_COPY(xp, x);
    _nx_crypto_huge_number_modulus(xp, p);

    /* m1 = xp ^ ep mod p */
    m1 = &temp1;

    /* Buffer usage: 1 * buffer_size of m + 8 bytes */
    _nx_crypto_huge_number_mont_power_modulus(xp, ep, p, m1, scratch);

    /* m1 * qi * q */
    _nx_crypto_huge_number_multiply(&qi, m1, &temp3);
    _nx_crypto_huge_number_multiply(&temp3, q, result);


    /* In the following calculation, buffer of temp3 is used by temp2. */
    /* eq = e mod (q - 1) */
    eq = &temp2;
    NX_CRYPTO_HUGE_NUMBER_COPY(eq, e);
    NX_CRYPTO_HUGE_NUMBER_COPY(&temp1, q);
    _nx_crypto_huge_number_subtract(&temp1, &digit);
    _nx_crypto_huge_number_modulus(eq, &temp1);

    /* xq = x mod q */
    xq = &temp3;
    NX_CRYPTO_HUGE_NUMBER_COPY(xq, x);
    _nx_crypto_huge_number_modulus(xq, q);

    /* m2 = xq ^ eq mod q */
    m2 = &temp1;

    /* Buffer usage: 1 * buffer_size of m + 8 bytes */
    _nx_crypto_huge_number_mont_power_modulus(xq, eq, q, m2, scratch);

    /* pi * p * m2 */
    _nx_crypto_huge_number_multiply(&pi, m2, &temp3);

    /* Buffer of temp2 is not used anymore. It is used by temp1.
     * So buffer length of temp1 is 1.5 * buffer length of m.
     * It is large enough to hold the result of pi * p * m2 */
    _nx_crypto_huge_number_multiply(p, &temp3, &temp1);


    /* r = (m1 * qi * q + m2 * pi * p) mod m */
    _nx_crypto_huge_number_add(result, &temp1);
    _nx_crypto_huge_number_modulus(result, m);
}

