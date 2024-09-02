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
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
extern NX_CRYPTO_METHOD crypto_method_ec_secp256;
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_dtls_send_clienthello_sec_spf_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length, ULONG available_size);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello                    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with a ClientHello message,    */
/*    which kicks off a DTLS handshake when sent to a remote DTLS server. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    send_packet                           Outgoing TLS packet           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_handshake_hash_init    Initialize the finished hash  */
/*    _nx_secure_tls_newest_supported_version                             */
/*                                          Get the version of DTLS to use*/
/*    [nx_crypto_init]                      Crypto initialization         */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    [nx_secure_tls_session_time_function] Get the current time for the  */
/*                                            TLS timestamp               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nx_secure_dtls_session_start                                        */
/*    nx_secure_dtls_client_handshake                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated cookie handling,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_send_clienthello(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet)
{
ULONG  length;
UINT   gmt_time;
UINT   random_value;
UINT   i;
USHORT ciphersuite;
USHORT ciphersuites_length;

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
USHORT                      extension_length;
USHORT                      extension_type;
UINT                        status;
NX_SECURE_TLS_PSK_STORE    *psk_store;
NX_CRYPTO_EXTENDED_OUTPUT   extended_output;
#endif
#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
UCHAR                      *extension_total_length_ptr;
USHORT                      extension_total_length;
const NX_CRYPTO_METHOD     *crypto_method = NX_NULL;
#endif
UCHAR                      *packet_buffer;
NX_SECURE_TLS_SESSION      *tls_session;
const NX_SECURE_TLS_CRYPTO *crypto_table;
USHORT                      protocol_version;

    /* ClientHello structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |   2    | <CS Len>     |    1    | <Comp Len>  |    2    | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | CS Len | Ciphersuites |Comp Len | Compression |Ext. Len | Extensions |
     */

    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    packet_buffer = send_packet -> nx_packet_prepend_ptr;

    /* Initialize the handshake hashes used for the Finished message. */
    _nx_secure_tls_handshake_hash_init(tls_session);

    /* At this point, the remote session is not active - that is, incoming records are not encrypted. */
    tls_session -> nx_secure_tls_remote_session_active = 0;

    /* Since we are establishing a new session, we start in non-encrypted mode. */
    tls_session -> nx_secure_tls_local_session_active = 0;

    /* Length of the ciphersuites list that follows. The client can send any number of suites for the
     * server to choose from, with a 2-byte length field. */
    crypto_table = tls_session -> nx_secure_tls_crypto_table;
    ciphersuites_length = (USHORT)(2 + (crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size << 1));

    if ((10u + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random) +
         tls_session -> nx_secure_tls_session_id_length + dtls_session -> nx_secure_dtls_cookie_length + ciphersuites_length) >
        ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_prepend_ptr)))
    {

        /* Packet buffer is too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* First two bytes of the hello following the header are the DTLS major and minor version numbers. */
    _nx_secure_tls_protocol_version_get(tls_session, &protocol_version, NX_SECURE_DTLS);

    if (protocol_version == 0x0)
    {
        /* Error, no versions enabled. */
        return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
    }
    tls_session -> nx_secure_tls_protocol_version = protocol_version;

    if (dtls_session -> nx_secure_dtls_cookie_length > 0)
    {

        /* Skip record header. */
        send_packet -> nx_packet_prepend_ptr += (NX_SECURE_DTLS_RECORD_HEADER_SIZE +
                                                 NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE);
        length = send_packet -> nx_packet_length - (NX_SECURE_DTLS_RECORD_HEADER_SIZE +
                                                    NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE);

        /* Locate the buffer to add cookie. */
        packet_buffer = send_packet -> nx_packet_prepend_ptr +
            (2 + sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random) +
             1 + tls_session -> nx_secure_tls_session_id_length);

        /* Put the length into the record. */
        packet_buffer[0] = (UCHAR)dtls_session -> nx_secure_dtls_cookie_length;
        packet_buffer++;

        /* Move the data to insert cookie. */
        NX_SECURE_MEMMOVE(packet_buffer + dtls_session -> nx_secure_dtls_cookie_length,
                packet_buffer, (UINT)(send_packet -> nx_packet_append_ptr - packet_buffer)); /* Use case of memmove is verified.  */

        /* Set cookie. */
        NX_SECURE_MEMCPY(packet_buffer, dtls_session -> nx_secure_dtls_client_cookie_ptr,
               dtls_session -> nx_secure_dtls_cookie_length); /* Use case of memcpy is verified. */
        length += dtls_session -> nx_secure_dtls_cookie_length;

        /* Finally, we have a complete length and can put it into the buffer. Before that,
           save off and return the number of bytes we wrote and need to send. */
        send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_prepend_ptr + length;
        send_packet -> nx_packet_length = length;

        return(NX_SUCCESS);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

    packet_buffer[length]     = (UCHAR)((protocol_version & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(protocol_version & 0x00FF);
    length += 2;

    /* Set the Client random data, used in key generation. First 4 bytes is GMT time. */
    gmt_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        gmt_time = tls_session -> nx_secure_tls_session_time_function();
    }
    NX_CHANGE_ULONG_ENDIAN(gmt_time);
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, (UCHAR *)&gmt_time, sizeof(gmt_time)); /* Use case of memcpy is verified. */

    /* Next 28 bytes is random data. */
    for (i = 0; i < 28; i += (UINT)sizeof(random_value))
    {
        random_value = (UINT)NX_RAND();
        NX_CHANGE_ULONG_ENDIAN(random_value);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random + (i + 4))     = (UCHAR)(random_value);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random + (i + 5)) = (UCHAR)(random_value >> 8);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random + (i + 6)) = (UCHAR)(random_value >> 16);
        *(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random + (i + 7)) = (UCHAR)(random_value >> 24);
    }

    /* Copy the random data into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random,
                     sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random)); /* Use case of memcpy is verified. */
    length += sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random);

    /* Session ID length is one byte. */
    packet_buffer[length] = tls_session -> nx_secure_tls_session_id_length;
    length++;

    /* Session ID follows. */
    if (tls_session -> nx_secure_tls_session_id_length > 0)
    {
        NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_session_id, tls_session -> nx_secure_tls_session_id_length); /* Use case of memcpy is verified. */
        length += tls_session -> nx_secure_tls_session_id_length;
    }

    /* Cookie length - 0 for first client hello */
    packet_buffer[length] = 0;
    length += 1;

    /* Length of the ciphersuites list that follows. The client can send any number of suites for the
     * server to choose from, with a 2-byte length field. */
    packet_buffer[length] = (UCHAR)((ciphersuites_length & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ciphersuites_length & 0x00FF);
    length += 2;

    /* Our chosen ciphersuites. In sending a ClientHello, we need to include all the ciphersuites
     * we are interested in speaking. This will be a list of ciphersuites the user has chosen to support
     * and all of them will be presented to the remote server in this initial message. */

    for (i = 0; i < crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size; i++)
    {
        ciphersuite = crypto_table -> nx_secure_tls_ciphersuite_lookup_table[i].nx_secure_tls_ciphersuite;
        packet_buffer[length] = (UCHAR)((ciphersuite & 0xFF00) >> 8);
        packet_buffer[length + 1] = (UCHAR)(ciphersuite & 0x00FF);
        length += 2;

#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
        if (crypto_table -> nx_secure_tls_ciphersuite_lookup_table[i].nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
        {
            crypto_method = crypto_table -> nx_secure_tls_ciphersuite_lookup_table[i].nx_secure_tls_public_auth;
        }
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE || NX_SECURE_ENABLE_ECC_CIPHERSUITE */
    }

    /* For the secure rengotiation indication extension, we need to send a
       Signalling Ciphersuite Value (SCSV) as detailled in RFC5746. */
    ciphersuite = TLS_EMPTY_RENEGOTIATION_INFO_SCSV;
    packet_buffer[length] = (UCHAR)((ciphersuite & 0xFF00) >> 8);
    packet_buffer[length + 1] = (UCHAR)(ciphersuite & 0x00FF);
    length += 2;

    /* Compression methods length - one byte. For now we only have the NULL method, for a length of 1. */
    packet_buffer[length] = 0x1;
    length++;

    /* Compression methods - for now this is NULL. */
    packet_buffer[length] = 0x0;
    length++;

    /* TLS extensions, once supported, will be here in the ClientHello. */

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    if (crypto_method)
    {
        if (crypto_method -> nx_crypto_init == NX_NULL)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count == 0)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_prepend_ptr)) < (20u + length))
        {

            /* Packet buffer is too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        psk_store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0];
        status = crypto_method -> nx_crypto_init((NX_CRYPTO_METHOD*)crypto_method,
                                                 psk_store -> nx_secure_tls_psk_data,
                                                 (USHORT)(psk_store -> nx_secure_tls_psk_data_size << 3),
                                                 NX_NULL,
                                                 tls_session -> nx_secure_public_auth_metadata_area,
                                                 tls_session -> nx_secure_public_auth_metadata_size);
        if (status)
        {
            return(status);
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_HASH_METHOD_SET,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)crypto_method,
                                                      NX_NULL,
                                                      (USHORT)(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size << 3),
                                                      (UCHAR *)(tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method),
                                                      sizeof(NX_CRYPTO_METHOD),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CURVE_SET,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)crypto_method,
                                                      NX_NULL, 0,
                                                      (UCHAR *)&crypto_method_ec_secp256,
                                                      sizeof(NX_CRYPTO_METHOD),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        extension_total_length_ptr = &packet_buffer[length];
        extension_total_length = 18;
        length += 2;

        /* Supported Elliptic Curves Extension.  */
        extension_type = NX_SECURE_TLS_EXTENSION_EC_GROUPS;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* Supported Groups Length: 4.  */
        extension_length = 4;
        packet_buffer[length] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_length & 0xFF);
        length += 2;

        /* Supported Groups List Length: 2.  */
        extension_length = 2;
        packet_buffer[length] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_length & 0xFF);
        length += 2;

        /* Supported Group: secp256r1.  */
        extension_type = (USHORT)NX_CRYPTO_EC_SECP256R1;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* ec_point_formats Extension.  */
        extension_type = NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* ec_point_formats Length: 2.  */
        extension_length = 2;
        packet_buffer[length] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_length & 0xFF);
        length += 2;

        /* EC point formats Length: 1.  */
        packet_buffer[length] = 1;
        length += 1;

        /* EC point format: uncompressed(0).  */
        packet_buffer[length] = 0;
        length += 1;

        /* ecjpake_key_kp_pair Extension.  */
        extension_type = NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR;
        packet_buffer[length] = (UCHAR)((extension_type >> 8) & 0xFF);
        packet_buffer[length + 1] = (UCHAR)(extension_type & 0xFF);
        length += 2;

        /* ecjpake_key_kp_pair Length.  */
        length += 2;
        extended_output.nx_crypto_extended_output_data = &packet_buffer[length];
        extended_output.nx_crypto_extended_output_length_in_byte =
            (ULONG)send_packet -> nx_packet_data_end - (ULONG)&packet_buffer[length];
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CLIENT_HELLO_GENERATE,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)crypto_method,
                                                      NX_NULL, 0,
                                                      NX_NULL, 0, NX_NULL,
                                                      (UCHAR *)&extended_output,
                                                      sizeof(extended_output),
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        /* Set length for ecjpake_key_kp_pair extension. */
        extension_length = (USHORT)extended_output.nx_crypto_extended_output_actual_size;
        packet_buffer[length - 2] = (UCHAR)((extension_length >> 8) & 0xFF);
        packet_buffer[length - 1] = (UCHAR)(extension_length & 0xFF);

        extension_total_length = (USHORT)(extension_total_length +
                                          extended_output.nx_crypto_extended_output_actual_size);
        length += extended_output.nx_crypto_extended_output_actual_size;

        /* Set extension total length. */
        extension_total_length_ptr[0] = (UCHAR)((extension_total_length >> 8) & 0xFF);
        extension_total_length_ptr[1] = (UCHAR)(extension_total_length & 0xFF);
    }
#endif

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    if (crypto_method == NX_NULL)
    {
        extension_total_length_ptr = &packet_buffer[length];
        extension_total_length = 18;
        length += 2;

        _nx_secure_dtls_send_clienthello_sec_spf_extensions(&dtls_session -> nx_secure_dtls_tls_session,
                                                            packet_buffer, &length, &extension_total_length,
                                                            ((ULONG)send_packet -> nx_packet_data_end -
                                                             (ULONG)packet_buffer));
        if (extension_total_length == 0)
        {

            /* No extensions, remove the extensions length. */
            length -= 2;
        }
        else
        {

            /* Set extension total length. */
            extension_total_length_ptr[0] = (UCHAR)((extension_total_length >> 8) & 0xFF);
            extension_total_length_ptr[1] = (UCHAR)(extension_total_length & 0xFF);
        }
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

    /* Finally, we have a complete length and can put it into the buffer. Before that,
       save off and return the number of bytes we wrote and need to send. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_prepend_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    /* We are starting a new handshake, so both local and remote sessions are not
     * active. */
    dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_remote_session_active = 0;
    dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_local_session_active = 0;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello_sec_spf_extensions PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Supported Elliptic Curves extension and the  */
/*    Supported Point Formats extension to an outgoing ClientHello        */
/*    record. See RFC 4492 section 5.1.                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_offset                         Offset into packet buffer     */
/*    extension_length                      Return length of data         */
/*    available_size                        Available size of buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello      Send TLS ClientHello extension*/
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
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_dtls_send_clienthello_sec_spf_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
ULONG  offset;
USHORT ext_len, list_len, ext, i;
NX_SECURE_TLS_ECC *ecc_info;

    ecc_info = &(tls_session -> nx_secure_tls_ecc);

    if (ecc_info -> nx_secure_tls_ecc_supported_groups_count == 0)
    {
        *extension_length = 0;
        return(NX_SUCCESS);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    if (available_size < (offset + 12u + (ULONG)(ecc_info -> nx_secure_tls_ecc_supported_groups_count << 1)))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Supported Elliptic Curves Extension. */
    ext = NX_SECURE_TLS_EXTENSION_EC_GROUPS;  /* Supported Groups */
    list_len = (USHORT)(ecc_info -> nx_secure_tls_ecc_supported_groups_count * sizeof(USHORT));
    ext_len = (USHORT)(list_len + 2);

    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((ext_len & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext_len & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((list_len & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(list_len & 0x00FF);
    offset += 2;

    for (i = 0; i < ecc_info -> nx_secure_tls_ecc_supported_groups_count; i++)
    {
        packet_buffer[offset] = (UCHAR)((ecc_info -> nx_secure_tls_ecc_supported_groups[i] & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(ecc_info -> nx_secure_tls_ecc_supported_groups[i] & 0x00FF);
        offset += 2;
    }

    /* ec_point_formats Extension.  */
    ext = NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS;
    ext_len = 2; /* ec_point_formats Length: 2.  */

    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    packet_buffer[offset] = (UCHAR)((ext_len & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext_len & 0x00FF);
    offset += 2;

    packet_buffer[offset] = 1;
    offset += 1;

    packet_buffer[offset] = 0;
    offset += 1;

    /* Return our updated packet offset. */
    *extension_length = (USHORT)(offset - *packet_offset);
    *packet_offset = offset;

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#endif /* NX_SECURE_ENABLE_DTLS */

