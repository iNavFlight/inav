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
/** NetX POP3 Client Component                                            */
/**                                                                       */
/**   Post Office Protocol Version 3 (POP3)                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_pop3_client.h                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Post Office Protocol Version 3 (POP3)    */
/*    Client component, including all data types and external references. */
/*    It is assumed that tx_api.h, tx_port.h, nx_api.h, nx_port.h,        */
/*    fx_api.h and fx_port.h have already been included.                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef NX_POP3_CLIENT_H
#define NX_POP3_CLIENT_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */ 
extern   "C" {

#endif


#include    "nx_md5.h"
#include    "nx_api.h"

#define NX_POP3_CLIENT_ERROR_CONSTANT         0xB0  

#define NX_POP3_PARAM_ERROR                  (NX_POP3_CLIENT_ERROR_CONSTANT | 0x1) /* Invalid non pointer parameter passed    */
#define NX_POP3_INVALID_MAIL_ITEM            (NX_POP3_CLIENT_ERROR_CONSTANT | 0x2) /* Client block pool not large enough to store packet payload. */
#define NX_POP3_APOP_FAILED_MD5_DIGEST       (NX_POP3_CLIENT_ERROR_CONSTANT | 0x3) /* Client authentication failed; MD5 digest of server ID and password failed. */
#define NX_POP3_SERVER_ERROR_STATUS          (NX_POP3_CLIENT_ERROR_CONSTANT | 0x4) /* Server reply appears to missing expected argument(s). */   
#define NX_POP3_MAIL_BUFFER_OVERFLOW         (NX_POP3_CLIENT_ERROR_CONSTANT | 0x5) /* Mail data extraction has filled up mail buffer before extraction complete. */   
#define NX_POP3_INSUFFICIENT_PACKET_PAYLOAD  (NX_POP3_CLIENT_ERROR_CONSTANT | 0x6) /* Client mail spool callback is not successful. */   
#define NX_POP3_CLIENT_INVALID_STATE         (NX_POP3_CLIENT_ERROR_CONSTANT | 0x7)/* Client not in proper state for specific POP3 action or command. */   
#define NX_POP3_CLIENT_INVALID_INDEX         (NX_POP3_CLIENT_ERROR_CONSTANT | 0x8) /* Client command refers to an out of bounds mail index. */   



#define NX_POP3_MAX_BINARY_MD5              16
#define NX_POP3_MAX_ASCII_MD5               32

#define NX_POP3_SERVER_PROCESS_ID_SIZE      75 

/* POP3 Server replies */
#define NX_POP3_POSITIVE_STATUS             "+OK"
#define NX_POP3_NEGATIVE_STATUS             "-ERR"


/* Set the maximum size for Client command and Server reply text. */

#define NX_POP3_MAX_SERVER_REPLY            512
#define NX_POP3_MAX_CLIENT_COMMAND          150  

/* Create symbols for common POP3 sequences. */
#define NX_POP3_COMMAND_TERMINATION         "\r\n"
#define NX_POP3_END_OF_MESSAGE_TAG          ".\r\n"
#define NX_POP3_DOT                         "."
#define NX_POP3_END_OF_MESSAGE              "\r\n.\r\n"

/* Enumerate the server POP3 replies. */
#define NX_POP3_CODE_INVALID                0
#define NX_POP3_CODE_OK                     1
#define NX_POP3_CODE_ERR                    2


/* Define POP3 Client commands strings.  */

#define NX_POP3_COMMAND_GREETING            "GREETING"
#define NX_POP3_COMMAND_USER                "USER"
#define NX_POP3_COMMAND_APOP                "APOP"
#define NX_POP3_COMMAND_PASS                "PASS"
#define NX_POP3_COMMAND_STAT                "STAT"
#define NX_POP3_COMMAND_RETR                "RETR"
#define NX_POP3_COMMAND_DELE                "DELE"
#define NX_POP3_COMMAND_QUIT                "QUIT"
#define NX_POP3_COMMAND_LIST                "LIST"
#define NX_POP3_COMMAND_RSET                "RSET"
#define NX_POP3_COMMAND_NOOP                "NOOP"

/* Determine the maximum size buffer to store messages in.  This need not be
   limited to a single packet payload of message data, since the POP3 Client
   will receive one or more packets till it sees an end of message symbol,
   and appends each packet message data to this buffer.   */

#ifndef NX_POP3_CLIENT_MAIL_BUFFER_SIZE
#define NX_POP3_CLIENT_MAIL_BUFFER_SIZE                 2000 
#endif



#ifndef NX_POP3_CLIENT_PACKET_TIMEOUT
#define NX_POP3_CLIENT_PACKET_TIMEOUT                   (1 * NX_IP_PERIODIC_RATE)    
#endif

#ifndef NX_POP3_TCP_SOCKET_SEND_WAIT     
#define NX_POP3_TCP_SOCKET_SEND_WAIT                    (10 * NX_IP_PERIODIC_RATE)
#endif



#ifndef NX_POP3_CLIENT_CONNECTION_TIMEOUT
#define NX_POP3_CLIENT_CONNECTION_TIMEOUT               (30  * NX_IP_PERIODIC_RATE) 
#endif

#ifndef NX_POP3_CLIENT_DISCONNECT_TIMEOUT
#define NX_POP3_CLIENT_DISCONNECT_TIMEOUT               (10 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_POP3_SERVER_REPLY_TIMEOUT
#define NX_POP3_SERVER_REPLY_TIMEOUT                    (10 * NX_IP_PERIODIC_RATE)  
#endif

#ifndef NX_POP3_MAX_USERNAME
#define NX_POP3_MAX_USERNAME                            40
#endif

#ifndef NX_POP3_MAX_PASSWORD
#define NX_POP3_MAX_PASSWORD                            20
#endif

/* Set the size of the POP3 Client window size. This should leave room
   for the IP and TCP headers within the IP instance MTU. */
#ifndef NX_POP3_CLIENT_TCP_WINDOW_SIZE
#define NX_POP3_CLIENT_TCP_WINDOW_SIZE                  1460
#endif


/* Define the POP3 Client structure  */

typedef struct NX_POP3_CLIENT_STRUCT
{
    CHAR                            nx_pop3_client_name[NX_POP3_MAX_USERNAME + 1];      /* Client name (also used in authentication) */
    CHAR                            nx_pop3_client_password[NX_POP3_MAX_PASSWORD + 1];  /* Client password for authentication */
    NX_TCP_SOCKET                   nx_pop3_client_tcp_socket;                      /* NetX TCP client socket.  */
    NX_PACKET_POOL                 *nx_pop3_client_packet_pool_ptr;                 /* Packet pool for allocating packets for transmitting POP3 messages */
    UINT                            nx_pop3_client_enable_APOP_authentication;      /* Enable client for APOP authentication  */
    UINT                            nx_pop3_client_mail_status;                     /* Indication if the mail item was retrieved successfully */
    UINT                            nx_pop3_client_maildrop_items;                 /* Number of mail messages waiting in client (user) maildrop. */
    UINT                            nx_pop3_client_maildrop_index;                 /* Index of current mail item. */
    ULONG                           nx_pop3_client_total_message_size;             /* Size of message data in bytes sitting in client (user) maildrop. */
    UINT                            nx_pop3_client_ready_to_download;              /* Indicate POP3 Client can download mail data (e.g. RETR accepted by server). */
    CHAR                            nx_pop3_server_process_id[NX_POP3_SERVER_PROCESS_ID_SIZE + 1];
    NX_MD5                          nx_pop3_client_md5data;       
    NX_PACKET                      *nx_pop3_client_message_ptr;
} NX_POP3_CLIENT;


#ifndef NX_POP3_CLIENT_SOURCE_CODE     

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */


#define   nx_pop3_client_create                     _nx_pop3_client_create
#define   nxd_pop3_client_create                    _nxd_pop3_client_create
#define   nx_pop3_client_mail_items_get             _nx_pop3_client_mail_items_get
#define   nx_pop3_client_mail_item_size_get         _nx_pop3_client_mail_item_size_get
#define   nx_pop3_client_mail_item_message_get      _nx_pop3_client_mail_item_message_get
#define   nx_pop3_client_mail_item_get              _nx_pop3_client_mail_item_get
#define   nx_pop3_client_mail_item_delete           _nx_pop3_client_mail_item_delete
#define   nx_pop3_client_quit                       _nx_pop3_client_quit
#define   nx_pop3_client_delete                     _nx_pop3_client_delete

#else

/* Services with error checking.  */

#define   nx_pop3_client_create                     _nxe_pop3_client_create
#define   nxd_pop3_client_create                    _nxde_pop3_client_create
#define   nx_pop3_client_mail_items_get             _nxe_pop3_client_mail_items_get
#define   nx_pop3_client_mail_item_size_get         _nxe_pop3_client_mail_item_size_get
#define   nx_pop3_client_mail_item_message_get      _nxe_pop3_client_mail_item_message_get
#define   nx_pop3_client_mail_item_get              _nxe_pop3_client_mail_item_get
#define   nx_pop3_client_mail_item_delete           _nxe_pop3_client_mail_item_delete
#define   nx_pop3_client_quit                       _nxe_pop3_client_quit
#define   nx_pop3_client_delete                     _nxe_pop3_client_delete

#endif /* if NX_DISABLE_ERROR_CHECKING */



UINT    nx_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, ULONG server_ip_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    nxd_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, NXD_ADDRESS *server_duo_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    nx_pop3_client_mail_items_get(NX_POP3_CLIENT *client_ptr, UINT *number_mail_items, ULONG *maildrop_total_size);
UINT    nx_pop3_client_mail_item_size_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *size);
UINT    nx_pop3_client_mail_item_message_get(NX_POP3_CLIENT *client_ptr, NX_PACKET **recv_packet_ptr, ULONG *bytes_retrieved, UINT *final_packet); 
UINT    nx_pop3_client_mail_item_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *item_size);
UINT    nx_pop3_client_mail_item_delete(NX_POP3_CLIENT *client_ptr, UINT mail_index);
UINT    nx_pop3_client_quit(NX_POP3_CLIENT *client_ptr);
UINT    nx_pop3_client_delete(NX_POP3_CLIENT *client_ptr);


#else   /* if NX_POP3_CLIENT_SOURCE_CODE */


/* Client and session specific functions.  */

UINT    _nx_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, ULONG server_ip_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    _nxe_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, ULONG server_ip_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    _nxd_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, NXD_ADDRESS *server_duo_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    _nxde_pop3_client_create(NX_POP3_CLIENT *client_ptr, UINT APOP_authentication, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool_ptr, NXD_ADDRESS *server_duo_address, ULONG server_port, CHAR *client_name, CHAR *client_password);
UINT    _nx_pop3_client_mail_items_get(NX_POP3_CLIENT *client_ptr, UINT *number_mail_items, ULONG *maildrop_total_size);
UINT    _nxe_pop3_client_mail_items_get(NX_POP3_CLIENT *client_ptr, UINT *number_mail_items, ULONG *maildrop_total_size);
UINT    _nx_pop3_client_mail_item_size_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *size);
UINT    _nxe_pop3_client_mail_item_size_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *size);
UINT    _nx_pop3_client_mail_item_message_get(NX_POP3_CLIENT *client_ptr, NX_PACKET **recv_packet_ptr, ULONG *bytes_retrieved, UINT *final_packet); 
UINT    _nxe_pop3_client_mail_item_message_get(NX_POP3_CLIENT *client_ptr, NX_PACKET **recv_packet_ptr, ULONG *bytes_retrieved, UINT *final_packet); 
UINT    _nx_pop3_client_mail_item_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *item_size);
UINT    _nxe_pop3_client_mail_item_get(NX_POP3_CLIENT *client_ptr, UINT mail_item, ULONG *item_size);
UINT    _nx_pop3_client_mail_item_delete(NX_POP3_CLIENT *client_ptr, UINT mail_index);
UINT    _nxe_pop3_client_mail_item_delete(NX_POP3_CLIENT *client_ptr, UINT mail_index);
UINT    _nx_pop3_client_delete(NX_POP3_CLIENT *client_ptr);
UINT    _nxe_pop3_client_delete(NX_POP3_CLIENT *client_ptr);
UINT    _nx_pop3_client_quit(NX_POP3_CLIENT *client_ptr);
UINT    _nxe_pop3_client_quit(NX_POP3_CLIENT *client_ptr);

#endif

/* NetX POP3 Client internal functions */

UINT    _nx_pop3_digest_authenticate(NX_POP3_CLIENT *client_ptr, CHAR *process_ID_ptr, UINT process_ID_length, CHAR *result);
VOID    _nx_pop3_parse_process_id(NX_POP3_CLIENT *client_ptr, CHAR *buffer, UINT buffer_length);
VOID    _nx_pop3_hex_ascii_convert(CHAR *source, UINT source_length, CHAR *destination);
UINT    _nxd_pop3_client_connect(NX_POP3_CLIENT *client_ptr, NXD_ADDRESS *server_ip_address, ULONG server_port);
UINT    _nx_pop3_server_number_convert(UINT number, CHAR *string_to_convert);
VOID    _nx_pop3_parse_response(CHAR *buffer, UINT argument_index, UINT buffer_length, CHAR *argument, UINT argument_length, UINT convert_to_uppercase, UINT include_crlf);
UINT    _nx_pop3_client_user_pass(NX_POP3_CLIENT *client_ptr);
UINT    _nx_pop3_client_apop(NX_POP3_CLIENT *client_ptr);


/* If a C++ compiler is being used....*/
#ifdef   __cplusplus
        }
#endif


#endif /* NX_POP3_CLIENT_H  */
