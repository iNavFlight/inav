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
/*    nx_pppoe_server.h                                   PORTABLE C      */  
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX PPP Over Ethernet (PPPoE) Server         */ 
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
/*  02-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the compiler errors,  */
/*                                            resulting in version 6.1.4  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_PPPOE_SERVER_H
#define NX_PPPOE_SERVER_H

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

/* Define the PPPoE Server ID.  */
#define NX_PPPOE_SERVER_ID                                  0x50505045UL
     
/* Enable the feature to control the session established.  */
/*
#define NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
*/

/* If the driver is not initialized in other module, enable the feature to initialize the driver in PPPoE module .  */
/*
#define NX_PPPOE_SERVER_INITIALIZE_DRIVER_ENABLE
*/

/* Define the PPPoE Thread time slice.  */ 
#ifndef NX_PPPOE_SERVER_THREAD_TIME_SLICE
#define NX_PPPOE_SERVER_THREAD_TIME_SLICE                   TX_NO_TIME_SLICE
#endif      

/* Define the number of pppoe clients. */  
#ifndef NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER
#define NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER           10
#endif                  
                                      
/* Define the size of pppoe Host-Uniq.  */  
#ifndef NX_PPPOE_SERVER_MAX_HOST_UNIQ_SIZE
#define NX_PPPOE_SERVER_MAX_HOST_UNIQ_SIZE                  32
#endif                  
                                    
/* Define the size of pppoe Relay-Session-Id.  */  
#ifndef NX_PPPOE_SERVER_MAX_RELAY_SESSION_ID_SIZE
#define NX_PPPOE_SERVER_MAX_RELAY_SESSION_ID_SIZE           12
#endif            

/* Define the minimum size of packet payload to avoid packet chained. Maximum Payload Size of Ethernet(1500) + Ethernet Header + CRC + Four-byte alignment.  */

#ifndef NX_PPPOE_SERVER_MIN_PACKET_PAYLOAD_SIZE
#define NX_PPPOE_SERVER_MIN_PACKET_PAYLOAD_SIZE             1520
#endif
                            
/* The time out in timer ticks on allocating packets or appending data into packets.  */
#ifndef NX_PPPOE_SERVER_PACKET_TIMEOUT
#define NX_PPPOE_SERVER_PACKET_TIMEOUT                      (NX_IP_PERIODIC_RATE)           /* 1 second  */
#endif
                           
/* Set the start Session ID for assigning to the PPPoE session. */
#ifndef NX_PPPOE_SERVER_START_SESSION_ID
#define NX_PPPOE_SERVER_START_SESSION_ID                    0x4944
#endif

/* Define PPPoE ethernet header size.  */
#define NX_PPPOE_SERVER_ETHER_HEADER_SIZE                   14

/* Define PPPoE ethernet types.  */
#define NX_PPPOE_SERVER_ETHER_TYPE_DISCOVERY                0x8863
#define NX_PPPOE_SERVER_ETHER_TYPE_SESSION                  0x8864
                
/* Define PPPoE version and type.   */                                         
#define NX_PPPOE_SERVER_VERSION_TYPE                        0x11     /* Version 1, Type 1.  */  
                                                                     
/* Define PPPoE codes.   */                                         
#define NX_PPPOE_SERVER_CODE_ZERO                           0x00
#define NX_PPPOE_SERVER_CODE_PADI                           0x09 
#define NX_PPPOE_SERVER_CODE_PADO                           0x07 
#define NX_PPPOE_SERVER_CODE_PADR                           0x19
#define NX_PPPOE_SERVER_CODE_PADS                           0x65
#define NX_PPPOE_SERVER_CODE_PADT                           0xa7
                                                                                 
/* Define the PPPoE Area Offsets.  */
#define NX_PPPOE_SERVER_OFFSET_VER_TYPE                     0       /* 1 byte, version + type: 0x11                     */  
#define NX_PPPOE_SERVER_OFFSET_CODE                         1       /* 1 byte, code: Discovery or Session               */
#define NX_PPPOE_SERVER_OFFSET_SESSION_ID                   2       /* 2 bytes, session id: unique session identifieer  */
#define NX_PPPOE_SERVER_OFFSET_LENGTH                       4       /* 2 bytes, length: the length of PPPoE payload     */ 
#define NX_PPPOE_SERVER_OFFSET_PAYLOAD                      6       /* variable, payload                                */ 
                                                                                                                              
/* Define the PPPoE Tag Types.  */
#define NX_PPPOE_SERVER_TAG_TYPE_END_OF_LIST                0x0000       
#define NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME               0x0101   
#define NX_PPPOE_SERVER_TAG_TYPE_AC_NAME                    0x0102   
#define NX_PPPOE_SERVER_TAG_TYPE_HOST_UNIQ                  0x0103   
#define NX_PPPOE_SERVER_TAG_TYPE_AC_COOKIE                  0x0104   
#define NX_PPPOE_SERVER_TAG_TYPE_VENDOR_SPECIFIC            0x0105   
#define NX_PPPOE_SERVER_TAG_TYPE_RELAY_SESSION_ID           0x0110   
#define NX_PPPOE_SERVER_TAG_TYPE_SERVICE_NAME_ERROR         0x0201   
#define NX_PPPOE_SERVER_TAG_TYPE_AC_SYSTEM_ERROR            0x0202    
#define NX_PPPOE_SERVER_TAG_TYPE_GENERIC_ERROR              0x0203   
                                                                   
/* Define the PPPoE Error flags.  */
#define NX_PPPOE_SERVER_ERROR_SERVICE_NAME                  ((ULONG) 0x00000001)    /* Service Name Error.              */
#define NX_PPPOE_SERVER_ERROR_AC_SYSTEM                     ((ULONG) 0x00000002)    /* AC-System Error                  */ 
#define NX_PPPOE_SERVER_ERROR_GENERIC                       ((ULONG) 0x00000004)    /* Generic Error                    */

/* Define event flags for PPPoE thread control.  */
#define NX_PPPOE_SERVER_ALL_EVENTS                          ((ULONG) 0xFFFFFFFF)    /* All event flags                      */
#define NX_PPPOE_SERVER_PACKET_RECEIVE_EVENT                ((ULONG) 0x00000001)    /* PPPoE Server receive packet event    */
#define NX_PPPOE_SERVER_SESSION_RECEIVE_EVENT               ((ULONG) 0x00000002)    /* PPPoE Session receive packet event   */

/* Define error codes from PPPoE Server operation.  */
#define NX_PPPOE_SERVER_SUCCESS                             0x00    /* Success                                           */
#define NX_PPPOE_SERVER_PTR_ERROR                           0xC1    /* Invalid input pointers                            */ 
#define NX_PPPOE_SERVER_INVALID_INTERFACE                   0xC2    /* Invalid interface                                 */ 
#define NX_PPPOE_SERVER_PACKET_PAYLOAD_ERROR                0xC3    /* Invalid payload size of packet                    */
#define NX_PPPOE_SERVER_MEMORY_SIZE_ERROR                   0xC4    /* Invalid memory size                               */
#define NX_PPPOE_SERVER_PRIORITY_ERROR                      0xC5    /* Invalid priority                                  */
#define NX_PPPOE_SERVER_NOT_ENABLED                         0xC6    /* PPPoE is not enabled                              */ 
#define NX_PPPOE_SERVER_INVALID_SESSION                     0xC7    /* Invalid Session                                   */
#define NX_PPPOE_SERVER_SESSION_NOT_ESTABLISHED             0xC8    /* PPPoE Session is not established                  */
#define NX_PPPOE_SERVER_SERVICE_NAME_ERROR                  0xC9    /* Service name error                                */
#define NX_PPPOE_SERVER_AC_NAME_ERROR                       0xCA    /* AC Name error                                     */
#define NX_PPPOE_SERVER_CLIENT_SESSION_FULL                 0xCB    /* Client Session full                               */
#define NX_PPPOE_SERVER_CLIENT_SESSION_NOT_FOUND            0xCC    /* Not found the client session                      */
#define NX_PPPOE_SERVER_HOST_UNIQ_CACHE_ERROR               0xCD    /* The cache is not enough to record the Host Uniq   */
#define NX_PPPOE_SERVER_RELAY_SESSION_ID_CACHE_ERROR        0xCF    /* The cache is not enough to record Relay Session ID*/

/* Define the PPPoE Client Session structure containing the session id and physical address.  */

typedef struct NX_PPPOE_CLIENT_SESSION_STRUCT
{             

    USHORT          nx_pppoe_valid;
    USHORT          nx_pppoe_session_id;
    ULONG           nx_pppoe_physical_address_msw;
    ULONG           nx_pppoe_physical_address_lsw;
    UCHAR          *nx_pppoe_service_name;
#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
    UINT            nx_pppoe_service_name_length;
    UINT            nx_pppoe_packet_receive_stopped;
    NX_PACKET      *nx_pppoe_deferred_received_packet_head,
                   *nx_pppoe_deferred_received_packet_tail;
    UCHAR          *nx_pppoe_generic_error;
#endif
    UCHAR           nx_pppoe_host_uniq[NX_PPPOE_SERVER_MAX_HOST_UNIQ_SIZE];   
    UINT            nx_pppoe_host_uniq_size;
    UCHAR           nx_pppoe_relay_session_id[NX_PPPOE_SERVER_MAX_RELAY_SESSION_ID_SIZE];
    UINT            nx_pppoe_relay_session_id_size;
    UINT            nx_pppoe_error_flag;
} NX_PPPOE_CLIENT_SESSION;



/* Define the main PPPoE Server data structure.  */

typedef struct NX_PPPOE_SERVER_STRUCT 
{

    ULONG                       nx_pppoe_id;
    UINT                        nx_pppoe_enabled;
    UCHAR                      *nx_pppoe_name;  
    UINT                        nx_pppoe_name_length;
    UCHAR                      *nx_pppoe_ac_name;  
    UINT                        nx_pppoe_ac_name_length;
    NX_IP                      *nx_pppoe_ip_ptr;
    NX_INTERFACE               *nx_pppoe_interface_ptr;
    NX_PACKET_POOL             *nx_pppoe_packet_pool_ptr;  
    TX_EVENT_FLAGS_GROUP        nx_pppoe_events;      
    TX_THREAD                   nx_pppoe_thread;      
    NX_PACKET                  *nx_pppoe_deferred_received_packet_head,
                               *nx_pppoe_deferred_received_packet_tail;
    UCHAR                     **nx_pppoe_service_name;
    UINT                        nx_pppoe_service_name_count;
    USHORT                      nx_pppoe_session_id;
    USHORT                      nx_pppoe_reserved[2];
    NX_PPPOE_CLIENT_SESSION     nx_pppoe_client_session[NX_PPPOE_SERVER_MAX_CLIENT_SESSION_NUMBER];

    /* Define the callback nofiy function.  */
    VOID                      (*nx_pppoe_discover_initiation_notify)(UINT session_index);
    VOID                      (*nx_pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data);
    VOID                      (*nx_pppoe_discover_terminate_notify)(UINT session_index);    
    VOID                      (*nx_pppoe_discover_terminate_confirm)(UINT session_index);
    VOID                      (*nx_pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id); 
    VOID                      (*nx_pppoe_data_send_notify)(UINT session_index, UCHAR *data);
    
    /* Define the Link Driver entry point.  */
    VOID                      (*nx_pppoe_link_driver_entry)(struct NX_IP_DRIVER_STRUCT *);

} NX_PPPOE_SERVER;


#ifndef NX_PPPOE_SERVER_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_pppoe_server_create              _nx_pppoe_server_create
#define nx_pppoe_server_delete              _nx_pppoe_server_delete  
#define nx_pppoe_server_enable              _nx_pppoe_server_enable
#define nx_pppoe_server_disable             _nx_pppoe_server_disable   
#define nx_pppoe_server_callback_notify_set _nx_pppoe_server_callback_notify_set
#define nx_pppoe_server_ac_name_set         _nx_pppoe_server_ac_name_set 
#define nx_pppoe_server_service_name_set    _nx_pppoe_server_service_name_set
#define nx_pppoe_server_session_send        _nx_pppoe_server_session_send
#define nx_pppoe_server_session_packet_send _nx_pppoe_server_session_packet_send
#define nx_pppoe_server_session_terminate   _nx_pppoe_server_session_terminate
#define nx_pppoe_server_session_get         _nx_pppoe_server_session_get

#else

/* Services with error checking.  */

#define nx_pppoe_server_create              _nxe_pppoe_server_create
#define nx_pppoe_server_delete              _nxe_pppoe_server_delete     
#define nx_pppoe_server_enable              _nxe_pppoe_server_enable
#define nx_pppoe_server_disable             _nxe_pppoe_server_disable      
#define nx_pppoe_server_callback_notify_set _nxe_pppoe_server_callback_notify_set
#define nx_pppoe_server_ac_name_set         _nxe_pppoe_server_ac_name_set 
#define nx_pppoe_server_service_name_set    _nxe_pppoe_server_service_name_set
#define nx_pppoe_server_session_send        _nxe_pppoe_server_session_send
#define nx_pppoe_server_session_packet_send _nxe_pppoe_server_session_packet_send
#define nx_pppoe_server_session_terminate   _nxe_pppoe_server_session_terminate
#define nx_pppoe_server_session_get         _nxe_pppoe_server_session_get

#endif /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT    nx_pppoe_server_create(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                               VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *), NX_PACKET_POOL *pool_ptr,
                               VOID *stack_ptr, ULONG stack_size, UINT priority);
UINT    nx_pppoe_server_delete(NX_PPPOE_SERVER *pppoe_server_ptr);  
UINT    nx_pppoe_server_enable(NX_PPPOE_SERVER *pppoe_server_ptr);
UINT    nx_pppoe_server_disable(NX_PPPOE_SERVER *pppoe_server_ptr);     
UINT    nx_pppoe_server_callback_notify_set(NX_PPPOE_SERVER *pppoe_server_ptr, 
                                            VOID (* pppoe_discover_initiation_notify)(UINT session_index), 
                                            VOID (* pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data),
                                            VOID (* pppoe_discover_terminate_notify)(UINT session_index),
                                            VOID (* pppoe_discover_terminate_confirm)(UINT session_index),
                                            VOID (* pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id),
                                            VOID (* pppoe_data_send_notify)(UINT session_index, UCHAR *data));
UINT    nx_pppoe_server_ac_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *ac_name, UINT ac_name_length);
UINT    nx_pppoe_server_service_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR **service_name, UINT service_name_count);
UINT    nx_pppoe_server_session_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, UCHAR *data_ptr, UINT data_length);
UINT    nx_pppoe_server_session_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, NX_PACKET *packet_ptr);
UINT    nx_pppoe_server_session_terminate(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index); 
UINT    nx_pppoe_server_session_get(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id);
VOID    _nx_pppoe_server_packet_deferred_receive(NX_PACKET *packet_ptr);

#else
                                                            
UINT    _nxe_pppoe_server_create(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                                 VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *), NX_PACKET_POOL *pool_ptr,
                                 VOID *stack_ptr, ULONG stack_size, UINT priority);
UINT    _nx_pppoe_server_create(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *name, NX_IP *ip_ptr, UINT interface_index,
                                VOID (*pppoe_link_driver)(struct NX_IP_DRIVER_STRUCT *), NX_PACKET_POOL *pool_ptr,
                                VOID *stack_ptr, ULONG stack_size, UINT priority); 
UINT    _nxe_pppoe_server_delete(NX_PPPOE_SERVER *pppoe_server_ptr);
UINT    _nx_pppoe_server_delete(NX_PPPOE_SERVER *pppoe_server_ptr);   
UINT    _nxe_pppoe_server_enable(NX_PPPOE_SERVER *pppoe_server_ptr); 
UINT    _nx_pppoe_server_enable(NX_PPPOE_SERVER *pppoe_server_ptr);
UINT    _nxe_pppoe_server_disable(NX_PPPOE_SERVER *pppoe_server_ptr); 
UINT    _nx_pppoe_server_disable(NX_PPPOE_SERVER *pppoe_server_ptr);              
UINT    _nxe_pppoe_server_callback_notify_set(NX_PPPOE_SERVER *pppoe_server_ptr, 
                                              VOID (* pppoe_discover_initiation_notify)(UINT session_index), 
                                              VOID (* pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data),
                                              VOID (* pppoe_discover_terminate_notify)(UINT session_index),
                                              VOID (* pppoe_discover_terminate_confirm)(UINT session_index),
                                              VOID (* pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id),
                                              VOID (* pppoe_data_send_notify)(UINT session_index, UCHAR *data));
UINT    _nx_pppoe_server_callback_notify_set(NX_PPPOE_SERVER *pppoe_server_ptr, 
                                             VOID (* pppoe_discover_initiation_notify)(UINT session_index), 
                                             VOID (* pppoe_discover_request_notify)(UINT session_index, ULONG length, UCHAR *data),
                                             VOID (* pppoe_discover_terminate_notify)(UINT session_index),
                                             VOID (* pppoe_discover_terminate_confirm)(UINT session_index),
                                             VOID (* pppoe_data_receive_notify)(UINT session_index, ULONG length, UCHAR *data, UINT packet_id),
                                             VOID (* pppoe_data_send_notify)(UINT session_index, UCHAR *data));
UINT    _nxe_pppoe_server_ac_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *ac_name, UINT ac_name_length);
UINT    _nx_pppoe_server_ac_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR *ac_name, UINT ac_name_length);                                             
UINT    _nxe_pppoe_server_service_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR **service_name, UINT service_name_count);
UINT    _nx_pppoe_server_service_name_set(NX_PPPOE_SERVER *pppoe_server_ptr, UCHAR **service_name, UINT service_name_count);
UINT    _nxe_pppoe_server_session_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, UCHAR *data_ptr, UINT data_length);
UINT    _nx_pppoe_server_session_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, UCHAR *data_ptr, UINT data_length);
UINT    _nxe_pppoe_server_session_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, NX_PACKET *packet_ptr);
UINT    _nx_pppoe_server_session_packet_send(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, NX_PACKET *packet_ptr);
UINT    _nxe_pppoe_server_session_terminate(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index);
UINT    _nx_pppoe_server_session_terminate(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index);
UINT    _nxe_pppoe_server_session_get(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id);
UINT    _nx_pppoe_server_session_get(NX_PPPOE_SERVER *pppoe_server_ptr, UINT session_index, ULONG *client_mac_msw, ULONG *client_mac_lsw, ULONG *session_id);
VOID    _nx_pppoe_server_packet_deferred_receive(NX_PACKET *packet_ptr);

#endif /* NX_PPPOE_SERVER_SOURCE_CODE */

#ifdef NX_PPPOE_SERVER_SESSION_CONTROL_ENABLE
VOID    PppInitInd(UINT length, UCHAR *aData);
VOID    PppDiscoverCnf(UINT length, UCHAR *aData, UINT interfaceHandle); 
VOID    PppOpenCnf(UCHAR accept, UINT interfaceHandle);
VOID    PppCloseInd(UINT interfaceHandle, UCHAR *causeCode);
VOID    PppCloseCnf(UINT interfaceHandle);
VOID    PppTransmitDataCnf(UINT interfaceHandle, UCHAR *aData, UINT packet_id); 
VOID    PppReceiveDataInd(UINT interfaceHandle, UINT length, UCHAR *aData);
#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif /* NX_PPPOE_SERVER_H */ 
