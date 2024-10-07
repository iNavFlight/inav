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
/*    _nx_secure_tls_verify_mac                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
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
/*    tls_session                           TLS control block             */
/*    header_data                           TLS record header data        */
/*    header_length                         Length of header data         */
/*    packet_ptr                            TLS record packet             */
/*    offset                                Offset to TLS record in packet*/
/*    length                                Length of payload data        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_verify_mac]                Verify record MAC checksum    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_record         Process TLS record data       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), fixed    */
/*                                            AES-CBC padding oracle,     */
/*                                            verified memcpy use cases,  */
/*                                            supported chained packet,   */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            reorganized internal logic, */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            checked seq number overflow,*/
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_verify_mac(NX_SECURE_TLS_SESSION *tls_session, UCHAR *header_data,
                               USHORT header_length, NX_PACKET *packet_ptr, ULONG offset, UINT *length)
{
UCHAR *mac_secret;
UINT status;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

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

    status = tls_session -> nx_secure_verify_mac(tls_session -> nx_secure_tls_session_ciphersuite, mac_secret, tls_session -> nx_secure_tls_remote_sequence_number,
                                                 header_data, header_length, packet_ptr, offset, length,
                                                 tls_session -> nx_secure_hash_mac_metadata_area, tls_session -> nx_secure_hash_mac_metadata_size);
    return(status);
}

