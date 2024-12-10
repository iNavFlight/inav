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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_udp.h                                            PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX User Datagram Protocol (UDP) component,  */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_UDP_H
#define NX_UDP_H

#include "nx_api.h"


/* Define UDP constants.  */

#define NX_UDP_ID ((ULONG)0x55445020)


/* Define Basic UDP packet header data type.  This will be used to
   build new UDP packets and to examine incoming packets into NetX.  */

typedef struct NX_UDP_HEADER_STRUCT
{

    /* Define the first 32-bit word of the UDP header.  This word contains
       the following information:

            bits 31-16  UDP 16-bit source port number
            bits 15-0   UDP 16-bit destination port number
     */
    ULONG nx_udp_header_word_0;

    /* Define the second and final word of the UDP header.  This word contains
       the following information:

            bits 31-16  UDP 16-bit UDP length (including 8 header bytes)
            bits 15-0   UDP 16-bit checksum (including header and pseudo IP header)
     */
    ULONG nx_udp_header_word_1;
} NX_UDP_HEADER;


/* Define UDP component function prototypes.  */
UINT _nxd_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT port);

VOID _nx_udp_bind_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
UINT _nx_udp_enable(NX_IP *ip_ptr);
UINT _nx_udp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT _nx_udp_info_get(NX_IP *ip_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                      ULONG *udp_packets_received, ULONG *udp_bytes_received,
                      ULONG *udp_invalid_packets, ULONG *udp_receive_packets_dropped,
                      ULONG *udp_checksum_errors);
VOID _nx_udp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_udp_receive_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
UINT _nx_udp_socket_bind(NX_UDP_SOCKET *socket_ptr, UINT  port, ULONG wait_option);
UINT _nx_udp_socket_bytes_available(NX_UDP_SOCKET *socket_ptr, ULONG *bytes_available);
UINT _nx_udp_socket_checksum_disable(NX_UDP_SOCKET *socket_ptr);
UINT _nx_udp_socket_checksum_enable(NX_UDP_SOCKET *socket_ptr);
UINT _nx_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                           ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG queue_maximum);
UINT _nx_udp_socket_delete(NX_UDP_SOCKET *socket_ptr);
UINT _nx_udp_socket_info_get(NX_UDP_SOCKET *socket_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                             ULONG *udp_packets_received, ULONG *udp_bytes_received, ULONG *udp_packets_queued,
                             ULONG *udp_receive_packets_dropped, ULONG *udp_checksum_errors);
UINT _nxd_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT port, UINT address_index);
UINT _nxde_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT port, UINT address_index);
UINT _nx_udp_socket_port_get(NX_UDP_SOCKET *socket_ptr, UINT *port_ptr);
UINT _nx_udp_socket_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr,
                            ULONG wait_option);
UINT _nx_udp_socket_receive_notify(NX_UDP_SOCKET *socket_ptr,
                                   VOID (*udp_receive_notify)(NX_UDP_SOCKET *socket_ptr));

UINT _nx_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                         ULONG ip_address, UINT port);
UINT _nx_udp_socket_unbind(NX_UDP_SOCKET *socket_ptr);
UINT _nx_udp_source_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *port);
UINT _nx_udp_packet_info_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *protocol, UINT *port, UINT *interface_index);
UINT _nxd_udp_source_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT *port);
#ifdef NX_ENABLE_TCPIP_OFFLOAD
/* Define the direct UDP packet receive processing. This is used with TCP/IP offload feature.  */
VOID _nx_udp_socket_driver_packet_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                          NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip, UINT remote_port);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT _nxe_udp_enable(NX_IP *ip_ptr);
UINT _nxe_udp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT _nxe_udp_info_get(NX_IP *ip_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                       ULONG *udp_packets_received, ULONG *udp_bytes_received,
                       ULONG *udp_invalid_packets, ULONG *udp_receive_packets_dropped,
                       ULONG *udp_checksum_errors);
UINT _nxe_udp_socket_bind(NX_UDP_SOCKET *socket_ptr, UINT  port, ULONG wait_option);
UINT _nxe_udp_socket_bytes_available(NX_UDP_SOCKET *socket_ptr, ULONG *bytes_available);
UINT _nxe_udp_socket_checksum_disable(NX_UDP_SOCKET *socket_ptr);
UINT _nxe_udp_socket_checksum_enable(NX_UDP_SOCKET *socket_ptr);
UINT _nxe_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG queue_maximum, UINT udp_socket_size);
UINT _nxe_udp_socket_delete(NX_UDP_SOCKET *socket_ptr);

UINT _nxe_udp_source_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *port);
UINT _nxde_udp_source_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT *port);
UINT _nxe_udp_socket_info_get(NX_UDP_SOCKET *socket_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                              ULONG *udp_packets_received, ULONG *udp_bytes_received, ULONG *udp_packets_queued,
                              ULONG *udp_receive_packets_dropped, ULONG *udp_checksum_errors);
UINT _nxe_udp_socket_port_get(NX_UDP_SOCKET *socket_ptr, UINT *port_ptr);
UINT _nxe_udp_socket_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr,
                             ULONG wait_option);
UINT _nxe_udp_socket_receive_notify(NX_UDP_SOCKET *socket_ptr,
                                    VOID (*udp_receive_notify)(NX_UDP_SOCKET *socket_ptr));
UINT _nx_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                ULONG ip_address, UINT port, UINT address_index);
UINT _nxe_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr,
                                 ULONG ip_address, UINT port, UINT source_index);
UINT _nxe_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr,
                          ULONG ip_address, UINT port);
UINT _nxe_udp_socket_unbind(NX_UDP_SOCKET *socket_ptr);



UINT _nxd_udp_packet_info_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address,
                                  UINT *protocol, UINT *port, UINT *interface_index);
UINT _nxde_udp_packet_info_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address,
                                   UINT *protocol, UINT *port, UINT *interface_index);
UINT _nxe_udp_packet_info_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *protocol, UINT *port, UINT *interface_index);
UINT _nxde_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, NXD_ADDRESS *ip_address, UINT port);

/* UDP component data declarations follow.  */

#endif

