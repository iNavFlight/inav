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
/** NetX Duo Component                                                    */
/**                                                                       */
/**   Trivial File Transfer Protocol (TFTP) Server                        */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nxd_tftp_server.h                                   PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Trivial File Transfer Protocol (TFTP)    */ 
/*    Server for NetX Duo.  It supports IPv4 and IPv6 networks.           */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included, along with fx_api.h and fx_port.h.                        */   
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

#ifndef NXD_TFTP_SERVER_H
#define NXD_TFTP_SERVER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the TFTP ID.  */
                            
#define NXD_TFTP_SERVER_ID                          0x54465461UL

#ifndef      NX_TFTP_NO_FILEX
#include    "fx_api.h"
#else
#include    "filex_stub.h"
#endif

/* Define TFTP maximum error string.  */

#ifndef NX_TFTP_ERROR_STRING_MAX
#define NX_TFTP_ERROR_STRING_MAX            64          /* Maximum error sting size   */
#endif


/* Define the maximum number of clients the TFTP Server can accommodate.  */

#ifndef NX_TFTP_MAX_CLIENTS
#define NX_TFTP_MAX_CLIENTS                 10
#endif


/* Define TFTP UDP socket create options.  */

#ifndef NX_TFTP_TYPE_OF_SERVICE
#define NX_TFTP_TYPE_OF_SERVICE             NX_IP_NORMAL
#endif

#ifndef NX_TFTP_FRAGMENT_OPTION
#define NX_TFTP_FRAGMENT_OPTION             NX_DONT_FRAGMENT
#endif  

#ifndef NX_TFTP_TIME_TO_LIVE
#define NX_TFTP_TIME_TO_LIVE                0x80
#endif

#ifndef NX_PHYSICAL_TRAILER        
#define NX_PHYSICAL_TRAILER                 4
#endif

#ifndef NX_TFTP_SERVER_PRIORITY
#define NX_TFTP_SERVER_PRIORITY             16
#endif

#ifndef NX_TFTP_SERVER_TIME_SLICE
#define NX_TFTP_SERVER_TIME_SLICE           2
#endif


/* To enable a retransmission on client requests (e.g. resend
   ACK and data packets, as well apply a timeout on a Client request), 
   define this option. 
#define NX_TFTP_SERVER_RETRANSMIT_ENABLE
*/


#ifdef  NX_TFTP_SERVER_RETRANSMIT_ENABLE

/* Define the timer expiration for updating time remaining on a TFTP Client 
   request activity timeout (in timer ticks). */
#ifndef NX_TFTP_SERVER_TIMEOUT_PERIOD
#define NX_TFTP_SERVER_TIMEOUT_PERIOD      20
#endif 

/* Define the interval before retransmitting an ACK or data packet (in timer ticks).  */
#ifndef NX_TFTP_SERVER_RETRANSMIT_TIMEOUT
#define NX_TFTP_SERVER_RETRANSMIT_TIMEOUT  200
#endif

/* Define the max number of retries retransmitting a data packet or ACK if no response or a previous
   (duplicate) ACK or data packet, respectively, is received. */
#ifndef NX_TFTP_SERVER_MAX_RETRIES
#define NX_TFTP_SERVER_MAX_RETRIES         5
#endif

#else

/* Define the number of instances TFTP server receives a duplicate data or an ACK packet
   without sending an error message. This does not utilize the retransmission timeout
   and is available only if NX_TFTP_SERVER_RETRANSMIT_ENABLE is not defined.  */

#ifndef NX_TFTP_MAX_CLIENT_RETRANSMITS
#define NX_TFTP_MAX_CLIENT_RETRANSMITS     2
#endif

#endif /* NX_TFTP_SERVER_RETRANSMIT_ENABLE */

 
#define NX_TFTP_QUEUE_DEPTH                 5
        
#define NX_TFTP_FILE_TRANSFER_MAX           512         /* 512 byte maximum file transfer                     */


/* Derive the maximum TFTP packet size, including Ethernet, IP and UDP headers, and  
   accounting for potential physical driver needs at the end of the packet.  */

#define NX_TFTP_PACKET_SIZE                (NX_UDP_PACKET + NX_TFTP_FILE_TRANSFER_MAX + NX_PHYSICAL_TRAILER) 


/* Define open types.  */

#define NX_TFTP_OPEN_FOR_READ               0x01        /* TFTP open for reading                                */
#define NX_TFTP_OPEN_FOR_WRITE              0x02        /* TFTP open for writing                                */ 


/* Define TFTP message codes.  */

#define NX_TFTP_CODE_READ                   0x01        /* TFTP read file request                               */ 
#define NX_TFTP_CODE_WRITE                  0x02        /* TFTP write file request                              */ 
#define NX_TFTP_CODE_DATA                   0x03        /* TFTP data packet                                     */ 
#define NX_TFTP_CODE_ACK                    0x04        /* TFTP command/data acknowledgement                    */ 
#define NX_TFTP_CODE_ERROR                  0x05        /* TFTP error message                                   */ 


/* Define TFTP error code constants.  */

#define NX_TFTP_ERROR_NOT_DEFINED           0x00        /* TFTP not defined error code, see error string        */
#define NX_TFTP_ERROR_FILE_NOT_FOUND        0x01        /* TFTP file not found error code                       */ 
#define NX_TFTP_ERROR_ACCESS_VIOLATION      0x02        /* TFTP file access violation error code                */ 
#define NX_TFTP_ERROR_DISK_FULL             0x03        /* TFTP disk full error code                            */ 
#define NX_TFTP_ERROR_ILLEGAL_OPERATION     0x04        /* TFTP illegal operation error code                    */ 
#define NX_TFTP_CODE_ERROR                  0x05        /* TFTP client request received error code from server  */ 
#define NX_TFTP_ERROR_FILE_EXISTS           0x06        /* TFTP file already exists error code                  */ 
#define NX_TFTP_ERROR_NO_SUCH_USER          0x07        /* TFTP no such user error code                         */ 
#define NX_INVALID_TFTP_SERVER_ADDRESS      0x08        /* Invalid TFTP server IP extraced from received packet */
#define NX_TFTP_NO_ACK_RECEIVED             0x09        /* Did not receive TFTP server ACK response             */
#define NX_TFTP_INVALID_BLOCK_NUMBER        0x0A        /* Invalid block number received from Server response   */
#define NX_TFTP_INVALID_ADDRESS_TYPE        0x0B        /* Invalid IP version associated with client data       */
#define NX_TFTP_SESSION_TIMED_OUT           0x0C        /* No response from client or retransmissions limit hit */ 


/* Define offsets into the TFTP message buffer.  */

#define NX_TFTP_CODE_OFFSET                 0           /* Offset to TFTP code in buffer                        */
#define NX_TFTP_FILENAME_OFFSET             2           /* Offset to TFTP filename in message                   */ 
#define NX_TFTP_BLOCK_NUMBER_OFFSET         2           /* Offset to TFTP block number in buffer                */ 
#define NX_TFTP_DATA_OFFSET                 4           /* Offset to TFTP data in buffer                        */ 
#define NX_TFTP_ERROR_CODE_OFFSET           2           /* Offset to TFTP error code                            */ 
#define NX_TFTP_ERROR_STRING_OFFSET         4           /* Offset to TFPT error string                          */ 


/* Define return code constants.  */

#define NX_TFTP_ERROR                       0xC0        /* TFTP internal error                                  */ 
#define NX_TFTP_TIMEOUT                     0xC1        /* TFTP timeout occurred                                */ 
#define NX_TFTP_FAILED                      0xC2        /* TFTP error                                           */ 
#define NX_TFTP_NOT_OPEN                    0xC3        /* TFTP not opened error                                */ 
#define NX_TFTP_NOT_CLOSED                  0xC4        /* TFTP not closed error                                */ 
#define NX_TFTP_END_OF_FILE                 0xC5        /* TFTP end of file error                               */ 
#define NX_TFTP_POOL_ERROR                  0xC6        /* TFTP packet pool size error - less than 560 bytes    */ 


/* Define TFTP connection states.  */

#define NX_TFTP_STATE_NOT_OPEN              0           /* TFTP connection not open                             */ 
#define NX_TFTP_STATE_OPEN                  1           /* TFTP connection open                                 */ 
#define NX_TFTP_STATE_WRITE_OPEN            2           /* TFTP connection open for writing                     */ 
#define NX_TFTP_STATE_END_OF_FILE           3           /* TFTP connection at end of file                       */ 
#define NX_TFTP_STATE_ERROR                 4           /* TFTP error condition                                 */ 
#define NX_TFTP_STATE_FINISHED              5           /* TFTP finished writing condition                      */ 


/* Define TFTP Server events.  */

#define NX_TFTP_SERVER_RECEIVE_EVENT      0x01         /* TFTP received Client packet                           */
#define NX_TFTP_SERVER_TIMER_EVENT        0x02         /* TFTP timer event                                      */ 
#define NX_SERVER_TFTP_ANY_EVENT          0x0F         /* Any event                                             */                           

/* Define the TFTP Server UDP port number */

#define NX_TFTP_SERVER_PORT                 69          

/* Define the per client request structure for the TFTP Server data structure.  */

typedef struct NX_TFTP_CLIENT_REQUEST_STRUCT
{
    UINT            nx_tftp_client_request_port;                    /* Port of client request               */
    NXD_ADDRESS     nx_tftp_client_request_ip_address;              /* IP address of client                 */ 
    USHORT          nx_tftp_client_request_block_number;            /* Block number of file transfer        */ 
    USHORT          nx_tftp_client_request_reserved;                /* Reserved for future use              */ 
    UINT            nx_tftp_client_request_open_type;               /* Open type of client request          */
    ULONG           nx_tftp_client_request_remaining_bytes;         /* Remaining bytes to send              */ 
    UINT            nx_tftp_client_request_exact_fit;               /* Exact fit flag                       */ 
    ULONG           nx_tftp_client_request_last_activity_time;      /* Time of last activity                */ 
    FX_FILE         nx_tftp_client_request_file;                    /* File control block                   */ 
    ULONG           nx_tftp_client_file_size;                       /* Size of file in bytes                */ 
    ULONG           nx_tftp_client_previous_write_size;             /* Size of data in last data packet     */ 
#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    UINT            nx_tftp_client_retransmit_timeout;              /* Time between retransmits from server */ 
    UINT            nx_tftp_client_retransmit_retries;              /* Number of retries on current data,ACK*/
#else
    UINT            nx_tftp_client_request_retransmits;             /* Number of retransmits from client    */
#endif
} NX_TFTP_CLIENT_REQUEST;


/* Define the TFTP Server data structure.  */

typedef struct NX_TFTP_SERVER_STRUCT 
{
    ULONG           nx_tftp_server_id;                              /* TFTP Server ID                       */
    CHAR           *nx_tftp_server_name;                            /* Name of this TFTP client             */
    NX_IP          *nx_tftp_server_ip_ptr;                          /* Pointer to associated IP structure   */ 
    NX_PACKET_POOL *nx_tftp_server_packet_pool_ptr;                 /* Pointer to TFTP server packet pool   */ 
    FX_MEDIA       *nx_tftp_server_media_ptr;                       /* Pointer to media control block       */ 
    ULONG           nx_tftp_server_open_for_write_requests;         /* Number of open for write requests    */ 
    ULONG           nx_tftp_server_open_for_read_requests;          /* Number of open for read requests     */ 
    ULONG           nx_tftp_server_acks_received;                   /* Number of ACKs received              */ 
    ULONG           nx_tftp_server_data_blocks_received;            /* Number of data blocks received       */ 
    ULONG           nx_tftp_server_errors_received;                 /* Number of errors received            */ 
    ULONG           nx_tftp_server_total_bytes_sent;                /* Number of total bytes sent           */ 
    ULONG           nx_tftp_server_total_bytes_received;            /* Number of total bytes received       */ 
    ULONG           nx_tftp_server_unknown_commands;                /* Number of unknown commands received  */ 
    ULONG           nx_tftp_server_allocation_errors;               /* Number of allocation errors          */ 
    ULONG           nx_tftp_server_clients_exceeded_errors;         /* Number of maximum clients errors     */ 
    ULONG           nx_tftp_server_unknown_clients_errors;          /* Number of unknown clients errors     */ 
    UINT            nx_tftp_server_error_code;                      /* Last error code received             */ 
    CHAR            nx_tftp_server_error_string[NX_TFTP_ERROR_STRING_MAX + 1];
    NX_TFTP_CLIENT_REQUEST                                          /* TFTP client request array            */ 
                    nx_tftp_server_client_list[NX_TFTP_MAX_CLIENTS]; 
    NX_UDP_SOCKET   nx_tftp_server_socket;                          /* TFTP Server UDP socket               */ 
    TX_THREAD       nx_tftp_server_thread;                          /* TFTP server thread                   */ 
    TX_EVENT_FLAGS_GROUP
                    nx_tftp_server_event_flags;                     /* TFTP server thread events            */ 
#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
    TX_TIMER        nx_tftp_server_timer;                           /* TFTP server activity timeout timer   */ 
#endif
} NX_TFTP_SERVER;


#ifndef NX_TFTP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */
                                    
/* NetX TFTP services mapped to NetX Duo TFTP services */
#define nx_tftp_server_create            _nxd_tftp_server_create
#define nx_tftp_server_delete            _nxd_tftp_server_delete
#define nx_tftp_server_start             _nxd_tftp_server_start
#define nx_tftp_server_stop              _nxd_tftp_server_stop

/* NetX Duo (IPv4 and IPv6 supported) TFTP services */
#define nxd_tftp_server_create           _nxd_tftp_server_create
#define nxd_tftp_server_delete           _nxd_tftp_server_delete
#define nxd_tftp_server_start            _nxd_tftp_server_start
#define nxd_tftp_server_stop             _nxd_tftp_server_stop

#else

/* Services with error checking.  */
                                                                  
/* NetX TFTP services mapped to NetX Duo TFTP services with error checking*/
#define nx_tftp_server_create            _nxde_tftp_server_create
#define nx_tftp_server_delete            _nxde_tftp_server_delete
#define nx_tftp_server_start             _nxde_tftp_server_start
#define nx_tftp_server_stop              _nxde_tftp_server_stop

/* NetX Duo (IPv4 and IPv6 supported) TFTP services with error checking */
#define nxd_tftp_server_create           _nxde_tftp_server_create
#define nxd_tftp_server_delete           _nxde_tftp_server_delete
#define nxd_tftp_server_start            _nxde_tftp_server_start
#define nxd_tftp_server_stop             _nxde_tftp_server_stop

#endif   /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */
                                                              
UINT        nxd_tftp_server_create(NX_TFTP_SERVER *tftp_server_ptr, CHAR *tftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr);
UINT        nxd_tftp_server_delete(NX_TFTP_SERVER *tftp_server_ptr);
UINT        nxd_tftp_server_start(NX_TFTP_SERVER *tftp_server_ptr);
UINT        nxd_tftp_server_stop(NX_TFTP_SERVER *tftp_server_ptr);

#else

/* TFTP source code is being compiled, do not perform any API mapping.  */
                                                         
UINT        _nxde_tftp_server_create(NX_TFTP_SERVER *tftp_server_ptr, CHAR *tftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr);
UINT        _nxd_tftp_server_create(NX_TFTP_SERVER *tftp_server_ptr, CHAR *tftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr);
UINT        _nxde_tftp_server_delete(NX_TFTP_SERVER *tftp_server_ptr);
UINT        _nxd_tftp_server_delete(NX_TFTP_SERVER *tftp_server_ptr);
UINT        _nxde_tftp_server_start(NX_TFTP_SERVER *tftp_server_ptr);
UINT        _nxd_tftp_server_start(NX_TFTP_SERVER *tftp_server_ptr);
UINT        _nxde_tftp_server_stop(NX_TFTP_SERVER *tftp_server_ptr);
UINT        _nxd_tftp_server_stop(NX_TFTP_SERVER *tftp_server_ptr);

#endif    /* NX_TFTP_SOURCE_CODE */

/* Internal TFTP server functions */

void        _nx_tftp_server_thread_entry(ULONG tftp_server);
void        _nx_tftp_server_open_for_read_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
void        _nx_tftp_server_open_for_write_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
VOID        _nx_tftp_server_data_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
VOID        _nx_tftp_server_ack_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
VOID        _nx_tftp_server_error_process(NX_TFTP_SERVER *server_ptr, NX_PACKET *packet_ptr);
NX_TFTP_CLIENT_REQUEST * _nx_tftp_server_find_client_request(NX_TFTP_SERVER *server_ptr, UINT port, NXD_ADDRESS *ip_address);
VOID        _nx_tftp_server_send_error(NX_TFTP_SERVER *server_ptr, NXD_ADDRESS *ip_address, UINT port, UINT error, CHAR *error_message);
VOID        _nx_tftp_server_data_present(NX_UDP_SOCKET *socket_ptr);
VOID        _nx_tftp_server_process_received_data(NX_TFTP_SERVER *server_ptr);
UINT        _nx_tftp_server_send_data(NX_TFTP_SERVER *server_ptr, NX_TFTP_CLIENT_REQUEST *client_request_ptr, UINT retransmit);
UINT        _nx_tftp_server_send_ack(NX_TFTP_SERVER *server_ptr, NX_TFTP_CLIENT_REQUEST *client_request_ptr, UINT retransmit);
#ifdef NX_TFTP_SERVER_RETRANSMIT_ENABLE
VOID        _nx_tftp_server_timer_process(NX_TFTP_SERVER *server_ptr);
VOID        _nx_tftp_server_timer_entry(ULONG tftp_server_address);
#endif
/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif    /* NXD_TFTP_SERVER_H */
