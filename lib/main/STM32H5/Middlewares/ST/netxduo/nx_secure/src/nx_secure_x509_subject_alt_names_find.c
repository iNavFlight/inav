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
/*    _nx_secure_x509_subject_alt_names_find              PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses through the list of names in an X.509          */
/*    subjectAltName extension, looking for a particular name. This is    */
/*    typically used to see if a DNS name is present in a subjectAltName  */
/*    extension if the Common Name did not match.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    extension                             subjectAltName extension data */
/*    name                                  Name to search for            */
/*    name_length                           Length of name                */
/*    name_type                             Type of name                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_wildcard_compare      Wildcard compare for names    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_common_name_dns_check Check Common Name by DNS      */
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
UINT _nx_secure_x509_subject_alt_names_find(NX_SECURE_X509_EXTENSION *extension, const UCHAR *name,
                                            UINT name_length, USHORT name_type)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
const UCHAR *current_buffer;
ULONG        length;
ULONG        header_length;
UINT         status;
const UCHAR *compare_name;
ULONG        compare_length;
INT          compare_value;

    /* Now, parse the subjectAltName extension. */
    /* subjectAltName ASN.1 format:
        SubjectAltName ::= GeneralNames

        GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName

        GeneralName ::= CHOICE {
             otherName                 [0]  AnotherName,
             rfc822Name                [1]  IA5String,
             dNSName                   [2]  IA5String,
             x400Address               [3]  ORAddress,
             directoryName             [4]  Name,
             ediPartyName              [5]  EDIPartyName,
             uniformResourceIdentifier [6]  IA5String,
             iPAddress                 [7]  OCTET STRING,
             registeredID              [8]  OBJECT IDENTIFIER }

        AnotherName ::= SEQUENCE {
             type-id    OBJECT IDENTIFIER,
             value      [0] EXPLICIT ANY DEFINED BY type-id }

        EDIPartyName ::= SEQUENCE {
             nameAssigner              [0]  DirectoryString OPTIONAL,
             partyName                 [1]  DirectoryString }
     */

    /* The length of our extensions is the length of the sequence. */
    current_buffer = extension -> nx_secure_x509_extension_data;
    length = extension -> nx_secure_x509_extension_data_length;

    /*  First, parse the name sequence. */
    status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

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

    /* The names are in the body of the sequence structure, so use our tlv_data and length. */
    current_buffer = tlv_data;
    length = tlv_length;


    /* Keep looping until we run out of data to parse. */
    while (length > 0)
    {
        /*  First, parse the context-specific tag (if it exists). */
        status = _nx_secure_x509_asn1_tlv_block_parse(current_buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        /* If the next item up is not a context-sensitive tag, then not a valid subjectAltName. */
        if (!(tlv_type_class == NX_SECURE_ASN_TAG_CLASS_CONTEXT))
        {
            /* No extensions block is OK because it is non-existent in v1 and v2, and
               OPTIONAL in v3. */
            return(NX_SECURE_X509_ALT_NAME_NOT_FOUND);
        }

        current_buffer += header_length;

        /* If the name type we are searching for doesn't match what we just parsed,
           continue to the next entry. */
        if (tlv_type != name_type)
        {
            continue;
        }

        /* Process the name type we found. */
        switch (tlv_type)
        {
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_DNSNAME:
            /* Now we have an IA5 string to compare against our name. */
            compare_name = tlv_data;
            compare_length = tlv_length;
            break;
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_OTHERNAME:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_RFC822NAME:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_X400ADDRESS:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_DIRECTORYNAME:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_EDIPARTYNAME:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_UNIFORMRESOURCEIDENTIFIER:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_IPADDRESS:
        case NX_SECURE_X509_SUB_ALT_NAME_TAG_REGISTEREDID:
        default:
            /* Deliberate fall-through. These name types are not supported. */
            continue;
        }

        /* Compare the names, using wildcard matching. */
        compare_value =  _nx_secure_x509_wildcard_compare(name, name_length,
                                                          compare_name, compare_length);

        if (compare_value == 0)
        {
            /* We found a match! */
            return(NX_SECURE_X509_SUCCESS);
        }

        current_buffer += tlv_length;
    } /* End while-loop. */

    return(NX_SECURE_X509_ALT_NAME_NOT_FOUND);
}

