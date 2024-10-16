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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_server_certificate_remove            PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a server certificate from a TLS session X.509 */
/*    certificate store based on the unique ID assigned when the          */
/*    certificate was added to the store with                             */
/*    nx_secure_tls_server_certificate_add.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    cert_id                               Certificate identifier        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove server certificate     */
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
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_server_certificate_remove(NX_SECURE_TLS_SESSION *tls_session, UINT cert_id)
{
UINT status;

    /* Remove the certificate from the local store. */
    status = _nx_secure_x509_store_certificate_remove(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                      NX_NULL, NX_SECURE_X509_CERT_LOCATION_LOCAL, cert_id);

    /* Translate some X.509 return values into TLS return values. */
    if (status == NX_SECURE_X509_CERTIFICATE_NOT_FOUND)
    {
        return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
    }

    /* Return completion status.  */
    return(status);
}

