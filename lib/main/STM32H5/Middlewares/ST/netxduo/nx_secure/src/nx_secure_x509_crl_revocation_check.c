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
static UINT _nx_secure_x509_crl_parse_entry(const UCHAR *buffer, ULONG length, UINT *bytes_processed,
                                            const UCHAR **serial_number, UINT *serial_number_length);
#endif /* NX_SECURE_X509_DISABLE_CRL */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_revocation_check                PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function takes a DER-encoded Certificate Revocation List and   */
/*    searches for a specific certificate in that list. The issuer of the */
/*    CRL is validated against a supplied certificate store, the          */
/*    CRL issuer is validated to be the same as the one for the           */
/*    certificate being checked, and the serial number of the certificate */
/*    in question is used to search the revoked certificates list.        */
/*    If the issuers match, the signature checks out, and the certificate */
/*    is not present in the list, the call is successful. All other cases */
/*    cause an error to be returned.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crl_data                              Pointer to DER-encoded CRL    */
/*    crl_length                            Length of CRL data in buffer  */
/*    store                                 Certificate store to be used  */
/*    certificate                           The certificate being checked */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*    _nx_secure_x509_certificate_revocation_list_parse                   */
/*                                          Parse revocation list         */
/*    _nx_secure_x509_crl_parse_entry       Parse an entry in crl         */
/*    _nx_secure_x509_crl_verify            Verify revocation list        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*    _nx_secure_x509_store_certificate_find                              */
/*                                          Find a cert in a store        */
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
/*  08-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.8  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            modified to improve code    */
/*                                            coverage result,            */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            checked expiration for all  */
/*                                            the certs in the chain,     */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_crl_revocation_check(const UCHAR *crl_data, UINT crl_length,
                                          NX_SECURE_X509_CERTIFICATE_STORE *store,
                                          NX_SECURE_X509_CERT *certificate)
{
#ifndef NX_SECURE_X509_DISABLE_CRL
NX_SECURE_X509_CRL   crl;
UINT                 status;
UINT                 crl_bytes;
INT                  compare_value;
UINT                 bytes_processed;
UINT                 length;
const UCHAR         *current_buffer;
NX_SECURE_X509_CERT *issuer_certificate;
UINT                 issuer_location;
const UCHAR         *serial_number = NX_NULL;
UINT                 serial_number_length;

    NX_SECURE_MEMSET(&crl, 0, sizeof(NX_SECURE_X509_CRL));

    /* First, parse the CRL. */
    status = _nx_secure_x509_certificate_revocation_list_parse(crl_data, crl_length, &crl_bytes, &crl);

    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Check that the issuers match according to process in RFC 5280. */
    compare_value = _nx_secure_x509_distinguished_name_compare(&crl.nx_secure_x509_crl_issuer, &certificate -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);

    if (compare_value)
    {
        /* The issuers did not match, return error. */
        return(NX_SECURE_X509_CRL_ISSUER_MISMATCH);
    }

    /* Now, check the signature of the CRL. */

    /* First, get the issuer certificate. If we have a valid store and CRL, the issuer should be available. */
    status = _nx_secure_x509_store_certificate_find(store, &crl.nx_secure_x509_crl_issuer, 0, &issuer_certificate, &issuer_location);

    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Now, check that the issuer is valid. */
    status = _nx_secure_x509_certificate_chain_verify(store, issuer_certificate, 0);

    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* Now, verify that the CRL itself is OK. */
    status = _nx_secure_x509_crl_verify(certificate, &crl, store, issuer_certificate);

    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* If we get here, then the CRL is OK and is issued by the same CA as the certificate in question.
       Finally, we parse the revocation list and see if the serial number of the certificate being
       validated is in the CRL revocation list. */

    /* Parse and search the revokedCertificates list that we obtained when we parsed the CRL.
     *       revokedCertificates     SEQUENCE OF SEQUENCE
     *       {
     *            userCertificate         CertificateSerialNumber,
     *            revocationDate          Time,
     *            crlEntryExtensions      Extensions OPTIONAL
     *                                     -- if present, version MUST be v2
     *       }  OPTIONAL,
     */
    current_buffer = crl.nx_secure_x509_crl_revoked_certs;
    length = crl.nx_secure_x509_crl_revoked_certs_length;
    /* Loop through all the entries in the sequence. */
    while (length > 0)
    {
        /* Parse an entry in the revokedCertificates list and get back the serial number. */
        status = _nx_secure_x509_crl_parse_entry(current_buffer, length, &bytes_processed, &serial_number, &serial_number_length);

        if (status != NX_SECURE_X509_SUCCESS)
        {
            return(status);
        }

        /* Make sure we don't run past the end of the sequence if one of the entries was too big. */
        NX_ASSERT(bytes_processed <= length);

        /* Compare the serial number we got from the list (if it exists) to the one in our certificate. */
        compare_value = NX_SECURE_MEMCMP(serial_number, certificate -> nx_secure_x509_serial_number,
                               certificate -> nx_secure_x509_serial_number_length);

        /* See if we have a match. */
        if (compare_value == 0)
        {
            /* This certificate has been revoked! */
            return(NX_SECURE_X509_CRL_CERTIFICATE_REVOKED);
        }

        /* No match, go to the next one. */
        length -= bytes_processed;
        current_buffer = current_buffer + bytes_processed;
    }

    /* If we get here, the CRL was good and the certificate has not been revoked. */
    return(NX_SECURE_X509_SUCCESS);
#else /* NX_SECURE_X509_DISABLE_CRL */
    NX_CRYPTO_PARAMETER_NOT_USED(crl_data);
    NX_CRYPTO_PARAMETER_NOT_USED(crl_length);
    NX_CRYPTO_PARAMETER_NOT_USED(store);
    NX_CRYPTO_PARAMETER_NOT_USED(certificate);
#ifdef NX_CRYPTO_STANDALONE_ENABLE
    return(NX_CRYPTO_FORMAT_NOT_SUPPORTED);
#else
    return(NX_NOT_SUPPORTED);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
#endif /* NX_SECURE_X509_DISABLE_CRL */
}

#ifndef NX_SECURE_X509_DISABLE_CRL
/* Helper function to parse entries in the revokedCertificates list. */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_parse_entry                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses an entry in the revokedCertificates list       */
/*    embedded within an X.509 CRL. Each entry contains the serial number */
/*    of a revoked certificate. This serial number is returned so the     */
/*    caller can compare it to the serial number of the certificate being */
/*    checked for revocation.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer into CRL data         */
/*    length                                Length of CRL data            */
/*    bytes_processed                       Return bytes parsed           */
/*    serial_number                         Return cert serial number     */
/*    serial_number_length                  Return serial number length   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_x509_crl_parse_entry(const UCHAR *buffer, ULONG length, UINT *bytes_processed,
                                            const UCHAR **serial_number, UINT *serial_number_length)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
const UCHAR *current_buffer;
ULONG        header_length;
UINT         status;


    /* Each entry in the list is a sequence containing the serial number, revocation date, and extensions. */
    *bytes_processed = 0;
    current_buffer = buffer;

    /*  First, parse the sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    *bytes_processed += header_length + tlv_length;
    current_buffer = current_buffer + header_length;
    length = tlv_length;

    /*  Next, parse the serial number. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* The serial number is an ASN.1 Integer. */
    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    /* Save off the serial number in case someone wants it. */
    *serial_number = tlv_data;
    *serial_number_length = (USHORT)tlv_length;

    /* Don't adjust bytes processed since we handled that above. Just adjust buffer. */
    current_buffer = current_buffer + (tlv_length + header_length);

    /* Parse the "revocationDate" field. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if ((tlv_type != NX_SECURE_ASN_TAG_UTC_TIME && tlv_type != NX_SECURE_ASN_TAG_GENERALIZED_TIME) || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    /* Extensions are optional. Add here when implemented. */

    return(NX_SECURE_X509_SUCCESS);
}
#endif /* NX_SECURE_X509_DISABLE_CRL */
