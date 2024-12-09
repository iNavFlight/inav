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
/** NetX WebSocket Component                                              */
/**                                                                       */
/**   WebSocket Protocol                                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_websocket_client.h                               PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX WebSocket Protocol component, including  */
/*    all data types and external references.                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Bo Chen                  Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/

#ifndef NX_WEBSOCKET_CLIENT_H
#define NX_WEBSOCKET_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_sha1.h"

#ifdef NX_SECURE_ENABLE
#include "nx_secure_tls_api.h"
#endif /* NX_SECURE_ENABLE */

#ifdef NX_DISABLE_PACKET_CHAIN
#error "NX_DISABLE_PACKET_CHAIN must not be defined"
#endif /* NX_DISABLE_PACKET_CHAIN */

/* Define the WebSocket ID.  */
#define NX_WEBSOCKET_CLIENT_ID                  0x57454253UL

/* Define the GUID size.  */
#define NX_WEBSOCKET_CLIENT_GUID_SIZE           16

/* Define the WebSocket Key size.  */
#define NX_WEBSOCKET_CLIENT_KEY_SIZE            26

/* WebSocket Header Format:
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16)              |
    |N|V|V|V|       |S|             |                               |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    | Masking-key , if MASK set to 1                                |
    +-------------------------------- - - - - - - - - - - - - - - - +
    | Payload Data...                                               |
    +-------------------------------- - - - - - - - - - - - - - - - +
*/

/* First two bytes are required.  */

/* Define FIN.  */
#define NX_WEBSOCKET_FIN                            0x80
#define NX_WEBSOCKET_FIN_MASK                       0x80

/* Define the opcode.  */
#define NX_WEBSOCKET_OPCODE_CONTINUATION_FRAME      0x00
#define NX_WEBSOCKET_OPCODE_TEXT_FRAME              0x01
#define NX_WEBSOCKET_OPCODE_BINARY_FRAME            0x02
#define NX_WEBSOCKET_OPCODE_CONNECTION_CLOSE        0x08
#define NX_WEBSOCKET_OPCODE_PING                    0x09
#define NX_WEBSOCKET_OPCODE_PONG                    0x0A
#define NX_WEBSOCKET_OPCODE_MASK                    0x0F

/* Define the mask bit.  */
#define NX_WEBSOCKET_MASK                           0x80
#define NX_WEBSOCKET_MASK_MASK                      0x80

/* Define the payload length, always using 7bits + 16bits for payload length.  */
#define NX_WEBSOCKET_PAYLOAD_LEN_MASK               0x7F
#define NX_WEBSOCKET_PAYLOAD_LEN_16BITS             0x7E /* Payload length: 126, the following 2 bytes interpreted as a 16-bits unsigned integer are the payload length.  */
#define NX_WEBSOCKET_PAYLOAD_LEN_64BITS             0x7F /* Payload length: 127, the following 4 bytes interpreted as a 64-bits unsigned integer are the payload length.  */
#define NX_WEBSOCKET_EXTENDED_PAYLOAD_16BITS_SIZE   2
#define NX_WEBSOCKET_EXTENDED_PAYLOAD_64BITS_SIZE   8

/* Define the masking key size.  */
#define NX_WEBSOCKET_MASKING_KEY_SIZE               4

/* Define the basic header size (2 bytes normal header  + 4 bytes masking key)*/
#define NX_WEBSOCKET_HEADER_NORMAL_SIZE             6

/* Define the header size (2 bytes normal header  + 2 bytes extended payload length + 4 bytes masking key). 8 bytes extended payload length is not supported yet.  */
#define NX_WEBSOCKET_HEADER_SIZE                    8

/* Define the state.  */
#define NX_WEBSOCKET_CLIENT_STATE_INITIALIZE           0
#define NX_WEBSOCKET_CLIENT_STATE_IDLE                 1
#define NX_WEBSOCKET_CLIENT_STATE_CONNECTING           2
#define NX_WEBSOCKET_CLIENT_STATE_CONNECTED            3
#define NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_SENT      4
#define NX_WEBSOCKET_CLIENT_STATE_DISCONNECT_RECEIVED  5

/* Define the return value.  */
#define NX_WEBSOCKET_SUCCESS                        0x0
#define NX_WEBSOCKET_ERROR                          0x30001
#define NX_WEBSOCKET_CONNECTING                     0x30002
#define NX_WEBSOCKET_ALREADY_CONNECTED              0x30003
#define NX_WEBSOCKET_NOT_CONNECTED                  0x30004
#define NX_WEBSOCKET_DATA_APPEND_FAILURE            0x30005
#define NX_WEBSOCKET_INVALID_STATE                  0x30006
#define NX_WEBSOCKET_INVALID_PACKET                 0x30007
#define NX_WEBSOCKET_INVALID_STATUS_CODE            0x30008
#define NX_WEBSOCKET_DISCONNECTED                   0x30009

/* Define the websocket Client data structure.  */
typedef struct NX_WEBSOCKET_CLIENT_STRUCT
{

    /* WebSocket Client ID.  */
    ULONG                       nx_websocket_client_id;

    /* Name of this WebSocket Client.  */
    UCHAR                      *nx_websocket_client_name;

    /* Pointer to associated IP structure.  */
    NX_IP                      *nx_websocket_client_ip_ptr;

    /* Pointer to WebSocket Client packet pool.  */
    NX_PACKET_POOL             *nx_websocket_client_packet_pool_ptr;

    /* State.  */
    UINT                        nx_websocket_client_state;

    /* Pointer to WebSocket Client TCP socket.  */
    NX_TCP_SOCKET              *nx_websocket_client_socket_ptr;

#ifdef NX_SECURE_ENABLE
    UINT                        nx_websocket_client_use_tls;
    NX_SECURE_TLS_SESSION      *nx_websocket_client_tls_session_ptr;
#endif

    /* Pointer to the received packet to be processed. This packet may be composited by more than one tcp/tls packet. */
    NX_PACKET                  *nx_websocket_client_processing_packet;

    /* Globally Unique Identifier.  */
    UCHAR                       nx_websocket_client_guid[NX_WEBSOCKET_CLIENT_GUID_SIZE];

    /* Protocol Name and length */
    UCHAR                      *nx_websocket_client_subprotocol;
    UINT                        nx_websocket_client_subprotocol_length;

    /* WebSocket-Key.  */
    UINT                        nx_websocket_client_key_size;
    UCHAR                       nx_websocket_client_key[NX_WEBSOCKET_CLIENT_KEY_SIZE];

    /* Websocket frame header parse context */
    UCHAR                       nx_websocket_client_frame_header_found;
    UCHAR                       nx_websocket_client_frame_fragmented;
    UCHAR                       nx_websocket_client_frame_opcode;
    UCHAR                       nx_websocket_client_frame_masked;
    UCHAR                       nx_websocket_client_frame_masking_key[4];
    ULONG                       nx_websocket_client_frame_data_length;
    ULONG                       nx_websocket_client_frame_data_received;

    /* SHA1 in connect response calculation for Sec-Protocol-Accept field */
    NX_SHA1                     nx_websocket_client_sha1;

    /* Connection status callback function.  */
    VOID                        (*nx_websocket_client_connection_status_callback)(struct NX_WEBSOCKET_CLIENT_STRUCT *, VOID *, UINT);

    /* Pointer to an argument passed to connection status callback.  */
    VOID                       *nx_websocket_client_connection_context;

    /* Define the websocket protect purpose mutex */
    TX_MUTEX                    nx_websocket_client_mutex;

} NX_WEBSOCKET_CLIENT;



#ifndef NX_WEBSOCKET_CLIENT_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_websocket_client_create                          _nx_websocket_client_create
#define nx_websocket_client_delete                          _nx_websocket_client_delete
#define nx_websocket_client_connect                         _nx_websocket_client_connect
#ifdef NX_SECURE_ENABLE
#define nx_websocket_client_secure_connect                  _nx_websocket_client_secure_connect
#endif /* NX_SECURE_ENABLE */
#define nx_websocket_client_disconnect                      _nx_websocket_client_disconnect
#define nx_websocket_client_packet_allocate                 _nx_websocket_client_packet_allocate
#define nx_websocket_client_send                            _nx_websocket_client_send
#define nx_websocket_client_receive                         _nx_websocket_client_receive
#define nx_websocket_client_connection_status_callback_set  _nx_websocket_client_connection_status_callback_set

#else

/* Services with error checking.  */

#define nx_websocket_client_create                          _nxe_websocket_client_create
#define nx_websocket_client_delete                          _nxe_websocket_client_delete
#define nx_websocket_client_connect                         _nxe_websocket_client_connect
#ifdef NX_SECURE_ENABLE
#define nx_websocket_client_secure_connect                  _nxe_websocket_client_secure_connect
#endif /* NX_SECURE_ENABLE */
#define nx_websocket_client_disconnect                      _nxe_websocket_client_disconnect
#define nx_websocket_client_packet_allocate                 _nxe_websocket_client_packet_allocate
#define nx_websocket_client_send                            _nxe_websocket_client_send
#define nx_websocket_client_receive                         _nxe_websocket_client_receive
#define nx_websocket_client_connection_status_callback_set  _nxe_websocket_client_connection_status_callback_set

#endif  /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT  nx_websocket_client_create(NX_WEBSOCKET_CLIENT *client_ptr, UCHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr);
UINT  nx_websocket_client_delete(NX_WEBSOCKET_CLIENT *client_ptr);
UINT  nx_websocket_client_packet_allocate(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT  nx_websocket_client_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_TCP_SOCKET *socket_ptr,
                                  UCHAR *host, UINT host_length,
                                  UCHAR *uri_path, UINT uri_path_length,
                                  UCHAR *protocol, UINT protocol_length,UINT wait_option);
#ifdef NX_SECURE_ENABLE
UINT  nx_websocket_client_secure_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                         UCHAR *host, UINT host_length,
                                         UCHAR *uri_path, UINT uri_path_length,
                                         UCHAR *protocol, UINT protocol_length,UINT wait_option);
#endif /* NX_SECURE_ENABLE */
UINT  nx_websocket_client_disconnect(NX_WEBSOCKET_CLIENT *client_ptr, UINT wait_option);
UINT  nx_websocket_client_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT code, UINT is_final, UINT wait_option);
UINT  nx_websocket_client_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code, UINT wait_option);
UINT  nx_websocket_client_connection_status_callback_set(NX_WEBSOCKET_CLIENT *client_ptr, VOID *context,
                                                         VOID (*connection_status_callback)(NX_WEBSOCKET_CLIENT *, VOID *, UINT));

#else

/* Websocket source code is being compiled, do not perform any API mapping.  */

UINT  _nxe_websocket_client_create(NX_WEBSOCKET_CLIENT *client_ptr, UCHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr);
UINT  _nx_websocket_client_create(NX_WEBSOCKET_CLIENT *client_ptr, UCHAR *client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr);
UINT  _nxe_websocket_client_delete(NX_WEBSOCKET_CLIENT *client_ptr);
UINT  _nx_websocket_client_delete(NX_WEBSOCKET_CLIENT *client_ptr);
UINT  _nxe_websocket_client_packet_allocate(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT  _nx_websocket_client_packet_allocate(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT  _nxe_websocket_client_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_TCP_SOCKET *socket_ptr,
                                    UCHAR *host, UINT host_length,
                                    UCHAR *uri_path, UINT uri_path_length,
                                    UCHAR *protocol, UINT protocol_length,UINT wait_option);
UINT  _nx_websocket_client_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_TCP_SOCKET *socket_ptr,
                                   UCHAR *host, UINT host_length,
                                   UCHAR *uri_path, UINT uri_path_length,
                                   UCHAR *protocol, UINT protocol_length,UINT wait_option);
#ifdef NX_SECURE_ENABLE
UINT  _nxe_websocket_client_secure_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                           UCHAR *host, UINT host_length,
                                           UCHAR *uri_path, UINT uri_path_length,
                                           UCHAR *protocol, UINT protocol_length,UINT wait_option);
UINT  _nx_websocket_client_secure_connect(NX_WEBSOCKET_CLIENT *client_ptr, NX_SECURE_TLS_SESSION *tls_session,
                                          UCHAR *host, UINT host_length,
                                          UCHAR *uri_path, UINT uri_path_length,
                                          UCHAR *protocol, UINT protocol_length,UINT wait_option);
#endif /* NX_SECURE_ENABLE */
UINT  _nxe_websocket_client_disconnect(NX_WEBSOCKET_CLIENT *client_ptr, UINT wait_option);
UINT  _nx_websocket_client_disconnect(NX_WEBSOCKET_CLIENT *client_ptr, UINT wait_option);
UINT  _nxe_websocket_client_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT code, UINT is_final, UINT wait_option);
UINT  _nx_websocket_client_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT code, UINT is_final, UINT wait_option);
UINT  _nxe_websocket_client_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code, UINT wait_option);
UINT  _nx_websocket_client_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code, UINT wait_option);
UINT  _nxe_websocket_client_connection_status_callback_set(NX_WEBSOCKET_CLIENT *client_ptr, VOID * context,
                                                           VOID (*connection_status_callback)(NX_WEBSOCKET_CLIENT *, VOID *, UINT));
UINT  _nx_websocket_client_connection_status_callback_set(NX_WEBSOCKET_CLIENT *client_ptr, VOID * context,
                                                          VOID (*connection_status_callback)(NX_WEBSOCKET_CLIENT *, VOID *, UINT));

#endif  /* NX_WEBSOCKET_CLIENT_SOURCE_CODE */

/* Define internal websocket functions.  */
UINT  _nx_websocket_client_connect_internal(NX_WEBSOCKET_CLIENT *client_ptr,
                                            UCHAR *host, UINT host_length,
                                            UCHAR *uri_path, UINT uri_path_length,
                                            UCHAR *protocol, UINT protocol_length,UINT wait_option);
UINT  _nx_websocket_client_name_compare(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length);
UINT  _nx_websocket_client_connect_response_process(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr);
UINT  _nx_websocket_client_packet_trim(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG trim_size);
UINT  _nx_websocket_client_packet_send(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT  _nx_websocket_client_packet_receive(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT  _nx_websocket_client_data_process(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET **packet_ptr, UINT *code);
UINT  _nx_websocket_client_connect_response_check(NX_WEBSOCKET_CLIENT *client_ptr, NX_PACKET *packet_ptr, UINT wait_option);
void  _nx_websocket_client_cleanup(NX_WEBSOCKET_CLIENT *client_ptr);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NX_WEBSOCKET_CLIENT_H */
