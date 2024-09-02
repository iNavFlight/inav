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
#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_remote_certificate           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TLS server certificate message, */
/*    extracting the RSA or DH public key and verifying the validity of   */
/*    the certificate. It parses the X509 certificate and fills in the    */
/*    relevant information if the caller has allocated space for it using */
/*    nx_secure_remote_certificate_allocate.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    data_length                           Length of packet buffer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*    _nx_secure_x509_certificate_list_add  Add incoming cert to store    */
/*    _nx_secure_x509_certificate_parse     Extract public key data       */
/*    _nx_secure_x509_free_certificate_get  Get free cert for storage     */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            fixed certificate buffer    */
/*                                            allocation,                 */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*  04-25-2022     Timothy Stapko           Modified comment(s),          */
/*                                            removed unnecessary code,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_remote_certificate(NX_SECURE_TLS_SESSION *tls_session,
                                               UCHAR *packet_buffer, UINT message_length,
                                               UINT data_length)
{
UINT                 length;
UINT                 total_length;
UINT                 cert_length = 0;
UINT                 status;
NX_SECURE_X509_CERT *certificate;
UCHAR               *endpoint_raw_ptr;
UINT                 endpoint_length;
UINT                 bytes_processed;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT                 extensions_length;
#endif
UCHAR               *cert_buffer;
ULONG                cert_buf_size;


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



    /* At this point, the packet buffer is filled with a TLS record. We can use the remainder of
        the buffer to hold certificate structures for parsing. The remote certificates will
        remain in the packet buffer and only the X.509 parsing structure (NX_SECURE_X509_CERT) 
        will be allocated. See _nx_secure_tls_process_remote_certificate for more info. */
    /* Typical layout of packet buffer and certificate buffer. Cert buffer allocates top-down, X.509 parsing structures
        are allocated and used only until the certificate chain verification step. After that, the remote certs are cleared
        and then the endpoint certificate is copied into the cert buffer (only the endpoint) for later use. The packet buffer
        following this function (on success) should have the following layout (assuming no user-allocated certs):
        |                      Packet buffer                     |    Certificate buffer    | 
        |<-----------data------------------>|-->  free space  <--| Endpoint Cert 1 | X.509  | 
    */

    if (data_length > tls_session -> nx_secure_tls_packet_buffer_size)
    {
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Certificate buffer is at the end of the record in the record assembly buffer. */
    cert_buffer = &tls_session->nx_secure_tls_packet_buffer[data_length];

    /* The size of the buffer is the remaining space in the record assembly buffer. */
    cert_buf_size = tls_session -> nx_secure_tls_packet_buffer_size - data_length;

    /* Use our length as an index into the buffer. */
    length = 0;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        /* The certificate chain in TLS 1.3 contains a "context" at
           the beginning of the handshake record. If the first byte is non-zero
           it means the following bytes (length given as the value of that byte)
           should be the context. */
        packet_buffer++;
        message_length--;
      
    }
#endif
      
     
    /* Extract the certificate(s) from the incoming data, starting with. */
    total_length = (UINT)((packet_buffer[0] << 16) + (packet_buffer[1] << 8) + packet_buffer[2]);
    length = length + 3;

    /* Make sure what we extracted makes sense. */
    if (total_length > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* See if remote host sent an empty certificate message. */
    if (total_length == 0)
    {
        /*  No certificate received! */
        return(NX_SECURE_TLS_EMPTY_REMOTE_CERTIFICATE_RECEIVED);
    }

    /* Keep subtracting from the total length until no more certificates left. */
    while (total_length > 0)
    {
        /* Extract the next certificate's length. */
        cert_length = (UINT)((packet_buffer[length] << 16) + (packet_buffer[length + 1] << 8) + packet_buffer[length + 2]);
        length = length + 3;

        /* Make sure the individual cert length makes sense. */
        if ((cert_length + 3) > total_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Get a reference to the remote endpoint certificate that was allocated earlier. */
        status = _nx_secure_x509_free_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                      &certificate);

        /* If there are no free certificates, attempt to allocate from the packet reassembly buffer 
           (the certificate buffer is carved from the packet buffer in nx_secure_tls_process_record). */
        if (status != NX_SUCCESS)
        {
            /* No remote certificates added. Instead try extracting space from packet buffer. */
            if(cert_buf_size < sizeof(NX_SECURE_X509_CERT))
            {
                /* Not enough space to allocate the X.509 structure. */
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);                
            }

            /* Get space for the parsing structure. */
            cert_buf_size -= sizeof(NX_SECURE_X509_CERT);
            certificate = (NX_SECURE_X509_CERT*)(&cert_buffer[cert_buf_size]);
            NX_SECURE_MEMSET(certificate, 0, sizeof(NX_SECURE_X509_CERT));

            /* Point structure to certificate being parsed. Note that the certificate 
               structure points directly into the packet buffer where the certificate
               is located - this certificate structure must NOT be used outside this function. */
            certificate -> nx_secure_x509_certificate_raw_data_length = cert_length;
            certificate -> nx_secure_x509_certificate_raw_data = &packet_buffer[length];
            certificate -> nx_secure_x509_certificate_raw_buffer_size = cert_length;
            certificate -> nx_secure_x509_user_allocated_cert = NX_FALSE;
        }
        else
        {
            /* Make sure we have enough space to save our certificate. */
            if (certificate -> nx_secure_x509_certificate_raw_buffer_size < cert_length)
            {
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);
            }

            /* Copy the certificate from the packet buffer into our allocated certificate space. */
            certificate -> nx_secure_x509_certificate_raw_data_length = cert_length;
            NX_SECURE_MEMCPY(certificate -> nx_secure_x509_certificate_raw_data, &packet_buffer[length], cert_length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
        }
        length += cert_length;
        
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Parse the DER-encoded X509 certificate to extract the public key data. */
        status = _nx_secure_x509_certificate_parse(certificate -> nx_secure_x509_certificate_raw_data, cert_length, &bytes_processed, certificate);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Make sure we parsed a valid certificate. */
        if (status != NX_SUCCESS)
        {

            /* Translate some X.509 return values into TLS return values. */
            if (status == NX_SECURE_X509_UNSUPPORTED_PUBLIC_CIPHER)
            {
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }

            return(status);
        }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        /* Check for TLS 1.3 extensions following each certificate. */
        if(tls_session->nx_secure_tls_1_3)
        {
            extensions_length = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);

            /* Add extensions length bytes. */
            length += 2;
            
            /* Add extensions length to offset. */
            length += extensions_length;
            
            /* Adjust the total length with our extension data. */
            total_length -= (2 + extensions_length);
        }
#endif

        /* Advance the variable of total_length. */
        total_length -= (3 + cert_length);
        
        /* Assign the TLS Session metadata areas to the certificate for later use. */
        certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
        certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

        certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
        certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

        /* Add the certificate to the remote store. */
        /* Parse and initialize the remote certificate for use in subsequent operations. */
        status = _nx_secure_x509_certificate_list_add(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates,
                                                      certificate, NX_TRUE);

        /* Make sure we parsed a valid certificate. */
        if (status != NX_SUCCESS)
        {

            /* Translate some X.509 return values into TLS return values. */
            if (status == NX_SECURE_X509_CERT_ID_DUPLICATE)
            {
                return(NX_SECURE_TLS_CERT_ID_DUPLICATE);
            }

            return(status);
        }

        /* Make sure the certificate has it's cipher table initialized. */
        certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
        certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size;
    }
        
    /* =============================== CERTIFICATE CHAIN VERIFICATION ======================================== */
    /* Verify the certificates we received are valid against the trusted store. */
    status = _nx_secure_tls_remote_certificate_verify(tls_session);

    if(status != NX_SUCCESS)
    {
        return(status);
    }    

    /* ======================== Save off the endpoint certificate for later use. =============================*/
    /* Get the endpoint from the certificates we just parsed. */
    status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                             &certificate);
    NX_ASSERT(status == NX_SUCCESS);

    /* If the endpoint certificate was NOT allocated by the user application, we need to
        make a copy and save it for later use. */
    if(!certificate -> nx_secure_x509_user_allocated_cert)
    { 
        /* Free all certificates that we added to the packet buffer. Do this before the 
           call to nx_secure_x509_free_certificate_get so that if there are user-allocated
           certificates the endpoint is put into one of them. */
        status = _nx_secure_tls_remote_certificate_free_all(tls_session);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Save the raw pointer to endpoint so we can clear remote certificate state. */
        endpoint_raw_ptr = certificate->nx_secure_x509_certificate_raw_data;
        endpoint_length = certificate->nx_secure_x509_certificate_raw_data_length;

        /* Now allocate a new certificate for the endpoint. */
        status = _nx_secure_x509_free_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                    &certificate);

        /* If there are no free certificates, attempt to allocate from the packet reassembly buffer 
            (the certificate buffer is carved from the packet buffer in nx_secure_tls_process_record). */
        if (status != NX_SUCCESS)
        {
            /* No remote certificates added. Instead try extracting space from packet buffer. */
            cert_buffer = &tls_session -> nx_secure_tls_packet_buffer[data_length];
            cert_buf_size = tls_session -> nx_secure_tls_packet_buffer_size - data_length;

            /* Get space for the parsing structure. */
            cert_buf_size -= sizeof(NX_SECURE_X509_CERT);
            certificate = (NX_SECURE_X509_CERT*)(&cert_buffer[cert_buf_size]);
            NX_SECURE_MEMSET(certificate, 0, sizeof(NX_SECURE_X509_CERT));

            if(cert_buf_size < endpoint_length)
            {

                /* Not enough space to allocate the raw certificate data. */
                return(NX_SECURE_TLS_INSUFFICIENT_CERT_SPACE);
            }

            /* Allocate space for the endpoint certificate. */
            cert_buf_size -= endpoint_length;

            /* Point structure to certificate being parsed. */
            certificate -> nx_secure_x509_certificate_raw_data = &cert_buffer[cert_buf_size];
            certificate -> nx_secure_x509_certificate_raw_buffer_size = endpoint_length;

            /* Update total remaining size. */
            tls_session -> nx_secure_tls_packet_buffer_size -= (sizeof(NX_SECURE_X509_CERT) + endpoint_length);
        }

        /* Copy the certificate data to the end of the certificate buffer or use an allocated certificate. */
        certificate -> nx_secure_x509_certificate_raw_data_length = endpoint_length;
        NX_SECURE_MEMCPY(certificate->nx_secure_x509_certificate_raw_data, endpoint_raw_ptr, endpoint_length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
        
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        /* Re-parse the certificate using the original data. */
        status = _nx_secure_x509_certificate_parse(certificate -> nx_secure_x509_certificate_raw_data, endpoint_length, &bytes_processed, certificate);
        NX_ASSERT(status == NX_SUCCESS);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);
    
        /* Re-add the remote endpoint certificate for later use. */
        status = _nx_secure_x509_certificate_list_add(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_remote_certificates,
                                                      certificate, NX_TRUE);
        NX_ASSERT(status == NX_SUCCESS);

        /* Make sure the certificate has it's cipher table initialized. */
        certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
        certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size; 
    }
#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a ServerCertificate message, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else
    /* Set our state to indicate we successfully parsed the Certificate message. */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE;

    return(status);
#endif
}

