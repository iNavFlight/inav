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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_hash_record                          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function hashes an outgoing TLS record to generate the Message */
/*    Authentication Code (MAC) value that is placed at the end of all    */
/*    encrypted TLS messages.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    sequence_num                          Record sequence number        */
/*    header                                Record header                 */
/*    header_length                         Length of record header       */
/*    packet_ptr                            TLS record packet             */
/*    offset                                Offset to TLS record in packet*/
/*    length                                Length of payload data        */
/*    record_hash                           Pointer to output hash buffer */
/*    hash_length                           Length of hash                */
/*    mac_secret                            Key used for MAC generation   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_verify_mac             Verify record MAC checksum    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            supported chained packet,   */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            fixed compiler warning,     */
/*                                            resulting in version 6.1.7  */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            adjusted parameters list,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_hash_record(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite,
                                ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                UCHAR *header, UINT header_length, NX_PACKET *packet_ptr,
                                ULONG offset, UINT length, UCHAR *record_hash, UINT *hash_length,
                                UCHAR *mac_secret, VOID *metadata, ULONG metadata_size)
{
UINT                    hash_size;
UINT                    status = NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE;
const NX_CRYPTO_METHOD *authentication_method;
UCHAR                   adjusted_sequence_num[8];
VOID                   *handler = NX_NULL;
ULONG                   current_length;

    NX_PARAMETER_NOT_USED(header_length);

    /* We need to generate a Message Authentication Code (MAC) for each record during an "active" TLS session
       (following a ChangeCipherSpec message). The hash algorithm is determined by the ciphersuite, and HMAC
       is used with that hash algorithm to protect the TLS record contents from tampering.

       The MAC is generated as:

       HMAC_hash(MAC_write_secret, seq_num + type + version + length + data);

     */

    /* Get our authentication method from the ciphersuite. */
    authentication_method = ciphersuite -> nx_secure_tls_hash;

    /* Get the hash size and MAC secret for our current session. */
    hash_size = ciphersuite -> nx_secure_tls_hash_size;

    /* Correct the endianness of our sequence number before hashing. */
    adjusted_sequence_num[0] = (UCHAR)(sequence_num[1] >> 24);
    adjusted_sequence_num[1] = (UCHAR)(sequence_num[1] >> 16);
    adjusted_sequence_num[2] = (UCHAR)(sequence_num[1] >> 8);
    adjusted_sequence_num[3] = (UCHAR)(sequence_num[1]);
    adjusted_sequence_num[4] = (UCHAR)(sequence_num[0] >> 24);
    adjusted_sequence_num[5] = (UCHAR)(sequence_num[0] >> 16);
    adjusted_sequence_num[6] = (UCHAR)(sequence_num[0] >> 8);
    adjusted_sequence_num[7] = (UCHAR)(sequence_num[0]);

    if (authentication_method -> nx_crypto_init)
    {
        status = authentication_method -> nx_crypto_init((NX_CRYPTO_METHOD*)authentication_method,
                                                mac_secret,
                                                (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                &handler,
                                                metadata,
                                                metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* TLS header type, version, and length are in the proper order. */
    if (authentication_method -> nx_crypto_operation != NX_NULL)
    {
        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                     handler,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     mac_secret,
                                                     (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     handler,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     adjusted_sequence_num,
                                                     8,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     handler,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     header,
                                                     5,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Locate to start packet of TLS record payload. */
        while (packet_ptr)
        {
            current_length = (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
            if (offset >= current_length)
            {

                /* Move to next packet. */
                offset -= current_length;
                packet_ptr = packet_ptr -> nx_packet_next;
            }
            else
            {

                /* Found offset in current packet. */
                break;
            }
        }

        /* Hash TLS record payload. */
        while ((length > 0) && packet_ptr)
        {
            current_length = (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
            current_length -= offset;
            offset = 0;
            if (current_length > length)
            {
                current_length = length;
            }
            status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                        handler,
                                                        (NX_CRYPTO_METHOD*)authentication_method,
                                                        NX_NULL,
                                                        0,
                                                        &packet_ptr -> nx_packet_prepend_ptr[offset],
                                                        current_length,
                                                        NX_NULL,
                                                        NX_NULL,
                                                        0,
                                                        metadata,
                                                        metadata_size,
                                                        NX_NULL,
                                                        NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            length -= current_length;
            packet_ptr = packet_ptr -> nx_packet_next;
        }

        if (length > 0)
        {

            /* Not all TLS record payload is hashed. */
            return(NX_SECURE_TLS_INVALID_PACKET);
        }

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                     handler,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     record_hash,
                                                     NX_SECURE_TLS_MAX_HASH_SIZE,
                                                     metadata,
                                                     metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(adjusted_sequence_num, 0, 8);
#endif /* NX_SECURE_KEY_CLEAR  */
    }

    if (authentication_method -> nx_crypto_cleanup)
    {
        status = authentication_method -> nx_crypto_cleanup(metadata);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}

