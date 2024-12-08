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
/** NetX Component                                                        */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "nx_api.h"

/* Define the base64 letters.  */
static CHAR _nx_utility_base64_array[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_string_length_check                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function traverses the string and returns the string length,   */
/*    if the string is invalid or the string length is bigger than max    */
/*    string length, returns error.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input_string                          Pointer to input string       */
/*    string_length                         Pointer to string length      */
/*    max_string_length                     Max string length             */
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_utility_string_length_check(CHAR *input_string, UINT *string_length, UINT max_string_length)
{

UINT    i;


    /* Check for invalid input pointers.  */
    if (input_string == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Traverse the string.  */
    for (i = 0; input_string[i]; i++)
    {

        /* Check if the string length is bigger than the max string length.  */
        if (i >= max_string_length)
        {
            return(NX_SIZE_ERROR);
        }
    }

    /* Return the string length if string_length is not NULL.
       String_length being NULL indicates the caller needs to check for string 
       length within the max_string_length. */
    if (string_length)
    {
        *string_length = i;
    }

    /* Return success.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_string_to_uint                          PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts the string to unsigned integer.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    input_string                          Pointer to input string       */
/*    string_length                         Length of input string        */
/*    number                                Pointer to the number         */
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
/*  12-31-2020     Yuxin Zhou               Initial Version 6.1.3         */
/*                                                                        */
/**************************************************************************/
UINT  _nx_utility_string_to_uint(CHAR *input_string, UINT string_length, UINT *number)
{

UINT i;


    /* Check for invalid input pointers.  */
    if ((input_string == NX_NULL) || (number == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check string length.  */
    if (string_length == 0)
    {
        return(NX_SIZE_ERROR);
    }

    /* Initialize.  */
    i = 0;
    *number = 0;

    /* Traverse the string.  */
    while (i < string_length)
    {

        /* Is a numeric character present?  */
        if ((input_string[i] >= '0') && (input_string[i] <= '9'))
        {

            /* Check overflow. Max Value: Hex:0xFFFFFFFF, Decimal: 4294967295.  */
            if (((*number == 429496729) && (input_string[i] > '5')) ||
                (*number >= 429496730))
            {
                return(NX_OVERFLOW);
            }

            /* Yes, numeric character is present. Update the number.  */
            *number =  (*number * 10) + (UINT) (input_string[i] - '0');
        }
        else
        {
            return(NX_INVALID_PARAMETERS);
        }

        i++;
    }

    /* Return success.  */
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_uint_to_string                          PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts the unsigned integer to string.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    number                                Input number                  */
/*    base                                  Base of the conversion        */
/*                                           8 for OCT                    */
/*                                           10 for DEC                   */
/*                                           16 for HEX                   */
/*    string_buffer                         Pointer to string buffer      */
/*    string_buffer_size                    Size of string buffer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    size                                  The size of output string     */
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
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            checked invalid input value,*/
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT _nx_utility_uint_to_string(UINT number, UINT base, CHAR *string_buffer, UINT string_buffer_size)
{
UINT i;
UINT digit;
UINT size;

    /* Check for invalid input pointers.  */
    if ((string_buffer == NX_NULL) || (string_buffer_size == 0) || (base == 0))
    {
        return(0);
    }

    /* Initialize.  */
    i = 0;
    size = 0;

    /* Loop to convert the number to ASCII. Minus 1 to put NULL terminal.  */
    while (size < string_buffer_size - 1)
    {

        /* Shift the current digits over one.  */
        for (i = size; i != 0; i--)
        {

            /* Move each digit over one place.  */
            string_buffer[i] = string_buffer[i-1];
        }

        /* Compute the next decimal digit.  */
        digit = number % base;

        /* Update the input number.  */
        number = number / base;

        /* Store the new digit in ASCII form.  */
        if (digit < 10)
        {
            string_buffer[0] = (CHAR) (digit + '0');
        }
        else
        {
            string_buffer[0] = (CHAR) (digit + 'a' - 0xa);
        }

        /* Increment the size.  */
        size++;

        /* Determine if the number is now zero.  */
        if (number == 0)
            break;
    }

    /* Determine if there is an overflow error.  */
    if (number)
    {

        /* Error, return bad values to user.  */
        size = 0;
    }

    /* Make the string NULL terminated.  */
    string_buffer[size] = (CHAR) NX_NULL;

    /* Return size to caller.  */
    return(size);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_base64_encode                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encodes the input string into a base64                */
/*    representation.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                                  Name string                   */
/*    name_size                             Size of name                  */
/*    base64name                            Encoded base64 name string    */
/*    base64name_size                       Size of encoded base64 name   */
/*    bytes_copied                          Number of bytes copied        */
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
/*  04-02-2021     Yuxin Zhou               Initial Version 6.1.6         */
/*  10-31-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            improved the internal logic,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_utility_base64_encode(UCHAR *name, UINT name_size, UCHAR *base64name, UINT base64name_size, UINT *bytes_copied)
{
UINT    pad;
UINT    i, j;
UINT    step;
UINT    input_name_size = name_size;


    /* Check for invalid input pointers.  */
    if ((name == NX_NULL) || (base64name == NX_NULL) || (bytes_copied == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check the size.  */
    if ((name_size == 0) || (base64name_size == 0))
    {
        return(NX_SIZE_ERROR);
    }

    /* Adjust the length to represent the base64 name.  */
    name_size = ((name_size * 8) / 6);

    /* Default padding to none.  */
    pad = 0;

    /* Determine if an extra conversion is needed.  */
    if ((name_size * 6) % 24)
    {

        /* Some padding is needed.  */

        /* Calculate the number of pad characters.  */
        pad = (name_size * 6) % 24;
        pad = (24 - pad) / 6;
        pad = pad - 1;

        /* Adjust the length to pickup the character fraction.  */
        name_size++;
    }

    /* Check the buffer size.  */
    if (base64name_size <= (name_size + pad))
    {
        return(NX_SIZE_ERROR);
    }

    /* Setup index into the base64name.  */
    j = 0;

    /* Compute the base64name.  */
    step = 0;
    i = 0;
    while (j < name_size)
    {

        /* Determine which step we are in.  */
        if (step == 0)
        {

            /* Use first 6 bits of name character for index.  */
            base64name[j++] = (UCHAR)_nx_utility_base64_array[((UCHAR)name[i]) >> 2];
            step++;
        }
        else if (step == 1)
        {

            /* Use last 2 bits of name character and first 4 bits of next name character for index.  */
            if ((i + 1) < input_name_size)
            {
                base64name[j++] = (UCHAR)_nx_utility_base64_array[((((UCHAR)name[i]) & 0x3) << 4) | (((UCHAR)name[i + 1]) >> 4)];
            }
            else
            {

                /* If no more name character, pad with zero.  */
                base64name[j++] = (UCHAR)_nx_utility_base64_array[(((UCHAR)name[i]) & 0x3) << 4];
            }
            i++;
            step++;
        }
        else if (step == 2)
        {

            /* Use last 4 bits of name character and first 2 bits of next name character for index.  */
            if ((i + 1) < input_name_size)
            {
                base64name[j++] = (UCHAR)_nx_utility_base64_array[((((UCHAR)name[i]) & 0xF) << 2) | (((UCHAR)name[i + 1]) >> 6)];
            }
            else
            {

                /* If no more name character, pad with zero.  */
                base64name[j++] = (UCHAR)_nx_utility_base64_array[(((UCHAR)name[i]) & 0xF) << 2];
            }
            i++;
            step++;
        }
        else /* Step 3 */
        {

            /* Use last 6 bits of name character for index.  */
            base64name[j++] = (UCHAR)_nx_utility_base64_array[(((UCHAR)name[i]) & 0x3F)];
            i++;
            step = 0;
        }
    }

    /* Now add the PAD characters.  */
    while (pad--)
    {

        /* Pad base64name with '=' characters.  */
        base64name[j++] = '=';
    }

    /* Put a NULL character in.  */
    base64name[j] = NX_NULL;
    *bytes_copied = j;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_utility_base64_decode                           PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decodes the input base64 ASCII string and converts    */
/*    it into a standard ASCII representation.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    base64name                            Encoded base64 name string    */ 
/*    base64name_size                       Size of encoded base64 name   */ 
/*    name                                  Name string                   */ 
/*    name_size                             Size of name                  */ 
/*    bytes_copied                          Number of bytes copied        */
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
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-02-2021     Yuxin Zhou               Initial Version 6.1.6         */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            removed useless condition,  */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of reading  */
/*                                            overflow,                   */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_utility_base64_decode(UCHAR *base64name, UINT base64name_size, UCHAR *name, UINT name_size, UINT *bytes_copied)
{
UINT    i, j;
UINT    value1, value2;
UINT    step;
UINT    source_size = base64name_size;

    /* Check for invalid input pointers.  */
    if ((base64name == NX_NULL) || (name == NX_NULL) || (bytes_copied == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check the size.  */
    if ((base64name_size == 0) || (name_size == 0))
    {
        return(NX_SIZE_ERROR);
    }

    /* Adjust the length to represent the ASCII name.  */
    base64name_size = ((base64name_size * 6) / 8);

    if ((base64name_size) && (base64name[source_size - 1] == '='))
    {
        base64name_size--;

        if ((base64name_size) && (base64name[source_size - 2] == '='))
        {
            base64name_size--;
        }
    }

    /* Check the buffer size.  */
    if (name_size <= base64name_size)
    {
        return(NX_SIZE_ERROR);
    }

    /* Setup index into the ASCII name.  */
    j = 0;

    /* Compute the ASCII name.  */
    step = 0;
    i =  0;
    while ((j < base64name_size) && (base64name[i]) && (base64name[i] != '='))
    {

        /* Derive values of the Base64 name.  */
        if ((base64name[i] >= 'A') && (base64name[i] <= 'Z'))
            value1 =  (UINT) (base64name[i] - 'A');
        else if ((base64name[i] >= 'a') && (base64name[i] <= 'z'))
            value1 =  (UINT) (base64name[i] - 'a') + 26;
        else if ((base64name[i] >= '0') && (base64name[i] <= '9'))
            value1 =  (UINT) (base64name[i] - '0') + 52;
        else if ((base64name[i] == '+') ||
                 (base64name[i] == '-')) /* Base64 URL.  */
            value1 =  62;
        else if ((base64name[i] == '/') ||
                 (base64name[i] == '_')) /* Base64 URL.  */
            value1 =  63;
        else
            value1 =  0;

        /* Derive value for the next character.  */
        if ((base64name[i + 1] >= 'A') && (base64name[i + 1] <= 'Z'))
            value2 =  (UINT) (base64name[i+1] - 'A');
        else if ((base64name[i + 1] >= 'a') && (base64name[i + 1] <= 'z'))
            value2 =  (UINT) (base64name[i+1] - 'a') + 26;
        else if ((base64name[i + 1] >= '0') && (base64name[i + 1] <= '9'))
            value2 =  (UINT) (base64name[i+1] - '0') + 52;
        else if ((base64name[i + 1] == '+') ||
                 (base64name[i + 1] == '-')) /* Base64 URL.  */
            value2 =  62;
        else if ((base64name[i + 1] == '/') ||
                 (base64name[i + 1] == '_')) /* Base64 URL.  */
            value2 =  63;
        else
            value2 =  0;

        /* Determine which step we are in.  */
        if (step == 0)
        {

            /* Use first value and first 2 bits of second value.  */
            name[j++] = (UCHAR) (((value1 & 0x3f) << 2) | ((value2 >> 4) & 3));
            i++;
            step++;
        }
        else if (step == 1)
        {

            /* Use last 4 bits of first value and first 4 bits of next value.  */
            name[j++] = (UCHAR) (((value1 & 0xF) << 4) | (value2 >> 2));
            i++;
            step++;
        }
        else
        {

            /* Use first 2 bits and following 6 bits of next value.  */
            name[j++] = (UCHAR) (((value1 & 3) << 6) | (value2 & 0x3f));
            i++;
            i++;
            step =  0;
        }
    }

    /* Put a NULL character in.  */
    name[j] = NX_NULL;
    *bytes_copied = j;

    return(NX_SUCCESS);
}