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
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_secure_dtls_api.h                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance DTLS implementation for the NetXDuo TCP/IP         */
/*    protocol.                                                           */
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


#ifndef SRC_NX_SECURE_DTLS_API_H_
#define SRC_NX_SECURE_DTLS_API_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "nx_api.h"
#include "nx_secure_tls_api.h"
#include "nx_secure_dtls.h"

#ifndef NX_SECURE_SOURCE_CODE
/* Services without error checking. */
#ifdef NX_SECURE_DISABLE_ERROR_CHECKING
#define nx_secure_dtls_initialize                             _nx_secure_dtls_initialize
#define nx_secure_dtls_session_create                         _nx_secure_dtls_session_create
#define nx_secure_dtls_session_delete                         _nx_secure_dtls_session_delete
#define nx_secure_dtls_session_end                            _nx_secure_dtls_session_end
#define nx_secure_dtls_session_receive                        _nx_secure_dtls_session_receive
#define nx_secure_dtls_session_reset                          _nx_secure_dtls_session_reset
#define nx_secure_dtls_session_send                           _nx_secure_dtls_session_send
#define nx_secure_dtls_server_session_send                    _nx_secure_dtls_server_session_send
#define nx_secure_dtls_client_session_send                    _nx_secure_dtls_server_session_send
#define nx_secure_dtls_session_start                          _nx_secure_dtls_session_start
#define nx_secure_dtls_packet_allocate                        _nx_secure_dtls_packet_allocate
#define nx_secure_dtls_client_session_start                   _nx_secure_dtls_client_session_start
#define nx_secure_dtls_server_session_start                   _nx_secure_dtls_server_session_start
#define nx_secure_dtls_server_create                          _nx_secure_dtls_server_create
#define nx_secure_dtls_server_delete                          _nx_secure_dtls_server_delete
#define nx_secure_dtls_server_local_certificate_add           _nx_secure_dtls_server_local_certificate_add
#define nx_secure_dtls_server_local_certificate_remove        _nx_secure_dtls_server_local_certificate_remove
#define nx_secure_dtls_server_notify_set                      _nx_secure_dtls_server_notify_set
#define nx_secure_dtls_server_start                           _nx_secure_dtls_server_start
#define nx_secure_dtls_server_stop                            _nx_secure_dtls_server_stop
#define nx_secure_dtls_server_trusted_certificate_add         _nx_secure_dtls_server_trusted_certificate_add
#define nx_secure_dtls_server_trusted_certificate_remove      _nx_secure_dtls_server_trusted_certificate_remove
#define nx_secure_dtls_server_x509_client_verify_configure    _nx_secure_dtls_server_x509_client_verify_configure
#define nx_secure_dtls_server_x509_client_verify_disable      _nx_secure_dtls_server_x509_client_verify_disable
#define nx_secure_dtls_session_client_info_get                _nx_secure_dtls_session_client_info_get
#define nx_secure_dtls_psk_add                                _nx_secure_dtls_psk_add
#define nx_secure_dtls_session_local_certificate_add          _nx_secure_dtls_session_local_certificate_add
#define nx_secure_dtls_session_local_certificate_remove       _nx_secure_dtls_session_local_certificate_remove
#define nx_secure_dtls_session_trusted_certificate_add        _nx_secure_dtls_session_trusted_certificate_add
#define nx_secure_dtls_session_trusted_certificate_remove     _nx_secure_dtls_session_trusted_certificate_remove
#define nx_secure_dtls_server_psk_add                         _nx_secure_dtls_server_psk_add
#define nx_secure_dtls_client_protocol_version_override       _nx_secure_dtls_client_protocol_version_override
#define nx_secure_dtls_server_protocol_version_override       _nx_secure_dtls_server_protocol_version_override
#define nx_secure_dtls_ecc_initialize                         _nx_secure_dtls_ecc_initialize
#define nx_secure_dtls_server_ecc_initialize                  _nx_secure_dtls_server_ecc_initialize
#else
/* Services with error checking. */
#define nx_secure_dtls_initialize                             _nx_secure_dtls_initialize
#define nx_secure_dtls_session_create                         _nxe_secure_dtls_session_create
#define nx_secure_dtls_session_delete                         _nxe_secure_dtls_session_delete
#define nx_secure_dtls_session_end                            _nxe_secure_dtls_session_end
#define nx_secure_dtls_session_receive                        _nxe_secure_dtls_session_receive
#define nx_secure_dtls_session_reset                          _nxe_secure_dtls_session_reset
#define nx_secure_dtls_session_send                           _nxe_secure_dtls_session_send
#define nx_secure_dtls_server_session_send                    _nxe_secure_dtls_server_session_send
#define nx_secure_dtls_client_session_send                    _nxe_secure_dtls_server_session_send
#define nx_secure_dtls_session_start                          _nxe_secure_dtls_session_start
#define nx_secure_dtls_packet_allocate                        _nxe_secure_dtls_packet_allocate
#define nx_secure_dtls_client_session_start                   _nxe_secure_dtls_client_session_start
#define nx_secure_dtls_server_session_start                   _nxe_secure_dtls_server_session_start
#define nx_secure_dtls_server_create                          _nxe_secure_dtls_server_create
#define nx_secure_dtls_server_delete                          _nxe_secure_dtls_server_delete
#define nx_secure_dtls_server_local_certificate_add           _nxe_secure_dtls_server_local_certificate_add
#define nx_secure_dtls_server_local_certificate_remove        _nxe_secure_dtls_server_local_certificate_remove
#define nx_secure_dtls_server_notify_set                      _nxe_secure_dtls_server_notify_set
#define nx_secure_dtls_server_start                           _nxe_secure_dtls_server_start
#define nx_secure_dtls_server_stop                            _nxe_secure_dtls_server_stop
#define nx_secure_dtls_server_trusted_certificate_add         _nxe_secure_dtls_server_trusted_certificate_add
#define nx_secure_dtls_server_trusted_certificate_remove      _nxe_secure_dtls_server_trusted_certificate_remove
#define nx_secure_dtls_server_x509_client_verify_configure    _nxe_secure_dtls_server_x509_client_verify_configure
#define nx_secure_dtls_server_x509_client_verify_disable      _nxe_secure_dtls_server_x509_client_verify_disable
#define nx_secure_dtls_session_client_info_get                _nxe_secure_dtls_session_client_info_get
#define nx_secure_dtls_psk_add                                _nxe_secure_dtls_psk_add
#define nx_secure_dtls_session_local_certificate_add          _nxe_secure_dtls_session_local_certificate_add
#define nx_secure_dtls_session_local_certificate_remove       _nxe_secure_dtls_session_local_certificate_remove
#define nx_secure_dtls_session_trusted_certificate_add        _nxe_secure_dtls_session_trusted_certificate_add
#define nx_secure_dtls_session_trusted_certificate_remove     _nxe_secure_dtls_session_trusted_certificate_remove
#define nx_secure_dtls_server_psk_add                         _nxe_secure_dtls_server_psk_add
#define nx_secure_dtls_client_protocol_version_override       _nxe_secure_dtls_client_protocol_version_override
#define nx_secure_dtls_server_protocol_version_override       _nxe_secure_dtls_server_protocol_version_override
#define nx_secure_dtls_ecc_initialize                         _nxe_secure_dtls_ecc_initialize
#define nx_secure_dtls_server_ecc_initialize                  _nxe_secure_dtls_server_ecc_initialize
#endif /* NX_SECURE_DISABLE_ERROR_CHECKING */

/* Define the function prototypes of the DTLS API. */
VOID nx_secure_dtls_initialize(VOID);

UINT nx_secure_dtls_session_create(NX_SECURE_DTLS_SESSION *session_ptr,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *metadata_buffer, ULONG metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT certs_number,
                                    UCHAR *remote_certificate_buffer, ULONG remote_certificate_buffer_size);

UINT nx_secure_dtls_session_delete(NX_SECURE_DTLS_SESSION *dtls_session);
UINT nx_secure_dtls_session_end(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);
UINT nx_secure_dtls_session_receive(NX_SECURE_DTLS_SESSION *dtls_session,
                                    NX_PACKET **packet_ptr_ptr, ULONG wait_option);
UINT nx_secure_dtls_session_reset(NX_SECURE_DTLS_SESSION *session_ptr);
UINT nx_secure_dtls_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr,
                                 NXD_ADDRESS *ip_address, UINT port);
UINT nx_secure_dtls_server_session_send(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET *packet_ptr);
UINT nx_secure_dtls_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket,
                                  UINT is_client, UINT wait_option);
UINT nx_secure_dtls_packet_allocate(NX_SECURE_DTLS_SESSION *dtls_session, NX_PACKET_POOL *pool_ptr,
                                    NX_PACKET **packet_ptr, ULONG wait_option);

UINT nx_secure_dtls_client_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket, NXD_ADDRESS *ip_address, UINT port, UINT wait_option);
UINT nx_secure_dtls_server_session_start(NX_SECURE_DTLS_SESSION *dtls_session, UINT wait_option);

UINT nx_secure_dtls_server_create(NX_SECURE_DTLS_SERVER *server_ptr, NX_IP *ip_ptr, UINT port, ULONG timeout,
                                    VOID *session_buffer, UINT session_buffer_size,
                                    const NX_SECURE_TLS_CRYPTO *crypto_table,
                                    VOID *crypto_metadata_buffer, ULONG crypto_metadata_size,
                                    UCHAR *packet_reassembly_buffer, UINT packet_reassembly_buffer_size,
                                    UINT (*connect_notify)(NX_SECURE_DTLS_SESSION *dtls_session, NXD_ADDRESS *ip_address, UINT port),
                                    UINT (*receive_notify)(NX_SECURE_DTLS_SESSION *dtls_session));

UINT nx_secure_dtls_server_delete(NX_SECURE_DTLS_SERVER *server_ptr);


UINT nx_secure_dtls_server_local_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                   NX_SECURE_X509_CERT *certificate, UINT cert_id);

UINT nx_secure_dtls_server_local_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                      UCHAR *common_name, UINT common_name_length, UINT cert_id);


UINT nx_secure_dtls_server_notify_set(NX_SECURE_DTLS_SERVER *server_ptr,
                                        UINT (*disconnect_notify)(NX_SECURE_DTLS_SESSION *dtls_session),
                                        UINT (*error_notify)(NX_SECURE_DTLS_SESSION *dtls_session, UINT error_code));

UINT nx_secure_dtls_server_start(NX_SECURE_DTLS_SERVER *server_ptr);

UINT nx_secure_dtls_server_stop(NX_SECURE_DTLS_SERVER *server_ptr);

UINT nx_secure_dtls_server_trusted_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);


UINT nx_secure_dtls_server_trusted_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);

UINT nx_secure_dtls_server_psk_add(NX_SECURE_DTLS_SERVER *server_ptr, UCHAR *pre_shared_key,
                                    UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                                    UINT hint_length);

UINT nx_secure_dtls_server_x509_client_verify_configure(NX_SECURE_DTLS_SERVER *server_ptr, UINT certs_per_session,
                                                          UCHAR *certs_buffer, ULONG buffer_size);

UINT nx_secure_dtls_server_x509_client_verify_disable(NX_SECURE_DTLS_SERVER *server_ptr);

UINT nx_secure_dtls_session_client_info_get(NX_SECURE_DTLS_SESSION *dtls_session,
                                              NXD_ADDRESS *client_ip_address, UINT *client_port, UINT *local_port);


UINT nx_secure_dtls_session_local_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                   NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT nx_secure_dtls_session_local_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                       UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT nx_secure_dtls_session_trusted_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                     NX_SECURE_X509_CERT *certificate, UINT cert_id);
UINT nx_secure_dtls_session_trusted_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                        UCHAR *common_name, UINT common_name_length, UINT cert_id);
UINT nx_secure_dtls_psk_add(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *pre_shared_key,
                             UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                             UINT hint_length);
UINT nx_secure_dtls_client_protocol_version_override(NX_SECURE_DTLS_SESSION *dtls_session,
                                                     USHORT protocol_version);
UINT nx_secure_dtls_server_protocol_version_override(NX_SECURE_DTLS_SERVER *dtls_server,
                                                     USHORT protocol_version);


#endif /* NX_SECURE_SOURCE_CODE */


#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_DTLS_API_H_ */

