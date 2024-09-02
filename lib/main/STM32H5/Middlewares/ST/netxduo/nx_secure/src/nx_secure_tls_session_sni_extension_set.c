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

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_sni_extension_set            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks sets the DNS Name in a TLS client for the      */
/*    Server Name Indication (SNI) extension (RFC 6066). The current RFC  */
/*    specifies that only a single DNS Name entry is allowed in the SNI   */
/*    extension data. If other name types are added in the future, new    */
/*    API will be added.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    dns_name                              Pointer to the server name    */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            fix compile error,          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_sni_extension_set(NX_SECURE_TLS_SESSION *tls_session,
                                              NX_SECURE_X509_DNS_NAME *dns_name)
{
#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED

    /* Set the pointer to the SNI DNS name so it can be sent in the ClientHello. */
    tls_session -> nx_secure_tls_sni_extension_server_name = dns_name;

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(dns_name);

    return(NX_NOT_ENABLED);
#endif
}

