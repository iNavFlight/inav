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
/*    _nx_secure_x509_extended_key_usage_extension_parse  PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses through an X.509 certificate for an Extended   */
/*    KeyUsage extension for a a specific key usage provided by the       */
/*    caller. If the extension is not found or if the specified usage is  */
/*    not present in the extension, an error is returned.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to X.509 certificate  */
/*    key_usage                             Key usage OID to find         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*    _nx_secure_x509_oid_parse             Parse OID in certificate      */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_extended_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate,
                                                        UINT key_usage)
{
USHORT                   tlv_type;
USHORT                   tlv_type_class;
ULONG                    tlv_length;
const UCHAR             *tlv_data;
const UCHAR             *current_buffer;
ULONG                    length;
ULONG                    header_length;
UINT                     status;
UINT                     usage_oid;
NX_SECURE_X509_EXTENSION key_usage_extension;

    /* Find and parse the keyUsage extension. */
    /* ExtendedKeyUsage ASN.1 format:

       id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 }

       ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId

       KeyPurposeId ::= OBJECT IDENTIFIER

     */

    /* Find the KeyUsage extension in the certificate. */
    status = _nx_secure_x509_extension_find(certificate, &key_usage_extension, NX_SECURE_TLS_X509_TYPE_EXTENDED_KEY_USAGE);

    /* See if extension present - it is OK if not present! */
    if (status != NX_SECURE_X509_SUCCESS)
    {
        return(status);
    }

    /* The length of our extensions is the length of the sequence. */
    current_buffer = key_usage_extension.nx_secure_x509_extension_data;
    length = key_usage_extension.nx_secure_x509_extension_data_length;

    /*  Parse the sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

    /*  Make sure we parsed the block alright. */
    if (status != 0)
    {
        return(status);
    }

    /* If the next item up is not a sequence, then it isn't an extensions block. */
    if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_UNIVERSAL && tlv_type == NX_SECURE_ASN_TAG_SEQUENCE))
    {
        /* We were expecting a bitfield but got something else. */
        return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
    }

    /* The names are in the body of the sequence structure, so use our tlv_data and length. */
    current_buffer = tlv_data;
    length = tlv_length;

    /* Loop through OIDs to find the one we are looking for. */
    while (length > 0)
    {
        /*  First, parse the OID tag (if it exists). */
        status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        /* If the sequence does not contain OIDs, error. */
        if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_UNIVERSAL && tlv_type == NX_SECURE_ASN_TAG_OID))
        {
            return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
        }

        current_buffer += header_length;

        /* The OID is in the data we extracted. */
        _nx_secure_x509_oid_parse(tlv_data, tlv_length, &usage_oid);

        /* If our OID matches the passed-in key usage OID type, success! */
        if (usage_oid == key_usage)
        {
            return(NX_SECURE_X509_SUCCESS);
        }

        current_buffer += tlv_length;
    }

    /* Got through the entire list but didn't find the specified key usage. */
    return(NX_SECURE_X509_EXT_KEY_USAGE_NOT_FOUND);
}

