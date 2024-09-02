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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_common_name_dns_check               PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks a certificate's Common Name against a Top      */
/*    Level Domain name (TLD) provided by the caller for the purposes of  */
/*    DNS validation of a remote host. This utility function is intended  */
/*    to be called from within a certificate validation callback routine  */
/*    provided by the application. The TLD name should be the top part of */
/*    the URL used to access the remote host (the "."-separated string    */
/*    before the first slash).                                            */
/*                                                                        */
/*    NOTE 1: If the Common Name does not match the provided string, the  */
/*            "subject alt name" field is compared as well.               */
/*                                                                        */
/*    NOTE 2: It is important to understand the format of the common name */
/*            (and subject alt name) in expected certificates. For        */
/*            example, some certificates may use a raw IP address or a    */
/*            wild card. The DNS TLD string must be formatted such that   */
/*            it will match the expected values in received certificates. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to certificate        */
/*    dns_tld                               Top-level domain name         */
/*    dns_tls_length                        Length of TLS in bytes        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Validity of certificate       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*    _nx_secure_x509_subject_alt_names_find                              */
/*                                          Find subject alt names        */
/*    _nx_secure_x509_wildcard_compare      Wildcard compare for names    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
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
UINT _nx_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate, const UCHAR *dns_tld,
                                           UINT dns_tld_length)
{
INT                      compare_value;
UINT                     status;
const UCHAR             *common_name;
USHORT                   common_name_len;
NX_SECURE_X509_EXTENSION alt_name_extension;

    /* Get access to our certificate fields. */
    common_name = certificate -> nx_secure_x509_distinguished_name.nx_secure_x509_common_name;
    common_name_len = certificate -> nx_secure_x509_distinguished_name.nx_secure_x509_common_name_length;

    /* Compare the given string against the common name. */
    compare_value = _nx_secure_x509_wildcard_compare(dns_tld, dns_tld_length, common_name, common_name_len);

    if (compare_value == 0)
    {
        return(NX_SECURE_X509_SUCCESS);
    }

    /* Find the subject alt name extension in the certificate. */
    status = _nx_secure_x509_extension_find(certificate, &alt_name_extension, NX_SECURE_TLS_X509_TYPE_SUBJECT_ALT_NAME);

    /* See if extension present - it is OK if not present! */
    if (status == NX_SECURE_X509_SUCCESS)
    {
        /* Extract the subject alt name string from the parsed extension. */
        status = _nx_secure_x509_subject_alt_names_find(&alt_name_extension, dns_tld, dns_tld_length, NX_SECURE_X509_SUB_ALT_NAME_TAG_DNSNAME);

        if (status == NX_SECURE_X509_SUCCESS)
        {
            return(NX_SECURE_X509_SUCCESS);
        }
    }

    /* If we get here, none of the strings matched. */
    return(NX_SECURE_X509_CERTIFICATE_DNS_MISMATCH);
}

