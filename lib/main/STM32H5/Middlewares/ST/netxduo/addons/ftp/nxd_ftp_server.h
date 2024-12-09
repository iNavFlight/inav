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
/**   File Transfer Protocol (FTP)                                        */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nxd_ftp_server.h                                    PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Duo File Transfer Protocol (FTP over     */ 
/*    IPv6) component, including all data types and external references.  */ 
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of clearing */
/*                                            data socket, included       */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_FTP_SERVER_H
#define NXD_FTP_SERVER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"


#ifndef      NX_FTP_NO_FILEX
#include    "fx_api.h"
#else
#include    "filex_stub.h"
#endif


/* Define the FTP Server ID.  */
                                             
#define NXD_FTP_SERVER_ID                           0x46545201UL


/* Define the maximum number of clients the FTP Server can accommodate.  */

#ifndef NX_FTP_MAX_CLIENTS
#define NX_FTP_MAX_CLIENTS                  4
#endif


/* Define FTP TCP socket create options.  */


#ifndef NX_FTP_CONTROL_TOS
#define NX_FTP_CONTROL_TOS                  NX_IP_NORMAL
#endif

#ifndef NX_FTP_DATA_TOS
#define NX_FTP_DATA_TOS                     NX_IP_NORMAL
#endif

#ifndef NX_FTP_FRAGMENT_OPTION
#define NX_FTP_FRAGMENT_OPTION              NX_DONT_FRAGMENT
#endif  

#ifndef NX_FTP_CONTROL_WINDOW_SIZE
#define NX_FTP_CONTROL_WINDOW_SIZE          400
#endif

#ifndef NX_FTP_DATA_WINDOW_SIZE
#define NX_FTP_DATA_WINDOW_SIZE             2048  
#endif


#ifndef NX_FTP_TIME_TO_LIVE
#define NX_FTP_TIME_TO_LIVE                 0x80
#endif

#ifndef NX_FTP_SERVER_TIMEOUT       
#define NX_FTP_SERVER_TIMEOUT               NX_IP_PERIODIC_RATE
#endif

#ifndef NX_FTP_SERVER_PRIORITY
#define NX_FTP_SERVER_PRIORITY              16
#endif

#ifndef NX_FTP_SERVER_TIME_SLICE
#define NX_FTP_SERVER_TIME_SLICE            2
#endif

#ifndef NX_FTP_USERNAME_SIZE
#define NX_FTP_USERNAME_SIZE                20
#endif

#ifndef NX_FTP_PASSWORD_SIZE
#define NX_FTP_PASSWORD_SIZE                20
#endif

#ifndef NX_FTP_ACTIVITY_TIMEOUT       
#define NX_FTP_ACTIVITY_TIMEOUT             240         /* Seconds allowed with no activity                    */ 
#endif

#ifndef NX_FTP_TIMEOUT_PERIOD
#define NX_FTP_TIMEOUT_PERIOD               60          /* Number of seconds to check                          */
#endif

#ifndef NX_PHYSICAL_TRAILER
#define NX_PHYSICAL_TRAILER                 4           /* Number of bytes to reserve at the end of buffer     */ 
#endif

/* Define the FTP data port retry parameters.  */

#ifndef NX_FTP_SERVER_RETRY_SECONDS
#define NX_FTP_SERVER_RETRY_SECONDS         2           /* 2 second initial timeout                            */ 
#endif

#ifndef NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH
#define NX_FTP_SERVER_TRANSMIT_QUEUE_DEPTH  20          /* Maximum of 20 queued transmit packets               */ 
#endif

#ifndef NX_FTP_SERVER_RETRY_MAX
#define NX_FTP_SERVER_RETRY_MAX             10          /* Maximum of 10 retries per packet                    */ 
#endif

#ifndef NX_FTP_SERVER_RETRY_SHIFT
#define NX_FTP_SERVER_RETRY_SHIFT           1           /* Every retry is twice as long                        */
#endif

/* The minimum FTP Server packet size must accomodate the largest expected packet sent e.g. the LIST command: 
   The filename can be up to 256 bytes, plus file info (which has a fixed size of 52 bytes).
   Note the 256 value is the assumed max size of a filename, defined by FX_MAX_LONG_NAME_LEN.
                                                                                            .
   File download (data) packets can be split up over multiple packets, but commands should be in one packet. */
                                                                                            
#ifndef NX_FTP_SERVER_MIN_PACKET_PAYLOAD
#define NX_FTP_SERVER_MIN_PACKET_PAYLOAD   (256 + 52 + NX_TCP_PACKET + NX_PHYSICAL_TRAILER)  
#endif


/* Define open types.  */

#define NX_FTP_OPEN_FOR_READ                0x01        /* FTP file open for reading                           */
#define NX_FTP_OPEN_FOR_WRITE               0x02        /* FTP file open for writing                           */ 


/* Define transfer modes. Note: just support stream mode and block mode yet.  */

#define NX_FTP_TRANSFER_MODE_STREAM         0           /* FTP stream transfer mode                            */
#define NX_FTP_TRANSFER_MODE_BLOCK          1           /* FTP block transfer mode                             */
#define NX_FTP_TRANSFER_MODE_COMPRESSED     2           /* FTP compressed transfer mode                        */


/* Define Server thread events.  */

#define NX_FTP_SERVER_CONNECT               0x01        /* FTP connection is present                           */
#define NX_FTP_SERVER_DATA_DISCONNECT       0x02        /* FTP data disconnection is present                   */ 
#define NX_FTP_SERVER_COMMAND               0x04        /* FTP client command is present                       */ 
#define NX_FTP_SERVER_DATA                  0x08        /* FTP client data is present                          */ 
#define NX_FTP_SERVER_ACTIVITY_TIMEOUT      0x10        /* FTP activity timeout check                          */ 
#define NX_FTP_SERVER_CONTROL_DISCONNECT    0x20        /* FTP client disconnect of control socket             */ 
#define NX_FTP_STOP_EVENT                   0x40        /* FTP stop service                                    */ 
#define NX_FTP_ANY_EVENT                    0xFF        /* Any FTP event                                       */


/* Define return code constants.  */

#define NX_FTP_ERROR                        0xD0        /* Generic FTP internal error - deprecated             */ 
#define NX_FTP_TIMEOUT                      0xD1        /* FTP timeout occurred                                */ 
#define NX_FTP_FAILED                       0xD2        /* FTP error                                           */ 
#define NX_FTP_NOT_CONNECTED                0xD3        /* FTP not connected error                             */ 
#define NX_FTP_NOT_DISCONNECTED             0xD4        /* FTP not disconnected error                          */ 
#define NX_FTP_NOT_OPEN                     0xD5        /* FTP not opened error                                */ 
#define NX_FTP_NOT_CLOSED                   0xD6        /* FTP not closed error                                */ 
#define NX_FTP_END_OF_FILE                  0xD7        /* FTP end of file status                              */ 
#define NX_FTP_END_OF_LISTING               0xD8        /* FTP end of directory listing status                 */ 
#define NX_FTP_EXPECTED_1XX_CODE            0xD9        /* Expected a 1xx response from server                 */
#define NX_FTP_EXPECTED_2XX_CODE            0xDA        /* Expected a 2xx response from server                 */
#define NX_FTP_EXPECTED_22X_CODE            0xDB        /* Expected a 22x response from server                 */
#define NX_FTP_EXPECTED_23X_CODE            0xDC        /* Expected a 23x response from server                 */
#define NX_FTP_EXPECTED_3XX_CODE            0xDD        /* Expected a 3xx response from server                 */
#define NX_FTP_EXPECTED_33X_CODE            0xDE        /* Expected a 33x response from server                 */
#define NX_FTP_INVALID_NUMBER               0xDF        /* Extraced an invalid number from server response     */
#define NX_FTP_INVALID_ADDRESS              0x1D0       /* Invalid IP address parsed from FTP command          */
#define NX_FTP_INVALID_COMMAND              0x1D1       /* Invalid FTP command (bad syntax, unknown command)   */
#define NX_FTP_INVALID_LOGIN                0x1D2       /* Unable to perform successful user login             */
#define NX_FTP_INSUFFICIENT_PACKET_PAYLOAD  0x1D3       /* Packet payload less than the max length filename    */
#define NX_FTP_SERVER_INVALID_SIZE          0x1D4       /* Invalid FTP file size                               */
#define NX_FTP_SERVER_END_OF_BLOCK          0x1D5       /* FTP end of block                                    */

/* Define FTP connection states.  */

#define NX_FTP_STATE_NOT_CONNECTED          1           /* FTP not connected                                   */ 
#define NX_FTP_STATE_CONNECTED              2           /* FTP connected                                       */ 
#define NX_FTP_STATE_OPEN                   3           /* FTP file open for reading                           */ 
#define NX_FTP_STATE_WRITE_OPEN             4           /* FTP file open for writing                           */ 


/* Define the FTP Server TCP port numbers.  */

#define NX_FTP_SERVER_CONTROL_PORT          21          /* Control Port for FTP server                         */
#define NX_FTP_SERVER_DATA_PORT             20          /* Data Port for FTP server                            */ 

/* Define the size for buffer to store an IPv6 address represented in ASCII. */
#define NX_FTP_IPV6_ADDRESS_BUFSIZE         60

/* Define the FTP basic commands.  The ASCII command will be parsed and converted to the numerical 
   representation shown below.  */

#define NX_FTP_NOOP                         0
#define NX_FTP_USER                         1
#define NX_FTP_PASS                         2
#define NX_FTP_QUIT                         3
#define NX_FTP_RETR                         4
#define NX_FTP_STOR                         5
#define NX_FTP_RNFR                         6
#define NX_FTP_RNTO                         7
#define NX_FTP_DELE                         8
#define NX_FTP_RMD                          9
#define NX_FTP_MKD                          10
#define NX_FTP_NLST                         11
#define NX_FTP_PORT                         12
#define NX_FTP_CWD                          13
#define NX_FTP_PWD                          14
#define NX_FTP_TYPE                         15
#define NX_FTP_LIST                         16
#define NX_FTP_CDUP                         17
#define NX_FTP_INVALID                      18
#define NX_FTP_EPRT                         19
#define NX_FTP_PASV                         20
#define NX_FTP_EPSV                         21
#define NX_FTP_MODE                         22



/* Define the per client request structure for the FTP Server data structure.  */

typedef struct NX_FTP_CLIENT_REQUEST_STRUCT
{
    UINT            nx_ftp_client_request_data_port;                /* Client's data port                  */ 
    UINT            nx_ftp_client_request_ip_type;                  /* Client IP type (IPv4 or IPv6)       */
    UINT            nx_ftp_client_request_open_type;                /* Open type of client request         */
    UINT            nx_ftp_client_request_authenticated;            /* Authenticated flag                  */ 
    CHAR            nx_ftp_client_request_read_only;                /* Read-only flag                      */
    CHAR            nx_ftp_client_request_transfer_type;            /* FTP Data transfer type              */ 
    CHAR            nx_ftp_client_request_login;                    /* FTP Login flag                      */ 
    CHAR            nx_ftp_client_request_passive_transfer_enabled; /* Client enabled for passive transfer */
    UINT            nx_ftp_client_request_transfer_mode;            /* Client transfer mode                */
    ULONG           nx_ftp_client_request_activity_timeout;         /* Timeout for client activity         */ 
    ULONG           nx_ftp_client_request_total_bytes;              /* Total bytes read or written         */ 
    ULONG           nx_ftp_client_request_block_bytes;              /* Block bytes in block mode           */ 
    CHAR            nx_ftp_client_request_username[NX_FTP_USERNAME_SIZE];
    CHAR            nx_ftp_client_request_password[NX_FTP_PASSWORD_SIZE];
    NX_PACKET       *nx_ftp_client_request_packet;                  /* Previous request packet             */ 
    FX_FILE         nx_ftp_client_request_file;                     /* File control block                  */ 
    FX_LOCAL_PATH   nx_ftp_client_local_path;                       /* Local path control block            */ 
    NX_TCP_SOCKET   nx_ftp_client_request_control_socket;           /* Client control socket               */ 
    NX_TCP_SOCKET   nx_ftp_client_request_data_socket;              /* Client data socket                  */ 
} NX_FTP_CLIENT_REQUEST;


/* Define the FTP Server data structure.  */

typedef struct NX_FTP_SERVER_STRUCT 
{
    ULONG           nx_ftp_server_id;                               /* FTP Server ID                       */
    CHAR           *nx_ftp_server_name;                             /* Name of this FTP server             */
    NX_IP          *nx_ftp_server_ip_ptr;                           /* Pointer to associated IP structure  */ 

    NX_PACKET_POOL *nx_ftp_server_packet_pool_ptr;                  /* Pointer to FTP server packet pool   */ 
    FX_MEDIA       *nx_ftp_server_media_ptr;                        /* Pointer to media control block      */ 
    ULONG           nx_ftp_server_connection_requests;              /* Number of connection requests       */ 
    ULONG           nx_ftp_server_disconnection_requests;           /* Number of disconnection requests    */ 
    ULONG           nx_ftp_server_login_errors;                     /* Number of login errors              */ 
    ULONG           nx_ftp_server_authentication_errors;            /* Number of access w/o authentication */ 
    ULONG           nx_ftp_server_total_bytes_sent;                 /* Number of total bytes sent          */ 
    ULONG           nx_ftp_server_total_bytes_received;             /* Number of total bytes received      */ 
    ULONG           nx_ftp_server_unknown_commands;                 /* Number of unknown commands received */ 
    ULONG           nx_ftp_server_allocation_errors;                /* Number of allocation errors         */ 
    ULONG           nx_ftp_server_relisten_errors;                  /* Number of relisten errors           */ 
    ULONG           nx_ftp_server_activity_timeouts;                /* Number of activity timeouts         */ 
    UINT            nx_ftp_server_data_port;                        /* Port the data socket binds          */ 
    NX_FTP_CLIENT_REQUEST                                           /* FTP client request array            */ 
                    nx_ftp_server_client_list[NX_FTP_MAX_CLIENTS]; 
    TX_EVENT_FLAGS_GROUP
                    nx_ftp_server_event_flags;                      /* FTP server thread events            */ 
    TX_TIMER        nx_ftp_server_timer;                            /* FTP server activity timeout timer   */ 
    TX_THREAD       nx_ftp_server_thread;                           /* FTP server thread                   */ 
    UINT            (*nx_ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info); 
    UINT            (*nx_ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
#ifndef NX_DISABLE_IPV4
    UINT            (*nx_ftp_login_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info); 
    UINT            (*nx_ftp_logout_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
#endif /* NX_DISABLE_IPV4 */

    struct NX_FTP_SERVER_STRUCT
                    *nx_ftp_next_server_ptr,
                    *nx_ftp_previous_server_ptr;
} NX_FTP_SERVER;


#ifndef NX_FTP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_ftp_server_create                        _nx_ftp_server_create 
#define nxd_ftp_server_create                       _nxd_ftp_server_create
#define nx_ftp_server_delete                        _nx_ftp_server_delete
#define nx_ftp_server_start                         _nx_ftp_server_start
#define nx_ftp_server_stop                          _nx_ftp_server_stop

#else

/* Services with error checking.  */

#define nx_ftp_server_create                        _nxe_ftp_server_create 
#define nxd_ftp_server_create                       _nxde_ftp_server_create
#define nx_ftp_server_delete                        _nxe_ftp_server_delete
#define nx_ftp_server_start                         _nxe_ftp_server_start
#define nx_ftp_server_stop                          _nxe_ftp_server_stop

#endif

/* Define the prototypes accessible to the application software.  */
UINT        nx_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)); 
UINT        nxd_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info));
UINT        nx_ftp_server_delete(NX_FTP_SERVER *ftp_server_ptr);
UINT        nx_ftp_server_start(NX_FTP_SERVER *ftp_server_ptr);
UINT        nx_ftp_server_stop(NX_FTP_SERVER *ftp_server_ptr);

#else

/* FTP source code is being compiled, do not perform any API mapping.  */
UINT        _nxe_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info));
UINT        _nx_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout_ipv4)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)); 
UINT        _nxde_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info));
UINT        _nxd_ftp_server_create(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info));
UINT        _nxe_ftp_server_delete(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nx_ftp_server_delete(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nxe_ftp_server_start(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nx_ftp_server_start(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nxe_ftp_server_stop(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nx_ftp_server_stop(NX_FTP_SERVER *ftp_server_ptr);
       
/* Internal FTP functions. */        
UINT        _nx_ftp_server_create_internal(NX_FTP_SERVER *ftp_server_ptr, CHAR *ftp_server_name, NX_IP *ip_ptr, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                UINT (*ftp_login)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info),
                UINT (*ftp_logout)(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info));
VOID        _nx_ftp_server_response(NX_TCP_SOCKET *socket, NX_PACKET *packet_ptr, CHAR *reply_code, CHAR *message);
VOID        _nx_ftp_server_directory_response(NX_TCP_SOCKET *socket, NX_PACKET *packet_ptr, CHAR *reply_code, CHAR *message, CHAR *directory);
VOID        _nx_ftp_server_thread_entry(ULONG ftp_server_address);
VOID        _nx_ftp_server_command_process(NX_FTP_SERVER *ftp_server_ptr);
VOID        _nx_ftp_server_connect_process(NX_FTP_SERVER *ftp_server_ptr);
VOID        _nx_ftp_server_command_present(NX_TCP_SOCKET *control_socket_ptr);
VOID        _nx_ftp_server_connection_present(NX_TCP_SOCKET *control_socket_ptr, UINT port);
VOID        _nx_ftp_server_data_disconnect(NX_TCP_SOCKET *data_socket_ptr);
VOID        _nx_ftp_server_data_disconnect_process(NX_FTP_SERVER *ftp_server_ptr);
VOID        _nx_ftp_server_data_present(NX_TCP_SOCKET *data_socket_ptr);
VOID        _nx_ftp_server_data_process(NX_FTP_SERVER *ftp_server_ptr);
UINT        _nx_ftp_server_parse_command(NX_PACKET *packet_ptr);
VOID        _nx_ftp_server_timeout(ULONG ftp_server_address);
VOID        _nx_ftp_server_timeout_processing(NX_FTP_SERVER *ftp_server_ptr);
VOID        _nx_ftp_server_control_disconnect(NX_TCP_SOCKET *control_socket_ptr);
VOID        _nx_ftp_server_control_disconnect_processing(NX_FTP_SERVER *ftp_server_ptr);
VOID        _nx_ftp_server_data_socket_cleanup(NX_FTP_SERVER *ftp_server_ptr, NX_FTP_CLIENT_REQUEST *client_req_ptr);

#endif  /* NX_FTP_SOURCE_CODE */

/* Internal functions. */
#ifdef FEATURE_NX_IPV6
UINT        _nx_ftp_utility_parse_IPv6_address(CHAR *buffer_ptr, UINT buffer_length, NXD_ADDRESS *ipduo_address);
UINT        _nx_ftp_utility_parse_port_number(CHAR *buffer_ptr, UINT buffer_length, UINT *portnumber);
UINT        _nx_ftp_server_utility_fill_port_number(CHAR *buffer_ptr, UINT port_number);
#endif

UINT        _nx_ftp_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_FTP_CLIENT_REQUEST *client_request_ptr, NX_PACKET **packet_ptr, UINT packet_type, UINT wait_option);
VOID        _nx_ftp_server_block_size_get(NX_FTP_SERVER *ftp_server_ptr, UINT ftp_command, CHAR *filename, ULONG *block_size);
UINT        _nx_ftp_server_block_header_send(NX_PACKET_POOL *pool_ptr, NX_FTP_CLIENT_REQUEST *client_request_ptr, ULONG block_size);
UINT        _nx_ftp_server_block_header_retrieve(NX_FTP_CLIENT_REQUEST *client_request_ptr, NX_PACKET *packet_ptr);


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif    /* NXD_FTP_SERVER_H */
