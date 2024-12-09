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
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_packet.h                                         PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX packet memory management component,      */
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

#ifndef NX_PAC_H
#define NX_PAC_H

#include "nx_api.h"


#define NX_PACKET_POOL_ID           ((ULONG)0x5041434B)


/* Define constants for packet free, allocated, enqueued, and driver transmit done.
   These will be used in the nx_packet_tcp_queue_next field to indicate the state of
   the packet.  */

#define NX_PACKET_FREE              ((ALIGN_TYPE)0xFFFFFFFF) /* Packet is available and in the pool  */
#define NX_PACKET_ALLOCATED         ((ALIGN_TYPE)0xAAAAAAAA) /* Packet has been allocated            */
#define NX_PACKET_ENQUEUED          ((ALIGN_TYPE)0xEEEEEEEE) /* Packet is the tail of TCP queue.     */
                                                             /* A value that is none of the above    */
                                                             /*   also indicates the packet is in a  */
                                                             /*   TCP queue                          */

/* Define the constant for driver done and receive packet is available. These will be
   used in the nx_packet_queue_next field to indicate the state of a TCP packet.  */

#define NX_DRIVER_TX_DONE           ((ALIGN_TYPE)0xDDDDDDDD) /* Driver has sent the TCP packet       */
#define NX_PACKET_READY             ((ALIGN_TYPE)0xBBBBBBBB) /* Packet is ready for retrieval        */


#ifdef NX_ENABLE_PACKET_DEBUG_INFO
/* Define strings for packet debug information. */
#define NX_PACKET_ARP_WAITING_QUEUE "ARP waiting queue"
#define NX_PACKET_ND_WAITING_QUEUE  "ND waiting queue"
#define NX_PACKET_TCP_LISTEN_QUEUE  "TCP listen queue"
#define NX_PACKET_TCP_RECEIVE_QUEUE "TCP receive queue"
#define NX_PACKET_UDP_RECEIVE_QUEUE "UDP receive queue"
#define NX_PACKET_IP_FRAGMENT_QUEUE "IP fragment queue"
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */


/* Define packet pool management function prototypes.  */

UINT _nx_packet_allocate(NX_PACKET_POOL *pool_ptr,  NX_PACKET **packet_ptr,
                         ULONG packet_type, ULONG wait_option);
UINT _nx_packet_copy(NX_PACKET *packet_ptr, NX_PACKET **new_packet_ptr,
                     NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT _nx_packet_data_append(NX_PACKET *packet_ptr, VOID *data_start, ULONG data_size,
                            NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT _nx_packet_data_extract_offset(NX_PACKET *packet_ptr, ULONG offset, VOID *buffer_start, ULONG buffer_length, ULONG *bytes_copied);
UINT _nx_packet_data_retrieve(NX_PACKET *packet_ptr, VOID *buffer_start, ULONG *bytes_copied);
UINT _nx_packet_data_adjust(NX_PACKET *packet_ptr, ULONG header_size);
#ifdef NX_ENABLE_PACKET_DEBUG_INFO
UINT _nx_packet_debug_info_get(NX_PACKET_POOL *pool_ptr, UINT packet_index, NX_PACKET **packet_pptr,
                               ULONG *packet_status, CHAR **thread_info, CHAR **file_info, ULONG *line);
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */
UINT _nx_packet_length_get(NX_PACKET *packet_ptr, ULONG *length);
UINT _nx_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name, ULONG payload_size,
                            VOID *memory_ptr, ULONG memory_size);
UINT _nx_packet_pool_delete(NX_PACKET_POOL *pool_ptr);
UINT _nx_packet_pool_info_get(NX_PACKET_POOL *pool_ptr, ULONG *total_packets, ULONG *free_packets,
                              ULONG *empty_pool_requests, ULONG *empty_pool_suspensions,
                              ULONG *invalid_packet_releases);
UINT _nx_packet_release(NX_PACKET *packet_ptr);
UINT _nx_packet_transmit_release(NX_PACKET *packet_ptr);
VOID _nx_packet_pool_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
VOID _nx_packet_pool_initialize(VOID);
UINT _nx_packet_pool_low_watermark_set(NX_PACKET_POOL *pool_ptr, ULONG low_watermark);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT _nxe_packet_allocate(NX_PACKET_POOL *pool_ptr,  NX_PACKET **packet_ptr,
                          ULONG packet_type, ULONG wait_option);
UINT _nxe_packet_copy(NX_PACKET *packet_ptr, NX_PACKET **new_packet_ptr,
                      NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT _nxe_packet_data_append(NX_PACKET *packet_ptr, VOID *data_start, ULONG data_size,
                             NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT _nxe_packet_data_extract_offset(NX_PACKET *packet_ptr, ULONG offset, VOID *buffer_start, ULONG buffer_length, ULONG *bytes_copied);
UINT _nxe_packet_data_retrieve(NX_PACKET *packet_ptr, VOID *buffer_start, ULONG *bytes_copied);
UINT _nxe_packet_length_get(NX_PACKET *packet_ptr, ULONG *length);
UINT _nxe_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name, ULONG payload_size,
                             VOID *memory_ptr, ULONG memory_size, UINT pool_control_block_size);
UINT _nxe_packet_pool_delete(NX_PACKET_POOL *pool_ptr);
UINT _nxe_packet_pool_info_get(NX_PACKET_POOL *pool_ptr, ULONG *total_packets, ULONG *free_packets,
                               ULONG *empty_pool_requests, ULONG *empty_pool_suspensions,
                               ULONG *invalid_packet_releases);
UINT _nxe_packet_release(NX_PACKET **packet_ptr_ptr);
UINT _nxe_packet_transmit_release(NX_PACKET **packet_ptr_ptr);
UINT _nxe_packet_pool_low_watermark_set(NX_PACKET_POOL *pool_ptr, ULONG low_watermark);


/* Packet pool management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

/*lint -e767 suppress different definitions.  */
#ifdef NX_PACKET_POOL_INIT
#define PACKET_POOL_DECLARE
#else
#define PACKET_POOL_DECLARE extern
#endif
/*lint +e767 enable checking for different definitions.  */


/* Define the head pointer of the created packet pool list.  */

PACKET_POOL_DECLARE  NX_PACKET_POOL     *_nx_packet_pool_created_ptr;


/* Define the variable that holds the number of created packet pools.  */

PACKET_POOL_DECLARE  ULONG _nx_packet_pool_created_count;


#endif

