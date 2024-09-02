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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_find_curve_method                    PORTABLE C      */
/*                                                           6.2.0        */
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
/*    tls_session                           TLS control block             */
/*    named_curve                           Named curve ID                */
/*    curve_method                          Pointer to hold the curve     */
/*                                            method                      */
/*    curve_priority                        Pointer to return value for   */
/*                                            priority value              */
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
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate Pre-Master Secret    */
/*    _nx_secure_tls_process_certificate_verify                           */
/*                                          Process CertificateVerify     */
/*    _nx_secure_tls_proc_clienthello_sec_sa_extension                    */
/*                                          Process supported groups      */
/*                                            extensions in ClientHello   */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*    _nx_secure_tls_send_certificate_verify                              */
/*                                          Send CertificateVerify        */
/*    _nx_secure_tls_send_server_key_exchange                             */
/*                                          Send ServerKeyExchange        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            curve priority return value,*/
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            ECC curve table in X509,    */
/*                                            resulting in version 6.1.6  */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            updated parameters list,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_find_curve_method(NX_SECURE_TLS_ECC *tls_ecc, USHORT named_curve,
                                      const NX_CRYPTO_METHOD **curve_method, UINT *curve_priority)
{
USHORT i;

    *curve_method = NX_NULL;

    /* Find out the curve method for the named curve. */
    for (i = 0; i < tls_ecc -> nx_secure_tls_ecc_supported_groups_count; i++)
    {
        if (named_curve == tls_ecc -> nx_secure_tls_ecc_supported_groups[i])
        {
            *curve_method = tls_ecc -> nx_secure_tls_ecc_curves[i];

            /* The index in the supported list is the curve priority: lower value == higher priority. */
            if(curve_priority != NX_NULL)
            {
                *curve_priority = i;
            }
            break;
        }
    }

    if (*curve_method == NX_NULL)
    {
        return(NX_CRYTPO_MISSING_ECC_CURVE);
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
