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
/* Helper functions. */
static UINT _nx_secure_x509_crl_tbscert_list_parse(const UCHAR *buffer, ULONG length,
                                                   UINT *bytes_processed, NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_signature_algorithm_parse(const UCHAR *buffer, ULONG length,
                                                          UINT *bytes_processed,
                                                          NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_signature_data_parse(const UCHAR *buffer, ULONG length,
                                                     UINT *bytes_processed, NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_version_parse(const UCHAR *buffer, ULONG length, UINT *bytes_processed,
                                              NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_issuer_parse(const UCHAR *buffer, ULONG length, UINT *bytes_processed,
                                             NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_update_times_parse(const UCHAR *buffer, ULONG length,
                                                   UINT *bytes_processed, NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_revoked_certs_list_parse(const UCHAR *buffer, ULONG length,
                                                         UINT *bytes_processed,
                                                         NX_SECURE_X509_CRL *crl);
static UINT _nx_secure_x509_crl_extensions_parse(const UCHAR *buffer, ULONG length,
                                                 UINT *bytes_processed, NX_SECURE_X509_CRL *crl);



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_certificate_revocation_list_parse   PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded X509 Certificate Revocation List */
/*    (CRL) with the express purpose of identifying whether the supplied  */
/*    certificate is present in the supplied CRL and therefore has been   */
/*    revoked by the issuing CA.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Certificate being verified    */
/*    crl                                   Pointer to CRL data           */
/*    crl_length                            Length of CRL data            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    certificate status                    NX_SUCCESS if cert is good    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_crl_signature_algorithm_parse                       */
/*                                          Parse signature algorithm in  */
/*    _nx_secure_x509_crl_signature_data_parse                            */
/*                                          Parse signature data in crl   */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
UINT _nx_secure_x509_certificate_revocation_list_parse(const UCHAR *buffer, UINT length,
                                                       UINT *bytes_processed,
                                                       NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
UINT         bytes;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    /*  ASN.1 for CRL structure from RFC 5280:
     *  CertificateList  ::=  SEQUENCE
     *  {
     *      tbsCertList          TBSCertList,
     *      signatureAlgorithm   AlgorithmIdentifier,
     *      signatureValue       BIT STRING
     *   }
     *
     *   TBSCertList  ::=  SEQUENCE
     *   {
     *       version                 Version OPTIONAL,
     *                                    -- if present, MUST be v2
     *       signature               AlgorithmIdentifier,
     *       issuer                  Name,
     *       thisUpdate              Time,
     *       nextUpdate              Time OPTIONAL,
     *       revokedCertificates     SEQUENCE OF SEQUENCE
     *       {
     *            userCertificate         CertificateSerialNumber,
     *            revocationDate          Time,
     *            crlEntryExtensions      Extensions OPTIONAL
     *                                     -- if present, version MUST be v2
     *       }  OPTIONAL,
     *       crlExtensions           [0]  EXPLICIT Extensions OPTIONAL
     *                                     -- if present, version MUST be v2
     *   }
     *
     *  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
     *
     *  CertificateSerialNumber  ::=  INTEGER
     *
     *  Time ::= CHOICE
     *  {
     *      utcTime        UTCTime,
     *      generalTime    GeneralizedTime
     *  }
     *
     *  Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
     *
     *  Extension  ::=  SEQUENCE
     *  {
     *      extnID      OBJECT IDENTIFIER,
     *      critical    BOOLEAN DEFAULT FALSE,
     *      extnValue   OCTET STRING
     *                   -- contains the DER encoding of an ASN.1 value
     *                   -- corresponding to the extension type identified
     *                   -- by extnID
     *  }
     *
     */

    /*  Parse a TLV block - first block should be the CertificateList. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_INVALID_CERTIFICATE_SEQUENCE);
    }

    *bytes_processed = header_length;
    length = tlv_length;

    /* Next block should be the TBSCertList. */
    status = _nx_secure_x509_crl_tbscert_list_parse(tlv_data, tlv_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    *bytes_processed += bytes;

    /*  Following the certificate data is the signature algorithm data. */
    tlv_data = &tlv_data[bytes];
    length -= bytes;
    status = _nx_secure_x509_crl_signature_algorithm_parse(tlv_data, length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    *bytes_processed += bytes;

    /* Finally, the signature itself is at the end of the CRL. */
    tlv_data = &tlv_data[bytes];
    length -= bytes;
    status = _nx_secure_x509_crl_signature_data_parse(tlv_data, length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Return the number of bytes we processed. */
    *bytes_processed += bytes;


    /* Successfully parsed an X509 CRL. */
    return(NX_SECURE_X509_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the top-level ASN.1 sequence of a Certificate  */
/*    Revocation List (CRL). The sequence contains the list of            */
/*    certificates that have been revoked by the providing issuer.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if CRL was parsed  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_crl_extensions_parse  Parse extensions in crl       */
/*    _nx_secure_x509_crl_issuer_parse      Parse issuer in crl           */
/*    _nx_secure_x509_crl_revoked_certs_list_parse                        */
/*                                          Parse revoked certificate list*/
/*    _nx_secure_x509_crl_signature_algorithm_parse                       */
/*                                          Parse signature algorithm in  */
/*                                            crl                         */
/*    _nx_secure_x509_crl_update_times_parse                              */
/*                                          Parse update times in crl     */
/*    _nx_secure_x509_crl_version_parse     Parse version in crl          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_revocation_list_parse                   */
/*                                          Parse revocation list         */
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
static UINT _nx_secure_x509_crl_tbscert_list_parse(const UCHAR *buffer, ULONG length,
                                                   UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
UINT         bytes;
UINT         cur_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    /*
     * TBSCertList  ::=  SEQUENCE
     * {
     *       version                 Version OPTIONAL,
     *                                    -- if present, MUST be v2
     *       signature               AlgorithmIdentifier,
     *       issuer                  Name,
     *       thisUpdate              Time,
     *       nextUpdate              Time OPTIONAL,
     *       revokedCertificates     SEQUENCE OF SEQUENCE
     *       {
     *            userCertificate         CertificateSerialNumber,
     *            revocationDate          Time,
     *            crlEntryExtensions      Extensions OPTIONAL
     *                                     -- if present, version MUST be v2
     *       }  OPTIONAL,
     *       crlExtensions           [0]  EXPLICIT Extensions OPTIONAL
     *                                     -- if present, version MUST be v2
     * }
     */


    /*  Parse the sequence container for TBSCertList. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* Buffer should contain the certificate data sequence. */
    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_INVALID_CERTIFICATE_DATA);
    }

    /* Save off a pointer to the data for later use in verification. Includes the complete TBSCertList structure. */
    crl -> nx_secure_x509_crl_verify_data = buffer;
    crl -> nx_secure_x509_crl_verify_data_length = (USHORT)(tlv_length + header_length);

    *bytes_processed = header_length;
    cur_length = tlv_length;


    /*  First up is the version data. */
    status = _nx_secure_x509_crl_version_parse(tlv_data, tlv_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Next, the signature algorithm. */
    tlv_data = &tlv_data[bytes];
    cur_length -= bytes;
    *bytes_processed += bytes;
    status = _nx_secure_x509_crl_signature_algorithm_parse(tlv_data, cur_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Next is the issuer data. */
    tlv_data = &tlv_data[bytes];
    cur_length -= bytes;
    *bytes_processed += bytes;
    status = _nx_secure_x509_crl_issuer_parse(tlv_data, cur_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Update times - both this and the next update. */
    tlv_data = &tlv_data[bytes];
    cur_length -= bytes;
    *bytes_processed += bytes;
    status = _nx_secure_x509_crl_update_times_parse(tlv_data, cur_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Parse the revoked certificates list. */
    tlv_data = &tlv_data[bytes];
    cur_length -= bytes;
    *bytes_processed += bytes;
    status = _nx_secure_x509_crl_revoked_certs_list_parse(tlv_data, cur_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    /* Parse extensions. */
    tlv_data = &tlv_data[bytes];
    cur_length -= bytes;
    *bytes_processed += bytes;


    /* The following fields are all optional, so stop parsing with success if
       they were not included. */
    if (cur_length == 0)
    {
        return(NX_SECURE_X509_SUCCESS);
    }

    status = _nx_secure_x509_crl_extensions_parse(tlv_data, cur_length, &bytes, crl);

    if (status != 0)
    {
        return(status);
    }

    *bytes_processed += bytes;

    /* Parsed X509 certificate data successfully. */
    return(NX_SECURE_X509_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_signature_algorithm_parse       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the signature algorithm field in an X.509 CRL. */
/*    The signature algorithm identifies which cryptographic routines     */
/*    were used to sign the CRL.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_oid_parse             Parse OID in certificate      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_revocation_list_parse                   */
/*                                          Parse revocation list         */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_signature_algorithm_parse(const UCHAR *buffer, ULONG length,
                                                          UINT *bytes_processed,
                                                          NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
UINT         oid;
ULONG        header_length;
UINT         status;
UCHAR        oid_found = NX_CRYPTO_FALSE;

    /* The signature algorithm is a sequence of OIDs that is terminated by a NULL ASN.1 tag. */
    *bytes_processed = 0;

    /*  First, parse the sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    /*  Limit the length to the sequence length. */
    length = tlv_length;

    /*  While the tag is not NULL and we don't run out of buffer, keep parsing OIDs. */
    *bytes_processed += header_length;
    while (length > 0)
    {
        status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);
        if (status != 0)
        {
            return status;
        }

        /*  Update bytes processed. */
        *bytes_processed += (header_length + tlv_length);

        /* Now check out what we got. */
        if (tlv_type == NX_SECURE_ASN_TAG_OID)
        {
            _nx_secure_x509_oid_parse(tlv_data, tlv_length, &oid);

            crl -> nx_secure_x509_crl_signature_algorithm = (USHORT)oid;

            oid_found = NX_CRYPTO_TRUE;
        }
        else if (tlv_type == NX_SECURE_ASN_TAG_NULL)
        {
            break;
        }
        else
        {
            return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
        }

        /* Advance the data pointer. */
        tlv_data = &tlv_data[tlv_length];
    }

    if (oid_found == NX_CRYPTO_TRUE)
    {
        return(NX_SECURE_X509_SUCCESS);
    }

    /* We were expecting a signature algorithm OID but didn't find one. */
    return(NX_SECURE_X509_MISSING_SIGNATURE_ALGORITHM);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_signature_data_parse            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the signature field of an X.509 CRL. The       */
/*    signature is used to verify that the CRL came from the actual       */
/*    issuer and is not a forgery by an attacker.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_revocation_list_parse                   */
/*                                          Parse revocation list         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_x509_crl_signature_data_parse(const UCHAR *buffer, ULONG length,
                                                     UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    /*  First, parse the sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_BIT_STRING || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    /* Save off a pointer to the signature data and its length. */
    /* Signature has a 0 byte at the front we need to skip.
     * This is due to the data being encoded as an ASN.1 bit string, which may
     * require padding bits to get to a multiple of 8 for byte alignment. The byte
     * represents the number of padding bits, but in X509 it should always be 0. */
    crl -> nx_secure_x509_crl_signature_data = tlv_data + 1;
    crl -> nx_secure_x509_crl_signature_data_length = tlv_length - 1;

    /* Return the number of bytes we processed. */
    *bytes_processed = header_length + tlv_length;

    return(NX_SECURE_X509_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_version_parse                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the version field of an X.509 CRL, giving the  */
/*    version of X.509 the CRL uses.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_version_parse(const UCHAR *buffer, ULONG length,
                                              UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    /*  Parse a TLV block and get information to continue parsing. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* Check to see if version exists. If the version is present it will have a type
     * of NX_SECURE_ASN_TAG_BER. Otherwise it is an X509v1 certificate or invalid
     * (determined higher up the call stack). */
    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        /* No Version field found, assume v1. */
        crl -> nx_secure_x509_crl_version = 0;
        *bytes_processed = 0;
        return(NX_SECURE_X509_SUCCESS);
    }

    crl -> nx_secure_x509_crl_version = (USHORT)tlv_data[0];

    /* Return the number of bytes we processed. */
    *bytes_processed = header_length + tlv_length;

    /* Always return success - this field is optional. */
    return(NX_SECURE_X509_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_issuer_parse                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the issuer distinguished name field in an      */
/*    X.509 CRL. The issuer is the CA entity that created and signed the  */
/*    CRL.                                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_distinguished_name_parse                            */
/*                                          Parse Distinguished Name      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_issuer_parse(const UCHAR *buffer, ULONG length,
                                             UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
UINT         bytes;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    /*  First, parse the sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
    }

    /* Now, parse the distinguished name. */
    status = _nx_secure_x509_distinguished_name_parse(tlv_data, tlv_length, &bytes, &crl -> nx_secure_x509_crl_issuer);

    /* Return the number of bytes we processed. */
    *bytes_processed = bytes + header_length;

    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_update_times_parse              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the update times field in an X.509 CRL. The    */
/*    update times provided indicate when the next CRL should be          */
/*    available for download.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_update_times_parse(const UCHAR *buffer, ULONG length,
                                                   UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;
const UCHAR *current_buffer;

    current_buffer = buffer;

    /* Parse the "thisUpdate" field. */
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

    /* Extract the data we need. */
    crl -> nx_secure_x509_crl_time_format = tlv_type;
    crl -> nx_secure_x509_crl_this_update = tlv_data;
    crl -> nx_secure_x509_crl_this_update_length =  (USHORT)tlv_length;

    /* Advance to our next item. */
    current_buffer = current_buffer + (tlv_length + header_length);

    /* Return our bytes processed. */
    *bytes_processed = header_length + tlv_length;

    /* Parse the optional "nextUpdate" field, if it exists. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* The "nextUpdate" field is optional, so if we don't parse a time type here, that is OK. */
    if ((tlv_type != NX_SECURE_ASN_TAG_UTC_TIME && tlv_type != NX_SECURE_ASN_TAG_GENERALIZED_TIME) || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        crl -> nx_secure_x509_crl_next_update = NX_CRYPTO_NULL;
        crl -> nx_secure_x509_crl_next_update_length = 0;
        return(NX_SECURE_X509_SUCCESS);
    }

    crl -> nx_secure_x509_crl_next_update = tlv_data;
    crl -> nx_secure_x509_crl_next_update_length = (USHORT)tlv_length;

    /* Return our bytes processed. */
    *bytes_processed += header_length + tlv_length;

    return(NX_SECURE_X509_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_revoked_certs_list_parse        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the revokedCertificates list from an X.509   */
/*    CRL. The certificates list is not parsed here, but a pointer to the */
/*    list is saved in the CRL information structure so later searches    */
/*    for revoked certificates can be done quickly.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_revoked_certs_list_parse(const UCHAR *buffer, ULONG length,
                                                         UINT *bytes_processed,
                                                         NX_SECURE_X509_CRL *crl)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;
const UCHAR *current_buffer;

    current_buffer = buffer;

    /*  Parse the sequence container for TBSCertList. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* Buffer should contain the certificate data sequence. */
    if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_INVALID_CERTIFICATE_DATA);
    }

    *bytes_processed = header_length + tlv_length;

    /* Save off a pointer to the beginning of the revoked certificates list for later use. */
    crl -> nx_secure_x509_crl_revoked_certs = tlv_data;
    crl -> nx_secure_x509_crl_revoked_certs_length = tlv_length;

    return(NX_SECURE_X509_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_crl_extensions_parse                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the extensions field in an X.509 CRL.          */
/*    Currently, no extensions are supported so the function simply       */
/*    advances the parsing position in the raw CRL data buffer.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to CRL data           */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes parsed           */
/*    crl                                   CRL information structure     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                NX_SUCCESS if parsing success */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
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
static UINT _nx_secure_x509_crl_extensions_parse(const UCHAR *buffer, ULONG length,
                                                 UINT *bytes_processed, NX_SECURE_X509_CRL *crl)
{
USHORT tlv_type;
USHORT tlv_type_class;
ULONG  tlv_length;
/*
   ULONG        extensions_sequence_length;
   UINT         extension_oid;
 */
const UCHAR *tlv_data;
const UCHAR *current_buffer;
ULONG        header_length;
UINT         status;

    NX_CRYPTO_PARAMETER_NOT_USED(bytes_processed);
    NX_CRYPTO_PARAMETER_NOT_USED(crl);

    current_buffer = buffer;

    /*  First, parse the context-specific tag (if it exists). */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* Make sure we update our bytes processed. */
    *bytes_processed = header_length + tlv_length;


    /* If the next item up is not a sequence, then it isn't an extensions block. */
    if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_CONTEXT && tlv_type == NX_SECURE_X509_CRL_TAG_EXTENSIONS))
    {
        /* No extensions block is OK because it is OPTIONAL. */
        return(NX_SECURE_X509_SUCCESS);
    }

    return(NX_SECURE_X509_SUCCESS);
}
#endif /* NX_SECURE_X509_DISABLE_CRL */
