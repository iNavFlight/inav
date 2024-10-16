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

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_x509_dns_name_initialize                PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the X509 DNS name init call.     */
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
/*    _nx_secure_x509_dns_name_initialize                                 */
/*                                          Actual DNS name init call     */
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
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_x509_dns_name_initialize(NX_SECURE_X509_DNS_NAME *dns_name,
                                          const UCHAR *name_string, USHORT length)
{
UINT status;

    if (dns_name == NX_CRYPTO_NULL)
    {
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#else
        return(NX_PTR_ERROR);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
    }

    if (length > NX_SECURE_X509_DNS_NAME_MAX)
    {
        return(NX_SECURE_X509_NAME_STRING_TOO_LONG);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Actual function call. */
    status = _nx_secure_x509_dns_name_initialize(dns_name, name_string, length);

    /* Return completion status.  */
    return(status);
}

