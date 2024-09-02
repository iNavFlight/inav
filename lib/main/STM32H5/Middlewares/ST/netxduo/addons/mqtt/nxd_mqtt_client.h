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
/**   MQTT (MQTT)                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_mqtt_client.h                                   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX MQTT Client component, including all     */
/*    data types and external references.  It is assumed that nx_api.h    */
/*    and nx_port.h have already been included.                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added ack receive notify,   */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            added packet id parameter,  */
/*                                            resulting in version 6.1.2  */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported maximum transmit  */
/*                                            queue depth,                */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*  10-31-2022     Bo Chen                  Modified comment(s), supported*/
/*                                            mqtt over websocket,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef _NXD_MQTT_CLIENT_H_
#define _NXD_MQTT_CLIENT_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

#ifdef NX_SECURE_ENABLE
#include "nx_secure_tls_api.h"
#endif /* NX_SECURE_ENABLE */

#ifdef NXD_MQTT_CLOUD_ENABLE
#include "nx_cloud.h"
#endif /* NXD_MQTT_CLOUD_ENABLE */

#ifdef NXD_MQTT_REQUIRE_TLS
#ifndef NX_SECURE_ENABLE
#error "The feature NXD_MQTT_REQUIRE_TLS requires NX_SECURE_ENABLE."
#endif /* NX_SECURE_ENABLE */
#endif /* NXD_MQTT_REQUIRE_TLS */

#ifdef NXD_MQTT_OVER_WEBSOCKET
#include "nx_websocket_client.h"
#endif /* NXD_MQTT_OVER_WEBSOCKET */

/* Defined, MQTT transmit queue depth is enabled. It must be positive integer.  */
/*
#define NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH                          20
*/

/* Define MQTT protocol for websocket.  */
#define NXD_MQTT_OVER_WEBSOCKET_PROTOCOL                               "mqtt"

/* Define memcpy, memset and memcmp functions used internal. */
#ifndef NXD_MQTT_SECURE_MEMCPY
#define NXD_MQTT_SECURE_MEMCPY                                         memcpy
#endif /* NXD_MQTT_SECURE_MEMCPY */

#ifndef NXD_MQTT_SECURE_MEMCMP
#define NXD_MQTT_SECURE_MEMCMP                                         memcmp
#endif /* NXD_MQTT_SECURE_MEMCMP */

#ifndef NXD_MQTT_SECURE_MEMSET
#define NXD_MQTT_SECURE_MEMSET                                         memset
#endif /* NXD_SECURE_MEMSET */

#ifndef NXD_MQTT_SECURE_MEMMOVE
#define NXD_MQTT_SECURE_MEMMOVE                                        memmove
#endif /* NXD_MQTT_SECURE_MEMMOVE */

/* Define the default MQTT Non-TLS (Non-secure) port number */
#define NXD_MQTT_PORT                                                  1883


/* Define the default TCP socket window size. */
#ifndef NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE
#define NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE                             8192
#endif /* NXD_MQTT_CLIENT_SOCKET_WINDOW_SIZE */

/* Define the default MQTT Thread time slice. */
#ifndef NXD_MQTT_CLIENT_THREAD_TIME_SLICE
#define NXD_MQTT_CLIENT_THREAD_TIME_SLICE                              2
#endif

/* Set the default timer rate for the keepalive timer, in ThreadX timer ticks.
   THe default is one second. */
#ifndef NXD_MQTT_KEEPALIVE_TIMER_RATE
#define NXD_MQTT_KEEPALIVE_TIMER_RATE                                  (NX_IP_PERIODIC_RATE)
#endif

/* Set the default timeout for PING response. */
/* After sending out the MQTT Ping Request, if the client does not receive Ping Response within this
   time, the client shall disconnect from the server. The default is one second. */
#ifndef NXD_MQTT_PING_TIMEOUT_DELAY
#define NXD_MQTT_PING_TIMEOUT_DELAY                                    (NX_IP_PERIODIC_RATE)
#endif


/* Deprecated. This symbol is defined for compatibility. */
#ifndef NXD_MQTT_MAX_TOPIC_NAME_LENGTH
#define NXD_MQTT_MAX_TOPIC_NAME_LENGTH                                 12
#endif

/* Deprecated. This symbol is defined for compatibility. */
#ifndef NXD_MQTT_MAX_MESSAGE_LENGTH
#define NXD_MQTT_MAX_MESSAGE_LENGTH                                    32
#endif

#ifndef NXD_MQTT_INITIAL_PACKET_ID_VALUE
#define NXD_MQTT_INITIAL_PACKET_ID_VALUE                               1
#endif

/* Set a timeout for socket operations (send, receive, disconnect). */
#ifndef NXD_MQTT_SOCKET_TIMEOUT
#define NXD_MQTT_SOCKET_TIMEOUT                                         NX_WAIT_FOREVER
#endif

/* Define the default MQTT TLS (secure) port number */
#define NXD_MQTT_TLS_PORT                                              8883

/* Define the default MQTT TLS (secure) over WebSocket port number */
#define NXD_MQTT_OVER_WEBSOCKET_TLS_PORT                               443


#define MQTT_PROTOCOL_LEVEL                                            4

/* Define bit fields and constant values used in the CONNECT packet. */
#define MQTT_CONNECT_FLAGS_USERNAME                                    (1 << 7)
#define MQTT_CONNECT_FLAGS_PASSWORD                                    (1 << 6)
#define MQTT_CONNECT_FLAGS_WILL_RETAIN                                 (1 << 5)
#define MQTT_CONNECT_FLAGS_WILL_QOS_0                                  0
#define MQTT_CONNECT_FLAGS_WILL_QOS_1                                  (1 << 3)
#define MQTT_CONNECT_FLAGS_WILL_QOS_2                                  (2 << 3)
#define MQTT_CONNECT_FLAGS_WILL_FLAG                                   (1 << 2)
#define MQTT_CONNECT_FLAGS_CLEAN_SESSION                               (1 << 1)
#define MQTT_CONNECT_FLAGS_WILL_QOS_FIELD                              (3 << 3)

/* Define bit fields and constant values used in the CONNACK packet. */
#define MQTT_CONNACK_CONNECT_FLAGS_SP                                  (1)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_ACCEPTED                      (0)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_UNACCEPTABLE_PROTOCOL_VERSION (1)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_IDENTIFIER_REJECTED           (2)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_SERVER_UNAVAILABLE            (3)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_BAD_USERNAME_PASSWORD         (4)
#define MQTT_CONNACK_CONNECT_RETURN_CODE_NOT_AUTHORIZED                (5)

/* Define bit fields and constant values used in the PUBLISH packet. */
#define MQTT_PUBLISH_DUP_FLAG                                          (1 << 3)
#define MQTT_PUBLISH_QOS_LEVEL_0                                       (0)
#define MQTT_PUBLISH_QOS_LEVEL_1                                       (1 << 1)
#define MQTT_PUBLISH_QOS_LEVEL_2                                       (2 << 1)
#define MQTT_PUBLISH_QOS_LEVEL_FIELD                                   (3 << 1)
#define MQTT_PUBLISH_RETAIN                                            (1)

#define MQTT_CONTROL_PACKET_TYPE_FIELD                                 0xF0

#define MQTT_FIXED_HEADER_SIZE                                         2


#define MQTT_CONTROL_PACKET_TYPE_CONNECT                               (1)
#define MQTT_CONTROL_PACKET_TYPE_CONNACK                               (2)
#define MQTT_CONTROL_PACKET_TYPE_PUBLISH                               (3)
#define MQTT_CONTROL_PACKET_TYPE_PUBACK                                (4)
#define MQTT_CONTROL_PACKET_TYPE_PUBREC                                (5)
#define MQTT_CONTROL_PACKET_TYPE_PUBREL                                (6)
#define MQTT_CONTROL_PACKET_TYPE_PUBCOMP                               (7)
#define MQTT_CONTROL_PACKET_TYPE_SUBSCRIBE                             (8)
#define MQTT_CONTROL_PACKET_TYPE_SUBACK                                (9)
#define MQTT_CONTROL_PACKET_TYPE_UNSUBSCRIBE                           (10)
#define MQTT_CONTROL_PACKET_TYPE_UNSUBACK                              (11)
#define MQTT_CONTROL_PACKET_TYPE_PINGREQ                               (12)
#define MQTT_CONTROL_PACKET_TYPE_PINGRESP                              (13)
#define MQTT_CONTROL_PACKET_TYPE_DISCONNECT                            (14)
/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */


typedef struct MQTT_PACKET_CONNACK_STRUCT
{

    UCHAR mqtt_connack_packet_header;
    UCHAR mqtt_connack_packet_remaining_length;
    UCHAR mqtt_connack_packet_ack_flags;
    UCHAR mqtt_connack_packet_return_code;
} MQTT_PACKET_CONNACK;

typedef struct MQTT_PACKET_PUBLISH_STRUCT
{
    UCHAR mqtt_publish_packet_header;
    UCHAR mqtt_publish_packet_remaining_length;
} MQTT_PACKET_PUBLISH;

typedef struct MQTT_PACKET_PUBLISH_RESPONSE_STRUCT
{
    UCHAR mqtt_publish_response_packet_header;
    UCHAR mqtt_publish_response_packet_remaining_length;
    UCHAR mqtt_publish_response_packet_packet_identifier_msb;
    UCHAR mqtt_publish_response_packet_packet_identifier_lsb;
} MQTT_PACKET_PUBLISH_RESPONSE;


typedef struct MQTT_PACKET_SUBSCRIBE_STRUCT
{
    UCHAR mqtt_subscribe_packet_header;
    UCHAR mqtt_subscribe_packet_remaining_length;
    UCHAR mqtt_subscribe_packet_packet_identifier_msb;
    UCHAR mqtt_subscribe_packet_packet_identifier_lsb;
} MQTT_PACKET_SUBSCRIBE;

typedef struct MQTT_PACKET_PING_STRUCT
{
    UCHAR mqtt_ping_packet_header;
    UCHAR mqtt_ping_packet_remaining_length;
} MQTT_PACKET_PING;

typedef struct MQTT_PACKET_DISCONNECT_STRUCT
{
    UCHAR mqtt_disconnect_packet_header;
    UCHAR mqtt_disconnect_packet_remaining_length;
} MQTT_PACKET_DISCONNECT;


/* Define the NetX MQTT CLIENT ID.  */
#define NXD_MQTT_CLIENT_ID                   0x4D515454

/* Define the default MQTT Non-TLS (Non-secure) port number */
#define NXD_MQTT_CLIENT_NONTLS_PORT          1883

/* Define the default MQTT TLS (secure) port number */
#define NXD_MQTT_CLIENT_TLS_PORT             8883


#define NXD_MQTT_CLIENT_STATE_INITIALIZE     0
#define NXD_MQTT_CLIENT_STATE_IDLE           1
#define NXD_MQTT_CLIENT_STATE_CONNECTING     2
#define NXD_MQTT_CLIENT_STATE_CONNECTED      3


#define NXD_MQTT_SUCCESS                     0x0
#define NXD_MQTT_ALREADY_CONNECTED           0x10001
#define NXD_MQTT_NOT_CONNECTED               0x10002
#define NXD_MQTT_MUTEX_FAILURE               0x10003
#define NXD_MQTT_INTERNAL_ERROR              0x10004
#define NXD_MQTT_CONNECT_FAILURE             0x10005
#define NXD_MQTT_PACKET_POOL_FAILURE         0x10006
#define NXD_MQTT_COMMUNICATION_FAILURE       0x10007
#define NXD_MQTT_SERVER_MESSAGE_FAILURE      0x10008
#define NXD_MQTT_INVALID_PARAMETER           0x10009
#define NXD_MQTT_NO_MESSAGE                  0x1000A
#define NXD_MQTT_PACKET_POOL_EMPTY           0x1000B
#define NXD_MQTT_QOS2_NOT_SUPPORTED          0x1000C
#define NXD_MQTT_INSUFFICIENT_BUFFER_SPACE   0x1000D
#define NXD_MQTT_CLIENT_NOT_RUNNING          0x1000E
#define NXD_MQTT_INVALID_PACKET              0x1000F
#define NXD_MQTT_PARTIAL_PACKET              0x10010
#define NXD_MQTT_CONNECTING                  0x10011
#define NXD_MQTT_INVALID_STATE               0x10012

/* The following error codes match the Connect Return code in CONNACK message. */
#define NXD_MQTT_ERROR_CONNECT_RETURN_CODE   0x10080
#define NXD_MQTT_ERROR_UNACCEPTABLE_PROTOCOL 0x10081
#define NXD_MQTT_ERROR_IDENTIFYIER_REJECTED  0x10082
#define NXD_MQTT_ERROR_SERVER_UNAVAILABLE    0x10083
#define NXD_MQTT_ERROR_BAD_USERNAME_PASSWORD 0x10084
#define NXD_MQTT_ERROR_NOT_AUTHORIZED        0x10085


/* Define the basic MQTT Client control block. */
typedef struct NXD_MQTT_CLIENT_STRUCT
{
    CHAR                          *nxd_mqtt_client_name;
    CHAR                          *nxd_mqtt_client_id;
    UINT                           nxd_mqtt_client_id_length;
    CHAR                          *nxd_mqtt_client_username;
    USHORT                         nxd_mqtt_client_username_length;
    CHAR                          *nxd_mqtt_client_password;
    USHORT                         nxd_mqtt_client_password_length;
    UCHAR                          nxd_mqtt_client_will_qos_retain;
    const UCHAR                   *nxd_mqtt_client_will_topic;
    UINT                           nxd_mqtt_client_will_topic_length;
    const UCHAR                   *nxd_mqtt_client_will_message;
    UINT                           nxd_mqtt_client_will_message_length;
    NX_IP                         *nxd_mqtt_client_ip_ptr;                          /* Pointer to associated IP structure   */
    NX_PACKET_POOL                *nxd_mqtt_client_packet_pool_ptr;                 /* Pointer to client packet pool        */
    TX_MUTEX                      *nxd_mqtt_client_mutex_ptr;                       /* Pointer to client mutex              */
    TX_TIMER                       nxd_mqtt_timer;
#ifndef NXD_MQTT_CLOUD_ENABLE
    TX_MUTEX                       nxd_mqtt_protection;
    TX_THREAD                      nxd_mqtt_thread;
    TX_EVENT_FLAGS_GROUP           nxd_mqtt_events;
#else
    NX_CLOUD                      *nxd_mqtt_client_cloud_ptr;                      /* Pointer to associated CLOUD structure.                    */
    NX_CLOUD                       nxd_mqtt_client_cloud;                          /* MQTT cloud.                                               */
    NX_CLOUD_MODULE                nxd_mqtt_client_cloud_module;                   /* Define mqtt module that running on cloud helper thread.   */
#endif /* NXD_MQTT_CLOUD_ENABLE */
    UINT                           nxd_mqtt_ping_timeout;
    UINT                           nxd_mqtt_ping_not_responded;                     /* Flag indicating the ping has been responded or not. */
    UINT                           nxd_mqtt_ping_sent_time;                         /* TX Timer tick when the ping message was sent. */
    UINT                           nxd_mqtt_timeout;                                /* TX Timer tick when the next timeout happens. */
    UINT                           nxd_mqtt_timer_value;                            /* MQTT Client periodic timer tick value.  */
    UINT                           nxd_mqtt_keepalive;                              /* Keepalive value, converted to TX ticks. */
    UINT                           nxd_mqtt_clean_session;                          /* Clean session flag. */
    UINT                           nxd_mqtt_client_state;                           /* Record client state                  */
    NX_TCP_SOCKET                  nxd_mqtt_client_socket;
    struct NXD_MQTT_CLIENT_STRUCT *nxd_mqtt_client_next;
    UINT                           nxd_mqtt_client_packet_identifier;
    NX_PACKET                     *nxd_mqtt_client_processing_packet;
    NX_PACKET                     *message_transmit_queue_head;
    NX_PACKET                     *message_transmit_queue_tail;
#ifdef NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH
    UINT                           message_transmit_queue_depth;
#endif /* NXD_MQTT_MAXIMUM_TRANSMIT_QUEUE_DEPTH */
    NX_PACKET                     *message_receive_queue_head;
    NX_PACKET                     *message_receive_queue_tail;
    UINT                           message_receive_queue_depth;
    VOID                         (*nxd_mqtt_client_receive_notify)(struct NXD_MQTT_CLIENT_STRUCT *client_ptr, UINT number_of_messages);
    VOID                         (*nxd_mqtt_connect_notify)(struct NXD_MQTT_CLIENT_STRUCT *client_ptr, UINT status, VOID *context);
    VOID                          *nxd_mqtt_connect_context;
    VOID                         (*nxd_mqtt_disconnect_notify)(struct NXD_MQTT_CLIENT_STRUCT *client_ptr);
    UINT                         (*nxd_mqtt_packet_receive_notify)(struct NXD_MQTT_CLIENT_STRUCT *client_ptr, NX_PACKET *packet_ptr, VOID *context);
    VOID                          *nxd_mqtt_packet_receive_context;
    VOID                         (*nxd_mqtt_ack_receive_notify)(struct NXD_MQTT_CLIENT_STRUCT *client_ptr, UINT type, USHORT packet_id, NX_PACKET *transmit_packet_ptr, VOID *context);
    VOID                          *nxd_mqtt_ack_receive_context;
#ifdef NX_SECURE_ENABLE
    UINT                           nxd_mqtt_client_use_tls;
    UINT                         (*nxd_mqtt_tls_setup)(struct NXD_MQTT_CLIENT_STRUCT *, NX_SECURE_TLS_SESSION *,
                                                       NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *);
    NX_SECURE_X509_CERT            nxd_mqtt_tls_certificate;
    NX_SECURE_X509_CERT            nxd_mqtt_tls_trusted_certificate;
    NX_SECURE_TLS_SESSION          nxd_mqtt_tls_session;
    UINT                           nxd_mqtt_tls_in_progress;
#endif
#ifdef NXD_MQTT_OVER_WEBSOCKET
    UINT                           nxd_mqtt_client_use_websocket;
    NX_WEBSOCKET_CLIENT            nxd_mqtt_client_websocket;
    UCHAR                         *nxd_mqtt_client_websocket_host;
    UINT                           nxd_mqtt_client_websocket_host_length;
    UCHAR                         *nxd_mqtt_client_websocket_uri_path;
    UINT                           nxd_mqtt_client_websocket_uri_path_length;
#endif /* NXD_MQTT_OVER_WEBSOCKET */
} NXD_MQTT_CLIENT;



#ifndef NXD_MQTT_CLIENT_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

#define nxd_mqtt_client_create                _nxd_mqtt_client_create
#define nxd_mqtt_client_login_set             _nxd_mqtt_client_login_set
#define nxd_mqtt_client_will_message_set      _nxd_mqtt_client_will_message_set
#define nxd_mqtt_client_delete                _nxd_mqtt_client_delete
#define nxd_mqtt_client_connect               _nxd_mqtt_client_connect
#define nxd_mqtt_client_secure_connect        _nxd_mqtt_client_secure_connect
#define nxd_mqtt_client_publish               _nxd_mqtt_client_publish
#define nxd_mqtt_client_subscribe             _nxd_mqtt_client_subscribe
#define nxd_mqtt_client_unsubscribe           _nxd_mqtt_client_unsubscribe
#define nxd_mqtt_client_disconnect            _nxd_mqtt_client_disconnect
#define nxd_mqtt_client_receive_notify_set    _nxd_mqtt_client_receive_notify_set
#define nxd_mqtt_client_message_get           _nxd_mqtt_client_message_get
#define nxd_mqtt_client_disconnect_notify_set _nxd_mqtt_client_disconnect_notify_set
#ifdef NXD_MQTT_OVER_WEBSOCKET
#define nxd_mqtt_client_websocket_set         _nxd_mqtt_client_websocket_set
#endif /* NXD_MQTT_OVER_WEBSOCKET */
#else /* if !NXD_MQTT_CLIENT_SOURCE_CODE */

#define nxd_mqtt_client_create                _nxde_mqtt_client_create
#define nxd_mqtt_client_login_set             _nxde_mqtt_client_login_set
#define nxd_mqtt_client_will_message_set      _nxde_mqtt_client_will_message_set
#define nxd_mqtt_client_delete                _nxde_mqtt_client_delete
#define nxd_mqtt_client_connect               _nxde_mqtt_client_connect
#define nxd_mqtt_client_secure_connect        _nxde_mqtt_client_secure_connect
#define nxd_mqtt_client_publish               _nxde_mqtt_client_publish
#define nxd_mqtt_client_subscribe             _nxde_mqtt_client_subscribe
#define nxd_mqtt_client_unsubscribe           _nxde_mqtt_client_unsubscribe
#define nxd_mqtt_client_disconnect            _nxde_mqtt_client_disconnect
#define nxd_mqtt_client_receive_notify_set    _nxde_mqtt_client_receive_notify_set
#define nxd_mqtt_client_message_get           _nxde_mqtt_client_message_get
#define nxd_mqtt_client_disconnect_notify_set _nxde_mqtt_client_disconnect_notify_set
#ifdef NXD_MQTT_OVER_WEBSOCKET
#define nxd_mqtt_client_websocket_set         _nxde_mqtt_client_websocket_set
#endif /* NXD_MQTT_OVER_WEBSOCKET */
#endif /* NX_DISABLE_ERROR_CHECKING */


UINT nxd_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name, CHAR *client_id, UINT client_id_length,
                            NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                            VOID *memory_ptr, ULONG memory_size);

UINT nxd_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                             UINT keepalive, UINT clean_session, ULONG timeout);

UINT nxd_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                               CHAR *username, UINT username_length, CHAR *password, UINT password_length);

UINT nxd_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                      const UCHAR *will_topic, UINT will_topic_length, UCHAR *will_message, UINT will_message_length,
                                      UINT will_retain_flag, UINT will_QoS);
#ifdef NX_SECURE_ENABLE
UINT nxd_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                    UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *, NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                    UINT keepalive, UINT clean_session, ULONG timeout);
#endif /* NX_SECURE_ENABLE */
UINT nxd_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, CHAR *message, UINT message_length,
                             UINT retain, UINT QoS, ULONG timeout);
UINT nxd_mqtt_client_subscribe(NXD_MQTT_CLIENT *mqtt_client_pr, CHAR *topic_name, UINT topic_name_length, UINT QoS);
UINT nxd_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *mqtt_client_pr, CHAR *topic_name, UINT topic_name_length);
UINT nxd_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                        VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT number_of_messages));
UINT nxd_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                 UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length);
UINT nxd_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr);

UINT nxd_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr);
UINT nxd_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *));
#ifdef NXD_MQTT_OVER_WEBSOCKET
UINT nxd_mqtt_client_websocket_set(NXD_MQTT_CLIENT *client_ptr, UCHAR *host, UINT host_length, UCHAR *uri_path, UINT uri_path_length);
#endif /* NXD_MQTT_OVER_WEBSOCKET */

#else /* ifdef NXD_MQTT_CLIENT_SOURCE_CODE */

UINT _nxd_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                              UINT keepalive, UINT clean_session, ULONG wait_option);
UINT _nxd_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                             CHAR *client_id, UINT client_id_length,
                             NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                             VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                             VOID *memory_ptr, ULONG memory_size);
UINT _nxd_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr);
UINT _nxd_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr);
UINT _nxd_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *));
UINT _nxd_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                CHAR *username, UINT username_length, CHAR *password, UINT password_length);
UINT _nxd_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                  UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length);
UINT _nxd_mqtt_client_publish_packet_send(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr,
                                          USHORT packet_id, UINT QoS, ULONG wait_option);
UINT _nxd_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                              CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG timeout);
UINT _nxd_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                         VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count));
UINT _nxd_mqtt_client_release_callback_set(NXD_MQTT_CLIENT *client_ptr, VOID (*memory_release_function)(CHAR *, UINT));
UINT _nxd_mqtt_client_sub_unsub(NXD_MQTT_CLIENT *client_ptr, UINT op,
                                CHAR *topic_name, UINT topic_name_length, USHORT *packet_id_ptr, UINT QoS);
UINT _nxd_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS);
UINT _nxd_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length);
UINT _nxd_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                       const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                       UINT will_message_length, UINT will_retain_flag, UINT will_QoS);
UINT _nxd_mqtt_read_remaining_length(NX_PACKET *packet_ptr, UINT *remaining_length, ULONG *offset_ptr);
UINT _nxd_mqtt_client_set_fixed_header(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, UCHAR control_header, UINT length, UINT wait_option);
UINT _nxd_mqtt_client_append_message(NXD_MQTT_CLIENT *client_ptr, NX_PACKET *packet_ptr, CHAR *message, UINT length, ULONG wait_option);
VOID _nxd_mqtt_client_connection_end(NXD_MQTT_CLIENT *client_ptr, ULONG wait_option);
UINT _nxd_mqtt_process_publish_packet(NX_PACKET *packet_ptr, ULONG *topic_offset_ptr, USHORT *topic_length_ptr,
                                      ULONG *message_offset_ptr, ULONG *message_length_ptr);

UINT _nxde_mqtt_client_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                               UINT keepalive, UINT clean_session, ULONG timeout);
UINT _nxde_mqtt_client_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name, CHAR *client_id, UINT client_id_length,
                              NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr,
                              VOID *stack_ptr, ULONG stack_size, UINT mqtt_thread_priority,
                              VOID *memory_ptr, ULONG memory_size);
UINT _nxde_mqtt_client_delete(NXD_MQTT_CLIENT *client_ptr);
UINT _nxde_mqtt_client_disconnect_notify_set(NXD_MQTT_CLIENT *client_ptr, VOID (*disconnect_notify)(NXD_MQTT_CLIENT *));
UINT _nxde_mqtt_client_disconnect(NXD_MQTT_CLIENT *client_ptr);
UINT _nxde_mqtt_client_login_set(NXD_MQTT_CLIENT *client_ptr,
                                 CHAR *username, UINT username_length, CHAR *password, UINT password_length);
UINT _nxde_mqtt_client_message_get(NXD_MQTT_CLIENT *client_ptr, UCHAR *topic_buffer, UINT topic_buffer_size, UINT *actual_topic_length,
                                   UCHAR *message_buffer, UINT message_buffer_size, UINT *actual_message_length);
UINT _nxde_mqtt_client_publish(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length,
                               CHAR *message, UINT message_length, UINT retain, UINT QoS, ULONG timeout);
UINT _nxde_mqtt_client_receive_notify_set(NXD_MQTT_CLIENT *client_ptr,
                                          VOID (*receive_notify)(NXD_MQTT_CLIENT *client_ptr, UINT message_count));
UINT _nxde_mqtt_client_release_callback_set(NXD_MQTT_CLIENT *client_ptr, VOID (*release_callback)(CHAR *, UINT));
UINT _nxde_mqtt_client_subscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length, UINT QoS);
UINT _nxde_mqtt_client_unsubscribe(NXD_MQTT_CLIENT *client_ptr, CHAR *topic_name, UINT topic_name_length);
UINT _nxde_mqtt_client_will_message_set(NXD_MQTT_CLIENT *client_ptr,
                                        const UCHAR *will_topic, UINT will_topic_length, const UCHAR *will_message,
                                        UINT will_message_length, UINT will_retain_flag, UINT will_QoS);

#ifdef NX_SECURE_ENABLE
UINT _nxd_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                     UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                       NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                     UINT keepalive, UINT clean_session, ULONG wait_option);
UINT _nxde_mqtt_client_secure_connect(NXD_MQTT_CLIENT *client_ptr, NXD_ADDRESS *server_ip, UINT server_port,
                                      UINT (*tls_setup)(NXD_MQTT_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *,
                                                        NX_SECURE_X509_CERT *, NX_SECURE_X509_CERT *),
                                      UINT keepalive, UINT clean_session, ULONG timeout);
#endif /* NX_SECURE_ENABLE */

#ifdef NXD_MQTT_OVER_WEBSOCKET
UINT _nxd_mqtt_client_websocket_set(NXD_MQTT_CLIENT *client_ptr, UCHAR *host, UINT host_length, UCHAR *uri_path, UINT uri_path_length);
UINT _nxde_mqtt_client_websocket_set(NXD_MQTT_CLIENT *client_ptr, UCHAR *host, UINT host_length, UCHAR *uri_path, UINT uri_path_length);
VOID _nxd_mqtt_client_websocket_connection_status_callback(NX_WEBSOCKET_CLIENT *websocket_client_ptr, VOID *context, UINT status);
#endif /* NXD_MQTT_OVER_WEBSOCKET */

#endif /* ifndef NXD_MQTT_CLIENT_SOURCE_CODE */

#ifdef NXD_MQTT_CLOUD_ENABLE
/* MQTT create function based on cloud helper.  */
UINT _nxd_mqtt_client_cloud_create(NXD_MQTT_CLIENT *client_ptr, CHAR *client_name,
                                   CHAR *client_id, UINT client_id_length,
                                   NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_CLOUD *cloud_ptr);
#endif /* NXD_MQTT_CLOUD_ENABLE */

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
}
#endif


#endif /* NXD_MQTT_CLIENT_H  */

