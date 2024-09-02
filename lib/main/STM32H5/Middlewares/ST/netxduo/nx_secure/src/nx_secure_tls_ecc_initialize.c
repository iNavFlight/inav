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

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE

#ifndef NX_SECURE_DISABLE_X509
/* Supported named curves. */
extern const USHORT *_nx_secure_x509_ecc_supported_groups;

/* Number of supported named curves. */
extern USHORT  _nx_secure_x509_ecc_supported_groups_count;

/* Corresponding crypto methods for the supported named curve. */
extern const NX_CRYPTO_METHOD **_nx_secure_x509_ecc_curves;
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_ecc_initialize                       PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes supported curve lists for TLS.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
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
/*  04-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            ECC curve table in X509,    */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ecc_initialize(NX_SECURE_TLS_SESSION *tls_session,
                                   const USHORT *supported_groups, USHORT supported_group_count,
                                   const NX_CRYPTO_METHOD **curves)
{

    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups = supported_groups;
    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count = supported_group_count;
    tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_curves = curves;

#ifndef NX_SECURE_DISABLE_X509
    _nx_secure_x509_ecc_supported_groups = supported_groups;
    _nx_secure_x509_ecc_supported_groups_count = supported_group_count;
    _nx_secure_x509_ecc_curves = curves;
#endif

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


