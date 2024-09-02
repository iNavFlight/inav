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
/**   Trivial File Transfer Protocol (TFTP) Client                        */
/**                                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nxd_tftp_client.h                                   PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Trivial File Transfer Protocol (TFTP)    */ 
/*    Client component, including all data types and external references  */ 
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

#ifndef NXD_TFTP_CLIENT_H
#define NXD_TFTP_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the NetX TFTP CLIENT ID.  */
                                             
#define NXD_TFTP_CLIENT_ID                          0x54465460UL


/* Define TFTP maximum error string.  */

#ifndef NX_TFTP_ERROR_STRING_MAX
#define NX_TFTP_ERROR_STRING_MAX            64          /* Maximum error sting size   */
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


/* If the host application wishes to specify a source port, define it here. The default is to let
   NetX choose the TFTP Client source port. */
#ifndef NX_TFTP_SOURCE_PORT
#define NX_TFTP_SOURCE_PORT                NX_ANY_PORT
#endif

#define NX_TFTP_QUEUE_DEPTH                 5
        
#define NX_TFTP_FILE_TRANSFER_MAX           512         /* 512 byte maximum file transfer                      */


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
#define NX_TFTP_INVALID_SERVER_ADDRESS      0x08        /* Invalid TFTP server IP extracted from received packet*/
#define NX_TFTP_NO_ACK_RECEIVED             0x09        /* Did not receive TFTP server ACK response             */
#define NX_TFTP_INVALID_BLOCK_NUMBER        0x0A        /* Invalid block number received from Server response   */
#define NX_TFTP_INVALID_INTERFACE           0x0B        /* Invalid interface for TFTP Client                    */
                                                        /* (or multihome not supported)                         */
#define NX_TFTP_INVALID_IP_VERSION          0x0C        /* Invalid or unsupported IP version specified          */


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


/* Define the TFTP Server UDP port number */

#define NX_TFTP_SERVER_PORT                 69          /* Port for TFTP server                                 */


/* Define the basic TFTP Client data structure.  */

typedef struct NX_TFTP_CLIENT_STRUCT 
{
    ULONG           nx_tftp_client_id;                              /* TFTP Client ID                       */
    CHAR           *nx_tftp_client_name;                            /* Name of this TFTP client             */
    UINT            nx_tftp_client_interface_index;                 /* Index specifying network interface   */
    NX_IP          *nx_tftp_client_ip_ptr;                          /* Pointer to associated IP structure   */ 
    NX_PACKET_POOL *nx_tftp_client_packet_pool_ptr;                 /* Pointer to TFTP client packet pool   */ 
    NXD_ADDRESS     nx_tftp_client_server_ip;                       /* Server's IP address                  */ 
    UINT            nx_tftp_client_server_port;                     /* Server's port number (69 originally) */ 
    UINT            nx_tftp_client_state;                           /* State of TFTP client                 */ 
    USHORT          nx_tftp_client_block_number;                    /* Block number in file transfer        */ 
    USHORT          nx_tftp_client_reserved;                        /* Reserved for future use              */ 
    UINT            nx_tftp_client_error_code;                      /* Error code received                  */ 
    CHAR            nx_tftp_client_error_string[NX_TFTP_ERROR_STRING_MAX + 1];
    NX_UDP_SOCKET   nx_tftp_client_socket;                          /* TFTP Socket                          */

} NX_TFTP_CLIENT;



#ifndef NX_TFTP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/******************* Services without error checking.  ***********************/

/* NetX (IPv4 supported only) services  */
#define nx_tftp_client_file_open                _nx_tftp_client_file_open
                                
/* For NetX applications, map the NetX TFTP service to the NetX Duo TFTP service.  */
#define nx_tftp_client_create(a,b,c,d)          _nxd_tftp_client_create(a,b,c,d,NX_IP_VERSION_V4)
#define nx_tftp_client_packet_allocate(a,b,c)   _nxd_tftp_client_packet_allocate(a,b,c, NX_IP_VERSION_V4)
#define nx_tftp_client_delete                   _nxd_tftp_client_delete                         
#define nx_tftp_client_error_info_get           _nxd_tftp_client_error_info_get
#define nx_tftp_client_file_close(a)            _nxd_tftp_client_file_close(a, NX_IP_VERSION_V4)
#define nx_tftp_client_file_read(a,b,c)         _nxd_tftp_client_file_read(a,b,c,NX_IP_VERSION_V4)
#define nx_tftp_client_file_write(a,b,c)        _nxd_tftp_client_file_write(a,b,c,NX_IP_VERSION_V4)
#define nx_tftp_client_set_interface            _nxd_tftp_client_set_interface

/* NetX Duo TFTP (IPv4 and IPv6 supported) services. */
#define nxd_tftp_client_create                  _nxd_tftp_client_create
#define nxd_tftp_client_delete                  _nxd_tftp_client_delete
#define nxd_tftp_client_file_open               _nxd_tftp_client_file_open
#define nxd_tftp_client_error_info_get          _nxd_tftp_client_error_info_get
#define nxd_tftp_client_file_close              _nxd_tftp_client_file_close
#define nxd_tftp_client_file_read               _nxd_tftp_client_file_read
#define nxd_tftp_client_file_write              _nxd_tftp_client_file_write
#define nxd_tftp_client_packet_allocate         _nxd_tftp_client_packet_allocate
#define nxd_tftp_client_set_interface           _nxd_tftp_client_set_interface

#else

/*************** Services with error checking.  ******************/

/* NetX (IPv4 supported only) services  */
#define nx_tftp_client_file_open                _nxe_tftp_client_file_open
                                            
/* For NetX TFTP applications, map the NetX TFTP service to the equivalent NetX Duo TFTP service.  */
#define nx_tftp_client_create(a,b,c,d)          _nxde_tftp_client_create(a,b,c,d,NX_IP_VERSION_V4)
#define nx_tftp_client_packet_allocate(a,b,c)   _nxde_tftp_client_packet_allocate(a,b,c,NX_IP_VERSION_V4)
#define nx_tftp_client_delete                   _nxde_tftp_client_delete
#define nx_tftp_client_error_info_get           _nxde_tftp_client_error_info_get
#define nx_tftp_client_file_close(a)            _nxde_tftp_client_file_close(a, NX_IP_VERSION_V4)
#define nx_tftp_client_file_read(a,b,c)         _nxde_tftp_client_file_read(a,b,c,NX_IP_VERSION_V4)
#define nx_tftp_client_file_write(a,b,c)        _nxde_tftp_client_file_write(a,b,c,NX_IP_VERSION_V4)
#define nx_tftp_client_set_interface            _nxde_tftp_client_set_interface

/* NetX Duo (IPv4 and IPv6 supported) services. */
#define nxd_tftp_client_create                  _nxde_tftp_client_create
#define nxd_tftp_client_delete                  _nxde_tftp_client_delete
#define nxd_tftp_client_file_open               _nxde_tftp_client_file_open
#define nxd_tftp_client_error_info_get          _nxde_tftp_client_error_info_get
#define nxd_tftp_client_file_close              _nxde_tftp_client_file_close
#define nxd_tftp_client_file_read               _nxde_tftp_client_file_read
#define nxd_tftp_client_file_write              _nxde_tftp_client_file_write
#define nxd_tftp_client_packet_allocate         _nxde_tftp_client_packet_allocate
#define nxd_tftp_client_set_interface           _nxde_tftp_client_set_interface


#endif  /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT        nx_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, ULONG server_ip_address, UINT open_type, ULONG wait_option);

UINT        nxd_tftp_client_create(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *tftp_client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, UINT ip_type);
UINT        nxd_tftp_client_delete(NX_TFTP_CLIENT *tftp_client_ptr);
UINT        nxd_tftp_client_error_info_get(NX_TFTP_CLIENT *tftp_client_ptr, UINT *error_code, CHAR **error_string);
UINT        nxd_tftp_client_file_close(NX_TFTP_CLIENT *tftp_client_ptr, UINT ip_type);
UINT        nxd_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT ip_type);
UINT        nxd_tftp_client_file_read(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        nxd_tftp_client_file_write(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option, UINT ip_type);
UINT        nxd_tftp_client_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        nxd_tftp_client_set_interface(NX_TFTP_CLIENT *tftpv6_client_ptr, UINT if_index);

#else

/* TFTP source code is being compiled, do not perform any API mapping.  */
             
UINT        _nxe_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, ULONG server_ip_address, UINT open_type, ULONG wait_option);
UINT        _nx_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, ULONG server_ip_address, UINT open_type, ULONG wait_option);  

UINT        _nxde_tftp_client_create(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *tftp_client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, UINT ip_type);
UINT        _nxd_tftp_client_create(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *tftp_client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, UINT ip_type);
UINT        _nxde_tftp_client_delete(NX_TFTP_CLIENT *tftp_client_ptr);
UINT        _nxd_tftp_client_delete(NX_TFTP_CLIENT *tftp_client_ptr);
UINT        _nxde_tftp_client_error_info_get(NX_TFTP_CLIENT *tftp_client_ptr, UINT *error_code, CHAR **error_string);
UINT        _nxd_tftp_client_error_info_get(NX_TFTP_CLIENT *tftp_client_ptr, UINT *error_code, CHAR **error_string);
UINT        _nxde_tftp_client_file_close(NX_TFTP_CLIENT *tftp_client_ptr, UINT ip_type);
UINT        _nxd_tftp_client_file_close(NX_TFTP_CLIENT *tftp_client_ptr, UINT ip_type);   
UINT        _nxde_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT ip_type);
UINT        _nxd_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT ip_type);
UINT        _nxde_tftp_client_file_read(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxd_tftp_client_file_read(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxde_tftp_client_file_write(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxd_tftp_client_file_write(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxde_tftp_client_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxd_tftp_client_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type);
UINT        _nxde_tftp_client_set_interface(NX_TFTP_CLIENT *tftp_client_ptr, UINT if_index);
UINT        _nxd_tftp_client_set_interface(NX_TFTP_CLIENT *tftp_client_ptr, UINT if_index);



#endif    /* NX_TFTP_SOURCE_CODE */
                            
/* Internal functions. */  
UINT  _nx_tftp_client_file_open_internal(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT  ip_type);


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif   

#endif   /* NXD_TFTP_CLIENT_H */
