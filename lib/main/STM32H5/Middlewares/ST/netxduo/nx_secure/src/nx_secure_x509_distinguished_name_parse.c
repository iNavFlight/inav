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

static UINT _nx_secure_x509_extract_name_oid_data(const UCHAR *buffer, UINT oid, ULONG length,
                                                  UINT *bytes_processed,
                                                  NX_SECURE_X509_DISTINGUISHED_NAME *name);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_parse            PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded X.509 Distinguished Name in the  */
/*    supplied buffer, placing the parsed data into a structure that is   */
/*    returned to the caller.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to data to be parsed  */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return number of bytes parsed */
/*    name                                  Return parsed name structure  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_asn1_tlv_block_parse  Parse ASN.1 block             */
/*    _nx_secure_x509_extract_name_oid_data Extract Distinguished Name    */
/*    _nx_secure_x509_oid_parse             Parse OID in certificate      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_issuer_parse      Parse issuer in crl           */
/*    _nx_secure_x509_parse_issuer          Parse issuer in certificate   */
/*    _nx_secure_x509_parse_subject         Parse subject in certificate  */
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
UINT _nx_secure_x509_distinguished_name_parse(const UCHAR *buffer, UINT length, UINT *bytes_processed,
                                              NX_SECURE_X509_DISTINGUISHED_NAME *name)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
UINT         bytes;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;
UINT         current_index;
const UCHAR *sequence_ptr;
ULONG        sequence_length;
UINT         oid;

    /* Parse distinguished name information including Common Name, country/region, organization, etc... */
    /*
     *          Sequence (16): Name information - parsed above this point in hierarchy
     *             Set (17): Information (multiple sets here - one per OID)
     *                 Sequence (16): Information (multiple sequences here?)
     *                     ASN.1 OID (6)   (e.g. Country/Region)
     *                     ASN.1 String (UTF8-12, Printable-19) (e.g. "US")
     *             ...
     */

    current_index = 0;
    *bytes_processed = 0;

    /*  Continue parsing ASN.1 sets until we reach the end of the surrounding sequence, determined
     *  by the "length" parameter. */
    while (length > 0)
    {
        /*  First, parse the set. */
        status = _nx_secure_x509_asn1_tlv_block_parse(&buffer[current_index], (ULONG *)&length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        if (tlv_type != NX_SECURE_ASN_TAG_SET || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
        {
            return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
        }

        /*  Advance the top-level buffer pointer to the next SET entry. */
        current_index += (header_length + tlv_length);
        *bytes_processed += (header_length + tlv_length);
        sequence_length = tlv_length;

        /*  Next, parse the sequence. */
        status = _nx_secure_x509_asn1_tlv_block_parse(tlv_data, &sequence_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        if (tlv_type != NX_SECURE_ASN_TAG_SEQUENCE || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
        {
            return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
        }

        /*  Save the sequence for the calls below. */
        sequence_ptr = tlv_data;
        sequence_length = tlv_length;

        /*  Now we have the actual data. First extract the OID.*/
        status = _nx_secure_x509_asn1_tlv_block_parse(sequence_ptr, &sequence_length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);

        /*  Make sure we parsed the block alright. */
        if (status != 0)
        {
            return(status);
        }

        if (tlv_type != NX_SECURE_ASN_TAG_OID || tlv_type_class != NX_SECURE_ASN_TAG_CLASS_UNIVERSAL)
        {
            return(NX_SECURE_X509_UNEXPECTED_ASN1_TAG);
        }

        /* The OID is in the data we extracted. */
        _nx_secure_x509_oid_parse(tlv_data, tlv_length, &oid);

        /* Extract the data in the block following the OID. Use the remaining length of the sequence
         * for the length of the data going in. */
        status = _nx_secure_x509_extract_name_oid_data(&tlv_data[tlv_length], oid, sequence_length, &bytes, name);

        if (status != 0)
        {
            return(status);
        }
    }

    /* All done parsing id info! */
    return(NX_SECURE_X509_SUCCESS);
}


/* This function extracts name identification data from the certificate distinguished name fields (subject and issuer)
   by matching OIDs and populating the correct fields in the distinguished name structure. */
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_extract_name_oid_data               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded X.509 Distinguished Name in the  */
/*    supplied buffer, placing the parsed data into a structure that is   */
/*    returned to the caller.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    buffer                                Pointer to data to be parsed  */
/*    oid                                   OID data to be parsed         */
/*    length                                Length of data in buffer      */
/*    bytes_processed                       Return number of bytes parsed */
/*    name                                  Return parsed name structure  */
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
/*    _nx_secure_x509_distinguished_name_parse                            */
/*                                          Parse Distinguished Name      */
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
static UINT _nx_secure_x509_extract_name_oid_data(const UCHAR *buffer, UINT oid, ULONG length,
                                                  UINT *bytes_processed,
                                                  NX_SECURE_X509_DISTINGUISHED_NAME *name)
{
USHORT       tlv_type;
USHORT       tlv_type_class;
ULONG        tlv_length;
const UCHAR *tlv_data;
ULONG        header_length;
UINT         status;

    status = _nx_secure_x509_asn1_tlv_block_parse(buffer, &length, &tlv_type, &tlv_type_class, &tlv_length, &tlv_data, &header_length);
    if (status != 0)
    {
        return(status);
    }

    *bytes_processed = (header_length + tlv_length);

    switch (oid)
    {
    case NX_SECURE_TLS_X509_TYPE_COMMON_NAME:
        name -> nx_secure_x509_common_name = tlv_data;
        name -> nx_secure_x509_common_name_length = (USHORT)tlv_length;
        break;
    case NX_SECURE_TLS_X509_TYPE_COUNTRY:
        name -> nx_secure_x509_country = tlv_data;
        name -> nx_secure_x509_country_length = (USHORT)tlv_length;
        break;
    case NX_SECURE_TLS_X509_TYPE_STATE:
        name -> nx_secure_x509_state = tlv_data;
        name -> nx_secure_x509_state_length = (USHORT)tlv_length;
        break;
    case NX_SECURE_TLS_X509_TYPE_ORGANIZATION:
        name -> nx_secure_x509_organization = tlv_data;
        name -> nx_secure_x509_organization_length = (USHORT)tlv_length;
        break;
    case NX_SECURE_TLS_X509_TYPE_ORG_UNIT:
        name -> nx_secure_x509_org_unit = tlv_data;
        name -> nx_secure_x509_org_unit_length = (USHORT)tlv_length;
        break;
#ifdef NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
    /* When using extended distinguished names, parse them here. */
    case NX_SECURE_TLS_X509_TYPE_LOCALITY:
        name -> nx_secure_x509_locality = tlv_data;
        name -> nx_secure_x509_locality_length = (USHORT)tlv_length;
        break;
#endif
    default:
        /* In the case of unrecognized OIDs, just return success. There are numerous
           extensions and their presence is not a problem if we don't recognize them. */
        break;
    }

    /* We have successfully extracted our data based on the OID. */
    return(NX_SECURE_X509_SUCCESS);
}

