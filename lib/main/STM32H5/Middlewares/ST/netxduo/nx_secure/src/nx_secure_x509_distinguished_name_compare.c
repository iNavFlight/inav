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

static INT _nx_secure_x509_distinguished_name_field_compare(const UCHAR *field1, UINT length1,
                                                            const UCHAR *field2, UINT length2);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare          PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares two X509 Distinguished Names to see if they  */
/*    are equal. Returns 0 on equality, non-zero otherwise. The           */
/*    compare_fields parameter is a bitmap that indicates which fields to */
/*    compare. In general, internal comparisons (as when validating a     */
/*    certificate chain) will use NX_SECURE_X509_NAME_ALL_FIELDS, which   */
/*    will do a full name comparison using the enabled fields (strict     */
/*    name compare and extended name fields change the behavior). For any */
/*    calls coming from application/user API will use the bit value       */
/*    NX_SECURE_X509_NAME_COMMON_NAME so only the Common Name fields are  */
/*    compared (even if strict name comparison is enabled). Eventually,   */
/*    the bitfield may be exposed to the user level to allow users to     */
/*    have more control over X.509 distinguished name comparisons.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                                  Pointer to an X509 name       */
/*    compare_name                          Name to compare against       */
/*    compare_fields                        Bitmap of fields to compare   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    equality                              0 if equal, else non-zero     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_field_compare                    */
/*                                          Compare common names          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_certificate       Send DTLS certificate         */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*    _nx_secure_x509_certificate_list_add  Add incoming cert to store    */
/*    _nx_secure_x509_certificate_list_find Find certificate by name      */
/*    _nx_secure_x509_certificate_list_remove                             */
/*                                          Remove certificate from list  */
/*    _nx_secure_x509_crl_revocation_check  Check revocation in crl       */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get remote host certificate   */
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
INT _nx_secure_x509_distinguished_name_compare(NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                               NX_SECURE_X509_DISTINGUISHED_NAME *compare_name,
                                               ULONG compare_fields)
{
INT status = 0;

    /* NOTE: The status is updated using a binary OR operation - using addition would allow
             someone to construct a distinguished name that falsely matches another by explioting
             the memcmp return values (which are signed so if you had one string that returned a
             positive value you could negate that by using a string that compared with an equal
             magnitude negative value). */

    /* Compare common names for equality. */
    if (compare_fields & NX_SECURE_X509_NAME_COMMON_NAME)
    {
        status = _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_common_name,
                                                                  name -> nx_secure_x509_common_name_length,
                                                                  compare_name -> nx_secure_x509_common_name,
                                                                  compare_name -> nx_secure_x509_common_name_length);
    }

    /* If we are doing strict X509 comparisons, compare ALL fields. */
#ifdef NX_SECURE_X509_STRICT_NAME_COMPARE
    if (compare_fields & NX_SECURE_X509_NAME_COUNTRY)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_country,
                                                                   name -> nx_secure_x509_country_length,
                                                                   compare_name -> nx_secure_x509_country,
                                                                   compare_name -> nx_secure_x509_country_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_ORGANIZATION)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_organization,
                                                                   name -> nx_secure_x509_organization_length,
                                                                   compare_name -> nx_secure_x509_organization,
                                                                   compare_name -> nx_secure_x509_organization_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_ORG_UNIT)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_org_unit,
                                                                   name -> nx_secure_x509_org_unit_length,
                                                                   compare_name -> nx_secure_x509_org_unit,
                                                                   compare_name -> nx_secure_x509_org_unit_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_QUALIFIER)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_distinguished_name_qualifier,
                                                                   name -> nx_secure_x509_distinguished_name_qualifier_length,
                                                                   compare_name -> nx_secure_x509_distinguished_name_qualifier,
                                                                   compare_name -> nx_secure_x509_distinguished_name_qualifier_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_STATE)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_state,
                                                                   name -> nx_secure_x509_state_length,
                                                                   compare_name -> nx_secure_x509_state,
                                                                   compare_name -> nx_secure_x509_state_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_SERIAL_NUMBER)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_serial_number,
                                                                   name -> nx_secure_x509_serial_number_length,
                                                                   compare_name -> nx_secure_x509_serial_number,
                                                                   compare_name -> nx_secure_x509_serial_number_length);
    }


#ifdef NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES


    if (compare_fields & NX_SECURE_X509_NAME_LOCALITY)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_locality,
                                                                   name -> nx_secure_x509_locality_length,
                                                                   compare_name -> nx_secure_x509_locality,
                                                                   compare_name -> nx_secure_x509_locality_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_TITLE)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_title,
                                                                   name -> nx_secure_x509_title_length,
                                                                   compare_name -> nx_secure_x509_title,
                                                                   compare_name -> nx_secure_x509_title_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_SURNAME)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_surname,
                                                                   name -> nx_secure_x509_surname_length,
                                                                   compare_name -> nx_secure_x509_surname,
                                                                   compare_name -> nx_secure_x509_surname_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_GIVEN_NAME)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_given_name,
                                                                   name -> nx_secure_x509_given_name_length,
                                                                   compare_name -> nx_secure_x509_given_name,
                                                                   compare_name -> nx_secure_x509_given_name_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_INITIALS)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_initials,
                                                                   name -> nx_secure_x509_initials_length,
                                                                   compare_name -> nx_secure_x509_initials,
                                                                   compare_name -> nx_secure_x509_initials_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_PSEUDONYM)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_pseudonym,
                                                                   name -> nx_secure_x509_pseudonym_length,
                                                                   compare_name -> nx_secure_x509_pseudonym,
                                                                   compare_name -> nx_secure_x509_pseudonym_length);
    }

    if (compare_fields & NX_SECURE_X509_NAME_GENERATION_QUALIFIER)
    {
        status |= _nx_secure_x509_distinguished_name_field_compare(name -> nx_secure_x509_generation_qualifier,
                                                                   name -> nx_secure_x509_generation_qualifier_length,
                                                                   compare_name -> nx_secure_x509_generation_qualifier,
                                                                   compare_name -> nx_secure_x509_generation_qualifier_length);
    }
#endif
#endif

    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_field_compare    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function compares two X509 Distinguished Name fields given two */
/*    separate lengths to see if they are equal. Handles special cases    */
/*    where lengths are not equal and when both lengths are 0.            */
/*    Returns 0 on equality or both lengths 0, non-zero otherwise.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    field1                                Pointer to an X509 name field */
/*    length1                               Length of field1              */
/*    field2                                Pointer to an X509 name field */
/*    length2                               Length of field2              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    equality                              0 if equal, else non-zero     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
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
static INT _nx_secure_x509_distinguished_name_field_compare(const UCHAR *field1, const UINT length1,
                                                            const UCHAR *field2, const UINT length2)
{
INT compare_value;

    if (length1 == length2)
    {
        /* If lengths are both 0, return equality. */
        if (length1 == 0)
        {
            compare_value = 0;
        }
        else
        {
            /* Lengths are equal and non-zero, actually compare strings. */
            compare_value = NX_SECURE_MEMCMP(field1, field2, length1);
        }
    }
    else
    {
        /* Lengths are not equal, return inequality. */
        compare_value = 1;
    }

    return(compare_value);
}

