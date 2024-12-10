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
/** NetX SMTP Client Component                                            */
/**                                                                       */
/**   Simple Mail Transfer Protocol (SMTP)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_smtp_client.h                                   PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Simple Mail Transfer Protocol (SMTP)     */
/*    Client component, including all data types and external references. */
/*    It is assumed that tx_api.h, tx_port.h, nx_api.h, and nx_port.h,    */
/*    have already been included.                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
#ifndef NXD_SMTP_CLIENT_H
#define NXD_SMTP_CLIENT_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */ 
extern   "C" {

#endif

#include "nx_api.h"


/* NX SMTP Client configurable options.  */

/* Set the TCP socket window size. */
    
#ifndef NX_SMTP_CLIENT_TCP_WINDOW_SIZE
#define NX_SMTP_CLIENT_TCP_WINDOW_SIZE         1460
#endif

/* Set timeout on Client packet allocation in ticks.  */

#ifndef NX_SMTP_CLIENT_PACKET_TIMEOUT
#define NX_SMTP_CLIENT_PACKET_TIMEOUT           (2 * NX_IP_PERIODIC_RATE)    
#endif

/* Set Client TCP connection timeout in seconds.  */

#ifndef NX_SMTP_CLIENT_CONNECTION_TIMEOUT     
#define NX_SMTP_CLIENT_CONNECTION_TIMEOUT     (10 * NX_IP_PERIODIC_RATE)
#endif

/* Set Client TCP disconnect timeout in seconds.  */

#ifndef NX_SMTP_CLIENT_DISCONNECT_TIMEOUT 
#define NX_SMTP_CLIENT_DISCONNECT_TIMEOUT      (5 * NX_IP_PERIODIC_RATE)
#endif

/* Set Client timeout in seconds for waiting for server reply to client greeting.  */

#ifndef NX_SMTP_GREETING_TIMEOUT
#define NX_SMTP_GREETING_TIMEOUT              (10 * NX_IP_PERIODIC_RATE)
#endif


/* Set Client 'envelope' timeout in seconds for waiting for server reply to client commands.  */

#ifndef NX_SMTP_ENVELOPE_TIMEOUT
#define NX_SMTP_ENVELOPE_TIMEOUT              (10 * NX_IP_PERIODIC_RATE)
#endif


/* Set Client timeout in seconds for waiting to receive server acceptance of client message data.  */

#ifndef NX_SMTP_MESSAGE_TIMEOUT
#define NX_SMTP_MESSAGE_TIMEOUT               (30 * NX_IP_PERIODIC_RATE) 
#endif


/* Set timeout for TCP socket send completion.  */

#ifndef NX_SMTP_CLIENT_SEND_TIMEOUT     
#define NX_SMTP_CLIENT_SEND_TIMEOUT            (5 * NX_IP_PERIODIC_RATE)
#endif


/* Define size for Client profile data. */

#ifndef NX_SMTP_CLIENT_MAX_USERNAME
#define NX_SMTP_CLIENT_MAX_USERNAME             40
#endif

#ifndef NX_SMTP_CLIENT_MAX_PASSWORD
#define NX_SMTP_CLIENT_MAX_PASSWORD             20
#endif



/* Define size of the buffer to extract the server challenge for authentication.    
   There is no specific size here so 200 bytes is sufficient to cover all digest string handling.  */
#ifndef NX_SMTP_SERVER_CHALLENGE_MAX_STRING
#define NX_SMTP_SERVER_CHALLENGE_MAX_STRING     200
#endif


/* Define size for handling data for authentication (LOGIN, PLAIN):
   PLAIN requires rooms for authorization-id\0authentication-id\0passwd'. 
   The two bytes are for the NULL byte between the first two auth id and 
   between auth id and password. */

#define NX_SMTP_CLIENT_AUTH_CHALLENGE_SIZE    (NX_SMTP_CLIENT_MAX_USERNAME + NX_SMTP_CLIENT_MAX_USERNAME + NX_SMTP_CLIENT_MAX_PASSWORD + 2)
#define NX_SMTP_CLIENT_AUTH_CHALLENGE_ENCODED_SIZE  (NX_SMTP_CLIENT_AUTH_CHALLENGE_SIZE * 4 / 3 + 1)


/* These define the states of the protocol state machine */
                    
#define   NX_SMTP_CLIENT_STATE_AWAITING_REPLY       0xFFFFFFFF     /* Session state depends on outcome of current response handler.  */
#define   NX_SMTP_CLIENT_STATE_COMPLETED_NORMALLY   0xFFFFFFFE     /* No internal errors, session completed normally.  */
#define   NX_SMTP_CLIENT_STATE_ERROR                0xFFFFFFFD     /* Internal errors e.g. TCP send or receive fails; session terminated abnormally.  */


#define NX_SMTP_INVALID_PARAM                   0xA5                      /* Invalid non pointer input in an SMTP service call */
#define NX_SMTP_INTERNAL_ERROR                  0xA3                      /* Internal processing error */
#define NX_SMTP_AUTHENTICATION_ERROR            0xA0                      /* Invalid input for creating Client authentication. */
#define NX_SMTP_OVERSIZE_MAIL_DATA              0xA1                      /* Mail message exceeds buffer size */
#define NX_SMTP_INVALID_SERVER_REPLY            0xA2                      /* Unknown or invalid server reply */
#define NX_SMTP_SERVER_ERROR_CODE_RECEIVED      0xA4                      /* Received an SMTP Server error code */
#define NX_SMTP_PACKET_ALLOCATE_ERROR           0xA6                      /* Error allocating packet for SMTP message transmission */
#define NX_SMTP_GREET_REPLY_ERROR               0xA7                      /* Error in response to Client SMTP GREET command */
#define NX_SMTP_HELLO_REPLY_ERROR               0xA8                      /* Error in response to Client SMTP HELO or EHLO command */
#define NX_SMTP_MAIL_REPLY_ERROR                0xA9                      /* Error in response to Client SMTP MAIL command */
#define NX_SMTP_RCPT_REPLY_ERROR                0xAA                      /* Error in response to Client SMTP RCPT command */
#define NX_SMTP_MESSAGE_REPLY_ERROR             0xAB                      /* Error in response to Client SMTP MESSAGE data sent */
#define NX_SMTP_DATA_REPLY_ERROR                0xAC                      /* Error in response to Client SMTP DATA command */
#define NX_SMTP_AUTH_REPLY_ERROR                0xAD                      /* Error in response to Client SMTP AUTH command */
#define NX_SMTP_SERVER_ERROR_REPLY              0xAE                      /* Error in parsing Server reply code (not found or unknown) */
#define NX_SMTP_TRANSMIT_ERROR                  0xAF                      /* Error occurred during TCP packet transmission e.g. send queue full */
#define NX_SMTP_INVALID_SERVER_CHALLENGE        0xB0                      /* Invalid server challenge (e.g. missing enclosing angle brackets). */
#define NX_SMTP_OVERSIZE_SERVER_REPLY           0xB1                      /* Server reply exceeds client session buffer size */
#define NX_SMTP_CLIENT_NOT_INTIALIZED           0xB2                      /* Client not created successfully e.g. socket create failed. Cannot transmit mail. */

/* Basic SMTP commands supported by this NetX SMTP API.  */

#define NX_SMTP_COMMAND_EHLO                            "EHLO"
#define NX_SMTP_COMMAND_HELO                            "HELO"
#define NX_SMTP_COMMAND_MAIL                            "MAIL FROM"
#define NX_SMTP_COMMAND_RCPT                            "RCPT TO"
#define NX_SMTP_COMMAND_AUTH                            "AUTH"
#define NX_SMTP_COMMAND_NOOP                            "NOOP"
#define NX_SMTP_COMMAND_DATA                            "DATA"
#define NX_SMTP_COMMAND_RSET                            "RSET"
#define NX_SMTP_COMMAND_QUIT                            "QUIT"

/* List of common SMTP server reply codes */
                  
#define     NX_SMTP_CODE_GREETING_OK                       220 
#define     NX_SMTP_CODE_ACKNOWLEDGE_QUIT                  221
#define     NX_SMTP_CODE_AUTHENTICATION_SUCCESSFUL         235
#define     NX_SMTP_CODE_OK_TO_CONTINUE                    250
#define     NX_SMTP_CODE_CANNOT_VERIFY_RECIPIENT           252
#define     NX_SMTP_CODE_AUTHENTICATION_TYPE_ACCEPTED      334
#define     NX_SMTP_CODE_SEND_MAIL_INPUT                   354
#define     NX_SMTP_CODE_SERVICE_NOT_AVAILABLE             421
#define     NX_SMTP_CODE_SERVICE_INTERNAL_SERVER_ERROR     451
#define     NX_SMTP_CODE_INSUFFICIENT_STORAGE              452
#define     NX_SMTP_CODE_AUTH_FAILED_INTERNAL_SERVER_ERROR 454
#define     NX_SMTP_CODE_COMMAND_SYNTAX_ERROR              500
#define     NX_SMTP_CODE_PARAMETER_SYNTAX_ERROR            501
#define     NX_SMTP_CODE_COMMAND_NOT_IMPLEMENTED           502
#define     NX_SMTP_CODE_BAD_SEQUENCE                      503
#define     NX_SMTP_CODE_PARAMETER_NOT_IMPLEMENTED         504
#define     NX_SMTP_CODE_AUTH_REQUIRED                     530
#define     NX_SMTP_CODE_AUTH_FAILED                       535
#define     NX_SMTP_CODE_REQUESTED_ACTION_NOT_TAKEN        550
#define     NX_SMTP_CODE_USER_NOT_LOCAL                    551 
#define     NX_SMTP_CODE_OVERSIZE_MAIL_DATA                552
#define     NX_SMTP_CODE_BAD_MAILBOX                       553
#define     NX_SMTP_CODE_TRANSACTION_FAILED                554
#define     NX_SMTP_CODE_BAD_SERVER_CODE_RECEIVED          555


/* Common components of SMTP command messages */

#define NX_SMTP_LINE_TERMINATOR                     "\r\n"
#define NX_SMTP_EOM                                 "\r\n.\r\n"   
#define NX_SMTP_MESSAGE_ID                          "Message-ID"
#define NX_SMTP_TO_STRING                           "To: "
#define NX_SMTP_FROM_STRING                         "From: "
#define NX_SMTP_SUBJECT_STRING                      "Subject: "
#define NX_SMTP_MAIL_HEADER_COMPONENTS              "MIME-Version: 1.0\r\n" \
                                                    "Content-Type: text/plain;\r\n" \
                                                    "  charset=\"utf-8\"\r\n" \
                                                    "Content-Transfer-Encoding: 8bit\r\n" \
                                                    "\r\n"


/* Enumerated states of the protocol state machine. These MUST be in the 
   same order as the list of protocol states in NX_SMTP_CLIENT_STATES.*/  

typedef enum NX_SMTP_CLIENT_STATE_ENUM
{
    NX_SMTP_CLIENT_STATE_IDLE = 0,      
    NX_SMTP_CLIENT_STATE_GREETING,      /*1*/ 
    NX_SMTP_CLIENT_STATE_EHLO,          /*2 */ 
    NX_SMTP_CLIENT_STATE_HELO,          /*3*/ 
    NX_SMTP_CLIENT_STATE_MAIL,          /*4*/ 
    NX_SMTP_CLIENT_STATE_RCPT,          /*5*/ 
    NX_SMTP_CLIENT_STATE_DATA,          /*6*/ 
    NX_SMTP_CLIENT_STATE_MESSAGE,       /*7*/ 
    NX_SMTP_CLIENT_STATE_RSET,          /*8*/ 
    NX_SMTP_CLIENT_STATE_QUIT,          /*9*/ 
    NX_SMTP_CLIENT_STATE_NOOP,          /*10 */ 
    NX_SMTP_CLIENT_STATE_AUTH,          /*11*/ 
    NX_SMTP_CLIENT_STATE_AUTH_CHALLENGE /*12*/ 

} NX_SMTP_CLIENT_STATE;


/* Enumeration of common server challenges to the client */

#define       NX_SMTP_CLIENT_REPLY_TO_UNKNOWN_PROMPT        1
#define       NX_SMTP_CLIENT_REPLY_TO_USERNAME_PROMPT       2
#define       NX_SMTP_CLIENT_REPLY_TO_PASSWORD_PROMPT       3
#define       NX_SMTP_CLIENT_REPLY_SERVER_CHALLENGE_PROMPT  4   

/* Common server challenges from the SMTP server. */

#define NX_SMTP_USERNAME_PROMPT                  "Username:"
#define NX_SMTP_PASSWORD_PROMPT                  "Password:"

/* ID for identifying as an SMTP client */

#define NX_SMTP_CLIENT_ID                       0x534D5450UL

/* Define the character to cancel authentication process (RFC mandated). */

#define NX_SMTP_CANCEL_AUTHENTICATION           "*"


/* Enumeration of the state of authentication between server and client */

typedef enum  NX_SMTP_AUTHENTICATION_STATE_ENUM
{
    NX_SMTP_NOT_AUTHENTICATED,
    NX_SMTP_AUTHENTICATION_IN_PROGRESS, 
    NX_SMTP_AUTHENTICATION_FAILED,
    NX_SMTP_AUTHENTICATION_SUCCEEDED

} NX_SMTP_AUTHENTICATION_STATE;


/* Defines for deciding priority of mail.  */

#define NX_SMTP_MAIL_PRIORITY_LOW               0x01
#define NX_SMTP_MAIL_PRIORITY_NORMAL            0x02
#define NX_SMTP_MAIL_PRIORITY_HIGH              0x04


/* Defines for type of mail recipient.  */

#define NX_SMTP_RECIPIENT_TO                    0x01
#define NX_SMTP_RECIPIENT_CC                    0x02
#define NX_SMTP_RECIPIENT_BCC                   0x04


/* Define size of SMTP reply status codes (RFC mandated). */

#define NX_SMTP_SERVER_REPLY_CODE_SIZE                  3

#define NX_SMTP_CLIENT_AUTH_NONE                         0xFFFF
#define NX_SMTP_CLIENT_AUTH_LOGIN                        1
#define NX_SMTP_CLIENT_AUTH_LOGIN_TEXT                   "AUTH LOGIN"
#define NX_SMTP_CLIENT_AUTH_CRAM_MD5                     2
#define NX_SMTP_CLIENT_AUTH_CRAM_MD5_TEXT                "AUTH CRAM-MD5"
#define NX_SMTP_CLIENT_AUTH_PLAIN                        3
#define NX_SMTP_CLIENT_AUTH_PLAIN_TEXT                   "AUTH PLAIN"

/* Define the NetX SMTP RECIPIENT structure */

/* Define the NetX SMTP MAIL structure */

typedef struct NX_SMTP_CLIENT_MAIL_STRUCT
{
    CHAR                                    *nx_smtp_client_mail_recipient_address;         /* Recipient's mailbox address */
    CHAR                                    *nx_smtp_client_mail_from_address;              /* Sender's mailbox address.  */
    UINT                                    nx_smtp_client_mail_priority;                   /* Mail item priority level */
    CHAR                                    *nx_smtp_client_mail_subject;
    CHAR                                    *nx_smtp_client_mail_body;                      /* Pointer to text of mail to send.  */
    UINT                                    nx_smtp_client_mail_body_length;                /* Size of mail buffer.  */
} NX_SMTP_CLIENT_MAIL;


/* Define the SMTP client structure  */

typedef struct NX_SMTP_CLIENT_STRUCT
{
    ULONG                           nx_smtp_client_id;                                      /* SMTP ID for identify client service.  */
    CHAR                            nx_smtp_username[NX_SMTP_CLIENT_MAX_USERNAME + 1];      /* Client name (may be used in authentication) */
    CHAR                            nx_smtp_password[NX_SMTP_CLIENT_MAX_PASSWORD + 1];      /* Client password (used in authentication) */
    CHAR                            nx_smtp_client_domain[NX_SMTP_CLIENT_MAX_USERNAME + 1]; /* Client domain of the client (and sender) */
    UINT                            nx_smtp_client_authentication_type;                     /* Default Client authentication. */
    NX_IP                           *nx_smtp_client_ip_ptr;                                 /* Client IP instance  */
    NX_PACKET_POOL                  *nx_smtp_client_packet_pool_ptr;                        /* Client packet pool for sending data packets to the server */
    NX_SMTP_CLIENT_MAIL             nx_smtp_client_mail;                                    /* Session mail is the collection of parameters required to create an SMTP mail message. */
    UINT                            nx_smtp_client_init_status;                             /* If true SMTP client successfully created and ready for transmitting mail. */
    NXD_ADDRESS                     nx_smtp_server_address;                                 /* Server IP address (IPv4 or IPv6).  */
    USHORT                          nx_smtp_server_port;                                    /* Server port.  */
    NX_TCP_SOCKET                   nx_smtp_client_socket;                                  /* Client NetX TCP socket.  */
    UINT                            nx_smtp_client_cmd_state;                               /* Command state of the SMTP session.  */
    UINT                            nx_smtp_client_rsp_state;                               /* Response state of the SMTP session.  */
    UINT                            nx_smtp_client_reply_code_status;                       /* Reply code received from SMTP server.  */
    NX_PACKET                       *nx_smtp_server_packet;                                 /* Packet containing server reply. */
    UINT                            nx_smtp_client_authentication_reply;                    /* Buffer holding server reply text during authentication process */
    NX_SMTP_AUTHENTICATION_STATE    nx_smtp_client_authentication_state;                    /* State of the authentication process */
    UINT                            nx_smtp_client_data_transparency_bytes;                 /* Extra bytes allowed for data transparency processing to add to message data.  */
    UINT                            nx_smtp_client_mail_status;                             /* Status of mail acceptance by the server */
    UINT                            nx_smtp_client_mute;                                    /* Mute command state; client waits for another packet in same SMTP state */

} NX_SMTP_CLIENT;


typedef struct NX_SMTP_CLIENT_STATES_STRUCT
{
    UINT    (*cmd) (NX_SMTP_CLIENT *client_ptr);
    UINT    (*rsp) (NX_SMTP_CLIENT *client_ptr);

} NX_SMTP_CLIENT_STATES;


#ifndef     NX_SMTP_SOURCE_CODE     

/* Define the system API mappings based on the error checking 
   selected by the user.   */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */


#ifdef NX_SMTP_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define   nxd_smtp_client_create                 _nxd_smtp_client_create
#define   nx_smtp_client_delete                  _nx_smtp_client_delete
#define   nx_smtp_mail_send                      _nx_smtp_mail_send
#else

/* Services with error checking.  */
#define nxd_smtp_client_create                   _nxde_smtp_client_create
#define nx_smtp_client_delete                    _nxe_smtp_client_delete
#define nx_smtp_mail_send                        _nxe_smtp_mail_send


#endif /* NX_SMTP_DISABLE_ERROR_CHECKING */


/* Define the prototypes accessible to the application software.  */
UINT    nxd_smtp_client_create(NX_SMTP_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *client_packet_pool_ptr, 
                               CHAR *username, CHAR *password, CHAR *from_address,
                               CHAR *client_domain, UINT authentication_type, 
                               NXD_ADDRESS *server_address, UINT port);

UINT    nx_smtp_client_delete (NX_SMTP_CLIENT *client_ptr);
UINT    nx_smtp_mail_send(NX_SMTP_CLIENT *client_ptr, CHAR *recipient_address, UINT priority, 
                          CHAR *subject, CHAR *mail_body, UINT mail_body_length);
    

#else  /*  NX_SMTP_SOURCE_CODE */


/* SMTP source code is being compiled, do not perform any API mapping.  */

UINT  _nxd_smtp_client_create(NX_SMTP_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *client_packet_pool_ptr, 
                              CHAR *username, CHAR *password, CHAR *from_address,
                              CHAR *client_domain, UINT authentication_type, 
                              NXD_ADDRESS *server_address, UINT port);
UINT  _nxde_smtp_client_create(NX_SMTP_CLIENT *client_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *client_packet_pool_ptr, 
                               CHAR *username, CHAR *password, CHAR *from_address,
                               CHAR *client_domain, UINT authentication_type, 
                               NXD_ADDRESS *server_address, UINT port);

UINT    _nx_smtp_client_delete (NX_SMTP_CLIENT *client_ptr);
UINT    _nxe_smtp_client_delete (NX_SMTP_CLIENT *client_ptr);

UINT    _nx_smtp_mail_send(NX_SMTP_CLIENT *client_ptr, CHAR *recipient_address, UINT priority, 
                           CHAR *subject, CHAR *mail_body, UINT mail_body_length);

UINT    _nxe_smtp_mail_send(NX_SMTP_CLIENT *client_ptr, CHAR *recipient_address, UINT priority, 
                            CHAR *subject, CHAR *mail_body, UINT mail_body_length);





#endif   /*  NX_SMTP_SOURCE_CODE */

/* If a C++ compiler is being used....*/
#ifdef   __cplusplus
}
#endif


#endif /* NXD_SMTP_CLIENT_H  */
