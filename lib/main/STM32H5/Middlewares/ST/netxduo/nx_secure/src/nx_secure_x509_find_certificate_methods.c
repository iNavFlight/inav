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
/*    _nx_secure_x509_find_certificate_methods            PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds crypto methods specified in a certificate.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    cert                                  Pointer to X509 certificate   */
/*    signature_algorithm                   Id for signature method       */
/*    crypto_methods                        Return matching table entry   */
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
/*    _nx_secure_tls_process_certificate_verify                           */
/*                                          Process CertificateVerify     */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*    _nx_secure_tls_send_certificate_verify                              */
/*                                          Send certificate verify       */
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
UINT _nx_secure_x509_find_certificate_methods(NX_SECURE_X509_CERT *cert, USHORT signature_algorithm,
                                              NX_SECURE_X509_CRYPTO **crypto_methods)
{
SHORT index;

    /* The number of ciphersuites is very small so a linear search should be fine. */
    for (index = 0; index < cert -> nx_secure_x509_cipher_table_size; ++index)
    {
        /* See if the ciphersuite is supported. */
        if (cert -> nx_secure_x509_cipher_table[index].nx_secure_x509_crypto_identifier == signature_algorithm)
        {
            *crypto_methods = &cert -> nx_secure_x509_cipher_table[index];
            return(NX_SECURE_X509_SUCCESS);
        }
    }

    /* No entry found, crypto routines unknown. */
    *crypto_methods = NX_CRYPTO_NULL;
    return(NX_SECURE_X509_UNKNOWN_CERT_SIG_ALGORITHM);
}

