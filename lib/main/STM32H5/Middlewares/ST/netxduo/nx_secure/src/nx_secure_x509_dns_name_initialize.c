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


/* Include necessary system files.  */

#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_dns_name_initialize                 PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes an X509 DNS name for use with extensions  */
/*    that use domain name entries.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dns_name                              Name structure                */
/*    name_string                           DNS name string               */
/*    length                                Length of name string         */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_dns_name_initialize(NX_SECURE_X509_DNS_NAME *dns_name,
                                         const UCHAR *name_string, USHORT length)
{

    /* Make sure we don't copy over the end of the buffer. */
    if (dns_name -> nx_secure_x509_dns_name_length > NX_SECURE_X509_DNS_NAME_MAX)
    {
        dns_name -> nx_secure_x509_dns_name_length = NX_SECURE_X509_DNS_NAME_MAX;
    }

    /* Copy the name string into the entry structure. */
    NX_SECURE_MEMCPY(dns_name -> nx_secure_x509_dns_name, name_string, length); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */

    dns_name -> nx_secure_x509_dns_name_length = length;

    /* Return completion status.  */
    return(NX_SECURE_X509_SUCCESS);
}

