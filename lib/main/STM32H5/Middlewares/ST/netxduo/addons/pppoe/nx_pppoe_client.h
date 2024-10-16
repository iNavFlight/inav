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
/**   PPP Over Ethernet (PPPoE)                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nx_pppoe_client.h                                   PORTABLE C      */  
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX PPP Over Ethernet (PPPoE) Client         */ 
/*    componet, including all data types and external references. It is   */ 
/*    assumed that nx_api.h and nx_port.h have already been included.     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            string length verification, */
/*                                            resulting in version 6.1.3  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_PPPOE_CLIENT_H
#define NX_PPPOE_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Note: Prerequisite for using PPPoE.
         Redefine NX_PHYSICAL_HEADER to 24 to ensure enough space for filling in physical header.
         Physical header:14(Ethernet header) + 6(PPPoE header) + 2(PPP header) + 2(four-byte aligment).  */

/* Define the Driver command for PPPoE packet.  */
#ifndef NX_LINK_PPPOE_DISCOVERY_SEND
#define NX_LINK_PPPOE_DISCOVERY_SEND 51
#endif
#ifndef NX_LINK_PPPOE_SESSION_SEND
#define NX_LINK_PPPOE_SESSION_SEND   52
#endif

/* Define the PPPoE Client ID.  */
#define NX_PPPOE_CLIENT_ID                                  0x504F4543UL

/* If the driver is not initialized in other module, enable the feature to initialize the driver in PPPoE module .  */
/*
#define NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE
*/

/* Define the PPPoE Thread time slice.  */ 
#ifndef NX_PPPOE_CLIENT_THREAD_TIME_SLICE
#define NX_PPPOE_CLIENT_THREAD_TIME_SLICE                   TX_NO_TIME_SLICE
#endif      

/* Define the init timeout for PADI.  */
#ifndef NX_PPPOE_CLIENT_PADI_INIT_TIMEOUT
#define NX_PPPOE_CLIENT_PADI_INIT_TIMEOUT                   1
#endif

/* Define the maximum number for PADI.  */
#ifndef NX_PPPOE_CLIENT_PADI_COUNT
#define NX_PPPOE_CLIENT_PADI_COUNT                          4
#endif

/* Define the init timeout for PADR.  */
#ifndef NX_PPPOE_CLIENT_PADR_INIT_TIMEOUT 
#define NX_PPPOE_CLIENT_PADR_INIT_TIMEOUT                   1
#endif

/* Define the maximum number for PADR.  */
#ifndef NX_PPPOE_CLIENT_PADR_COUNT
#define NX_PPPOE_CLIENT_PADR_COUNT                          4
#endif

/* Define the size of pppoe AC-Name.  */  
#ifndef NX_PPPOE_CLIENT_MAX_AC_NAME_SIZE
#define NX_PPPOE_CLIENT_MAX_AC_NAME_SIZE                    32
#endif

/* Define the size of pppoe AC-Cookie.  */  
#ifndef NX_PPPOE_CLIENT_MAX_AC_COOKIE_SIZE
#define NX_PPPOE_CLIENT_MAX_AC_COOKIE_SIZE                  32
#endif 

/* Define the size of pppoe Relay-Session-Id.  */  
#ifndef NX_PPPOE_CLIENT_MAX_RELAY_SESSION_ID_SIZE
#define NX_PPPOE_CLIENT_MAX_RELAY_SESSION_ID_SIZE           12
#endif            

/* Define the minimum size of packet payload to avoid packet chained. Maximum Payload Size of Ethernet(1500) + Ethernet Header + CRC + Four-byte alignment.  */

#ifndef NX_PPPOE_CLIENT_MIN_PACKET_PAYLOAD_SIZE
#define NX_PPPOE_CLIENT_MIN_PACKET_PAYLOAD_SIZE             1520
#endif

/* Define PPPoE ethernet header size.  */
#define NX_PPPOE_CLIENT_ETHER_HEADER_SIZE                   14

/* Define PPPoE ethernet types.  */
#define NX_PPPOE_CLIENT_ETHER_TYPE_DISCOVERY                0x8863
#define NX_PPPOE_CLIENT_ETHER_TYPE_SESSION                  0x8864
                
/* Define PPPoE version and type.   */                                         
#define NX_PPPOE_CLIENT_VERSION_TYPE                        0x11     /* Version 1, Type 1.  */  
                                                                     
/* Define PPPoE codes.   */                                         
#define NX_PPPOE_CLIENT_CODE_ZERO                           0x00
#define NX_PPPOE_CLIENT_CODE_PADI                           0x09 
#define NX_PPPOE_CLIENT_CODE_PADO                           0x07 
#define NX_PPPOE_CLIENT_CODE_PADR                           0x19
#define NX_PPPOE_CLIENT_CODE_PADS                           0x65
#define NX_PPPOE_CLIENT_CODE_PADT                           0xa7
                                                                                 
/* Define the PPPoE Area Offsets.  */
#define NX_PPPOE_CLIENT_OFFSET_VER_TYPE                     0       /* 1 byte, version + type: 0x11                     */  
#define NX_PPPOE_CLIENT_OFFSET_CODE                         1       /* 1 byte, code: Discovery or Session               */
#define NX_PPPOE_CLIENT_OFFSET_SESSION_ID                   2       /* 2 bytes, session id: unique session identifieer  */
#define NX_PPPOE_CLIENT_OFFSET_LENGTH                       4       /* 2 bytes, length: the length of PPPoE payload     */ 
#define NX_PPPOE_CLIENT_OFFSET_PAYLOAD                      6       /* variable, payload                                */ 
                                                                                                                              
/* Define the PPPoE Tag Types.  */
#define NX_PPPOE_CLIENT_TAG_TYPE_END_OF_LIST                0x0000       
#define NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME               0x0101   
#define NX_PPPOE_CLIENT_TAG_TYPE_AC_NAME                    0x0102   
#define NX_PPPOE_CLIENT_TAG_TYPE_HOST_UNIQ                  0x0103   
#define NX_PPPOE_CLIENT_TAG_TYPE_AC_COOKIE                  0x0104   
#define NX_PPPOE_CLIENT_TAG_TYPE_VENDOR_SPECIFIC            0x0105   
#define NX_PPPOE_CLIENT_TAG_TYPE_RELAY_SESSION_ID           0x0110   
#define NX_PPPOE_CLIENT_TAG_TYPE_SERVICE_NAME_ERROR         0x0201   
#define NX_PPPOE_CLIENT_TAG_TYPE_AC_SYSTEM_ERROR            0x0202    
#define NX_PPPOE_CLIENT_TAG_TYPE_GENERIC_ERROR              0x0203   
                                                                   
/* Define the PPPoE Error flags.  */
#define NX_PPPOE_CLIENT_ERROR_SERVICE_NAME                  ((ULONG) 0x00000001)    /* Service Name Error.              */
#define NX_PPPOE_CLIENT_ERROR_AC_SYSTEM                     ((ULONG) 0x00000002)    /* AC-System Error                  */ 
#define NX_PPPOE_CLIENT_ERROR_GENERIC                       ((ULONG) 0x00000004)    /* Generic Error                    */

/* Define event flags for PPPoE thread control.  */
#define NX_PPPOE_CLIENT_ALL_EVENTS                          ((ULONG) 0xFFFFFFFF)    /* All event flags                      */
#define NX_PPPOE_CLIENT_PACKET_RECEIVE_EVENT                ((ULONG) 0x00000001)    /* PPPoE Client receive packet event    */
#define NX_PPPOE_CLIENT_TIMER_PERIODIC_EVENT                ((ULONG) 0x00000002)    /* PPPoE CLient timer Periodic event    */
#define NX_PPPOE_CLIENT_SESSION_CONNECT_CLEANUP_EVENT       ((ULONG) 0x00000004)    /* PPPoE Client Session cleanup event   */

/* Define PPPoE Client state.  */
#define NX_PPPOE_CLIENT_STATE_INITIAL                       0
#define NX_PPPOE_CLIENT_STATE_PADI_SENT                     1
#define NX_PPPOE_CLIENT_STATE_PADR_SENT                     2
#define NX_PPPOE_CLIENT_STATE_ESTABLISHED                   3

/* Define error codes from PPPoE Server operation.  */
#define NX_PPPOE_CLIENT_SUCCESS                             0x00    /* Success                                           */
#define NX_PPPOE_CLIENT_PTR_ERROR                           0xD1    /* Invalid input pointers                            */ 
#define NX_PPPOE_CLIENT_INVALID_INTERFACE                   0xD2    /* Invalid interface                                 */ 
#define NX_PPPOE_CLIENT_PACKET_PAYLOAD_ERROR                0xD3    /* Invalid payload size of packet                    */
#define NX_PPPOE_CLIENT_MEMORY_SIZE_ERROR                   0xD4    /* Invalid memory size                               */
#define NX_PPPOE_CLIENT_PRIORITY_ERROR                      0xD5    /* Invalid priority                                  */
#define NX_PPPOE_CLIENT_NOT_ENABLED                         0xD6    /* PPPoE is not enabled                              */ 
#define NX_PPPOE_CLIENT_INVALID_SESSION                     0xD7    /* Invalid Session                                   */
#define NX_PPPOE_CLIENT_SESSION_NOT_ESTABLISHED             0xD8    /* PPPoE Session is not established                  */
#define NX_PPPOE_CLIENT_SERVICE_NAME_ERROR                  0xD9    /* Service name error                                */
#define NX_PPPOE_CLIENT_AC_NAME_ERROR                       0xDA    /* AC Name error                                     */
#define NX_PPPOE_CLIENT_CLIENT_SESSION_FULL                 0xDB    /* Client Session full                               */
#define NX_PPPOE_CLIENT_CLIENT_SESSION_NOT_FOUND            0xDC    /* Not found the client session                      */
#define NX_PPPOE_CLIENT_HOST_UNIQ_CACHE_ERROR               0xDD    /* The cache is not enough to record the Host Uniq   */
#define NX_PPPOE_CLIENT_RELAY_SESSION_ID_CACHE_ERROR        0xDF    /* The cache is not enough to record Relay Session ID*/


/* Define the PPPoE Server Session structure containing the session id and physical address.  */

typedef struct NX_PPPOE_SERVER_SESSION_STRUCT
{             

    USHORT          nx_pppoe_session_id;
    USHORT          reserved;
    ULONG           nx_pppoe_physical_address_msw;
    ULONG           nx_pppoe_physical_address_lsw;
} NX_PPPOE_SERVER_SESSION;


/* Define the main PPPoE Server data structure.  */

typedef struct NX_PPPOE_CLIENT_STRUCT 
{

    ULONG                       nx_pppoe_id;
    UCHAR                      *nx_pppoe_name;
    NX_IP                      *nx_pppoe_ip_ptr;
    NX_INTERFACE               *nx_pppoe_interface_ptr;
    NX_PACKET_POOL             *nx_pppoe_packet_pool_ptr;  
    TX_EVENT_FLAGS_GROUP        nx_pppoe_events;
    TX_THREAD                   nx_pppoe_thread;
    TX_TIMER                    nx_pppoe_timer;
    NX_PACKET                  *nx_pppoe_deferred_received_packet_head,
                               *nx_pppoe_deferred_received_packet_tail;
    UINT                        nx_pppoe_state;
    NX_PPPOE_SERVER_SESSION     nx_pppoe_server_session;
    UCHAR                      *nx_pppoe_service_name;
    UINT                        nx_pppoe_service_name_length;
    UCHAR                      *nx_pppoe_host_uniq;
    UINT                        nx_pppoe_host_uniq_length;
    UCHAR                       nx_pppoe_ac_name[NX_PPPOE_CLIENT_MAX_AC_NAME_SIZE];
    UINT                        nx_pppoe_ac_name_size;
    UCHAR                       nx_pppoe_ac_cookie[NX_PPPOE_CLIENT_MAX_AC_COOKIE_SIZE];
    UINT                        nx_pppoe_ac_cookie_size;
    UCHAR                       nx_pppoe_relay_session_id[NX_PPPOE_CLIENT_MAX_RELAY_SESSION_ID_SIZE];
    UINT                        nx_pppoe_relay_session_id_size;
    UINT                        nx_pppoe_error_flag;

    /* Define the retransmit timeout and count for PADI and PADR.  */
    UINT                        nx_pppoe_rtr_timeout;
    UINT                        nx_pppoe_rtr_count;

    /* Define the Link Driver entry point.  */
    VOID                      (*nx_pppoe_link_driver_entry)(struct NX_IP_DRIVER_STRUCT *);

    /* Define the function to receive PPPoE packet.  */
    VOID                      (*nx_pppoe_packet_receive)(NX_PACKET *packet_ptr);

    /* Define the PPPoE session connect suspension pointer that contains the pointer to the
       thread suspended attempting to establish a PPPoE session connection.  */
    TX_THREAD                  *nx_pppoe_session_connect_suspended_thread;

} NX_PPPOE_CLIENT;


#ifndef NX_PPPOE_CLIENT_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_pppoe_client_create              _nx_pppoe_client_create
#define nx_pppoe_client_delete              _nx_pppoe_client_delete  
#define nx_pppoe_client_host_uniq_set       _nx_pppoe_client_host_uniq_set
#define nx_pppoe_client_host_uniq_set_extended _nx_pppoe_client_host_uniq_set_extended
#define nx_pppoe_client_service_name_set    _nx_pppoe_client_service_name_set
#define nx_pppoe_client_service_name_set_extended _nx_pppoe_client_service_name_set_extended
#define nx_pppoe_client_session_connect     _nx_pppoe_client_session_connect
#define nx_pppoe_client_session_packet_send _nx_pppoe_client_session_packet_send
#define nx_pppoe_client_session_terminate   _nx_pppoe_client_session_terminate
#define nx_pppoe_client_session_get         _nx_pppoe_client_session_get

#else

/* Services with error checking.  */

#define nx_pppoe_client_create              _nxe_pppoe_client_create
#define nx_pppoe_client_delete              _nxe_pppoe_client_delete     
#define nx_pppoe_client_host_uniq_set       _nxe_pppoe_client_host_uniq_set
#define nx_pppoe_client_host_uniq_set_extended _nxe_pppoe_client_host_uniq_set_extended
#define nx_pppoe_client_service_name_set    _nxe_pppoe_client_service_name_set
#define nx_pppoe_client_service_name_set_extended _nxe_pppoe_client_service_name_set_extended
#define nx_pppoe_client_session_connect     _nxe_pppoe_client_session_connect
#define nx_pppoe_client_session_packet_send _nxe_pppoe_client_session_packet_send
#define nx_pppoe_client_session_terminate   _nxe_pppoe_client_session_terminate
#define nx_pppoe_client_session_get         _nxe_pppoe_client_session_get

#endif /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT    nx_pppoe_client_create(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                               VOID (*pppoe_packet_receive)(NX_PACKET *packet_ptr));
UINT    nx_pppoe_client_delete(NX_PPPOE_CLIENT *pppoe_client_ptr);
UINT    nx_pppoe_client_host_uniq_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq);
UINT    nx_pppoe_client_host_uniq_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq, UINT host_uniq_length);
UINT    nx_pppoe_client_service_name_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name); 
UINT    nx_pppoe_client_service_name_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name, UINT service_name_length); 
UINT    nx_pppoe_client_session_connect(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG wait_option);
UINT    nx_pppoe_client_session_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr);
UINT    nx_pppoe_client_session_terminate(NX_PPPOE_CLIENT *pppoe_client_ptr); 
UINT    nx_pppoe_client_session_get(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG *server_mac_msw, ULONG *server_mac_lsw, ULONG *session_id);
VOID    _nx_pppoe_client_packet_deferred_receive(NX_PACKET *packet_ptr);

#else

UINT    _nxe_pppoe_client_create(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                               VOID (*pppoe_packet_receive)(NX_PACKET *packet_ptr));
UINT    _nx_pppoe_client_create(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               NX_PACKET_POOL *pool_ptr, VOID *stack_ptr, ULONG stack_size, UINT priority,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                               VOID (*pppoe_packet_receive)(NX_PACKET *packet_ptr));
UINT    _nxe_pppoe_client_delete(NX_PPPOE_CLIENT *pppoe_client_ptr);
UINT    _nx_pppoe_client_delete(NX_PPPOE_CLIENT *pppoe_client_ptr);   
UINT    _nxe_pppoe_client_host_uniq_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq);
UINT    _nx_pppoe_client_host_uniq_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq);
UINT    _nxe_pppoe_client_host_uniq_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq, UINT host_uniq_length);
UINT    _nx_pppoe_client_host_uniq_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *host_uniq, UINT host_uniq_length);
UINT    _nxe_pppoe_client_service_name_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name);
UINT    _nx_pppoe_client_service_name_set(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name);
UINT    _nxe_pppoe_client_service_name_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name, UINT service_name_length);
UINT    _nx_pppoe_client_service_name_set_extended(NX_PPPOE_CLIENT *pppoe_client_ptr, UCHAR *service_name, UINT service_name_length);
UINT    _nxe_pppoe_client_session_connect(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG wait_option);
UINT    _nx_pppoe_client_session_connect(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG wait_option);
UINT    _nxe_pppoe_client_session_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr);
UINT    _nx_pppoe_client_session_packet_send(NX_PPPOE_CLIENT *pppoe_client_ptr, NX_PACKET *packet_ptr);
UINT    _nxe_pppoe_client_session_terminate(NX_PPPOE_CLIENT *pppoe_client_ptr);
UINT    _nx_pppoe_client_session_terminate(NX_PPPOE_CLIENT *pppoe_client_ptr);
UINT    _nxe_pppoe_client_session_get(NX_PPPOE_CLIENT *pppoe_client_ptr, ULONG *server_mac_msw, ULONG *server_mac_lsw, ULONG *session_id);
UINT    _nx_pppoe_client_session_get(NX_PPPOE_CLIENT *pppoe_server_ptr, ULONG *server_mac_msw, ULONG *server_mac_lsw, ULONG *session_id);
VOID    _nx_pppoe_client_packet_deferred_receive(NX_PACKET *packet_ptr);

#endif /* NX_PPPOE_CLIENT_SOURCE_CODE */


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif /* NX_PPPOE_CLIENT_H */ 
