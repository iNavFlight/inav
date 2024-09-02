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

static UINT _nx_secure_tls_record_data_encrypt_init(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                                    ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                                    UCHAR record_type, UINT *data_offset,
                                                    const NX_CRYPTO_METHOD *session_cipher_method);

UCHAR _nx_secure_tls_record_block_buffer[NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_payload_encrypt               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts the payload of an outgoing TLS record using  */
/*    the session keys generated and ciphersuite determined during the    */
/*    TLS handshake.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Pointer to packet data        */
/*    sequence_num                          Current TLS/DTLS message num  */
/*    record_type                           TLS record type               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Decryption ciphers            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_record           Send DTLS encrypted record    */
/*    _nx_secure_tls_send_record            Send TLS encrypted record     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            fixed data copy in chained  */
/*                                            packet,                     */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), called   */
/*                                            NX_CRYPTO_ENCRYPT_CALCULATE */
/*                                            to finalize the encryption  */
/*                                            of this record, resulting   */
/*                                            in version 6.1.8            */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            reorganized internal logic, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_record_payload_encrypt(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                           ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                           UCHAR record_type)
{
UINT                                  status;
UCHAR                                *iv;
const NX_CRYPTO_METHOD               *session_cipher_method;
UINT                                  block_size;
USHORT                                iv_size;
NX_PACKET                            *current_packet;
ULONG                                 current_length;
ULONG                                 rounded_length;
ULONG                                 copy_length;
ULONG                                 remainder_length;
UINT                                  data_offset = 0;
VOID                                 *handler = NX_NULL;
VOID                                 *crypto_method_metadata;
UINT                                 icv_size = 0;
UCHAR                                *icv_ptr = NX_NULL;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Select metadata based on the current mode. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {

        /* The socket is a TLS server, so use the server cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
    }
    else
    {

        /* The socket is a TLS client, so use the client cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
    }

    /* Select the encryption algorithm based on the ciphersuite. Then, using the session keys and the chosen
       cipher, encrypt the data in place. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;

    if (session_cipher_method -> nx_crypto_operation == NX_NULL)
    {

        /* No encryption needed. */
        return(NX_SUCCESS);
    }

    block_size = session_cipher_method -> nx_crypto_block_size_in_bytes;

    /* Get the size of the IV used by the session cipher. */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Make sure our block size is small enough to fit into our buffer. */
    NX_ASSERT((iv_size <= NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE) &&
              (block_size <= NX_SECURE_TLS_MAX_CIPHER_BLOCK_SIZE));
    status = _nx_secure_tls_record_data_encrypt_init(tls_session, send_packet, sequence_num,
                                                     record_type, &data_offset, session_cipher_method);
    if (status)
    {
        return(status);
    }

    /* Iterate through the packet chain using a temporary pointer. */
    current_packet = send_packet;

    /* Loop through all packets in the chain. */
    do
    {

        /* Get our current packet length. Use the data_offset from any previous iterations. */
        current_length = (ULONG)(current_packet -> nx_packet_append_ptr -
                                 current_packet -> nx_packet_prepend_ptr) - data_offset;

        /* See if there are more packets in the chain. */
        if (current_packet -> nx_packet_next == NX_NULL)
        {

            if (current_length)
            {

                /* Encrypt any remaining data in the current packet since it is our last. */
                /* Offset should be such that the remaining data in the packet is remainder_length bytes. Add the block-boundary
                   length to the previous offset to get this new offset. Also, add padding as this is the last bit of data to
                   be encrypted (all previous packets should have been encrypted without padding).  */
                status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_UPDATE,
                                                                      handler,
                                                                      (NX_CRYPTO_METHOD *)session_cipher_method,
                                                                      NX_NULL,
                                                                      0,
                                                                      &current_packet -> nx_packet_prepend_ptr[data_offset],
                                                                      current_length,
                                                                      NX_NULL,
                                                                      &current_packet -> nx_packet_prepend_ptr[data_offset],
                                                                      current_length,
                                                                      crypto_method_metadata,
                                                                      tls_session -> nx_secure_session_cipher_metadata_size,
                                                                      NX_NULL, NX_NULL);
                if (status != NX_SUCCESS)
                {
                    return(status);
                }

                /* CBC-mode ciphers need to have their IV's updated after encryption. */
                if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
                {

                    /* New IV is the last encrypted block of the output. */
                    NX_SECURE_MEMCPY(iv, &current_packet -> nx_packet_prepend_ptr[current_length + data_offset - iv_size], /*  lgtm[cpp/banned-api-usage-required-any] */
                                     iv_size); /* Use case of memcpy is verified. */
                }
            }
        }
        else
        {

            /* Figure out how much we can encrypt. Get an evenly-divisible
               block of data and the remainder. */
            if (block_size == 0)
            {
                remainder_length = 0;
            }
            else
            {
                remainder_length = (ULONG)((current_length % block_size));
            }
            rounded_length = current_length - remainder_length;

            if (rounded_length > 0)
            {
                /* Encrypt remaining data in the current packet from our previous offset to evenly-divisible block boundary. */
                status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_UPDATE,
                                                                      handler,
                                                                      (NX_CRYPTO_METHOD *)session_cipher_method,
                                                                      NX_NULL,
                                                                      0,
                                                                      &current_packet -> nx_packet_prepend_ptr[data_offset],
                                                                      rounded_length,
                                                                      NX_NULL,
                                                                      &current_packet -> nx_packet_prepend_ptr[data_offset],
                                                                      rounded_length,
                                                                      crypto_method_metadata,
                                                                      tls_session -> nx_secure_session_cipher_metadata_size,
                                                                      NX_NULL, NX_NULL);

                if (status != NX_SUCCESS)
                {
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */
                    return(status);
                }
            }

            if (remainder_length)
            {

                /* Copy data into temporary buffer for encryption.
                   Pointers:                          packet2->prepend_ptr[data_offset]---v
                   Lengths:   |  current length |  remainder |  block size - remainder    |  packet 2 remainder   |
                   Data:      |**Packet 1 data***************|**Packet 2 data*************************************|
                   Temporary:                   |   record block buffer  [block size]     |
                 */

                /* Offset for remainder bytes is rounded_length + data_offset. */
                NX_SECURE_MEMCPY(&_nx_secure_tls_record_block_buffer[0], /*  lgtm[cpp/banned-api-usage-required-any] */
                                 &current_packet -> nx_packet_prepend_ptr[rounded_length + data_offset],
                                 remainder_length); /* Use case of memcpy is verified. */
                copy_length = (ULONG)(current_packet -> nx_packet_next -> nx_packet_append_ptr -
                                      current_packet -> nx_packet_next -> nx_packet_prepend_ptr);
                if (copy_length > (ULONG)(block_size - remainder_length))
                {
                    copy_length = (ULONG)(block_size - remainder_length);
                }
                NX_SECURE_MEMCPY(&_nx_secure_tls_record_block_buffer[remainder_length], /* lgtm[cpp/banned-api-usage-required-any] */
                                 current_packet -> nx_packet_next -> nx_packet_prepend_ptr,
                                 copy_length); /* Use case of memcpy is verified. */

                /* Encrypt the remainder block. */
                status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_UPDATE,
                                                                      handler,
                                                                      (NX_CRYPTO_METHOD *)session_cipher_method,
                                                                      NX_NULL,
                                                                      0,
                                                                      _nx_secure_tls_record_block_buffer,
                                                                      (remainder_length + copy_length),
                                                                      NX_NULL,
                                                                      _nx_secure_tls_record_block_buffer,
                                                                      (remainder_length + copy_length),
                                                                      crypto_method_metadata,
                                                                      tls_session -> nx_secure_session_cipher_metadata_size,
                                                                      NX_NULL, NX_NULL);

                if (status != NX_SUCCESS)
                {
#ifdef NX_SECURE_KEY_CLEAR
                    NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */

                    return(status);
                }

                /* Copy data from temporary buffer back into packets. */
                NX_SECURE_MEMCPY(&current_packet -> nx_packet_prepend_ptr[rounded_length + data_offset], /*  lgtm[cpp/banned-api-usage-required-any] */
                                 &_nx_secure_tls_record_block_buffer[0], remainder_length); /* Use case of memcpy is verified. */
                NX_SECURE_MEMCPY(current_packet -> nx_packet_next -> nx_packet_prepend_ptr, /* lgtm[cpp/banned-api-usage-required-any] */
                                 &_nx_secure_tls_record_block_buffer[remainder_length],
                                 copy_length); /* Use case of memcpy is verified. */

                /* CBC-mode ciphers need to have their IV's updated after encryption. */
                if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
                {

                    /* New IV is the last encrypted block of the output. */
                    NX_SECURE_MEMCPY(iv, &_nx_secure_tls_record_block_buffer, iv_size); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
                }

#ifdef NX_SECURE_KEY_CLEAR
                NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */

                /* Finally, our new offset for the next round is the number of bytes we already
                   encrypted (along with the remainder bytes) in the next packet. */
                data_offset = copy_length;
            }
            else
            {
                data_offset = 0;
            }
        }

        /* Move to the next packet. */
        current_packet = current_packet -> nx_packet_next;
    } while (current_packet != NX_NULL);

    if (session_cipher_method -> nx_crypto_ICV_size_in_bits > 0)
    {

        /* Get icv_size and icv_ptr for AEAD cipher */
        if ((session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3) >
            sizeof(_nx_secure_tls_record_block_buffer))
        {
            return(NX_SIZE_ERROR);
        }

        icv_size = session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3;
        icv_ptr = _nx_secure_tls_record_block_buffer;
    }

    /* Call NX_CRYPTO_ENCRYPT_CALCULATE to finalize the encryption of this record. */
    status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_CALCULATE,
                                                              handler,
                                                              (NX_CRYPTO_METHOD*)session_cipher_method,
                                                              NX_NULL,
                                                              NX_NULL,
                                                              NX_NULL,
                                                              0,
                                                              NX_NULL,
                                                              icv_ptr,
                                                              icv_size,
                                                              crypto_method_metadata,
                                                              tls_session -> nx_secure_session_cipher_metadata_size,
                                                              NX_NULL, NX_NULL);
    if (status)
    {
        return(status);
    }

    if (icv_ptr && icv_size)
    {

        /* Append data for AEAD cipher */
        status = nx_packet_data_append(send_packet, icv_ptr, icv_size,
                                           tls_session -> nx_secure_tls_packet_pool, NX_WAIT_FOREVER);

        return(status);
    }
    
    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_record_data_encrypt_init             PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is a static helper function used to initialize        */
/*    metadata for encryption.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Pointer to packet data        */
/*    sequence_num                          Current TLS/DTLS message num  */
/*    record_type                           TLS record type               */
/*    data_offset                           Data offset for Initial Vector*/
/*    session_cipher_method                 Pointer to cipher method      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Decryption ciphers            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_record_payload_encrypt Encrypt payload               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            reorganized internal logic, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_secure_tls_record_data_encrypt_init(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet,
                                                    ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                                    UCHAR record_type, UINT *data_offset,
                                                    const NX_CRYPTO_METHOD *session_cipher_method)
{
UINT                                  status;
UCHAR                                *iv;
VOID                                 *handler = NX_NULL;
VOID                                 *crypto_method_metadata;
UINT                                  block_size;
USHORT                                iv_size;
UCHAR                                 padding_length;
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UCHAR                                 additional_data[13];
UINT                                  additional_data_size = 0;
UCHAR                                 nonce[13];
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT                                  record_length;
#endif
UINT                                  message_length;
#else
    NX_PARAMETER_NOT_USED(sequence_num);
    NX_PARAMETER_NOT_USED(record_type);
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

    /* Select IV based on the current mode. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {

        /* The socket is a TLS server, so use the server cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv;
        handler = tls_session -> nx_secure_session_cipher_handler_server;
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_server;
    }
    else
    {

        /* The socket is a TLS client, so use the client cipher to encrypt. */
        iv = tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv;
        handler = tls_session -> nx_secure_session_cipher_handler_client;
        crypto_method_metadata = tls_session -> nx_secure_session_cipher_metadata_area_client;
    }

    /* Offset into current packet data. */
    *data_offset = 0;

    /* See if we need to add any data to the beginning of the payload such as an IV (e.g. for AES-CBC mode). */
    /* !!! NOTE: This relies on nx_secure_tls_packet_allocate reserving block_size bytes between NX_PACKET.nx_packet_prepend_ptr
                 and nx_packet_append_ptr !!! */
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    if ((session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_8) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_12) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CCM_16) ||
        (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_16) ||
        NX_SECURE_AEAD_CIPHER_CHECK(session_cipher_method -> nx_crypto_algorithm))
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session -> nx_secure_tls_1_3)
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

            /* There is no explicit ICV in TLS 1.3 AEAD. */
            record_length = send_packet -> nx_packet_length + (session_cipher_method -> nx_crypto_ICV_size_in_bits >> 3);
            message_length = send_packet -> nx_packet_length;

            additional_data[0] = record_type;
            additional_data[1] = (UCHAR)(0x03);
            additional_data[2] = (UCHAR)(0x03);
            additional_data[3] = (UCHAR)((record_length) >> 8);
            additional_data[4] = (UCHAR)(record_length);

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

            /* The length of CCMClientNonce or CCMServerNonce is 12 bytes.  */
            nonce[0] = 12;

            /* Copy client_write_IV or server_write_IV.  */
            NX_SECURE_MEMCPY(&nonce[1], iv, 4); /* Use case of memcpy is verified. */

            /* Correct the endianness of our sequence number before hashing. */
            nonce[5] = (UCHAR)(sequence_num[1] >> 24);
            nonce[6] = (UCHAR)(sequence_num[1] >> 16);
            nonce[7] = (UCHAR)(sequence_num[1] >> 8);
            nonce[8] = (UCHAR)(sequence_num[1]);
            nonce[9] = (UCHAR)(sequence_num[0] >> 24);
            nonce[10] = (UCHAR)(sequence_num[0] >> 16);
            nonce[11] = (UCHAR)(sequence_num[0] >> 8);
            nonce[12] = (UCHAR)(sequence_num[0]);

            /*  additional_data = seq_num + TLSCompressed.type +
                            TLSCompressed.version + TLSCompressed.length;
             */
            message_length = send_packet -> nx_packet_length - 8;
            NX_SECURE_MEMCPY(additional_data, &nonce[5], 8); /* Use case of memcpy is verified. */
            additional_data[8]  = record_type;
            additional_data[9]  = (UCHAR)(tls_session -> nx_secure_tls_protocol_version >> 8);
            additional_data[10] = (UCHAR)(tls_session -> nx_secure_tls_protocol_version);
            additional_data[11] = (UCHAR)(message_length >> 8);
            additional_data[12] = (UCHAR)(message_length);

            /* We have 13 bytes of additional data (8 bytes seq num + 5 bytes header). */
            additional_data_size = 13;

            /* Copy our IV into our data buffer at the head of the payload. */
            NX_SECURE_MEMCPY(send_packet -> nx_packet_prepend_ptr, &nonce[5], 8); /* Use case of memcpy is verified. */
            *data_offset = 8;
        }

        /* Initialize crypto algorithm.  */
        status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_INITIALIZE,
                                                              handler,
                                                              (NX_CRYPTO_METHOD*)session_cipher_method,
                                                              NX_NULL,
                                                              0,
                                                              additional_data,
                                                              additional_data_size,
                                                              nonce,
                                                              NX_NULL,
                                                              message_length,
                                                              crypto_method_metadata,
                                                              tls_session -> nx_secure_session_cipher_metadata_size,
                                                              NX_NULL, NX_NULL);
        return(status);
    }
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

    block_size = session_cipher_method -> nx_crypto_block_size_in_bytes;

    /* Get the size of the IV used by the session cipher. */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    if (session_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
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

        /* TLS 1.0 does not use an explicit IV in CBC-mode ciphers, so don't include it
           in the record. */
        if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_0)
        {
            if (iv_size > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_prepend_ptr)))
            {

                /* Packet buffer too small. */
                return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
            }

            /* IV size is equal to the AES block size. Copy our IV into our data buffer
               at the head of the payload. */
            NX_SECURE_MEMCPY(send_packet -> nx_packet_prepend_ptr, iv, iv_size); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
            *data_offset = iv_size;
        }

        if (iv_size != block_size)
        {

            /* Invalid size. */
            return(NX_SECURE_TLS_INVALID_STATE);
        }

        /* Initialize crypto algorithm.  */
        status = session_cipher_method -> nx_crypto_operation(NX_CRYPTO_ENCRYPT_INITIALIZE,
                                                              handler,
                                                              (NX_CRYPTO_METHOD*)session_cipher_method,
                                                              NX_NULL,
                                                              0,
                                                              NX_NULL,
                                                              0,
                                                              iv,
                                                              NX_NULL,
                                                              0,
                                                              crypto_method_metadata,
                                                              tls_session -> nx_secure_session_cipher_metadata_size,
                                                              NX_NULL, NX_NULL);

        if (status)
        {
            return(status);
        }
    }

    /* Padding - final output must be an integral multiple of the block size (16 bytes for AES
     * modes used in TLS). If the data is not a multiple, the padding consists of bytes each
     * with the value of the length of the padding (e.g. for 3 bytes, the padding would be 0x03,
     * 0x03, 0x03).
     */
    if (block_size > 0)
    {
        padding_length = (UCHAR)(block_size -
                                 (send_packet -> nx_packet_length % block_size));
        NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, padding_length - 1, padding_length);
        status = nx_packet_data_append(send_packet, _nx_secure_tls_record_block_buffer,
                                       padding_length, tls_session -> nx_secure_tls_packet_pool,
                                       NX_WAIT_FOREVER);
#ifdef NX_SECURE_KEY_CLEAR
        NX_SECURE_MEMSET(_nx_secure_tls_record_block_buffer, 0, block_size);
#endif /* NX_SECURE_KEY_CLEAR  */

        return(status);
    }

    return(NX_SUCCESS);
}
