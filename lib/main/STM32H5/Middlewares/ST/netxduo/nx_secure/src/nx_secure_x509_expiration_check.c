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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    X.509 Digital Certificates                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_x509.h"

/* Local helper function. */
static UINT _nx_secure_x509_asn1_time_to_unix_convert(const UCHAR *asn1_time, USHORT asn1_length,
                                                      USHORT format, ULONG *unix_time);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_expiration_check                    PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks a certificate's validity period against the    */
/*    current time, which is a 32-bit UNIX-epoch format value of GMT.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to certificate        */
/*    current_time                          Current GMT value             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Validity of certificate       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_time_to_unix_convert                           */
/*                                          Convert ASN.1 time to UNIX    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_expiration_check(NX_SECURE_X509_CERT *certificate, ULONG current_time)
{
ULONG not_before;
ULONG not_after;
UINT  status;

    /* First, convert the X.509 ASN.1 time format into 32-bit UINX-epoch format of the "not before" field. */
    status = _nx_secure_x509_asn1_time_to_unix_convert(certificate -> nx_secure_x509_not_before, certificate -> nx_secure_x509_not_before_length,
                                                       certificate -> nx_secure_x509_validity_format, &not_before);
    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Convert the "not after" time field. */
    status = _nx_secure_x509_asn1_time_to_unix_convert(certificate -> nx_secure_x509_not_after, certificate -> nx_secure_x509_not_after_length,
                                                       certificate -> nx_secure_x509_validity_format, &not_after);
    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Check if certificate is expired. */
    if (current_time > not_after)
    {
        /* Certificate is expired. */
        return(NX_SECURE_X509_CERTIFICATE_EXPIRED);
    }

    /* Check if certificate is not yet valid. */
    if (current_time < not_before)
    {
        /* Certificate is not valid yet. */
        return(NX_SECURE_X509_CERTIFICATE_NOT_YET_VALID);
    }

    return(NX_SECURE_X509_SUCCESS);
}



/* Helper function to convert the ASN.1 time formats into UNIX epoch time for comparison. */

#define date_2_chars_to_int(buffer, index) (ULONG)(((buffer[index] - '0') * 10) + (buffer[index + 1] - '0'))

/* Array indexed on month - 1 gives the total number of days in all previous months (through last day of previous
   month). Leap years are handled in the logic below and are not reflected in this array. */
/* J   F   M   A    M    J    J    A    S    O    N    D */
static const UINT days_before_month[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_asn1_time_to_unix_convert           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts ASN.1 time to 32-bit UNIX-epoch format value */
/*    of GMT.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    asn1_time                             String of ASN.1 time          */
/*    asn1_length                           Length of ASN.1 time string   */
/*    format                                Format of UNIX time           */
/*    unix_time                             UNIX time value for output    */
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
/*    _nx_secure_x509_expiration_check      Verify expiration of cert     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            changed LONG to ULONG to    */
/*                                            extend the time range,      */
/*                                            removed unused code,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_x509_asn1_time_to_unix_convert(const UCHAR *asn1_time, USHORT asn1_length,
                                                      USHORT format, ULONG *unix_time)
{
ULONG year, month, day, hour, minute, second;
UINT index;

    NX_CRYPTO_PARAMETER_NOT_USED(asn1_length);
    index = 0;

    /* See what format we are using. */
    if (format == NX_SECURE_ASN_TAG_UTC_TIME)
    {
        /* UTCTime is either "YYMMDDhhmm[ss]Z" or "YYMMDDhhmm[ss](+|-)hhmm" */
        year = date_2_chars_to_int(asn1_time, 0);
        month = date_2_chars_to_int(asn1_time, 2);
        day = date_2_chars_to_int(asn1_time, 4) - 1; /* For calculations, day is 0-based. */
        hour = date_2_chars_to_int(asn1_time, 6);
        minute = date_2_chars_to_int(asn1_time, 8);
        second = 0;

        /* Check the next field, can be 'Z' for Zulu time (GMT) or [+/-] for local time offset. */
        index = 10;

        /* Check for optional seconds. */
        if (asn1_time[index] != 'Z' && asn1_time[index] != '+' && asn1_time[index] != '-')
        {
            second = date_2_chars_to_int(asn1_time, index);
            index += 2;
        }

        /* Check for GMT time or local time offset. */
        if (asn1_time[index] != 'Z')
        {
            /* Check for optional local time offset. NOTE: The additions and subtractions here may
             * result in values > 24 or < 0 but that is OK for the calculations. */
            if (asn1_time[index] == '+')
            {
                index++; /* Skip the '+' */
                hour -= date_2_chars_to_int(asn1_time, index);
                index += 2;
                minute -= date_2_chars_to_int(asn1_time, index);
            }
            else if (asn1_time[index] == '-')
            {
                index++; /* Skip the '-' */
                hour += date_2_chars_to_int(asn1_time, index);
                index += 2;
                minute += date_2_chars_to_int(asn1_time, index);
            }
            else
            {
                /* Not a correct UTC time! */
                return(NX_SECURE_X509_INVALID_DATE_FORMAT);
            }
        }

        /* printf("year: %lu, month: %lu, day: %lu, hour: %lu, minute: %lu, second: %lu\n", year, month, day, hour, minute, second);*/

        /* Now we have our time in integers, calculate leap years. We aren't concerned with years outside the UNIX
           time range of 1970-2038 so we can assume every 4 years starting with 1972 is a leap year (years divisible
           by 100 are NOT leap years unless also divisible by 400, which the year 2000 is). Using integer division gives
           us the floor of the number of 4 year periods, so add 1. */
        if (year >= 70)
        {
            /* Year is before 2000. Subtract 72 to get duration from first leap year in epoch. */
            year -= 70;
            day += ((year + 2) / 4);
        }
        else
        {
            /* Year is 2000 or greater. Add 28 (2000-1972) to get duration from first leap year in epoch. */
            year += 30;
            day += ((year - 2) / 4) + 1;
        }

        /* If it is leap year and month is before March, subtract 1 day. */
        if (((year + 2) % 4 == 0) && (month < 3))
        {
            day -= 1;
        }

        /* Finally, calculate the number of seconds from the extracted values. */
        day += year * 365;
        day += days_before_month[month - 1];
        hour += day * 24;
        minute += hour * 60;
        second += minute * 60;

        /* Finally, return the converted time. */
        *unix_time = second;
    }
    else if (format == NX_SECURE_ASN_TAG_GENERALIZED_TIME)
    {
        /* Generalized time formats:
             Local time only. ``YYYYMMDDHH[MM[SS[.fff]]]'', where the optional fff is three decimal places (fractions of seconds).
             Universal time (UTC time) only. ``YYYYMMDDHH[MM[SS[.fff]]]Z''. MM, SS, .fff are optional.
             Difference between local and UTC times. ``YYYYMMDDHH[MM[SS[.fff]]]+-HHMM''. +/-HHMM is local time offset. */
        /* TODO: Implement conversion to 32-bit UNIX time. */
        return(NX_SECURE_X509_INVALID_DATE_FORMAT);
    }
    else
    {
        return(NX_SECURE_X509_INVALID_DATE_FORMAT);
    }

    return(NX_SECURE_X509_SUCCESS);
}

