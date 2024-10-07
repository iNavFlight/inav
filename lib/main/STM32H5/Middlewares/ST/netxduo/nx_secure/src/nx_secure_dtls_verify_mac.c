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
static UCHAR _generated_hash[NX_SECURE_TLS_MAX_HASH_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_verify_mac                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the Message Authentication Code (MAC) that   */
/*    is included in encrypted DTLS records. It hashes the incoming       */
/*    message data and then compares it to the MAC in the received        */
/*    record. If there is a mismatch, then the record has been corrupted  */
/*    in transit and represents a possible security breach.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    header_data                           DTLS record header data       */
/*    header_length                         Length of header data         */
/*    data                                  DTLS record payload data      */
/*    length                                Length of payload data        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_hash_record           Generate payload data hash    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
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
UINT _nx_secure_dtls_verify_mac(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *header_data,
                                USHORT header_length, UCHAR *data, UINT *length)
{
UCHAR                                *mac_secret;
USHORT                                hash_size;
UINT                                  status;
INT                                   compare_result;
USHORT                                data_length;
UCHAR                                *received_hash;
UINT                                  hash_length;
UCHAR                                 header[20];
NX_SECURE_TLS_SESSION                *tls_session;

    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Select our proper MAC secret for hashing. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* If we are a server, we need to use the client's MAC secret. */
        mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_mac_secret;
    }
    else
    {
        /* We are a client, so use the server's MAC secret. */
        mac_secret = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_mac_secret;
    }

    if (hash_size >= *length)
    {
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

    /* Adjust the length in the header to match data without hash. */

    /* In DTLS, the length is at offset 11. */
    header[11] = (UCHAR)((data_length >> 8) & 0x00FF);
    header[12] = (UCHAR)(data_length & 0x00FF);

    /* Generate the hash on the plaintext data. */
    status = _nx_secure_dtls_hash_record(dtls_session, tls_session -> nx_secure_tls_remote_sequence_number, header, header_length,
                                         data, (USHORT)(data_length), _generated_hash, &hash_length, mac_secret);

    if (status != NX_SUCCESS)
    {
        /* The hash operation failed for some reason. */
        return(NX_SECURE_TLS_HASH_MAC_VERIFY_FAILURE);
    }

    /* In DTLS, the sequence number is explicit in the record. In TLS the sequence number would be incremented here. */

    /* Now, compare the hash we generated to the one we received. */
    received_hash = &data[data_length];
    compare_result = NX_SECURE_MEMCMP(received_hash, _generated_hash, hash_size);

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
#endif /* NX_SECURE_ENABLE_DTLS */

