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
/**   Point-to-Point Protocol (PPP)                                       */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nx_ppp.h                                            PORTABLE C      */  
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Point-to-Point Protocol (PPP)            */ 
/*    component, including all data types and external references.        */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included.                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_PPP_H
#define NX_PPP_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the PPP ID.  */

#define NX_PPP_ID                                           0x50505020UL

#define NX_PPP_SERIAL_BUFFER_ALERT_THRESHOLD                NX_PPP_SERIAL_BUFFER_SIZE/4

/* Defined, support for transmitting PPP over Ethernet.  */
/*
#define NX_PPP_PPPOE_ENABLE
*/

/* If defined, this removes logic for compiling PPP transmit and receive statistics. 
#define NX_PPP_DISABLE_INFO
*/

/* If defined, this enables PPP event logging.
#define NX_PPP_DEBUG_LOG_ENABLE
*/

/* If defined, this enables data saved to the PPP log to be printed out (printf).  
#define NX_PPP_DEBUG_LOG_PRINT_ENABLE
*/

/* If defined, this disables CHAP authentication. 
#define NX_PPP_DISABLE_CHAP
*/

/* If defined, this disables PAP authentication. 
#define NX_PPP_DISABLE_PAP
*/

/* If defined, the primary DNS address request option is not set in NAKed list. 
#define NX_PPP_DNS_OPTION_DISABLE
*/

/* Define how many times the PPP should try to request DNS address from the Peer. This
   will have no effect if NX_PPP_DNS_OPTION_DISABLE is defined. */
#ifndef NX_PPP_DNS_ADDRESS_MAX_RETRIES
#define NX_PPP_DNS_ADDRESS_MAX_RETRIES                      2 
#endif

/* Define the thread time slice.  */
#ifndef NX_PPP_THREAD_TIME_SLICE
#define NX_PPP_THREAD_TIME_SLICE                            TX_NO_TIME_SLICE
#endif

/* Set the link transfer size. */
#ifndef NX_PPP_MRU
#ifdef NX_PPP_PPPOE_ENABLE
#define NX_PPP_MRU                                          1480        /* Minimum value!  */
#else /* !NX_PPP_PPPOE_ENABLE  */
#define NX_PPP_MRU                                          1500        /* Minimum value!  */
#endif /* NX_PPP_PPPOE_ENABLE  */
#endif /* NX_PPP_MRU  */

/* Minimum MRU to accept in  MRU parsed from received LCP configuration request. */
#ifndef NX_PPP_MINIMUM_MRU
#ifdef NX_PPP_PPPOE_ENABLE
#define NX_PPP_MINIMUM_MRU                                  1480
#else /* !NX_PPP_PPPOE_ENABLE  */
#define NX_PPP_MINIMUM_MRU                                  1500
#endif /* NX_PPP_PPPOE_ENABLE  */
#endif /* NX_PPP_MINIMUM_MRU  */

/* Size of the receive character buffer. */
#ifndef NX_PPP_SERIAL_BUFFER_SIZE
#define NX_PPP_SERIAL_BUFFER_SIZE                           NX_PPP_MRU*2
#endif

  
/* User name buffer size for PAP login.  */
#ifndef NX_PPP_NAME_SIZE
#define NX_PPP_NAME_SIZE                                    32
#endif

/* Password buffer size for PAP login.  */
#ifndef NX_PPP_PASSWORD_SIZE
#define NX_PPP_PASSWORD_SIZE                                32
#endif

/* Buffer size of the random value to process the CHAP challenge name */
#ifndef NX_PPP_VALUE_SIZE
#define NX_PPP_VALUE_SIZE                                   32
#endif

/* Buffer size of the hash value to process the CHAP challenge name */
#ifndef NX_PPP_HASHED_VALUE_SIZE
#define NX_PPP_HASHED_VALUE_SIZE                            16
#endif


/* The time period in timer ticks at which the PPP timer function executes and wakes up the PPP processing thread
   to check for PPP events. */
#ifndef NX_PPP_BASE_TIMEOUT
#define NX_PPP_BASE_TIMEOUT                                 (NX_IP_PERIODIC_RATE * 1)        /* 1 second  */
#endif

/* The time out in timer ticks on allocating packets for processing PPP data into IP packets either receiving or sending. */
#ifndef NX_PPP_TIMEOUT
#define NX_PPP_TIMEOUT                                      (NX_IP_PERIODIC_RATE * 4)       /* 4 seconds  */
#endif

/* Number of times the PPP task times out waiting for another byte in the serial buffer. On reaching
   this value, the PPP instance releases the packet and allocates a fresh packet for a new message.  */
#ifndef NX_PPP_RECEIVE_TIMEOUTS
#define NX_PPP_RECEIVE_TIMEOUTS                             4
#endif

/* Timeout in seconds on the PPP task to receive a response to PPP protocol request.  */
#ifndef NX_PPP_PROTOCOL_TIMEOUT
#define NX_PPP_PROTOCOL_TIMEOUT                             4                            /* 4 seconds.  */
#endif

/* Size of debug entry, this is also the wrap around index for debug output to overwrite oldest data. */
#ifndef NX_PPP_DEBUG_LOG_SIZE
#define NX_PPP_DEBUG_LOG_SIZE                               50
#endif

/* Maximum amount of data to add to debug output from received packet. */
#ifndef NX_PPP_DEBUG_FRAME_SIZE         
#define NX_PPP_DEBUG_FRAME_SIZE                             50
#endif

/* Size of the NAK list. Not in use. */
#ifndef NX_PPP_OPTION_MESSAGE_LENGTH
#define NX_PPP_OPTION_MESSAGE_LENGTH                        64              
#endif

/* Number of instances the PPP instance resends another LCP configure request message without a response. 
   When this number is reached, the PPP instance aborts the PPP handshake, and the link status is down. */
#ifndef NX_PPP_MAX_LCP_PROTOCOL_RETRIES
#define NX_PPP_MAX_LCP_PROTOCOL_RETRIES                     20
#endif

/* Number of instances the PPP instance resends another PAP authentication request message without a response. 
   When this number is reached, the PPP instance aborts the PPP handshake, and the link status is down. */
#ifndef NX_PPP_MAX_PAP_PROTOCOL_RETRIES
#define NX_PPP_MAX_PAP_PROTOCOL_RETRIES                     20
#endif

/* Number of instances the PPP instance resends another CHAP challenge message without a response. 
   When this number is reached, the PPP instance aborts the PPP handshake, and the link status is down. */
#ifndef NX_PPP_MAX_CHAP_PROTOCOL_RETRIES
#define NX_PPP_MAX_CHAP_PROTOCOL_RETRIES                    20
#endif

/* Number of instances the PPP instance times out before the IPCP part of the PPP handshake.
   When this number is reached, the PPP instance aborts and the link is down. */
#ifndef NX_PPP_MAX_IPCP_PROTOCOL_RETRIES
#define NX_PPP_MAX_IPCP_PROTOCOL_RETRIES                    20
#endif

/* Define the packet header.  */
#ifdef NX_PPP_PPPOE_ENABLE
#ifndef NX_PPP_PACKET
#define NX_PPP_PACKET                                       22  /* Ethernet Header(14) + (PPPoE Header(6) + 2), keep four-byte alignment for Ethernet.  */
#endif /* NX_PPP_PACKET  */
#else /* !NX_PPP_PPPOE_ENABLE  */
#define NX_PPP_PACKET                                       0
#endif /* NX_PPP_PPPOE_ENABLE  */

/* Define the minimum PPP packet payload, the PPP commands (LCP, PAP, CHAP, IPCP) should be in one packet.  */

#ifndef NX_PPP_MIN_PACKET_PAYLOAD
#define NX_PPP_MIN_PACKET_PAYLOAD                           (NX_PPP_PACKET + 128)
#endif

/* Define PPP protocol types.  */

#define NX_PPP_LCP_PROTOCOL                                 0xC021
#define NX_PPP_IPCP_PROTOCOL                                0x8021
#define NX_PPP_PAP_PROTOCOL                                 0xC023
#define NX_PPP_CHAP_PROTOCOL                                0xC223
#define NX_PPP_DATA                                         0x0021
            

/* Define PPP LCP codes and action.   */

#define NX_PPP_LCP_CONFIGURE_REQUEST                        1
#define NX_PPP_LCP_CONFIGURE_ACK                            2
#define NX_PPP_LCP_CONFIGURE_NAK                            3
#define NX_PPP_LCP_CONFIGURE_REJECT                         4
#define NX_PPP_LCP_TERMINATE_REQUEST                        5
#define NX_PPP_LCP_TERMINATE_ACK                            6
#define NX_PPP_LCP_CODE_REJECT                              7
#define NX_PPP_LCP_PROTOCOL_REJECT                          8
#define NX_PPP_LCP_ECHO_REQUEST                             9   
#define NX_PPP_LCP_ECHO_REPLY                               10
#define NX_PPP_LCP_DISCARD_REQUEST                          11
#define NX_PPP_LCP_TIMEOUT                                  99


/* Define PPP PAP codes and action.  */

#define NX_PPP_PAP_AUTHENTICATE_REQUEST                     1
#define NX_PPP_PAP_AUTHENTICATE_ACK                         2
#define NX_PPP_PAP_AUTHENTICATE_NAK                         3
#define NX_PPP_PAP_AUTHENTICATE_TIMEOUT                     99


/* Define PPP CHAP codes and action.  */

#define NX_PPP_CHAP_CHALLENGE_REQUEST                       1
#define NX_PPP_CHAP_CHALLENGE_RESPONSE                      2
#define NX_PPP_CHAP_CHALLENGE_SUCCESS                       3
#define NX_PPP_CHAP_CHALLENGE_FAILURE                       4
#define NX_PPP_CHAP_CHALLENGE_TIMEOUT                       99


/* Define PPP IPCP codes and action.   */

#define NX_PPP_IPCP_CONFIGURE_REQUEST                       1
#define NX_PPP_IPCP_CONFIGURE_ACK                           2
#define NX_PPP_IPCP_CONFIGURE_NAK                           3
#define NX_PPP_IPCP_CONFIGURE_REJECT                        4
#define NX_PPP_IPCP_TERMINATE_REQUEST                       5
#define NX_PPP_IPCP_TERMINATE_ACK                           6
#define NX_PPP_IPCP_TIMEOUT                                 99


/* Define API return codes.  */

#define NX_PPP_FAILURE                                      0xb0
#define NX_PPP_BUFFER_FULL                                  0xb1
#define NX_PPP_BAD_PACKET                                   0xb3
#define NX_PPP_ERROR                                        0xb4
#define NX_PPP_NOT_ESTABLISHED                              0xb5
#define NX_PPP_INVALID_PARAMETER                            0xb6
#define NX_PPP_ADDRESS_NOT_ESTABLISHED                      0xb7
#define NX_PPP_ALREADY_STOPPED                              0xb8
#define NX_PPP_ALREADY_STARTED                              0xb9

/* Define PPP state machine states.  */

#define NX_PPP_STOPPED                                      0
#define NX_PPP_STARTED                                      1
#define NX_PPP_ESTABLISHED                                  2


/* Define LCP state machine states.  */

#define NX_PPP_LCP_INITIAL_STATE                            0
#define NX_PPP_LCP_START_STATE                              1
#define NX_PPP_LCP_CONFIGURE_REQUEST_SENT_STATE             2
#define NX_PPP_LCP_CONFIGURE_REQUEST_ACKED_STATE            3
#define NX_PPP_LCP_PEER_CONFIGURE_REQUEST_ACKED_STATE       4
#define NX_PPP_LCP_STOPPING_STATE                           5
#define NX_PPP_LCP_STOPPED_STATE                            6
#define NX_PPP_LCP_FAILED_STATE                             7
#define NX_PPP_LCP_COMPLETED_STATE                          8


/* Define PAP state machine states.  */

#define NX_PPP_PAP_INITIAL_STATE                            0
#define NX_PPP_PAP_START_STATE                              1
#define NX_PPP_PAP_AUTHENTICATE_REQUEST_SENT_STATE          2
#define NX_PPP_PAP_AUTHENTICATE_REQUEST_WAIT_STATE          3
#define NX_PPP_PAP_FAILED_STATE                             4
#define NX_PPP_PAP_COMPLETED_STATE                          5


/* Define CHAP state machine states.  */

#define NX_PPP_CHAP_INITIAL_STATE                           0
#define NX_PPP_CHAP_START_STATE                             1
#define NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_STATE            2
#define NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_BOTH_STATE       3
#define NX_PPP_CHAP_CHALLENGE_REQUEST_WAIT_STATE            4
#define NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_RESPONDED_STATE  5
#define NX_PPP_CHAP_CHALLENGE_RESPONSE_WAIT_STATE           6
#define NX_PPP_CHAP_COMPLETED_NEW_STATE                     8
#define NX_PPP_CHAP_COMPLETED_NEW_SENT_STATE                9
#define NX_PPP_CHAP_CHALLENGE_FAILED_STATE                  10
#define NX_PPP_CHAP_COMPLETED_STATE                         11


/* Define IPCP state machine states.  */

#define NX_PPP_IPCP_INITIAL_STATE                           0
#define NX_PPP_IPCP_START_STATE                             1
#define NX_PPP_IPCP_CONFIGURE_REQUEST_SENT_STATE            2
#define NX_PPP_IPCP_CONFIGURE_REQUEST_ACKED_STATE           3
#define NX_PPP_IPCP_PEER_CONFIGURE_REQUEST_ACKED_STATE      4
#define NX_PPP_IPCP_STOPPING_STATE                          5
#define NX_PPP_IPCP_STOPPED_STATE                           6
#define NX_PPP_IPCP_FAILED_STATE                            7
#define NX_PPP_IPCP_COMPLETED_STATE                         8


/* Define event flags for PPP thread control.  */

#define NX_PPP_EVENT_START                                  0x01
#define NX_PPP_EVENT_STOP                                   0x02
#define NX_PPP_EVENT_PACKET_RECEIVE                         0x04
#define NX_PPP_EVENT_RAW_STRING_SEND                        0x08
#define NX_PPP_EVENT_IP_PACKET_SEND                         0x10
#define NX_PPP_EVENT_CHAP_CHALLENGE                         0x20
#define NX_PPP_EVENT_TIMEOUT                                0x40
#define NX_PPP_EVENT_PPPOE_PACKET_RECEIVE                   0x80


/* Define PPP status return values.  */

#define NX_PPP_STATUS_LCP_IN_PROGRESS                       0
#define NX_PPP_STATUS_LCP_FAILED                            1
#define NX_PPP_STATUS_PAP_IN_PROGRESS                       2
#define NX_PPP_STATUS_PAP_FAILED                            3
#define NX_PPP_STATUS_CHAP_IN_PROGRESS                      4
#define NX_PPP_STATUS_CHAP_FAILED                           5
#define NX_PPP_STATUS_IPCP_IN_PROGRESS                      6
#define NX_PPP_STATUS_IPCP_FAILED                           7
#define NX_PPP_STATUS_ESTABLISHED                           8

/* Define NAKed list options */
#define NX_PPP_IP_COMPRESSION_OPTION                        0x02
#define NX_PPP_IP_ADDRESS_OPTION                            0x03
#define NX_PPP_DNS_SERVER_OPTION                            0x81
#define NX_PPP_DNS_SECONDARY_SERVER_OPTION                  0x83



/* Define optional debug log.  This is used to capture the PPP traffic for debug
   purposes.  */

typedef struct NX_PPP_DEBUG_ENTRY_STRUCT
{

    ULONG           nx_ppp_debug_entry_time_stamp;
    UCHAR           nx_ppp_debug_ppp_state;
    UCHAR           nx_ppp_debug_lcp_state;
    UCHAR           nx_ppp_debug_pap_state;
    UCHAR           nx_ppp_debug_chap_state;
    UCHAR           nx_ppp_debug_ipcp_state;
    UCHAR           nx_ppp_debug_authenticated;
    UCHAR           nx_ppp_debug_frame_type;
    ULONG           nx_ppp_debug_packet_length;
    UCHAR           nx_ppp_debug_frame[NX_PPP_DEBUG_FRAME_SIZE];
} NX_PPP_DEBUG_ENTRY;



/* Define the main PPP data structure.  */

typedef struct NX_PPP_STRUCT 
{

    ULONG           nx_ppp_id;
    CHAR            *nx_ppp_name;
    NX_IP           *nx_ppp_ip_ptr;
    NX_INTERFACE    *nx_ppp_interface_ptr;
    UINT            nx_ppp_interface_index;
    NX_PACKET_POOL  *nx_ppp_packet_pool_ptr;
    UCHAR           nx_ppp_transmit_id;
    UCHAR           nx_ppp_receive_id;
    UCHAR           nx_ppp_lcp_echo_reply_id;
    UCHAR           nx_ppp_reserved;
    ULONG           nx_ppp_mru;
    UINT            nx_ppp_server;
    UINT            nx_ppp_state;
    UINT            nx_ppp_lcp_state;
    UINT            nx_ppp_pap_state;
    UINT            nx_ppp_chap_state;
    UINT            nx_ppp_ipcp_state;
    UINT            nx_ppp_authenticated;    
    UINT            nx_ppp_generate_authentication_protocol; 
    UINT            nx_ppp_verify_authentication_protocol;
    USHORT          nx_ppp_pap_enabled;
    USHORT          nx_ppp_chap_enabled;
    CHAR            nx_ppp_chap_challenger_name[NX_PPP_NAME_SIZE + 1];
    CHAR            nx_ppp_chap_random_value[NX_PPP_VALUE_SIZE + 1];
    UCHAR           nx_ppp_ipcp_local_ip[4];
    UCHAR           nx_ppp_ipcp_peer_ip[4];
    ULONG           nx_ppp_primary_dns_address;
    ULONG           nx_ppp_secondary_dns_address; 
    UINT            nx_ppp_dns_address_retries;
    UINT            nx_ppp_secondary_dns_address_retries;
    ULONG           nx_ppp_timeout;
    ULONG           nx_ppp_receive_timeouts;
    ULONG           nx_ppp_protocol_retry_counter;
#ifndef NX_PPP_DISABLE_INFO
    ULONG           nx_ppp_packet_allocate_timeouts;
    ULONG           nx_ppp_frame_timeouts;
    ULONG           nx_ppp_internal_errors;
    ULONG           nx_ppp_frame_crc_errors;
    ULONG           nx_ppp_packet_overflow;
    ULONG           nx_ppp_bytes_received;
    ULONG           nx_ppp_bytes_sent;
    ULONG           nx_ppp_bytes_dropped;
    ULONG           nx_ppp_bytes_invalid;
    ULONG           nx_ppp_total_frames_received;
    ULONG           nx_ppp_lcp_frames_received;
    ULONG           nx_ppp_lcp_frames_sent;
    ULONG           nx_ppp_lcp_configure_requests_sent;
    ULONG           nx_ppp_lcp_configure_requests_received;
    ULONG           nx_ppp_lcp_configure_acks_sent;
    ULONG           nx_ppp_lcp_configure_acks_received;
    ULONG           nx_ppp_lcp_configure_naks_sent;
    ULONG           nx_ppp_lcp_configure_naks_received;
    ULONG           nx_ppp_lcp_configure_rejects_sent;
    ULONG           nx_ppp_lcp_configure_rejects_received;
    ULONG           nx_ppp_lcp_terminate_requests_sent;
    ULONG           nx_ppp_lcp_terminate_requests_received;
    ULONG           nx_ppp_lcp_terminate_acks_sent;
    ULONG           nx_ppp_lcp_terminate_acks_received;
    ULONG           nx_ppp_lcp_code_rejects_sent;
    ULONG           nx_ppp_lcp_code_rejects_received;
    ULONG           nx_ppp_lcp_protocol_rejects_sent;
    ULONG           nx_ppp_lcp_protocol_rejects_received;
    ULONG           nx_ppp_lcp_echo_requests_sent;
    ULONG           nx_ppp_lcp_echo_replies_received;
    ULONG           nx_ppp_lcp_echo_requests_received;
    ULONG           nx_ppp_lcp_echo_requests_dropped;
    ULONG           nx_ppp_lcp_echo_replies_sent;
    ULONG           nx_ppp_lcp_discard_requests_sent;
    ULONG           nx_ppp_lcp_discard_requests_received;
    ULONG           nx_ppp_lcp_unknown_requests_received;
    ULONG           nx_ppp_lcp_state_machine_unhandled_requests;
    ULONG           nx_ppp_lcp_state_machine_timeouts;
#ifndef NX_PPP_DISABLE_PAP
    ULONG           nx_ppp_pap_frames_received;
    ULONG           nx_ppp_pap_frames_sent;
    ULONG           nx_ppp_pap_authenticate_requests_sent;
    ULONG           nx_ppp_pap_authenticate_requests_received;
    ULONG           nx_ppp_pap_authenticate_acks_sent;
    ULONG           nx_ppp_pap_authenticate_acks_received;
    ULONG           nx_ppp_pap_authenticate_naks_sent;
    ULONG           nx_ppp_pap_authenticate_naks_received;
    ULONG           nx_ppp_pap_unknown_requests_received;
    ULONG           nx_ppp_pap_state_machine_unhandled_requests;
    ULONG           nx_ppp_pap_state_machine_timeouts;
#endif
#ifndef NX_PPP_DISABLE_CHAP
    ULONG           nx_ppp_chap_frames_received;
    ULONG           nx_ppp_chap_frames_sent;
    ULONG           nx_ppp_chap_challenge_requests_sent;
    ULONG           nx_ppp_chap_challenge_requests_received;
    ULONG           nx_ppp_chap_challenge_responses_sent;
    ULONG           nx_ppp_chap_challenge_responses_received;
    ULONG           nx_ppp_chap_challenge_successes_sent;
    ULONG           nx_ppp_chap_challenge_successes_received;
    ULONG           nx_ppp_chap_challenge_failures_sent;
    ULONG           nx_ppp_chap_challenge_failures_received;
    ULONG           nx_ppp_chap_unknown_requests_received;
    ULONG           nx_ppp_chap_state_machine_unhandled_requests;
    ULONG           nx_ppp_chap_state_machine_timeouts;
#endif
    ULONG           nx_ppp_ipcp_frames_received;
    ULONG           nx_ppp_ipcp_frames_sent;
    ULONG           nx_ppp_ipcp_configure_requests_sent;
    ULONG           nx_ppp_ipcp_configure_requests_received;
    ULONG           nx_ppp_ipcp_configure_acks_sent;
    ULONG           nx_ppp_ipcp_configure_acks_received;
    ULONG           nx_ppp_ipcp_configure_naks_sent;
    ULONG           nx_ppp_ipcp_configure_naks_received;
    ULONG           nx_ppp_ipcp_configure_rejects_sent;
    ULONG           nx_ppp_ipcp_configure_rejects_received;
    ULONG           nx_ppp_ipcp_terminate_requests_sent;
    ULONG           nx_ppp_ipcp_terminate_requests_received;
    ULONG           nx_ppp_ipcp_terminate_acks_sent;
    ULONG           nx_ppp_ipcp_terminate_acks_received;
    ULONG           nx_ppp_ipcp_unknown_requests_received;
    ULONG           nx_ppp_ipcp_state_machine_unhandled_requests;
    ULONG           nx_ppp_ipcp_state_machine_timeouts;
    ULONG           nx_ppp_ip_frames_received;
    ULONG           nx_ppp_ip_frames_sent;
    ULONG           nx_ppp_receive_frames_dropped;
    ULONG           nx_ppp_transmit_frames_dropped;
    ULONG           nx_ppp_invalid_frame_id;
#endif

    void            (*nx_ppp_byte_send)(UCHAR byte);

#ifdef NX_PPP_PPPOE_ENABLE
    void            (*nx_ppp_packet_send)(NX_PACKET *packet_ptr);
#endif /* NX_PPP_PPPOE_ENABLE  */

    void            (*nx_ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr);
    void            (*nx_ppp_nak_authentication_notify)(void);

    void            (*nx_ppp_link_up_callback)(struct NX_PPP_STRUCT *ppp_ptr);
    void            (*nx_ppp_link_down_callback)(struct NX_PPP_STRUCT *ppp_ptr);

    UINT            (*nx_ppp_pap_verify_login)(CHAR *name, CHAR *password);
    UINT            (*nx_ppp_pap_generate_login)(CHAR *name, CHAR *password);

    UINT            (*nx_ppp_chap_get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name);
    UINT            (*nx_ppp_chap_get_responder_values)(CHAR *sys, CHAR *name, CHAR *secret);
    UINT            (*nx_ppp_chap_get_verification_values)(CHAR *sys, CHAR *name, CHAR *secret);
    TX_EVENT_FLAGS_GROUP  
                    nx_ppp_event;
    TX_TIMER        nx_ppp_timer;

    UCHAR           nx_ppp_serial_buffer[NX_PPP_SERIAL_BUFFER_SIZE];
    USHORT          nx_ppp_serial_buffer_write_index;
    USHORT          nx_ppp_serial_buffer_read_index;
    USHORT          nx_ppp_serial_buffer_byte_count;

    NX_PACKET       *nx_ppp_receive_partial_packet;
    ULONG           nx_ppp_receive_buffer_size;
    UCHAR           *nx_ppp_receive_buffer_ptr;

    NX_PACKET       *nx_ppp_head_packet;

#ifdef NX_PPP_PPPOE_ENABLE

    /* Define the deferred packet processing queue for driver packet.  */
    NX_PACKET       *nx_ppp_deferred_received_packet_head,
                    *nx_ppp_deferred_received_packet_tail;
#endif /* NX_PPP_PPPOE_ENABLE  */

    NX_PACKET       *nx_ppp_raw_packet_queue_head;
    NX_PACKET       *nx_ppp_raw_packet_queue_tail;
    ULONG           nx_ppp_raw_packet_queue_count;

    NX_PACKET       *nx_ppp_ip_packet_queue_head;
    NX_PACKET       *nx_ppp_ip_packet_queue_tail;
    ULONG           nx_ppp_ip_packet_queue_count;

    TX_THREAD       nx_ppp_thread;
    struct NX_PPP_STRUCT 
                    *nx_ppp_created_next,
                    *nx_ppp_created_previous;

    UCHAR           nx_ppp_naked_list[NX_PPP_OPTION_MESSAGE_LENGTH]; 
    UCHAR           nx_ppp_peer_naked_list[NX_PPP_OPTION_MESSAGE_LENGTH]; 
    UCHAR           nx_ppp_rejected_list[NX_PPP_OPTION_MESSAGE_LENGTH];

#ifdef NX_PPP_DEBUG_LOG_ENABLE
    NX_PPP_DEBUG_ENTRY
                    nx_ppp_debug_log[NX_PPP_DEBUG_LOG_SIZE];
    UINT            nx_ppp_debug_log_oldest_index;
#endif

} NX_PPP;


#ifndef NX_PPP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_ppp_byte_receive                 _nx_ppp_byte_receive
#define nx_ppp_chap_challenge               _nx_ppp_chap_challenge
#define nx_ppp_chap_enable                  _nx_ppp_chap_enable
#define nx_ppp_create                       _nx_ppp_create
#define nx_ppp_delete                       _nx_ppp_delete
#define nx_ppp_dns_address_get              _nx_ppp_dns_address_get
#define nx_ppp_dns_address_set              _nx_ppp_dns_address_set
#define nx_ppp_secondary_dns_address_get    _nx_ppp_secondary_dns_address_get
#define nx_ppp_secondary_dns_address_set    _nx_ppp_secondary_dns_address_set
#define nx_ppp_driver                       _nx_ppp_driver
#define nx_ppp_interface_index_get          _nx_ppp_interface_index_get 
#define nx_ppp_ip_address_assign            _nx_ppp_ip_address_assign
#define nx_ppp_link_down_notify             _nx_ppp_link_down_notify
#define nx_ppp_link_up_notify               _nx_ppp_link_up_notify
#define nx_ppp_nak_authentication_notify    _nx_ppp_nak_authentication_notify
#define nx_ppp_pap_enable                   _nx_ppp_pap_enable
#define nx_ppp_raw_string_send              _nx_ppp_raw_string_send
#define nx_ppp_start                        _nx_ppp_start
#define nx_ppp_stop                         _nx_ppp_stop
#define nx_ppp_restart                      _nx_ppp_restart
#define nx_ppp_status_get                   _nx_ppp_status_get
#define nx_ppp_ping_request                 _nx_ppp_ping_request
#ifdef NX_PPP_PPPOE_ENABLE
#define nx_ppp_packet_receive               _nx_ppp_packet_receive
#define nx_ppp_packet_send_set              _nx_ppp_packet_send_set
#endif /* NX_PPP_PPPOE_ENABLE  */

#else

/* Services with error checking.  */

#define nx_ppp_byte_receive                 _nxe_ppp_byte_receive
#define nx_ppp_chap_challenge               _nxe_ppp_chap_challenge
#define nx_ppp_chap_enable                  _nxe_ppp_chap_enable
#define nx_ppp_create                       _nxe_ppp_create
#define nx_ppp_delete                       _nxe_ppp_delete
#define nx_ppp_dns_address_get              _nxe_ppp_dns_address_get
#define nx_ppp_dns_address_set              _nxe_ppp_dns_address_set
#define nx_ppp_secondary_dns_address_get    _nxe_ppp_secondary_dns_address_get
#define nx_ppp_secondary_dns_address_set    _nxe_ppp_secondary_dns_address_set
#define nx_ppp_driver                       _nx_ppp_driver
#define nx_ppp_interface_index_get          _nxe_ppp_interface_index_get 
#define nx_ppp_ip_address_assign            _nxe_ppp_ip_address_assign
#define nx_ppp_link_down_notify             _nxe_ppp_link_down_notify
#define nx_ppp_link_up_notify               _nxe_ppp_link_up_notify
#define nx_ppp_nak_authentication_notify    _nxe_ppp_nak_authentication_notify
#define nx_ppp_pap_enable                   _nxe_ppp_pap_enable
#define nx_ppp_raw_string_send              _nxe_ppp_raw_string_send
#define nx_ppp_start                        _nxe_ppp_start
#define nx_ppp_stop                         _nxe_ppp_stop
#define nx_ppp_restart                      _nxe_ppp_restart
#define nx_ppp_status_get                   _nxe_ppp_status_get
#define nx_ppp_ping_request                 _nxe_ppp_ping_request
#ifdef NX_PPP_PPPOE_ENABLE
#define nx_ppp_packet_receive               _nxe_ppp_packet_receive
#define nx_ppp_packet_send_set              _nxe_ppp_packet_send_set
#endif /* NX_PPP_PPPOE_ENABLE  */

#endif /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT    nx_ppp_byte_receive(NX_PPP *ppp_ptr, UCHAR byte);
UINT    nx_ppp_chap_challenge(NX_PPP *ppp_ptr);
UINT    nx_ppp_chap_enable(NX_PPP *ppp_ptr, 
               UINT (*get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name),
               UINT (*get_responder_values)(CHAR *system, CHAR *name, CHAR *secret),
               UINT (*get_verification_values)(CHAR *system, CHAR *name, CHAR *secret));
UINT    nx_ppp_create(NX_PPP *ppp_ptr, CHAR *name, NX_IP *ip_ptr, 
               VOID *stack_memory_ptr, ULONG stack_size, UINT thread_priority, 
               NX_PACKET_POOL *pool_ptr,
               void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr),
               void (*ppp_byte_send)(UCHAR byte));
UINT    nx_ppp_delete(NX_PPP *ppp_ptr);
UINT    nx_ppp_dns_address_get(NX_PPP *ppp_ptr, ULONG *dns_address_ptr);
UINT    nx_ppp_dns_address_set(NX_PPP *ppp_ptr, ULONG dns_address);
UINT    nx_ppp_secondary_dns_address_get(NX_PPP *ppp_ptr, ULONG *secondary_dns_address_ptr); 
UINT    nx_ppp_secondary_dns_address_set(NX_PPP *ppp_ptr, ULONG secondary_dns_address);
void    nx_ppp_driver(NX_IP_DRIVER *driver_req_ptr);
UINT    nx_ppp_interface_index_get(NX_PPP *ppp_ptr, UINT *index_ptr);
UINT    nx_ppp_ip_address_assign(NX_PPP *ppp_ptr, ULONG local_ip_address, ULONG peer_ip_address);
UINT    nx_ppp_link_down_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_down_callback)(NX_PPP *ppp_ptr));
UINT    nx_ppp_link_up_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_up_callback)(NX_PPP *ppp_ptr));
UINT    nx_ppp_nak_authentication_notify(NX_PPP *ppp_ptr, void (*nak_authentication_notify)(void));
UINT    nx_ppp_pap_enable(NX_PPP *ppp_ptr, UINT (*generate_login)(CHAR *name, CHAR *password), UINT (*verify_login)(CHAR *name, CHAR *password));
UINT    nx_ppp_raw_string_send(NX_PPP *ppp_ptr, CHAR *string_ptr);
UINT    nx_ppp_start(NX_PPP *ppp_ptr);
UINT    nx_ppp_stop(NX_PPP *ppp_ptr);
UINT    nx_ppp_restart(NX_PPP *ppp_ptr);
UINT    nx_ppp_status_get(NX_PPP *ppp_ptr, UINT *status_ptr);
UINT    nx_ppp_ping_request(NX_PPP *ppp_ptr, CHAR *data_ptr, ULONG data_size,  ULONG wait_option);
#ifdef NX_PPP_PPPOE_ENABLE
UINT    nx_ppp_packet_receive(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    nx_ppp_packet_send_set(NX_PPP *ppp_ptr, void (*nx_ppp_packet_send)(NX_PACKET *packet_ptr));
#endif /* NX_PPP_PPPOE_ENABLE  */

#else /* NX_PPP_SOURCE_CODE  */

UINT    _nxe_ppp_byte_receive(NX_PPP *ppp_ptr, UCHAR byte);
UINT    _nx_ppp_byte_receive(NX_PPP *ppp_ptr, UCHAR byte);
UINT    _nxe_ppp_chap_challenge(NX_PPP *ppp_ptr);
UINT    _nx_ppp_chap_challenge(NX_PPP *ppp_ptr);
UINT    _nxe_ppp_chap_enable(NX_PPP *ppp_ptr, 
               UINT (*get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name),
               UINT (*get_responder_values)(CHAR *system, CHAR *name, CHAR *secret),
               UINT (*get_verification_values)(CHAR *system, CHAR *name, CHAR *secret));
UINT    _nx_ppp_chap_enable(NX_PPP *ppp_ptr, 
               UINT (*get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name),
               UINT (*get_responder_values)(CHAR *system, CHAR *name, CHAR *secret),
               UINT (*get_verification_values)(CHAR *system, CHAR *name, CHAR *secret));
UINT    _nxe_ppp_create(NX_PPP *ppp_ptr, CHAR *name, NX_IP *ip_ptr, 
               VOID *stack_memory_ptr, ULONG stack_size, UINT thread_priority, 
               NX_PACKET_POOL *pool_ptr,
               void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr),
               void (*ppp_byte_send)(UCHAR byte));
UINT    _nx_ppp_create(NX_PPP *ppp_ptr, CHAR *name, NX_IP *ip_ptr, 
               VOID *stack_memory_ptr, ULONG stack_size, UINT thread_priority, 
               NX_PACKET_POOL *pool_ptr,
               void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr),
               void (*ppp_byte_send)(UCHAR byte));
UINT    _nxe_ppp_delete(NX_PPP *ppp_ptr);
UINT    _nx_ppp_delete(NX_PPP *ppp_ptr);
UINT    _nxe_ppp_dns_address_get(NX_PPP *ppp_ptr, ULONG *dns_address_ptr);
UINT    _nx_ppp_dns_address_get(NX_PPP *ppp_ptr, ULONG *dns_address_ptr);
UINT    _nxe_ppp_dns_address_set(NX_PPP *ppp_ptr, ULONG dns_address);
UINT    _nx_ppp_dns_address_set(NX_PPP *ppp_ptr, ULONG dns_address);
UINT    _nxe_ppp_secondary_dns_address_get(NX_PPP *ppp_ptr, ULONG *secondary_dns_address_ptr);
UINT    _nx_ppp_secondary_dns_address_get(NX_PPP *ppp_ptr, ULONG *secondary_dns_address_ptr);
UINT    _nxe_ppp_secondary_dns_address_set(NX_PPP *ppp_ptr, ULONG secondary_dns_address);
UINT    _nx_ppp_secondary_dns_address_set(NX_PPP *ppp_ptr, ULONG secondary_dns_address);
UINT    _nxe_ppp_interface_index_get(NX_PPP *ppp_ptr, UINT *index_ptr);
UINT    _nx_ppp_interface_index_get(NX_PPP *ppp_ptr, UINT *index_ptr);
UINT    _nxe_ppp_ip_address_assign(NX_PPP *ppp_ptr, ULONG local_ip_address, ULONG peer_ip_address);
UINT    _nx_ppp_ip_address_assign(NX_PPP *ppp_ptr, ULONG local_ip_address, ULONG peer_ip_address);
UINT    _nxe_ppp_link_down_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_down_callback)(NX_PPP *ppp_ptr));
UINT    _nx_ppp_link_down_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_down_callback)(NX_PPP *ppp_ptr));
UINT    _nxe_ppp_link_up_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_up_callback)(NX_PPP *ppp_ptr)); 
UINT    _nx_ppp_link_up_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_up_callback)(NX_PPP *ppp_ptr));
UINT    _nxe_ppp_nak_authentication_notify(NX_PPP *ppp_ptr, void (*nak_authentication_notify)(void));
UINT    _nx_ppp_nak_authentication_notify(NX_PPP *ppp_ptr, void (*nak_authentication_notify)(void));
UINT    _nxe_ppp_pap_enable(NX_PPP *ppp_ptr, UINT (*generate_login)(CHAR *name, CHAR *password), UINT (*verify_login)(CHAR *name, CHAR *password));
UINT    _nx_ppp_pap_enable(NX_PPP *ppp_ptr, UINT (*generate_login)(CHAR *name, CHAR *password), UINT (*verify_login)(CHAR *name, CHAR *password));
UINT    _nxe_ppp_raw_string_send(NX_PPP *ppp_ptr, CHAR *string_ptr);
UINT    _nx_ppp_raw_string_send(NX_PPP *ppp_ptr, CHAR *string_ptr);
UINT    _nxe_ppp_start(NX_PPP *ppp_ptr);
UINT    _nx_ppp_start(NX_PPP *ppp_ptr);
UINT    _nxe_ppp_stop(NX_PPP *ppp_ptr);
UINT    _nx_ppp_stop(NX_PPP *ppp_ptr);
UINT    _nxe_ppp_restart(NX_PPP *ppp_ptr);
UINT    _nx_ppp_restart(NX_PPP *ppp_ptr);
UINT    _nxe_ppp_status_get(NX_PPP *ppp_ptr, UINT *status_ptr);
UINT    _nx_ppp_status_get(NX_PPP *ppp_ptr, UINT *status_ptr);
UINT    _nxe_ppp_ping_request(NX_PPP *ppp_ptr, CHAR *data_ptr, ULONG data_size,  ULONG wait_option);
UINT    _nx_ppp_ping_request(NX_PPP *ppp_ptr, CHAR *data_ptr, ULONG data_size,  ULONG wait_option);
#ifdef NX_PPP_PPPOE_ENABLE
UINT    _nxe_ppp_packet_receive(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    _nx_ppp_packet_receive(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    _nxe_ppp_packet_send_set(NX_PPP *ppp_ptr, VOID (*nx_ppp_packet_send)(NX_PACKET *packet_ptr));
UINT    _nx_ppp_packet_send_set(NX_PPP *ppp_ptr, VOID (*nx_ppp_packet_send)(NX_PACKET *packet_ptr));
#endif /* NX_PPP_PPPOE_ENABLE  */

/* Define internal PPP services. */
void    _nx_ppp_thread_entry(ULONG ppp_addr);
void    _nx_ppp_driver(NX_IP_DRIVER *driver_req_ptr);
void    _nx_ppp_receive_packet_get(NX_PPP *ppp_ptr, NX_PACKET **return_packet_ptr);
void    _nx_ppp_receive_packet_process(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_timeout(NX_PPP *ppp_ptr);
void    _nx_ppp_timer_entry(ULONG id);
void    _nx_ppp_netx_packet_transfer(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_process_deferred_raw_string_send(NX_PPP *ppp_ptr);
void    _nx_ppp_process_deferred_ip_packet_send(NX_PPP *ppp_ptr);
void    _nx_ppp_lcp_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_lcp_code_reject(NX_PPP *ppp_ptr, UCHAR *lcp_ptr);
void    _nx_ppp_lcp_configure_reply_send(NX_PPP *ppp_ptr, UINT configure_status, UCHAR *lcp_ptr, UCHAR *naked_list, UCHAR *rejected_list);
void    _nx_ppp_lcp_configure_request_send(NX_PPP *ppp_ptr);
UINT    _nx_ppp_lcp_configuration_retrieve(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr, UCHAR *naked_list, UCHAR *rejected_list, UINT *configure_status);
void    _nx_ppp_lcp_nak_configure_list(NX_PPP *ppp_ptr, UCHAR *naked_list);
void    _nx_ppp_lcp_terminate_ack_send(NX_PPP *ppp_ptr);
void    _nx_ppp_lcp_terminate_request_send(NX_PPP *ppp_ptr);
void    _nx_ppp_pap_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_pap_authentication_request(NX_PPP *ppp_ptr);
UINT    _nx_ppp_pap_login_valid(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_pap_authentication_ack(NX_PPP *ppp_ptr);
void    _nx_ppp_pap_authentication_nak(NX_PPP *ppp_ptr);
void    _nx_ppp_chap_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_chap_challenge_send(NX_PPP *ppp_ptr);
void    _nx_ppp_chap_challenge_respond(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    _nx_ppp_chap_challenge_validate(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_ipcp_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    _nx_ppp_ipcp_configure_check(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr, UCHAR *naked_list, UCHAR *rejected_list, UCHAR *good_data);
void    _nx_ppp_ipcp_configure_request_send(NX_PPP *ppp_ptr, UCHAR *negotiate_list);
void    _nx_ppp_ipcp_response_extract(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_ipcp_response_send(NX_PPP *ppp_ptr, UCHAR type, UCHAR *data, UCHAR length, NX_PACKET *cfg_packet_ptr);
void    _nx_ppp_ipcp_terminate_send(NX_PPP *ppp_ptr);
void    _nx_ppp_ipcp_terminate_ack_send(NX_PPP *ppp_ptr);
void    _nx_ppp_packet_transmit(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
UINT    _nx_ppp_check_crc(NX_PACKET *packet_ptr);
UINT    _nx_ppp_crc_append(NX_PACKET *packet_ptr, UCHAR crc[2]);
void    _nx_ppp_debug_log_capture(NX_PPP *ppp_ptr, UCHAR packet_type, NX_PACKET *packet_ptr);
void    _nx_ppp_debug_log_capture_protocol(NX_PPP *ppp_ptr);
void    _nx_ppp_hash_generator(unsigned char *hvalue,  unsigned char id, unsigned char *secret,  unsigned char *rand_value, UINT length);
void    _nx_ppp_lcp_ping_reply(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);
void    _nx_ppp_lcp_ping_process_echo_reply(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr);

#endif /* NX_PPP_SOURCE_CODE */

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif /* NX_PPP_H */ 
