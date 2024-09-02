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

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
static UCHAR save_iv[20]; /* Must be large enough to hold the block size for session ciphers! */
#endif

static UINT _nx_secure_tls_record_chained_packet_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                                         NX_PACKET *encrypted_packet, UINT offset,
                                                         UINT message_length, NX_PACKET **decrypted_packet,
                                                         UCHAR *additional_data, UINT additional_data_size,
                                                         UCHAR *iv, UINT wait_option);
static UINT _nx_secure_tls_record_packet_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                                 const NX_CRYPTO_METHOD *session_cipher_method,
                                                 NX_PACKET *encrypted_packet, UINT offset, UINT message_length,
                                                 NX_PACKET *decrypted_packet, UINT *bytes_processed,
                                                 UINT wait_option);
static UINT _nx_secure_tls_data_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                        UCHAR *input, UCHAR *output, UINT length);

/* Defined in nx_secure_tls_record_payload_encrypt.c */
extern UCHAR _nx_secure_tls_record_block_buffer[NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_payload_decrypt               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decrypts the payload of an incoming TLS record using  */
/*    the session keys generated and ciphersuite determined during the    */
/*    TLS handshake.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    encrypted_packet                      Pointer to packet containing  */
/*                                            encrypted_packet            */
/*    offset                                Offset of message data in     */
/*                                            encrypted_packet            */
/*    message_length                        Length of message data in     */
/*                                            encrypted_packet            */
/*    decrypted_packet                      Pointer to packet containing  */
/*                                            decrypted_packet            */
/*    sequence_num                          Record sequence number        */
/*    record_type                           Record type                   */
/*    wait_option                           Control timeout options       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    nx_secure_tls_packet_release          Release packet                */
/*    _nx_secure_tls_record_chained_packet_decrypt                        */
/*                                          Decrypt chained packet        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record data      */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_payload_decrypt(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *encrypted_packet,
                                           UINT offset, UINT message_length, NX_PACKET **decrypted_packet,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type, UINT wait_option)
{
UINT                                  status;
UCHAR                                *iv;
UCHAR                                 padding_length;
UCHAR                                 copy_size;
const NX_CRYPTO_METHOD               *session_cipher_method;
USHORT                                iv_size;
UINT                                  i;
ULONG                                 bytes_copied;
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT                                  icv_size;
UCHAR                                 additional_data[13];
UINT                                  additional_data_size;
UCHAR                                 nonce[13];
#else
    NX_PARAMETER_NOT_USED(sequence_num);
    NX_PARAMETER_NOT_USED(record_type);
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */


    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Select the decryption algorithm based on the ciphersuite. Then, using the session keys and the chosen
       cipher, decrypt the data. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    /* Get the size of the IV used by the session cipher. */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Select our proper data structures. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {

        /* The socket is a TLS server, so use the *CLIENT* cipher to decrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
    }
    else
    {

        /* The socket is a TLS client, so use the *SERVER* cipher to decrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
    }

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    if ((session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_8) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_12) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_16) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_16) ||
        NX_SECURE_AEAD_CIPHER_CHECK(session_cipher_method -> nx_crypto_algorithm))
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        /* TLS 1.3 AEAD uses a different Nonce construction from earlier versions. */
        if(tls_session->nx_secure_tls_1_3)
        {
            /*
             Each AEAD algorithm will specify a range of possible lengths for the
             per-record nonce, from N_MIN bytes to N_MAX bytes of input [RFC5116].
             The length of the TLS per-record nonce (iv_length) is set to the
             larger of 8 bytes and N_MIN for the AEAD algorithm (see [RFC5116],
             Section 4).  An AEAD algorithm where N_MAX is less than 8 bytes
             MUST NOT be used with TLS.  The per-record nonce for the AEAD
             construction is formed as follows:

             1.  The 64-bit record sequence number is encoded in network byte
                 order and padded to the left with zeros to iv_length.

             2.  The padded sequence number is XORed with either the static
                 client_write_iv or server_write_iv (depending on the role).

             The resulting quantity (of length iv_length) is used as the
             per-record nonce.

             Note: This is a different construction from that in TLS 1.2, which
             specified a partially explicit nonce.
             */

            icv_size = (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3);

            /* The length of the nonce is 12 bytes.  */
            nonce[0] = 12;

            /* Copy client_write_IV or server_write_IV.  */
            NX_SECURE_MEMCPY(&nonce[1], iv, 12); /* Use case of memcpy is verified. */

            /* Correct the endianness of our sequence number and XOR with
             * the IV. Pad to the left with zeroes. */
            nonce[1]  = (UCHAR)(nonce[1] ^ 0);
            nonce[2]  = (UCHAR)(nonce[2] ^ 0);
            nonce[3]  = (UCHAR)(nonce[3] ^ 0);
            nonce[4]  = (UCHAR)(nonce[4] ^ 0);
            nonce[5]  = (UCHAR)(nonce[5] ^ (sequence_num[1] >> 24));
            nonce[6]  = (UCHAR)(nonce[6] ^ (sequence_num[1] >> 16));
            nonce[7]  = (UCHAR)(nonce[7] ^ (sequence_num[1] >> 8));
            nonce[8]  = (UCHAR)(nonce[8] ^ (sequence_num[1]));
            nonce[9]  = (UCHAR)(nonce[9] ^ (sequence_num[0] >> 24));
            nonce[10] = (UCHAR)(nonce[10] ^ (sequence_num[0] >> 16));
            nonce[11] = (UCHAR)(nonce[11] ^ (sequence_num[0] >> 8));
            nonce[12] = (UCHAR)(nonce[12] ^ (sequence_num[0]));

            /*  additional_data = record header
             *  record header = TLSCiphertext.opaque_type ||
                                TLSCiphertext.legacy_record_version ||
                                TLSCiphertext.length
             */
            additional_data[0] = record_type;
            additional_data[1] = (UCHAR)(0x03);
            additional_data[2] = (UCHAR)(0x03);
            additional_data[3] = (UCHAR)((message_length) >> 8);
            additional_data[4] = (UCHAR)(message_length);


            /* We have 5 bytes of additional data. */
            additional_data_size = 5;
        }
        else
#endif
        {
            /* AEAD ciphers structure:
                 struct {
                     opaque nonce_explicit[SecurityParameters.record_iv_length];
                     aead-ciphered struct {
                         opaque content[TLSCompressed.length];
                     };
                 } GenericAEADCipher;
             */

            /* The nonce of the CCM cipher is passed into crypto method using iv_ptr.
               struct {
                   uint32 client_write_IV; // low order 32-bits
                   uint64 seq_num;         // TLS sequence number
               } CCMClientNonce.
               struct {
                   uint32 server_write_IV; // low order 32-bits
                   uint64 seq_num; // TLS sequence number
               } CCMServerNonce.
             */


            icv_size = (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3);

            if (message_length < (8 + icv_size))
            {
                return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
            }

            /* Remove length of nonce_explicit.  */
            message_length -= 8;

            /* The length of CCMClientNonce or CCMServerNonce is 12 bytes.  */
            nonce[0] = 12;

            /* Copy client_write_IV or server_write_IV.  */
            NX_SECURE_MEMCPY(&nonce[1], iv, 4); /* Use case of memcpy is verified. */

            /* Correct the endianness of our sequence number before hashing. */
            additional_data[0] = (UCHAR)(sequence_num[1] >> 24);
            additional_data[1] = (UCHAR)(sequence_num[1] >> 16);
            additional_data[2] = (UCHAR)(sequence_num[1] >> 8);
            additional_data[3] = (UCHAR)(sequence_num[1]);
            additional_data[4] = (UCHAR)(sequence_num[0] >> 24);
            additional_data[5] = (UCHAR)(sequence_num[0] >> 16);
            additional_data[6] = (UCHAR)(sequence_num[0] >> 8);
            additional_data[7] = (UCHAR)(sequence_num[0]);

            /* Copy nonce_explicit from the data.  */
            status = nx_packet_data_extract_offset(encrypted_packet, offset,
                                                   &nonce[5], 8, &bytes_copied);
            if (status || (bytes_copied != 8))
            {
                return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
            }

            /*  additional_data = seq_num + TLSCompressed.type +
                            TLSCompressed.version + TLSCompressed.length;
             */
            additional_data[8]  = record_type;
            additional_data[9]  = (UCHAR)(tls_session -> nx_secure_tls_protocol_version >> 8);
            additional_data[10] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version);
            additional_data[11] = (UCHAR)((message_length - icv_size) >> 8);
            additional_data[12] = (UCHAR)(message_length - icv_size);

            /* We have 13 bytes of additional data (8 bytes seq num + 5 bytes header). */
            additional_data_size = 13;

            /* Decrypt data following the nonce_explicit. */
            offset += 8;
        }

        if (message_length < icv_size)
        {
            return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
        }

        /* Decrypt the message payload using the session crypto method, and move the decrypted data to the beginning of the buffer. */
        if (session_cipher_method -> nx_crypto_operation)
        {

            /* Decrypt the message payload using the session crypto method, and move the decrypted data to the beginning of the buffer. */
            status = _nx_secure_tls_record_chained_packet_decrypt(tls_session, encrypted_packet, offset,
                                                                  message_length, decrypted_packet,
                                                                  additional_data,
                                                                  additional_data_size,
                                                                  nonce,
                                                                  wait_option);
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(additional_data, 0, sizeof(additional_data));
            NX_SECURE_MEMSET(nonce, 0, sizeof(nonce));
#endif /* NX_SECURE_KEY_CLEAR  */

            if (status == NX_CRYPTO_AUTHENTICATION_FAILED)
            {
                return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
            }

            if (status != NX_SECURE_TLS_SUCCESS)
            {
                return(status);
            }
        }
    }
    else
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */
    {
        /* CBC mode has a specific structure for encrypted data, so handle that here:
               block-ciphered struct {
                   opaque IV[CipherSpec.block_length];
                   opaque content[TLSCompressed.length];
                   opaque MAC[CipherSpec.hash_size];
                   uint8 padding[GenericBlockCipher.padding_length];
                   uint8 padding_length;
               } GenericBlockCipher;
         */

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        if ((iv_size > message_length) ||
            (iv_size > sizeof(save_iv)))
#else
        if (iv_size > message_length)
#endif
        {

            /* Message length error. */
            return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        /* TLS 1.0 does not include the IV in the record, so use the one from the session.*/
        if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0)
#endif
        {
            /* Copy IV from the beginning of the payload into our session buffer. */
            status = nx_packet_data_extract_offset(encrypted_packet, offset,
                                                   iv, iv_size, &bytes_copied);
            if (status || (bytes_copied != iv_size))
            {
                return(NX_SECURE_TLS_INVALID_PACKET);
            }
            offset += iv_size;

            /* Adjust payload length to account for IV that we saved off above. */
            message_length -= iv_size;
        }

        /* Decrypt the message payload using the session crypto method, and move the decrypted data to the beginning of the buffer. */
        status = _nx_secure_tls_record_chained_packet_decrypt(tls_session, encrypted_packet, offset,
                                                            message_length, decrypted_packet,
                                                            NX_NULL, 0, iv,
                                                            wait_option);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        if (session_cipher_method -> nx_crypto_operation == NX_NULL)
        {
            return(NX_SECURE_TLS_SUCCESS);
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED)
        /* Update our IV for CBC mode. */
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0)
        {
            if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
            {

                /* New IV is the last encrypted block of the output. */
                status = nx_packet_data_extract_offset(encrypted_packet, (offset + message_length) - iv_size,
                                                       iv, iv_size, &bytes_copied);
                if (status || (bytes_copied != iv_size))
                {
                    nx_secure_tls_packet_release(*decrypted_packet);
                    return(status);
                }
            }
        }
#endif

        /* Get padding length from the final byte - CBC padding consists of a number of
           bytes each with a value equal to the padding length (e.g. 0x3 0x3 0x3 for 3 bytes of padding). */
        message_length = (*decrypted_packet) -> nx_packet_length;
        status = nx_packet_data_extract_offset(*decrypted_packet, message_length - 1,
                                               &padding_length, 1, &bytes_copied);
        if (status)
        {
            nx_secure_tls_packet_release(*decrypted_packet);
            return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
        }

        /* If padding length is greater than our length,
          we have an error - don't check padding. */
        if (padding_length < message_length)
        {
            status = NX_SUCCESS;

            /* Check all padding values. */
            offset = (UINT)(message_length - (UINT)(1 + padding_length));
            while (offset < (message_length - 1))
            {
                copy_size = (UCHAR)(((message_length - 1) - offset) & 0xFF);
                if (copy_size > sizeof(_nx_secure_tls_record_block_buffer))
                {
                    copy_size = sizeof(_nx_secure_tls_record_block_buffer);
                }
                status = nx_packet_data_extract_offset(*decrypted_packet,
                                                       offset, _nx_secure_tls_record_block_buffer,
                                                       copy_size, &bytes_copied);
                NX_ASSERT(status == NX_SUCCESS);

                offset += bytes_copied;

                for(i = 0; i < bytes_copied; i++)
                {
                    if(_nx_secure_tls_record_block_buffer[i] != padding_length)
                    {

                        /* Padding byte is incorrect! */
                        status = NX_SECURE_TLS_PADDING_CHECK_FAILED;
                        break;
                    }
                }
            }
        }
        else
        {
            /* Decryption or padding error! */
            status = NX_SECURE_TLS_PADDING_CHECK_FAILED;
        }

        /* Adjust length to remove padding. */
        /* Simply set packet length. The packet will be adjusted by caller. */
        message_length -= (UINT)(padding_length + 1);
        (*decrypted_packet) -> nx_packet_length = message_length;

        /* Return error status if appropriate. */
        if(status != NX_SUCCESS)
        {
            nx_secure_tls_packet_release(*decrypted_packet);
            return(status);
        }
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_chained_packet_decrypt        PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decrypts the payload of an incoming TLS record in     */
/*    chained packet using the session keys generated and ciphersuite     */
/*    determined during the TLS handshake.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    encrypted_packet                      Pointer to packet containing  */
/*                                            encrypted_packet            */
/*    offset                                Offset of message data in     */
/*                                            encrypted_packet            */
/*    message_length                        Length of message data in     */
/*                                            encrypted_packet            */
/*    decrypted_packet                      Pointer to packet containing  */
/*                                            decrypted_packet            */
/*    additional_data                       Pointer to additional data    */
/*    additional_data_size                  Size of additional data       */
/*    iv                                    Pointer to initial vector     */
/*    wait_option                           Control timeout options       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    _nx_secure_tls_record_packet_decrypt  Decrypt packet in one packet  */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_record_payload_decrypt Decrypt TLS record payload    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            reorganized internal logic, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_record_chained_packet_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                                         NX_PACKET *encrypted_packet, UINT offset,
                                                         UINT message_length, NX_PACKET **decrypted_packet,
                                                         UCHAR *additional_data, UINT additional_data_size,
                                                         UCHAR *iv, UINT wait_option)
{
UINT status;
UCHAR *icv_ptr;
UINT icv_size;
UINT bytes_processed;
ULONG bytes_copied;
NX_PACKET *packet_ptr;
UINT icv_offset = offset;
VOID *handler = NX_NULL;
VOID *crypto_method_metadata;
const NX_CRYPTO_METHOD *session_cipher_method;

    /* Get ICV size. It is zero for CBC mode and non zero for AEAD mode. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;
    icv_size = session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3;
    if ((icv_size > message_length) || (icv_size > sizeof(_nx_secure_tls_record_block_buffer)))
    {

        /* Invalid packet. */
        return(NX_SECURE_TLS_INVALID_PACKET);
    }
    message_length -= icv_size;
    icv_offset += message_length;

    /* Select our proper data structures. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {

        /* The socket is a TLS server, so use the *CLIENT* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
    }
    else
    {

        /* The socket is a TLS client, so use the *SERVER* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
    }

    /* Set additional data pointer and length.  */
    if (session_cipher_method -> nx_crypto_operation)
    {

        /* Set additional data pointer and length.  */
        status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT_INITIALIZE,
                                                            handler,
                                                            (NX_CRYPTO_METHOD*)session_cipher_method,
                                                            NX_NULL, 0,
                                                            additional_data,
                                                            additional_data_size,
                                                            iv,
                                                            NX_NULL,
                                                            message_length,
                                                            crypto_method_metadata,
                                                            tls_session -> nx_secure_session_cipher_metadata_size,
                                                            NX_NULL, NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Allocate another packet for decryption. */
    status = nx_packet_allocate(tls_session -> nx_secure_tls_packet_pool, &packet_ptr, 0, wait_option);
    if (status)
    {
        return(status);
    }
    packet_ptr -> nx_packet_last = packet_ptr;

    /* Loop to decrypt data in chained packet. */
    while (message_length > 0)
    {
        status = _nx_secure_tls_record_packet_decrypt(tls_session, session_cipher_method, encrypted_packet,
                                                      offset, message_length, packet_ptr, &bytes_processed, wait_option);
        if (status)
        {
            nx_secure_tls_packet_release(packet_ptr);
            return(status);
        }

        offset += bytes_processed;
        message_length -= bytes_processed;
    }

    if (session_cipher_method -> nx_crypto_operation == NX_NULL)
    {

        /* Nothing to do for null encryption. */
        *decrypted_packet = packet_ptr;
        return(NX_SECURE_TLS_SUCCESS);
    }

    /* Extract ICV. */
    icv_ptr = _nx_secure_tls_record_block_buffer;
    if (icv_size > 0)
    {
        status = nx_packet_data_extract_offset(encrypted_packet, icv_offset,
                                               icv_ptr, icv_size, &bytes_copied);
        if (status || (bytes_copied != icv_size))
        {
            nx_secure_tls_packet_release(packet_ptr);
            return(NX_SECURE_TLS_INVALID_PACKET);
        }
    }

    status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT_CALCULATE,
                                                          handler,
                                                          (NX_CRYPTO_METHOD*)session_cipher_method,
                                                          NX_NULL, 0,
                                                          icv_ptr,
                                                          icv_size,
                                                          NX_NULL,
                                                          NX_NULL,
                                                          0,
                                                          crypto_method_metadata,
                                                          tls_session -> nx_secure_session_cipher_metadata_size,
                                                          NX_NULL, NX_NULL);

    if (status)
    {
        nx_secure_tls_packet_release(packet_ptr);
        if (status == NX_CRYPTO_AUTHENTICATION_FAILED)
        {
            return(NX_SECURE_TLS_AEAD_DECRYPT_FAIL);
        }
        else
        {
            return(status);
        }
    }

    *decrypted_packet = packet_ptr;
    return(NX_SECURE_TLS_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_packet_decrypt                PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decrypts the payload of an incoming TLS record in     */
/*    one packet using the session keys generated and ciphersuite         */
/*    determined during the TLS handshake.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    session_cipher_method                 Pointer to cipher method      */
/*    encrypted_packet                      Pointer to packet containing  */
/*                                            encrypted_packet            */
/*    offset                                Offset of message data in     */
/*                                            encrypted_packet            */
/*    message_length                        Length of message data in     */
/*                                            encrypted_packet            */
/*    decrypted_packet                      Pointer to packet containing  */
/*                                            decrypted_packet            */
/*    bytes_processed                       Output bytes processed        */
/*    wait_option                           Control timeout options       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*    _nx_secure_tls_data_decrypt           Decrypt data                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_record_chained_packet_decrypt                        */
/*                                          Decrypt chained packet        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the issue of endless loop,  */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_record_packet_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                                 const NX_CRYPTO_METHOD *session_cipher_method,
                                                 NX_PACKET *encrypted_packet, UINT offset, UINT message_length,
                                                 NX_PACKET *decrypted_packet, UINT *bytes_processed,
                                                 UINT wait_option)
{
UINT status;
UCHAR *input;
UCHAR *output;
NX_PACKET *original_encrypted_packet = encrypted_packet;
NX_PACKET *packet_ptr;
UINT length;
UINT block_size;
UINT encrypted_length;
UINT decrypted_length;
ULONG bytes_copied;
UINT remainder_length;
UINT original_offset = offset;

    /* Total packet length is no less than message length from nx_secure_tls_process_record.c.
     * We don't need to validate packet error in this function.  */

    /* block_size must be no larger than NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE. */
    block_size = session_cipher_method -> nx_crypto_block_size_in_bytes;
    NX_ASSERT(block_size <= sizeof(_nx_secure_tls_record_block_buffer));

    /* Locate input data. */
    while (offset)
    {
        encrypted_length = (UINT)(encrypted_packet -> nx_packet_append_ptr - encrypted_packet -> nx_packet_prepend_ptr);
        if (encrypted_length > offset)
        {

            /* Input data are in current packet*/
            break;
        }

        /* Go to next packet. */
        offset -= encrypted_length;
        encrypted_packet = encrypted_packet -> nx_packet_next;
    }

    /* Get actual encrypted_length in current packet. */
    encrypted_length = (UINT)(encrypted_packet -> nx_packet_append_ptr - encrypted_packet -> nx_packet_prepend_ptr);
    encrypted_length -= offset;
    if (encrypted_length < block_size)
    {

        /* Use _nx_secure_tls_record_block_buffer for decryption. */
        status = nx_packet_data_extract_offset(original_encrypted_packet, original_offset,
                                               _nx_secure_tls_record_block_buffer, block_size, &bytes_copied);
        if (status)
        {
            return(status);
        }
        input = _nx_secure_tls_record_block_buffer;
        length = bytes_copied;
    }
    else
    {
        input = encrypted_packet -> nx_packet_prepend_ptr + offset;
        length = encrypted_length;

        if (block_size)
        {
            
            /* Get rounded length. */
            remainder_length = length % block_size;
        }
        else
        {
            remainder_length = 0;
        }
        
        length -= remainder_length;
    }
    if (length > message_length)
    {
        length = message_length;
    }

    /* decrypted_packet can not be NULL. */
    NX_ASSERT(decrypted_packet != NX_NULL);

    /* decrypted_packet -> nx_packet_last is set by caller. */
    /* Get available size of decrypted_packet. */
    packet_ptr = decrypted_packet -> nx_packet_last;
    decrypted_length = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr);

    if (decrypted_length == 0)
    {

        /* Allocate another packet for decryption. */
        status = nx_packet_allocate(tls_session -> nx_secure_tls_packet_pool, &packet_ptr, 0, wait_option);
        if (status)
        {
            return(status);
        }

        /* Link to decrypted_packet. */
        decrypted_packet -> nx_packet_last -> nx_packet_next = packet_ptr;
        decrypted_packet -> nx_packet_last = packet_ptr;
        decrypted_length = (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr);
    }

    if ((decrypted_length < block_size) && (decrypted_length < length))
    {

        /* Use _nx_secure_tls_record_block_buffer for decryption output. */
        output = _nx_secure_tls_record_block_buffer;
        decrypted_length = block_size;
    }
    else
    {

        /* Decrypt output to packet directly. */
        output = packet_ptr -> nx_packet_append_ptr;

        if ((block_size) && (decrypted_length > block_size))
        {
            
            /* Get rounded length. */
            remainder_length = decrypted_length % block_size;
        }
        else
        {
            remainder_length = 0;
        }
        
        decrypted_length -= remainder_length;
    }
    if (length > decrypted_length)
    {
        length = decrypted_length;
    }

    if (session_cipher_method -> nx_crypto_operation == NX_NULL)
    {
        NX_SECURE_MEMCPY(output, input, length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
    }
    else
    {

        /* Decrypt data. */
        status = _nx_secure_tls_data_decrypt(tls_session, input, output, length);
        if (status)
        {
            return(status);
        }
    }

    if (output == _nx_secure_tls_record_block_buffer)
    {

        /* Append decrypted data. */
        status = nx_packet_data_append(decrypted_packet, output, length,
                                       tls_session -> nx_secure_tls_packet_pool, wait_option);
        if (status)
        {
            return(status);
        }
    }
    else
    {

        /* Adjust decrypted packet. */
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_append_ptr + length;
        decrypted_packet -> nx_packet_length += length;
    }

    *bytes_processed = length;

    return(NX_SECURE_TLS_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_data_decrypt                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function decrypts partial incoming TLS record data using the   */
/*    session keys generated and ciphersuite determined during the TLS    */
/*    handshake.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    input                                 Pointer to cipher data        */
/*    output                                Output of plain data          */
/*    length                                Length of message data        */
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
/*    _nx_secure_tls_record_packet_decrypt  Decrypt data in packet        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020     Timothy Stapko           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_data_decrypt(NX_SECURE_TLS_SESSION *tls_session,
                                        UCHAR *input, UCHAR *output, UINT length)
{
UINT                    status;
VOID                   *handler = NX_NULL;
VOID                   *crypto_method_metadata;
const NX_CRYPTO_METHOD *session_cipher_method;

    /* Select our proper data structures. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {

        /* The socket is a TLS server, so use the *CLIENT* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
    }
    else
    {

        /* The socket is a TLS client, so use the *SERVER* cipher to decrypt. */
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
    }

    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    /* Decrypt data.  */
    status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT_UPDATE,
                                                          handler,
                                                          (NX_CRYPTO_METHOD*)session_cipher_method,
                                                          NX_NULL, 0,
                                                          input,
                                                          length,
                                                          NX_NULL,
                                                          output,
                                                          length,
                                                          crypto_method_metadata,
                                                          tls_session -> nx_secure_session_cipher_metadata_size,
                                                          NX_NULL, NX_NULL);

    return(status);
}
