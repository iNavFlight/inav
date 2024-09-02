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
/** NetX Component                                                        */
/**                                                                       */
/**   Multiple TCP Socket/TLS Session support module                      */
/**                                                                       */
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_tcpserver.h                                      PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX TCP Server module component,             */
/*    including all data types and external references.                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed packet leak issue,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            deprecated unused macros,   */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported ECC configuration,*/
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_TCPSERVER_H
#define NX_TCPSERVER_H

#include "tx_api.h"
#include "nx_api.h"
#ifdef NX_WEB_HTTPS_ENABLE

/* Enable TLS for the TCPServer module to support HTTPS */
#ifndef NX_TCPSERVER_ENABLE_TLS
#define NX_TCPSERVER_ENABLE_TLS
#endif

#include "nx_secure_tls_api.h"
#endif /* NX_WEB_HTTPS_ENABLE */

/* Deprecated. This symbol is defined for compatibility. */
#ifndef NX_TCPSERVER_ACCEPT_WAIT    
#define NX_TCPSERVER_ACCEPT_WAIT 1
#endif /* NX_TCPSERVER_ACCEPT_WAIT */

/* Deprecated. This symbol is defined for compatibility. */
#ifndef NX_TCPSERVER_DISCONNECT_WAIT    
#define NX_TCPSERVER_DISCONNECT_WAIT 1
#endif /* NX_TCPSERVER_DISCONNECT_WAIT */

/* Deprecated. This symbol is defined for compatibility. */
#ifndef NX_TCPSERVER_PRIORITY    
#define NX_TCPSERVER_PRIORITY 4
#endif /* NX_TCPSERVER_PRIORITY */

#ifndef NX_TCPSERVER_TIMEOUT_PERIOD    
#define NX_TCPSERVER_TIMEOUT_PERIOD 1
#endif /* NX_TCPSERVER_TIMEOUT_PERIOD */

/* Define thread events. */
#define NX_TCPSERVER_CONNECT            0x00000001
#define NX_TCPSERVER_DATA               0x00000002
#define NX_TCPSERVER_DISCONNECT         0x00000004
#define NX_TCPSERVER_TIMEOUT            0x00000008
#define NX_TCPSERVER_ANY_EVENT          0xFFFFFFFF

/* ERROR code */
#define NX_TCPSERVER_FAIL 0x01

/* TCP Server session structure - contains individual
   TCP sockets and TLS sessions. */
typedef struct NX_TCP_SESSION_STRUCT
{
    /* TCP socket used for this session. */
    NX_TCP_SOCKET           nx_tcp_session_socket;

    /* Expiration timeout for this socket. */
    ULONG                   nx_tcp_session_expiration;

    /* Connection flag. */
    UINT                    nx_tcp_session_connected;

    /* Reserved value for passing data to/from individual sessions. */
    ULONG                   nx_tcp_session_reserved;

#ifdef NX_TCPSERVER_ENABLE_TLS
    /* Flag set to NX_TRUE if using TLS. */
    UINT                    nx_tcp_session_using_tls;

    /* If TLS is enabled, we also have a TLS session to maintain. */
    NX_SECURE_TLS_SESSION   nx_tcp_session_tls_session;
#endif

} NX_TCP_SESSION;

/* TCPSERVER structure */
typedef struct NX_TCPSERVER_STRUCT
{
    NX_IP                  *nx_tcpserver_ip;
    NX_TCP_SESSION         *nx_tcpserver_sessions;
    UINT                    nx_tcpserver_sessions_count;
    UINT                    nx_tcpserver_listen_port;
    NX_TCP_SESSION         *nx_tcpserver_listen_session;
    TX_THREAD               nx_tcpserver_thread;
    TX_TIMER                nx_tcpserver_timer;
    TX_EVENT_FLAGS_GROUP    nx_tcpserver_event_flags;
    ULONG                   nx_tcpserver_timeout;
    ULONG                   nx_tcpserver_accept_wait_option;
    VOID                  (*nx_tcpserver_new_connection)(struct NX_TCPSERVER_STRUCT *server_ptr, NX_TCP_SESSION *session_ptr);
    VOID                  (*nx_tcpserver_receive_data)(struct NX_TCPSERVER_STRUCT *server_ptr, NX_TCP_SESSION *session_ptr);
    VOID                  (*nx_tcpserver_connection_end)(struct NX_TCPSERVER_STRUCT *server_ptr, NX_TCP_SESSION *session_ptr);
    VOID                  (*nx_tcpserver_connection_timeout)(struct NX_TCPSERVER_STRUCT *server_ptr, NX_TCP_SESSION *session_ptr);
    ULONG                   nx_tcpserver_reserved;
} NX_TCPSERVER;


#ifndef NX_TCPSERVER_SOURCE_CODE

/* APIs */
#define nx_tcpserver_create         _nx_tcpserver_create
#define nx_tcpserver_start          _nx_tcpserver_start
#define nx_tcpserver_stop           _nx_tcpserver_stop
#define nx_tcpserver_delete         _nx_tcpserver_delete
#ifdef NX_TCPSERVER_ENABLE_TLS
#define nx_tcpserver_tls_setup      _nx_tcpserver_tls_setup
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#define nx_tcpserver_tls_ecc_setup  _nx_tcpserver_tls_ecc_setup
#endif
#endif

#ifdef NX_TCPSERVER_ENABLE_TLS
UINT nx_tcpserver_tls_setup(NX_TCPSERVER *server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                            VOID *metadata_buffer, ULONG metadata_size, UCHAR* packet_buffer, UINT packet_buffer_size, NX_SECURE_X509_CERT *identity_certificate,
                            NX_SECURE_X509_CERT *trusted_certificates[], UINT trusted_certs_num, NX_SECURE_X509_CERT *remote_certificates[], UINT remote_certs_num,
                            UCHAR *remote_certificate_buffer, UINT remote_cert_buffer_size);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT nx_tcpserver_tls_ecc_setup(NX_TCPSERVER *server_ptr,
                                const USHORT *supported_groups, USHORT supported_group_count,
                                const NX_CRYPTO_METHOD **curves);
#endif
#endif

UINT nx_tcpserver_create(NX_IP *ip_ptr, NX_TCPSERVER *server_ptr, CHAR *name, 
                         ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                         VOID (*new_connection)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                         VOID (*receive_data)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                         VOID (*connection_end)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                         VOID (*connection_timeout)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                         ULONG timeout, VOID *stack_ptr, UINT stack_size,
                         VOID *sessions_buffer, UINT buffer_size, UINT thread_priority, ULONG accept_wait_option);


UINT nx_tcpserver_start(NX_TCPSERVER *server_ptr, UINT port, UINT listen_queue_size);

UINT nx_tcpserver_stop(NX_TCPSERVER *server_ptr);

UINT nx_tcpserver_delete(NX_TCPSERVER *server_ptr);

#else

#ifdef NX_TCPSERVER_ENABLE_TLS
UINT _nx_tcpserver_tls_setup(NX_TCPSERVER *server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                             VOID *metadata_buffer, ULONG metadata_size, UCHAR* packet_buffer, UINT packet_buffer_size, NX_SECURE_X509_CERT *identity_certificate,
                             NX_SECURE_X509_CERT *trusted_certificates[], UINT trusted_certs_num, NX_SECURE_X509_CERT *remote_certificates[], UINT remote_certs_num,
                             UCHAR *remote_certificate_buffer, UINT remote_cert_buffer_size);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT _nx_tcpserver_tls_ecc_setup(NX_TCPSERVER *server_ptr,
                                 const USHORT *supported_groups, USHORT supported_group_count,
                                 const NX_CRYPTO_METHOD **curves);
#endif
#endif

UINT _nx_tcpserver_create(NX_IP *ip_ptr, NX_TCPSERVER *server_ptr, CHAR *name, 
                          ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                          VOID (*new_connection)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*receive_data)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*connection_end)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          VOID (*connection_timeout)(NX_TCPSERVER *server_ptr, NX_TCP_SESSION *session_ptr),
                          ULONG timeout, VOID *stack_ptr, UINT stack_size,
                          VOID *sessions_buffer, UINT buffer_size, UINT thread_priority, ULONG accept_wait_option);


UINT _nx_tcpserver_start(NX_TCPSERVER *server_ptr, UINT port, UINT listen_queue_size);

UINT _nx_tcpserver_stop(NX_TCPSERVER *server_ptr);

UINT _nx_tcpserver_delete(NX_TCPSERVER *server_ptr);

#endif

#endif /* NX_TCPSERVER_H */
