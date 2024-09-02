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
/*    _nx_secure_x509_extension_find                      PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses through the list of extensions in an X.509     */
/*    certificate looking for a specific extension. The data in the       */
/*    extension is returned in a structure for use in specific extension  */
/*    handling functions.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Certificate to search         */
/*    extension                             Return instruction data       */
/*    extension_id                          Extension to search for       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_oid_parse             Parse OID in certificate      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_x509_common_name_dns_check Check Common Name by DNS      */
/*    _nx_secure_x509_extended_key_usage_extension_parse                  */
/*                                          Parse Extended KeyUsage       */
/*                                            extension                   */
/*    _nx_secure_x509_key_usage_extension_parse                           */
/*                                          Parse KeyUsage extension      */
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
UINT _nx_secure_x509_extension_find(NX_SECURE_X509_CERT *certificate,
                                    NX_SECURE_X509_EXTENSION *extension, USHORT extension_id)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
ULONG        extensions_sequence_length;
ULONG        seq_length;
UINT         extension_oid = 0;
USHORT       critical_flag;
const UCHAR *tlv_data;
const UCHAR *current_buffer;
ULONG        header_length;
UINT         status;
UINT         found_extension = NX_CRYPTO_FALSE;

    /* Now, parse the extensions. */
    /* Extension ASN.1 format:
     *   ASN.1 Sequence: Extension (single)
     *    {
     *       OID: Extension identifier
     *       BOOLEAN: Critical (default=False)
     *       OCTET STRING: DER-encoded specific extension data
     *    }
     */

    /* The length of our extensions is the length of the sequence. */
    current_buffer = certificate -> nx_secure_x509_extensions_data;
    extensions_sequence_length = certificate -> nx_secure_x509_extensions_data_length;

    /* Keep looping until we run out of data to parse. */
    while (extensions_sequence_length > 0)
    {
        /*  Next, parse the single extension sequence. */
        status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &extensions_sequence_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        /* If the next item up is not a sequence, then it isn't an extensions block. */
        if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_UNIVERSAL && tlv_type == NX_SECURE_ASN_TAG_SEQUENCE))
        {
            /* The extensions sequence isn't empty and we should be seeing another extension sequence
               but we got something else so something is amiss. */
            return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
        }

        /* Advance our buffer to the next item. We are now in the sequence so advance buffer after this
           instead of pointing to tlv_data. */
        current_buffer += header_length + tlv_length;
        seq_length = tlv_length;

        /* Parse the OID. */
        status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);
        if (status != 0)
        {
            return status;
        }

        /* Now check out what we got. Should be an OID... */
        if (tlv_type == NX_SECURE_ASN_TAG_OID)
        {
            /* The OID is in the data we extracted. */
            _nx_secure_x509_oid_parse(tlv_data, tlv_length, &extension_oid);
        }
        else
        {
            /* Invalid extension, end parsing. */
            return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
        }

        /* See if we found a match. */
        if (extension_oid == extension_id)
        {
            /* Advance the working pointer to the data immediately following the OID. */
            current_buffer = tlv_data + tlv_length;

            /* We found a matching extension, break out and populate the return structure. */
            found_extension = NX_CRYPTO_TRUE;
            break;
        }
    } /* End while loop. */

    /* Did we find the extension we were looking for? */
    if (found_extension != NX_CRYPTO_TRUE)
    {
        return(NX_SECURE_X509_EXTENSION_NOT_FOUND);
    }

    /* If we get here, we found a matching extension. */
    extension -> nx_secure_x509_extension_id = (USHORT)extension_oid;

    /* Parse the Critical boolean. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);
    if (status != 0)
    {
        return status;
    }

    /* Now check out what we got. Possibly a boolean for the critical flag, but
       if it isn't there, then it's default is FALSE. */
    critical_flag = NX_CRYPTO_FALSE;
    if (tlv_type == NX_SECURE_ASN_TAG_BOOLEAN)
    {
        /* The boolean is in the data we extracted. Convert ASN.1 boolean TRUE (all bits set) to integer 1. */
        critical_flag = tlv_data[0] != 0;

        /* Advance our buffer to the next item. */
        current_buffer += header_length + tlv_length;

        /* Parse the octet string containing the DER-encoded extension data. */
        status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &seq_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);
        if (status != 0)
        {
            return status;
        }
    }
    extension -> nx_secure_x509_extension_critical = critical_flag;

    /* Now check out what we got. Should be an octet string... */
    if (tlv_type != NX_SECURE_ASN_TAG_OCTET_STRING)
    {
        /* Invalid extension, end parsing. */
        return(NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE);
    }

    /* Save off a pointer to the extension data (format determined by actual extension type). */
    extension -> nx_secure_x509_extension_data = tlv_data;
    extension -> nx_secure_x509_extension_data_length = tlv_length;

#if 0
    /* Dispatch based on OID to individual parsing routines. */
    switch (extension_oid)
    {
    case NX_SECURE_TLS_X509_TYPE_AUTHORITY_KEY_ID:
        /*printf("Got authority key ID extension.\n");*/
        break;
    case NX_SECURE_TLS_X509_TYPE_SUBJECT_KEY_ID:
        /*printf("Got subject key ID extension.\n");*/
        break;
    case NX_SECURE_TLS_X509_TYPE_BASIC_CONSTRAINTS:
        /*printf("Got basic constraints extension. \n");*/
        break;
    case NX_SECURE_TLS_X509_TYPE_NETSCAPE_COMMENT:
        /*printf("Got Netscape comment extension. \n");*/
        break;
    case NX_SECURE_TLS_X509_TYPE_UNKNOWN:
        /* Unknown extension, ignore. */
        /*printf("Unknown extension OID\n");*/
        break;
    default:
        /* Unsupported extension, ignore. */
        /*printf("Unsupported extension OID: %d\n", extension_oid);*/
        break;
    }
#endif


    return(NX_SECURE_X509_SUCCESS);
}

