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
/*    _nx_secure_tls_send_certificate                     PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with the TLS Certificate       */
/*    message, which contains the identity certificate for this device.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be filled           */
/*    wait_option                           Controls timeout actions      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get local certificate to send */
/*    nx_packet_data_append                 Append data to packet         */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*  04-25-2022     Zhen Kong                Modified comment(s), removed  */
/*                                            unreachable error code,     */
/*                                            resulting in version 6.1.11 */
/**************************************************************************/
UINT _nx_secure_tls_send_certificate(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                     ULONG wait_option)
{
UINT                 length;
UINT                 total_length;
UCHAR                length_buffer[3];
UINT                 status = NX_SECURE_TLS_SUCCESS;
NX_SECURE_X509_CERT *cert;
INT                  compare_result = 0;
UCHAR               *record_start;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT                 extensions_length;
#endif


    /* Structure:
     * |      3       |                     <Total Length>                      |
     * | Total length |       3        |   <Cert[0] Length> | 3 + <Cert[x] Len> |
     * |              | Cert[0] Length |   Certificate[0]   |    More certs...  |
     */

    /* TLS 1.3 Structure:
     * |    1    | <Ctx Len> |      3       |                     <Total Length>                         |
     * | Ctx Len | Context   | Total length |       3        |   <Cert[x] Length> |   2    |   <ExtLen>  |
     * |                                    | Cert[x] Length |   Certificate[x]   | ExtLen |  Extensions |
     */

    /* See if the local certificate has been overridden. If so, use that instead. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
    {
        cert = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
    }
    else
    {
        /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              NX_NULL, &cert);
    }

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    /* See if this is a TLS client sending a certificate in response to a certificate request from
       the remote server. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT &&
        (status == NX_SECURE_X509_CERTIFICATE_NOT_FOUND))
    {
        /* If this is a TLS client, as per the RFC we can have no certificate assigned in which
           case our response to the server that has requested our certificate will contain
           an empty certificate field. */
        cert = NX_NULL;

        /* Clear the requested flag so no further certificate-specific messages are sent. */
        tls_session -> nx_secure_tls_client_certificate_requested = NX_FALSE;
    }
    else
#endif
    {
        if (status)
        {

            /* _nx_secure_x509_local_device_certificate_get can only return
                NX_SECURE_X509_CERTIFICATE_NOT_FOUND in this case. */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }
    }

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 3u)
    {

        /* Packet buffer is too small to hold random and ID. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* In TLS 1.3, the context length is the first field, followed by the context itself (if necessary). */
    if(tls_session->nx_secure_tls_1_3)
    {
        send_packet -> nx_packet_append_ptr[0] = 0;
        send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + 1;
        send_packet -> nx_packet_length = send_packet -> nx_packet_length + (USHORT)(1);
    }
#endif

    /* Save a reference to the start of our record. */
    record_start = send_packet -> nx_packet_append_ptr;

    /* Pointer to where we are going to place the next certificate. */
    /* The first 3 bytes hold the total length field.  Therefore
       certificate data starts from offset 3.*/
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + 3;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + (USHORT)(3);

    /* Total length is the length of all certificates and is the first 3 bytes of the message. */
    total_length = 0;

    while (status == NX_SUCCESS)
    {
        /* Place certificate data into the packet buffer with the appropriate length.
           NOTE: We need to use the RAW data of the certificate when sending, not the parsed certificate! */
        length = cert -> nx_secure_x509_certificate_raw_data_length;

        /* Total length is increased by the length of the certificate plus the 3 bytes for
           the certificate length parameter. */
        total_length += (length + 3);

        /* Put the length of this certificate into the buffer. */
        length_buffer[0] = (UCHAR)((length & 0xFF0000) >> 16);
        length_buffer[1] = (UCHAR)((length & 0xFF00) >> 8);
        length_buffer[2] = (UCHAR)(length & 0xFF);

        /* Release the protection before suspending on nx_packet_data_append. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Put the length into the buffer. */
        status = nx_packet_data_append(send_packet, length_buffer, 3,
                                       tls_session -> nx_secure_tls_packet_pool, wait_option);

        /* Get the protection after nx_packet_data_append. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Release the protection before suspending on nx_packet_data_append. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Put the certificate data into the buffer. */
        status = nx_packet_data_append(send_packet, cert -> nx_secure_x509_certificate_raw_data, length,
                                       tls_session -> nx_secure_tls_packet_pool, wait_option);

        /* Get the protection after nx_packet_data_append. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        /* Check for TLS 1.3 extensions following each certificate. */
        if(tls_session->nx_secure_tls_1_3)
        {
            extensions_length = 0;

            /* Add extension length to packet. */
            send_packet -> nx_packet_append_ptr[0] = (UCHAR)((extensions_length & 0xFF00) >> 8);
            send_packet -> nx_packet_append_ptr[1] = (UCHAR)(extensions_length & 0x00FF);

            /* Adjust pointer and length in packet according to extensions sent. */
            send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + 2;
            send_packet -> nx_packet_length = send_packet -> nx_packet_length + (USHORT)(2);
            total_length += 2;
        }
#endif


        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Get certificate issuer - if it exists in the store, send it, otherwise we are done. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              &cert -> nx_secure_x509_issuer, &cert);

        /* If certificate was not found, don't try to dereference. */
        if (status == NX_SUCCESS)
        {
            /* Prevent infinite loop if certificate is self-signed. */
            compare_result = _nx_secure_x509_distinguished_name_compare(&cert -> nx_secure_x509_distinguished_name,
                                                                        &cert -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);
            if (compare_result == 0)
            {
                break;
            }
        }

        /* If an issuer certificate was not found, that is OK - this should actually be the common case
         * as any certificate that is not self-signed should not have its root CA certificate on the
         * same device.
         */
    }

    /* Put the total length of all certificates into the buffer. */
    record_start[0] = (UCHAR)((total_length & 0xFF0000) >> 16);
    record_start[1] = (UCHAR)((total_length & 0xFF00) >> 8);
    record_start[2] = (UCHAR)(total_length & 0xFF);

    return(NX_SUCCESS);
}
