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
/*    nxd_ftp_client.h                                    PORTABLE C      */ 
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_FTP_CLIENT_H
#define NXD_FTP_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"


/* Define the FTP Client ID.  */

                       
#define NXD_FTP_CLIENT_ID                           0x46545200UL


/* Define the maximum number of clients the FTP Server can accommodate.  */

#ifndef NX_FTP_MAX_CLIENTS
#define NX_FTP_MAX_CLIENTS                  4
#endif


/* Define FTP TCP socket create options.  Normally any TCP port will do but this
   gives the application a means to specify a source port.  */

#ifndef NX_FTP_CLIENT_SOURCE_PORT
#define NX_FTP_CLIENT_SOURCE_PORT           NX_ANY_PORT
#endif

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

#ifndef NX_FTP_USERNAME_SIZE
#define NX_FTP_USERNAME_SIZE                20
#endif

#ifndef NX_FTP_PASSWORD_SIZE
#define NX_FTP_PASSWORD_SIZE                20
#endif

#ifndef NX_FTP_TIMEOUT_PERIOD
#define NX_FTP_TIMEOUT_PERIOD               60          /* Number of seconds to check                          */
#endif



/* Define open types.  */

#define NX_FTP_OPEN_FOR_READ                0x01        /* FTP file open for reading                           */
#define NX_FTP_OPEN_FOR_WRITE               0x02        /* FTP file open for writing                           */ 


/* Define transfer modes. Note: just support stream mode and block mode yet.  */

#define NX_FTP_TRANSFER_MODE_STREAM         0           /* FTP stream transmission mode                        */
#define NX_FTP_TRANSFER_MODE_BLOCK          1           /* FTP block transmission mode                         */
#define NX_FTP_TRANSFER_MODE_COMPRESSED     2           /* FTP compressed transmission mode                    */


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
#define NX_FTP_CLIENT_INVALID_SIZE          0x1D2       /* Invalid FTP file size                               */
#define NX_FTP_CLIENT_FILE_SIZE_ALREADY_SET 0x1D3       /* The file size is already set                        */
#define NX_FTP_CLIENT_NOT_BLOCK_MODE        0x1D4       /* Block mode is not enabled                           */
#define NX_FTP_CLIENT_END_OF_BLOCK          0x1D5       /* FTP end of block                                    */


/* Define FTP connection states.  */

#define NX_FTP_STATE_NOT_CONNECTED          1           /* FTP not connected                                   */ 
#define NX_FTP_STATE_CONNECTED              2           /* FTP connected                                       */ 
#define NX_FTP_STATE_OPEN                   3           /* FTP file open for reading                           */ 
#define NX_FTP_STATE_WRITE_OPEN             4           /* FTP file open for writing                           */ 


/* Define the FTP Server TCP port numbers.  */

#define NX_FTP_SERVER_CONTROL_PORT          21          /* Control Port for FTP server                         */
#define NX_FTP_SERVER_DATA_PORT             20          /* Data Port for FTP server in active transfer mode    */ 

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



/* Define the basic FTP Client data structure.  */

typedef struct NX_FTP_CLIENT_STRUCT 
{
    ULONG           nx_ftp_client_id;                               /* FTP Client ID                       */
    CHAR           *nx_ftp_client_name;                             /* Name of this FTP client             */
    NX_IP          *nx_ftp_client_ip_ptr;                           /* Pointer to associated IP structure  */ 
    NX_PACKET_POOL *nx_ftp_client_packet_pool_ptr;                  /* Pointer to FTP client packet pool   */ 
    ULONG           nx_ftp_client_server_ip;                        /* Server's IP address                 */ 
    UINT            nx_ftp_client_state;                            /* State of FTP client                 */ 
    NX_TCP_SOCKET   nx_ftp_client_control_socket;                   /* Client FTP control socket           */
    NX_TCP_SOCKET   nx_ftp_client_data_socket;                      /* Client FTP data transfer socket     */ 
    UINT            nx_ftp_client_data_port;                        /* Port the Client data socket binds   */
    UINT            nx_ftp_client_passive_transfer_enabled;         /* Client enabled for passive transfer */
    UINT            nx_ftp_client_transfer_mode;                    /* Client transfer mode                */
    ULONG           nx_ftp_client_block_total_size;                 /* Total size of data in block mode    */
    ULONG           nx_ftp_client_block_remaining_size;             /* Remaining size of data in block mode*/
} NX_FTP_CLIENT;


#ifndef NX_FTP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_ftp_client_connect                       _nx_ftp_client_connect  
#define nxd_ftp_client_connect                      _nxd_ftp_client_connect
#define nx_ftp_client_create                        _nx_ftp_client_create
#define nx_ftp_client_delete                        _nx_ftp_client_delete
#define nx_ftp_client_directory_create              _nx_ftp_client_directory_create
#define nx_ftp_client_directory_default_set         _nx_ftp_client_directory_default_set
#define nx_ftp_client_directory_delete              _nx_ftp_client_directory_delete
#define nx_ftp_client_directory_listing_get         _nx_ftp_client_directory_listing_get
#define nx_ftp_client_directory_listing_continue    _nx_ftp_client_directory_listing_continue
#define nx_ftp_client_disconnect                    _nx_ftp_client_disconnect
#define nx_ftp_client_file_close                    _nx_ftp_client_file_close
#define nx_ftp_client_file_delete                   _nx_ftp_client_file_delete
#define nx_ftp_client_file_open                     _nx_ftp_client_file_open
#define nx_ftp_client_file_read                     _nx_ftp_client_file_read
#define nx_ftp_client_file_rename                   _nx_ftp_client_file_rename
#define nx_ftp_client_file_write                    _nx_ftp_client_file_write
#define nx_ftp_client_file_size_set                 _nx_ftp_client_file_size_set
#define nx_ftp_client_passive_mode_set              _nx_ftp_client_passive_mode_set
#define nx_ftp_client_transfer_mode_set             _nx_ftp_client_transfer_mode_set

#else

/* Services with error checking.  */

#define nx_ftp_client_connect                       _nxe_ftp_client_connect  
#define nxd_ftp_client_connect                      _nxde_ftp_client_connect
#define nx_ftp_client_create                        _nxe_ftp_client_create
#define nx_ftp_client_delete                        _nxe_ftp_client_delete
#define nx_ftp_client_directory_create              _nxe_ftp_client_directory_create
#define nx_ftp_client_directory_default_set         _nxe_ftp_client_directory_default_set
#define nx_ftp_client_directory_delete              _nxe_ftp_client_directory_delete  
#define nx_ftp_client_directory_listing_get         _nxe_ftp_client_directory_listing_get
#define nx_ftp_client_directory_listing_continue    _nxe_ftp_client_directory_listing_continue
#define nx_ftp_client_disconnect                    _nxe_ftp_client_disconnect
#define nx_ftp_client_file_close                    _nxe_ftp_client_file_close
#define nx_ftp_client_file_delete                   _nxe_ftp_client_file_delete
#define nx_ftp_client_file_open                     _nxe_ftp_client_file_open
#define nx_ftp_client_file_read                     _nxe_ftp_client_file_read
#define nx_ftp_client_file_rename                   _nxe_ftp_client_file_rename
#define nx_ftp_client_file_write                    _nxe_ftp_client_file_write
#define nx_ftp_client_file_size_set                 _nxe_ftp_client_file_size_set
#define nx_ftp_client_passive_mode_set              _nxe_ftp_client_passive_mode_set
#define nx_ftp_client_transfer_mode_set             _nxe_ftp_client_transfer_mode_set

#endif

/* Define the prototypes accessible to the application software.  */

UINT        nx_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, ULONG server_ip, CHAR *username, CHAR *password, ULONG wait_option);
UINT        nxd_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ipduo, CHAR *username, CHAR *password, ULONG wait_option);
UINT        nx_ftp_client_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *ftp_client_name, NX_IP *ip_ptr, ULONG window_size, NX_PACKET_POOL *pool_ptr);
UINT        nx_ftp_client_delete(NX_FTP_CLIENT *ftp_client_ptr);
UINT        nx_ftp_client_directory_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        nx_ftp_client_directory_default_set(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, ULONG wait_option);
UINT        nx_ftp_client_directory_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        nx_ftp_client_directory_listing_get(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        nx_ftp_client_directory_listing_continue(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        nx_ftp_client_disconnect(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        nx_ftp_client_file_close(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        nx_ftp_client_file_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, ULONG wait_option);
UINT        nx_ftp_client_file_open(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, UINT open_type, ULONG wait_option);
UINT        nx_ftp_client_file_read(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        nx_ftp_client_file_rename(NX_FTP_CLIENT *ftp_ptr, CHAR *filename, CHAR *new_filename, ULONG wait_option);
UINT        nx_ftp_client_file_write(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        nx_ftp_client_file_size_set(NX_FTP_CLIENT *ftp_client_ptr, ULONG file_size);
UINT        nx_ftp_client_passive_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT passive_mode_enabled);
UINT        nx_ftp_client_transfer_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT transfer_mode);

#else

/* FTP source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, ULONG server_ip, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nx_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, ULONG server_ip, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxde_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ipduo, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxd_ftp_client_connect(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ipduo, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nxe_ftp_client_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *ftp_client_name, NX_IP *ip_ptr, ULONG window_size, NX_PACKET_POOL *pool_ptr);
UINT        _nx_ftp_client_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *ftp_client_name, NX_IP *ip_ptr, ULONG window_size, NX_PACKET_POOL *pool_ptr);
UINT        _nxe_ftp_client_delete(NX_FTP_CLIENT *ftp_client_ptr);
UINT        _nx_ftp_client_delete(NX_FTP_CLIENT *ftp_client_ptr);
UINT        _nxe_ftp_client_directory_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        _nx_ftp_client_directory_create(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        _nxe_ftp_client_directory_default_set(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, ULONG wait_option);
UINT        _nx_ftp_client_directory_default_set(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, ULONG wait_option);
UINT        _nxe_ftp_client_directory_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        _nx_ftp_client_directory_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_name, ULONG wait_option);
UINT        _nxe_ftp_client_directory_listing_get(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_ftp_client_directory_listing_get(NX_FTP_CLIENT *ftp_client_ptr, CHAR *directory_path, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_directory_listing_continue(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_ftp_client_directory_listing_continue(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_disconnect(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nx_ftp_client_disconnect(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_file_close(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nx_ftp_client_file_close(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_file_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, ULONG wait_option);
UINT        _nx_ftp_client_file_delete(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, ULONG wait_option);
UINT        _nxe_ftp_client_file_open(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, UINT open_type, ULONG wait_option);
UINT        _nx_ftp_client_file_open(NX_FTP_CLIENT *ftp_client_ptr, CHAR *file_name, UINT open_type, ULONG wait_option);
UINT        _nxe_ftp_client_file_read(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_ftp_client_file_read(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_file_rename(NX_FTP_CLIENT *ftp_client_ptr, CHAR *filename, CHAR *new_filename, ULONG wait_option);
UINT        _nx_ftp_client_file_rename(NX_FTP_CLIENT *ftp_ptr, CHAR *filename, CHAR *new_filename, ULONG wait_option);
UINT        _nxe_ftp_client_file_write(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        _nx_ftp_client_file_write(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT        _nxe_ftp_client_file_size_set(NX_FTP_CLIENT *ftp_client_ptr, ULONG file_size);
UINT        _nx_ftp_client_file_size_set(NX_FTP_CLIENT *ftp_client_ptr, ULONG file_size);
UINT        _nxe_ftp_client_passive_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT passive_mode_enabled);
UINT        _nx_ftp_client_passive_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT passive_mode_enabled);
UINT        _nxe_ftp_client_transfer_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT transfer_mode);
UINT        _nx_ftp_client_transfer_mode_set(NX_FTP_CLIENT *ftp_client_ptr, UINT transfer_mode);

#endif   /* NX_FTP_SOURCE_CODE */

/* Internal functions. */
VOID        _nx_ftp_client_data_disconnect(NX_TCP_SOCKET *data_socket_ptr);
UINT        _nx_ftp_client_packet_allocate(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT        _nx_ftp_client_active_transfer_setup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nx_ftp_client_passive_transfer_setup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
VOID        _nx_ftp_client_data_socket_cleanup(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nx_ftp_client_block_mode_send(NX_FTP_CLIENT *ftp_client_ptr, ULONG wait_option);
UINT        _nx_ftp_client_block_header_send(NX_FTP_CLIENT *ftp_client_ptr, ULONG block_size);
UINT        _nx_ftp_client_block_header_retrieve(NX_FTP_CLIENT *ftp_client_ptr, NX_PACKET *packet_ptr);
UINT        _nx_ftp_client_connect_internal(NX_FTP_CLIENT *ftp_client_ptr, NXD_ADDRESS *server_ip, CHAR *username, CHAR *password, ULONG wait_option);
UINT        _nx_ftp_utility_convert_IPv6_to_ascii(NX_TCP_SOCKET *tcp_socket_ptr, CHAR *buffer, UINT buffer_length, UINT *size);
UINT        _nx_ftp_utility_convert_number_ascii(ULONG number, CHAR *numstring);
UINT        _nx_ftp_utility_convert_portnumber_ascii(UINT number, CHAR *numstring, UINT *numstring_length);


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_FTP_CLIENT_H */
