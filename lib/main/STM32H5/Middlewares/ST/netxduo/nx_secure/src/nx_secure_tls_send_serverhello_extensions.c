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

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
static UINT _nx_secure_tls_send_serverhello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);

static UINT _nx_secure_tls_send_serverhello_key_share_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
static UINT _nx_secure_tls_send_serverhello_psk_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, ULONG *packet_length,
                                                   USHORT *extension_length, ULONG available_size);
#endif

#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_extensions          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerHello extensions.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_offset                         Offset into packet buffer     */
/*    available_size                        Available size of buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_sec_reneg_extension                 */
/*                                          Send ServerHello Renegotiation*/
/*                                            extension                   */
/*    _nx_secure_tls_send_serverhello_ec_extension                        */
/*                                          Send ClientHello EC extension */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello       Send TLS ServerHello          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed unused code,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG available_size)
{
ULONG  length = *packet_offset;
UCHAR *extension_offset;
#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) || ((NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED))
USHORT extension_length = 0;
#endif /* !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) || (NX_SECURE_TLS_TLS_1_3_ENABLED)  */
USHORT total_extensions_length;
UINT   status = NX_SUCCESS;

#if defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) && (!NX_SECURE_TLS_TLS_1_3_ENABLED)
    NX_PARAMETER_NOT_USED(tls_session);
#endif /* defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) && (!NX_SECURE_TLS_TLS_1_3_ENABLED) */

    if (available_size < (*packet_offset + 2u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Save an offset to the beginning of the extensions so we can fill in the length
       once all the extensions are added. */
    extension_offset = &packet_buffer[length];

    /* The extensions length field is two bytes. */
    length += 2;
    total_extensions_length = 0;

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
    /* Renegotiation is deprecated in TLS 1.3 so don't send extension. */
    if(!tls_session->nx_secure_tls_1_3)
#endif
    {
        /* We have to add renegotiation extensions in both initial sessions and renegotiating sessions. */
        status = _nx_secure_tls_send_serverhello_sec_reneg_extension(tls_session, packet_buffer, &length,
                                                            &extension_length, available_size);
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */


#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
    if(tls_session->nx_secure_tls_1_3)
    {

        /* MUST send the supported versions extension in all TLS 1.3 ServerHellos. */
        status = _nx_secure_tls_send_serverhello_supported_versions_extension(tls_session, packet_buffer, &length,
                                                                              &extension_length, available_size);
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* MUST send the key share extension in all TLS 1.3 ServerHellos. */
        status = _nx_secure_tls_send_serverhello_key_share_extension(tls_session, packet_buffer, &length,
                                                                     &extension_length, available_size);

        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
        if(status != NX_SUCCESS)
        {
            return(status);
        }

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        if ((tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY) &&
            (tls_session->nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size))
        {

            /* Send the PSK extension for PSK key exchange modes. */
            status = _nx_secure_tls_send_serverhello_psk_extension(tls_session, packet_buffer, &length,
                                                                   &extension_length, available_size);

            total_extensions_length = (USHORT)(total_extensions_length + extension_length);
            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }
#endif
    }
#endif



    /* Make sure we actually wrote some extension data. If none, back up the packet pointer and length. */
    if (total_extensions_length == 0)
    {
        /* No extensions written, back up our pointer and length. */
        length -= 2;
    }
    else
    {
        /* Put the extensions length into the packet at our original offset and add
           the total to our packet length. */
        extension_offset[0] = (UCHAR)((total_extensions_length & 0xFF00) >> 8);
        extension_offset[1] = (UCHAR)(total_extensions_length & 0x00FF);
    }

    *packet_offset = length;

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_sec_reneg_extension PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Secure Renegotiation Indication extension    */
/*    to an outgoing ServerHello record if the ServerHello is part of a   */
/*    renegotiation handshake. See RFC 5746 for more information.         */
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
/*    _nx_secure_tls_send_serverhello_extensions                          */
/*                                          Send TLS ServerHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Secure Renegotiation Indication Extensions structure (for serverhello):
     * |     2      |           12        |         12           |
     * |  Ext Type  |  client_verify_data |  server_verify_data  |
     */
    /*  From RFC 5746:
        struct {
             opaque renegotiated_connection<0..255>;
         } RenegotiationInfo;

          The contents of this extension are specified as follows.

      -  If this is the initial handshake for a connection, then the
         "renegotiated_connection" field is of zero length in both the
         ClientHello and the ServerHello.  Thus, the entire encoding of the
         extension is ff 01 00 01 00.  The first two octets represent the
         extension type, the third and fourth octets the length of the
         extension itself, and the final octet the zero length byte for the
         "renegotiated_connection" field.

      -  For ClientHellos that are renegotiating, this field contains the
         "client_verify_data" specified in Section 3.1.

      -  For ServerHellos that are renegotiating, this field contains the
         concatenation of client_verify_data and server_verify_data.  For
         current versions of TLS, this will be a 24-byte value (for SSLv3,
         it will be a 72-byte value).
     */

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    if (available_size < (offset + 2u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION;

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* See if this is the initial handshake or not. */
    if (!tls_session -> nx_secure_tls_local_session_active)
    {
        /* The extension has zero data because this is an initial handshake. Send
           the encoded extension as documented in the RFC. */

        if (available_size < (offset + 3u))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Fill in the length of current extension. */
        packet_buffer[offset] = 0x00;
        packet_buffer[offset + 1] = 0x01;

        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset + 2] = 0x00;

        offset += 3;
    }
    else
    {
        /* Fill in the length of current extension. */
        if (available_size < (offset + 3u + 2 * NX_SECURE_TLS_FINISHED_HASH_SIZE))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        data_length = (NX_SECURE_TLS_FINISHED_HASH_SIZE * 2) + 1;
        packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
        offset += 2;

        /* The extension actually has a second length field of 1 byte that needs to be populated. */
        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset] = (UCHAR)((NX_SECURE_TLS_FINISHED_HASH_SIZE * 2) & 0x00FF);
        offset++;

        /* Copy the verify data into the packet. */
        NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
        NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_local_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
    }

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_offset);

    /* Return our updated packet offset. */
    *packet_offset = offset;


    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_supported_versions_extension        */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Supported Versions extension, added in TLS   */
/*    v1.3.                                                               */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
static UINT _nx_secure_tls_send_serverhello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length, ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;


    /* Supported Versions Extension structure:
     * |     2      |         2          |           2           |
     * |  Ext Type  |  Extension length  | Selected TLS Versions |
     */
    NX_PARAMETER_NOT_USED(tls_session);

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    if (available_size < (offset + 6u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SUPPORTED_VERSIONS;

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    data_length = (UINT)(2);

    /* Set the total extension length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;


    /* Set the version value. */
    packet_buffer[offset] = 0x03;  //(UCHAR)((NX_SECURE_TLS_VERSION_TLS_1_3 & 0xFF00) >> 8);
    packet_buffer[offset + 1] = 0x04; //(UCHAR)(NX_SECURE_TLS_VERSION_TLS_1_3 & 0x00FF);
    offset += 2;

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_offset);

    /* Return our updated packet offset. */
    *packet_offset = offset;


    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_key_share_extension PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Key Share extension, added in TLS            */
/*    v1.3.                                                               */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send TLS ClientHello extension*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compilation issue with      */
/*                                            TLS 1.3 and disabling TLS   */
/*                                            server,                     */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
static UINT _nx_secure_tls_send_serverhello_key_share_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED            
ULONG  offset;
ULONG  length_offset;
USHORT ext;
UINT   i;
UINT   entry_length;
UINT   key_length;
UINT   data_length;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
USHORT named_curve;

    /* Key Share Extension structure (From TLS 1.3 RFC 8446):
     *  struct {
     *     NamedGroup group;
     *     opaque key_exchange<1..2^16-1>;
     *  } KeyShareEntry;
     *
     *  In a ServerHello message, the "extension_data" field of this
     *  extension contains a KeyShareServerHello value:
     *
     *     struct {
     *         KeyShareEntry server_share;
     *     } KeyShareServerHello;
     *
     *  server_share:  A single KeyShareEntry value that is in the same group
     *     as one of the client's shares.
     *
     *  If using (EC)DHE key establishment, servers offer exactly one
     *  KeyShareEntry in the ServerHello.  This value MUST be in the same
     *  group as the KeyShareEntry value offered by the client that the
     *  server has selected for the negotiated key exchange.  Servers
     *  MUST NOT send a KeyShareEntry for any group not indicated in the
     *  client's "supported_groups" extension and MUST NOT send a
     *  KeyShareEntry when using the "psk_ke" PskKeyExchangeMode.  If using
     *  (EC)DHE key establishment and a HelloRetryRequest containing a
     *  "key_share" extension was received by the client, the client MUST
     *  verify that the selected NamedGroup in the ServerHello is the same as
     *  that in the HelloRetryRequest.  If this check fails, the client MUST
     *  abort the handshake with an "illegal_parameter" alert.
     *
     *  Diffie-Hellman [DH] parameters for both clients and servers are
     *  encoded in the opaque key_exchange field of a KeyShareEntry in a
     *  KeyShare structure.  The opaque value contains the Diffie-Hellman
     *  public value (Y = g^X mod p) for the specified group (see [RFC7919]
     *  for group definitions) encoded as a big-endian integer and padded to
     *  the left with zeros to the size of p in bytes.
     *
     *  ECDHE structure for key_exchange (For secp256r1, secp384r1 and secp521r1)
     *  struct {
     *     uint8 legacy_form = 4;
     *     opaque X[coordinate_length];
     *     opaque Y[coordinate_length];
     *  } UncompressedPointRepresentation;
     *
     *   X and Y respectively are the binary representations of the x and y
     *   values in network byte order.  There are no internal length markers,
     *   so each number representation occupies as many octets as implied by
     *   the curve parameters.  For P-256 this means that each of X and Y use
     *   32 octets, padded on the left by zeros if necessary.  For P-384 they
     *   take 48 octets each, and for P-521 they take 66 octets each.
     *
     *  The X,Y point is the public key (Q) which is generated using the following:
     *  -  The public key to put into the KeyShareEntry.key_exchange
     *     structure is the result of applying the ECDH scalar multiplication
     *     function to the secret key of appropriate length (into scalar
     *     input) and the standard public basepoint (into u-coordinate point
     *     input).
     *
     *  -  The ECDH shared secret is the result of applying the ECDH scalar
     *     multiplication function to the secret key (into scalar input) and
     *     the peer's public key (into u-coordinate point input).  The output
     *     is used raw, with no processing.
     *
     *  NamedGroup examples: secp256r1(0x0017), secp384r1(0x0018), secp521r1(0x0019)
     * |     2      |         2          |       2      |    2    |    <key_len>      |
     * |  Ext Type  |  Extension length  |  Named Group | key_len | key_exchange data |
     */

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    if (available_size < (offset + 2u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_KEY_SHARE;

    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    i = tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data_selected;
    ecdhe_data = &(tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[i]);
    named_curve = (USHORT)(ecdhe_data -> nx_secure_tls_ecdhe_named_curve);
    if (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY)
    {

        if (available_size < (offset + 4u))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Put the fixed length into the packet. */
        packet_buffer[offset] = 0;
        packet_buffer[offset + 1] = 2;
        offset += 2;

        /* Put the selected group into the packet. */
        packet_buffer[offset] = (UCHAR)((named_curve & 0xFF00) >> 8);;
        packet_buffer[offset + 1] = (UCHAR)(named_curve & 0x00FF);
        offset += 2;

        /* Return the amount of data we wrote. */
        *extension_length = (USHORT)(offset - *packet_offset);

        /* Return our updated packet offset. */
        *packet_offset = offset;

        return(NX_SUCCESS);
    }

    /* Add Total Length (2 bytes) to packet after we fill in the key data. */
    length_offset = offset;
    data_length = 0;
    offset += 2;

    /* Key length will differ for each curve. For p256, 32 octets per coordinate, plus the "legacy_form" byte. */
    key_length = ecdhe_data->nx_secure_tls_ecdhe_public_key_length;

    /* Entry is named group(2) + key len(2) + key data(key len). */
    entry_length = 2 + 2 + key_length;

    if (available_size < (offset + 4u + key_length))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Set the named group. */
    packet_buffer[offset] = (UCHAR)((named_curve & 0xFF00) >> 8);;
    packet_buffer[offset + 1] = (UCHAR)(named_curve & 0x00FF);
    offset += 2;

    /* Set the key length. */
    packet_buffer[offset] = (UCHAR)((key_length & 0xFF00) >> 8);;
    packet_buffer[offset + 1] = (UCHAR)(key_length & 0x00FF);
    offset += 2;

    /* Set the key data from our already-generated ECC keys. */
    NX_SECURE_MEMCPY(&packet_buffer[offset], &ecdhe_data->nx_secure_tls_ecdhe_public_key[0], key_length); /* Use case of memcpy is verified. */
    offset += (key_length);

    /* Get the length of the entire extension. */
    data_length = data_length + (UINT)(entry_length);

    /* Go back and set the total extension length. */
    packet_buffer[length_offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[length_offset + 1] = (UCHAR)(data_length & 0x00FF);

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_offset);

    /* Return our updated packet offset. */
    *packet_offset = offset;


    return(NX_SUCCESS);
#else
    /* Server is disabled, we shouldn't be processing a ClientHello - error! */
    return(NX_SECURE_TLS_INVALID_STATE);
#endif            
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_serverhello_psk_extension       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the pre_shared_key extension, added in TLS       */
/*    v1.3.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_length                         Length of data in packet      */
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
/*    _nx_secure_tls_send_serverhello_extensions                          */
/*                                          Send TLS ServerHello extension*/
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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && defined (NX_SECURE_ENABLE_PSK_CIPHERSUITES) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
static UINT _nx_secure_tls_send_serverhello_psk_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, ULONG *packet_length,
                                                   USHORT *extension_length, ULONG available_size)
{
ULONG  offset;
USHORT ext;

    /* Key Share Extension structure (From TLS 1.3 RFC 8446):
          struct {
              opaque identity<1..2^16-1>;
              uint32 obfuscated_ticket_age;
          } PskIdentity;

          opaque PskBinderEntry<32..255>;

          struct {
              PskIdentity identities<7..2^16-1>;
              PskBinderEntry binders<33..2^16-1>;
          } OfferedPsks;

          struct {
              select (Handshake.msg_type) {
                  case client_hello: OfferedPsks;
                  case server_hello: uint16 selected_identity;
              };
          } PreSharedKeyExtension;

       identity:  A label for a key.  For instance, a ticket (as defined in
          Appendix B.3.4) or a label for a pre-shared key established
          externally.

       obfuscated_ticket_age:  An obfuscated version of the age of the key.
          Section 4.2.11.1 describes how to form this value for identities
          established via the NewSessionTicket message.  For identities
          established externally, an obfuscated_ticket_age of 0 SHOULD be
          used, and servers MUST ignore the value.

       identities:  A list of the identities that the client is willing to
          negotiate with the server.  If sent alongside the "early_data"
          extension (see Section 4.2.10), the first identity is the one used
          for 0-RTT data.

       binders:  A series of HMAC values, one for each value in the
          identities list and in the same order, computed as described
          below.

       selected_identity:  The server's chosen identity expressed as a
          (0-based) index into the identities in the client's list.


      ServerHello extension layout:
      |     2      |         2          |       2      |
      |  Ext Type  |  Extension length  |  Selected ID |
    */
    NX_PARAMETER_NOT_USED(tls_session);

    /* Start with our passed-in packet offset. */
    offset = *packet_length;

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_PRE_SHARED_KEY;

    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if (available_size < (offset + 6u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* The total extension length - only the uint16 of the selected identity field. */
    packet_buffer[offset] = (UCHAR)(0);
    packet_buffer[offset + 1] = (UCHAR)(2);
    offset += 2;

    /* The selected_identity index into the ClientHello PSK list. */
    packet_buffer[offset] = (UCHAR)(0);
    packet_buffer[offset + 1] = (UCHAR)(0);
    offset += 2;


    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_length);

    /* Return our updated packet offset. */
    *packet_length = offset;

    return(NX_SUCCESS);
}
#endif


