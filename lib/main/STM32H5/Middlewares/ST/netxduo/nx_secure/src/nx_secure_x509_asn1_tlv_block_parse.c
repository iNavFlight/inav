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
/*    _nx_secure_x509_asn1_tlv_block_parse                PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses an ASN.1 type-length-value (TLV) block for use */
/*    by the X509 parser.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to data to be parsed  */
/*    tlv_type                              Return block type             */
/*    tlv_tag_class                         Return class of the tag       */
/*    tlv_length                            Return parsed length          */
/*    tlv_data                              Return pointer to block data  */
/*    header_length                         Return length of block itself */
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
/*    _nx_secure_x509_certificate_parse     Extract public key data       */
/*    _nx_secure_x509_certificate_revocation_list_parse                   */
/*                                          Parse revocation list         */
/*    _nx_secure_x509_certificate_verify    Verify a certificate          */
/*    _nx_secure_x509_crl_extensions_parse  Parse extensions in crl       */
/*    _nx_secure_x509_crl_issuer_parse      Parse issuer in crl           */
/*    _nx_secure_x509_crl_parse_entry       Parse an entry in crl         */
/*    _nx_secure_x509_crl_revoked_certs_list_parse                        */
/*                                          Parse revoked certificate list*/
/*                                            in crl                      */
/*    _nx_secure_x509_crl_signature_algorithm_parse                       */
/*                                          Parse signature algorithm in  */
/*                                            crl                         */
/*    _nx_secure_x509_crl_signature_data_parse                            */
/*                                          Parse signature data in crl   */
/*    _nx_secure_x509_crl_tbscert_list_parse                              */
/*                                          Parse TBSCertList in crl      */
/*    _nx_secure_x509_crl_update_times_parse                              */
/*                                          Parse Update times in crl     */
/*    _nx_secure_x509_crl_verify            Verify revocation list        */
/*    _nx_secure_x509_crl_version_parse     Parse version in crl          */
/*    _nx_secure_x509_extended_key_usage_extension_parse                  */
/*                                          Parse Extended KeyUsage       */
/*                                            extension                   */
/*    _nx_secure_x509_distinguished_name_parse                            */
/*                                          Parse Distinguished Name      */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*    _nx_secure_x509_extract_name_oid_data Extract Distinguished Name    */
/*    _nx_secure_x509_extract_oid_data      Extract OID data              */
/*    _nx_secure_x509_key_usage_extension_parse                           */
/*                                          Parse KeyUsage extension      */
/*    _nx_secure_x509_parse_cert_data       Parse certificate             */
/*    _nx_secure_x509_parse_extensions      Parse extension in certificate*/
/*    _nx_secure_x509_parse_issuer          Parse issuer in certificate   */
/*    _nx_secure_x509_parse_public_key      Parse public key in           */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_serial_num      Parse serial number in        */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_signature_algorithm                           */
/*                                          Parse signature algorithm in  */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_signature_data  Parse signature data in       */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_subject         Parse subject in certificate  */
/*    _nx_secure_x509_parse_unique_ids      Parse unique IDs in           */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_validity        Parse validity in certificate */
/*    _nx_secure_x509_parse_version         Parse version in certificate  */
/*    _nx_secure_x509_pkcs1_rsa_private_key_parse                         */
/*                                          Parse RSA key (PKCS#1 format) */
/*    _nx_secure_x509_pkcs7_decode          Decode the PKCS#7 signature   */
/*    _nx_secure_x509_policy_qualifiers_parse                             */
/*                                          Parse policy qualifiers       */
/*    _nx_secure_x509_subject_alt_names_find                              */
/*                                          Find subject alt names        */
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
UINT _nx_secure_x509_asn1_tlv_block_parse(const UCHAR *buffer, ULONG *buffer_length, USHORT *tlv_type,
                                          USHORT *tlv_tag_class, ULONG *tlv_length,
                                          const UCHAR **tlv_data, ULONG *header_length)
{
UINT   current_index;
USHORT current_tag;
ULONG  length;
ULONG  length_bytes;

    current_index = 0;
    current_tag = buffer[current_index];

    if (*buffer_length < 1)
    {
        return(NX_SECURE_X509_ASN1_LENGTH_TOO_LONG);
    }

    /*  Handle multi-byte encoded tags. */
    if ((current_tag & NX_SECURE_ASN_TAG_MULTIBYTE_MASK) == NX_SECURE_ASN_TAG_MULTIBYTE_MASK)
    {
        return(NX_SECURE_X509_MULTIBYTE_TAG_UNSUPPORTED);
    }
    else
    {
        *header_length = 1;
        *buffer_length = *buffer_length - 1;
    }

    /* Get the class of the tag so we can return it. */
    *tlv_tag_class = (USHORT)((current_tag & NX_SECURE_ASN_TAG_CLASS_MASK) >> 6);

    /* Make sure we have a valid tag class. */
    if (*tlv_tag_class == NX_SECURE_ASN_TAG_CLASS_APPLICATION ||
        *tlv_tag_class == NX_SECURE_ASN_TAG_CLASS_PRIVATE)
    {
        /* The tag class is invalid, return error. */
        return(NX_SECURE_X509_INVALID_TAG_CLASS);
    }

    /* The caller actually handles what happens based on the tag type. */
    if (current_tag & NX_SECURE_ASN_TAG_CONSTRUCTED_MASK)
    {
        current_tag = current_tag & (USHORT)(~NX_SECURE_ASN_TAG_CONSTRUCTED_MASK);
    }

    /* Clear out the class and constructed bits before returning the tag value. */
    *tlv_type = current_tag & NX_SECURE_ASN_TAG_MASK;
    current_index++;

    if (*buffer_length < 1)
    {
        return(NX_SECURE_X509_ASN1_LENGTH_TOO_LONG);
    }

    if (current_tag == NX_SECURE_ASN_TAG_NULL)
    {
        /*  If tag is NULL, there is no length byte, just a value of zero, */
        *tlv_length = 1;

        /* Set the data pointer and return. */
        *tlv_data = &buffer[current_index];

        return(NX_SECURE_X509_SUCCESS);
    }

    /* Handle the length. */
    length = buffer[current_index];
    current_index++;
    *header_length = *header_length + 1;
    *buffer_length = *buffer_length - 1;

    /* Check for multi-byte length by looking at the top bit of the length byte. */
    if (length & 0x80)
    {
        /* Multi-byte length:
           > 127, high bit is set, and lower 7 bits becomes the number of following bytes of *length*
           so 841 bytes of Value is encoded as 0x82, 0x03, 0x49 (0x82 = 2 bytes of length, 0x0349 = 841).
         */

        /*  Mask off top bit to get the number of bytes in length. */
        length_bytes = length & 0x7F;
        length = 0;

        /*  Check for length too big to handle. */
        if (length_bytes > 4 || length_bytes > *buffer_length)
        {

            return(NX_SECURE_X509_ASN1_LENGTH_TOO_LONG);
        }

        /* Update header length. */
        *header_length = *header_length + length_bytes;
        *buffer_length = *buffer_length - length_bytes;

        while (length_bytes > 0)
        {
            /* Shift length one byte up and add in next byte. */
            length <<= 8;
            length += buffer[current_index];

            /* Advance our index by one byte. */
            current_index++;
            length_bytes--;
        }
    }
    else
    {
        /* Single-byte length:
           <= 127 (7 bits), length is the number of bytes of Value */
        *tlv_length = length;
    }

    if (length > *buffer_length)
    {
        return(NX_SECURE_X509_ASN1_LENGTH_TOO_LONG);
    }

    *buffer_length = *buffer_length - length;

    /* Set the length to return to caller. */
    *tlv_length = length;

    /*  Now, we can set the tld value */
    *tlv_data = &buffer[current_index];

    return(NX_SECURE_X509_SUCCESS);
}

