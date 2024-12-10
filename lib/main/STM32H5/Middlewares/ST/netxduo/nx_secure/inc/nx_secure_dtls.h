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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_dtls.h                                    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all service prototypes and data structure         */
/*    definitions for DTLS implementation.                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.3  */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            fixed out-of-order handling,*/
/*                                            updated cookie handling,    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_DTLS_H_
#define SRC_NX_SECURE_DTLS_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_secure_tls.h"

/* DTLS protocol versions. These are needed in the TLS header to support DTLS functionality in
   shared DTLS/TLS services. */
#define NX_SECURE_DTLS_VERSION_MAJOR              0xFE
#define NX_SECURE_DTLS_VERSION_MINOR_1_0          0xFF
#define NX_SECURE_DTLS_VERSION_MINOR_1_2          0xFD

#define NX_SECURE_DTLS_VERSION_1_0                ((NX_SECURE_DTLS_VERSION_MAJOR << 8) | NX_SECURE_DTLS_VERSION_MINOR_1_0)
#define NX_SECURE_DTLS_VERSION_1_2                ((NX_SECURE_DTLS_VERSION_MAJOR << 8) | NX_SECURE_DTLS_VERSION_MINOR_1_2)

/* DTLS constants. */
#define NX_SECURE_DTLS_RECORD_HEADER_SIZE         (NX_SECURE_TLS_RECORD_HEADER_SIZE + 8)    /* Size of the DTLS record header in bytes. */
#define NX_SECURE_DTLS_HANDSHAKE_HEADER_SIZE      (NX_SECURE_TLS_HANDSHAKE_HEADER_SIZE + 8) /* Size of the DTLS handshake record header in bytes. */


/* Default DTLS retransmit rate of 1 second. */
#ifndef NX_SECURE_DTLS_RETRANSMIT_TIMEOUT
#define NX_SECURE_DTLS_RETRANSMIT_TIMEOUT         NX_IP_PERIODIC_RATE
#endif /* NX_SECURE_DTLS_RETRANSMIT_TIMEOUT  */

/* Default maximum DTLS retransmit rate of 60 seconds. */
#ifndef NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT
#define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT (60 * NX_IP_PERIODIC_RATE)
#endif /* NX_SECURE_DTLS_MAX_RETRANSMIT_TIMEOUT  */

/* Default maximum DTLS retransmit retries. */
#ifndef NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES
#define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES 10
#endif /* NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES  */

/* This define specifies how the retransmit timeout period changes between successive retries. If this
   value is 0, the initial retransmit timeout is the same as subsequent retransmit timeouts. If this
   value is 1, each successive retransmit is twice as long. The default value is 1.  */
#ifndef NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT
#define NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT     1
#endif /* NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT */

/* Default DTLS Cookie length.  */
#ifndef NX_SECURE_DTLS_COOKIE_LENGTH
#define NX_SECURE_DTLS_COOKIE_LENGTH              32
#endif /* NX_SECURE_DTLS_COOKIE_LENGTH  */

/* The cookie size limit for DTLS 1.2 clinet. */
#define NX_SECURE_DTLS_MAX_COOKIE_LENGTH          255

/* Event flag masks for DTLS retransmit thread. */
#define NX_SECURE_DTLS_ALL_EVENTS                 ((ULONG)0xFFFFFFFF)   /* All event flags              */
#define NX_SECURE_DTLS_PERIODIC_EVENT             ((ULONG)0x00000001)   /* Periodic event               */

/* Forward declaration for pointer in DTLS session structure. */
struct NX_SECURE_DTLS_SERVER_STRUCT;

/* Definition of the top-level DTLS session control block used by the application. */
typedef struct NX_SECURE_DTLS_SESSION_STRUCT
{
    /* TLS state not specific to DTLS is stored in the TLS session. */
    NX_SECURE_TLS_SESSION nx_secure_dtls_tls_session;

    /* Underlying UDP socket for DTLS. */
    NX_UDP_SOCKET *nx_secure_dtls_udp_socket;

    /* UDP doesn't have a persistent state like TCP, so save off IP address and Port. */
    UINT        nx_secure_dtls_local_ip_address_index;
    UINT        nx_secure_dtls_local_port;

    /* Save remote IP and port. */
    NXD_ADDRESS nx_secure_dtls_remote_ip_address;
    UINT        nx_secure_dtls_remote_port;

    /* Flag for session is in use or not. */
    UINT nx_secure_dtls_session_in_use;

    /* The DTLS handshake starts with a cookie exchange, save it here. */
    USHORT nx_secure_dtls_cookie_length;
    UCHAR  nx_secure_dtls_cookie[NX_SECURE_DTLS_COOKIE_LENGTH];

    UCHAR *nx_secure_dtls_client_cookie_ptr;

    /* The DTLS handshake messages have a sequence number that is incremented
       with each message sent. */
    USHORT nx_secure_dtls_local_handshake_sequence;

    /* Save off the current fragment length (how much we have reassembled) so we know when
     * we have complete handshake message data. */
    UINT nx_secure_dtls_fragment_length;

    /* Sequence number of the current handshake record, used for fragmentation. */
    UINT nx_secure_dtls_remote_handshake_sequence;

    /* Current expected sequence number for DTLS handshake messages. */
    UINT nx_secure_dtls_expected_handshake_sequence;

    /* The DTLS epoch, used as the first part of the explicit DTLS sequence number. */
    USHORT nx_secure_dtls_local_epoch;
    USHORT nx_secure_dtls_remote_epoch;

    /* Define the DTLS sent queue. This queue is used to keep track of transmitted packets
       already sent. DTLS will keep packets in this queue until the response flight is
       received from the remote host, at which point the DTLS stack will release them.  */
    ULONG      nx_secure_dtls_transmit_queue_maximum;
    ULONG      nx_secure_dtls_transmit_sent_count;
    NX_PACKET *nx_secure_dtls_transmit_sent_head,
              *nx_secure_dtls_transmit_sent_tail;

    /* Create a timer that is used to control the retransmission of dropped datagrams
       during the DTLS handshake. */
    ULONG nx_secure_dtls_handshake_timeout;
    ULONG nx_secure_dtls_timeout_retries;

    /* Pointer to parent DTLS server structure (for server sessions). */
    struct NX_SECURE_DTLS_SERVER_STRUCT *nx_secure_dtls_server_parent;

    /* Pointer to our UDP packet receive queue. */
    NX_PACKET *nx_secure_dtls_receive_queue_head;

    /* Bitfield used for sliding window checks. */
    ULONG nx_secure_dtls_sliding_window;

    /* Pointer to the thread waiting for packet. */
    TX_THREAD *nx_secure_dtls_thread_suspended;

    /* Define the link between other DTLS structures created by the application.  */
    struct NX_SECURE_DTLS_SESSION_STRUCT
        *nx_secure_dtls_created_previous,
        *nx_secure_dtls_created_next;

} NX_SECURE_DTLS_SESSION;


/* DTLS Server structure. Used to contain the information for handling multiple DTLS sessions on a single port.  */
typedef struct NX_SECURE_DTLS_SERVER_STRUCT
{
    /* Pointer to supplied IP instance for this DTLS server. */
    NX_IP *nx_dtls_server_ip_ptr;

    /* UDP socket shared by all sessions. */
    NX_UDP_SOCKET          nx_dtls_server_udp_socket;

    /* Pointer to session buffer - control blocks for each session in this server. */
    NX_SECURE_DTLS_SESSION *nx_dtls_server_sessions;

    /* Number of sessions assigned to this server. */
    UINT                    nx_dtls_server_sessions_count;

    /* The port this DTLS server is assigned to. */
    UINT                    nx_dtls_server_listen_port;

    /* Timeout value for the server. */
    ULONG                   nx_dtls_server_timeout;

    /* Notification callbacks for DTLS connections. */
    UINT (*nx_secure_dtls_connect_notify)(struct NX_SECURE_DTLS_SESSION_STRUCT *dtls_session, NXD_ADDRESS *ip_address, UINT port);
    UINT (*nx_secure_dtls_receive_notify)(struct NX_SECURE_DTLS_SESSION_STRUCT *dtls_session);
    UINT (*nx_secure_dtls_disconnect_notify)(struct NX_SECURE_DTLS_SESSION_STRUCT *dtls_session);
    UINT (*nx_secure_dtls_error_notify)(struct NX_SECURE_DTLS_SESSION_STRUCT *dtls_session, UINT error_code);

    /* This field overrides the version. */
    USHORT                  nx_dtls_server_protocol_version_override;
    UCHAR                   nx_dtls_server_reserved_field[2];

    /* Reserved for possible future use. */
    ULONG                   nx_dtls_server_reserved;

    /* Define the link between other DTLS server structures created by the application.  */
    struct NX_SECURE_DTLS_SERVER_STRUCT
        *nx_dtls_server_created_previous,
        *nx_dtls_server_created_next;
} NX_SECURE_DTLS_SERVER;





/* Define API functions. */
VOID _nx_secure_dtls_initialize(VOID);
UINT _nx_secure_dtls_server_create(NX_SECURE_DTLS_SERVER *server_ptr, NX_IP *ip_ptr, UINT port, ULONG timeout,
                                   VOID *session_buffer, UINT session_buffer_size,
                                   const NX_SECURE_TLS_CRYPTO *crypto_table,
                                   VOID *crypto_metadata_buffer, ULONG crypto_metadata_size,
                                   UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                   UINT (*connect_notify)(NX_SECURE_DTLS_SESSION *dtls_session, NXD_ADDRESS *ip_address, UINT port),
                                   UINT (*receive_notify)(NX_SECURE_DTLS_SESSION *dtls_session));

UINT _nx_secure_dtls_server_local_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                  NX_SECURE_X509_CERT *certificate, UINT cert_id);


UINT _nx_secure_dtls_server_start(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nx_secure_dtls_session_create(NX_SECURE_DTLS_SESSION *session_ptr,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *metadata_buffer, ULONG metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT certs_number,
                                    UCHAR *remote_certificate_buffer, ULONG remote_certificate_buffer_size);

UINT _nx_secure_dtls_session_delete(NX_SECURE_DTLS_SESSION *dtls_session);
UINT _nx_secure_dtls_session_end(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);
UINT _nx_secure_dtls_session_receive(NX_SECURE_DTLS_SESSION *dtls_session,
                                     NX_PACKET **packet_ptr_ptr, ULONG wait_option);
UINT _nx_secure_dtls_session_reset(NX_SECURE_DTLS_SESSION *session_ptr);
UINT _nx_secure_dtls_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                  NXD_ADDRESS *ip_address, UINT port);
UINT _nx_secure_dtls_server_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr);
UINT _nx_secure_dtls_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket,
                                   UINT is_client, UINT wait_option);
UINT _nx_secure_dtls_packet_allocate(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET_POOL *pool_ptr,
                                     NX_PACKET **packet_ptr, ULONG wait_option);

UINT _nx_secure_dtls_client_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket, NXD_ADDRESS *ip_address, UINT port, UINT wait_option);
UINT _nx_secure_dtls_server_session_start(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);

UINT _nx_secure_dtls_server_delete(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nx_secure_dtls_server_local_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                      UCHAR *common_name, UINT common_name_length, UINT cert_id);


UINT _nx_secure_dtls_server_notify_set(NX_SECURE_DTLS_SERVER *server_ptr,
                                        UINT (*disconnect_notify)(NX_SECURE_DTLS_SESSION *dtls_session),
                                        UINT (*error_notify)(NX_SECURE_DTLS_SESSION *dtls_session, UINT error_code));

UINT _nx_secure_dtls_server_stop(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nx_secure_dtls_server_trusted_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);


UINT _nx_secure_dtls_server_trusted_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);

UINT _nx_secure_dtls_server_psk_add(NX_SECURE_DTLS_SERVER *server_ptr, UCHAR *pre_shared_key,
                                    UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                                    UINT hint_length);

UINT _nx_secure_dtls_server_x509_client_verify_configure(NX_SECURE_DTLS_SERVER *server_ptr, UINT certs_per_session,
                                                          UCHAR *certs_buffer, ULONG buffer_size);

UINT _nx_secure_dtls_server_x509_client_verify_disable(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nx_secure_dtls_session_client_info_get(NX_SECURE_DTLS_SESSION *dtls_session,
                                              NXD_ADDRESS *client_ip_address, UINT *client_port, UINT *local_port);

UINT _nx_secure_dtls_session_local_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                   NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nx_secure_dtls_session_local_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                       UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT _nx_secure_dtls_session_trusted_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nx_secure_dtls_session_trusted_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT _nx_secure_dtls_psk_add(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *pre_shared_key,
                             UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                             UINT hint_length);
UINT _nx_secure_dtls_client_protocol_version_override(NX_SECURE_DTLS_SESSION *dtls_session,
                                                      USHORT protocol_version);
UINT _nx_secure_dtls_server_protocol_version_override(NX_SECURE_DTLS_SERVER *dtls_server,
                                                      USHORT protocol_version);
UINT _nx_secure_dtls_ecc_initialize(NX_SECURE_DTLS_SESSION *dtls_session,
                                    const USHORT *supported_groups, USHORT supported_group_count,
                                    const NX_CRYPTO_METHOD **curves);
UINT _nx_secure_dtls_server_ecc_initialize(NX_SECURE_DTLS_SERVER *server_ptr,
                                           const USHORT *supported_groups, USHORT supported_group_count,
                                           const NX_CRYPTO_METHOD **curves);


/* Error-checking shell API. */
UINT _nxe_secure_dtls_session_create(NX_SECURE_DTLS_SESSION *session_ptr,
                                     const NX_SECURE_TLS_CRYPTO *crypto_table,
                                     VOID *metadata_buffer, ULONG metadata_size,
                                     UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                     UINT certs_number,
                                     UCHAR *remote_certificate_buffer, ULONG remote_certificate_buffer_size);


UINT _nxe_secure_dtls_session_delete(NX_SECURE_DTLS_SESSION *dtls_session);
UINT _nxe_secure_dtls_session_end(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);
UINT _nxe_secure_dtls_session_receive(NX_SECURE_DTLS_SESSION *dtls_session,
                                      NX_PACKET **packet_ptr_ptr, ULONG wait_option);
UINT _nxe_secure_dtls_session_reset(NX_SECURE_DTLS_SESSION *session_ptr);
UINT _nxe_secure_dtls_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                   NXD_ADDRESS *ip_address, UINT port);
UINT _nxe_secure_dtls_server_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr);
UINT _nxe_secure_dtls_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket,
                                    UINT is_client, UINT wait_option);

UINT _nxe_secure_dtls_client_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket, NXD_ADDRESS *ip_address, UINT port, UINT wait_option);
UINT _nxe_secure_dtls_server_session_start(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);

UINT _nxe_secure_dtls_server_create(NX_SECURE_DTLS_SERVER *server_ptr, NX_IP *ip_ptr, UINT port, ULONG timeout,
                                    VOID *session_buffer, UINT session_buffer_size,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *crypto_metadata_buffer, ULONG crypto_metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT (*connect_notify)(NX_SECURE_DTLS_SESSION *dtls_session, NXD_ADDRESS *ip_address, UINT port),
                                    UINT (*receive_notify)(NX_SECURE_DTLS_SESSION *dtls_session));

UINT _nxe_secure_dtls_server_delete(NX_SECURE_DTLS_SERVER *server_ptr);


UINT _nxe_secure_dtls_server_local_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                   NX_SECURE_X509_CERT *certificate, UINT cert_id);

UINT _nxe_secure_dtls_server_local_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                      UCHAR *common_name, UINT common_name_length, UINT cert_id);


UINT _nxe_secure_dtls_server_notify_set(NX_SECURE_DTLS_SERVER *server_ptr,
                                        UINT (*disconnect_notify)(NX_SECURE_DTLS_SESSION *dtls_session),
                                        UINT (*error_notify)(NX_SECURE_DTLS_SESSION *dtls_session, UINT error_code));

UINT _nxe_secure_dtls_server_start(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nxe_secure_dtls_server_stop(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nxe_secure_dtls_server_trusted_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);


UINT _nxe_secure_dtls_server_trusted_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);

UINT _nxe_secure_dtls_server_psk_add(NX_SECURE_DTLS_SERVER *server_ptr, UCHAR *pre_shared_key,
                                     UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                                     UINT hint_length);


UINT _nxe_secure_dtls_server_x509_client_verify_configure(NX_SECURE_DTLS_SERVER *server_ptr, UINT certs_per_session,
                                                          UCHAR *certs_buffer, ULONG buffer_size);

UINT _nxe_secure_dtls_server_x509_client_verify_disable(NX_SECURE_DTLS_SERVER *server_ptr);

UINT _nxe_secure_dtls_session_client_info_get(NX_SECURE_DTLS_SESSION *dtls_session,
                                              NXD_ADDRESS *client_ip_address, UINT *client_port, UINT *local_port);

UINT _nxe_secure_dtls_packet_allocate(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET_POOL *pool_ptr,
                                      NX_PACKET **packet_ptr, ULONG wait_option);

UINT _nxe_secure_dtls_session_local_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                   NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nxe_secure_dtls_session_local_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                       UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT _nxe_secure_dtls_session_trusted_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT _nxe_secure_dtls_session_trusted_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT _nxe_secure_dtls_psk_add(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *pre_shared_key,
                             UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                             UINT hint_length);
UINT _nxe_secure_dtls_client_protocol_version_override(NX_SECURE_DTLS_SESSION *dtls_session,
                                                       USHORT protocol_version);
UINT _nxe_secure_dtls_server_protocol_version_override(NX_SECURE_DTLS_SERVER *dtls_server,
                                                       USHORT protocol_version);
UINT _nxe_secure_dtls_ecc_initialize(NX_SECURE_DTLS_SESSION *dtls_session,
                                    const USHORT *supported_groups, USHORT supported_group_count,
                                    const NX_CRYPTO_METHOD **curves);
UINT _nxe_secure_dtls_server_ecc_initialize(NX_SECURE_DTLS_SERVER *server_ptr,
                                            const USHORT *supported_groups, USHORT supported_group_count,
                                            const NX_CRYPTO_METHOD **curves);


/* Define internal functions. */
VOID _nx_secure_dtls_receive_callback(NX_UDP_SOCKET *socket_ptr);

#ifdef NX_SECURE_ENABLE_DTLS



UINT _nx_secure_dtls_allocate_handshake_packet(NX_SECURE_DTLS_SESSION *dtls_session,
                                               NX_PACKET_POOL *packet_pool, NX_PACKET **packet_ptr,
                                               ULONG wait_option);

UINT _nx_secure_dtls_hash_record(NX_SECURE_DTLS_SESSION *dtls_session,
                                 ULONG sequence_num[NX_SECURE_TLS_SEQUENCE_NUMBER_SIZE],
                                 UCHAR *header, UINT header_length, UCHAR *data, UINT length,
                                 UCHAR *record_hash, UINT *hash_length, UCHAR *mac_secret);

UINT _nx_secure_dtls_client_handshake(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                      UINT data_length, ULONG wait_option);


UINT _nx_secure_dtls_process_handshake_header(UCHAR *packet_buffer, USHORT *message_type,
                                              UINT *header_size, UINT *message_length,
                                              UINT *message_seq, UINT *fragment_offset,
                                              UINT *fragment_length);


UINT _nx_secure_dtls_process_header(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                    ULONG record_offset, USHORT *message_type, UINT *length,
                                    UCHAR *header_data, USHORT *header_length);

UINT _nx_secure_dtls_session_sliding_window_check(NX_SECURE_DTLS_SESSION *dtls_session, ULONG *sequence_number);
UINT _nx_secure_dtls_session_sliding_window_update(NX_SECURE_DTLS_SESSION *dtls_session, ULONG *sequence_number);

UINT _nx_secure_dtls_process_record(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                    ULONG record_offset, ULONG *bytes_processed, ULONG wait_option);


UINT _nx_secure_dtls_verify_mac(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *header_data,
                                USHORT header_length, UCHAR *data, UINT *length);

UINT _nx_secure_dtls_send_handshake_record(NX_SECURE_DTLS_SESSION *dtls_session,
                                           NX_PACKET *send_packet, UCHAR handshake_type,
                                           ULONG wait_option, UINT include_in_finished);


UINT _nx_secure_dtls_send_record(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet,
                                 UCHAR record_type, ULONG wait_option);

UINT _nx_secure_dtls_server_handshake(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                      UINT data_length, ULONG wait_option);


UINT _nx_secure_dtls_send_clienthello(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet);
UINT _nx_secure_dtls_send_helloverifyrequest(NX_SECURE_DTLS_SESSION *dtls_session,
                                             NX_PACKET *send_packet);

UINT _nx_secure_dtls_process_clienthello(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                         UINT message_length);
UINT _nx_secure_dtls_process_helloverifyrequest(NX_SECURE_DTLS_SESSION *dtls_session,
                                                UCHAR *packet_buffer, UINT message_length);
UINT _nx_secure_dtls_send_serverhello(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *send_packet);

VOID _nx_secure_dtls_retransmit_queue_flush(NX_SECURE_DTLS_SESSION *dtls_session);
VOID _nx_secure_dtls_retransmit(NX_SECURE_DTLS_SESSION *dtls_session);

VOID nx_secure_dtls_session_cache_delete(NX_SECURE_DTLS_SERVER *dtls_server, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port);
UINT nx_secure_dtls_session_cache_get_new(NX_SECURE_DTLS_SERVER *dtls_server, NX_SECURE_DTLS_SESSION **dtls_session, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port);
UINT  nx_secure_dtls_session_cache_find(NX_SECURE_DTLS_SERVER *dtls_server, NX_SECURE_DTLS_SESSION **dtls_session, NXD_ADDRESS *ip_address, UINT remote_port, UINT local_port);

#endif /* NX_SECURE_ENABLE_DTLS */

/* DTLS component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef NX_SECURE_DTLS_INIT
#define DTLS_DECLARE
#else
#define DTLS_DECLARE extern
#endif

/* Define the head pointer of the created DTLS session and DTLS server list.  */
DTLS_DECLARE  NX_SECURE_DTLS_SESSION *_nx_secure_dtls_created_ptr;
DTLS_DECLARE  ULONG    _nx_secure_dtls_created_count;
DTLS_DECLARE  NX_SECURE_DTLS_SERVER *_nx_secure_dtls_server_created_ptr;
DTLS_DECLARE  ULONG    _nx_secure_dtls_server_created_count;

#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_DTLS_H_ */

