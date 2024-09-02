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
#include "nx_crypto_huge_number.h"

#ifndef NX_CRYPTO_SELF_TEST
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_add_digit                    PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates addition for huge number and digit.        */
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
/*    _nx_crypto_huge_number_add_digit_unsigned                           */
/*                                          Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_digit_unsigned                      */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_add_digit(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit)
{
    if (value -> nx_crypto_huge_number_is_negative)
    {
        if ((value -> nx_crypto_huge_number_size > 1) ||
            (value -> nx_crypto_huge_number_data[0] >= digit))
        {

            /* |value| >= digit */
            /* value = -(|value| - digit) */
            _nx_crypto_huge_number_subtract_digit_unsigned(value, digit);
        }
        else
        {

            /* value < digit */
            /* value = digit - |value| */
            value -> nx_crypto_huge_number_data[0] = digit - value -> nx_crypto_huge_number_data[0];
            value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_FALSE;
        }
    }
    else
    {

        /* value = value + digit */
        _nx_crypto_huge_number_add_digit_unsigned(value, digit);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_subtract_digit               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates subtraction for huge number and digit.     */
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
/*    _nx_crypto_huge_number_add_digit_unsigned                           */
/*                                          Calculate addition for        */
/*                                            unsigned huge numbers       */
/*    _nx_crypto_huge_number_subtract_digit_unsigned                      */
/*                                          Calculate subtraction for     */
/*                                            unsigned huge numbers       */
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
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_subtract_digit(NX_CRYPTO_HUGE_NUMBER *value, HN_UBASE digit)
{
    if (value -> nx_crypto_huge_number_is_negative)
    {

        /* value = -(|value| + digit) */
        _nx_crypto_huge_number_add_digit_unsigned(value, digit);
    }
    else
    {
        if ((value -> nx_crypto_huge_number_size > 1) ||
            (value -> nx_crypto_huge_number_data[0] >= digit))
        {

            /* value >= digit */
            /* value = value - digit */
            _nx_crypto_huge_number_subtract_digit_unsigned(value, digit);
        }
        else
        {

            /* value < digit */
            /* value = -(digit - value) */
            value -> nx_crypto_huge_number_data[0] = digit - value -> nx_crypto_huge_number_data[0];
            value -> nx_crypto_huge_number_is_negative = NX_CRYPTO_TRUE;
        }
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_huge_number_power_modulus                PORTABLE C      */
/*                                                           6.1.9        */
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
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Number being exponentiated    */
/*    exponent                              Exponent number               */
/*    modulus                               Modulus number                */
/*    result                                Result buffer                 */
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
/*    _nx_crypto_huge_number_multiply       Multiply a huge number by     */
/*                                          a second huge number          */
/*    _nx_crypto_huge_number_modulus        Calculate the remainder after */
/*                                          dividing a huge number by     */
/*                                          another huge number           */
/*    _nx_crypto_huge_number_square         Compute the square of a value */
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
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID _nx_crypto_huge_number_power_modulus(NX_CRYPTO_HUGE_NUMBER *number,
                                                         NX_CRYPTO_HUGE_NUMBER *exponent,
                                                         NX_CRYPTO_HUGE_NUMBER *modulus,
                                                         NX_CRYPTO_HUGE_NUMBER *result,
                                                         HN_UBASE *scratch)
{
UINT                  index, bit;
HN_UBASE              cur_block;
HN_UBASE             *val;
UINT                  exp_size;
NX_CRYPTO_HUGE_NUMBER temp;

    NX_CRYPTO_HUGE_NUMBER_INITIALIZE(&temp, scratch,
                                     number -> nx_crypto_huge_buffer_size);

    /* Clear out the result since we need it to start with a simple value. */
    NX_CRYPTO_HUGE_NUMBER_SET_DIGIT(result, 1);

    exp_size = exponent -> nx_crypto_huge_number_size;
    val = exponent -> nx_crypto_huge_number_data;
    /* Loop through the bits of the exponent. For each bit set, multiply the result by the running square. */
    for (index = 0; index < exp_size; index++)
    {
        /* Current byte in the exponent determines whether we multiply or not. */
        cur_block = val[index];

        /* Loop over the bits in the current byte to see whether we add or not. */
        for (bit = 0; bit < HN_SHIFT; ++bit)
        {
            /* A non-zero bit means we need to multiply. */
            if (cur_block & 1)
            {
                /* Multiply the result by the running square (number) and put the result in a scratch buffer. */
                _nx_crypto_huge_number_multiply(result, number, &temp);
                NX_CRYPTO_HUGE_NUMBER_COPY(result, &temp);

                /* Take the modulus of the product we just calculated. If we didn't do this here the result would quickly grow
                   larger than our buffers (or the universe) could store. Put the result in our result buffer. */
                _nx_crypto_huge_number_modulus(result, modulus);
            }

            /* Shift the data to go to the next bit. */
            cur_block = cur_block >> 1;

            _nx_crypto_huge_number_square(number, &temp);
            _nx_crypto_huge_number_modulus(&temp, modulus);
            NX_CRYPTO_HUGE_NUMBER_COPY(number, &temp);

            /* Take the modulus of our squared product and put the result into our number buffer. */
        }
    }
}

#endif
