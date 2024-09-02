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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_tcp.h                                            PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Transmission Control Protocol component, */
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

#ifndef NX_TCP_H
#define NX_TCP_H

#include "nx_api.h"


/* Define TCP constants.  */

#define NX_TCP_ID                       ((ULONG)0x54435020)


/* Define the TCP header typical size.  */

#define NX_TCP_HEADER_SIZE              ((ULONG)0x50000000) /* Typical 5 word TCP header    */
#define NX_TCP_SYN_HEADER               ((ULONG)0x70000000) /* SYN header with MSS option   */
#define NX_TCP_HEADER_SHIFT             28                  /* Shift down to pickup length  */
#define NX_TCP_SYN_OPTION_SIZE          8                   /* 8 bytes of TCP SYN option    */
#define NX_TCP_SYN_SIZE                 (NX_TCP_SYN_OPTION_SIZE + sizeof(NX_TCP_HEADER))


/* Define the TCP header control fields.  */

#define NX_TCP_CONTROL_MASK             ((ULONG)0x00170000) /* ACK, RST, SYN, and FIN bits  */
#define NX_TCP_URG_BIT                  ((ULONG)0x00200000) /* Urgent data bit              */
#define NX_TCP_ACK_BIT                  ((ULONG)0x00100000) /* Acknowledgement bit          */
#define NX_TCP_PSH_BIT                  ((ULONG)0x00080000) /* Push bit                     */
#define NX_TCP_RST_BIT                  ((ULONG)0x00040000) /* Reset bit                    */
#define NX_TCP_SYN_BIT                  ((ULONG)0x00020000) /* Sequence bit                 */
#define NX_TCP_FIN_BIT                  ((ULONG)0x00010000) /* Finish bit                   */


/* Define the MSS option for the TCP header.  */

#define NX_TCP_MSS_OPTION               ((ULONG)0x02040000) /* Maximum Segment Size option  */
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
#define NX_TCP_RWIN_OPTION              ((ULONG)0x03030000) /* 24 bits, so NOP, 0x3, 0x3, scale value  */
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
#define NX_TCP_MSS_SIZE                 1460                /* Maximum Segment Size         */
#define NX_TCP_OPTION_END               ((ULONG)0x01010100) /* NOPs and end of TCP options  */
#define NX_TCP_EOL_KIND                 0x00                /* EOL option kind              */
#define NX_TCP_NOP_KIND                 0x01                /* NOP option kind              */
#define NX_TCP_MSS_KIND                 0x02                /* MSS option kind              */
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
#define NX_TCP_RWIN_KIND                0x03                /* RWIN option kind             */
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */


/* Define constants for the optional TCP keepalive Timer.  To enable this
   feature, the TCP source must be compiled with NX_ENABLE_TCP_KEEPALIVE
   defined.  */

#ifndef NX_TCP_KEEPALIVE_INITIAL
#define NX_TCP_KEEPALIVE_INITIAL        7200        /* Number of seconds for initial */
#endif                                              /*   keepalive expiration, the   */
                                                    /*   default is 2 hours (120 min)*/
#ifndef NX_TCP_KEEPALIVE_RETRY
#define NX_TCP_KEEPALIVE_RETRY          75          /* After initial expiration,     */
#endif                                              /*   retry every 75 seconds      */

#ifndef NX_TCP_KEEPALIVE_RETRIES
#define NX_TCP_KEEPALIVE_RETRIES        10          /* Retry a maximum of 10 times   */
#endif

#ifndef NX_TCP_MAXIMUM_TX_QUEUE
#define NX_TCP_MAXIMUM_TX_QUEUE         20          /* Maximum number of transmit    */
#endif                                              /*   packets queued              */

#ifndef NX_TCP_MAXIMUM_RETRIES
#define NX_TCP_MAXIMUM_RETRIES          10          /* Maximum number of transmit    */
#endif                                              /*   retries allowed             */

#ifndef NX_TCP_RETRY_SHIFT
#define NX_TCP_RETRY_SHIFT              0           /* Shift that is applied to      */
#endif                                              /*   last timeout for back off,  */
                                                    /*   i.e. a value of zero means  */
                                                    /*   constant timeouts, a value  */
                                                    /*   of 1 causes each successive */
                                                    /*   be multiplied by two, etc.  */

#ifndef NX_TCP_MAXIMUM_SEGMENT_LIFETIME
#define NX_TCP_MAXIMUM_SEGMENT_LIFETIME 120         /* Number of seconds for maximum */
#endif                                              /* segment lifetime, the         */
                                                    /* default is 2 minutes (120s)   */


/* Define the maximum receive queue depth for TCP socket. */
#ifdef NX_ENABLE_LOW_WATERMARK
#ifndef NX_TCP_MAXIMUM_RX_QUEUE
#define NX_TCP_MAXIMUM_RX_QUEUE         20
#endif /* NX_TCP_MAXIMUM_RX_QUEUE */
#endif /* NX_ENABLE_LOW_WATERMARK */

/* Define the rate for the TCP fast periodic timer.  This timer is used to process
   delayed ACKs and packet re-transmission.  Hence, it must have greater resolution
   than the 200ms delayed ACK requirement.  By default, the fast periodic timer is
   setup on a 100ms periodic.  The number supplied is used to divide the
   NX_IP_PERIODIC_RATE value to actually derive the ticks.  Dividing
   by 10 yields a 100ms base periodic.  */

#ifndef NX_TCP_FAST_TIMER_RATE
#define NX_TCP_FAST_TIMER_RATE          10
#endif


/* Define the rate for the TCP delayed ACK timer, which by default is 200ms.  The
   number supplied is used to divide the NX_IP_PERIODIC_RATE value to
   actually derive the ticks.  Dividing by 5 yields a 200ms periodic.  */

#ifndef NX_TCP_ACK_TIMER_RATE
#define NX_TCP_ACK_TIMER_RATE           5
#endif

/* Define the rate for the TCP retransmit timer, which by default is set to
   one second.  The number supplied is used to divide the NX_IP_PERIODIC_RATE
   value to actually derive the ticks.  Dividing by 1 yields a 1 second periodic.  */

#ifndef NX_TCP_TRANSMIT_TIMER_RATE
#define NX_TCP_TRANSMIT_TIMER_RATE      1
#endif

/* Define the value of the TCP minimum acceptable MSS for the host to accept the connection,
   which by default is 128.  */

#ifndef NX_TCP_MSS_MINIMUM
#define NX_TCP_MSS_MINIMUM              128
#endif


/* Define Basic TCP packet header data type.  This will be used to
   build new TCP packets and to examine incoming packets into NetX.  */

typedef  struct NX_TCP_HEADER_STRUCT
{

    /* Define the first 32-bit word of the TCP header.  This word contains
       the following information:

            bits 31-16  TCP 16-bit source port number
            bits 15-0   TCP 16-bit destination port number
     */
    ULONG nx_tcp_header_word_0;

    /* Define the second word of the TCP header.  This word contains
       the following information:

            bits 31-0   TCP 32-bit sequence number
     */
    ULONG nx_tcp_sequence_number;

    /* Define the third word of the TCP header.  This word contains
       the following information:

            bits 31-0   TCP 32-bit acknowledgment number
     */
    ULONG nx_tcp_acknowledgment_number;

    /* Define the fourth 32-bit word of the TCP header.  This word contains
       the following information:

            bits 31-28  TCP 4-bit header length
            bits 27-22  TCP 6-bit reserved field
            bit  21     TCP Urgent bit (URG)
            bit  20     TCP Acknowledgement bit (ACK)
            bit  19     TCP Push bit (PSH)
            bit  18     TCP Reset connection bit (RST)
            bit  17     TCP Synchronize sequence numbers bit (SYN)
            bit  16     TCP Sender has reached the end of its byte stream (FIN)
            bits 15-0   TCP 16-bit window size
     */
    ULONG nx_tcp_header_word_3;

    /* Define the fifth 32-bit word of the TCP header.  This word contains
       the following information:

            bits 31-16  TCP 16-bit TCP checksum
            bits 15-0   TCP 16-bit TCP urgent pointer
     */
    ULONG nx_tcp_header_word_4;
} NX_TCP_HEADER;


/* Define TCP component API function prototypes.  */

UINT _nxd_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option);
UINT _nxd_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *peer_ip_address, ULONG *peer_port);
UINT _nx_tcp_client_socket_bind(NX_TCP_SOCKET *socket_ptr, UINT port, ULONG wait_option);
UINT _nx_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, ULONG server_ip, UINT server_port, ULONG wait_option);
UINT _nx_tcp_client_socket_port_get(NX_TCP_SOCKET *socket_ptr, UINT *port_ptr);
UINT _nx_tcp_client_socket_unbind(NX_TCP_SOCKET *socket_ptr);
UINT _nx_tcp_enable(NX_IP *ip_ptr);
UINT _nx_tcp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT _nx_tcp_info_get(NX_IP *ip_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                      ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                      ULONG *tcp_invalid_packets, ULONG *tcp_receive_packets_dropped,
                      ULONG *tcp_checksum_errors, ULONG *tcp_connections,
                      ULONG *tcp_disconnections, ULONG *tcp_connections_dropped,
                      ULONG *tcp_retransmit_packets);
UINT _nx_tcp_server_socket_accept(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT _nx_tcp_server_socket_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr, UINT listen_queue_size,
                                  VOID (*tcp_listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port));
UINT _nx_tcp_server_socket_relisten(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr);
UINT _nx_tcp_server_socket_unaccept(NX_TCP_SOCKET *socket_ptr);
UINT _nx_tcp_server_socket_unlisten(NX_IP *ip_ptr, UINT port);
UINT _nx_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                           ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                           VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                           VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_delete(NX_TCP_SOCKET *socket_ptr);
UINT _nx_tcp_socket_disconnect(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT _nx_tcp_socket_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                             ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                             ULONG *tcp_retransmit_packets, ULONG *tcp_packets_queued,
                             ULONG *tcp_checksum_errors, ULONG *tcp_socket_state,
                             ULONG *tcp_transmit_queue_depth, ULONG *tcp_transmit_window,
                             ULONG *tcp_receive_window);
UINT _nx_tcp_socket_mss_get(NX_TCP_SOCKET *socket_ptr, ULONG *mss);
UINT _nx_tcp_socket_mss_peer_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_mss);
UINT _nx_tcp_socket_mss_set(NX_TCP_SOCKET *socket_ptr, ULONG mss);
UINT _nx_tcp_socket_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT _nx_tcp_socket_receive_notify(NX_TCP_SOCKET *socket_ptr,
                                   VOID (*tcp_receive_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_window_update_notify_set(NX_TCP_SOCKET *socket_ptr,
                                             VOID (*tcp_windows_update_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT _nx_tcp_socket_send_internal(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
UINT _nx_tcp_socket_state_wait(NX_TCP_SOCKET *socket_ptr, UINT desired_state, ULONG wait_option);
UINT _nx_tcp_socket_transmit_configure(NX_TCP_SOCKET *socket_ptr, ULONG max_queue_depth, ULONG timeout,
                                       ULONG max_retries, ULONG timeout_shift);
UINT _nx_tcp_socket_queue_depth_notify_set(NX_TCP_SOCKET *socket_ptr,  VOID (*tcp_socket_queue_depth_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_establish_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_establish_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_disconnect_complete_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_disconnect_complete_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_timed_wait_callback(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_timed_wait_callback)(NX_TCP_SOCKET *socket_ptr));
UINT _nx_tcp_socket_receive_queue_max_set(NX_TCP_SOCKET *socket_ptr, UINT receive_queue_maximum);
#ifdef NX_ENABLE_TCPIP_OFFLOAD
/* Define the direct TCP packet receive processing. This is used with TCP/IP offload feature.  */
VOID _nx_tcp_socket_driver_packet_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr);

/* Define the direct TCP established processing.  This is used with TCP/IP offload feature.  */
UINT _nx_tcp_socket_driver_establish(NX_TCP_SOCKET *socket_ptr, NX_INTERFACE *interface_ptr, UINT remote_port);
UINT _nx_tcp_server_socket_driver_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

/* Define TCP component internal function prototypes.  */
VOID _nx_tcp_cleanup_deferred(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
VOID _nx_tcp_client_bind_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
VOID _nx_tcp_deferred_cleanup_check(NX_IP *ip_ptr);
VOID _nx_tcp_fast_periodic_processing(NX_IP *ip_ptr);
VOID _nx_tcp_socket_retransmit(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, UINT need_fast_retransmit);
VOID _nx_tcp_connect_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
VOID _nx_tcp_disconnect_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
VOID _nx_tcp_initialize(VOID);
UINT _nx_tcp_mss_option_get(UCHAR *option_ptr, ULONG option_area_size, ULONG *mss);
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
UINT _nx_tcp_window_scaling_option_get(UCHAR *option_ptr, ULONG option_area_size, ULONG *window_scale);
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
VOID _nx_tcp_no_connection_reset(NX_IP *ip_ptr, NX_PACKET *packet_ptr, NX_TCP_HEADER *tcp_header_ptr);
VOID _nx_tcp_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_tcp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_tcp_packet_send_ack(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence);
VOID _nx_tcp_packet_send_fin(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence);
VOID _nx_tcp_packet_send_rst(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *header_ptr);
VOID _nx_tcp_packet_send_syn(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence);
VOID _nx_tcp_packet_send_probe(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence, UCHAR data);
VOID _nx_tcp_packet_send_control(NX_TCP_SOCKET *socket_ptr, ULONG control_bits, ULONG tx_sequence,
                                 ULONG ack_number, ULONG option_word_1, ULONG option_word_2, UCHAR *data);
VOID _nx_tcp_periodic_processing(NX_IP *ip_ptr);
VOID _nx_tcp_queue_process(NX_IP *ip_ptr);
VOID _nx_tcp_receive_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);
UINT _nx_tcp_socket_bytes_available(NX_TCP_SOCKET *socket_ptr, ULONG *bytes_available);
VOID _nx_tcp_socket_connection_reset(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_packet_process(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr);
UINT _nx_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_ip_address, ULONG *peer_port);

VOID _nx_tcp_socket_receive_queue_flush(NX_TCP_SOCKET *socket_ptr);
UINT _nx_tcp_socket_state_ack_check(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr);
VOID _nx_tcp_socket_state_closing(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr);
UINT _nx_tcp_socket_state_data_check(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr);
VOID _nx_tcp_socket_state_data_trim_front(NX_PACKET *packet_ptr, ULONG amount);
VOID _nx_tcp_socket_state_data_trim(NX_PACKET *packet_ptr, ULONG amount);
VOID _nx_tcp_socket_state_established(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_state_fin_wait1(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_state_fin_wait2(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_state_last_ack(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr);
VOID _nx_tcp_socket_state_syn_sent(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr, NX_PACKET *packet_ptr);
VOID _nx_tcp_socket_state_syn_received(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr);
VOID _nx_tcp_socket_state_transmit_check(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_thread_resume(TX_THREAD **suspension_list_head, UINT status);
VOID _nx_tcp_socket_thread_suspend(TX_THREAD **suspension_list_head, VOID (*suspend_cleanup)(TX_THREAD * NX_CLEANUP_PARAMETER), NX_TCP_SOCKET *socket_ptr, TX_MUTEX *mutex_ptr, ULONG wait_option);
VOID _nx_tcp_socket_transmit_queue_flush(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_socket_block_cleanup(NX_TCP_SOCKET *socket_ptr);
VOID _nx_tcp_transmit_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);


/* Define error checking shells for TCP API services.  These are only referenced by the
   application.  */

UINT _nxde_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *server_ip, UINT server_port, ULONG wait_option);
UINT _nxde_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *peer_ip_address, ULONG *peer_port);
UINT _nxe_tcp_client_socket_bind(NX_TCP_SOCKET *socket_ptr, UINT port, ULONG wait_option);
UINT _nxe_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, ULONG server_ip, UINT server_port, ULONG wait_option);
UINT _nxe_tcp_client_socket_port_get(NX_TCP_SOCKET *socket_ptr, UINT *port_ptr);
UINT _nxe_tcp_client_socket_unbind(NX_TCP_SOCKET *socket_ptr);
UINT _nxe_tcp_enable(NX_IP *ip_ptr);
UINT _nxe_tcp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT _nxe_tcp_info_get(NX_IP *ip_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                       ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                       ULONG *tcp_invalid_packets, ULONG *tcp_receive_packets_dropped,
                       ULONG *tcp_checksum_errors, ULONG *tcp_connections,
                       ULONG *tcp_disconnections, ULONG *tcp_connections_dropped,
                       ULONG *tcp_retransmit_packets);
UINT _nxe_tcp_server_socket_accept(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT _nxe_tcp_server_socket_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr, UINT listen_queue_size,
                                   VOID (*tcp_listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port));
UINT _nxe_tcp_server_socket_relisten(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr);
UINT _nxe_tcp_server_socket_unaccept(NX_TCP_SOCKET *socket_ptr);
UINT _nxe_tcp_server_socket_unlisten(NX_IP *ip_ptr, UINT port);
UINT _nxe_tcp_socket_bytes_available(NX_TCP_SOCKET *socket_ptr, ULONG *bytes_available);
UINT _nxe_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                            VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                            VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr),
                            UINT tcp_socket_size);
UINT _nxe_tcp_socket_delete(NX_TCP_SOCKET *socket_ptr);
UINT _nxe_tcp_socket_disconnect(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT _nxe_tcp_socket_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                              ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                              ULONG *tcp_retransmit_packets, ULONG *tcp_packets_queued,
                              ULONG *tcp_checksum_errors, ULONG *tcp_socket_state,
                              ULONG *tcp_transmit_queue_depth, ULONG *tcp_transmit_window,
                              ULONG *tcp_receive_window);
UINT _nxe_tcp_socket_mss_get(NX_TCP_SOCKET *socket_ptr, ULONG *mss);
UINT _nxe_tcp_socket_mss_peer_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_mss);
UINT _nxe_tcp_socket_mss_set(NX_TCP_SOCKET *socket_ptr, ULONG mss);
UINT _nxe_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_ip_address, ULONG *peer_port);
UINT _nxe_tcp_socket_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT _nxe_tcp_socket_receive_notify(NX_TCP_SOCKET *socket_ptr,
                                    VOID (*tcp_receive_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nxe_tcp_socket_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr, ULONG wait_option);
UINT _nxe_tcp_socket_state_wait(NX_TCP_SOCKET *socket_ptr, UINT desired_state, ULONG wait_option);
UINT _nxe_tcp_socket_transmit_configure(NX_TCP_SOCKET *socket_ptr, ULONG max_queue_depth, ULONG timeout,
                                        ULONG max_retries, ULONG timeout_shift);
UINT _nxe_tcp_socket_window_update_notify_set(NX_TCP_SOCKET *socket_ptr,
                                              VOID (*tcp_socket_window_update_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nxe_tcp_socket_receive_queue_max_set(NX_TCP_SOCKET *socket_ptr, UINT receive_queue_maximum);
UINT _nxe_tcp_socket_establish_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_establish_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nxe_tcp_socket_disconnect_complete_notify(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_disconnect_complete_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nxe_tcp_socket_queue_depth_notify_set(NX_TCP_SOCKET *socket_ptr,  VOID (*tcp_socket_queue_depth_notify)(NX_TCP_SOCKET *socket_ptr));
UINT _nxe_tcp_socket_timed_wait_callback(NX_TCP_SOCKET *socket_ptr, VOID (*tcp_timed_wait_callback)(NX_TCP_SOCKET *socket_ptr));


/* TCP component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

/*lint -e767 suppress different definitions.  */
#ifdef NX_TCP_INIT
#define TCP_DECLARE
#else
#define TCP_DECLARE extern
#endif
/*lint +e767 enable checking for different definitions.  */

/* Define global data for the TCP component.  */

/* Define the actual number of ticks for the fast periodic timer.  */

TCP_DECLARE ULONG           _nx_tcp_fast_timer_rate;

/* Define the actual number of ticks for the delayed ACK timer.  */

TCP_DECLARE ULONG           _nx_tcp_ack_timer_rate;

/* Define the actual number of ticks for the retransmit timer.  */

TCP_DECLARE ULONG           _nx_tcp_transmit_timer_rate;

/* Define the actual number of ticks for the 2MSL(Maximum Segment Lifetime) timer.  */

TCP_DECLARE ULONG           _nx_tcp_2MSL_timer_rate;


#endif

