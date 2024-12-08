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
/** NetX NAT Component                                                    */
/**                                                                       */
/**   Network Address Translation Protocol (NAT)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_nat.h                                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Network Address Translation Protocol     */
/*    (NAT) component, including all data types and external references.  */
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
/*                                                                        */
/**************************************************************************/

#ifndef  NX_NAT_H 
#define  NX_NAT_H 


#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */ 
extern   "C" {

#endif


#include "nx_api.h"
#include "nx_ip.h"
#include "nx_system.h"

#ifdef NX_NAT_ENABLE

/* Thread ID for identifying as an NAT device.  */

#define NX_NAT_ID                                   0x4E4154UL
           

/* Internal error processing codes. */

#define NX_NAT_ERROR_CONSTANT                       0xD00
                                                       
#define NX_NAT_PARAM_ERROR                          (NX_NAT_ERROR_CONSTANT | 0x01)      /* Invalid parameter for NAT service */      
#define NX_NAT_CACHE_ERROR                          (NX_NAT_ERROR_CONSTANT | 0x02)      /* NAT translation cache currently is full. */     
#define NX_NAT_NOT_ENABLED                          (NX_NAT_ERROR_CONSTANT | 0x03)      /* NAT is not enabled. */      
#define NX_NAT_ENTRY_NOT_FOUND                      (NX_NAT_ERROR_CONSTANT | 0x04)      /* Did not find the entry in NAT entry list. */   
#define NX_NAT_INVALID_PROTOCOL                     (NX_NAT_ERROR_CONSTANT | 0x05)      /* Invalid network protocol specified for translation entry.  */ 
#define NX_NAT_ROUTE_FIND_ERROR                     (NX_NAT_ERROR_CONSTANT | 0x06)      /* Nat can not find the suitable interface to send the packet.  */  
#define NX_NAT_INVALID_ENTRY                        (NX_NAT_ERROR_CONSTANT | 0x07)      /* Invalid entry submitted for translation entry list. (e.g. invalid address). */ 
#define NX_NAT_CACHE_FULL                           (NX_NAT_ERROR_CONSTANT | 0x08)      /* NAT translation cache currently is full. */     
#define NX_NAT_NO_FREE_PORT_AVAILABLE               (NX_NAT_ERROR_CONSTANT | 0x09)      /* NAT unable to provide a unique public source port for outbound packet. */   
#define NX_NAT_ZERO_UDP_CHECKSUM                    (NX_NAT_ERROR_CONSTANT | 0x0A)      /* UDP header checksum is zero but NAT not is configured to accept packets with zero UDP checksum. */  
#define NX_NAT_PACKET_CONSUMED_FAILED               (NX_NAT_ERROR_CONSTANT | 0x0B)      /* NAT sonsumed the packets failed. */  
#define NX_NAT_ENTRY_TYPE_ERROR                     (NX_NAT_ERROR_CONSTANT | 0x0C)      /* The entry translation type error. */ 
#define NX_NAT_PORT_UNAVAILABLE                     (NX_NAT_ERROR_CONSTANT | 0x0D)      /* The port is unavailable.          */
               

/* Define the NAT entry attribute, */
#define NX_NAT_STATIC_ENTRY                         1
#define NX_NAT_DYNAMIC_ENTRY                        2

/* NetX NAT translation entry's transaction status levels. */            

/* Define packet type based on direction (inbound, outbound, local). */

#define NX_NAT_INBOUND_PACKET                       1      /* Inbound packet with local host destination on private network */
#define NX_NAT_OUTBOUND_PACKET                      2      /* Outbound packet with external host destination on external network */ 


/* Define the minimum count of NAT entries.  */

#ifndef NX_NAT_MIN_ENTRY_COUNT
#define NX_NAT_MIN_ENTRY_COUNT                      3
#endif


/* Set the default expiration timeout (sec) for translation entries,
   24 hours for TCP sessions, 4 minutes for non-TCP sessions.
   RFC 2663, Section2.6, Page5. */

#ifndef NX_NAT_TCP_SESSION_TIMEOUT
#define NX_NAT_TCP_SESSION_TIMEOUT                  (86400 * NX_IP_PERIODIC_RATE)
#endif

/* For backward compatibility, map the symbol NX_NAT_ENTRY_RESPONSE_TIMEOUT to NX_NAT_NON_TCP_SESSION_TIMEOUT.  */
#ifdef NX_NAT_ENTRY_RESPONSE_TIMEOUT
#define NX_NAT_NON_TCP_SESSION_TIMEOUT              NX_NAT_ENTRY_RESPONSE_TIMEOUT
#endif /* NX_NAT_ENTRY_RESPONSE_TIMEOUT  */

#ifndef NX_NAT_NON_TCP_SESSION_TIMEOUT
#define NX_NAT_NON_TCP_SESSION_TIMEOUT              (240 * NX_IP_PERIODIC_RATE)
#endif /* NX_NAT_NON_TCP_SESSION_TIMEOUT  */

/* Defined, this option enables automatic replacement when NAT cache is full.
   Notice: only replace the oldest non-TCP session.  */
/*
#define NX_NAT_ENABLE_REPLACEMENT
*/


/* Set the ICMP query identifier/port for assigning to outbound ICMP/UDP/TCP packets
   on NAT devices configured for port overloading (sharing a single global IP 
   address). Note this number must be high enough not to exceed with the local host 
   ICMP, UDP, TCP packet query IDs/port. */            
                     
/* Set the minimum TCP port for assigning to outbound TCP packets. */
#ifndef NX_NAT_START_TCP_PORT
#define NX_NAT_START_TCP_PORT                       20000
#endif           

/* Set the maximum TCP port for assigning to outbound TCP packets. */

#ifndef NX_NAT_END_TCP_PORT
#define NX_NAT_END_TCP_PORT                         (NX_NAT_START_TCP_PORT + 10000)
#endif                   
                                  
/* Set the minimum UDP port for assigning to outbound UDP packets. */
#ifndef NX_NAT_START_UDP_PORT
#define NX_NAT_START_UDP_PORT                       20000
#endif        

/* Set the maximum UDP port for assigning to outbound UDP packets. */

#ifndef NX_NAT_END_UDP_PORT
#define NX_NAT_END_UDP_PORT                         (NX_NAT_START_UDP_PORT + 10000)
#endif   
                          
/* Set the minimum ICMP query identifier for assigning to outbound ICMP packets. */
#ifndef NX_NAT_START_ICMP_QUERY_ID
#define NX_NAT_START_ICMP_QUERY_ID                  20000
#endif                                               

/* Set the maximum ICMP query identifier for assigning to outbound ICMP packets. */

#ifndef NX_NAT_END_ICMP_QUERY_ID
#define NX_NAT_END_ICMP_QUERY_ID                    (NX_NAT_START_ICMP_QUERY_ID + 10000)
#endif      


/* Configure NAT to disable record the packet forward counter.  */
/*
#define NX_DISABLE_NAT_INFO
*/

/* Define the NAT translation table entry structure.  */
typedef struct NX_NAT_TRANSLATION_ENTRY_STRUCT
{                  

    /*
      Local Network                                 External Network
                      |----------------|
  <local IP,          |                |<external IP,                        <peer IP, 
       ---------------|                |-----------------------------------------
   local port>        |                | external port>                       peer port>
                      |                |
                      |                |
                      |----------------|
    */
    struct NX_NAT_TRANSLATION_ENTRY_STRUCT  *next_entry_ptr;                /* Pointer to the next translation entry in table */
    ULONG                                   peer_ip_address;                /* IP address of an external host sending/receiving packets through NAT. */
    ULONG                                   local_ip_address;               /* IP address of the local (private) host. */ 
    USHORT                                  peer_port;                      /* Source port of an external host sending/receiving packets through NAT. */ 
    USHORT                                  external_port;                  /* The external port of local (private) host. */
    USHORT                                  local_port;                     /* Port of the local (private) host. */  
    UCHAR                                   translation_type;               /* Translation type (static or dynamic).  */   
    UCHAR                                   protocol;                       /* Packet's network sub protocol (TCP, UDP etc). */
    ULONG                                   response_timeout;               /* Expiration timeout for the entry.     */      
    ULONG                                   response_timestamp;             /* The last timestamp for entry used. */
} NX_NAT_TRANSLATION_ENTRY;
       

/* Define the NAT device structure.  */
typedef struct NX_NAT_DEVICE_STRUCT
{          
       
    ULONG                                  nx_nat_id;                           /* NAT Server thread ID  */   
    NX_IP                                  *nx_nat_ip_ptr;                      /* IP instance for NAT's network. */     
    UCHAR                                  nx_nat_global_interface_index;       /* NAT's global network.  */
    UCHAR                                  reserved[3];                         /* Reserved.            */
#ifndef NX_DISABLE_NAT_INFO
    ULONG                                  forwarded_packets_received;          /* Total number of packets received by NAT. */
    ULONG                                  forwarded_packets_dropped;           /* Total number of packets which cannot be forwarded. */
    ULONG                                  forwarded_packets_sent;              /* Total number of packets sent by NAT. */        
#endif                                                                                                                                           
    NX_NAT_TRANSLATION_ENTRY               *nx_nat_dynamic_available_entry_head;/* Define the head pointer of available dynamic entries list.   */
    NX_NAT_TRANSLATION_ENTRY               *nx_nat_dynamic_active_entry_head;   /* Define the head pointer of active dynamic entries list.      */ 
    UINT                                   nx_nat_dynamic_available_entries;    /* Define the number of available dynamic entries.              */  
    UINT                                   nx_nat_dynamic_active_entries;       /* Define the number of active dynamic entries.                 */        
    UINT                                   nx_nat_static_active_entries;        /* Define the number of active static entries.                  */                        
    VOID                                   (*nx_nat_cache_full_notify)(struct NX_NAT_DEVICE_STRUCT *);                   
} NX_NAT_DEVICE;        


#ifndef     NX_NAT_SOURCE_CODE     


/* Define the system API mappings based on the error checking 
   selected by the user.   */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */


#ifdef NX_NAT_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_nat_create                               _nx_nat_create
#define nx_nat_delete                               _nx_nat_delete            
#define nx_nat_enable                               _nx_nat_enable
#define nx_nat_disable                              _nx_nat_disable
#define nx_nat_cache_notify_set                     _nx_nat_cache_notify_set
#define nx_nat_inbound_entry_create                 _nx_nat_inbound_entry_create
#define nx_nat_inbound_entry_delete                 _nx_nat_inbound_entry_delete

#else

/* Services with error checking.  */

#define nx_nat_create                               _nxe_nat_create
#define nx_nat_delete                               _nxe_nat_delete
#define nx_nat_enable                               _nxe_nat_enable
#define nx_nat_disable                              _nxe_nat_disable
#define nx_nat_cache_notify_set                     _nxe_nat_cache_notify_set
#define nx_nat_inbound_entry_create                 _nxe_nat_inbound_entry_create
#define nx_nat_inbound_entry_delete                 _nxe_nat_inbound_entry_delete

#endif    /* NX_NAT_DISABLE_ERROR_CHECKING */

/* Define API services available for NAT applications. */
                                                                                                                                         
UINT    nx_nat_create(NX_NAT_DEVICE *nat_ptr, NX_IP *ip_ptr, UINT global_interface_index, VOID *dynamic_cache_memory, UINT dynamic_cache_size);                         
UINT    nx_nat_delete(NX_NAT_DEVICE *nat_ptr);    
UINT    nx_nat_enable(NX_NAT_DEVICE *nat_ptr);
UINT    nx_nat_disable(NX_NAT_DEVICE *nat_ptr);
UINT    nx_nat_cache_notify_set(NX_NAT_DEVICE *nat_ptr, VOID (*cache_full_notify_cb)(NX_NAT_DEVICE *nat_ptr));  
UINT    nx_nat_inbound_entry_create(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, ULONG local_ip_address, UINT external_port, USHORT local_port, UCHAR protocol);
UINT    nx_nat_inbound_entry_delete(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *delete_entry_ptr);
                                                                                                          
#else     /* NX_NAT_SOURCE_CODE */

/* NAT source code is being compiled, do not perform any API mapping.  */
                                                                                 
UINT    _nx_nat_create(NX_NAT_DEVICE *nat_ptr, NX_IP *ip_ptr, UINT global_interface_index, VOID *dynamic_cache_memory, UINT dynamic_cache_size);   
UINT    _nxe_nat_create(NX_NAT_DEVICE *nat_ptr, NX_IP *ip_ptr, UINT global_interface_index, VOID *dynamic_cache_memory, UINT dynamic_cache_size);                                
UINT    _nx_nat_delete(NX_NAT_DEVICE *nat_ptr);
UINT    _nxe_nat_delete(NX_NAT_DEVICE *nat_ptr);
UINT    _nx_nat_enable(NX_NAT_DEVICE *nat_ptr);
UINT    _nxe_nat_enable(NX_NAT_DEVICE *nat_pt);      
UINT    _nx_nat_disable(NX_NAT_DEVICE *nat_ptr);
UINT    _nxe_nat_disable(NX_NAT_DEVICE *nat_ptr);
UINT    _nx_nat_cache_notify_set(NX_NAT_DEVICE *nat_ptr, VOID (*cache_full_notify_cb)(NX_NAT_DEVICE *nat_ptr));  
UINT    _nxe_nat_cache_notify_set(NX_NAT_DEVICE *nat_ptr, VOID (*cache_full_notify_cb)(NX_NAT_DEVICE *nat_ptr));
UINT    _nx_nat_inbound_entry_create(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, ULONG local_ip_address, USHORT external_port, USHORT local_port, UCHAR protocol);
UINT    _nxe_nat_inbound_entry_create(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *entry_ptr, ULONG local_ip_address, USHORT external_port, USHORT local_port, UCHAR protocol);
UINT    _nx_nat_inbound_entry_delete(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *delete_entry_ptr);
UINT    _nxe_nat_inbound_entry_delete(NX_NAT_DEVICE *nat_ptr, NX_NAT_TRANSLATION_ENTRY *delete_entry_ptr);
#endif              
               
#endif /* NX_NAT_ENABLE  */

/* If a C++ compiler is being used....*/
#ifdef   __cplusplus
        }
#endif 

#endif /* NX_NAT_H  */

