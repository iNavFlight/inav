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
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

#ifdef NX_SECURE_ENABLE_DTLS
static UCHAR adjusted_sequence_num[8];
static UCHAR adjusted_header[5];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_hash_record                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function hashes an outgoing DTLS record to generate the Message*/
/*    Authentication Code (MAC) value that is placed at the end of all    */
/*    encrypted DTLS messages.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    sequence_num                          Record sequence number        */
/*    header                                Record header                 */
/*    header_length                         Length of record header       */
/*    data                                  Record payload                */
/*    length                                Length of record payload      */
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
/*    [nx_crypto_operation]                 Hash functions                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_record           Send DTLS records             */
/*    _nx_secure_dtls_verify_mac            Verify record MAC checksum    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_hash_record(NX_SECURE_DTLS_SESSION *dtls_session,
                                 ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                 UCHAR *header, UINT header_length, UCHAR *data, UINT length,
                                 UCHAR *record_hash, UINT *hash_length, UCHAR *mac_secret)
{
UINT                                  hash_size;
UINT                                  status = NX_SECURE_TLS_SUCCESS;
const NX_CRYPTO_METHOD               *authentication_method;
NX_SECURE_TLS_SESSION                *tls_session;

    NX_PARAMETER_NOT_USED(sequence_num);
    NX_PARAMETER_NOT_USED(header_length);

    /* We need to generate a Message Authentication Code (MAC) for each record during an "active" TLS session
       (following a ChangeCipherSpec message). The hash algorithm is determined by the ciphersuite, and HMAC
       is used with that hash algorithm to protect the TLS record contents from tampering.

       The MAC is generated as:

       HMAC_hash(MAC_write_secret, seq_num + type + version + length + data);

     */

    /* Get a reference to the internal TLS session. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our authentication method from the ciphersuite. */
    authentication_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* In DTLS, use the sequence number + epoch from the header. */
    NX_SECURE_MEMCPY(adjusted_sequence_num, &header[3], 8); /* Use case of memcpy is verified. */

    /* We need to extract the type, version, and length from the DTLS header. */
    adjusted_header[0] = header[0];  /* Type. */
    adjusted_header[1] = header[1];  /* Version. */
    adjusted_header[2] = header[2];
    adjusted_header[3] = header[11]; /* Length. */
    adjusted_header[4] = header[12];

    if (authentication_method -> nx_crypto_operation != NX_NULL)
    {
        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                     NX_NULL,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     mac_secret,
                                                     (NX_CRYPTO_KEY_SIZE)(hash_size << 3),
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     tls_session -> nx_secure_hash_mac_metadata_area,
                                                     tls_session -> nx_secure_hash_mac_metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     NX_NULL,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     adjusted_sequence_num,
                                                     8,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     tls_session -> nx_secure_hash_mac_metadata_area,
                                                     tls_session -> nx_secure_hash_mac_metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }  

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     NX_NULL,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     adjusted_header,
                                                     5,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     tls_session -> nx_secure_hash_mac_metadata_area,
                                                     tls_session -> nx_secure_hash_mac_metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                     NX_NULL,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     data,
                                                     length,
                                                     NX_NULL,
                                                     NX_NULL,
                                                     0,
                                                     tls_session -> nx_secure_hash_mac_metadata_area,
                                                     tls_session -> nx_secure_hash_mac_metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

        status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                     NX_NULL,
                                                     (NX_CRYPTO_METHOD*)authentication_method,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     0,
                                                     NX_NULL,
                                                     record_hash,
                                                     NX_SECURE_TLS_MAX_HASH_SIZE,
                                                     tls_session -> nx_secure_hash_mac_metadata_area,
                                                     tls_session -> nx_secure_hash_mac_metadata_size,
                                                     NX_NULL,
                                                     NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

    }

    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}
#endif /* NX_SECURE_ENABLE_DTLS */

