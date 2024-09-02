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

#ifndef NX_SECURE_X509_DISABLE_CRL
static UCHAR generated_hash[64];       /* We need to be able to hold the entire generated hash - SHA-512 = 64 bytes. */
static UCHAR decrypted_signature[512]; /* This needs to hold the entire decrypted data - RSA 2048-bit key = 256 bytes. */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_verify                          PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies a CRL by checking its signature against its  */
/*    issuer's public key. Note that a CRL does not have any crypto       */
/*    methods assigned to it, so the certificate being checked is also    */
/*    passed into this function so that its crypto routines may be        */
/*    utilized.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Certificate to be checked     */
/*    crl                                   Pointer to parsed CRL         */
/*    store                                 Pointer to trusted store      */
/*    issuer_certificate                    CRL issuer's certificate      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Crypto initialization         */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    _nx_secure_x509_pkcs7_decode          Decode the PKCS#7 signature   */
/*    _nx_secure_x509_find_certificate_methods                            */
/*                                          Find certificate methods      */
/*    _nx_secure_x509_find_curve_method     Find named curve used         */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_revocation_check  Check revocation in crl       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), update   */
/*                                            ECC find curve method,      */
/*                                            add KeyUsage check,         */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_crl_verify(NX_SECURE_X509_CERT *certificate, NX_SECURE_X509_CRL *crl,
                                NX_SECURE_X509_CERTIFICATE_STORE *store,
                                NX_SECURE_X509_CERT *issuer_certificate)
{
UINT                    status;
UINT                    oid_length;
const UCHAR            *oid;
UINT                    decrypted_hash_length;
const UCHAR            *decrypted_hash;
const UCHAR            *crl_verify_data;
UINT                    verify_data_length;
const UCHAR            *signature_data;
UINT                    signature_length;
UINT                    compare_result;
UINT                    hash_length;
const NX_CRYPTO_METHOD *hash_method;
const NX_CRYPTO_METHOD *public_cipher_method;
NX_SECURE_X509_CRYPTO  *crypto_methods;
VOID                   *handler = NX_CRYPTO_NULL;
#ifndef NX_SECURE_X509_DISABLE_KEY_USAGE_CHECK
USHORT                  key_usage_bitfield = 0;
#endif
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_EC_PUBLIC_KEY *ec_pubkey;
const NX_CRYPTO_METHOD  *curve_method;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

    NX_CRYPTO_PARAMETER_NOT_USED(store);

#ifndef NX_SECURE_X509_DISABLE_KEY_USAGE_CHECK
    /* Before we do any crypto verification, we need to check the KeyUsage extension. */
    status = _nx_secure_x509_key_usage_extension_parse(issuer_certificate, &key_usage_bitfield);

    /* If extension is not present, we don't need to verify per RFC 5280. */
    if(NX_SECURE_X509_SUCCESS == status)
    {
        /* The issuer cert has a KeyUsage extension - check the CRLSign bit. */
        if(!(key_usage_bitfield & NX_SECURE_X509_KEY_USAGE_CRL_SIGN))
        {
            return(NX_SECURE_X509_KEY_USAGE_ERROR);
        }
    }
#endif

    /* Get working pointers to relevant data. */
    crl_verify_data = crl -> nx_secure_x509_crl_verify_data;
    verify_data_length = crl -> nx_secure_x509_crl_verify_data_length;
    signature_data = crl -> nx_secure_x509_crl_signature_data;
    signature_length = crl -> nx_secure_x509_crl_signature_data_length;

    /* Find certificate crypto methods for this certificate. */
    status = _nx_secure_x509_find_certificate_methods(certificate, crl -> nx_secure_x509_crl_signature_algorithm, &crypto_methods);
    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Assign local pointers for the crypto methods. */
    hash_method = crypto_methods -> nx_secure_x509_hash_method;
    public_cipher_method = crypto_methods -> nx_secure_x509_public_cipher_method;

    if (hash_method -> nx_crypto_init)
    {
        status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                      NX_CRYPTO_NULL,
                                      0,
                                      &handler,
                                      certificate -> nx_secure_x509_hash_metadata_area,
                                      certificate -> nx_secure_x509_hash_metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }

    /* We need to generate a hash of this CRL in order to verify it against our trusted store. */
    if (hash_method -> nx_crypto_operation != NX_CRYPTO_NULL)
    {
        status = hash_method -> nx_crypto_operation(NX_CRYPTO_VERIFY,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_CRYPTO_NULL,
                                           0,
                                           (UCHAR *)crl_verify_data,
                                           verify_data_length,
                                           NX_CRYPTO_NULL,
                                           generated_hash,
                                           sizeof(generated_hash),
                                           certificate -> nx_secure_x509_hash_metadata_area,
                                           certificate -> nx_secure_x509_hash_metadata_size,
                                           NX_CRYPTO_NULL, NX_CRYPTO_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }

    if (hash_method -> nx_crypto_cleanup)
    {
        status = hash_method -> nx_crypto_cleanup(certificate -> nx_secure_x509_hash_metadata_area);

        if(status != NX_CRYPTO_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */                        
            return(status);
        }                                                     
    }

    hash_length = (hash_method -> nx_crypto_ICV_size_in_bits >> 3);

    /* Perform a public-key decryption operation on the extracted signature from the CRL.
     * In this case, the operation is doing a "reverse decryption", using the public key to decrypt, rather
     * than the private. This allows us to tie a trusted root certificate to a signature of a CRL
     * signed by that root CA's private key. when combined with a hash method, this is the basic digital
     * signature operation. */
    if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA ||
        public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_RSA)
    {
        /* Make sure the public algorithm of the issuer certificate is RSA. */
        if (issuer_certificate -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_RSA)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        if (public_cipher_method -> nx_crypto_init != NX_CRYPTO_NULL)
        {
            /* Initialize the crypto method with public key. */
            status = public_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)public_cipher_method,
                                                   (UCHAR *)issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                   (NX_CRYPTO_KEY_SIZE)(issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                   &handler,
                                                   certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                   certificate -> nx_secure_x509_public_cipher_metadata_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */                        
                return(status);
            }
        }

        if (public_cipher_method -> nx_crypto_operation != NX_CRYPTO_NULL)
        {
            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                        handler,
                                                        (NX_CRYPTO_METHOD*)public_cipher_method,
                                                        (UCHAR *)issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                        (NX_CRYPTO_KEY_SIZE)(issuer_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                        (UCHAR *)signature_data,
                                                        signature_length,
                                                        NX_CRYPTO_NULL,
                                                        decrypted_signature,
                                                        sizeof(decrypted_signature),
                                                        certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                        certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                        NX_CRYPTO_NULL, NX_CRYPTO_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */            
                return(status);
            }
        }

        if (public_cipher_method -> nx_crypto_cleanup)
        {
            status = public_cipher_method -> nx_crypto_cleanup(certificate -> nx_secure_x509_public_cipher_metadata_area);

            if(status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                /* Clear secrets on errors. */
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
                NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */            
                return(status);
            }
        }

        /* Decode the decrypted signature, which should be in PKCS#7 format. */
        status = _nx_secure_x509_pkcs7_decode(decrypted_signature, signature_length, &oid, &oid_length,
                                              &decrypted_hash, &decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
        if(status != NX_SECURE_X509_SUCCESS || decrypted_hash_length != hash_length)
        {
            /* Clear secrets on errors. */
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
            NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
        }
#endif /* NX_SECURE_KEY_CLEAR  */

        if (status != NX_SECURE_X509_SUCCESS)
        {
            return(status);
        }

        if (decrypted_hash_length != hash_length)
        {
            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        /* Compare generated hash with decrypted hash. */
        compare_result = (UINT)NX_SECURE_MEMCMP(generated_hash, decrypted_hash, decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
        NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

        /* If the comparision worked, return success. */
        if (compare_result == 0)
        {
            return(NX_SECURE_X509_SUCCESS);
        }
    }
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    else if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA)
    {
        /* Make sure the public algorithm of the issuer certificate is EC. */
        if (issuer_certificate -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_EC)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */
            return(NX_SECURE_X509_WRONG_SIGNATURE_METHOD);
        }

        /* Verify the ECDSA signature. */

        ec_pubkey = &issuer_certificate -> nx_secure_x509_public_key.ec_public_key;

        /* Find out which named curve the remote certificate is using. */
        status = _nx_secure_x509_find_curve_method((USHORT)(ec_pubkey -> nx_secure_ec_named_curve), &curve_method);

#ifdef NX_SECURE_KEY_CLEAR
        if(status != NX_SECURE_X509_SUCCESS || curve_method == NX_CRYPTO_NULL)
        {
            /* Clear secrets on errors. */
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
        }
#endif /* NX_SECURE_KEY_CLEAR  */

        if(status != NX_SECURE_X509_SUCCESS)
        {
            return(status);
        }

        if (curve_method == NX_CRYPTO_NULL)
        {
            /* The issuer certificate is using an unsupported curve. */
            return(NX_SECURE_X509_UNSUPPORTED_PUBLIC_CIPHER);
        }

        if (public_cipher_method -> nx_crypto_init != NX_CRYPTO_NULL)
        {
            status = public_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)public_cipher_method,
                                                            (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                            (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                            &handler,
                                                            certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                            certificate -> nx_secure_x509_public_cipher_metadata_size);
            if (status != NX_CRYPTO_SUCCESS)
            {
#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */
                return(status);
            }
        }
        if (public_cipher_method -> nx_crypto_operation == NX_CRYPTO_NULL)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */
            return(NX_SECURE_X509_MISSING_CRYPTO_ROUTINE);
        }

        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                             (NX_CRYPTO_METHOD*)public_cipher_method, NX_CRYPTO_NULL, 0,
                                                             (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_CRYPTO_NULL,
                                                             NX_CRYPTO_NULL, 0,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                             NX_CRYPTO_NULL, NX_CRYPTO_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(status);
        }

        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_VERIFY, handler,
                                                             (NX_CRYPTO_METHOD*)public_cipher_method,
                                                             (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                             (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                             generated_hash,
                                                             hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                                             NX_CRYPTO_NULL,
                                                             (UCHAR *)signature_data,
                                                             signature_length,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_area,
                                                             certificate -> nx_secure_x509_public_cipher_metadata_size,
                                                             NX_CRYPTO_NULL, NX_CRYPTO_NULL);
#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

        if (status == NX_CRYPTO_SUCCESS)
        {
            return(NX_SECURE_X509_SUCCESS);
        }
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
    else
    {
        return(NX_SECURE_X509_UNSUPPORTED_PUBLIC_CIPHER);
    }

#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(generated_hash, 0, sizeof(generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Comparison failed, return error. */
    return(NX_SECURE_X509_CRL_SIGNATURE_CHECK_FAILED);
}
#endif /* NX_SECURE_X509_DISABLE_CRL */
