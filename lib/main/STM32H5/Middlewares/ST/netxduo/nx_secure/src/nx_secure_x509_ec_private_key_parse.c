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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_ec_private_key_parse                PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded private EC key for use with      */
/*    X509 certificates.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer data to be parsed     */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return bytes processed        */
/*    ec_key                                Return EC key structure       */
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
/*    _nx_secure_x509_certificate_initialize                              */
/*                                          Initialize certificate        */
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
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            ignored public key in EC    */
/*                                            private key,                */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_ec_private_key_parse(const UCHAR *buffer, UINT length,
                                          UINT *bytes_processed,
                                          NX_SECURE_EC_PRIVATE_KEY *ec_key)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
ULONG        seq_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;
USHORT       version;


    /* Parse an ASN.1 DER-encoded EC private key file. */

    /* From RFC 5915.                                                          */
    /* ECPrivateKey ::= SEQUENCE {                                             */
    /*     version           INTEGER { ecPrivkeyVer1(1) } (ecPrivkeyVer1),     */
    /*     privateKey        OCTET STRING,                                     */
    /*     parameters [0]    ECParameters {{ NamedCurve }} OPTIONAL,           */
    /*     publicKey  [1]    BIT STRING OPTIONAL                               */
    /* }                                                                       */
    /*                                                                         */
    /* ECParameters ::= CHOICE {                                               */
    /*   namedCurve          OBJECT IDENTIFIER                                 */
    /*   -- implicitCurve    NULL                                              */
    /*   -- specifiedCurve  SpecifiedECDomain                                  */

    /*  Parse a TLV block and get information to continue parsing. */
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

    /* First item in the EC key sequence is a version field. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_INTEGER || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* Version shall be one. */
    version = tlv_data[0];

    if (version != 0x01)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];


    /* Parse our next field, the private key. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_OCTET_STRING || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The private key is an octet string, so no padding bytes are needed (as with a BITSTRING). */
    if (ec_key != NULL)
    {
        ec_key -> nx_secure_ec_private_key = tlv_data;
        ec_key -> nx_secure_ec_private_key_length = (USHORT)tlv_length;
    }

    /* Advance our working pointer past the last field. */
    tlv_data = &tlv_data[tlv_length];

    /* Parse our next field, the EC parameter. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != 0 || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_CONTEXT)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += header_length;
    seq_length = tlv_length;

    /* Parse the namedCurve. */
    status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    if (tlv_type != NX_SECURE_ASN_TAG_OID || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_PKCS1_INVALID_PRIVATE_KEY);
    }

    /* Update byte count. */
    *bytes_processed += (header_length + tlv_length);

    /* The value is an OID. */
    if (ec_key != NULL)
    {
        /* The OID is in the data we extracted. */
        _nx_secure_x509_oid_parse(tlv_data, tlv_length, &ec_key -> nx_secure_ec_named_curve);
    }

    /* The optional public key is ignored.  */
    return(NX_SECURE_X509_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
