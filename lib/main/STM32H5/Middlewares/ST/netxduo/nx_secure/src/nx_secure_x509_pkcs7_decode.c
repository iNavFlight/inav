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
/**    X.509 Digital Certificates - PKCS#7 parsing                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_pkcs7_decode                        PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decodes a PKCS#7 (RFC 5652) certificate signature     */
/*    and returns a pointer to the encapsulated hash for signature        */
/*    verification by the caller. Also returned is the OID for the        */
/*    signature algorithm.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    signature_pointer                     Pointer to PCKS#7 signature   */
/*    signature_length                      Length of entire signagure    */
/*    signature_oid                         Pointer to signature OID      */
/*    signature_oid_length                  Return length of OID          */
/*    hash_data                             Pointer to hash data          */
/*    hash_length                           Return length of hash         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Signature validity status     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_verify    Verify a certificate          */
/*    _nx_secure_x509_crl_verify            Verify revocation list        */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
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
UINT _nx_secure_x509_pkcs7_decode(const UCHAR *signature_pointer, UINT signature_length,
                                  const UCHAR **signature_oid, UINT *signature_oid_length,
                                  const UCHAR **hash_data, UINT *hash_length)
{
UINT         i;
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data = NX_CRYPTO_NULL;
ULONG        header_length;
ULONG        seq_length;
UINT         status;
const UCHAR *signature_data = NX_CRYPTO_NULL;
ULONG        remaining_length;

    /* Certificate signatures encrypted with RSA use PKCS#7 encoding (RFC 5652) which
     * has the following format:
     *    1 byte header (0x00)
     *    Padding type byte (should be < 2)
     *    <Padding>
     *    1 byte padding terminator (always 0x00)
     *    ASN.1 encoded signature
     *       Signature Sequence
     *           OID sequence
     *              signature algorithm OID(s) (can be multiple)
     *              NULL ASN.1 object terminator
     *           Octet string of hash value
     *    1 byte end padding (0x01)
     */

    signature_data = signature_pointer;
    remaining_length = signature_length;

    /* Check padding type. */
    if (signature_data[1] >= 2)
    {
        /* Invalid PKCS#7 encoding or decryption failure. */
        return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
    }

    /* Loop through padding - skip the first 2 bytes of padding type.
     * Also ensure that we don't run past the end of the buffer if
     * the NULL terminator isn't found. */
    i = 2;    
    while (i < signature_length)
    {
        if (signature_data[i] == 0x00)
        {
            break;
        }
        i++;
    }

    /* Skip over the padding null-terminator. */
    i++;

    /* Make sure we actually saw a NULL byte. */
    if (i >= signature_length)
    {
        return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
    }

    /* Advance our working pointer. */
    signature_data = &signature_data[i];
    remaining_length -= i;

    /* Now we have our ASN.1-encoded signature. */
    status = _nx_secure_x509_asn1_tlv_block_parse(signature_data, &remaining_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed a proper ASN.1 sequence. */
    if (status != 0 || tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
    }

    /* Advance our working pointer and adjust remaining length. */
    signature_data = tlv_data;
    remaining_length = tlv_length;

    /* Next up is the OID sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(signature_data, &remaining_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed a proper ASN.1 sequence. */
    if (status != 0 || tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
    }
    signature_data = tlv_data;
    seq_length = tlv_length;

    /* Next we parse the OID(s). */
    do
    {
        /* Parse at least 1 OID. */
        status = _nx_secure_x509_asn1_tlv_block_parse(signature_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed a proper ASN.1 sequence. */
        if (status != 0 || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
        {
            return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
        }

        /* Adjust our buffer pointer. */
        signature_data = &signature_data[tlv_length + header_length];

        /* If we see a NULL tag, we are at the end of the list. */
        if (tlv_type == NX_SECURE_ASN_TAG_NULL)
        {
            break;
        }

        /* Save off the OID. */
        *signature_oid = tlv_data;
        *signature_oid_length = tlv_length;
    } while (tlv_type == NX_SECURE_ASN_TAG_OID);

    /* Finally, we should be at the signature hash itself. */
    status = _nx_secure_x509_asn1_tlv_block_parse(signature_data, &remaining_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed a proper ASN.1 sequence. */
    if (status != 0 || tlv_type != NX_SECURE_ASN_TAG_OCTET_STRING || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
    {
        return(NX_SECURE_X509_PKCS7_PARSING_FAILED);
    }

    /* Return pointer to hash data and its length. */
    *hash_data = tlv_data;
    *hash_length = tlv_length;

    /* Signature is valid. */
    return(NX_SECURE_X509_SUCCESS);
}

