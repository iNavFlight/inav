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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

static UCHAR _generated_hash[NX_SECURE_TLS_MAX_HASH_SIZE];
static UCHAR _received_hash[NX_SECURE_TLS_MAX_HASH_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_verify_mac                               PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the Message Authentication Code (MAC) that   */
/*    is included in encrypted TLS records. It hashes the incoming        */
/*    message data and then compares it to the MAC in the received        */
/*    record. If there is a mismatch, then the record has been corrupted  */
/*    in transit and represents a possible security breach.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    mac_secret                            Key used for MAC generation   */
/*    sequence_num                          Record sequence number        */
/*    header_data                           TLS record header data        */
/*    header_length                         Length of header data         */
/*    packet_ptr                            TLS record packet             */
/*    offset                                Offset to TLS record in packet*/
/*    length                                Length of payload data        */
/*    hash_mac_metadata                     Metadata for hash mac crypto  */
/*    hash_mac_metadata_size                Size of hash mac metadata     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_hash_record            Generate payload data hash    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_verify_mac             Verify record MAC checksum    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_verify_mac(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, UCHAR *mac_secret, ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                           UCHAR *header_data, USHORT header_length, NX_PACKET *packet_ptr, ULONG offset, UINT *length,
                           VOID *hash_mac_metadata, ULONG hash_mac_metadata_size)
{

USHORT hash_size;
INT    compare_result;
USHORT data_length;
UINT   hash_length;
UCHAR  header[6];
ULONG  bytes_copied;



    /* Get the hash size and MAC secret for our current session. */
    hash_size = ciphersuite -> nx_secure_tls_hash_size;

    /* Check for 0-length records. With nothing to hash, don't continue to hash generation. */
    if (hash_size >= *length)
    {

        if (header_data[0] == (UCHAR)(NX_SECURE_TLS_APPLICATION_DATA) &&
            *length == hash_size)
        {
            /* BEAST attack mitigation. In TLS 1.0 and SSLv3, the implicit IV enables the BEAST
               attack. Some implementations counter the attack by sending an empty record which
               has the effect of resetting the IVs. We normally don't allow empty records since there
               is no data to hash, but in this case we want to allow it. */
            *length = 0;

            /* Increment the sequence number. */
            if ((sequence_num[0] + 1) == 0)
            {
                /* Check for overflow of the 32-bit unsigned number. */
                sequence_num[1]++;

                if (sequence_num[1] == 0)
                {

                    /* Check for overflow of the 64-bit unsigned number. As it should not reach here
                       in practical, we return a general error to prevent overflow theoretically. */
                    return(NX_NOT_SUCCESSFUL);
                }

            }
            sequence_num[0]++;

            return(NX_SUCCESS);
        }

        /* The record data was smaller than the selected hash... Error. */
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }

    /* Adjust our length so we only hash the record data, not the hash as well. */
    data_length = (USHORT)(*length - hash_size);

    /* Copy the header data into our local buffer so we can change it if we need to. */
    if (header_length > sizeof(header))
    {
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }
    NX_SECURE_MEMCPY(header, header_data, header_length); /* Use case of memcpy is verified. */

    /* Adjust the length in the header to match the length of the data before the hash was added. */
    header[3] = (UCHAR)((data_length >> 8) & 0x00FF);
    header[4] = (UCHAR)(data_length & 0x00FF);

    /* Generate the hash on the plaintext data. */
    _nx_secure_tls_hash_record(ciphersuite, sequence_num, header, header_length,
                               packet_ptr, offset, data_length, _generated_hash, &hash_length, mac_secret,
                               hash_mac_metadata, hash_mac_metadata_size);

    /* Increment the sequence number. */
    if ((sequence_num[0] + 1) == 0)
    {
        /* Check for overflow of the 32-bit unsigned number. */
        sequence_num[1]++;

        if (sequence_num[1] == 0)
        {

            /* Check for overflow of the 64-bit unsigned number. As it should not reach here
               in practical, we return a general error to prevent overflow theoretically. */
            return(NX_NOT_SUCCESSFUL);
        }

    }
    sequence_num[0]++;

    if (hash_size == 0)
    {

        /* For ciphersuite without explict hash, just return success. */
        return(NX_SECURE_TLS_SUCCESS);
    }

    /* Now, compare the hash we generated to the one we received. */
    if (nx_packet_data_extract_offset(packet_ptr,
                                      offset + data_length,
                                      _received_hash, hash_size, &bytes_copied))
                                      
    {

        /* The record data was smaller than the selected hash... Error. */
        return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
    }

    if (bytes_copied != hash_size)
    {

        /* The record data was smaller than the selected hash... Error. */
        return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
    }

    compare_result = NX_SECURE_MEMCMP(_received_hash, _generated_hash, hash_size);

#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_generated_hash, 0, sizeof(_generated_hash));
#endif /* NX_SECURE_KEY_CLEAR  */

    /* Before we return, adjust our data size so the caller will only see data, not the hash. */
    *length = data_length;

    /* If the hashes match, we are all good. Otherwise we have a problem. */
    if (compare_result == 0)
    {
        return(NX_SECURE_TLS_SUCCESS);
    }
    else
    {
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }
}

