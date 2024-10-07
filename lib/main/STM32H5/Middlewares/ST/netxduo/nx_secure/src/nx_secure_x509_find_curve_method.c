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

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE

/* Supported named curves. */
const USHORT *_nx_secure_x509_ecc_supported_groups;

/* Number of supported named curves. */
USHORT  _nx_secure_x509_ecc_supported_groups_count;

/* Corresponding crypto methods for the supported named curve. */
const NX_CRYPTO_METHOD **_nx_secure_x509_ecc_curves;

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_find_curve_method                   PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the curve method for the specified named curve  */
/*    ID.                                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    named_curve                           Named curve ID                */
/*    curve_method                          Pointer to hold the curve     */
/*                                            method                      */
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
/*    _nx_secure_x509_certificate_verify    Verify a certificate          */
/*    _nx_secure_x509_crl_verify            Verify revocation list        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-02-2021     Timothy Stapko           Initial Version 6.1.6         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_find_curve_method(USHORT named_curve, const NX_CRYPTO_METHOD **curve_method)
{
USHORT i;

    /* Find out the curve method for the named curve. */
    for (i = 0; i < _nx_secure_x509_ecc_supported_groups_count; i++)
    {
        if (named_curve == _nx_secure_x509_ecc_supported_groups[i])
        {
            *curve_method = _nx_secure_x509_ecc_curves[i];
            return(NX_SECURE_X509_SUCCESS);
        }
    }

    *curve_method = NX_CRYPTO_NULL;
    return(NX_CRYTPO_MISSING_ECC_CURVE);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
