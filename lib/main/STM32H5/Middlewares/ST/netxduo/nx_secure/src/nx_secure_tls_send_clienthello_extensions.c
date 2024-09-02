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

#ifndef NX_SECURE_TLS_CLIENT_DISABLED

static VOID _nx_secure_tls_get_signature_algorithm(NX_SECURE_TLS_SESSION *tls_session,
                                                   NX_SECURE_X509_CRYPTO *crypto_method,
                                                   USHORT *signature_algorithm);

static UINT _nx_secure_tls_send_clienthello_sig_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);
#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
static UINT _nx_secure_tls_send_clienthello_sni_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);
#endif
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_tls_send_clienthello_sec_spf_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                               UCHAR *packet_buffer, ULONG *packet_offset,
                                                               USHORT *extension_length,
                                                               ULONG available_size);
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
static UINT _nx_secure_tls_send_clienthello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size);

static UINT _nx_secure_tls_send_clienthello_key_share_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);

static UINT _nx_secure_tls_send_clienthello_psk_kem_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer, ULONG *packet_length,
                                                              USHORT *extension_length);

/* Note: PSK extension generation is called directly, so not static. */
UINT _nx_secure_tls_send_clienthello_psk_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_length,
                                                          ULONG extension_length_offset, ULONG total_extensions_length,
                                                          ULONG *extension_length,
                                                          ULONG available_size);
#endif

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size);
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_extensions          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with ClientHello extensions,   */
/*    which provide additional information to the remote host for the     */
/*    initial handshake negotiations.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet           */
/*    packet_offset                         Offset into packet buffer     */
/*    extensions_length                     Return total extension length */
/*    available_size                        Available size of buffer      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sec_reneg_extension                 */
/*                                          Send ClientHello Renegotiation*/
/*                                            extension                   */
/*    _nx_secure_tls_send_clienthello_sig_extension                       */
/*                                          Send ClientHello Signature    */
/*                                            extension                   */
/*    _nx_secure_tls_send_clienthello_sni_extension                       */
/*                                          Send ClientHello SNI extension*/
/*    _nx_secure_tls_send_clienthello_ec_extension                        */
/*                                          Send ClientHello EC extension */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello       Send TLS ClientHello          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yajun Xia                Modified comment(s),          */
/*                                            added exception case,       */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                ULONG *extensions_length, ULONG available_size)
{
ULONG  length = *packet_offset;
USHORT extension_length = 0, total_extensions_length;
UINT   status;

    total_extensions_length = 0;


#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    status = _nx_secure_tls_send_clienthello_sec_spf_extensions(tls_session,
                                                                packet_buffer,
                                                                &length,
                                                                &extension_length,
                                                                available_size);
    if (status != NX_SUCCESS)
    {
        return(status);
    }
    total_extensions_length = (USHORT)(total_extensions_length + extension_length);
#endif

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
    /* We have to add renegotiation extensions in both initial sessions and renegotiating sessions. */
    if (tls_session -> nx_secure_tls_renegotation_enabled == NX_TRUE)
    {
        status = _nx_secure_tls_send_clienthello_sec_reneg_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
        if(status != NX_SUCCESS)
        {
            return(status);
        }
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
    }
#endif

/* We can't do TLS 1.3 without ECC. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        /* Send supported TLS versions extensions (for TLS 1.3). */
        status = _nx_secure_tls_send_clienthello_supported_versions_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
        if(status != NX_SUCCESS)
        {
            return(status);
        }
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);

        /* Send KeyShare extension (for TLS 1.3). */
        _nx_secure_tls_send_clienthello_key_share_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
        if(status != NX_SUCCESS)
        {
            return(status);
        }
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);

        /* Including a "cookie" extension if one was provided in the HelloRetryRequest. */
        if ((tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY) && 
            (tls_session -> nx_secure_tls_cookie_length != 0))
        {

            /* Add Extension Type. */
            packet_buffer[length] = (UCHAR)((NX_SECURE_TLS_EXTENSION_COOKIE) >> 8);
            packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_EXTENSION_COOKIE);
            length += 2;

            /* Add Extension Length. */
            extension_length = (USHORT)(tls_session -> nx_secure_tls_cookie_length + 2);
            packet_buffer[length] = (UCHAR)(extension_length >> 8);
            packet_buffer[length + 1] = (UCHAR)(extension_length);
            length += 2;

            /* Add Cookie Length. */
            packet_buffer[length] = (UCHAR)(tls_session -> nx_secure_tls_cookie_length >> 8);
            packet_buffer[length + 1] = (UCHAR)(tls_session -> nx_secure_tls_cookie_length);
            length += 2;

            /* Add Cookie. */
            NX_SECURE_MEMCPY(&packet_buffer[length], tls_session -> nx_secure_tls_cookie, tls_session -> nx_secure_tls_cookie_length); /* Use case of memcpy is verified. */
            length += (tls_session -> nx_secure_tls_cookie_length);

            /* Update total extensions length and reset the stored cookie length. */
            total_extensions_length = (USHORT)(total_extensions_length + extension_length + 4);
            tls_session -> nx_secure_tls_cookie_length = 0;
        }
    }
#endif

    /* RFC 5246 7.4.1.4.1 Signature Algorithm:
       Note: this extension is not meaningful for TLS versions prior to 1.2.
       Clients MUST NOT offer it if they are offering prior versions. */
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
    {

        /* Send the available signature algorithms extension. */
        status = _nx_secure_tls_send_clienthello_sig_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
        if (status != NX_SUCCESS)
        {
            return(status);
        }
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
    }

#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
    /* Send the server name indication extension. */
    status = _nx_secure_tls_send_clienthello_sni_extension(tls_session, packet_buffer, &length, &extension_length, available_size);
    if(status != NX_SUCCESS)
    {
        return(status);
    }
    total_extensions_length = (USHORT)(total_extensions_length + extension_length);
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3 && tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_count > 0)
    {
        status = _nx_secure_tls_send_clienthello_psk_kem_extension(tls_session, packet_buffer, &length, &extension_length);
        if (status != NX_SUCCESS)
        {
            return(status);
        }
        total_extensions_length = (USHORT)(total_extensions_length + extension_length);
    }
#endif

    *extensions_length = total_extensions_length;
    *packet_offset = length;


    return(NX_SUCCESS);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sig_extension       PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Signature Algorithms extension to an         */
/*    outgoing ClientHello record. See RFC 5246 section 7.4.1.4.1.        */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed unused code,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_send_clienthello_sig_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length,
                                                          ULONG available_size)
{
ULONG  offset, ext_pos;
USHORT ext_len, sig_len, sighash_len, ext;
UINT i;
USHORT signature_algorithm;
NX_SECURE_X509_CRYPTO *cipher_table;

    /* Signature Extensions structure:
     * |     2     |     2   |       2      | <SigHash Len> |
     * |  Ext Type | Sig Len |  SigHash Len | SigHash Algos |
     *
     * Each algorithm pair has a hash ID and a public key operation ID represented
     * by a single octet. Therefore each entry in the list is 2 bytes long.
     */

    if (available_size < (*packet_offset + 6u +
                          (ULONG)(tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size << 1)))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    ext_pos = offset;
    offset += 6;

    ext_len = sighash_len = 0;

    cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;

    /* Loop the x509 cipher table to add signature algorithms. */
    for (i = 0; i < tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size; i++)
    {

        /* Map crypto method to signature algorithm. */
        _nx_secure_tls_get_signature_algorithm(tls_session, &cipher_table[i], &signature_algorithm);
        if (signature_algorithm == 0)
        {
            continue;
        }

        packet_buffer[offset] = (UCHAR)((signature_algorithm & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(signature_algorithm & 0x00FF);

        offset += 2;
        sighash_len = (USHORT)(sighash_len + 2);
    }

    ext = NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS;  /* Signature algorithms */
    ext_len = (USHORT)(ext_len + 2);

    sig_len = 2;
    ext_len = (USHORT)(ext_len + 2);

    sig_len = (USHORT)(sig_len + sighash_len);
    ext_len = (USHORT)(ext_len + sig_len);

    packet_buffer[ext_pos] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[ext_pos + 1] = (UCHAR)(ext & 0x00FF);
    ext_pos += 2;

    packet_buffer[ext_pos] = (UCHAR)((sig_len & 0xFF00) >> 8);
    packet_buffer[ext_pos + 1] = (UCHAR)(sig_len & 0x00FF);
    ext_pos += 2;

    packet_buffer[ext_pos] = (UCHAR)((sighash_len & 0xFF00) >> 8);
    packet_buffer[ext_pos + 1] = (UCHAR)(sighash_len & 0x00FF);

    /* Return our updated packet offset. */
    *extension_length = ext_len;
    *packet_offset = offset;

    return(NX_SUCCESS);
}

VOID _nx_secure_tls_get_signature_algorithm(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CRYPTO *crypto_method,
                                            USHORT *signature_algorithm)
{
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT j;
#endif /* (NX_SECURE_TLS_TLS_1_3_ENABLED) */
USHORT named_curve = 0;
UCHAR hash_algo = 0;
UCHAR sig_algo = 0;

    *signature_algorithm = 0;

    switch (crypto_method -> nx_secure_x509_public_cipher_method -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_KEY_EXCHANGE_RSA:
        sig_algo = NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA;
        break;
    case NX_CRYPTO_DIGITAL_SIGNATURE_DSA:
        sig_algo = NX_SECURE_TLS_SIGNATURE_ALGORITHM_DSA;
        break;
    case NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA:
        sig_algo = NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA;
        break;
    default:
        return;
    }

    switch (crypto_method -> nx_secure_x509_hash_method -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_HASH_MD5:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_MD5;
        break;
    case NX_CRYPTO_HASH_SHA1:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_SHA1;
        break;
    case NX_CRYPTO_HASH_SHA224:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_SHA224;
        break;
    case NX_CRYPTO_HASH_SHA256:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_SHA256;
        named_curve = (USHORT)NX_CRYPTO_EC_SECP256R1;
        break;
    case NX_CRYPTO_HASH_SHA384:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_SHA384;
        named_curve = (USHORT)NX_CRYPTO_EC_SECP384R1;
        break;
    case NX_CRYPTO_HASH_SHA512:
        hash_algo = NX_SECURE_TLS_HASH_ALGORITHM_SHA512;
        named_curve = (USHORT)NX_CRYPTO_EC_SECP521R1;
        break;
    default:
        return;
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* TLS 1.3 deprecates MD5, and SHA-1 is obsolete. Remove them from the signing list. */
    if(tls_session -> nx_secure_tls_1_3)
    {
        if((hash_algo == NX_SECURE_TLS_HASH_ALGORITHM_MD5) || (hash_algo == NX_SECURE_TLS_HASH_ALGORITHM_SHA1))
        {
            *signature_algorithm = 0;
            return;
        }
    }

    /* In TLS 1.3, the signing curve is constrained.  */
    if (tls_session -> nx_secure_tls_1_3 &&
        (named_curve != 0) &&
        (sig_algo == NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA))
    {
        for (j = 0; j < tls_session->nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count; j++)
        {
            if (named_curve == tls_session->nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups[j])
            {
                *signature_algorithm = (USHORT)((hash_algo << 8) + sig_algo);
                return;
            }
        }
    }
    else
#else
    NX_PARAMETER_NOT_USED(named_curve);
    NX_PARAMETER_NOT_USED(tls_session);
#endif
    {
        *signature_algorithm = (USHORT)((hash_algo << 8) + sig_algo);
    }
}

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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
/* We need to access the supported versions table located in nx_secure_tls_check_protocol_version.c. */
extern const NX_SECURE_VERSIONS_LIST nx_secure_supported_versions_list[];
static UINT _nx_secure_tls_send_clienthello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length, ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;
UINT   id = NX_SECURE_TLS;
USHORT protocol_version;
INT    i;

    /* Supported Versions Extension structure:
     * |     2      |         2          |      1       | <list length> |
     * |  Ext Type  |  Extension length  |  List Length |  TLS Versions |
     */
    NX_PARAMETER_NOT_USED(tls_session);

    if (available_size < (*packet_offset + 7u +
                          (nx_secure_supported_versions_list[id].nx_secure_versions_list_count << 1)))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SUPPORTED_VERSIONS;

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* Fill in the supported versions first. */
    offset += 3;
    data_length = 0;

    /* Add TLS 1.3 (0x0304) which is not in legacy supported versions list. */
    packet_buffer[offset] = (UCHAR)NX_SECURE_TLS_VERSION_MAJOR_3;
    packet_buffer[offset + 1] = (UCHAR)NX_SECURE_TLS_VERSION_MINOR_1_3;
    data_length += 2;
    offset += 2;

#ifndef NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE
    /* Loop through the supported versions to see if the passed-in version is one that is enabled. */
    for (i = (INT)(nx_secure_supported_versions_list[id].nx_secure_versions_list_count - 1); i >= 0; --i)
    {

        /* See if it is supported. */
        if (nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_is_supported)
        {

            /* Set the version value. */
            protocol_version = nx_secure_supported_versions_list[id].nx_secure_versions_list[i].nx_secure_tls_protocol_version;
            packet_buffer[offset] = (UCHAR)((protocol_version & 0xFF00) >> 8);
            packet_buffer[offset + 1] = (UCHAR)(protocol_version & 0x00FF);
            data_length += 2;
            offset += 2;
        }
        else
        {
            break;
        }
    }
#endif /* NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE */

    /* Fill in list length and extension length. */
    packet_buffer[*packet_offset + 4] = (UCHAR)(data_length & 0xFF);

    data_length++;
    packet_buffer[*packet_offset + 2] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[*packet_offset + 3] = (UCHAR)(data_length & 0x00FF);

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
/*    _nx_secure_tls_send_clienthello_key_share_extension PORTABLE C      */
/*                                                           6.1          */
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
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
static UINT _nx_secure_tls_send_clienthello_key_share_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer, ULONG *packet_offset,
                                                                USHORT *extension_length, ULONG available_size)
{
ULONG  offset;
ULONG  length_offset;
USHORT ext;
UINT   entry_index;
UINT   entry_length;
UINT   key_length;
UINT   data_length;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
USHORT named_curve;

    /* Key Share Extension structure (From TLS 1.3 RFC Draft #28):
     *  struct {
     *     NamedGroup group;
     *     opaque key_exchange<1..2^16-1>;
     *  } KeyShareEntry;
     *
     *  struct {
     *     KeyShareEntry client_shares<0..2^16-1>;
     *  } KeyShareClientHello;
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
     *                                   |                 <list length>                      |
     * |     2      |         2          |  <|       2      |    2    |     <Key len>     |>  |
     * |  Ext Type  |  Extension length  |  <|  Named Group | Key len | key_exchange data |>  |
     */
   
    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }
   
    if (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
    {
        entry_index = tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data_selected;
    }
    else
    {

        /* Select default entry is based on the actual curves we support. */
        entry_index = 0;
    }

    /* Get the ECDHE structure for our key data. */
    ecdhe_data = &tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[entry_index];

    /* Get the curve for the key we are sending. */
    named_curve = (USHORT)ecdhe_data->nx_secure_tls_ecdhe_named_curve;

    /* Key length will differ for each curve. For p256, 32 octets per coordinate, plus the "legacy_form" byte. */
    key_length = ecdhe_data->nx_secure_tls_ecdhe_public_key_length;

    if (available_size < (*packet_offset + 10u + key_length))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_KEY_SHARE;
    
    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;
    
    /* Add Total Length (2 bytes) and List length (2 bytes) to packet after we 
       fill in the key data. */
    length_offset = offset;
    data_length = 0;
    offset += 4;

    /* Entry is named group(2) + key len(2) + key data(key len). */
    entry_length = 2 + 2 + key_length;

    /* Set the named group. */
    packet_buffer[offset] = (UCHAR)((named_curve & 0xFF00) >> 8);;
    packet_buffer[offset + 1] = (UCHAR)(named_curve & 0x00FF);
    offset += 2;

    /* Set the key length. */
    packet_buffer[offset] = (UCHAR)((key_length & 0xFF00) >> 8);;
    packet_buffer[offset + 1] = (UCHAR)(key_length & 0x00FF);
    offset += 2;

    /* Set the key data from our already-generated ECC keys. */
    NX_SECURE_MEMCPY(&packet_buffer[offset], &tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[entry_index].nx_secure_tls_ecdhe_public_key[0], key_length); /* Use case of memcpy is verified. */
    offset += (key_length);

    /* Get the length of the entire extension. */
    data_length = data_length + (UINT)(entry_length);

    /* Go back and set the total extension length. */
    data_length = data_length + 2;
    packet_buffer[length_offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[length_offset + 1] = (UCHAR)(data_length & 0x00FF);
    length_offset += 2;
    data_length = data_length - 2;
    
    /* Finally, put the list length into the packet. */
    packet_buffer[length_offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[length_offset + 1] = (UCHAR)(data_length & 0x00FF);

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
/*    _nx_secure_tls_send_clienthello_psk_extension       PORTABLE C      */
/*                                                           6.1.8        */
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
/*  08-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            hash clone and cleanup,     */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_send_clienthello_psk_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, ULONG *packet_length,
                                                   ULONG extension_length_offset, ULONG total_extensions_length,
                                                   ULONG *extension_length, ULONG available_size)
{
ULONG  offset;
ULONG  length_offset;
USHORT ext;
UINT   i;
UINT   num_ids;
UINT   data_length;
UINT   ids_total;
UINT   id_len;
UINT   age;
UCHAR  *id;
UINT   binder_len = 0;
UCHAR  *binder;
UINT   num_binders;
UINT   binder_offset;
UINT   binder_total;
UINT   status;
UINT   partial_client_hello_len;
NX_SECURE_TLS_PSK_STORE *psk_store;


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


      ClientHello extension layout:
      |     2      |         2          |       2      |      <ID list len>        |      2      | <binders len>   |
      |  Ext Type  |  Extension length  |  ID list len | <|<ID len> | ID | age |>  | binders len | <len> | binder  |
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

    /* Add Total Length (2 bytes) and List length (2 bytes) to packet after we
       fill in the key data. */
    length_offset = offset;
    data_length = 4;
    offset += 4;

    /* Get our PSK store for easy access. */
    psk_store = tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_store;
    num_ids = tls_session->nx_secure_tls_credentials.nx_secure_tls_psk_count;
    ids_total = 0;
    binder_total = 0;

    /* Loop through all IDs. */
    for(i = 0; i < num_ids; ++i)
    {
        /* Setup the ID list. */
        id_len = psk_store[i].nx_secure_tls_psk_id_size;
        id = psk_store[i].nx_secure_tls_psk_id;

        if (available_size < (offset + 6u + id_len))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        packet_buffer[offset] = (UCHAR)((id_len & 0xFF00) >> 8);;
        packet_buffer[offset + 1] = (UCHAR)(id_len & 0x00FF);
        offset += 2;

        /* Put the ID data into our packet. */
        NX_SECURE_MEMCPY(&packet_buffer[offset], id, id_len); /* Use case of memcpy is verified. */
        offset += (UINT)(id_len);

        /* Set the obfuscated PSK age. */
        age = 0;
        packet_buffer[offset]     = (UCHAR)((age & 0xFF000000) >> 24);
        packet_buffer[offset + 1] = (UCHAR)((age & 0x00FF0000) >> 16);
        packet_buffer[offset + 2] = (UCHAR)((age & 0x0000FF00) >> 8);
        packet_buffer[offset + 3] = (UCHAR) (age & 0x000000FF);
        offset += 4;

        /* Update the length with the ID length (id_len), length field (2), and age field (4). */
        ids_total = ids_total + (UINT)(id_len + 2 + 4);
        
        /* Caclulate the length of the binder list - binder for each PSK + the length field. */
        binder_total += (UINT)(1 + binder_len);        
    }

    /* Put the list length into the packet - the list length is the 16-bit field following the total length field (16-bits). */
    packet_buffer[length_offset + 2] = (UCHAR)((ids_total & 0xFF00) >> 8);
    packet_buffer[length_offset + 3] = (UCHAR)(ids_total & 0x00FF);
    data_length += ids_total;

       
    /* Get the length of the ClientHello to this point, plus the length field that comes next. */
    partial_client_hello_len = (UINT)(*packet_length) + data_length + 2;
    
    /* Update the total length of the extension with the anticipated size of the binders - this is used in generating
       the binder hashes. */
//    printf(">>>Binder list length: %d\n", binder_total);
    binder_total = 33;
    data_length += binder_total; 

    /* Extension length. */
    packet_buffer[length_offset] =     (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[length_offset + 1] = (UCHAR)(data_length & 0x00FF);
    
    /* IMPORTANT: We also need to update the total extensions length field using the passed-in parameters.
       Failure to to so will invalidate the transcript hash we do next. */
    total_extensions_length += data_length + 4;
    packet_buffer[extension_length_offset] =      (UCHAR)((total_extensions_length & 0xFF00) >> 8);
    packet_buffer[extension_length_offset + 1] =  (UCHAR)(total_extensions_length & 0x00FF);


    /* Now calculate the binders for this session. The binder is an HMAC hash of the entire ClientHello
     * up to and including the pre_shared_key extension with the IDs added above but NOT the binders.
     * The Length fields for the extension, however, are set to a value that matches the complete extension
     * with assumed correct binder lengths.
     *
     * Binders are calculated using the following (ClientHello1 is the first ClientHello,
     * and "Truncate" removes the binder data which is being generated):
     *    Transcript-Hash(Truncate(ClientHello1))
     *
     * Each PSK is associated with a single Hash algorithm.  For PSKs
     * established via the ticket mechanism (Section 4.6.1), this is the KDF
     * Hash algorithm on the connection where the ticket was established.
     * For externally established PSKs, the Hash algorithm MUST be set when
     * the PSK is established or default to SHA-256 if no such algorithm is
     * defined.  The server MUST ensure that it selects a compatible PSK
     * (if any) and cipher suite.
     */

    /* If nx_secure_tls_client_state is IDLE, it means this is ClientHello1, need to initialize the handshake hash.
       If not, it means this is ClientHello2, need to save the metadata(ClientHello1 and HelloRetryRequest) to sratch buffer. */
    if (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_IDLE)
    {

        /* Initialize the handshake hash to hash the ClientHello up to now. */
        _nx_secure_tls_handshake_hash_init(tls_session);
    }
    else
    {

        /* Save the handshake hash state. */
        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */
    }

    /* Hash the ClientHello up to its current point. */
    /* The Transcript hash needs to include the handshake record header which means we need to calculate the full length
       of the expected ClientHello (with expected binder list length) and fill in the header appropriately. */
    UCHAR header[] = { 0x01, 0x00, 0x01, 0x30 };
    header[2] = (UCHAR)((partial_client_hello_len + binder_total + 2) >> 8);
    header[3] = (UCHAR)((partial_client_hello_len + binder_total + 2) & 0xFF);
    _nx_secure_tls_handshake_hash_update(tls_session, header, sizeof(header));
    _nx_secure_tls_handshake_hash_update(tls_session, packet_buffer, partial_client_hello_len);

    /* Save the transcript hash for the ClientHello, which is used in generating the PSK binders. */
    status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENTHELLO, NX_FALSE);
    if (status != NX_SUCCESS)
    {

        if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_IDLE)
        {
            NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                         tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);
        }

        return(status);
    }

    /* Restore the original metadata(ClientHello1 and HelloRetryRequest) from scratch buffer. */
    if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_IDLE)
    {
        NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                     tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);

        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */

        NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                     tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);
    }

    /* Loop through all IDs and set the binders accordingly. */
    for(i = 0; i < num_ids; ++i)
    {
        _nx_secure_tls_psk_binder_generate(tls_session, &psk_store[i]);
    }

    if (available_size < (offset + 2u))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Now add in the PSK binders. Skip the list length and fill in after. */
    binder_offset = offset;
    offset += 2;
    binder_total = 0;

    num_binders = num_ids;
    for(i = 0; i < num_binders; ++i)
    {
        binder_len = psk_store[i].nx_secure_tls_psk_binder_size;
        binder = psk_store[i].nx_secure_tls_psk_binder;

        if (available_size < (offset + 1u + binder_len))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Add in the length of the binder. */
        packet_buffer[offset] = (UCHAR)(binder_len);
        offset += (UINT)(1);

        /* Put the binder data into the packet. */
        NX_SECURE_MEMCPY(&packet_buffer[offset], binder, binder_len); /* Use case of memcpy is verified. */
        offset += (UINT)(binder_len);

        /* Update our total with the binder length (binder_len) and length field(1). */
        binder_total += (UINT)(1 + binder_len);
    }

    /* Binder list length - this was set above, but overwrite it here. */
    packet_buffer[binder_offset] = (UCHAR)((binder_total & 0xFF00) >> 8);;
    packet_buffer[binder_offset + 1] = (UCHAR)(binder_total & 0x00FF);

    /* Go back and set the total extension length. */
    packet_buffer[length_offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[length_offset + 1] = (UCHAR)(data_length & 0x00FF);

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_length);

    /* Return our updated packet offset. */
    *packet_length = offset;

    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_psk_kem_extension   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the psk_key_exchange_mode extension, added in TLS*/
/*    v1.3.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    packet_length                         Length of data in packet      */
/*    extension_length                      Return length of data         */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
static UINT _nx_secure_tls_send_clienthello_psk_kem_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer, ULONG *packet_length,
                                                              USHORT *extension_length)
{
ULONG offset;
USHORT length;

    /* PSK Key Exchange Modes Extension structure (From TLS 1.3 RFC 8446):
          enum { psk_ke(0), psk_dhe_ke(1), (255) } PskKeyExchangeMode;

          struct {
              PskKeyExchangeMode ke_modes<1..255>;
          } PskKeyExchangeModes;

       psk_ke: PSK-only key establishment. In this mode, the server
          MUST NOT supply a "key_share" value.

       psk_dhe_ke: PSK with (EC)DHE key establishment. In this mode, the
          client and server MUST supply "key_share" values as described in
          Section 4.2.8.

      ClientHello extension layout:
      |     2      |         2          |                 1               | <PSK Key Exchange Modes list len> |
      |  Ext Type  |  Extension length  | PSK Key Exchange Modes list len |    PSK Key Exchange Modes list    |
    */

    NX_PARAMETER_NOT_USED(tls_session);

    offset = *packet_length;

    /* Add extension type.  */
    packet_buffer[offset] = (UCHAR)((NX_SECURE_TLS_EXTENSION_PSK_KEY_EXCHANGE_MODES) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(NX_SECURE_TLS_EXTENSION_PSK_KEY_EXCHANGE_MODES);
    offset += 2;

    /* Add extension length.  */
    length = 3;
    packet_buffer[offset] = (UCHAR)(length >> 8);
    packet_buffer[offset + 1] = (UCHAR)(length);
    offset += 2;

    /* Add PSK Key Exchange Modes Length.  */
    packet_buffer[offset++] = 2;

    /* Add PSK Key Exchange Mode.  */
    /* psk_ke : 0, psk_dhe_ke : 1.  */
    packet_buffer[offset++] = 1;
    packet_buffer[offset++] = 0;

    /* Return the amount of data we wrote. */
    *extension_length = (USHORT)(offset - *packet_length);

    /* Return our updated packet offset. */
    *packet_length = offset;

    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_send_clienthello_sni_extension       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Server Name Indication extension to an       */
/*    outgoing ClientHello record if a server name has been set by the    */
/*    application.                                                        */
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
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_SNI_EXTENSION_DISABLED
static UINT _nx_secure_tls_send_clienthello_sni_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                          UCHAR *packet_buffer, ULONG *packet_offset,
                                                          USHORT *extension_length, ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Server Name Indication Extension structure:
     * |     2      |      2        |      1     |     2       |   <name length>   |
     * |  Ext Type  |  list length  |  name type | name length |  Host name string |
     */
    /*  From RFC 6066:

          struct {
              NameType name_type;
              select (name_type) {
                  case host_name: HostName;
              } name;
          } ServerName;

          enum {
              host_name(0), (255)
          } NameType;

          opaque HostName<1..2^16-1>;

          struct {
              ServerName server_name_list<1..2^16-1>
          } ServerNameList;

          The contents of this extension are specified as follows.

      -   The ServerNameList MUST NOT contain more than one name of the same
          name_type.

      -   Currently, the only server names supported are DNS hostnames.
     */

    /* If there is no SNI server name, just return. */
    if (tls_session -> nx_secure_tls_sni_extension_server_name == NX_NULL)
    {
        *extension_length = 0;
        return(NX_SUCCESS);
    }

    /* Start with our passed-in packet offset. */
    offset = *packet_offset;

    if (available_size < (offset + 9u + tls_session -> nx_secure_tls_sni_extension_server_name -> nx_secure_x509_dns_name_length))
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* The extension identifier. */
    ext = NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION;

    /* Put the extension ID into the packet. */
    packet_buffer[offset] = (UCHAR)((ext & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(ext & 0x00FF);
    offset += 2;

    /* Get the length of the entire extension. In this case it is a single name entry. Add 2 for the list length, plus 1 for
       the name_type field and 2 for the name length field (name type and length would be duplicated for all entries). */
    data_length = (UINT)(tls_session -> nx_secure_tls_sni_extension_server_name -> nx_secure_x509_dns_name_length + 5);

    /* Set the total extension length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;
    data_length -= 2; /* Remove list length. */

    /* Set the name list length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;
    data_length -= 3; /* Remove name type and name length. */

    /* Set the name type. */
    packet_buffer[offset] = NX_SECURE_TLS_SNI_NAME_TYPE_DNS;
    offset++;

    /* Set the name length. */
    packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
    packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
    offset += 2;

    /* Write the name into the packet. */
    NX_SECURE_MEMCPY(&packet_buffer[offset], tls_session -> nx_secure_tls_sni_extension_server_name -> nx_secure_x509_dns_name, data_length); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */
    offset += data_length;

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
/*    _nx_secure_tls_send_clienthello_sec_reneg_extension PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds the Secure Renegotiation Indication extension    */
/*    to an outgoing ClientHello record if the ClientHello is part of a   */
/*    renegotiation handshake (the extension should be empty for the      */
/*    initial handshake. See RFC 5746 for more information.               */
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
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_send_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                ULONG *packet_offset,
                                                                USHORT *extension_length,
                                                                ULONG available_size)
{
ULONG  offset;
USHORT ext;
UINT   data_length;

    /* Secure Renegotiation Indication Extensions structure:
     * |     2      |           12        |
     * |  Ext Type  |  client_verify_data |
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

#ifdef NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
    /* If we are using the SCSV ciphersuite for TLS 1.0 compatibility
       and the session is not yet active (first handshake, not a renegotiation),
       then don't send the empty extension below, just return with no
       offset adjustments. */
    if (!tls_session -> nx_secure_tls_local_session_active)
    {
        *extension_length = 0;
        return(NX_SUCCESS);
    }
#endif


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
        if (available_size < (offset + 3u + NX_SECURE_TLS_FINISHED_HASH_SIZE))
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        data_length = NX_SECURE_TLS_FINISHED_HASH_SIZE + 1;
        packet_buffer[offset] = (UCHAR)((data_length & 0xFF00) >> 8);
        packet_buffer[offset + 1] = (UCHAR)(data_length & 0x00FF);
        offset += 2;

        /* The extension actually has a second length field of 1 byte that needs to be populated. */
        /* Fill in the length of renegotiated connection field. */
        packet_buffer[offset] = (UCHAR)(NX_SECURE_TLS_FINISHED_HASH_SIZE & 0x00FF);
        offset++;

        /* Copy the verify data into the packet. */
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
/*    _nx_secure_tls_send_clienthello_sec_spf_extensions  PORTABLE C      */
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
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_tls_send_clienthello_sec_spf_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                               UCHAR *packet_buffer, ULONG *packet_offset,
                                                               USHORT *extension_length, ULONG available_size)
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
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

