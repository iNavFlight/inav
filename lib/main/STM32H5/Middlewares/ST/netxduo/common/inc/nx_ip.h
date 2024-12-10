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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_ip.h                                             PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Internet Protocol component,             */
/*    including all data types and external references.  It is assumed    */
/*    that nx_api.h and nx_port.h have already been included.             */
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

#ifndef NX_IP_H
#define NX_IP_H

#include "nx_api.h"


/* Define IP constants.  */

#define NX_IP_ID                     ((ULONG)0x49502020)



/* Define the mask for the IP header length field.  */

#define NX_IP_LENGTH_MASK            ((ULONG)0x0F000000)       /* Mask for length bit      */



/* Define IP event flags.  These events are processed by the IP thread. */

#define NX_IP_ALL_EVENTS             ((ULONG)0xFFFFFFFF)       /* All event flags              */
#define NX_IP_PERIODIC_EVENT         ((ULONG)0x00000001)       /* Periodic event               */
#define NX_IP_UNFRAG_EVENT           ((ULONG)0x00000002)       /* Unfragment event             */
#define NX_IP_ICMP_EVENT             ((ULONG)0x00000004)       /* ICMP message event           */
#define NX_IP_RECEIVE_EVENT          ((ULONG)0x00000008)       /* IP receive packet event      */
#define NX_IP_ARP_REC_EVENT          ((ULONG)0x00000010)       /* ARP deferred receive event   */
#define NX_IP_RARP_REC_EVENT         ((ULONG)0x00000020)       /* RARP deferred receive event  */
#define NX_IP_IGMP_EVENT             ((ULONG)0x00000040)       /* IGMP message event           */
#define NX_IP_TCP_EVENT              ((ULONG)0x00000080)       /* TCP message event            */
#define NX_IP_FAST_EVENT             ((ULONG)0x00000100)       /* Fast TCP timer event         */
#ifdef NX_DRIVER_DEFERRED_PROCESSING
#define NX_IP_DRIVER_PACKET_EVENT    ((ULONG)0x00000200)       /* Driver Deferred packet event */
#endif /* NX_DRIVER_DEFERRED_PROCESSING */
#define NX_IP_IGMP_ENABLE_EVENT      ((ULONG)0x00000400)       /* IGMP enable event            */
#define NX_IP_DRIVER_DEFERRED_EVENT  ((ULONG)0x00000800)       /* Driver deferred processing   */
                                                               /*   event                      */
#define NX_IP_TCP_CLEANUP_DEFERRED   ((ULONG)0x00001000)       /* Deferred TCP cleanup event   */
#ifdef NX_IPSEC_ENABLE
#define NX_IP_HW_DONE_EVENT          ((ULONG)0x00002000)       /* HW done event                */
#endif /* NX_IPSEC_ENABLE */
#define NX_IP_LINK_STATUS_EVENT      ((ULONG)0x00004000)       /* Link status change event     */


#ifndef NX_IP_FAST_TIMER_RATE
#define NX_IP_FAST_TIMER_RATE        10
#endif

/* Define the amount of time to sleep in nx_ip_(interface_)status_check */
#ifndef NX_IP_STATUS_CHECK_WAIT_TIME
#define NX_IP_STATUS_CHECK_WAIT_TIME 1
#endif /* NX_IP_STATUS_CHECK_WAIT_TIME */

#include "nx_ipv4.h"



/* Define IP function prototypes.  */
UINT   _nx_ip_auxiliary_packet_pool_set(NX_IP *ip_ptr, NX_PACKET_POOL *auxiliary_pool);
USHORT _nx_ip_checksum_compute(NX_PACKET *packet_ptr, ULONG protocol, UINT data_length,
                               ULONG *_src_ip_addr, ULONG *_dest_ip_addr);
UINT   _nx_ip_interface_address_mapping_configure(NX_IP *ip_ptr, UINT interface_index, UINT mapping_needed);
UINT   _nx_ip_interface_capability_get(NX_IP *ip_ptr, UINT interface_index, ULONG *interface_capability_flag);
UINT   _nx_ip_interface_capability_set(NX_IP *ip_ptr, UINT interface_index, ULONG interface_capability_flag);
UINT   _nx_ip_interface_info_get(NX_IP *ip_ptr, UINT interface_index, CHAR **interface_name, ULONG *ip_address,
                                 ULONG *network_mask, ULONG *mtu_size, ULONG *phsyical_address_msw,
                                 ULONG *physical_address_lsw);
UINT _nx_ip_interface_mtu_set(NX_IP *ip_ptr, UINT interface_index, ULONG mtu_size);
UINT _nx_ip_interface_physical_address_get(NX_IP *ip_ptr, UINT interface_index, ULONG *physical_msw, ULONG *physical_lsw);
UINT _nx_ip_interface_physical_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG physical_msw, ULONG physical_lsw, UINT update_driver);
UINT _nx_ip_interface_status_check(NX_IP *ip_ptr, UINT interface_index, ULONG needed_status, ULONG *actual_status,
                                   ULONG wait_option);
UINT _nx_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                   NX_PACKET_POOL *default_pool, VOID (*ip_link_driver)(NX_IP_DRIVER *),
                   VOID *memory_ptr, ULONG memory_size, UINT priority);
UINT _nx_ip_delete(NX_IP *ip_ptr);
VOID _nx_ip_delete_queue_clear(NX_PACKET *head_ptr);
VOID _nx_ip_deferred_link_status_process(NX_IP *ip_ptr);
VOID _nx_ip_driver_link_status_event(NX_IP *ip_ptr, UINT interface_index);
VOID _nx_ip_driver_deferred_enable(NX_IP *ip_ptr, VOID (*driver_deferred_packet_handler)(NX_IP *ip_ptr, NX_PACKET *packet_ptr));
VOID _nx_ip_driver_deferred_processing(NX_IP *ip_ptr);
VOID _nx_ip_driver_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
UINT _nx_ip_driver_direct_command(NX_IP *ip_ptr, UINT command, ULONG *return_value_ptr);
UINT _nx_ip_driver_interface_direct_command(NX_IP *ip_ptr, UINT command, UINT interface_index, ULONG *return_value_ptr);


UINT _nx_ip_forwarding_disable(NX_IP *ip_ptr);
UINT _nx_ip_forwarding_enable(NX_IP *ip_ptr);
UINT _nx_ip_fragment_disable(NX_IP *ip_ptr);
UINT _nx_ip_fragment_enable(NX_IP *ip_ptr);
UINT _nx_ip_info_get(NX_IP *ip_ptr, ULONG *ip_total_packets_sent, ULONG *ip_total_bytes_sent,
                     ULONG *ip_total_packets_received, ULONG *ip_total_bytes_received,
                     ULONG *ip_invalid_packets, ULONG *ip_receive_packets_dropped,
                     ULONG *ip_receive_checksum_errors, ULONG *ip_send_packets_dropped,
                     ULONG *ip_total_fragments_sent, ULONG *ip_total_fragments_received);
UINT _nx_ip_interface_attach(NX_IP *ip_ptr, CHAR *interface_name, ULONG ip_address, ULONG network_mask, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *));
UINT _nx_ip_interface_detach(NX_IP *ip_ptr, UINT index);
UINT _nx_ip_max_payload_size_find(NX_IP *ip_ptr, NXD_ADDRESS *dest_address, UINT if_index,
                                  UINT src_port, UINT dest_port, ULONG protocol, ULONG *start_offset_ptr,
                                  ULONG *payload_length_ptr);
UINT _nx_ip_raw_packet_disable(NX_IP *ip_ptr);
UINT _nx_ip_raw_packet_enable(NX_IP *ip_ptr);
UINT _nx_ip_raw_packet_filter_set(NX_IP *ip_ptr,  UINT (*raw_packet_filter)(NX_IP *, ULONG, NX_PACKET *));
UINT _nx_ip_raw_receive_queue_max_set(NX_IP *ip_ptr, ULONG queue_max);
UINT _nx_ip_raw_packet_receive(NX_IP *ip_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT _nxd_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *destination_ip, UINT address_index, ULONG protocol, UINT ttl, ULONG tos);
VOID _nx_ip_initialize(VOID);
VOID _nx_ip_periodic_timer_entry(ULONG ip_address);
VOID _nx_ip_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_ip_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
UINT _nx_ip_status_check(NX_IP *ip_ptr, ULONG needed_status, ULONG *actual_status, ULONG wait_option);
UINT _nx_ip_link_status_change_notify_set(NX_IP *ip_ptr,  VOID (*link_status_change_notify)(NX_IP *ip_ptr, UINT interface_index, UINT link_up));
VOID _nx_ip_thread_entry(ULONG ip_ptr_value);
VOID _nx_ip_raw_packet_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
UINT _nx_ip_raw_packet_processing(NX_IP *ip_ptr, ULONG protocol, NX_PACKET *packet_ptr);
UINT _nxd_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                             NXD_ADDRESS *destination_ip, ULONG protocol, UINT ttl, ULONG tos);
#ifndef NX_DISABLE_FRAGMENTATION
VOID _nx_ip_fragment_timeout_check(NX_IP *ip_ptr);
VOID _nx_ip_fragment_packet(struct NX_IP_DRIVER_STRUCT *driver_req_ptr);
VOID _nx_ip_fragment_assembly(NX_IP *ip_ptr);
#endif /* NX_DISABLE_FRAGMENTATION */
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
VOID _nx_ip_packet_checksum_compute(NX_PACKET *packet_ptr);
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT _nxe_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                    NX_PACKET_POOL *default_pool,
                    VOID (*ip_link_driver)(NX_IP_DRIVER *),
                    VOID *memory_ptr, ULONG memory_size, UINT priority, UINT ip_control_block_size);
UINT _nxe_ip_delete(NX_IP *ip_ptr);
UINT _nxe_ip_driver_direct_command(NX_IP *ip_ptr, UINT command, ULONG *return_value_ptr);
UINT _nxe_ip_driver_interface_direct_command(NX_IP *ip_ptr, UINT command, UINT interface_index, ULONG *return_value_ptr);
UINT _nxe_ip_auxiliary_packet_pool_set(NX_IP *ip_ptr, NX_PACKET_POOL *auxiliary_pool);


UINT _nxe_ip_forwarding_disable(NX_IP *ip_ptr);
UINT _nxe_ip_forwarding_enable(NX_IP *ip_ptr);
UINT _nxe_ip_fragment_disable(NX_IP *ip_ptr);
UINT _nxe_ip_fragment_enable(NX_IP *ip_ptr);
UINT _nxe_ip_info_get(NX_IP *ip_ptr, ULONG *ip_total_packets_sent, ULONG *ip_total_bytes_sent,
                      ULONG *ip_total_packets_received, ULONG *ip_total_bytes_received,
                      ULONG *ip_invalid_packets, ULONG *ip_receive_packets_dropped,
                      ULONG *ip_receive_checksum_errors, ULONG *ip_send_packets_dropped,
                      ULONG *ip_total_fragments_sent, ULONG *ip_total_fragments_received);
UINT _nxe_ip_interface_attach(NX_IP *ip_ptr, CHAR *interface_name, ULONG ip_address, ULONG network_mask, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *));
UINT _nxe_ip_interface_detach(NX_IP *ip_ptr, UINT index);
UINT _nxe_ip_max_payload_size_find(NX_IP *ip_ptr, NXD_ADDRESS *dest_address, UINT if_index,
                                   UINT src_port, UINT dest_port, ULONG protocol, ULONG *start_offset_ptr,
                                   ULONG *payload_length_ptr);
UINT _nxe_ip_raw_packet_disable(NX_IP *ip_ptr);
UINT _nxe_ip_raw_packet_enable(NX_IP *ip_ptr);
UINT _nxe_ip_raw_packet_receive(NX_IP *ip_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT _nxde_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *destination_ip, UINT address_index, ULONG protocol, UINT ttl, ULONG tos);
UINT _nxe_ip_status_check(NX_IP *ip_ptr, ULONG needed_status, ULONG *actual_status,
                          ULONG wait_option);

UINT _nxe_ip_link_status_change_notify_set(NX_IP *ip_ptr,  VOID (*link_status_change_notify)(NX_IP *ip_ptr, UINT interface_index, UINT link_up));
UINT _nxe_ip_interface_address_mapping_configure(NX_IP *ip_ptr, UINT interface_index, UINT mapping_needed);
UINT _nxe_ip_interface_capability_get(NX_IP *ip_ptr, UINT interface_index, ULONG *interface_capability_flag);
UINT _nxe_ip_interface_capability_set(NX_IP *ip_ptr, UINT interface_index, ULONG interface_capability_flag);
UINT _nxe_ip_interface_info_get(NX_IP *ip_ptr, UINT interface_index, CHAR **interface_name,
                                ULONG *ip_address, ULONG *network_mask, ULONG *mtu_size,
                                ULONG *physical_address_msw, ULONG *physical_address_lsw);
UINT _nxe_ip_interface_mtu_set(NX_IP *ip_ptr, UINT interface_index, ULONG mtu_size);
UINT _nxe_ip_interface_physical_address_get(NX_IP *ip_ptr, UINT interface_index,
                                            ULONG *physical_msw, ULONG *physical_lsw);
UINT _nxe_ip_interface_physical_address_set(NX_IP *ip_ptr, UINT interface_index,
                                            ULONG physical_msw, ULONG physical_lsw, UINT update_driver);
UINT _nxe_ip_interface_status_check(NX_IP *ip_ptr, UINT interface_index, ULONG needed_status,
                                    ULONG *actual_status, ULONG wait_option);
UINT _nxe_ip_raw_packet_filter_set(NX_IP *ip_ptr,
                                   UINT (*raw_packet_filter)(NX_IP *, ULONG, NX_PACKET *));
UINT _nxe_ip_raw_receive_queue_max_set(NX_IP *ip_ptr, ULONG queue_max);


VOID _nx_ip_fast_periodic_timer_create(NX_IP *ip_ptr);

UINT _nx_ip_dispatch_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT protocol);

/* IP component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

/*lint -e767 suppress different definitions.  */
#ifdef NX_IP_INIT
#define IP_DECLARE
#else
#define IP_DECLARE extern
#endif
/*lint +e767 enable checking for different definitions.  */


/* Define the head pointer of the created IP list.  */

IP_DECLARE  NX_IP *_nx_ip_created_ptr;


/* Define the number of created IP instances.  */

IP_DECLARE  ULONG _nx_ip_created_count;


#endif

