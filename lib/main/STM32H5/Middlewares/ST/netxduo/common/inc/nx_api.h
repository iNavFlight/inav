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
/**   Application Interface (API)                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_api.h                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance NetX TCP/IP implementation for the ThreadX         */
/*    real-time kernel.  All service prototypes and data structure        */
/*    definitions are defined in this file. Please note that basic data   */
/*    type definitions and other architecture-specific information is     */
/*    contained in the file nx_port.h.                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            ThreadX version check,      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.2  */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), added    */
/*                                            PTP timestamp capability,   */
/*                                            added function to convert   */
/*                                            string to unsigned integer, */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.3  */
/*  02-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.4  */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.5  */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            added functions for base64, */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.7  */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            added new ip filter,        */
/*                                            added function to convert   */
/*                                            unsigned integer to string, */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            added support for getting   */
/*                                            interface type,             */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            fixed compiler errors when  */
/*                                            TX_SAFETY_CRITICAL is       */
/*                                            enabled,                    */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Wenhui Xie               Modified comment(s), and      */
/*                                            supported HTTP Proxy,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_API_H
#define NX_API_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


/* Disable warning of parameter not used. */
#ifndef NX_PARAMETER_NOT_USED
#define NX_PARAMETER_NOT_USED(p) ((void)(p))
#endif /* NX_PARAMETER_NOT_USED */


/* Bypass ThreadX API error checking for internal NetX calls.  */
#include "tx_port.h"

#ifdef NX_SOURCE_CODE
#ifndef TX_SAFETY_CRITICAL
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#endif
#endif



/* Include the ThreadX and port-specific data type file.  */

#include "tx_api.h"
#include "nx_port.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_crypto_const.h"
#endif /* NX_IPSEC_ENABLE */
/* Include the ThreadX trace information.  */

#include "tx_trace.h"

#ifdef NX_ENABLE_TCPIP_OFFLOAD
#ifndef NX_ENABLE_INTERFACE_CAPABILITY
#error "NX_ENABLE_INTERFACE_CAPABILITY must be defined to enable TCP/IP offload"
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

/* Define symbols for compatibility before and after ThreadX 5.8. */
#if (((THREADX_MAJOR_VERSION << 8) | THREADX_MINOR_VERSION) >= 0x0508)
#define NX_CLEANUP_PARAMETER , ULONG suspension_sequence
#define NX_CLEANUP_ARGUMENT  , 0
#define NX_CLEANUP_EXTENSION NX_PARAMETER_NOT_USED(suspension_sequence);
#else
#define NX_CLEANUP_PARAMETER
#define NX_CLEANUP_ARGUMENT
#define NX_CLEANUP_EXTENSION
#endif /* (((THREADX_MAJOR_VERSION << 8) | THREADX_MINOR_VERSION) >= 0x0508) */

/* Define the get system state macro. By default, it simply maps to the variable _tx_thread_system_state.  */
#ifndef TX_THREAD_GET_SYSTEM_STATE
#define TX_THREAD_GET_SYSTEM_STATE()                        _tx_thread_system_state
#endif

/* Define basic alignment type used in block and byte pool operations. This data type must
   be at least 32-bits in size and also be large enough to hold a pointer type.  */

#ifndef ALIGN_TYPE_DEFINED
#define ALIGN_TYPE      ULONG
#endif


/* Define the extension to hold the control block.  */
#ifndef NX_THREAD_EXTENSION_PTR_SET
#define NX_THREAD_EXTENSION_PTR_SET(a, b)
#endif /* NX_THREAD_EXTENSION_PTR_SET  */

#ifndef NX_THREAD_EXTENSION_PTR_GET
#define NX_THREAD_EXTENSION_PTR_GET(a, b, c)                { \
                                                                (a) = (b *)(c); \
                                                            }
#endif /* NX_THREAD_EXTENSION_PTR_GET  */

#ifndef NX_TIMER_EXTENSION_PTR_SET
#define NX_TIMER_EXTENSION_PTR_SET(a, b)
#endif /* NX_TIMER_EXTENSION_PTR_SET  */

#ifndef NX_TIMER_EXTENSION_PTR_GET
#define NX_TIMER_EXTENSION_PTR_GET(a, b, c)                 { \
                                                                (a) = (b *)(c); \
                                                            }
#endif /* NX_TIMER_EXTENSION_PTR_GET  */


#ifdef TX_UCHAR_POINTER_ADD
#define NX_UCHAR_POINTER_ADD(a,b)                           TX_UCHAR_POINTER_ADD(a,b)
#else
#define NX_UCHAR_POINTER_ADD(a,b)                           (((UCHAR *) (a)) + ((UINT) (b)))
#endif


/* The symbol "NX_IP_PERIODIC_RATE" specifies the number of ThreadX timer ticks in one second.
   The value should be derived from TX_TIMER_TICKS_PER_SECOND, which is defined in ThreadX port.
   If TX_TIMER_TICKS_PER_SECOND is not defined, NX_IP_PERIODIC_RATE defaults to 100 ticks per second. */

#ifndef NX_IP_PERIODIC_RATE
#ifdef TX_TIMER_TICKS_PER_SECOND
#define NX_IP_PERIODIC_RATE                                 TX_TIMER_TICKS_PER_SECOND
#else
#define NX_IP_PERIODIC_RATE                                 100
#endif
#endif


/* This defines the ASSET and process on ASSET fail. */
#ifndef NX_DISABLE_ASSERT
#ifndef NX_ASSERT_FAIL
#define NX_ASSERT_FAIL                                      for (;;) {tx_thread_sleep(NX_WAIT_FOREVER); }
#endif /* NX_ASSERT_FAIL */
#define NX_ASSERT(s)                                        if (!(s)) {NX_ASSERT_FAIL}
#else
#define NX_ASSERT(s)
#endif /* NX_DISABLE_ASSERT */


#ifndef NX_RAND
#ifdef NX_HIGH_SECURITY
#error "The symbol NX_RAND must be defined to use a qualified random number generator."
#else
#define NX_RAND                                             rand
#endif
#endif

#ifndef NX_SRAND
#ifdef NX_HIGH_SECURITY
#error "The symbol NX_SRAND must be defined to use a qualified random number seed."
#else
#define NX_SRAND                                            srand
#endif
#endif


/* Define the max string length.  */
#ifndef NX_MAX_STRING_LENGTH
#define NX_MAX_STRING_LENGTH                                1024
#endif  /* NX_MAX_STRING_LENGTH */


/* Determine if tracing is enabled.  */

#ifdef TX_ENABLE_EVENT_TRACE

/* Define the object types in NetX, if not defined.  */

#ifndef NX_TRACE_OBJECT_TYPE_IP
#define NX_TRACE_OBJECT_TYPE_IP                             11          /* P1 = stack start address, P2 = stack size    */
#define NX_TRACE_OBJECT_TYPE_PACKET_POOL                    12          /* P1 = packet size, P2 = number of packets     */
#define NX_TRACE_OBJECT_TYPE_TCP_SOCKET                     13          /* P1 = IP address, P2 = window size            */
#define NX_TRACE_OBJECT_TYPE_UDP_SOCKET                     14          /* P1 = IP address, P2 = receive queue maximum  */
#endif


/* Define event filters that can be used to selectively disable certain events or groups of events.  */

#define NX_TRACE_ALL_EVENTS                                 0x00FF8000  /* All NetX events                           */
#define NX_TRACE_INTERNAL_EVENTS                            0x00008000  /* NetX internal events                      */
#define NX_TRACE_ARP_EVENTS                                 0x00010000  /* NetX ARP events                           */
#define NX_TRACE_ICMP_EVENTS                                0x00020000  /* NetX ICMP events                          */
#define NX_TRACE_IGMP_EVENTS                                0x00040000  /* NetX IGMP events                          */
#define NX_TRACE_IP_EVENTS                                  0x00080000  /* NetX IP events                            */
#define NX_TRACE_PACKET_EVENTS                              0x00100000  /* NetX Packet events                        */
#define NX_TRACE_RARP_EVENTS                                0x00200000  /* NetX RARP events                          */
#define NX_TRACE_TCP_EVENTS                                 0x00400000  /* NetX TCP events                           */
#define NX_TRACE_UDP_EVENTS                                 0x00800000  /* NetX UDP events                           */


/* Define the trace events in NetX, if not defined.  */

/* Define NetX Trace Events, along with a brief description of the additional information fields,
   where I1 -> Information Field 1, I2 -> Information Field 2, etc.  */

/* Define the NetX internal events first.  */

#ifndef NX_TRACE_INTERNAL_ARP_REQUEST_RECEIVE
#define NX_TRACE_INTERNAL_ARP_REQUEST_RECEIVE               300         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr                     */
#define NX_TRACE_INTERNAL_ARP_REQUEST_SEND                  301         /* I1 = ip ptr, I2 = destination IP address, I3 = packet ptr                */
#define NX_TRACE_INTERNAL_ARP_RESPONSE_RECEIVE              302         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr                     */
#define NX_TRACE_INTERNAL_ARP_RESPONSE_SEND                 303         /* I1 = ip ptr, I2 = destination IP address, I3 = packet ptr                */
#define NX_TRACE_INTERNAL_ICMP_RECEIVE                      304         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr, I4 = header word 0 */
#define NX_TRACE_INTERNAL_ICMP_SEND                         305         /* I1 = ip ptr, I2 = destination IP address, I3 = packet ptr, I4 = header 0 */
#define NX_TRACE_INTERNAL_IGMP_RECEIVE                      306         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr, I4 = header word 0 */

#define NX_TRACE_INTERNAL_IP_RECEIVE                        308         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr, I4 = packet length */
#define NX_TRACE_INTERNAL_IP_SEND                           309         /* I1 = ip ptr, I2 = destination IP address, I3 = packet ptr, I4 = length   */
#define NX_TRACE_INTERNAL_TCP_DATA_RECEIVE                  310         /* I1 = ip ptr, I2 = source IP address, I3 = packet ptr, I4 = sequence      */
#define NX_TRACE_INTERNAL_TCP_DATA_SEND                     311         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_FIN_RECEIVE                   312         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_FIN_SEND                      313         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_RESET_RECEIVE                 314         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_RESET_SEND                    315         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_SYN_RECEIVE                   316         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_TCP_SYN_SEND                      317         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = sequence             */
#define NX_TRACE_INTERNAL_UDP_RECEIVE                       318         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = header word 0        */
#define NX_TRACE_INTERNAL_UDP_SEND                          319         /* I1 = ip ptr, I2 = socket_ptr, I3 = packet ptr, I4 = header 0             */
#define NX_TRACE_INTERNAL_RARP_RECEIVE                      320         /* I1 = ip ptr, I2 = target IP address, I3 = packet ptr, I4 = header word 1 */
#define NX_TRACE_INTERNAL_RARP_SEND                         321         /* I1 = ip ptr, I2 = target IP address, I3 = packet ptr, I4 = header word 1 */
#define NX_TRACE_INTERNAL_TCP_RETRY                         322         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = number of retries    */
#define NX_TRACE_INTERNAL_TCP_STATE_CHANGE                  323         /* I1 = ip ptr, I2 = socket ptr, I3 = previous state, I4 = new state        */
#define NX_TRACE_INTERNAL_IO_DRIVER_PACKET_SEND             324         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */
#define NX_TRACE_INTERNAL_IO_DRIVER_INITIALIZE              325         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_LINK_ENABLE             326         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_LINK_DISABLE            327         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_PACKET_BROADCAST        328         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */
#define NX_TRACE_INTERNAL_IO_DRIVER_ARP_SEND                329         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */
#define NX_TRACE_INTERNAL_IO_DRIVER_ARP_RESPONSE_SEND       330         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */
#define NX_TRACE_INTERNAL_IO_DRIVER_RARP_SEND               331         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */
#define NX_TRACE_INTERNAL_IO_DRIVER_MULTICAST_JOIN          332         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_MULTICAST_LEAVE         333         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_STATUS              334         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_SPEED               335         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_DUPLEX_TYPE         336         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_ERROR_COUNT         337         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_RX_COUNT            338         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_TX_COUNT            339         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_GET_ALLOC_ERRORS        340         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_UNINITIALIZE            341         /* I1 = ip ptr                                                              */
#define NX_TRACE_INTERNAL_IO_DRIVER_DEFERRED_PROCESSING     342         /* I1 = ip ptr, I2 = packet ptr, I3 = packet size                           */

#define NX_TRACE_ARP_DYNAMIC_ENTRIES_INVALIDATE             350         /* I1 = ip ptr, I2 = entries invalidated                                    */
#define NX_TRACE_ARP_DYNAMIC_ENTRY_SET                      351         /* I1 = ip ptr, I2 = ip address, I3 = physical msw, I4 = physical lsw       */
#define NX_TRACE_ARP_ENABLE                                 352         /* I1 = ip ptr, I2 = arp cache memory, I3 = arp cache size                  */
#define NX_TRACE_ARP_GRATUITOUS_SEND                        353         /* I1 = ip ptr                                                              */
#define NX_TRACE_ARP_HARDWARE_ADDRESS_FIND                  354         /* I1 = ip ptr, I2 = ip_address, I3 = physical msw, I4 = physical lsw       */
#define NX_TRACE_ARP_INFO_GET                               355         /* I1 = ip ptr, I2 = arps sent, I3 = arp responses, I3 = arps received      */
#define NX_TRACE_ARP_IP_ADDRESS_FIND                        356         /* I1 = ip ptr, I2 = ip address, I3 = physical msw, I4 = physical lsw       */
#define NX_TRACE_ARP_STATIC_ENTRIES_DELETE                  357         /* I1 = ip ptr, I2 = entries deleted                                        */
#define NX_TRACE_ARP_STATIC_ENTRY_CREATE                    358         /* I1 = ip ptr, I2 = ip address, I3 = physical msw, I4 = physical_lsw       */
#define NX_TRACE_ARP_STATIC_ENTRY_DELETE                    359         /* I1 = ip ptr, I2 = ip address, I3 = physical_msw, I4 = physical_lsw       */
#define NX_TRACE_ICMP_ENABLE                                360         /* I1 = ip ptr                                                              */
#define NX_TRACE_ICMP_INFO_GET                              361         /* I1 = ip ptr, I2 = pings sent, I3 = ping responses, I4 = pings received   */
#define NX_TRACE_ICMP_PING                                  362         /* I1 = ip ptr, I2 = ip_address, I3 = data ptr, I4 = data size              */
#define NX_TRACE_IGMP_ENABLE                                363         /* I1 = ip ptr                                                              */
#define NX_TRACE_IGMP_INFO_GET                              364         /* I1 = ip ptr, I2 = reports sent, I3 = queries received, I4 = groups joined*/
#define NX_TRACE_IGMP_LOOPBACK_DISABLE                      365         /* I1 = ip ptr                                                              */
#define NX_TRACE_IGMP_LOOPBACK_ENABLE                       366         /* I1 = ip ptr                                                              */
#define NX_TRACE_IGMP_MULTICAST_JOIN                        367         /* I1 = ip ptr, I2 = group address                                          */
#define NX_TRACE_IGMP_MULTICAST_LEAVE                       368         /* I1 = ip ptr, I2 = group_address                                          */
#define NX_TRACE_IP_ADDRESS_CHANGE_NOTIFY                   369         /* I1 = ip ptr, I2 = ip address change notify, I3 = additional info         */
#define NX_TRACE_IP_ADDRESS_GET                             370         /* I1 = ip ptr, I2 = ip address, I3 = network_mask                          */
#define NX_TRACE_IP_ADDRESS_SET                             371         /* I1 = ip ptr, I2 = ip address, I3 = network_mask                          */
#define NX_TRACE_IP_CREATE                                  372         /* I1 = ip ptr, I2 = ip address, I3 = network mask, I4 = default_pool       */
#define NX_TRACE_IP_DELETE                                  373         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_DRIVER_DIRECT_COMMAND                   374         /* I1 = ip ptr, I2 = command, I3 = return value                             */
#define NX_TRACE_IP_FORWARDING_DISABLE                      375         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_FORWARDING_ENABLE                       376         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_FRAGMENT_DISABLE                        377         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_FRAGMENT_ENABLE                         378         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_GATEWAY_ADDRESS_SET                     379         /* I1 = ip ptr, I2 = gateway address                                        */
#define NX_TRACE_IP_INFO_GET                                380         /* I1 = ip ptr, I2 = bytes sent, I3 = bytes received, I4 = packets dropped  */
#define NX_TRACE_IP_RAW_PACKET_DISABLE                      381         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_RAW_PACKET_ENABLE                       382         /* I1 = ip ptr                                                              */
#define NX_TRACE_IP_RAW_PACKET_RECEIVE                      383         /* I1 = ip ptr, I2 = packet ptr, I3 = wait option                           */
#define NX_TRACE_IP_RAW_PACKET_SEND                         384         /* I1 = ip ptr, I2 = packet ptr, I3 = destination ip, I4 = type of service  */
#define NX_TRACE_IP_STATUS_CHECK                            385         /* I1 = ip ptr, I2 = needed status, I3 = actual status, I4 = wait option    */
#define NX_TRACE_PACKET_ALLOCATE                            386         /* I1 = pool ptr, I2 = packet ptr, I3 = packet type, I4 = available packets */
#define NX_TRACE_PACKET_COPY                                387         /* I1 = packet ptr, I2 = new packet ptr, I3 = pool ptr, I4 = wait option    */
#define NX_TRACE_PACKET_DATA_APPEND                         388         /* I1 = packet ptr, I2 = data start, I3 = data size, I4 = pool ptr          */
#define NX_TRACE_PACKET_DATA_RETRIEVE                       389         /* I1 = packet ptr, I2 = buffer start, I3 = bytes copied                    */
#define NX_TRACE_PACKET_LENGTH_GET                          390         /* I1 = packet ptr, I2 = length                                             */
#define NX_TRACE_PACKET_POOL_CREATE                         391         /* I1 = pool ptr, I2 = payload size, I3 = memory ptr, I4 = memory_size      */
#define NX_TRACE_PACKET_POOL_DELETE                         392         /* I1 = pool ptr                                                            */
#define NX_TRACE_PACKET_POOL_INFO_GET                       393         /* I1 = pool ptr, I2 = total_packets, I3 = free packets, I4 = empty requests*/
#define NX_TRACE_PACKET_RELEASE                             394         /* I1 = packet ptr, I2 = packet status, I3 = available packets              */
#define NX_TRACE_PACKET_TRANSMIT_RELEASE                    395         /* I1 = packet ptr, I2 = packet status, I3 = available packets              */
#define NX_TRACE_RARP_DISABLE                               396         /* I1 = ip ptr                                                              */
#define NX_TRACE_RARP_ENABLE                                397         /* I1 = ip ptr                                                              */
#define NX_TRACE_RARP_INFO_GET                              398         /* I1 = ip ptr, I2 = requests sent, I3 = responses received, I4 = invalids  */
#define NX_TRACE_SYSTEM_INITIALIZE                          399         /* none                                                                     */
#define NX_TRACE_TCP_CLIENT_SOCKET_BIND                     400         /* I1 = ip ptr, I2 = socket ptr, I3 = port, I4 = wait option                */
#define NX_TRACE_TCP_CLIENT_SOCKET_CONNECT                  401         /* I1 = ip ptr, I2 = socket ptr, I3 = server ip, I4 = server port           */
#define NX_TRACE_TCP_CLIENT_SOCKET_PORT_GET                 402         /* I1 = ip ptr, I2 = socket ptr, I3 = port                                  */
#define NX_TRACE_TCP_CLIENT_SOCKET_UNBIND                   403         /* I1 = ip ptr, I2 = socket ptr                                             */
#define NX_TRACE_TCP_ENABLE                                 404         /* I1 = ip ptr                                                              */
#define NX_TRACE_TCP_FREE_PORT_FIND                         405         /* I1 = ip ptr, I2 = port, I3 = free port                                   */
#define NX_TRACE_TCP_INFO_GET                               406         /* I1 = ip ptr, I2 = bytes sent, I3 = bytes received, I4 = invalid packets  */
#define NX_TRACE_TCP_SERVER_SOCKET_ACCEPT                   407         /* I1 = ip ptr, I2 = socket ptr, I3 = wait option, I4 = socket state        */
#define NX_TRACE_TCP_SERVER_SOCKET_LISTEN                   408         /* I1 = ip ptr, I2 = port, I3 = socket ptr, I4 = listen queue size          */
#define NX_TRACE_TCP_SERVER_SOCKET_RELISTEN                 409         /* I1 = ip ptr, I2 = port, I3 = socket ptr, I4 = socket state               */
#define NX_TRACE_TCP_SERVER_SOCKET_UNACCEPT                 410         /* I1 = ip ptr, I2 = socket ptr, I3 = socket state                          */
#define NX_TRACE_TCP_SERVER_SOCKET_UNLISTEN                 411         /* I1 = ip ptr, I2 = port                                                   */
#define NX_TRACE_TCP_SOCKET_CREATE                          412         /* I1 = ip ptr, I2 = socket ptr, I3 = type of service, I4 = window size     */
#define NX_TRACE_TCP_SOCKET_DELETE                          413         /* I1 = ip ptr, I2 = socket ptr, I3 = socket state                          */
#define NX_TRACE_TCP_SOCKET_DISCONNECT                      414         /* I1 = ip ptr, I2 = socket ptr, I3 = wait option, I4 = socket state        */
#define NX_TRACE_TCP_SOCKET_INFO_GET                        415         /* I1 = ip ptr, I2 = socket ptr, I3 = bytes sent, I4 = bytes received       */
#define NX_TRACE_TCP_SOCKET_MSS_GET                         416         /* I1 = ip ptr, I2 = socket ptr, I3 = mss, I4 = socket state                */
#define NX_TRACE_TCP_SOCKET_MSS_PEER_GET                    417         /* I1 = ip ptr, I2 = socket ptr, I3 = peer_mss, I4 = socket state           */
#define NX_TRACE_TCP_SOCKET_MSS_SET                         418         /* I1 = ip ptr, I2 = socket ptr, I3 = mss, I4 socket state                  */
#define NX_TRACE_TCP_SOCKET_RECEIVE                         419         /* I1 = socket ptr, I2 = packet ptr, I3 = length, I4 = rx sequence          */
#define NX_TRACE_TCP_SOCKET_RECEIVE_NOTIFY                  420         /* I1 = ip ptr, I2 = socket ptr, I3 = receive notify                        */
#define NX_TRACE_TCP_SOCKET_SEND                            421         /* I1 = socket ptr, I2 = packet ptr, I3 = length, I4 = tx sequence          */
#define NX_TRACE_TCP_SOCKET_STATE_WAIT                      422         /* I1 = ip ptr, I2 = socket ptr, I3 = desired state, I4 = previous state    */
#define NX_TRACE_TCP_SOCKET_TRANSMIT_CONFIGURE              423         /* I1 = ip ptr, I2 = socket ptr, I3 = queue depth, I4 = timeout             */
#define NX_TRACE_UDP_ENABLE                                 424         /* I1 = ip ptr                                                              */
#define NX_TRACE_UDP_FREE_PORT_FIND                         425         /* I1 = ip ptr, I2 = port, I3 = free port                                   */
#define NX_TRACE_UDP_INFO_GET                               426         /* I1 = ip ptr, I2 = bytes sent, I3 = bytes received, I4 = invalid packets  */
#define NX_TRACE_UDP_SOCKET_BIND                            427         /* I1 = ip ptr, I2 = socket ptr, I3 = port, I4 = wait option                */
#define NX_TRACE_UDP_SOCKET_CHECKSUM_DISABLE                428         /* I1 = ip ptr, I2 = socket ptr                                             */
#define NX_TRACE_UDP_SOCKET_CHECKSUM_ENABLE                 429         /* I1 = ip ptr, I2 = socket ptr                                             */
#define NX_TRACE_UDP_SOCKET_CREATE                          430         /* I1 = ip ptr, I2 = socket ptr, I3 = type of service, I4 = queue maximum   */
#define NX_TRACE_UDP_SOCKET_DELETE                          431         /* I1 = ip ptr, I2 = socket ptr                                             */
#define NX_TRACE_UDP_SOCKET_INFO_GET                        432         /* I1 = ip ptr, I2 = socket ptr, I3 = bytes sent, I4 = bytes received       */
#define NX_TRACE_UDP_SOCKET_PORT_GET                        433         /* I1 = ip ptr, I2 = socket ptr, I3 = port                                  */
#define NX_TRACE_UDP_SOCKET_RECEIVE                         434         /* I1 = ip ptr, I2 = socket ptr, I3 = packet ptr, I4 = packet size          */
#define NX_TRACE_UDP_SOCKET_RECEIVE_NOTIFY                  435         /* I1 = ip ptr, I2 = socket ptr, I3 = receive notify                        */
#define NX_TRACE_UDP_SOCKET_SEND                            436         /* I1 = socket ptr, I2 = packet ptr, I3 = packet size, I4 = ip address      */
#define NX_TRACE_UDP_SOCKET_UNBIND                          437         /* I1 = ip ptr, I2 = socket ptr, I3 = port                                  */
#define NX_TRACE_UDP_SOURCE_EXTRACT                         438         /* I1 = packet ptr, I2 = ip address, I3 = port                              */
#define NX_TRACE_IP_INTERFACE_ATTACH                        439         /* I1 = ip ptr, I2 = ip address, I3 = interface index                       */
#define NX_TRACE_UDP_SOCKET_BYTES_AVAILABLE                 440         /* I1 = ip ptr, I2 = socket ptr, I3 = bytes available                       */
#define NX_TRACE_IP_STATIC_ROUTE_ENABLE                     441         /* I1 = ip_ptr,                                                             */
#define NX_TRACE_IP_STATIC_ROUTE_DISABLE                    442         /* I1 = ip_ptr,                                                             */
#define NX_TRACE_IP_STATIC_ROUTE_ADD                        443         /* I1 = ip_ptr, I2 = network_address, I3 = net_mask, I4 = next_hop          */
#define NX_TRACE_IP_STATIC_ROUTE_DELETE                     444         /* I1 = ip_ptr, I2 = network_address, I3 = net_mask                         */
#define NX_TRACE_TCP_SOCKET_PEER_INFO_GET                   445         /* I1 = socket ptr, I2 = network_address, I3 = port                         */
#define NX_TRACE_TCP_SOCKET_WINDOW_UPDATE_NOTIFY_SET        446         /* I1 = socket ptr,                                                         */
#define NX_TRACE_UDP_SOCKET_INTERFACE_SET                   447         /* I1 = socket_ptr, I2 = interface_index,                                   */
#define NX_TRACE_UDP_SOCKET_INTERFACE_CLEAR                 448         /* I1 = socket_ptr,                                                         */
#define NX_TRACE_IP_INTERFACE_INFO_GET                      449         /* I1 = ip_ptr, I2 = ip_address, I3 = mtu_size, I4 = interface_index        */
#define NX_TRACE_PACKET_DATA_EXTRACT_OFFSET                 450         /* I1 = packet_ptr, I2 = buffer_length, I3 = bytes_copied,                  */


#endif


#ifndef NXD_TRACE_ICMP_ENABLE
#define NXD_TRACE_ICMP_ENABLE                               470         /* I1 = ip ptr                                                              */
#define NX_TRACE_ICMP_PING6                                 471         /* I1 = ip ptr, I2 = ip_address, I3 = data ptr, I4 = data size              */
#define NXD_TRACE_UDP_SOURCE_EXTRACT                        472         /* I1 = packet ptr, I2 = IP Version (4 or 6), I3 = ip address, I4 = port    */
#define NXD_TRACE_UDP_SOCKET_SET_INTERFACE                  473         /* I1 = udp_socket_ptr, I2 = interface_id                                   */
#define NXD_TRACE_TCP_SOCKET_SET_INTERFACE                  474         /* I1 = tcp_socket_ptr, I2 = interface_id                                   */
#define NXD_TRACE_UDP_SOCKET_SEND                           475         /* I1 = socket ptr, I2 = packet ptr, I3 = packet size, I4 = ip address      */
#define NXD_TRACE_ND_CACHE_DELETE                           476         /* I1 = dest_ip                                                             */
#define NXD_TRACE_ND_CACHE_ENTRY_SET                        477         /* I1 = IP address, I2 = physical msw, I3 = physical lsw                    */
#define NX_TRACE_ND_CACHE_IP_ADDRESS_FIND                   478         /* I1 = ip_ptr, I2 = IP address, I3 = physical msw, I4 = physical lsw       */
#define NXD_TRACE_ND_CACHE_INVALIDATE                       479         /* I1 = ip_ptr                                                              */
#define NXD_TRACE_IPV6_GLOBAL_ADDRESS_GET                   480         /* (Obsolete) I1 = ip_ptr, I2 = IP address lsw, I3 = prefix length          */
#define NXD_TRACE_IPV6_GLOBAL_ADDRESS_SET                   481         /* (Obsolete) I1 = ip_ptr, I2 = IP address lsw, I3 = prefix length          */
#define NX_TRACE_IPSTATIC_ROUTE_ADD                         482         /* I1 = ip_ptr, I2 = network address, I3 = net_mask, I4 = next hop address  */
#define NX_TRACE_IP_STATIC_ROUTING_ENABLE                   483         /* I1 = ip_ptr                                                              */
#define NX_TRACE_IP_STATIC_ROUTING_DISABLE                  484         /* I1 = ip_ptr                                                              */
#define NX_TRACE_IPV6_ENABLE                                485         /* I1 = ip_ptr                                                              */
#define NXD_TRACE_IPV6_RAW_PACKET_SEND                      486         /* I1 = ip_ptr, I2 = ip address lsw, I3 = protocol, I4 = packet_ptr         */
#define NXD_TRACE_IP_RAW_PACKET_SEND                        487         /* I1 = ip_ptr, I2 = ip address lsw, I3 = type of serveice, I4 = packet_ptr */
#define NXD_TRACE_IPV6_LINKLOCAL_ADDRESS_GET                488         /* (Obsolete) I1 = ip_ptr, I2 = IP address lsw                              */
#define NXD_TRACE_IPV6_LINKLOCAL_ADDRESS_SET                489         /* (Obsolete) I1 = ip_ptr, I2 = IP address lsw, I3 = prefix length          */
#define NXD_TRACE_IPV6_INITIATE_DAD_PROCESS                 490         /* I1 = ip_ptr                                                              */
#define NXD_TRACE_IPV6_DEFAULT_ROUTER_ADD                   491         /* I1 = ip_ptr, I2 = router addr lsw, I3 = router lifetime                  */
#define NXD_TRACE_IPV6_DEFAULT_ROUTER_DELETE                492         /* I1 = ip_ptr, I2 = router addr lsw,                                       */
#define NXD_TRACE_IPV6_INTERFACE_ADDRESS_GET                493         /* I1 = ip_ptr, I2 = IP address lsw,I3 = prefix length,I4 = interface_index */
#define NXD_TRACE_IPV6_INTERFACE_ADDRESS_SET                494         /* I1 = ip_ptr, I2 = IP address lsw,I3 = prefix length,I4 = interface_index */
#define NXD_TRACE_TCP_SOCKET_PEER_INFO_GET                  495         /* I1 = socket_ptr, I2 = Peer IP address, I3 = peer_port                    */
#define NXD_TRACE_IP_MAX_PAYLOAD_SIZE_FIND                  496         /* I1 = src address, I2 = dst address,I3 = payload length,I4 = start offset */
#define NX_TRACE_IPV6_DISABLE                               497         /* I1 = ip_ptr                                                              */
#define NXD_TRACE_IPV6_ADDRESS_CHANGE_NOTIFY                498         /* I1 = ip_ptr, I2 = notify_callback                                        */
#define NXD_TRACE_IPV6_STATELESS_ADDRESS_AUTOCONFIG_ENABLE  499         /* I1 = ip_ptr, I2 = interface_index                                        */
#define NXD_TRACE_IPV6_STATELESS_ADDRESS_AUTOCONFIG_DISABLE 500         /* I1 = ip_ptr, I2 = interface_index                                        */
#define NXD_TRACE_IP_RAW_PACKET_FILTER_SET                  501         /* I1 = ip_ptr, I2 = raw_pcket_filter                                       */
#endif

/* Map the trace macros to internal NetX versions so we can get interrupt protection.  */

#ifdef NX_SOURCE_CODE

#define NX_TRACE_OBJECT_REGISTER(t, p, n, a, b)             _nx_trace_object_register(t, (VOID *)p, (CHAR *)n, (ULONG)a, (ULONG)b)
#define NX_TRACE_OBJECT_UNREGISTER(o)                       _nx_trace_object_unregister((VOID *)o)
#define NX_TRACE_IN_LINE_INSERT(i, a, b, c, d, f, g, h)     _nx_trace_event_insert((ULONG)i, (ULONG)a, (ULONG)b, (ULONG)c, (ULONG)d, (ULONG)f, g, h)
#define NX_TRACE_EVENT_UPDATE(e, t, i, a, b, c, d)          _nx_trace_event_update((TX_TRACE_BUFFER_ENTRY *)e, (ULONG)t, (ULONG)i, (ULONG)a, (ULONG)b, (ULONG)c, (ULONG)d)
#endif
/* Define NetX trace prototypes.  */

VOID _nx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2);
VOID _nx_trace_object_unregister(VOID *object_ptr);
VOID _nx_trace_event_insert(ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4, ULONG filter, TX_TRACE_BUFFER_ENTRY **current_event, ULONG *current_timestamp);
VOID _nx_trace_event_update(TX_TRACE_BUFFER_ENTRY *event, ULONG timestamp, ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4);

#else
#define NX_TRACE_OBJECT_REGISTER(t, p, n, a, b)
#define NX_TRACE_OBJECT_UNREGISTER(o)
#define NX_TRACE_IN_LINE_INSERT(i, a, b, c, d, f, g, h)
#define NX_TRACE_EVENT_UPDATE(e, t, i, a, b, c, d)
#endif

/* If NX_PACKET_HEADER_PAD is defined, make sure NX_PACKET_HEADER_PAD_SIZE is also defined.  The default is 1, for backward compatibility. */
#ifdef NX_PACKET_HEADER_PAD
#ifndef NX_PACKET_HEADER_PAD_SIZE
#define NX_PACKET_HEADER_PAD_SIZE                1
#endif /* NX_PACKET_HEADER_PAD_SIZE */
#endif /* NX_PACKET_HEADER_PAD */

/* If NX_PACKET_ALIGNMENT is defined, packet header and payload are aligned automatically to the value specified
   in the symbol. The default is to align packet payload area to ULONG. */
#ifndef NX_PACKET_ALIGNMENT
#define NX_PACKET_ALIGNMENT                      sizeof(ULONG)
#endif /* NX_PACKET_ALIGNMENT */


/* Define basic constants for the NetX TCP/IP Stack.  */
#define AZURE_RTOS_NETXDUO
#define NETXDUO_MAJOR_VERSION                    6
#define NETXDUO_MINOR_VERSION                    2
#define NETXDUO_PATCH_VERSION                    0

/* Define the following symbols for backward compatibility */
#define EL_PRODUCT_NETXDUO
#define __PRODUCT_NETXDUO__
#define __NETXDUO_MAJOR_VERSION__                NETXDUO_MAJOR_VERSION
#define __NETXDUO_MINOR_VERSION__                NETXDUO_MINOR_VERSION

/* API input parameters and general constants.  */
#define NX_NO_WAIT                               0
#define NX_WAIT_FOREVER                          ((ULONG)0xFFFFFFFF)
#define NX_TRUE                                  1
#define NX_FALSE                                 0
#define NX_NULL                                  0
#define NX_FOREVER                               1
#define NX_INIT_PACKET_ID                        1
#ifndef NX_MAX_PORT
#define NX_MAX_PORT                              0xFFFF
#endif /* NX_MAX_PORT */
#define NX_LOWER_16_MASK                         ((ULONG)0x0000FFFF)
#define NX_CARRY_BIT                             ((ULONG)0x10000)
#define NX_SHIFT_BY_16                           16
#define NX_TCP_CLIENT                            1
#define NX_TCP_SERVER                            2
#define NX_ANY_PORT                              0
#ifndef NX_SEARCH_PORT_START
#define NX_SEARCH_PORT_START                     0xC000 /* Free port search start UDP/TCP */
#endif /* NX_SEARCH_PORT_START */

#ifndef NX_PHYSICAL_HEADER
#define NX_PHYSICAL_HEADER                       16    /* Maximum physical header        */
#endif

#ifndef NX_PHYSICAL_TRAILER
#define NX_PHYSICAL_TRAILER                      4     /* Maximum physical trailer       */
#endif

/* Specify th wait interval, in seconds. */
#ifndef NX_PATH_MTU_INCREASE_WAIT_INTERVAL
#define NX_PATH_MTU_INCREASE_WAIT_INTERVAL       600
#endif

#define NX_PATH_MTU_INCREASE_WAIT_INTERVAL_TICKS (NX_PATH_MTU_INCREASE_WAIT_INTERVAL * NX_IP_PERIODIC_RATE)

/* By default IPv6 is enabled. */
#ifndef NX_DISABLE_IPV6
#ifndef FEATURE_NX_IPV6
#define FEATURE_NX_IPV6
#endif /* FEATURE_NX_IPV6 */
#endif /* NX_DISABLE_IPV6 */

/* Remove the IPv6 component if NX_DISABLE_IPV6 is defined. */
#ifdef NX_DISABLE_IPV6
#ifdef FEATURE_NX_IPV6
#undef FEATURE_NX_IPV6
#endif /* FEATURE_NX_IPV6 */
#endif /* NX_DISABLE_IPV6 */

#ifdef NX_IPSEC_ENABLE

/* Define the IPsec header size.  The space reserved here is good for ESP/AH header and possiblly IPv4/v6 header in tunnel mode. */
#ifdef FEATURE_NX_IPV6
#define NX_IPSEC_MAX_HEADER_SIZE    (((NX_CRYPTO_MAX_IV_SIZE_IN_BITS + NX_CRYPTO_AUTHENTICATION_ICV_TRUNC_BITS + 7) / 8) + 16 + 40)
#else /* !FEATURE_NX_IPV6 */
#define NX_IPSEC_MAX_HEADER_SIZE    (((NX_CRYPTO_MAX_IV_SIZE_IN_BITS + NX_CRYPTO_AUTHENTICATION_ICV_TRUNC_BITS + 7) / 8) + 12 + 20)
#endif /* FEATURE_NX_IPV6 */

#else /* !NX_IPSEC_ENABLE */
#define NX_IPSEC_MAX_HEADER_SIZE    0
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_IPV6_HOST_ID_LENGTH
#define NX_IPV6_HOST_ID_LENGTH      64
#endif /* NX_IPV6_HOST_ID_LENGTH */

#define NX_IPv4_PACKET              (NX_PHYSICAL_HEADER + 20)   /* 20 bytes of IP header          */
#define NX_IPv4_TCP_PACKET          (NX_IPv4_PACKET + 20)       /* IP header plus 20 bytes        */
#define NX_IPv4_UDP_PACKET          (NX_IPv4_PACKET + 8)        /* IP header plus 8 bytes         */
#define NX_IPv4_ICMP_PACKET         (NX_IPv4_PACKET)
#define NX_IPv4_IGMP_PACKET         (NX_IPv4_PACKET)

#define NX_IPv6_PACKET              (NX_PHYSICAL_HEADER + 40)   /* 40 bytes of basic IPv6 Header  */
#define NX_IPv6_UDP_PACKET          (NX_IPv6_PACKET + 8)        /* IPv6 header plus 8 bytes       */
#define NX_IPv6_TCP_PACKET          (NX_IPv6_PACKET + 20)       /* IPv6 header plus 20 bytes      */
#define NX_IPv6_ICMP_PACKET         (NX_IPv6_PACKET)
#define NX_RECEIVE_PACKET           0                           /* This is for driver receive     */


#ifdef FEATURE_NX_IPV6
/* For systems with IPv6 enabled, the payload offset defaults to IPv6 header size. */
#define NX_IP_PACKET                NX_IPv6_PACKET
#define NX_TCP_PACKET               (NX_IPv6_TCP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#define NX_UDP_PACKET               (NX_IPv6_UDP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#define NX_ICMP_PACKET              (NX_IPv6_ICMP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#else
/* Define payload offset to IPv4 header size if IPv6 is not defined.  */
#define NX_IP_PACKET                NX_IPv4_PACKET
#define NX_TCP_PACKET               (NX_IPv4_TCP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#define NX_UDP_PACKET               (NX_IPv4_UDP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#define NX_ICMP_PACKET              (NX_IPv4_ICMP_PACKET + NX_IPSEC_MAX_HEADER_SIZE)
#endif /* FEATURE_NX_IPV6 */
#define NX_IGMP_PACKET              NX_IPv4_IGMP_PACKET


/* Define the ARP update rate, in terms of IP periodic intervals.  This can be defined on the
   command line as well.  */

#ifndef NX_ARP_UPDATE_RATE
#define NX_ARP_UPDATE_RATE          10
#endif


/* Define the ARP entry expiration rate, in terms of IP periodic intervals.  This can be defined on the
   command line as well.   A value of 0 disables ARP entry expiration, and is the default.  */

#ifndef NX_ARP_EXPIRATION_RATE
#define NX_ARP_EXPIRATION_RATE      0
#endif


/* Define the ARP maximum retry constant that specifies the maximum number of ARP requests that will be sent
   without receiving an ARP response.  Once this limit is reached, the ARP attempt is abandoned and
   any queued packet is released.  */

#ifndef NX_ARP_MAXIMUM_RETRIES
#define NX_ARP_MAXIMUM_RETRIES      18
#endif


/* Define the maximum number of packets that can be queued while waiting for ARP resolution of an
   IP address.  */

#ifndef NX_ARP_MAX_QUEUE_DEPTH
#define NX_ARP_MAX_QUEUE_DEPTH      4
#endif


/* Define the maximum time of IP reassembly.  */
#ifndef NX_IP_MAX_REASSEMBLY_TIME

/* Define the maximum time of IPv4 reassembly. RFC791, Section3.2, page27.  */
#ifndef NX_IPV4_MAX_REASSEMBLY_TIME
#define NX_IPV4_MAX_REASSEMBLY_TIME 15
#endif

/* Define the maximum time of IPv6 reassembly. RFC2460, Section4.5, page22.  */
#ifndef NX_IPV6_MAX_REASSEMBLY_TIME
#define NX_IPV6_MAX_REASSEMBLY_TIME 60
#endif

#else /* NX_IP_MAX_REASSEMBLY_TIME */

/* Define the maximum time of IP reassembly.  */
#define NX_IPV4_MAX_REASSEMBLY_TIME NX_IP_MAX_REASSEMBLY_TIME
#define NX_IPV6_MAX_REASSEMBLY_TIME NX_IP_MAX_REASSEMBLY_TIME

#endif /* NX_IP_MAX_REASSEMBLY_TIME */


/* Define the maximum number of the packets that can be queued for raw receive. */
#ifndef NX_IP_RAW_MAX_QUEUE_DEPTH
#define NX_IP_RAW_MAX_QUEUE_DEPTH   20
#endif

#ifndef NX_IP_ROUTING_TABLE_SIZE
#define NX_IP_ROUTING_TABLE_SIZE    8
#endif /* NX_IP_ROUTING_TABLE_SIZE */

/* For backward compatibility, map the smbol NX_RAW_PACKET_FILTER_ENABLE to
   NX_ENABLE_IP_RAW_PACKET_FILTER. */
#ifdef NX_RAW_PACKET_FILTER_ENABLE
#ifndef NX_ENABLE_IP_RAW_PACKET_FILTER
#define NX_ENABLE_IP_RAW_PACKET_FILTER
#endif /* NX_ENABLE_IP_RAW_PACKET_FILTER */
#endif /* NX_RAW_PACKET_FILTER_ENABLE */

/* For backward compatibility, map the symbol NXDUO_DISABLE_ICMPV6_REDIRECT_PROCESS
   to NX_DISABLE_ICMPV6_REDIRECT_PROCESS.   */
#ifdef NXDUO_DISABLE_ICMPV6_REDIRECT_PROCESS
#ifndef NX_DISABLE_ICMPV6_REDIRECT_PROCESS
#define NX_DISABLE_ICMPV6_REDIRECT_PROCESS
#endif /* NX_DISABLE_ICMPV6_REDIRECT_PROCESS */
#endif /* NXDUO_DISABLE_ICMPV6_REDIRECT_PROCESS */

/* For backward compatibility, convert map NXDUO_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS to NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS */
#ifdef NXDUO_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
#ifndef NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
#define NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
#endif /* NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS */
#endif /* NXDUO_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS */

/* For backward compatibility, convert map NXDUO_DISABLE_ICMPV6_ROUTER_SOLICITATION to NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */
#ifdef NXDUO_DISABLE_ICMPV6_ROUTER_SOLICITATION
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
#define NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */
#endif /* NXDUO_DISABLE_ICMPV6_ROUTER_SOLICITATION */

/* For backward compatibility, convert map NXDUO_DISABLE_ICMPV6_ERROR_MESSAGE to NX_DISABLE_ICMPV6_ERROR_MESSAGE */
#ifdef NXDUO_DISABLE_ICMPV6_ERROR_MESSAGE
#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
#define NX_DISABLE_ICMPV6_ERROR_MESSAGE
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */
#endif /* NXDUO_DISABLE_ICMPV6_ERROR_MESSAGE */

/* For backward compatibility, convert map NXDUO_DISABLE_DAD to NX_DISABLE_IPV6_DAD */
#ifdef NXDUO_DISABLE_DAD
#ifndef NX_DISABLE_IPV6_DAD
#define NX_DISABLE_IPV6_DAD
#endif /* NX_DISABLE_IPV6_DAD */
#endif /* NXDUO_DISABLE_DAD */

/* For backward compatibility, convert map NX_IPV6_ADDRESS_CHANGE_NOTIFY_ENABLE to NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
#ifdef NX_IPV6_ADDRESS_CHANGE_NOTIFY_ENABLE
#ifndef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
#define NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
#endif /* NX_IPV6_ADDRESS_CHANGE_NOTIFY_ENABLE */

/* For backward compatibility, convert map NX_IPV6_DISABLE_PURGE_UNUSED_CACHE_ENTRIES to NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES */
#ifdef NX_IPV6_DISABLE_PURGE_UNUSED_CACHE_ENTRIES
#ifndef NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
#define NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
#endif /* NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES */
#endif /* NX_IPV6_DISABLE_PURGE_UNUSED_CACHE_ENTRIES */

/* For backward compatibility, convert map NX_IPV6_MULTICAST_ENABLE to NX_ENABLE_IPV6_MULTICAST */
#ifdef NX_IPV6_MULTICAST_ENABLE
#ifndef NX_ENABLE_IPV6_MULTICAST
#define NX_ENABLE_IPV6_MULTICAST
#endif /* NX_ENABLE_IPV6_MULTICAST */
#endif /* NX_IPV6_MULTICAST_ENABLE */

/* Define the size of the IPv6 destination table. */
/* For backward compatibility, convert map NX_DESTINATION_TABLE_SIZE and NXDUO_DESTINATION_TABLE_SIZE to NX_IPV6_DESTINATION_TABLE_SIZE */
#ifdef NX_DESTINATION_TABLE_SIZE
#define NX_IPV6_DESTINATION_TABLE_SIZE  NX_DESTINATION_TABLE_SIZE
#endif /* NX_DESTINATION_TABLE_SIZE */

#ifdef NXDUO_DESTINATION_TABLE_SIZE
#define NX_IPV6_DESTINATION_TABLE_SIZE  NXDUO_DESTINATION_TABLE_SIZE
#endif /* NXDUO_DESTINATION_TABLE_SIZE */

#ifndef NX_IPV6_DESTINATION_TABLE_SIZE
#define NX_IPV6_DESTINATION_TABLE_SIZE  8
#ifndef NX_DESTINATION_TABLE_SIZE
#define NX_DESTINATION_TABLE_SIZE       NX_IPV6_DESTINATION_TABLE_SIZE
#endif /* NX_DESTINATION_TABLE_SIZE */
#ifndef NXDUO_DESTINATION_TABLE_SIZE
#define NXDUO_DESTINATION_TABLE_SIZE    NX_IPV6_DESTINATION_TABLE_SIZE
#endif /* NXDUO_DESTINATION_TABLE_SIZE */
#endif /* NX_IPV6_DESTINATION_TABLE_SIZE */

/* Define the size of the IPv6 ND cache table. */
#ifndef NX_IPV6_NEIGHBOR_CACHE_SIZE
#define NX_IPV6_NEIGHBOR_CACHE_SIZE     16
#endif /* NX_IPV6_NEIGHBOR_CACHE_SIZE */

/* Define the maximum ICMPv6 Duplicate Address Detect Transmit .  */
/* For backward compatibility, convert map NX_DUP_ADDR_DETECT_TRANSMITS and NXDUO_DUP_ADDR_DETECT_TRANSMITS to NX_IPV6_DAD_TRANSMITS */
#ifdef NX_DUP_ADDR_DETECT_TRANSMITS
#define NX_IPV6_DAD_TRANSMITS           NX_DUP_ADDR_DETECT_TRANSMITS
#endif /* NX_DUP_ADDR_DETECT_TRANSMITS */

#ifdef NXDUO_DUP_ADDR_DETECT_TRANSMITS
#define NX_IPV6_DAD_TRANSMITS           NXDUO_DUP_ADDR_DETECT_TRANSMITS
#endif /* NXDUO_DUP_ADDR_DETECT_TRANSMITS */

#ifndef NX_IPV6_DAD_TRANSMITS
#define NX_IPV6_DAD_TRANSMITS           3
#ifndef NX_DUP_ADDR_DETECT_TRANSMITS
#define NX_DUP_ADDR_DETECT_TRANSMITS    NX_IPV6_DAD_TRANSMITS
#endif /* NX_DUP_ADDR_DETECT_TRANSMITS */
#ifndef NXDUO_DUP_ADDR_DETECT_TRANSMITS
#define NXDUO_DUP_ADDR_DETECT_TRANSMITS NX_IPV6_DAD_TRANSMITS
#endif /* NXDUO_DUP_ADDR_DETECT_TRANSMITS */
#endif /* NX_IPV6_DAD_TRANSMITS */

/* For backward compatibility, convert map NX_ARP_DISABLE_AUTO_ARP_ENTRY to NX_DISABLE_ARP_AUTO_ENTRY. */
#ifdef NX_ARP_DISABLE_AUTO_ARP_ENTRY
#ifndef NX_DISABLE_ARP_AUTO_ENTRY
#define NX_DISABLE_ARP_AUTO_ENTRY
#endif /* NX_DISABLE_ARP_AUTO_ENTRY */
#endif /* NX_ARP_DISABLE_AUTO_ARP_ENTRY */

/* For backward compatibility, convert map NX_TCP_ENABLE_KEEPALIVE to NX_ENABLE_TCP_KEEPALIVE. */
#ifdef NX_TCP_ENABLE_KEEPALIVE
#ifndef NX_ENABLE_TCP_KEEPALIVE
#define NX_ENABLE_TCP_KEEPALIVE
#endif /* NX_ENABLE_TCP_KEEPALIVE */
#endif /* NX_TCP_ENABLE_KEEPALIVE */

/* For backward compatibility, convert map NX_TCP_ENABLE_WINDOW_SCALING to NX_ENABLE_TCP_WINDOW_SCALING. */
#ifdef NX_TCP_ENABLE_WINDOW_SCALING
#ifndef NX_ENABLE_TCP_WINDOW_SCALING
#define NX_ENABLE_TCP_WINDOW_SCALING
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
#endif /* NX_TCP_ENABLE_WINDOW_SCALING */

/* For backward compatibility, convert map NX_TCP_ENABLE_MSS_CHECK to NX_ENABLE_TCP_MSS_CHECK. */
#ifdef NX_TCP_ENABLE_MSS_CHECK
#ifndef NX_ENABLE_TCP_MSS_CHECK
#define NX_ENABLE_TCP_MSS_CHECK
#endif /* NX_ENABLE_TCP_MSS_CHECK */
#endif /* NX_TCP_ENABLE_MSS_CHECK */

/* For backward compatibility, convert map NX_DUAL_PACKET_POOL_ENABLE to NX_ENABLE_DUAL_PACKET_POOL. */
#ifdef NX_DUAL_PACKET_POOL_ENABLE
#ifndef NX_ENABLE_DUAL_PACKET_POOL
#define NX_ENABLE_DUAL_PACKET_POOL
#endif /* NX_ENABLE_DUAL_PACKET_POOL. */
#endif /* NX_DUAL_PACKET_POOL_ENABLE */

/* For backward compatibility, convert map NX_ENABLE_FEATURE_LOW_WATERMARK to NX_ENABLE_LOW_WATERMARK. */
#ifdef NX_ENABLE_FEATURE_LOW_WATERMARK
#ifndef NX_ENABLE_LOW_WATERMARK
#define NX_ENABLE_LOW_WATERMARK
#endif /* NX_ENABLE_LOW_WATERMARK */
#endif /* NX_ENABLE_FEATURE_LOW_WATERMARK */

#ifdef NX_DISABLE_ICMP_RX_CHECKSUM
#ifndef NX_DISABLE_ICMPV4_RX_CHECKSUM
#define NX_DISABLE_ICMPV4_RX_CHECKSUM
#endif
#ifndef NX_DISABLE_ICMPV6_RX_CHECKSUM
#define NX_DISABLE_ICMPV6_RX_CHECKSUM
#endif
#endif /* NX_DISABLE_ICMP_RX_CHECKSUM */

#ifdef NX_DISABLE_ICMP_TX_CHECKSUM
#ifndef NX_DISABLE_ICMPV4_TX_CHECKSUM
#define NX_DISABLE_ICMPV4_TX_CHECKSUM
#endif
#ifndef NX_DISABLE_ICMPV6_TX_CHECKSUM
#define NX_DISABLE_ICMPV6_TX_CHECKSUM
#endif
#endif /* NX_DISABLE_ICMP_TX_CHECKSUM */

/* For backward compatibility, convert map NX_DISABLE_ICMPv4_RX_CHECKSUM to NX_DISABLE_ICMPV4_RX_CHECKSUM. */
#ifdef NX_DISABLE_ICMPv4_RX_CHECKSUM
#ifndef NX_DISABLE_ICMPV4_RX_CHECKSUM
#define NX_DISABLE_ICMPV4_RX_CHECKSUM
#endif /* NX_DISABLE_ICMPV4_RX_CHECKSUM */
#endif /* NX_DISABLE_ICMPv4_RX_CHECKSUM */

/* For backward compatibility, convert map NX_DISABLE_ICMPv4_TX_CHECKSUM to NX_DISABLE_ICMPV4_TX_CHECKSUM. */
#ifdef NX_DISABLE_ICMPv4_TX_CHECKSUM
#ifndef NX_DISABLE_ICMPV4_TX_CHECKSUM
#define NX_DISABLE_ICMPV4_TX_CHECKSUM
#endif /* NX_DISABLE_ICMPV4_TX_CHECKSUM */
#endif /* NX_DISABLE_ICMPv4_TX_CHECKSUM */

/* For backward compatibility, convert map NX_DISABLE_ICMPv6_RX_CHECKSUM to NX_DISABLE_ICMPV6_RX_CHECKSUM. */
#ifdef NX_DISABLE_ICMPv6_RX_CHECKSUM
#ifndef NX_DISABLE_ICMPV6_RX_CHECKSUM
#define NX_DISABLE_ICMPV6_RX_CHECKSUM
#endif /* NX_DISABLE_ICMPV6_RX_CHECKSUM */
#endif /* NX_DISABLE_ICMPv6_RX_CHECKSUM */

/* For backward compatibility, convert map NX_DISABLE_ICMPv6_TX_CHECKSUM to NX_DISABLE_ICMPV6_TX_CHECKSUM. */
#ifdef NX_DISABLE_ICMPv6_TX_CHECKSUM
#ifndef NX_DISABLE_ICMPV6_TX_CHECKSUM
#define NX_DISABLE_ICMPV6_TX_CHECKSUM
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */
#endif /* NX_DISABLE_ICMPv6_TX_CHECKSUM */



#ifdef NX_ENABLE_EXTENDED_NOTIFY_SUPPORT
#ifdef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
#undef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
#endif /* NX_ENABLE_EXTENDED_NOTIFY_SUPPORT */
#else
#define NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
#endif /* !NX_ENABLE_EXTENDED_NOTIFY_SUPPORT */

/* Define the IPv4 fragment options.  */

#define NX_FRAGMENT_OKAY                       ((ULONG)0x00000000)
#define NX_DONT_FRAGMENT                       ((ULONG)0x00004000)
#define NX_MORE_FRAGMENTS                      ((ULONG)0x00002000)
#define NX_FRAG_OFFSET_MASK                    ((ULONG)0x00001FFF)


/* Define the IPv4 Type Of Service constants.  These will be supplied to the
   IPv4 output packet routine.  */

#define NX_IP_NORMAL                           ((ULONG)0x00000000) /* Normal IP delivery                     */
#define NX_IP_MIN_DELAY                        ((ULONG)0x00100000) /* Minimum Delay delivery                 */
#define NX_IP_MAX_DATA                         ((ULONG)0x00080000) /* Maximum Throughput delivery            */
#define NX_IP_MAX_RELIABLE                     ((ULONG)0x00040000) /* Maximum Reliable delivery              */
#define NX_IP_MIN_COST                         ((ULONG)0x00020000) /* Minimum Cost deliver                   */
#define NX_IP_TOS_MASK                         ((ULONG)0x00FF0000) /* Type of Service Mask                   */


/* Define the IPv4 length mask.   */

#define NX_IP_PACKET_SIZE_MASK                 ((ULONG)0x0000FFFF) /* Mask for isolating the IP packet length */


/* Define the IPv4 default time to live.  */

#define NX_IP_TIME_TO_LIVE                     ((ULONG)0x00000080) /* Default packet time to live            */
#define NX_IP_TIME_TO_LIVE_MASK                ((ULONG)0xFF000000) /* Mask for isolating the time to live    */
#define NX_IP_TIME_TO_LIVE_SHIFT               24                  /* Number of bits to shift left           */




/* Define IPv6 protocol types.  These types are used in the
   "next header" field in IPv6 header and optional headers. */
#define NX_PROTOCOL_NEXT_HEADER_HOP_BY_HOP     0
#define NX_PROTOCOL_ICMP                       1
#define NX_PROTOCOL_IGMP                       2
#define NX_PROTOCOL_IPV4                       4   /* IP-in-IP encapsulation */
#define NX_PROTOCOL_TCP                        6
#define NX_PROTOCOL_UDP                        17
#define NX_PROTOCOL_IPV6                       41  /* IP-in-IP encapsulation */
#define NX_PROTOCOL_NEXT_HEADER_ROUTING        43
#define NX_PROTOCOL_NEXT_HEADER_FRAGMENT       44
#define NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY 50
#define NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION 51
#define NX_PROTOCOL_ICMPV6                     58
#define NX_PROTOCOL_NO_NEXT_HEADER             59
#define NX_PROTOCOL_NEXT_HEADER_DESTINATION    60
#define NX_PROTOCOL_NEXT_HEADER_MOBILE         135

/* Define the type of Protocol in this IPv6 packet.  */
/* Note that these values can be directly mapped to the protocol field in an IPv4 header. */
#define NX_IP_ICMP                             ((ULONG)0x00010000) /* ICMP Protocol Type                     */
#define NX_IP_IGMP                             ((ULONG)0x00020000) /* IGMP Protocol Type                     */
#define NX_IP_TCP                              ((ULONG)0x00060000) /* TCP Protocol Type                      */
#define NX_IP_UDP                              ((ULONG)0x00110000) /* UDP Protocol Type                      */
#define NX_IP_ESP                              ((ULONG)0x00320000) /* ESP Protocol Type                      */
#define NX_IP_AH                               ((ULONG)0x00330000) /* UDP Protocol Type                      */
#define NX_IP_PROTOCOL_MASK                    ((ULONG)0x00FF0000) /* Protocol Type mask                     */


/* Define IPv4 address type masks and values.  These will determine the net id and
   host id fields of the supplied IPv4 address.  */

#define NX_IP_CLASS_A_MASK                     ((ULONG)0x80000000) /* Define mask for class A IP addresses   */
#define NX_IP_CLASS_A_TYPE                     ((ULONG)0x00000000) /* Define class A address type            */
#define NX_IP_CLASS_A_NETID                    ((ULONG)0x7F000000) /* Define class A network ID mask         */
#define NX_IP_CLASS_A_HOSTID                   ((ULONG)0x00FFFFFF) /* Define class A host ID mask            */

#define NX_IP_CLASS_B_MASK                     ((ULONG)0xC0000000) /* Define mask for class B IP addresses   */
#define NX_IP_CLASS_B_TYPE                     ((ULONG)0x80000000) /* Define class B address type            */
#define NX_IP_CLASS_B_NETID                    ((ULONG)0x3FFF0000) /* Define class B network ID mask         */
#define NX_IP_CLASS_B_HOSTID                   ((ULONG)0x0000FFFF) /* Define class B host ID mask            */

#define NX_IP_CLASS_C_MASK                     ((ULONG)0xE0000000) /* Define mask for class C IP addresses   */
#define NX_IP_CLASS_C_TYPE                     ((ULONG)0xC0000000) /* Define class C address type            */
#define NX_IP_CLASS_C_NETID                    ((ULONG)0x1FFFFF00) /* Define class C network ID mask         */
#define NX_IP_CLASS_C_HOSTID                   ((ULONG)0x000000FF) /* Define class C host ID mask            */

#define NX_IP_CLASS_D_MASK                     ((ULONG)0xF0000000) /* Define mask for class D IP addresses   */
#define NX_IP_CLASS_D_TYPE                     ((ULONG)0xE0000000) /* Define class D address type            */
#define NX_IP_CLASS_D_GROUP                    ((ULONG)0x0FFFFFFF) /* Define class D group ID mask           */
#define NX_IP_CLASS_D_HOSTID                   ((ULONG)0x00000000) /* Define class D host ID mask N/A        */

#define NX_IP_LIMITED_BROADCAST                ((ULONG)0xFFFFFFFF) /* Limited broadcast address (local net)  */
#define NX_IP_LOOPBACK_FIRST                   ((ULONG)0x7F000000) /* First loopback address 127.0.0.0       */
#define NX_IP_LOOPBACK_LAST                    ((ULONG)0x7FFFFFFF) /* Last loopback address  127.255.255.255 */

#define NX_IP_MULTICAST_UPPER                  ((ULONG)0x00000100) /* Upper two bytes of multicast Ethernet  */
#define NX_IP_MULTICAST_LOWER                  ((ULONG)0x5E000000) /* Lower 23 bits of address are from IP   */
#define NX_IP_MULTICAST_MASK                   ((ULONG)0x007FFFFF) /* Mask to pickup the lower 23 bits of IP */


/* Define the constants that determine how big the hash table is for destination IP
   addresses.  The value must be a power of two, so subtracting one gives us
   the mask.  */

#define NX_ROUTE_TABLE_SIZE                    32
#define NX_ROUTE_TABLE_MASK                    (NX_ROUTE_TABLE_SIZE - 1)

/* By default use 0xFF when sending raw packet.  */
#ifndef NX_IP_RAW
#define NX_IP_RAW                              0x00FF0000
#endif /* NX_IP_RAW */

/*
 * NX_ROUTE_TABLE_SIZE|MASK are used for ARP table. So we define NX_ARP_TABLE_SIZE|MASK so the
 *    macro names are more descriptive.
 *
 * NX_ROUTE_TABLE_SIZE and NX_ROUTE_TABLE_MASK will be deprecated in future releases.
 */
#define NX_ARP_TABLE_SIZE                      NX_ROUTE_TABLE_SIZE
#define NX_ARP_TABLE_MASK                      (NX_ARP_TABLE_SIZE - 1)



#ifdef FEATURE_NX_IPV6
/* Define IPv6 address state.  These states are defined in RFC 2462. */

#define NX_IPV6_ADDR_STATE_UNKNOWN             0x00
#define NX_IPV6_ADDR_STATE_TENTATIVE           0x01
#define NX_IPV6_ADDR_STATE_PREFERRED           0x02
#define NX_IPV6_ADDR_STATE_DEPRECATED          0x03
#define NX_IPV6_ADDR_STATE_VALID               0x04



/* Define the size of the IPv6 Default Router Table. */
#ifndef NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE
#define NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE      8
#endif /* NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE */

/* Define the size of the IPv6 prefix table. */
#ifndef NX_IPV6_PREFIX_LIST_TABLE_SIZE
#define NX_IPV6_PREFIX_LIST_TABLE_SIZE         8
#endif /* NX_IPV6_PREFIX_LIST_TABLE_SIZE */

/* Flags for the IPv6 default router table status field. */
#define NX_IPV6_ROUTE_TYPE_NOT_ROUTER          0x00
#define NX_IPV6_ROUTE_TYPE_SOLICITATED         0x01
#define NX_IPV6_ROUTE_TYPE_UNSOLICITATED       0x02
#define NX_IPV6_ROUTE_TYPE_STATIC              0x04
#define NX_IPV6_ROUTE_TYPE_DEFAULT             0x40
#define NX_IPV6_ROUTE_TYPE_VALID               0x80

#define NX_IPV6_ADDRESS_NOT_CONFIGURED         0
#define NX_IPV6_ADDRESS_MANUAL_CONFIG          1
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
#define NX_IPV6_ADDRESS_STATELESS_AUTO_CONFIG  2
#define NX_IPV6_ADDRESS_STATEFUL_AUTO_CONFIG   3
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
#define NX_IPV6_ADDRESS_BASED_ON_INTERFACE     4
#define NX_IPV6_ADDRESS_MANUAL_DELETE          5
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
#define NX_IPV6_ADDRESS_LIFETIME_EXPIRED       6
#define NX_IPV6_ADDRESS_DAD_SUCCESSFUL         7
#define NX_IPV6_ADDRESS_DAD_FAILURE            8
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */



/* Define the maximum number of packets that can be queued while waiting for Neighbor Discovery
   to resolution of an IPv6 address.  */
#ifndef NX_ND_MAX_QUEUE_DEPTH
#define NX_ND_MAX_QUEUE_DEPTH                  4
#endif

/* Define the ND cache entry structure. */
typedef struct ND_CACHE_ENTRY_STRUCT
{

    /* Neighbor IP address. */
    ULONG nx_nd_cache_dest_ip[4];

    /* Corresponding LLA.   */
    UCHAR nx_nd_cache_mac_addr[6];

    /* Padding.  Reserved for future use. */
    /*lint -esym(768,ND_CACHE_ENTRY_STRUCT::nx_nd_cache_reserved1) suppress member not referenced. It is reserved for future use. */
    UCHAR nx_nd_cache_reserved1;

    /* Padding.  Reserved for future use. */
    /*lint -esym(768,ND_CACHE_ENTRY_STRUCT::nx_nd_cache_reserved2) suppress member not referenced. It is reserved for future use. */
    UCHAR nx_nd_cache_reserved2;

    /* Number of Solicitation it needs to send before timing out. */
    UCHAR nx_nd_cache_num_solicit;

    /* Entry Status, such as INCOMPLETE, REACHABLE, and so on. */
    UCHAR nx_nd_cache_nd_status;

    /* Number of out going packets waiting for this entry to be resolved. */
    UCHAR nx_nd_cache_packet_waiting_queue_length;

    /* Whether or not this entry is statically configured. */
    UCHAR nx_nd_cache_is_static;

    /* Timeout value. */
    ULONG nx_nd_cache_timer_tick;

    /* Interface through which the destination can be reached. */
    struct NX_INTERFACE_STRUCT *nx_nd_cache_interface_ptr;

    /* Link to the default router table, if this neighbor is a router. */
    struct NX_IPV6_DEFAULT_ROUTER_ENTRY_STRUCT *nx_nd_cache_is_router;

    /*
     * Queue head and queue tail of the out going packets.
     * This queue is used for keeping outgoing packets while the stack is
     * resolving the target link layer address.
     */
    struct NX_PACKET_STRUCT *nx_nd_cache_packet_waiting_head;
    struct NX_PACKET_STRUCT *nx_nd_cache_packet_waiting_tail;

    /*
     * Local interface associated with this neighbor entry. If outoing_address is known
     * outgoing packets shall be sent using this address.
     */
    struct NXD_IPV6_ADDRESS_STRUCT *nx_nd_cache_outgoing_address;
} ND_CACHE_ENTRY;

/* Define the destination table entry type. */
typedef struct NX_IPV6_DESTINATION_ENTRY_STRUCT
{
    /* Flag indicates whether or not the entry is valid. */
    ULONG nx_ipv6_destination_entry_valid;

    /* Destination IP address. */
    ULONG nx_ipv6_destination_entry_destination_address[4];

    /* Next hop address.  Next hop could be the host, if it
       is on the local network, or a router. */
    ULONG nx_ipv6_destination_entry_next_hop[4];

    /* Cross link to the next hop entry in the ND cache. */
    ND_CACHE_ENTRY *nx_ipv6_destination_entry_nd_entry;

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

    /* Maximum transmission size for this destination. */
    ULONG nx_ipv6_destination_entry_path_mtu;

    /* MTU Timeout value. */
    ULONG nx_ipv6_destination_entry_MTU_timer_tick;
#endif
} NX_IPV6_DESTINATION_ENTRY;

/* Define data structure for IPv6 prefix table. */
typedef struct NX_IPV6_PREFIX_ENTRY_STRUCT
{
    /* Network IPv6 prefix address, in host byte order. */
    ULONG nx_ipv6_prefix_entry_network_address[4];

    /* Number of valid bits in the prefix. */
    ULONG nx_ipv6_prefix_entry_prefix_length;

    /* Valid life time, in seconds. */
    ULONG nx_ipv6_prefix_entry_valid_lifetime;

    /* Pointer to the previous entry. */
    struct NX_IPV6_PREFIX_ENTRY_STRUCT *nx_ipv6_prefix_entry_prev;

    /* Pointer to the next entry. */
    struct NX_IPV6_PREFIX_ENTRY_STRUCT *nx_ipv6_prefix_entry_next;
} NX_IPV6_PREFIX_ENTRY;


/* Define data structure for IPv6 default router table. */
typedef struct NX_IPV6_DEFAULT_ROUTER_ENTRY_STRUCT
{
    /* route type */
    UCHAR nx_ipv6_default_router_entry_flag;

    /* Reserved for future expansion. */
    /*lint -esym(768,NX_IPV6_DEFAULT_ROUTER_ENTRY_STRUCT::nx_ipv6_default_router_entry_reserved) suppress member not referenced. It is reserved for future use. */
    UCHAR nx_ipv6_default_router_entry_reserved;

    /* Router Life Time, in seconds, host-byte order. */
    USHORT nx_ipv6_default_router_entry_life_time;

    /* Router interface address. */
    ULONG nx_ipv6_default_router_entry_router_address[4];

    /* Router interface index.  */
    struct NX_INTERFACE_STRUCT *nx_ipv6_default_router_entry_interface_ptr;

    /* Point to the corresonding neighbor cache entry. */
    ND_CACHE_ENTRY *nx_ipv6_default_router_entry_neighbor_cache_ptr;
} NX_IPV6_DEFAULT_ROUTER_ENTRY;

#endif /* FEATURE_NX_IPV6 */
/* Define the constants that determine how big the hash table is for UDP ports.  The
   value must be a power of two, so subtracting one gives us the mask.  */

#define NX_UDP_PORT_TABLE_SIZE                     32
#define NX_UDP_PORT_TABLE_MASK                     (NX_UDP_PORT_TABLE_SIZE - 1)


/* Define the constants that determine how big the hash table is for TCP ports.  The
   value must be a power of two, so subtracting one gives us the mask.  */

#define NX_TCP_PORT_TABLE_SIZE                     32
#define NX_TCP_PORT_TABLE_MASK                     (NX_TCP_PORT_TABLE_SIZE - 1)


/* Define the maximum number of multicast groups the system can support.  This might
   be further limited by the underlying physical hardware.  */

#ifndef NX_MAX_MULTICAST_GROUPS
#define NX_MAX_MULTICAST_GROUPS                    7
#endif

/* The following symbol defines the number of IPv6 addresses in the system.
   A typical system shall have 2 IPv6 addresses: one linklocal address and one global address.  */


/* Define the maximum number of internal server resources for TCP connections.  Server
   connections require a listen control structure.  */

#ifndef NX_MAX_LISTEN_REQUESTS
#define NX_MAX_LISTEN_REQUESTS                     10
#endif

/* Define the max length of username and password for HTTP Proxy authentication.  */

/* Define the max length of username.  */
#ifndef NX_HTTP_PROXY_MAX_USERNAME
#define NX_HTTP_PROXY_MAX_USERNAME                 20
#endif

/* Define the max length of password.  */
#ifndef NX_HTTP_PROXY_MAX_PASSWORD
#define NX_HTTP_PROXY_MAX_PASSWORD                 20
#endif

/* NX_HTTP_PROXY_MAX_AUTHENTICATION is the max length of base64 of "name:password", 
   1 bytes for an extra conversion if needed, 2 bytes for pad if needed, 1 byte for null terminator and four byte alignment. */
#define NX_HTTP_PROXY_MAX_AUTHENTICATION           (((((NX_HTTP_PROXY_MAX_USERNAME + NX_HTTP_PROXY_MAX_PASSWORD  + 1 ) * 4 / 3) + 1 + 2 + 1) / 4 + 1) * 4)


/* Define the IP status checking/return bits.  */

#define NX_IP_INITIALIZE_DONE                      ((ULONG)0x0001)
#define NX_IP_ADDRESS_RESOLVED                     ((ULONG)0x0002)
#define NX_IP_LINK_ENABLED                         ((ULONG)0x0004)
#define NX_IP_ARP_ENABLED                          ((ULONG)0x0008)
#define NX_IP_UDP_ENABLED                          ((ULONG)0x0010)
#define NX_IP_TCP_ENABLED                          ((ULONG)0x0020)
#define NX_IP_IGMP_ENABLED                         ((ULONG)0x0040)
#define NX_IP_RARP_COMPLETE                        ((ULONG)0x0080)
#define NX_IP_INTERFACE_LINK_ENABLED               ((ULONG)0x0100)

/* Define various states in the TCP connection state machine.  */

#define NX_TCP_CLOSED                              1  /* Connection is closed state   */
#define NX_TCP_LISTEN_STATE                        2  /* Server listen state          */
#define NX_TCP_SYN_SENT                            3  /* SYN sent state               */
#define NX_TCP_SYN_RECEIVED                        4  /* SYN received state           */
#define NX_TCP_ESTABLISHED                         5  /* Connection established state */
#define NX_TCP_CLOSE_WAIT                          6  /* Close Wait state             */
#define NX_TCP_FIN_WAIT_1                          7  /* Finished Wait 1 state        */
#define NX_TCP_FIN_WAIT_2                          8  /* Finished Wait 2 state        */
#define NX_TCP_CLOSING                             9  /* Closing state                */
#define NX_TCP_TIMED_WAIT                          10 /* Timed wait state             */
#define NX_TCP_LAST_ACK                            11 /* Last ACK state               */


/* API return values.  */

#define NX_SUCCESS                                 0x00
#define NX_NO_PACKET                               0x01
#define NX_UNDERFLOW                               0x02
#define NX_OVERFLOW                                0x03
#define NX_NO_MAPPING                              0x04
#define NX_DELETED                                 0x05
#define NX_POOL_ERROR                              0x06
#define NX_PTR_ERROR                               0x07
#define NX_WAIT_ERROR                              0x08
#define NX_SIZE_ERROR                              0x09
#define NX_OPTION_ERROR                            0x0a
#define NX_DELETE_ERROR                            0x10
#define NX_CALLER_ERROR                            0x11
#define NX_INVALID_PACKET                          0x12
#define NX_INVALID_SOCKET                          0x13
#define NX_NOT_ENABLED                             0x14
#define NX_ALREADY_ENABLED                         0x15
#define NX_ENTRY_NOT_FOUND                         0x16
#define NX_NO_MORE_ENTRIES                         0x17
#define NX_ARP_TIMER_ERROR                         0x18
#define NX_RESERVED_CODE0                          0x19
#define NX_WAIT_ABORTED                            0x1A
#define NX_IP_INTERNAL_ERROR                       0x20
#define NX_IP_ADDRESS_ERROR                        0x21
#define NX_ALREADY_BOUND                           0x22
#define NX_PORT_UNAVAILABLE                        0x23
#define NX_NOT_BOUND                               0x24
#define NX_RESERVED_CODE1                          0x25
#define NX_SOCKET_UNBOUND                          0x26
#define NX_NOT_CREATED                             0x27
#define NX_SOCKETS_BOUND                           0x28
#define NX_NO_RESPONSE                             0x29
#define NX_POOL_DELETED                            0x30
#define NX_ALREADY_RELEASED                        0x31
#define NX_RESERVED_CODE2                          0x32
#define NX_MAX_LISTEN                              0x33
#define NX_DUPLICATE_LISTEN                        0x34
#define NX_NOT_CLOSED                              0x35
#define NX_NOT_LISTEN_STATE                        0x36
#define NX_IN_PROGRESS                             0x37
#define NX_NOT_CONNECTED                           0x38
#define NX_WINDOW_OVERFLOW                         0x39
#define NX_ALREADY_SUSPENDED                       0x40
#define NX_DISCONNECT_FAILED                       0x41
#define NX_STILL_BOUND                             0x42
#define NX_NOT_SUCCESSFUL                          0x43
#define NX_UNHANDLED_COMMAND                       0x44
#define NX_NO_FREE_PORTS                           0x45
#define NX_INVALID_PORT                            0x46
#define NX_INVALID_RELISTEN                        0x47
#define NX_CONNECTION_PENDING                      0x48
#define NX_TX_QUEUE_DEPTH                          0x49
#define NX_NOT_IMPLEMENTED                         0x4A
#define NX_NOT_SUPPORTED                           0x4B
#define NX_INVALID_INTERFACE                       0x4C
#define NX_INVALID_PARAMETERS                      0x4D
#define NX_NOT_FOUND                               0x4E
#define NX_CANNOT_START                            0x4F
#define NX_NO_INTERFACE_ADDRESS                    0x50
#define NX_INVALID_MTU_DATA                        0x51
#define NX_DUPLICATED_ENTRY                        0x52
#define NX_PACKET_OFFSET_ERROR                     0x53
#define NX_OPTION_HEADER_ERROR                     0x54
#define NX_CONTINUE                                0x55
#define NX_TCPIP_OFFLOAD_ERROR                     0x56

/* Define Link Driver constants.  */

#define NX_LINK_PACKET_SEND                        0
#define NX_LINK_INITIALIZE                         1
#define NX_LINK_ENABLE                             2
#define NX_LINK_DISABLE                            3
#define NX_LINK_PACKET_BROADCAST                   4
#define NX_LINK_ARP_SEND                           5
#define NX_LINK_ARP_RESPONSE_SEND                  6
#define NX_LINK_RARP_SEND                          7
#define NX_LINK_MULTICAST_JOIN                     8
#define NX_LINK_MULTICAST_LEAVE                    9
#define NX_LINK_GET_STATUS                         10
#define NX_LINK_GET_SPEED                          11
#define NX_LINK_GET_DUPLEX_TYPE                    12
#define NX_LINK_GET_ERROR_COUNT                    13
#define NX_LINK_GET_RX_COUNT                       14
#define NX_LINK_GET_TX_COUNT                       15
#define NX_LINK_GET_ALLOC_ERRORS                   16
#define NX_LINK_UNINITIALIZE                       17
#define NX_LINK_DEFERRED_PROCESSING                18
#define NX_LINK_INTERFACE_ATTACH                   19
#define NX_LINK_SET_PHYSICAL_ADDRESS               20
#define NX_INTERFACE_CAPABILITY_GET                21
#define NX_INTERFACE_CAPABILITY_SET                22
#define NX_LINK_INTERFACE_DETACH                   23
#define NX_LINK_FACTORY_ADDRESS_GET                24
#define NX_LINK_RX_ENABLE                          25
#define NX_LINK_RX_DISABLE                         26
#define NX_LINK_6LOWPAN_COMMAND                    27 /* 6LowPAN driver command, the sub command see nx_6lowpan.h.  */
#define NX_LINK_GET_INTERFACE_TYPE                 28

#define NX_LINK_USER_COMMAND                       50 /* Values after this value are reserved for application.  */


/* Define operations for TCP/IP offload callback functions.  */
#define NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT  0
#define NX_TCPIP_OFFLOAD_TCP_SERVER_SOCKET_LISTEN   1
#define NX_TCPIP_OFFLOAD_TCP_SERVER_SOCKET_ACCEPT   2
#define NX_TCPIP_OFFLOAD_TCP_SERVER_SOCKET_UNLISTEN 3
#define NX_TCPIP_OFFLOAD_TCP_SOCKET_DISCONNECT      4
#define NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND            5
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_BIND            6
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_UNBIND          7
#define NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND            8


/* Define Link Driver Capability Flags.  */
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
#define NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM   0x00000001
#define NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM   0x00000002
#define NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM    0x00000004
#define NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM    0x00000008
#define NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM    0x00000010
#define NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM    0x00000020
#define NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM 0x00000040
#define NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM 0x00000080
#define NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM 0x00000100
#define NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM 0x00000200
#define NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM   0x00000400
#define NX_INTERFACE_CAPABILITY_IGMP_RX_CHECKSUM   0x00000800
#define NX_INTERFACE_CAPABILITY_PTP_TIMESTAMP      0x00001000
#define NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD      0x00002000
#define NX_INTERFACE_CAPABILITY_CHECKSUM_ALL       (NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM | \
                                                    NX_INTERFACE_CAPABILITY_IGMP_RX_CHECKSUM)     
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#define NX_IP_VERSION_V4                           0x4
#define NX_IP_VERSION_V6                           0x6

#if defined(FEATURE_NX_IPV6) && defined(NX_IPV6_STATELESS_AUTOCONFIG_CONTROL)
#define NX_STATELESS_ADDRESS_AUTOCONFIG_ENABLED    0
#define NX_STATELESS_ADDRESS_AUTOCONFIG_DISABLED   1
#endif /* defined(FEATURE_NX_IPV6) && defined(NX_IPV6_STATELESS_AUTOCONFIG_CONTROL) */

/* Define the macro for building IPv4 addresses.  */
#define IP_ADDRESS(a, b, c, d)                     ((((ULONG)a) << 24) | (((ULONG)b) << 16) | (((ULONG)c) << 8) | ((ULONG)d))

/* Define the direction of IP packet. */
#ifdef NX_ENABLE_IP_PACKET_FILTER
#define NX_IP_PACKET_IN                            0
#define NX_IP_PACKET_OUT                           1
#endif /* NX_ENABLE_IP_PACKET_FILTER */

/* Define the interface type.  */
#define NX_INTERFACE_TYPE_UNKNOWN                  0
#define NX_INTERFACE_TYPE_OTHER                    1
#define NX_INTERFACE_TYPE_ETHERNET                 2
#define NX_INTERFACE_TYPE_WIFI                     3
#define NX_INTERFACE_TYPE_CELLULAR                 4
#define NX_INTERFACE_TYPE_BLUETOOTH                5
#define NX_INTERFACE_TYPE_LORAWAN                  6
#define NX_INTERFACE_TYPE_MAX                      7

#ifdef NX_ENABLE_THREAD
/* Define the packet type for Thread MLE.  */
#define NX_PACKET_TYPE_THREAD_MLE                  0x01
#endif /* NX_ENABLE_THREAD  */

/* Define IPv4/v6 Address structure */
typedef struct NXD_ADDRESS_STRUCT
{
    /* Flag indicating IP address format.  Valid values are:
       NX_IP_VERSION_V4 and NX_IP_VERSION_V6.
     */
    ULONG nxd_ip_version;

    /* Union that holds either IPv4 or IPv6 address. */
    union
    {

#ifndef NX_DISABLE_IPV4
        ULONG v4;
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
        ULONG v6[4];
#endif /* FEATURE_NX_IPV6 */
    } nxd_ip_address;
} NXD_ADDRESS;


#ifdef NX_IPSEC_ENABLE
/* Define the selector for IPSEC or TUNNEL. */
typedef struct NX_ADDRESS_SELECTOR_STRUCT
{
    /* Define selector source, destination address. */
    NXD_ADDRESS nx_selector_src_address_start;
    NXD_ADDRESS nx_selector_src_address_end;
    NXD_ADDRESS nx_selector_dst_address_start;
    NXD_ADDRESS nx_selector_dst_address_end;
} NX_ADDRESS_SELECTOR;

#endif /* NX_IPSEC_ENABLE */


/* Define the control block definitions for all system objects.  */


/* Define the basic memory management packet structure.  This structure is
   used to hold application data as well as internal control data.  */
struct NX_PACKET_POOL_STRUCT;

#ifdef NX_ENABLE_PACKET_DEBUG_INFO
/* Define macro to record packet debug information.  */
#define NX_PACKET_DEBUG(f,l,p)  {\
        (p) -> nx_packet_debug_file=f;\
        (p) -> nx_packet_debug_line=l;\
        if (tx_thread_identify())                            \
            (p) -> nx_packet_debug_thread = tx_thread_identify() -> tx_thread_name;                            \
        else                            \
            (p) -> nx_packet_debug_thread = "ISR";                            \
                                }
#else
#define NX_PACKET_DEBUG(f, l, p)
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */

typedef  struct NX_PACKET_STRUCT
{

    /* Define the pool this packet is associated with.  */
    struct NX_PACKET_POOL_STRUCT
                *nx_packet_pool_owner;

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Define the link to the chain (one or more) of packet extensions.  If this is NULL, there
       are no packet extensions for this packet.  */
    struct NX_PACKET_STRUCT
                *nx_packet_next;
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Define the pointer to the first byte written closest to the beginning of the
       buffer.  This is used to prepend information in front of the packet.  */
    UCHAR       *nx_packet_prepend_ptr;

    /* Define the pointer to the byte after the last character written in the buffer.  */
    UCHAR       *nx_packet_append_ptr;

    /* Define the packet data area start and end pointer.  These will be used to
       mark the physical boundaries of the packet.  */
    UCHAR       *nx_packet_data_start;
    UCHAR       *nx_packet_data_end;

    /* The above data are required by all packets. */
    /* The following data are required by header packet when optimized packet is not disabled. */

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Define the link to the last packet (if any) in the chain.  This is used to append
       information to the end without having to traverse the entire chain.  */
    struct NX_PACKET_STRUCT
                *nx_packet_last;
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Define the link that will be used to queue the packet.  */
    struct NX_PACKET_STRUCT
                *nx_packet_queue_next;

    /* Union that holds either tcp_queue_next or fragment_next. */
    union
    {

        /* Define the link that will be used to keep outgoing TCP packets queued
           so they can be ACKed or re-sent.  */
        struct NX_PACKET_STRUCT
                *nx_packet_tcp_queue_next;

#ifndef NX_DISABLE_FRAGMENTATION
        /* Define the link to the next fragment.  This is only used in IP fragmentation
           re-assembly.  */
        struct NX_PACKET_STRUCT
                *nx_packet_fragment_next;
#endif /* NX_DISABLE_FRAGMENTATION */
    } nx_packet_union_next;

    /* Define the total packet length.  */
    ULONG       nx_packet_length;

#ifndef NX_DISABLE_FRAGMENTATION
    ULONG       nx_packet_reassembly_time;  /* Time stamp for measuring the number of seconds a
                                               packet is in the reassebmly logic.  Once a time
                                               out value is reached packets of the same FRAG ID
                                               are released. */
#endif /* NX_DISABLE_FRAGMENTATION */

#ifdef FEATURE_NX_IPV6
    UCHAR       nx_packet_option_state;  /* Used by IPv6, indicating the current option
                                            being processed. */
    UCHAR       nx_packet_destination_header;
    USHORT      nx_packet_option_offset;
#endif /* FEATURE_NX_IPV6 */


    /*
       Tag the packet type. Valid values are either
       NX_IP_VERSION_V4 or NX_IP_VERSION_V6.
     */
    UCHAR       nx_packet_ip_version;

    /* RFC1122, Section3.2.1.5, Page32-33. RFC1122, Section4.2.2.15, Page90-91.  */
    /* nx_packet_identical_copy is to track the IP ID field in the IP header
       for the retransmitted TCP packet. The IP ID value should not change
       if the TCP header for a retransmitted packet is not changed.

       On retransmission, if the flag nx_packet_identical_copy is set, the same IP ID value is used for the retransmitted packet.
       Otherwise the value in the IP ID field is updated with the latest ID value in the IP instance.
     */
    UCHAR       nx_packet_identical_copy;

    /* Length of IP header including options. It is set for outgoing packet only. */
    UCHAR       nx_packet_ip_header_length;

    /*lint -esym(768,NX_PACKET_STRUCT::nx_packet_reserved) suppress member not referenced. It is reserved for future use. */
    UCHAR       nx_packet_reserved;

    /* Union that holds either IPv4 interface or IPv6 address. */
    union
    {

        /* Define the interface from which the packet was received, or the interface to transmit to. */
        struct NX_INTERFACE_STRUCT
                *nx_packet_interface_ptr;

        /* Point to the interface IPv6 address structure.  On transmit or receive path. */
        struct NXD_IPV6_ADDRESS_STRUCT
                *nx_packet_ipv6_address_ptr;
    } nx_packet_address;

#define nx_packet_ip_interface nx_packet_address.nx_packet_interface_ptr

    /* Points to the beginning of IPv4/6 header.  This field is used for */
    /* quick refernece to the IP header, in case there are optional headers. */
    UCHAR       *nx_packet_ip_header;

#ifdef NX_ENABLE_THREAD
    /* Define the packet type for MLE.  */
    UCHAR       nx_packet_type;

    /* Define the received signal strength.  */
    UCHAR       nx_packet_signal_strength;

    /* It is reserved for future use.  */
    UCHAR       nx_packet_thread_reserved[2];
#endif /* NX_ENABLE_THREAD  */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    /* Define the capability flag of hardware to be used by the packet. */
    ULONG       nx_packet_interface_capability_flag;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
    VOID        *nx_packet_ipsec_sa_ptr;

    /* Used in HW engine non-block mode. */
    USHORT      nx_packet_ipsec_op;

    /* Indicate the current protocol being processed. */
    USHORT      nx_packet_ipsec_state;
#endif /* NX_IPSEC_ENABLE */

#ifdef NX_ENABLE_PACKET_DEBUG_INFO
    /* Indicate the current thread that owns the packet. */
    CHAR       *nx_packet_debug_thread;

    /* Indicate the current file that is processing the packet. */
    CHAR       *nx_packet_debug_file;

    /* Indicate the current function that is processing the packet. */
    ULONG       nx_packet_debug_line;
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */

#ifdef NX_PACKET_HEADER_PAD

    /* Define a pad word for 16-byte alignment, if necessary.  */
    ULONG       nx_packet_packet_pad[NX_PACKET_HEADER_PAD_SIZE];
#endif
} NX_PACKET;


/* Define the Packet Pool control block that will be used to manage each individual
   packet pool.  */

typedef struct NX_PACKET_POOL_STRUCT
{

    /* Define the block pool ID used for error checking.  */
    ULONG       nx_packet_pool_id;

    /* Define the packet pool's name.  */
    CHAR       *nx_packet_pool_name;

    /* Define the number of available memory packets in the pool.  */
    ULONG       nx_packet_pool_available;

    /* Save the initial number of blocks.  */
    ULONG       nx_packet_pool_total;

    /* Define statistics and error counters for this packet pool.  */
    ULONG       nx_packet_pool_empty_requests;
    ULONG       nx_packet_pool_empty_suspensions;
    ULONG       nx_packet_pool_invalid_releases;

    /* Define the head pointer of the available packet pool.  */
    struct NX_PACKET_STRUCT    *nx_packet_pool_available_list;

    /* Save the start address of the packet pool's memory area.  */
    CHAR       *nx_packet_pool_start;

    /* Save the packet pool's size in bytes.  */
    ULONG       nx_packet_pool_size;

    /* Save the individual packet payload size - rounded for alignment.  */
    ULONG       nx_packet_pool_payload_size;

    /* Define the packet pool suspension list head along with a count of
       how many threads are suspended.  */
    TX_THREAD  *nx_packet_pool_suspension_list;
    ULONG       nx_packet_pool_suspended_count;

    /* Define the created list next and previous pointers.  */
    struct NX_PACKET_POOL_STRUCT
               *nx_packet_pool_created_next,
               *nx_packet_pool_created_previous;

#ifdef NX_ENABLE_LOW_WATERMARK
    /* Low watermark. */
    UINT        nx_packet_pool_low_watermark;
#endif /* NX_ENABLE_LOW_WATERMARK */
} NX_PACKET_POOL;


#ifndef NX_DISABLE_IPV4
/* Define the Address Resolution Protocol (ARP) structure that makes up the
   route table in each IP instance.  This is how IP addresses are translated
   to physical addresses in the system.  */

typedef struct NX_ARP_STRUCT
{

    /* Define a flag that indicates whether or not the mapping in this ARP
       entry is static.  */
    UINT nx_arp_route_static;

    /* Define the counter that indicates when the next ARP update request is
       sent.  This is always zero for static entries and initialized to the maximum
       value for new entries.  */
    UINT nx_arp_entry_next_update;

    /* Define the ARP retry counter that is incremented each time the ARP request
       is sent.  */
    UINT nx_arp_retries;

    /* Define the links for the IP ARP dynamic structures in the system.  This list
       is maintained in a most recently used fashion.  */
    struct NX_ARP_STRUCT
        *nx_arp_pool_next,
        *nx_arp_pool_previous;

    /* Define the links for the active ARP entry that is part of route
       information inside of an IP instance.  */
    struct NX_ARP_STRUCT
         *nx_arp_active_next,
         *nx_arp_active_previous,
        **nx_arp_active_list_head;

    /* Define the IP address that this entry is setup for.  */
    ULONG nx_arp_ip_address;

    /* Define the physical address that maps to this IP address.  */
    ULONG nx_arp_physical_address_msw;
    ULONG nx_arp_physical_address_lsw;

    /* Define the physical interface attached to this IP address. */
    struct NX_INTERFACE_STRUCT *nx_arp_ip_interface;

    /* Define a pointer to the queue holding one or more packets while address
       resolution is pending. The maximum number of packets that can be queued
       is defined by NX_APR_MAX_QUEUE_DEPTH. If ARP packet queue is exceeded,
       the oldest packet is discarded in favor of keeping the newer packet.  */
    struct NX_PACKET_STRUCT
        *nx_arp_packets_waiting;
} NX_ARP;
#endif /* NX_DISABLE_IPV4 */


/* Determine if the UDP control block has an extension defined. If not, 
   define the extension to whitespace.  */

#ifndef NX_UDP_SOCKET_MODULE_EXTENSION
#define NX_UDP_SOCKET_MODULE_EXTENSION
#endif


/* Define the basic UDP socket structure.  This structure is used to manage all information
   necessary to manage UDP transmission and reception.  */

typedef struct NX_UDP_SOCKET_STRUCT
{

    /* Define the UDP identification that is used to determine if the UDP socket has
       been created.  */
    ULONG       nx_udp_socket_id;

    /* Define the Application defined name for this UDP socket instance.  */
    CHAR        *nx_udp_socket_name;

    /* Define the UDP port that was bound to.  */
    UINT        nx_udp_socket_port;

    /* Define the entry that this UDP socket belongs to.  */
    struct NX_IP_STRUCT
                *nx_udp_socket_ip_ptr;

    /* Define the statistic and error counters for this UDP socket.  */
    ULONG       nx_udp_socket_packets_sent;
    ULONG       nx_udp_socket_bytes_sent;
    ULONG       nx_udp_socket_packets_received;
    ULONG       nx_udp_socket_bytes_received;
    ULONG       nx_udp_socket_invalid_packets;
    ULONG       nx_udp_socket_packets_dropped;
    ULONG       nx_udp_socket_checksum_errors;

    /* Define the type of service for this UDP instance.  */
    ULONG       nx_udp_socket_type_of_service;

    /* Define the time-to-live for this UDP instance.  */
    UINT        nx_udp_socket_time_to_live;

    /* Define the fragment enable bit for this UDP instance.  */
    ULONG       nx_udp_socket_fragment_enable;

    /* Define the UDP checksum disable flag for this UDP socket.  */
    UINT        nx_udp_socket_disable_checksum;

    /* Define the UDP receive packet queue pointers, queue counter, and
       the maximum queue depth.  */
    ULONG       nx_udp_socket_receive_count;
    ULONG       nx_udp_socket_queue_maximum;
    NX_PACKET   *nx_udp_socket_receive_head,
                *nx_udp_socket_receive_tail;

    /* Define the UDP socket bound list.  These pointers are used to manage the list
       of UDP sockets on a particular hashed port index.  */
    struct NX_UDP_SOCKET_STRUCT
                *nx_udp_socket_bound_next,
                *nx_udp_socket_bound_previous;

    /* Define the UDP socket bind suspension thread pointer.  This pointer points
       to the thread that that is suspended attempting to bind to a port that is
       already bound to another socket.  */
    TX_THREAD   *nx_udp_socket_bind_in_progress;

    /* Define the UDP receive suspension list head associated with a count of
       how many threads are suspended attempting to receive from the same UDP port.  */
    TX_THREAD   *nx_udp_socket_receive_suspension_list;
    ULONG       nx_udp_socket_receive_suspended_count;

    /* Define the UDP bind suspension list head associated with a count of
       how many threads are suspended attempting to bind to the same UDP port.  The
       currently bound socket will maintain the head pointer.  When a socket unbinds,
       the head of the suspension list is given the port and the remaining entries
       of the suspension list are transferred to its suspension list head pointer.  */
    TX_THREAD   *nx_udp_socket_bind_suspension_list;
    ULONG       nx_udp_socket_bind_suspended_count;

    /* Define the link between other UDP structures created by the application.  This
       is linked to the IP instance the socket was created on.  */
    struct NX_UDP_SOCKET_STRUCT
                *nx_udp_socket_created_next,
                *nx_udp_socket_created_previous;

    /* Define the callback function for receive packet notification. If specified
       by the application, this function is called whenever a receive packet is
       available on for the socket.  */
    VOID (*nx_udp_receive_callback)(struct NX_UDP_SOCKET_STRUCT *socket_ptr);

    /* This pointer is reserved for application specific use.  */
    /*lint -esym(768,NX_UDP_SOCKET_STRUCT::nx_udp_socket_reserved_ptr) suppress member not referenced. It is reserved for future use. */
    void        *nx_udp_socket_reserved_ptr;

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    /* Store a pointer to TCP/IP offload context.  */
    VOID        *nx_udp_socket_tcpip_offload_context;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Define the port extension in the UDP socket control block. This 
       is typically defined to whitespace in nx_port.h.  */
    NX_UDP_SOCKET_MODULE_EXTENSION
    
} NX_UDP_SOCKET;


/* Determine if the TCP control block has an extension defined. If not, 
   define the extension to whitespace.  */

#ifndef NX_TCP_SOCKET_MODULE_EXTENSION
#define NX_TCP_SOCKET_MODULE_EXTENSION
#endif


/* Define the basic TCP socket structure.  This structure is used to manage all information
   necessary to manage TCP transmission and reception.  */

typedef struct NX_TCP_SOCKET_STRUCT
{

    /* Define the TCP identification that is used to determine if the TCP socket has
       been created.  */
    ULONG       nx_tcp_socket_id;

    /* Define the Application defined name for this TCP socket instance.  */
    CHAR        *nx_tcp_socket_name;

    /* Define the socket type flag.  If true, this socket is a client socket.  */
    UINT        nx_tcp_socket_client_type;

    /* Define the TCP port that was bound to.  */
    UINT        nx_tcp_socket_port;

    /* Define the TCP socket's maximum segment size (mss). By default, this is setup to the
       IP's MTU less the size of the IP and TCP headers.  */
    ULONG       nx_tcp_socket_mss;

    /* Define the connected IP and port information.  */
    NXD_ADDRESS nx_tcp_socket_connect_ip;
    UINT        nx_tcp_socket_connect_port;
    ULONG       nx_tcp_socket_connect_mss;
    ULONG       nx_tcp_socket_peer_mss;
    struct NX_INTERFACE_STRUCT
                *nx_tcp_socket_connect_interface;
    ULONG       nx_tcp_socket_next_hop_address;

    /* mss2 is the holding place for the smss * smss value.
       It is computed and stored here once for later use. */
    ULONG       nx_tcp_socket_connect_mss2;

    ULONG       nx_tcp_socket_tx_slow_start_threshold;

    /* Define the state of the TCP connection.  */
    UINT        nx_tcp_socket_state;

    /* Define the receive and transmit sequence numbers.   */
    ULONG       nx_tcp_socket_tx_sequence;
    ULONG       nx_tcp_socket_rx_sequence;
    ULONG       nx_tcp_socket_rx_sequence_acked;
    ULONG       nx_tcp_socket_delayed_ack_timeout;
    ULONG       nx_tcp_socket_fin_sequence;
    USHORT      nx_tcp_socket_fin_received;
    USHORT      nx_tcp_socket_fin_acked;

    /* Track the advertised window size */
    ULONG       nx_tcp_socket_tx_window_advertised;
    ULONG       nx_tcp_socket_tx_window_congestion;
    ULONG       nx_tcp_socket_tx_outstanding_bytes; /* Data transmitted but not acked. */

    /* Define the transmit sequence that enters fast transmit. */
    ULONG       nx_tcp_socket_tx_sequence_recover;

    /* Define the previous cumulative acknowledgment.  */
    ULONG       nx_tcp_socket_previous_highest_ack;

    /* Counter for "ack-N-packet" */
    ULONG       nx_tcp_socket_ack_n_packet_counter;

    /* Counter for duplicated ACK  */
    UINT        nx_tcp_socket_duplicated_ack_received;

    /* Define the window size fields of the TCP socket structure.  */
    ULONG       nx_tcp_socket_rx_window_default;
    ULONG       nx_tcp_socket_rx_window_current;
    ULONG       nx_tcp_socket_rx_window_last_sent;

    /* Define the statistic and error counters for this TCP socket.  */
    ULONG       nx_tcp_socket_packets_sent;
    ULONG       nx_tcp_socket_bytes_sent;
    ULONG       nx_tcp_socket_packets_received;
    ULONG       nx_tcp_socket_bytes_received;
    ULONG       nx_tcp_socket_retransmit_packets;
    ULONG       nx_tcp_socket_checksum_errors;

    /* Define data for zero window probe. */
    ULONG       nx_tcp_socket_zero_window_probe_failure;
    ULONG       nx_tcp_socket_zero_window_probe_sequence;
    UCHAR       nx_tcp_socket_zero_window_probe_has_data;
    UCHAR       nx_tcp_socket_zero_window_probe_data;

    /* Define whether or not TCP socket is in fast recovery procedure. */
    UCHAR       nx_tcp_socket_fast_recovery;

#ifdef NX_ENABLE_HTTP_PROXY

    /* Define the state of HTTP Proxy connection.  */
    UCHAR       nx_tcp_socket_http_proxy_state;

    /* Define the packet to store HTTP response header.  */
    NX_PACKET   *nx_tcp_socket_http_proxy_header_packet;

    /* Define the IP and port for original server.  */
    NXD_ADDRESS nx_tcp_socket_original_server_ip;
    UINT        nx_tcp_socket_original_server_port;
#else
    /* Reserved to four bytes alignment. */
    /*lint -esym(768,NX_TCP_SOCKET_STRUCT::nx_tcp_socket_reserved) suppress member not referenced. It is reserved for future use. */
    UCHAR       nx_tcp_socket_reserved;
#endif /* NX_ENABLE_HTTP_PROXY  */

    /* Define the entry that this TCP socket belongs to.  */
    struct NX_IP_STRUCT
                *nx_tcp_socket_ip_ptr;

    /* Define the type of service for this TCP instance.  */
    ULONG       nx_tcp_socket_type_of_service;

    /* Define the time-to-live for this TCP instance.  */
    UINT        nx_tcp_socket_time_to_live;

    /* Define the fragment enable bit for this TCP instance.  */
    ULONG       nx_tcp_socket_fragment_enable;

    /* Define the TCP receive packet queue pointers, queue counter, and
       the maximum queue depth.  */
    ULONG       nx_tcp_socket_receive_queue_count;
    NX_PACKET   *nx_tcp_socket_receive_queue_head,
                *nx_tcp_socket_receive_queue_tail;

    /* Define the TCP packet sent queue. This queue is used to keep track of the
       transmit packets already send.  Before they can be released we need to receive
       an ACK back from the other end of the connection.  If no ACK is received, the
       packet(s) need to be re-transmitted.  */
    ULONG       nx_tcp_socket_transmit_queue_maximum;
    ULONG       nx_tcp_socket_transmit_sent_count;
    NX_PACKET   *nx_tcp_socket_transmit_sent_head,
                *nx_tcp_socket_transmit_sent_tail;

    /* Define the maximum TCP packet receive queue. */
#ifdef NX_ENABLE_LOW_WATERMARK
    ULONG   nx_tcp_socket_receive_queue_maximum;
#endif /* NX_ENABLE_LOW_WATERMARK */

    /* Define the TCP transmit timeout parameters.  If the socket timeout is non-zero,
       there is an active timeout on the TCP socket.  Subsequent timeouts are derived
       from the timeout rate, which is adjusted higher as timeouts occur.  */
    ULONG       nx_tcp_socket_timeout;
    ULONG       nx_tcp_socket_timeout_rate;
    ULONG       nx_tcp_socket_timeout_retries;
    ULONG       nx_tcp_socket_timeout_max_retries;
    ULONG       nx_tcp_socket_timeout_shift;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
    /* Local receive window size, when user creates the TCP socket. */
    ULONG       nx_tcp_socket_rx_window_maximum;

    /* Window scale this side needs to offer to the peer. */
    ULONG       nx_tcp_rcv_win_scale_value;

    /* Window scale offered by the peer.  0xFF indicates the peer does not support window scaling. */
    ULONG       nx_tcp_snd_win_scale_value;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

    /* Define the TCP keepalive timer parameters.  If enabled with NX_ENABLE_TCP_KEEPALIVE,
       these parameters are used to implement the keepalive timer.  */
#ifdef NX_ENABLE_TCP_KEEPALIVE
    ULONG       nx_tcp_socket_keepalive_timeout;
    ULONG       nx_tcp_socket_keepalive_retries;
#endif /* NX_ENABLE_TCP_KEEPALIVE */

    /* Define the TCP socket bound list.  These pointers are used to manage the list
       of TCP sockets on a particular hashed port index.  */
    struct NX_TCP_SOCKET_STRUCT
                *nx_tcp_socket_bound_next,
                *nx_tcp_socket_bound_previous;

    /* Define the TCP socket bind suspension thread pointer.  This pointer points
       to the thread that that is suspended attempting to bind to a port that is
       already bound to another socket.  */
    TX_THREAD   *nx_tcp_socket_bind_in_progress;

    /* Define the TCP receive suspension list head associated with a count of
       how many threads are suspended attempting to receive from the same TCP port.  */
    TX_THREAD   *nx_tcp_socket_receive_suspension_list;
    ULONG       nx_tcp_socket_receive_suspended_count;

    /* Define the TCP transmit suspension list head associated with a count of
       how many threads are suspended attempting to transmit from the same TCP port.  */
    TX_THREAD   *nx_tcp_socket_transmit_suspension_list;
    ULONG       nx_tcp_socket_transmit_suspended_count;

    /* Define the TCP connect suspension pointer that contains the pointer to the
       thread suspended attempting to establish a TCP connection.  */
    TX_THREAD   *nx_tcp_socket_connect_suspended_thread;

    /* Define the TCP disconnect suspension pointer that contains the pointer to the
       thread suspended attempting to break a TCP connection.  */
    TX_THREAD   *nx_tcp_socket_disconnect_suspended_thread;

    /* Define the TCP bind suspension list head associated with a count of
       how many threads are suspended attempting to bind to the same TCP port.  The
       currently bound socket will maintain the head pointer.  When a socket unbinds,
       the head of the suspension list is given the port and the remaining entries
       of the suspension list are transferred to its suspension list head pointer.  */
    TX_THREAD   *nx_tcp_socket_bind_suspension_list;
    ULONG       nx_tcp_socket_bind_suspended_count;

    /* Define the link between other TCP structures created by the application.  This
       is linked to the IP instance the socket was created on.  */
    struct NX_TCP_SOCKET_STRUCT
            *nx_tcp_socket_created_next,
            *nx_tcp_socket_created_previous;

    /* Define the callback function for urgent data reception.  This is for future use.  */
    VOID (*nx_tcp_urgent_data_callback)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
    /* Define the callback function for notifying an incoming SYN request. */
    UINT (*nx_tcp_socket_syn_received_notify)(struct NX_TCP_SOCKET_STRUCT *socket_ptr, NX_PACKET *packet_ptr);

    /* Define the callback function for notifying the host application of a connection handshake completion
       with a remote host.  */
    VOID (*nx_tcp_establish_notify)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

    /* Define the callback function for notifying the host application of disconnection completion
       with a remote host.  */
    VOID (*nx_tcp_disconnect_complete_notify)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

    /* Define the callback function for notifying the host application to set the socket
       state in the timed wait state.  */
    VOID (*nx_tcp_timed_wait_callback)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);
#endif

    /* Define the callback function for disconnect detection from the other side of
       the connection.  */
    VOID (*nx_tcp_disconnect_callback)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

    /* Define the callback function for receive packet notification. If specified
       by the application, this function is called whenever a receive packet is
       available on for the socket.  */
    VOID (*nx_tcp_receive_callback)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

    /* Define the callback function for receive packet notification. If specified
       by the application, this function is called whenever a receive packet is
       available on for the socket.  */
    VOID (*nx_tcp_socket_window_update_notify)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);

#ifdef   NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY
    VOID (*nx_tcp_socket_queue_depth_notify)(struct NX_TCP_SOCKET_STRUCT *socket_ptr);
#endif

    /* This pointer is reserved for application specific use.  */
    /*lint -esym(768,NX_TCP_SOCKET_STRUCT::nx_tcp_socket_reserved_ptr) suppress member not referenced. It is reserved for application specific use. */
    void    *nx_tcp_socket_reserved_ptr;

    /* Define the default maximum queue size. This is necessary to dynamically
       change the maximum queue size dynamically.  */
    ULONG   nx_tcp_socket_transmit_queue_maximum_default;

    /* Define a flag for enabling the keepalive feature per TCP socket. */
#ifdef NX_ENABLE_TCP_KEEPALIVE
    UINT    nx_tcp_socket_keepalive_enabled;
#endif /* NX_ENABLE_TCP_KEEPALIVE */

#ifdef FEATURE_NX_IPV6
    struct NXD_IPV6_ADDRESS_STRUCT *nx_tcp_socket_ipv6_addr;
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
    /* Stores a pointer to the SA, if the traffic needs to be protected. */
    VOID *nx_tcp_socket_egress_sa;

    /* Number of bytes to offset from IP header.  This offset is needed to accommondate security header. */
    UINT  nx_tcp_socket_egress_sa_data_offset;

#endif /* NX_IPSEC_ENABLE */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    /* Store a pointer to TCP/IP offload context.  */
    VOID *nx_tcp_socket_tcpip_offload_context;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Define the port extension in the TCP socket control block. This 
       is typically defined to whitespace in nx_port.h.  */
    NX_TCP_SOCKET_MODULE_EXTENSION
    
} NX_TCP_SOCKET;


/* Define the basic TCP listen request structure.  This structure is used to indicate
   which, if any, TCP ports are allowing a client connection.  */

typedef struct NX_TCP_LISTEN_STRUCT
{

    /* Define the port number that we are allowing a connection on.  */
    UINT        nx_tcp_listen_port;

    /* Define the listen callback routine that will be called when a connection
       request is received.  */
    VOID        (*nx_tcp_listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port);

    /* Define the previously created socket for this listen request.  */
    NX_TCP_SOCKET
                *nx_tcp_listen_socket_ptr;

    /* Define the listen queue for connect requests that come in when the previous socket
       given for a listen or relisten has been used.  */
    ULONG       nx_tcp_listen_queue_maximum;
    ULONG       nx_tcp_listen_queue_current;
    NX_PACKET   *nx_tcp_listen_queue_head,
                *nx_tcp_listen_queue_tail;

    /* Define the link between other TCP listen structures created by the application.  */
    struct NX_TCP_LISTEN_STRUCT
                *nx_tcp_listen_next,
                *nx_tcp_listen_previous;
} NX_TCP_LISTEN;

/* There should be at least one physical interface. */
#ifndef NX_MAX_PHYSICAL_INTERFACES
#define NX_MAX_PHYSICAL_INTERFACES    1
#endif /* NX_MAX_PHYSICAL_INTERFACES */

#ifndef NX_MAX_IPV6_ADDRESSES
#define NX_MAX_IPV6_ADDRESSES         (NX_MAX_PHYSICAL_INTERFACES * 3)
#endif /* NX_MAX_IPV6_ADDRESSES */

#if (NX_MAX_IPV6_ADDRESSES < 2)
#error "NetX Duo needs at least 2 IPv6 interface addresses. "
#endif

#ifndef NX_DISABLE_LOOPBACK_INTERFACE
/* Inside interface array, entries 0 up to NX_MAX_PHYSICAL_INTERFACES are assigned to the
   physical interfaces.  Entry NX_MAX_PHYSICAL_INTERFACES is assigned to the loopback interface. */
#define NX_LOOPBACK_INTERFACE         NX_MAX_PHYSICAL_INTERFACES
#define NX_LOOPBACK_IPV6_SOURCE_INDEX NX_MAX_IPV6_ADDRESSES
#define NX_LOOPBACK_IPV6_ENABLED      1
#else
#define NX_LOOPBACK_INTERFACE         0
#define NX_LOOPBACK_IPV6_ENABLED      0
#endif /* NX_DISALBE_LOOPBACK_INTERFACE */

#if (defined(NX_DISABLE_LOOPBACK_INTERFACE) && (NX_MAX_PHYSICAL_INTERFACES == 0))
#error "NetX is built without either physical interfaces or loopback interfaces."
#endif

#if defined(NX_DISABLE_LOOPBACK_INTERFACE)
#define NX_MAX_IP_INTERFACES          NX_MAX_PHYSICAL_INTERFACES
#else
#define NX_MAX_IP_INTERFACES          (NX_MAX_PHYSICAL_INTERFACES + 1)
#endif /* NX_DISABLE_LOOPBACK_INTERFACE */

#ifdef NX_DISABLE_PACKET_CHAIN
#ifndef NX_DISABLE_FRAGMENTATION
#error "IP fragmentation is not supported if packet chain is disabled."
#endif /* NX_DISABLE_FRAGMENTATION */
#endif /* NX_DISABLE_PACKET_CHAIN */

struct NX_IP_DRIVER_STRUCT;

typedef struct NXD_IPV6_ADDRESS_STRUCT
{
    UCHAR nxd_ipv6_address_valid;

    UCHAR nxd_ipv6_address_type;  /* IPv6 = 6 */

    /* Current state:Invalid,Tentative...*/
    UCHAR nxd_ipv6_address_state;

    /* Prefix length */
    UCHAR nxd_ipv6_address_prefix_length;

    /* Pointer to the physical interface where this address is attached to.*/
    struct NX_INTERFACE_STRUCT *nxd_ipv6_address_attached;

    /* Interface address,in host byte order */
    ULONG nxd_ipv6_address[4];

    /* Pointer to the next address, or NX_NULL */
    struct NXD_IPV6_ADDRESS_STRUCT *nxd_ipv6_address_next;

    /* Number of DAD packets to be transmitted. */
    /* This entry is used only if DAD is enabled. */
    UCHAR nxd_ipv6_address_DupAddrDetectTransmit;

    /* Describe how the interface is configured. */
    UCHAR nxd_ipv6_address_ConfigurationMethod;

    /* Define the index in array. */
    UCHAR nxd_ipv6_address_index;

    /* Reserved. */
    /*lint -esym(768,NXD_IPV6_ADDRESS_STRUCT::reserved) suppress member not referenced. It is reserved for future use. */
    UCHAR reserved;
} NXD_IPV6_ADDRESS;

/* Define the address interface structure. */
typedef struct NX_INTERFACE_STRUCT
{

    /* Flag indicating whether or not the interface entry is valid. */
    CHAR *nx_interface_name;
    UCHAR nx_interface_valid;
    UCHAR nx_interface_address_mapping_needed;

    /* Define the Link Up field that is manipulated by the associated link driver.  */
    UCHAR nx_interface_link_up;

    /* Define the index in array. */
    UCHAR nx_interface_index;

    /* Define a flag to check if link status change. */
    UCHAR nx_interface_link_status_change;

    /*lint -esym(768,NX_INTERFACE_STRUCT::nx_interface_reserved) suppress member not referenced. It is reserved for future use. */
    UCHAR nx_interface_reserved[3];

    /* Define the physical address of this IP instance.  These field are
       setup by the link driver during initialization.  */
    ULONG nx_interface_physical_address_msw;
    ULONG nx_interface_physical_address_lsw;

#ifndef NX_DISABLE_IPV4
    /* Define the IP address of this IP instance.  Loopback can be done by
       either using the same address or by using 127.*.*.*.  */
    ULONG nx_interface_ip_address;

    /* Define the network portion of the IP address.  */
    ULONG nx_interface_ip_network_mask;

    /* Define the network only bits of the IP address.  */
    ULONG nx_interface_ip_network;
#endif /* !NX_DISABLE_IPV4  */

    /* Define the head of the interface IPv6 address list.
       Typically the 1st entry on the list is the link local address,
       followed by one or more global IPv6 addresses.
     */
    struct NXD_IPV6_ADDRESS_STRUCT *nxd_interface_ipv6_address_list_head;

    /* Define information setup by the Link Driver.  */
    ULONG nx_interface_ip_mtu_size;

#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION

    /* Maximum number of router solicitation messages to send. */
    ULONG nx_ipv6_rtr_solicitation_max;

    /* Number of router solicitation messages to be sent. */
    ULONG nx_ipv6_rtr_solicitation_count;

    /* Time (in seconds) before sending out another Router Solicitation message. */
    ULONG nx_ipv6_rtr_solicitation_interval;

    /* Count down timer for sending out  router solicitation message. */
    ULONG nx_ipv6_rtr_solicitation_timer;
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */

#ifdef NX_IPV6_STATELESS_AUTOCONFIG_CONTROL
    ULONG nx_ipv6_stateless_address_autoconfig_status;
#endif /* NX_IPV6_STATELESS_AUTOCONFIG_CONTROL    */
    /* Define a pointer for use by the applicaiton.  Typically this is going to be
       used by the link drvier. */
    VOID *nx_interface_additional_link_info;

    /* Define the Link Driver entry point.  */
    VOID        (*nx_interface_link_driver_entry)(struct NX_IP_DRIVER_STRUCT *);

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    /* Define the capability flag of hardware for the interface. */
    ULONG nx_interface_capability_flag;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifndef NX_DISABLE_IPV4
    /* Define the ARP defend timeout.  */
    ULONG nx_interface_arp_defend_timeout;

    /* Define the IP probe address.  */
    ULONG nx_interface_ip_probe_address;

    /* Define the IP conflict notify handler. A non-null value for this function
       pointer results in NetX calling it when an IP address is found in an incoming
       ARP packet that matches that of nx_interface_ip_probe_address.  */
    VOID        (*nx_interface_ip_conflict_notify_handler)(struct NX_IP_STRUCT *, UINT, ULONG, ULONG, ULONG);
#endif /* !NX_DISABLE_IPV4  */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    /* Define the TCP/IP offload handler. */
    UINT        (*nx_interface_tcpip_offload_handler)(struct NX_IP_STRUCT *ip_ptr,
                                                      struct NX_INTERFACE_STRUCT *interface_ptr,
                                                      VOID *socket_ptr, UINT operation, NX_PACKET *packet_ptr,
                                                      NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip,
                                                      UINT local_port, UINT *remote_port, UINT wait_option);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
} NX_INTERFACE;

/* Define the static IPv4 routing table entry structure. */
#if defined(NX_ENABLE_IP_STATIC_ROUTING) && !defined(NX_DISABLE_IPV4)
typedef struct NX_IP_ROUTING_ENTRY_STRUCT
{
    /* Destination IP address, in host byte order */
    ULONG nx_ip_routing_dest_ip;

    /* Net mask, in host byte order */
    ULONG nx_ip_routing_net_mask;

    /* Next hop address, in host byte order.  */
    ULONG nx_ip_routing_next_hop_address;

    struct NX_INTERFACE_STRUCT
        *nx_ip_routing_entry_ip_interface;
} NX_IP_ROUTING_ENTRY;
#endif /* defined(NX_ENABLE_IP_STATIC_ROUTING) && !defined(NX_DISABLE_IPV4) */

#ifndef NX_DISABLE_IPV4
typedef struct NX_IPV4_MULTICAST_STRUCT
{

    /* Define the multicast registered group list.  */
    ULONG nx_ipv4_multicast_join_list;

    /* Define the multicast regstiered group interface list. */
    NX_INTERFACE *nx_ipv4_multicast_join_interface_list;

    /* Define the multicast registration count.  */
    ULONG nx_ipv4_multicast_join_count;

    /* Define the multicast random time list.  */
    ULONG nx_ipv4_multicast_update_time;

    /* Define the multicast loopback flag list. This flag is set based on the global
       loopback enable at the time the group was joined. */
    UINT nx_ipv4_multicast_loopback_enable;
} NX_IPV4_MULTICAST_ENTRY;
#endif /* NX_DISABLE_IPV4 */

#ifdef NX_ENABLE_IPV6_MULTICAST
typedef struct NX_IPV6_MULTICAST_STRUCT
{

    /* Define the MLD registered group list.  */
    ULONG nx_ip_mld_join_list[4];

    /* Define the MLD regstiered group interface list. */
    NX_INTERFACE *nx_ip_mld_join_interface_list;

    /* Define the MLD registration count.  */
    ULONG nx_ip_mld_join_count;
} NX_IPV6_MULTICAST_ENTRY;

#endif /* NX_ENABLE_IPV6_MULTICAST  */


/* Determine if the IP control block has an extension defined. If not, 
   define the extension to whitespace.  */

#ifndef NX_IP_MODULE_EXTENSION
#define NX_IP_MODULE_EXTENSION
#endif


/* Define the Internet Protocol (IP) structure.  Any number of IP instances
   may be used by the application.  */

typedef struct NX_IP_STRUCT
{

    /* Define the IP identification that is used to determine if the IP has
       been created.  */
    ULONG       nx_ip_id;

    /* Define the Application defined name for this IP instance.  */
    CHAR        *nx_ip_name;

    /* Define the IP address of this IP instance.  Loopback can be done by
       either using the same address or by using 127.*.*.*.  */
#if 1
#define nx_ip_address                  nx_ip_interface[0].nx_interface_ip_address
#define nx_ip_driver_mtu               nx_ip_interface[0].nx_interface_ip_mtu_size
#define nx_ip_driver_mapping_needed    nx_ip_interface[0].nx_interface_address_mapping_needed
#define nx_ip_network_mask             nx_ip_interface[0].nx_interface_ip_network_mask
#define nx_ip_network                  nx_ip_interface[0].nx_interface_ip_network
#define nx_ip_arp_physical_address_msw nx_ip_interface[0].nx_interface_physical_address_msw
#define nx_ip_arp_physical_address_lsw nx_ip_interface[0].nx_interface_physical_address_lsw
#define nx_ip_driver_link_up           nx_ip_interface[0].nx_interface_link_up
#define nx_ip_link_driver_entry        nx_ip_interface[0].nx_interface_link_driver_entry
#define nx_ip_additional_link_info     nx_ip_interface[0].nx_interface_additional_link_info
#endif

#ifndef NX_DISABLE_IPV4
    /* Define the gateway IP address.  */
    ULONG       nx_ip_gateway_address;   /* In host byte order */

    struct NX_INTERFACE_STRUCT
               *nx_ip_gateway_interface;
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* Define IPv6 addresses.  NetX6 allows user to specify Global address
       in addition to the link-local address generated automatically. */
    /* IPv6 address takes 16 bytes.  These addresses are 4-byte aligned */
    struct NXD_IPV6_ADDRESS_STRUCT nx_ipv6_address[NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED];

    /* Define the destination table. */
    NX_IPV6_DESTINATION_ENTRY
                nx_ipv6_destination_table[NX_IPV6_DESTINATION_TABLE_SIZE];

    /* Define the ND cache table. */
    ND_CACHE_ENTRY
                nx_ipv6_nd_cache[NX_IPV6_NEIGHBOR_CACHE_SIZE];

    /* Define the destination table size. */
    UINT        nx_ipv6_destination_table_size;
#endif /* FEATURE_NX_IPV6 */

    /* Define the statistic and error counters for this IP instance.   */
    ULONG       nx_ip_total_packet_send_requests;
    ULONG       nx_ip_total_packets_sent;
    ULONG       nx_ip_total_bytes_sent;
    ULONG       nx_ip_total_packets_received;
    ULONG       nx_ip_total_packets_delivered;
    ULONG       nx_ip_total_bytes_received;
    ULONG       nx_ip_packets_forwarded;
    ULONG       nx_ip_packets_reassembled;
    ULONG       nx_ip_reassembly_failures;
    ULONG       nx_ip_invalid_packets;
    ULONG       nx_ip_invalid_transmit_packets;
    ULONG       nx_ip_invalid_receive_address;
    ULONG       nx_ip_unknown_protocols_received;

    /*lint -esym(768,NX_IP_STRUCT::nx_ip_transmit_no_route_errors) suppress member not referenced. It is reserved for application specific use. */
    ULONG       nx_ip_transmit_resource_errors;
    ULONG       nx_ip_transmit_no_route_errors;
    ULONG       nx_ip_receive_packets_dropped;
    ULONG       nx_ip_receive_checksum_errors;
    ULONG       nx_ip_send_packets_dropped;
    ULONG       nx_ip_total_fragment_requests;
    ULONG       nx_ip_successful_fragment_requests;
    ULONG       nx_ip_fragment_failures;
    ULONG       nx_ip_total_fragments_sent;
    ULONG       nx_ip_total_fragments_received;
    ULONG       nx_ip_arp_requests_sent;
    ULONG       nx_ip_arp_requests_received;
    ULONG       nx_ip_arp_responses_sent;
    ULONG       nx_ip_arp_responses_received;
    ULONG       nx_ip_arp_aged_entries;
    ULONG       nx_ip_arp_invalid_messages;
    ULONG       nx_ip_arp_static_entries;
    ULONG       nx_ip_udp_packets_sent;
    ULONG       nx_ip_udp_bytes_sent;
    ULONG       nx_ip_udp_packets_received;
    ULONG       nx_ip_udp_bytes_received;
    ULONG       nx_ip_udp_invalid_packets;
    ULONG       nx_ip_udp_no_port_for_delivery;
    ULONG       nx_ip_udp_receive_packets_dropped;
    ULONG       nx_ip_udp_checksum_errors;
    ULONG       nx_ip_tcp_packets_sent;
    ULONG       nx_ip_tcp_bytes_sent;
    ULONG       nx_ip_tcp_packets_received;
    ULONG       nx_ip_tcp_bytes_received;
    ULONG       nx_ip_tcp_invalid_packets;
    ULONG       nx_ip_tcp_receive_packets_dropped;
    ULONG       nx_ip_tcp_checksum_errors;
    ULONG       nx_ip_tcp_connections;
    ULONG       nx_ip_tcp_passive_connections;
    ULONG       nx_ip_tcp_active_connections;
    ULONG       nx_ip_tcp_disconnections;
    ULONG       nx_ip_tcp_connections_dropped;
    ULONG       nx_ip_tcp_retransmit_packets;
    ULONG       nx_ip_tcp_resets_received;
    ULONG       nx_ip_tcp_resets_sent;
    ULONG       nx_ip_icmp_total_messages_received;
    ULONG       nx_ip_icmp_checksum_errors;
    ULONG       nx_ip_icmp_invalid_packets;
    ULONG       nx_ip_icmp_unhandled_messages;
    ULONG       nx_ip_pings_sent;
    ULONG       nx_ip_ping_timeouts;
    ULONG       nx_ip_ping_threads_suspended;
    ULONG       nx_ip_ping_responses_received;
    ULONG       nx_ip_pings_received;
    ULONG       nx_ip_pings_responded_to;
    ULONG       nx_ip_igmp_invalid_packets;
    ULONG       nx_ip_igmp_reports_sent;
    ULONG       nx_ip_igmp_queries_received;
    ULONG       nx_ip_igmp_checksum_errors;
    ULONG       nx_ip_igmp_groups_joined;
#ifndef NX_DISABLE_IGMPV2
    ULONG       nx_ip_igmp_router_version;
#endif
    ULONG       nx_ip_rarp_requests_sent;
    ULONG       nx_ip_rarp_responses_received;
    ULONG       nx_ip_rarp_invalid_messages;


    /* Define the IP forwarding flag.  This is by default set to NX_NULL.
       If forwarding is desired, the nx_ip_forward_packet_process service
       pointed to by this member should be called.  */
#ifndef NX_DISABLE_IPV4
    VOID        (*nx_ip_forward_packet_process)(struct NX_IP_STRUCT *, NX_PACKET *);

#ifdef NX_NAT_ENABLE
    /*  Define the NAT forwarded packet handler. This is by default set to NX_NULL.  */
    UINT        (*nx_ip_nat_packet_process)(struct NX_IP_STRUCT *, NX_PACKET *, UINT packet_process);

    /*  Define the NAT port verify handler. This is by default set to NX_NULL.  */
    UINT        (*nx_ip_nat_port_verify)(struct NX_IP_STRUCT *, UINT protocol, UINT port);
#endif
#endif /* !NX_DISABLE_IPV4  */

    /* Define the packet ID.  */
    ULONG       nx_ip_packet_id;

    /* Define the default packet pool.  */
    struct NX_PACKET_POOL_STRUCT
                *nx_ip_default_packet_pool;

#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Define the auxiliary packet pool for internal usage. */
    struct NX_PACKET_POOL_STRUCT
                *nx_ip_auxiliary_packet_pool;
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

    /* Define the internal mutex used for protection inside the NetX
       data structures.  */
    TX_MUTEX    nx_ip_protection;

    /* Define the initialize done flag.  */
    UINT        nx_ip_initialize_done;

#ifdef NX_DRIVER_DEFERRED_PROCESSING
    /* Define the Link Driver hardware deferred packet queue.  */
    NX_PACKET   *nx_ip_driver_deferred_packet_head,
                *nx_ip_driver_deferred_packet_tail;

    /* Define the Link Driver hardware deferred packet processing routine.  If the driver
       deferred processing is enabled, this routine is called from the IP helper thread.  */
    VOID        (*nx_ip_driver_deferred_packet_handler)(struct NX_IP_STRUCT *, NX_PACKET *);
#endif /* NX_DRIVER_DEFERRED_PROCESSING */

    /* Define the deferred packet processing queue.  This is used to
       process packets not initially processed in the receive ISR.  */
    NX_PACKET   *nx_ip_deferred_received_packet_head,
                *nx_ip_deferred_received_packet_tail;

    /* Define the raw IP function pointer that also indicates whether or
       not raw IP packet sending and receiving is enabled.  */
    UINT        (*nx_ip_raw_ip_processing)(struct NX_IP_STRUCT *, ULONG, NX_PACKET *);

#ifdef NX_ENABLE_IP_RAW_PACKET_FILTER
    /* Define the raw packet filter function pointer. */
    UINT        (*nx_ip_raw_packet_filter)(struct NX_IP_STRUCT *, ULONG, NX_PACKET *);
#endif /* NX_ENABLE_IP_RAW_PACKET_FILTER */

    /* Define the pointer to the raw IP packet queue.  */
    NX_PACKET   *nx_ip_raw_received_packet_head,
                *nx_ip_raw_received_packet_tail;

    /* Define the count of raw IP packets on the queue.  */
    ULONG       nx_ip_raw_received_packet_count;

    /* Define the maximum number of packets queued for receive. */
    ULONG       nx_ip_raw_received_packet_max;

    /* Define the raw packet suspension list head along with a count of
       how many threads are suspended.  */
    TX_THREAD   *nx_ip_raw_packet_suspension_list;
    ULONG       nx_ip_raw_packet_suspended_count;

    /* Define the IP helper thread that processes periodic ARP requests,
       reassembles IP messages, and helps handle TCP/IP packets.  */
    TX_THREAD   nx_ip_thread;

    /* Define the IP event flags that are used to stimulate the IP helper
       thread.  */
    TX_EVENT_FLAGS_GROUP
                nx_ip_events;

    /* Define the IP periodic timer for this IP instance.  */
    TX_TIMER    nx_ip_periodic_timer;

    /* Define the IP fragment function pointer that also indicates whether or
       IP fragmenting is enabled.  */
    VOID        (*nx_ip_fragment_processing)(struct NX_IP_DRIVER_STRUCT *);

    /* Define the IP unfragment function pointer.  */
    VOID        (*nx_ip_fragment_assembly)(struct NX_IP_STRUCT *);

    /* Define the IP unfragment timeout checking function pointer.  */
    VOID        (*nx_ip_fragment_timeout_check)(struct NX_IP_STRUCT *);

    /* Define the fragment pointer to the oldest fragment re-assembly.  If this is
       the same between any periodic the fragment re-assembly is too old and
       will be deleted.  */
    NX_PACKET   *nx_ip_timeout_fragment;

    /* Define the pointer to the fragmented IP packet queue.  This queue is
       appended when a fragmented packet is received and is drained inside
       the IP.  */
    NX_PACKET   *nx_ip_received_fragment_head,
                *nx_ip_received_fragment_tail;

    /* Define the pointer to the fragment re-assembly queue.  */
    NX_PACKET   *nx_ip_fragment_assembly_head,
                *nx_ip_fragment_assembly_tail;

#ifdef NX_ENABLE_6LOWPAN
    /* Define the created 6LoWPAN list. */
    VOID        *nx_ip_6lowpan_created_ptr;

#ifdef NX_ENABLE_THREAD
    /* Define the created Thread list. */
    VOID        *nx_ip_thread_created_ptr;
#endif /* NX_ENABLE_THREAD  */
#endif /* NX_ENABLE_6LOWPAN  */

#ifndef NX_DISABLE_IPV4
    /* Define the IP address change notification callback routine pointer.  */
    VOID        (*nx_ip_address_change_notify)(struct NX_IP_STRUCT *, VOID *);
    VOID        *nx_ip_address_change_notify_additional_info;

    /* Define the internal IP address change notification callback routine pointer, used in mDNS.  */
    VOID        (*nx_ip_address_change_notify_internal)(struct NX_IP_STRUCT *, VOID *);
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    VOID        (*nx_ipv6_address_change_notify)(struct NX_IP_STRUCT *ip_ptr, UINT status, UINT interface_index, UINT addres_index, ULONG *ip_address);

    /* Define the internal IPv6 address change notification callback routine pointer, used in mDNS.  */
    VOID        (*nx_ipv6_address_change_notify_internal)(struct NX_IP_STRUCT *ip_ptr, UINT status, UINT interface_index, UINT addres_index, ULONG *ip_address);
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
#endif /* FEATURE_NX_IPV6 */

#ifndef NX_DISABLE_IPV4
    /* Define the multicast entry. */
    NX_IPV4_MULTICAST_ENTRY nx_ipv4_multicast_entry[NX_MAX_MULTICAST_GROUPS];

    /* Define global IGMP loopback enable/disable flag. By default, IGMP loopback is
       disabled.  */
    UINT        nx_ip_igmp_global_loopback_enable;

    /* Define the IGMP receive packet processing routine.  This is setup when IGMP
       is enabled.  */
    void        (*nx_ip_igmp_packet_receive)(struct NX_IP_STRUCT *, struct NX_PACKET_STRUCT *);

    /* Define the IGMP periodic processing routine.  This is also setup when IGMP
       is enabled.  */
    void        (*nx_ip_igmp_periodic_processing)(struct NX_IP_STRUCT *);

    /* Define the IGMP packet queue processing routine.  This is setup when IGMP is
       enabled.  */
    void        (*nx_ip_igmp_queue_process)(struct NX_IP_STRUCT *);

    /* Define the IGMP message queue.  */
    NX_PACKET   *nx_ip_igmp_queue_head;
#endif /* !NX_DISABLE_IPV4  */

    /* Define the ICMP sequence number.  This is used in ICMP messages that
       require a sequence number.  */
    ULONG       nx_ip_icmp_sequence;

#ifdef NX_ENABLE_IPV6_MULTICAST

    /* Define the IPv6 Multicast Group structure.  */
    NX_IPV6_MULTICAST_ENTRY nx_ipv6_multicast_entry[NX_MAX_MULTICAST_GROUPS];

    /* Define the MLD join count.  */
    ULONG       nx_ipv6_multicast_groups_joined;

#endif /* NX_ENABLE_IPV6_MULTICAST  */

    /* Define the ICMP packet receive routine.  This also doubles as a
       mechanism to make sure ICMP is enabled.  If this function is NULL, ICMP
       is not enabled.  */
    void        (*nx_ip_icmp_packet_receive)(struct NX_IP_STRUCT *, struct NX_PACKET_STRUCT *);

#ifndef NX_DISABLE_IPV4
    /* Define the ICMP packet queue processing routine.  This is setup when ICMP is
       enabled.  */
    void        (*nx_ip_icmp_queue_process)(struct NX_IP_STRUCT *);

    /* Define the ICMP packet process routine */
    void        (*nx_ip_icmpv4_packet_process)(struct NX_IP_STRUCT *, NX_PACKET *);
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* Define the ICMPv6 packet process routine. */
    void        (*nx_ip_icmpv6_packet_process)(struct NX_IP_STRUCT *, NX_PACKET *);

    /* Define the Neighbor Discovery cache fast (10ms) periodic update routine. */
    void        (*nx_nd_cache_fast_periodic_update)(struct NX_IP_STRUCT *);

    /* Define the Neighbor Discovery cache slow (1000 ms) periodic update routine. */
    void        (*nx_nd_cache_slow_periodic_update)(struct NX_IP_STRUCT *);

    /* Define the ICMPv6 router advertisement flag callback. */
    void        (*nx_icmpv6_ra_flag_callback)(struct NX_IP_STRUCT *, UINT);

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
    /* Define the MTU path discovery periodic update. */
    void        (*nx_destination_table_periodic_update)(struct NX_IP_STRUCT *);
#endif

#endif /* FEATURE_NX_IPV6 */


    /* Define the ICMP message queue.  */
    NX_PACKET   *nx_ip_icmp_queue_head;

    /* Define the ICMP ping suspension list head associated with a count of
       how many threads are suspended attempting to ping.  */
    TX_THREAD   *nx_ip_icmp_ping_suspension_list;
    ULONG        nx_ip_icmp_ping_suspended_count;

    /* Define the UDP port information structure associated with this IP instance.  */
    struct NX_UDP_SOCKET_STRUCT
                *nx_ip_udp_port_table[NX_UDP_PORT_TABLE_SIZE];

    /* Define the head pointer of the created UDP socket list.  */
    struct NX_UDP_SOCKET_STRUCT
                *nx_ip_udp_created_sockets_ptr;

    /* Define the number of created UDP socket instances.  */
    ULONG       nx_ip_udp_created_sockets_count;

    /* Define the UDP packet receive routine.  This also doubles as a
       mechanism to make sure UDP is enabled.  If this function is NULL, UDP
       is not enabled.  */
    void        (*nx_ip_udp_packet_receive)(struct NX_IP_STRUCT *, struct NX_PACKET_STRUCT *);

    /* Define the TCP port information structure associated with this IP instance.  */
    struct NX_TCP_SOCKET_STRUCT
                *nx_ip_tcp_port_table[NX_TCP_PORT_TABLE_SIZE];

    /* Define the head pointer of the created TCP socket list.  */
    struct NX_TCP_SOCKET_STRUCT
                *nx_ip_tcp_created_sockets_ptr;

    /* Define the number of created TCP socket instances.  */
    ULONG       nx_ip_tcp_created_sockets_count;

    /* Define the TCP packet receive routine.  This also doubles as a
       mechanism to make sure TCP is enabled.  If this function is NULL, TCP
       is not enabled.  */
    void        (*nx_ip_tcp_packet_receive)(struct NX_IP_STRUCT *, struct NX_PACKET_STRUCT *);

    /* Define the TCP periodic processing routine for transmit timeout logic.  */
    void        (*nx_ip_tcp_periodic_processing)(struct NX_IP_STRUCT *);

    /* Define the TCP fast periodic processing routine for transmit timeout logic.  */
    void        (*nx_ip_tcp_fast_periodic_processing)(struct NX_IP_STRUCT *);

    /* Define the TCP packet queue processing routine.  This is setup when TCP is
       enabled.  */
    void        (*nx_ip_tcp_queue_process)(struct NX_IP_STRUCT *);

    /* Define the pointer to the incoming TCP packet queue.  */
    NX_PACKET   *nx_ip_tcp_queue_head,
                *nx_ip_tcp_queue_tail;

    /* Define the count of incoming TCP packets on the queue.  */
    ULONG       nx_ip_tcp_received_packet_count;

    /* Define the TCP listen request structure that contains the maximum number of
       listen requests allowed for this IP instance.  */
    NX_TCP_LISTEN
                nx_ip_tcp_server_listen_reqs[NX_MAX_LISTEN_REQUESTS];

    /* Define the head pointer of the available listen request structures.  */
    NX_TCP_LISTEN
                *nx_ip_tcp_available_listen_requests;

    /* Define the head pointer of the active listen requests.  These are made
       by issuing the nx_tcp_server_socket_listen service.  */
    NX_TCP_LISTEN
                *nx_ip_tcp_active_listen_requests;

#ifdef NX_ENABLE_HTTP_PROXY
    /* Define the IP address of HTTP proxy server.  */
    NXD_ADDRESS nx_ip_http_proxy_ip_address;

    /* Define the port of HTTP proxy server.  */
    USHORT      nx_ip_http_proxy_port;

    /* Define the flag indicating the HTTP proxy is enabled.  */
    USHORT      nx_ip_http_proxy_enable;

    /* Define the buffer for HTTP proxy authentication.  */
    UCHAR       nx_ip_http_proxy_authentication[NX_HTTP_PROXY_MAX_AUTHENTICATION];
    UINT        nx_ip_http_proxy_authentication_length;
#endif /* NX_ENABLE_HTTP_PROXY */

    /* Define a flag indicating the IP fast timer has been created */
    UINT        nx_ip_fast_periodic_timer_created;

    /* Define the fast IP periodic timer used for high resolution events for
       this IP instance.  */
    TX_TIMER    nx_ip_fast_periodic_timer;

#ifndef NX_DISABLE_IPV4
    /* Define the destination routing information associated with this IP
       instance.  */
    struct NX_ARP_STRUCT
                *nx_ip_arp_table[NX_ARP_TABLE_SIZE];

    /* Define the head pointer of the static ARP list.  */
    struct NX_ARP_STRUCT
                *nx_ip_arp_static_list;

    /* Define the head pointer of the dynamic ARP list.  */
    struct NX_ARP_STRUCT
                *nx_ip_arp_dynamic_list;

    /* Define the number of dynamic entries that are active.  */
    ULONG       nx_ip_arp_dynamic_active_count;

    /* Define the ARP deferred packet processing queue.  This is used to
       process ARP packets not initially processed in the receive ISR.  */
    NX_PACKET   *nx_ip_arp_deferred_received_packet_head,
                *nx_ip_arp_deferred_received_packet_tail;

    /* Define the ARP entry allocate routine.  This also doubles as a
       mechanism to make sure ARP is enabled.  If this function is NULL, ARP
       is not enabled.  */
    UINT        (*nx_ip_arp_allocate)(struct NX_IP_STRUCT *, struct NX_ARP_STRUCT **, UINT);

    /* Define the ARP periodic processing routine.  This is setup when ARP is
       enabled.  */
    void        (*nx_ip_arp_periodic_update)(struct NX_IP_STRUCT *);

    /* Define the ARP receive queue processing routine.  This is setup when ARP is
       enabled.  */
    void        (*nx_ip_arp_queue_process)(struct NX_IP_STRUCT *);

    /* Define the ARP send packet routine.  This is setup when ARP is
       enabled.  */
    void        (*nx_ip_arp_packet_send)(struct NX_IP_STRUCT *, ULONG destination_ip, NX_INTERFACE *nx_interface);

    /* Define the ARP gratuitous response handler. This routine is setup in the
       nx_arp_gratuitous_send function.  */
    void        (*nx_ip_arp_gratuitous_response_handler)(struct NX_IP_STRUCT *, NX_PACKET *);

#ifdef NX_ENABLE_ARP_MAC_CHANGE_NOTIFICATION
    /* Define the ARP collision notify handler. A non-null value for this function
       pointer results in NetX calling it whenever an IP address is found in an incoming
       ARP packet that matches that of the IP address in our ARP table.  */
    void        (*nx_ip_arp_collision_notify_response_handler)(void *);
#endif

    /* Define the ARP cache memory area.  This memory is supplied
       by the ARP enable function and is carved up by that function into as
       many ARP entries that will fit.  */
    struct NX_ARP_STRUCT
                *nx_ip_arp_cache_memory;

    /* Define the number of ARP entries that will fit in the ARP cache.  */
    ULONG       nx_ip_arp_total_entries;

    /* Define the RARP periodic processing routine.  This is setup when RARP is
       enabled.  It is also used to indicate RARP is enabled.  */
    void        (*nx_ip_rarp_periodic_update)(struct NX_IP_STRUCT *);

    /* Define the RARP receive queue processing routine.  This is setup when RARP is
       enabled.  */
    void        (*nx_ip_rarp_queue_process)(struct NX_IP_STRUCT *);

    /* Define the RARP deferred packet processing queue.  This is used to
       process RARP packets not initially processed in the receive ISR.  */
    NX_PACKET   *nx_ip_rarp_deferred_received_packet_head,
                *nx_ip_rarp_deferred_received_packet_tail;
#endif /* !NX_DISABLE_IPV4  */

    /* Define the link between other IP structures created by the application.  */
    struct NX_IP_STRUCT
                *nx_ip_created_next,
                *nx_ip_created_previous;

    /* This pointer is reserved for application specific use.  */
    /*lint -esym(768,NX_IP_STRUCT::nx_ip_reserved_ptr) suppress member not referenced. */
    void        *nx_ip_reserved_ptr;

    /* Define the TCP devered cleanup processing routine.  */
    void        (*nx_tcp_deferred_cleanup_check)(struct NX_IP_STRUCT *);

    /* Define the interfaces attached to this IP instance. */
    NX_INTERFACE nx_ip_interface[NX_MAX_IP_INTERFACES];

#ifndef NX_DISABLE_IPV4
    /* Define the IPv4 packet receive processing routine */
    void        (*nx_ipv4_packet_receive)(struct NX_IP_STRUCT *, NX_PACKET *);

    /* Define the static routing table, if the feature is enabled. */
#ifdef NX_ENABLE_IP_STATIC_ROUTING

    /* IPv4 static routing table. */
    NX_IP_ROUTING_ENTRY
                nx_ip_routing_table[NX_IP_ROUTING_TABLE_SIZE];

    /* Number of entries in the IPv4 static routing table. */
    ULONG       nx_ip_routing_table_entry_count;

#endif /* NX_ENABLE_IP_STATIC_ROUTING */
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6

    /* Number of valid entries in the IPv6 default router table. */
    USHORT  nx_ipv6_default_router_table_size;

    /* IPv6 default router table. */
    NX_IPV6_DEFAULT_ROUTER_ENTRY nx_ipv6_default_router_table[NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE];

    /* Create an index for round robin router selection. */
    UINT  nx_ipv6_default_router_table_round_robin_index;

    /*
     * IPv6 prefix table. Entries in the prefix table are put into two lists:
     *   the prefix list, and the free list. The prefix list keeps track of a list
     *   of valid prefix entries.  The free list points to the unused entries.
     */
    NX_IPV6_PREFIX_ENTRY nx_ipv6_prefix_list_table[NX_IPV6_PREFIX_LIST_TABLE_SIZE];

    /* Point to the 1st element of the prefix list. */
    NX_IPV6_PREFIX_ENTRY *nx_ipv6_prefix_list_ptr;

    /* Pointer to the 1st element of the prefix free list entry. */
    NX_IPV6_PREFIX_ENTRY *nx_ipv6_prefix_entry_free_list;

    /* Define the IPv6 packet receive processing routine */
    void        (*nx_ipv6_packet_receive)(struct NX_IP_STRUCT *, NX_PACKET *);

    /* Variable tracks the Neighbor Solicitation retransmission timer. */
    ULONG       nx_ipv6_retrans_timer_ticks;

    /* Variable tracks the Neighbor reachable timer. */
    ULONG       nx_ipv6_reachable_timer;

    /* Variable tracks the current hop_limit.  Hop limit is assigned by
       routers through the router advertisement message. */
    ULONG       nx_ipv6_hop_limit;

#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
    /* Define the IPsec Authentication Header process function, for the received packet. */
    UINT        (*nx_ip_ipsec_authentication_header_receive)(struct NX_IP_STRUCT *, NX_PACKET *, ULONG *, NX_PACKET **);

    /* Define the IPsec Authentication Header process function, for the transmit packet. */
    UINT        (*nx_ip_ipsec_authentication_header_transmit)(struct NX_IP_STRUCT *, NX_PACKET **, UINT, UINT);

    /* Define the IPsec Encapsulating Security Payload header process function, for the received packet. */
    UINT        (*nx_ip_ipsec_encapsulating_security_payload_receive)(struct NX_IP_STRUCT *, NX_PACKET *, ULONG *, NX_PACKET **);

    /* Define the IPsec Encapsulating Security Payload header insert function, for the transmit packet. */
    UINT        (*nx_ip_ipsec_encapsulating_security_payload_transmit)(struct NX_IP_STRUCT *, NX_PACKET **, UINT);

    /* Define the IPsec egress SA lookup routine. This is called by TCP/UDP/ICMP/RAW packet send. */
    UINT        (*nx_ip_packet_egress_sa_lookup)(struct NX_IP_STRUCT *ip_ptr,  NXD_ADDRESS *src_address,
                                              NXD_ADDRESS *dst_address, UCHAR protocol, ULONG src_port, ULONG dest_port,
                                              ULONG *data_offset, VOID **sa_ptr, UINT option);

    /* Define the head pointer of the ingress SA list. */
    VOID        *nx_ip_ipsec_ingress_sa_ptr;

    /* Define the head pointer of the egress SA list. */
    VOID        *nx_ip_ipsec_egress_sa_ptr;

    /* Define the reference to IKEv2. */
    VOID        *nx_ip_ipsec_ikev2_ptr;

    /* Define the packet queue header in which packet is processed by hardware engine. */
    NX_PACKET   *nx_ip_hw_done_packet_header_ptr;

    /* Define the packet queue tail in which packet is processed by hardware engine. */
    NX_PACKET   *nx_ip_hw_done_packet_tail_ptr;

#endif /* NX_IPSEC_ENABLE */

    /* Define the link status change notify routine. */
    VOID        (*nx_ip_link_status_change_callback)(struct NX_IP_STRUCT *, UINT, UINT);

#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Define the IP packet filter routine.  */
    UINT        (*nx_ip_packet_filter)(VOID *, UINT);

    /* Define the IP packet filter extended routine.  */
    /* Note: Developers are encouraged to use nx_ip_packet_filter_extended since nx_ip_packet_filter will be deprecated.*/
    UINT        (*nx_ip_packet_filter_extended)(struct NX_IP_STRUCT *ip_ptr, NX_PACKET *packet_ptr, UINT direction);
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    /* Define the port extension in the IP control block. This 
       is typically defined to whitespace in nx_port.h.  */
    NX_IP_MODULE_EXTENSION
    
} NX_IP;


/* Define the Driver interface structure that is typically allocated off of the
   local stack and passed to the IP Link Driver.  */

typedef struct NX_IP_DRIVER_STRUCT
{

    /* Define the driver command.  */
    UINT        nx_ip_driver_command;

    /* Define the driver return status.  */
    UINT        nx_ip_driver_status;

    /* Define the physical address that maps to the destination IP address.  */
    ULONG       nx_ip_driver_physical_address_msw;
    ULONG       nx_ip_driver_physical_address_lsw;

#ifdef NX_ENABLE_6LOWPAN
    /* Define the length of physical address. Used by 6LoWPAN driver.  */
    USHORT      nx_ip_driver_physical_address_length;

    /* Define the 6LoWPAN sub driver command, see nx_6lowpan.h. Used by 6LoWPAN driver.  */
    USHORT      nx_ip_driver_sub_command;
#endif /* NX_ENABLE_6LOWPAN */

    /* Define the datagram packet (if any) for the driver to send.  */
    NX_PACKET   *nx_ip_driver_packet;

    /* Define the return pointer for raw driver command requests.  */
    ULONG       *nx_ip_driver_return_ptr;

    /* Define the IP pointer associated with the request.  */
    struct NX_IP_STRUCT
                *nx_ip_driver_ptr;

    NX_INTERFACE *nx_ip_driver_interface;
} NX_IP_DRIVER;


/* Define the system API mappings based on the error checking
   selected by the user.  Note: this section is only applicable to
   application source code, hence the conditional that turns off this
   stuff when the include file is processed by the ThreadX source. */

#ifndef NX_SOURCE_CODE

/* Map old APIs to new APIs. */
#define nx_ip_raw_packet_interface_send                 nx_ip_raw_packet_source_send
#define nx_udp_socket_interface_send                    nx_udp_socket_source_send
#define nxd_icmp_interface_ping                         nxd_icmp_source_ping
#define nxd_ip_raw_packet_interface_send                nxd_ip_raw_packet_source_send
#define nxd_ipv6_global_address_set(i, a, p)            nxd_ipv6_address_set(i, 0, a, p, NX_NULL)
#define nxd_ipv6_linklocal_address_set(i, a)            nxd_ipv6_address_set(i, 0, a, 10, NX_NULL)
#define nxd_udp_socket_interface_send                   nxd_udp_socket_source_send


/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING
/* Services without error checking.  */
/* APIs for ARP. */
#define nx_arp_dynamic_entries_invalidate               _nx_arp_dynamic_entries_invalidate
#define nx_arp_dynamic_entry_set                        _nx_arp_dynamic_entry_set
#define nx_arp_enable                                   _nx_arp_enable
#define nx_arp_entry_delete                             _nx_arp_entry_delete
#define nx_arp_gratuitous_send                          _nx_arp_gratuitous_send
#define nx_arp_hardware_address_find                    _nx_arp_hardware_address_find
#define nx_arp_info_get                                 _nx_arp_info_get
#define nx_arp_ip_address_find                          _nx_arp_ip_address_find
#define nx_arp_static_entries_delete                    _nx_arp_static_entries_delete
#define nx_arp_static_entry_create                      _nx_arp_static_entry_create
#define nx_arp_static_entry_delete                      _nx_arp_static_entry_delete

/* APIs for ICMP. */
#define nx_icmp_enable                                  _nx_icmp_enable
#define nx_icmp_info_get                                _nx_icmp_info_get
#define nx_icmp_ping                                    _nx_icmp_ping
#define nxd_icmp_enable                                 _nxd_icmp_enable
#define nxd_icmp_ping                                   _nxd_icmp_ping
#define nxd_icmp_source_ping                            _nxd_icmp_source_ping
#define nxd_icmpv6_ra_flag_callback_set                 _nxd_icmpv6_ra_flag_callback_set

/* APIs for IGMP. */
#define nx_igmp_enable                                  _nx_igmp_enable
#define nx_igmp_info_get                                _nx_igmp_info_get
#define nx_igmp_loopback_disable                        _nx_igmp_loopback_disable
#define nx_igmp_multicast_leave                         _nx_igmp_multicast_leave
#define nx_igmp_multicast_interface_join                _nx_igmp_multicast_interface_join
#define nx_igmp_multicast_interface_leave               _nx_igmp_multicast_interface_leave
#define nx_igmp_loopback_enable                         _nx_igmp_loopback_enable
#define nx_igmp_multicast_join                          _nx_igmp_multicast_join

/* APIs for IP. */
#define nx_ip_address_change_notify                     _nx_ip_address_change_notify
#define nx_ip_address_get                               _nx_ip_address_get
#define nx_ip_address_set                               _nx_ip_address_set
#define nx_ip_auxiliary_packet_pool_set                 _nx_ip_auxiliary_packet_pool_set
#define nx_ip_create                                    _nx_ip_create
#define nx_ip_delete                                    _nx_ip_delete
#define nx_ip_driver_direct_command                     _nx_ip_driver_direct_command
#define nx_ip_driver_interface_direct_command           _nx_ip_driver_interface_direct_command
#define nx_ip_forwarding_disable                        _nx_ip_forwarding_disable
#define nx_ip_forwarding_enable                         _nx_ip_forwarding_enable
#define nx_ip_fragment_disable                          _nx_ip_fragment_disable
#define nx_ip_fragment_enable                           _nx_ip_fragment_enable
#define nx_ip_gateway_address_clear                     _nx_ip_gateway_address_clear
#define nx_ip_gateway_address_get                       _nx_ip_gateway_address_get
#define nx_ip_gateway_address_set                       _nx_ip_gateway_address_set
#define nx_ip_info_get                                  _nx_ip_info_get
#define nx_ip_interface_address_get                     _nx_ip_interface_address_get
#define nx_ip_interface_address_mapping_configure       _nx_ip_interface_address_mapping_configure
#define nx_ip_interface_address_set                     _nx_ip_interface_address_set
#define nx_ip_interface_attach                          _nx_ip_interface_attach
#define nx_ip_interface_capability_get                  _nx_ip_interface_capability_get
#define nx_ip_interface_capability_set                  _nx_ip_interface_capability_set
#define nx_ip_interface_detach                          _nx_ip_interface_detach
#define nx_ip_interface_info_get                        _nx_ip_interface_info_get
#define nx_ip_interface_mtu_set                         _nx_ip_interface_mtu_set
#define nx_ip_interface_physical_address_get            _nx_ip_interface_physical_address_get
#define nx_ip_interface_physical_address_set            _nx_ip_interface_physical_address_set
#define nx_ip_interface_status_check                    _nx_ip_interface_status_check
#define nx_ip_link_status_change_notify_set             _nx_ip_link_status_change_notify_set
#define nx_ip_max_payload_size_find                     _nx_ip_max_payload_size_find
#define nx_ip_status_check                              _nx_ip_status_check
#define nx_ip_static_route_add                          _nx_ip_static_route_add
#define nx_ip_static_route_delete                       _nx_ip_static_route_delete
#define nx_ipv4_multicast_interface_join                _nx_ipv4_multicast_interface_join
#define nx_ipv4_multicast_interface_leave               _nx_ipv4_multicast_interface_leave
#define nxd_ipv6_address_change_notify                  _nxd_ipv6_address_change_notify
#define nxd_ipv6_address_delete                         _nxd_ipv6_address_delete
#define nxd_ipv6_address_get                            _nxd_ipv6_address_get
#define nxd_ipv6_address_set                            _nxd_ipv6_address_set
#define nxd_ipv6_default_router_add                     _nxd_ipv6_default_router_add
#define nxd_ipv6_default_router_delete                  _nxd_ipv6_default_router_delete
#define nxd_ipv6_default_router_entry_get               _nxd_ipv6_default_router_entry_get
#define nxd_ipv6_default_router_get                     _nxd_ipv6_default_router_get
#define nxd_ipv6_default_router_number_of_entries_get   _nxd_ipv6_default_router_number_of_entries_get
#define nxd_ipv6_disable                                _nxd_ipv6_disable
#define nxd_ipv6_enable                                 _nxd_ipv6_enable
#define nxd_ipv6_multicast_interface_join               _nxd_ipv6_multicast_interface_join
#define nxd_ipv6_multicast_interface_leave              _nxd_ipv6_multicast_interface_leave
#define nxd_ipv6_stateless_address_autoconfig_disable   _nxd_ipv6_stateless_address_autoconfig_disable
#define nxd_ipv6_stateless_address_autoconfig_enable    _nxd_ipv6_stateless_address_autoconfig_enable

/* APIs for RAW service. */
#define nx_ip_raw_packet_disable                        _nx_ip_raw_packet_disable
#define nx_ip_raw_packet_enable                         _nx_ip_raw_packet_enable
#define nx_ip_raw_packet_filter_set                     _nx_ip_raw_packet_filter_set
#define nx_ip_raw_packet_receive                        _nx_ip_raw_packet_receive
#define nx_ip_raw_packet_send                           _nx_ip_raw_packet_send
#define nx_ip_raw_packet_source_send                    _nx_ip_raw_packet_source_send
#define nx_ip_raw_receive_queue_max_set                 _nx_ip_raw_receive_queue_max_set
#define nxd_ip_raw_packet_send                          _nxd_ip_raw_packet_send
#define nxd_ip_raw_packet_source_send                   _nxd_ip_raw_packet_source_send

/* APIs for ND cache. */
#define nxd_nd_cache_entry_set                          _nxd_nd_cache_entry_set
#define nxd_nd_cache_entry_delete                       _nxd_nd_cache_entry_delete
#define nxd_nd_cache_hardware_address_find              _nxd_nd_cache_hardware_address_find
#define nxd_nd_cache_invalidate                         _nxd_nd_cache_invalidate
#define nxd_nd_cache_ip_address_find                    _nxd_nd_cache_ip_address_find

/* APIs for packet pool. */
#define nx_packet_allocate                              _nx_packet_allocate
#define nx_packet_copy                                  _nx_packet_copy
#define nx_packet_data_append                           _nx_packet_data_append
#define nx_packet_data_extract_offset                   _nx_packet_data_extract_offset
#define nx_packet_data_retrieve                         _nx_packet_data_retrieve
#define nx_packet_length_get                            _nx_packet_length_get
#define nx_packet_pool_create                           _nx_packet_pool_create
#define nx_packet_pool_delete                           _nx_packet_pool_delete
#define nx_packet_pool_info_get                         _nx_packet_pool_info_get
#define nx_packet_pool_low_watermark_set                _nx_packet_pool_low_watermark_set
#define nx_packet_release                               _nx_packet_release
#define nx_packet_transmit_release                      _nx_packet_transmit_release

/* APIs for RARP. */
#define nx_rarp_disable                                 _nx_rarp_disable
#define nx_rarp_enable                                  _nx_rarp_enable
#define nx_rarp_info_get                                _nx_rarp_info_get

/* APIs for TCP. */
#define nx_tcp_client_socket_bind                       _nx_tcp_client_socket_bind
#define nx_tcp_client_socket_connect                    _nx_tcp_client_socket_connect
#define nx_tcp_client_socket_port_get                   _nx_tcp_client_socket_port_get
#define nx_tcp_client_socket_unbind                     _nx_tcp_client_socket_unbind
#define nx_tcp_enable                                   _nx_tcp_enable
#define nx_tcp_free_port_find                           _nx_tcp_free_port_find
#define nx_tcp_info_get                                 _nx_tcp_info_get
#define nx_tcp_server_socket_accept                     _nx_tcp_server_socket_accept
#define nx_tcp_server_socket_listen                     _nx_tcp_server_socket_listen
#define nx_tcp_server_socket_relisten                   _nx_tcp_server_socket_relisten
#define nx_tcp_server_socket_unaccept                   _nx_tcp_server_socket_unaccept
#define nx_tcp_server_socket_unlisten                   _nx_tcp_server_socket_unlisten
#define nx_tcp_socket_bytes_available                   _nx_tcp_socket_bytes_available
#define nx_tcp_socket_create                            _nx_tcp_socket_create
#define nx_tcp_socket_delete                            _nx_tcp_socket_delete
#define nx_tcp_socket_disconnect                        _nx_tcp_socket_disconnect
#define nx_tcp_socket_disconnect_complete_notify        _nx_tcp_socket_disconnect_complete_notify
#define nx_tcp_socket_establish_notify                  _nx_tcp_socket_establish_notify
#define nx_tcp_socket_info_get                          _nx_tcp_socket_info_get
#define nx_tcp_socket_mss_get                           _nx_tcp_socket_mss_get
#define nx_tcp_socket_mss_peer_get                      _nx_tcp_socket_mss_peer_get
#define nx_tcp_socket_mss_set                           _nx_tcp_socket_mss_set
#define nx_tcp_socket_peer_info_get                     _nx_tcp_socket_peer_info_get
#define nx_tcp_socket_queue_depth_notify_set            _nx_tcp_socket_queue_depth_notify_set
#define nx_tcp_socket_receive                           _nx_tcp_socket_receive
#define nx_tcp_socket_receive_notify                    _nx_tcp_socket_receive_notify
#define nx_tcp_socket_receive_queue_max_set             _nx_tcp_socket_receive_queue_max_set
#define nx_tcp_socket_send                              _nx_tcp_socket_send
#define nx_tcp_socket_state_wait                        _nx_tcp_socket_state_wait
#define nx_tcp_socket_timed_wait_callback               _nx_tcp_socket_timed_wait_callback
#define nx_tcp_socket_transmit_configure                _nx_tcp_socket_transmit_configure
#define nx_tcp_socket_window_update_notify_set          _nx_tcp_socket_window_update_notify_set
#define nxd_tcp_client_socket_connect                   _nxd_tcp_client_socket_connect
#define nxd_tcp_socket_peer_info_get                    _nxd_tcp_socket_peer_info_get

/* APIs for UDP. */

#define nx_udp_enable                                   _nx_udp_enable
#define nx_udp_free_port_find                           _nx_udp_free_port_find
#define nx_udp_info_get                                 _nx_udp_info_get
#define nx_udp_packet_info_extract                      _nx_udp_packet_info_extract
#define nx_udp_socket_bind                              _nx_udp_socket_bind
#define nx_udp_socket_bytes_available                   _nx_udp_socket_bytes_available
#define nx_udp_socket_checksum_disable                  _nx_udp_socket_checksum_disable
#define nx_udp_socket_checksum_enable                   _nx_udp_socket_checksum_enable
#define nx_udp_socket_create                            _nx_udp_socket_create
#define nx_udp_socket_delete                            _nx_udp_socket_delete
#define nx_udp_socket_info_get                          _nx_udp_socket_info_get
#define nx_udp_socket_port_get                          _nx_udp_socket_port_get
#define nx_udp_socket_receive                           _nx_udp_socket_receive
#define nx_udp_socket_receive_notify                    _nx_udp_socket_receive_notify
#define nx_udp_socket_send                              _nx_udp_socket_send
#define nx_udp_socket_source_send                       _nx_udp_socket_source_send
#define nx_udp_socket_unbind                            _nx_udp_socket_unbind
#define nx_udp_source_extract                           _nx_udp_source_extract
#define nxd_udp_packet_info_extract                     _nxd_udp_packet_info_extract
#define nxd_udp_socket_send                             _nxd_udp_socket_send
#define nxd_udp_socket_source_send                      _nxd_udp_socket_source_send
#define nxd_udp_source_extract                          _nxd_udp_source_extract

/* APIs for others. */
#define nx_system_initialize                            _nx_system_initialize
#define nx_http_proxy_client_enable                     _nx_http_proxy_client_enable

#else

/* Services with error checking.  */
/* APIs for ARP. */
#define nx_arp_dynamic_entries_invalidate               _nxe_arp_dynamic_entries_invalidate
#define nx_arp_dynamic_entry_set                        _nxe_arp_dynamic_entry_set
#define nx_arp_enable                                   _nxe_arp_enable
#define nx_arp_entry_delete                             _nxe_arp_entry_delete
#define nx_arp_gratuitous_send                          _nxe_arp_gratuitous_send
#define nx_arp_hardware_address_find                    _nxe_arp_hardware_address_find
#define nx_arp_info_get                                 _nxe_arp_info_get
#define nx_arp_ip_address_find                          _nxe_arp_ip_address_find
#define nx_arp_static_entries_delete                    _nxe_arp_static_entries_delete
#define nx_arp_static_entry_create                      _nxe_arp_static_entry_create
#define nx_arp_static_entry_delete                      _nxe_arp_static_entry_delete

/* APIs for ICMP. */
#define nx_icmp_enable                                  _nxe_icmp_enable
#define nx_icmp_info_get                                _nxe_icmp_info_get
#define nx_icmp_ping                                    _nxe_icmp_ping
#define nxd_icmp_enable                                 _nxde_icmp_enable
#define nxd_icmp_ping                                   _nxde_icmp_ping
#define nxd_icmp_source_ping                            _nxde_icmp_source_ping
#define nxd_icmpv6_ra_flag_callback_set                 _nxde_icmpv6_ra_flag_callback_set

/* APIs for IGMP. */
#define nx_igmp_enable                                  _nxe_igmp_enable
#define nx_igmp_info_get                                _nxe_igmp_info_get
#define nx_igmp_loopback_disable                        _nxe_igmp_loopback_disable
#define nx_igmp_loopback_enable                         _nxe_igmp_loopback_enable
#define nx_igmp_multicast_interface_join                _nxe_igmp_multicast_interface_join
#define nx_igmp_multicast_interface_leave               _nxe_igmp_multicast_interface_leave
#define nx_igmp_multicast_join                          _nxe_igmp_multicast_join
#define nx_igmp_multicast_leave                         _nxe_igmp_multicast_leave

/* APIs for IP. */
#define nx_ip_address_change_notify                     _nxe_ip_address_change_notify
#define nx_ip_address_get                               _nxe_ip_address_get
#define nx_ip_address_set                               _nxe_ip_address_set
#define nx_ip_auxiliary_packet_pool_set                 _nxe_ip_auxiliary_packet_pool_set
#define nx_ip_create(i, n, a, m, d, l, p, s, y)         _nxe_ip_create(i, n, a, m, d, l, p, s, y, sizeof(NX_IP))
#define nx_ip_delete                                    _nxe_ip_delete
#define nx_ip_driver_direct_command                     _nxe_ip_driver_direct_command
#define nx_ip_driver_interface_direct_command           _nxe_ip_driver_interface_direct_command
#define nx_ip_forwarding_disable                        _nxe_ip_forwarding_disable
#define nx_ip_forwarding_enable                         _nxe_ip_forwarding_enable
#define nx_ip_fragment_disable                          _nxe_ip_fragment_disable
#define nx_ip_fragment_enable                           _nxe_ip_fragment_enable
#define nx_ip_gateway_address_clear                     _nxe_ip_gateway_address_clear
#define nx_ip_gateway_address_get                       _nxe_ip_gateway_address_get
#define nx_ip_gateway_address_set                       _nxe_ip_gateway_address_set
#define nx_ip_info_get                                  _nxe_ip_info_get
#define nx_ip_interface_address_get                     _nxe_ip_interface_address_get
#define nx_ip_interface_address_mapping_configure       _nxe_ip_interface_address_mapping_configure
#define nx_ip_interface_address_set                     _nxe_ip_interface_address_set
#define nx_ip_interface_attach                          _nxe_ip_interface_attach
#define nx_ip_interface_capability_get                  _nxe_ip_interface_capability_get
#define nx_ip_interface_capability_set                  _nxe_ip_interface_capability_set
#define nx_ip_interface_detach                          _nxe_ip_interface_detach
#define nx_ip_interface_info_get                        _nxe_ip_interface_info_get
#define nx_ip_interface_mtu_set                         _nxe_ip_interface_mtu_set
#define nx_ip_interface_physical_address_get            _nxe_ip_interface_physical_address_get
#define nx_ip_interface_physical_address_set            _nxe_ip_interface_physical_address_set
#define nx_ip_interface_status_check                    _nxe_ip_interface_status_check
#define nx_ip_link_status_change_notify_set             _nxe_ip_link_status_change_notify_set
#define nx_ip_max_payload_size_find                     _nxe_ip_max_payload_size_find
#define nx_ip_status_check                              _nxe_ip_status_check
#define nx_ip_static_route_add                          _nxe_ip_static_route_add
#define nx_ip_static_route_delete                       _nxe_ip_static_route_delete
#define nx_ipv4_multicast_interface_join                _nxe_ipv4_multicast_interface_join
#define nx_ipv4_multicast_interface_leave               _nxe_ipv4_multicast_interface_leave
#define nxd_ipv6_address_change_notify                  _nxde_ipv6_address_change_notify
#define nxd_ipv6_address_delete                         _nxde_ipv6_address_delete
#define nxd_ipv6_address_get                            _nxde_ipv6_address_get
#define nxd_ipv6_address_set                            _nxde_ipv6_address_set
#define nxd_ipv6_default_router_add                     _nxde_ipv6_default_router_add
#define nxd_ipv6_default_router_delete                  _nxde_ipv6_default_router_delete
#define nxd_ipv6_default_router_entry_get               _nxde_ipv6_default_router_entry_get
#define nxd_ipv6_default_router_get                     _nxde_ipv6_default_router_get
#define nxd_ipv6_default_router_number_of_entries_get   _nxde_ipv6_default_router_number_of_entries_get
#define nxd_ipv6_disable                                _nxde_ipv6_disable
#define nxd_ipv6_enable                                 _nxde_ipv6_enable
#define nxd_ipv6_multicast_interface_join               _nxde_ipv6_multicast_interface_join
#define nxd_ipv6_multicast_interface_leave              _nxde_ipv6_multicast_interface_leave
#define nxd_ipv6_stateless_address_autoconfig_disable   _nxde_ipv6_stateless_address_autoconfig_disable
#define nxd_ipv6_stateless_address_autoconfig_enable    _nxde_ipv6_stateless_address_autoconfig_enable

/* APIs for RAW service. */
#define nx_ip_raw_packet_disable                        _nxe_ip_raw_packet_disable
#define nx_ip_raw_packet_enable                         _nxe_ip_raw_packet_enable
#define nx_ip_raw_packet_filter_set                     _nxe_ip_raw_packet_filter_set
#define nx_ip_raw_packet_receive                        _nxe_ip_raw_packet_receive
#define nx_ip_raw_packet_send(i, p, d, t)               _nxe_ip_raw_packet_send(i, &p, d, t)
#define nx_ip_raw_packet_source_send(i, p, d, a, t)     _nxe_ip_raw_packet_source_send(i, &p, d, a, t)
#define nx_ip_raw_receive_queue_max_set                 _nxe_ip_raw_receive_queue_max_set
#define nxd_ip_raw_packet_send(i, p, d, t, l, s)        _nxde_ip_raw_packet_send(i, &p, d, t, l, s)
#define nxd_ip_raw_packet_source_send                   _nxde_ip_raw_packet_source_send

/* APIs for ND cache. */
#define nxd_nd_cache_entry_set                          _nxde_nd_cache_entry_set
#define nxd_nd_cache_entry_delete                       _nxde_nd_cache_entry_delete
#define nxd_nd_cache_hardware_address_find              _nxde_nd_cache_hardware_address_find
#define nxd_nd_cache_invalidate                         _nxde_nd_cache_invalidate
#define nxd_nd_cache_ip_address_find                    _nxde_nd_cache_ip_address_find

/* APIs for packet pool. */
#define nx_packet_allocate                              _nxe_packet_allocate
#define nx_packet_copy                                  _nxe_packet_copy
#define nx_packet_data_append                           _nxe_packet_data_append
#define nx_packet_data_extract_offset                   _nxe_packet_data_extract_offset
#define nx_packet_data_retrieve                         _nxe_packet_data_retrieve
#define nx_packet_length_get                            _nxe_packet_length_get
#define nx_packet_pool_create(p, n, l, m, s)            _nxe_packet_pool_create(p, n, l, m, s, sizeof(NX_PACKET_POOL))
#define nx_packet_pool_delete                           _nxe_packet_pool_delete
#define nx_packet_pool_info_get                         _nxe_packet_pool_info_get
#define nx_packet_pool_low_watermark_set                _nxe_packet_pool_low_watermark_set
#define nx_packet_release(p)                            _nxe_packet_release(&p)
#define nx_packet_transmit_release(p)                   _nxe_packet_transmit_release(&p)

/* APIs for RARP. */
#define nx_rarp_disable                                 _nxe_rarp_disable
#define nx_rarp_enable                                  _nxe_rarp_enable
#define nx_rarp_info_get                                _nxe_rarp_info_get

/* APIs for TCP. */
#define nx_tcp_client_socket_bind                       _nxe_tcp_client_socket_bind
#define nx_tcp_client_socket_connect                    _nxe_tcp_client_socket_connect
#define nx_tcp_client_socket_port_get                   _nxe_tcp_client_socket_port_get
#define nx_tcp_client_socket_unbind                     _nxe_tcp_client_socket_unbind
#define nx_tcp_enable                                   _nxe_tcp_enable
#define nx_tcp_free_port_find                           _nxe_tcp_free_port_find
#define nx_tcp_info_get                                 _nxe_tcp_info_get
#define nx_tcp_server_socket_accept                     _nxe_tcp_server_socket_accept
#define nx_tcp_server_socket_listen                     _nxe_tcp_server_socket_listen
#define nx_tcp_server_socket_relisten                   _nxe_tcp_server_socket_relisten
#define nx_tcp_server_socket_unaccept                   _nxe_tcp_server_socket_unaccept
#define nx_tcp_server_socket_unlisten                   _nxe_tcp_server_socket_unlisten
#define nx_tcp_socket_bytes_available                   _nxe_tcp_socket_bytes_available
#define nx_tcp_socket_create(i, s, n, t, f, l, w, u, d) _nxe_tcp_socket_create(i, s, n, t, f, l, w, u, d, sizeof(NX_TCP_SOCKET))
#define nx_tcp_socket_delete                            _nxe_tcp_socket_delete
#define nx_tcp_socket_disconnect                        _nxe_tcp_socket_disconnect
#define nx_tcp_socket_disconnect_complete_notify        _nxe_tcp_socket_disconnect_complete_notify
#define nx_tcp_socket_establish_notify                  _nxe_tcp_socket_establish_notify
#define nx_tcp_socket_info_get                          _nxe_tcp_socket_info_get
#define nx_tcp_socket_mss_get                           _nxe_tcp_socket_mss_get
#define nx_tcp_socket_mss_peer_get                      _nxe_tcp_socket_mss_peer_get
#define nx_tcp_socket_mss_set                           _nxe_tcp_socket_mss_set
#define nx_tcp_socket_peer_info_get                     _nxe_tcp_socket_peer_info_get
#define nx_tcp_socket_queue_depth_notify_set            _nxe_tcp_socket_queue_depth_notify_set
#define nx_tcp_socket_receive                           _nxe_tcp_socket_receive
#define nx_tcp_socket_receive_notify                    _nxe_tcp_socket_receive_notify
#define nx_tcp_socket_receive_queue_max_set             _nxe_tcp_socket_receive_queue_max_set
#define nx_tcp_socket_send(s, p, t)                     _nxe_tcp_socket_send(s, &p, t)
#define nx_tcp_socket_state_wait                        _nxe_tcp_socket_state_wait
#define nx_tcp_socket_timed_wait_callback               _nxe_tcp_socket_timed_wait_callback
#define nx_tcp_socket_transmit_configure                _nxe_tcp_socket_transmit_configure
#define nx_tcp_socket_window_update_notify_set          _nxe_tcp_socket_window_update_notify_set
#define nxd_tcp_client_socket_connect                   _nxde_tcp_client_socket_connect
#define nxd_tcp_socket_peer_info_get                    _nxde_tcp_socket_peer_info_get

/* APIs for UDP. */
#define nx_udp_enable                                   _nxe_udp_enable
#define nx_udp_free_port_find                           _nxe_udp_free_port_find
#define nx_udp_info_get                                 _nxe_udp_info_get
#define nx_udp_packet_info_extract                      _nxe_udp_packet_info_extract
#define nx_udp_socket_bind                              _nxe_udp_socket_bind
#define nx_udp_socket_bytes_available                   _nxe_udp_socket_bytes_available
#define nx_udp_socket_checksum_disable                  _nxe_udp_socket_checksum_disable
#define nx_udp_socket_checksum_enable                   _nxe_udp_socket_checksum_enable
#define nx_udp_socket_create(i, s, n, t, f, l, q)       _nxe_udp_socket_create(i, s, n, t, f, l, q, sizeof(NX_UDP_SOCKET))
#define nx_udp_socket_delete                            _nxe_udp_socket_delete
#define nx_udp_socket_info_get                          _nxe_udp_socket_info_get
#define nx_udp_socket_port_get                          _nxe_udp_socket_port_get
#define nx_udp_socket_receive                           _nxe_udp_socket_receive
#define nx_udp_socket_receive_notify                    _nxe_udp_socket_receive_notify
#define nx_udp_socket_send(s, p, i, t)                  _nxe_udp_socket_send(s, &p, i, t)
#define nx_udp_socket_source_send(s, p, i, t, a)        _nxe_udp_socket_source_send(s, &p, i, t, a)
#define nx_udp_socket_unbind                            _nxe_udp_socket_unbind
#define nx_udp_source_extract                           _nxe_udp_source_extract
#define nxd_udp_packet_info_extract                     _nxde_udp_packet_info_extract
#define nxd_udp_socket_send(s, p, i, t)                 _nxde_udp_socket_send(s, &p, i, t)
#define nxd_udp_socket_source_send                      _nxde_udp_socket_source_send
#define nxd_udp_source_extract                          _nxde_udp_source_extract

/* APIs for others. */
#define nx_system_initialize                            _nx_system_initialize
#define nx_http_proxy_client_enable                     _nxe_http_proxy_client_enable
#endif


/* Define the function prototypes of the NetX Duo API.  */

/* APIs for ARP. */
UINT nx_arp_dynamic_entries_invalidate(NX_IP *ip_ptr);
UINT nx_arp_dynamic_entry_set(NX_IP *ip_ptr, ULONG ip_address,  ULONG physical_msw, ULONG physical_lsw);
UINT nx_arp_enable(NX_IP *ip_ptr, VOID *arp_cache_memory, ULONG arp_cache_size);
UINT nx_arp_entry_delete(NX_IP *ip_ptr, ULONG ip_address);
UINT nx_arp_gratuitous_send(NX_IP *ip_ptr, VOID (*response_handler)(NX_IP *ip_ptr, NX_PACKET *packet_ptr));
UINT nx_arp_hardware_address_find(NX_IP *ip_ptr, ULONG ip_address, ULONG *physical_msw, ULONG *physical_lsw);
UINT nx_arp_info_get(NX_IP *ip_ptr, ULONG *arp_requests_sent, ULONG *arp_requests_received,
                     ULONG *arp_responses_sent, ULONG *arp_responses_received,
                     ULONG *arp_dynamic_entries, ULONG *arp_static_entries,
                     ULONG *arp_aged_entries, ULONG *arp_invalid_messages);
UINT nx_arp_ip_address_find(NX_IP *ip_ptr, ULONG *ip_address,  ULONG physical_msw, ULONG physical_lsw);
UINT nx_arp_static_entries_delete(NX_IP *ip_ptr);
UINT nx_arp_static_entry_create(NX_IP *ip_ptr, ULONG ip_address, ULONG physical_msw, ULONG physical_lsw);
UINT nx_arp_static_entry_delete(NX_IP *ip_ptr, ULONG ip_address,  ULONG physical_msw, ULONG physical_lsw);

/* APIs for ICMP. */
UINT nx_icmp_enable(NX_IP *ip_ptr);
UINT nx_icmp_info_get(NX_IP *ip_ptr, ULONG *pings_sent, ULONG *ping_timeouts,
                      ULONG *ping_threads_suspended, ULONG *ping_responses_received,
                      ULONG *icmp_checksum_errors, ULONG *icmp_unhandled_messages);
UINT nx_icmp_ping(NX_IP *ip_ptr, ULONG ip_address, CHAR *data, ULONG data_size,
                  NX_PACKET **response_ptr, ULONG wait_option);
UINT nxd_icmp_enable(NX_IP *ip_ptr);
UINT nxd_icmp_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, CHAR *data_ptr, ULONG data_size,
                   NX_PACKET **response_ptr, ULONG wait_option);
UINT nxd_icmp_source_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, UINT address_index, CHAR *data_ptr,
                          ULONG data_size, NX_PACKET **response_ptr, ULONG wait_option);
UINT nxd_icmpv6_ra_flag_callback_set(NX_IP *ip_ptr,
                                     VOID (*icmpv6_ra_flag_callback)(NX_IP *ip_ptr, UINT ra_flag));

/* APIs for IGMP. */
UINT nx_igmp_enable(NX_IP *ip_ptr);
UINT nx_igmp_info_get(NX_IP *ip_ptr, ULONG *igmp_reports_sent, ULONG *igmp_queries_received,
                      ULONG *igmp_checksum_errors, ULONG *current_groups_joined);
UINT nx_igmp_loopback_disable(NX_IP *ip_ptr);
UINT nx_igmp_loopback_enable(NX_IP *ip_ptr);
UINT nx_igmp_multicast_interface_join(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT nx_igmp_multicast_interface_leave(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT nx_igmp_multicast_join(NX_IP *ip_ptr, ULONG group_address);
UINT nx_igmp_multicast_leave(NX_IP *ip_ptr, ULONG group_address);

/* APIs for IP. */
UINT nx_ip_address_change_notify(NX_IP *ip_ptr, VOID (*ip_address_change_notify)(NX_IP *, VOID *), VOID *additional_info);
UINT nx_ip_address_get(NX_IP *ip_ptr, ULONG *ip_address, ULONG *network_mask);
UINT nx_ip_address_set(NX_IP *ip_ptr, ULONG ip_address, ULONG network_mask);
UINT nx_ip_auxiliary_packet_pool_set(NX_IP *ip_ptr, NX_PACKET_POOL *auxiliary_pool);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                    NX_PACKET_POOL *default_pool,
                    VOID (*ip_link_driver)(NX_IP_DRIVER *),
                    VOID *memory_ptr, ULONG memory_size, UINT priority, UINT ip_control_block_size);
#else
UINT _nx_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                   NX_PACKET_POOL *default_pool,
                   VOID (*ip_link_driver)(NX_IP_DRIVER *),
                   VOID *memory_ptr, ULONG memory_size, UINT priority);
#endif
UINT nx_ip_delete(NX_IP *ip_ptr);
UINT nx_ip_driver_direct_command(NX_IP *ip_ptr, UINT command, ULONG *return_value_ptr);
UINT nx_ip_driver_interface_direct_command(NX_IP *ip_ptr, UINT command, UINT interface_index, ULONG *return_value_ptr);
UINT nx_ip_forwarding_disable(NX_IP *ip_ptr);
UINT nx_ip_forwarding_enable(NX_IP *ip_ptr);
UINT nx_ip_fragment_disable(NX_IP *ip_ptr);
UINT nx_ip_fragment_enable(NX_IP *ip_ptr);
UINT nx_ip_gateway_address_clear(NX_IP *ip_ptr);
UINT nx_ip_gateway_address_get(NX_IP *ip_ptr, ULONG *ip_address);
UINT nx_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address);
UINT nx_ip_info_get(NX_IP *ip_ptr, ULONG *ip_total_packets_sent, ULONG *ip_total_bytes_sent,
                    ULONG *ip_total_packets_received, ULONG *ip_total_bytes_received,
                    ULONG *ip_invalid_packets, ULONG *ip_receive_packets_dropped,
                    ULONG *ip_receive_checksum_errors, ULONG *ip_send_packets_dropped,
                    ULONG *ip_total_fragments_sent, ULONG *ip_total_fragments_received);
UINT nx_ip_interface_address_get(NX_IP *ip_ptr, UINT interface_index, ULONG *ip_address, ULONG *network_mask);
UINT nx_ip_interface_address_mapping_configure(NX_IP *ip_ptr, UINT interface_index, UINT mapping_needed);
UINT nx_ip_interface_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG network_mask);
UINT nx_ip_interface_attach(NX_IP *ip_ptr, CHAR *interface_name, ULONG ip_address, ULONG network_mask,
                            VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *));
UINT nx_ip_interface_capability_get(NX_IP *ip_ptr, UINT interface_index, ULONG *interface_capability_flag);
UINT nx_ip_interface_capability_set(NX_IP *ip_ptr, UINT interface_index, ULONG interface_capability_flag);
UINT nx_ip_interface_detach(NX_IP *ip_ptr, UINT index);
UINT nx_ip_interface_info_get(NX_IP *ip_ptr, UINT interface_index, CHAR **interface_name, ULONG *ip_address,
                              ULONG *network_mask, ULONG *mtu_size, ULONG *physical_address_msw,
                              ULONG *physical_address_lsw);
UINT nx_ip_interface_mtu_set(NX_IP *ip_ptr, UINT interface_index, ULONG mtu_size);
UINT nx_ip_interface_physical_address_get(NX_IP *ip_ptr, UINT interface_index, ULONG *physical_msw,
                                          ULONG *physical_lsw);
UINT nx_ip_interface_physical_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG physical_msw,
                                          ULONG physical_lsw, UINT update_driver);
UINT nx_ip_interface_status_check(NX_IP *ip_ptr, UINT interface_index, ULONG needed_status,
                                  ULONG *actual_status, ULONG wait_option);
UINT nx_ip_link_status_change_notify_set(NX_IP *ip_ptr,
                                         VOID (*link_status_change_notify)(NX_IP *ip_ptr,
                                                                           UINT interface_index,
                                                                           UINT link_up));
UINT nx_ip_max_payload_size_find(NX_IP *ip_ptr, NXD_ADDRESS *dest_address, UINT if_index,
                                 UINT src_port, UINT dest_port, ULONG protocol, ULONG *start_offset_ptr,
                                 ULONG *payload_length_ptr);
UINT nx_ip_status_check(NX_IP *ip_ptr, ULONG needed_status, ULONG *actual_status, ULONG wait_option);
UINT nx_ip_static_route_add(NX_IP *ip_ptr, ULONG network_address, ULONG net_mask, ULONG next_hop);
UINT nx_ip_static_route_delete(NX_IP *ip_ptr, ULONG network_address, ULONG net_mask);
UINT nx_ipv4_multicast_interface_join(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT nx_ipv4_multicast_interface_leave(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT nxd_ipv6_address_change_notify(NX_IP *ip_ptr,
                                    VOID (*ip_address_change_notify)(NX_IP *ip_ptr, UINT status, UINT interface_index, UINT address_index, ULONG *ip_address));
UINT nxd_ipv6_address_delete(NX_IP *ip_ptr, UINT address_index);
UINT nxd_ipv6_address_get(NX_IP *ip_ptr, UINT address_index, NXD_ADDRESS *ip_address,
                          ULONG *prefix_length, UINT *interface_index);
UINT nxd_ipv6_address_set(NX_IP *ip_ptr, UINT interface_index, NXD_ADDRESS *ip_address,
                          ULONG prefix_length, UINT *address_index);
UINT nxd_ipv6_default_router_add(NX_IP *ip_ptr, NXD_ADDRESS *router_addr,
                                 ULONG router_lifetime, UINT interface_index);
UINT nxd_ipv6_default_router_delete(NX_IP *ip_ptr, NXD_ADDRESS *router_addr);
UINT nxd_ipv6_default_router_entry_get(NX_IP *ip_ptr, UINT interface_index, UINT entry_index,
                                       NXD_ADDRESS *router_addr, ULONG *router_lifetime,
                                       ULONG *prefix_length, ULONG *configuration_method);
UINT nxd_ipv6_default_router_get(NX_IP *ip_ptr, UINT interface_index, NXD_ADDRESS *router_addr,
                                 ULONG *router_lifetime, ULONG *prefix_length);
UINT nxd_ipv6_default_router_number_of_entries_get(NX_IP *ip_ptr, UINT interface_index, UINT *num_entries);
UINT nxd_ipv6_disable(NX_IP *ip_ptr);
UINT nxd_ipv6_enable(NX_IP *ip_ptr);
UINT nxd_ipv6_multicast_interface_join(NX_IP *ip_ptr, NXD_ADDRESS *group_address, UINT interface_index);
UINT nxd_ipv6_multicast_interface_leave(NX_IP *ip_ptr, NXD_ADDRESS *group_address, UINT interface_index);
UINT nxd_ipv6_stateless_address_autoconfig_disable(NX_IP *ip_ptr, UINT interface_index);
UINT nxd_ipv6_stateless_address_autoconfig_enable(NX_IP *ip_ptr, UINT interface_index);

/* APIs for RAW service. */
UINT nx_ip_raw_packet_disable(NX_IP *ip_ptr);
UINT nx_ip_raw_packet_enable(NX_IP *ip_ptr);
UINT nx_ip_raw_packet_filter_set(NX_IP *ip_ptr, UINT (*raw_packet_filter)(NX_IP *, ULONG, NX_PACKET *));
UINT nx_ip_raw_packet_receive(NX_IP *ip_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr,
                             ULONG destination_ip, ULONG type_of_service);
UINT _nxe_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr,
                                    ULONG destination_ip, UINT address_index, ULONG type_of_service);
#else
UINT _nx_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                            ULONG destination_ip, ULONG type_of_service);
UINT _nx_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                   ULONG destination_ip, UINT address_index, ULONG type_of_service);
#endif /* NX_DISABLE_ERROR_CHECKING */
UINT nx_ip_raw_receive_queue_max_set(NX_IP *ip_ptr, ULONG queue_max);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxde_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr, NXD_ADDRESS *destination_ip,
                              ULONG protocol, UINT ttl, ULONG tos);
#else /* NX_DISABLE_ERROR_CHECKING */
UINT _nxd_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,  NXD_ADDRESS *destination_ip,
                             ULONG protocol, UINT ttl, ULONG tos);
#endif /* NX_DISABLE_ERROR_CHECKING */
UINT nxd_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, NXD_ADDRESS *destination_ip,
                                   UINT address_index, ULONG protocol, UINT ttl, ULONG tos);

/* APIs for ND cache. */
UINT nxd_nd_cache_entry_set(NX_IP *ip_ptr, ULONG *dest_ip, UINT interface_index, CHAR *mac);
UINT nxd_nd_cache_entry_delete(NX_IP *ip_ptr, ULONG *dest_ip);
UINT nxd_nd_cache_hardware_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                                        ULONG *physical_msw, ULONG *physical_lsw, UINT *interface_index);
UINT nxd_nd_cache_invalidate(NX_IP *ip_ptr);
UINT nxd_nd_cache_ip_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                                  ULONG physical_msw, ULONG physical_lsw, UINT *interface_index);

/* APIs for packet pool. */
UINT nx_packet_allocate(NX_PACKET_POOL *pool_ptr,  NX_PACKET **packet_ptr,
                        ULONG packet_type, ULONG wait_option);
UINT nx_packet_copy(NX_PACKET *packet_ptr, NX_PACKET **new_packet_ptr,
                    NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT nx_packet_data_append(NX_PACKET *packet_ptr, VOID *data_start, ULONG data_size,
                           NX_PACKET_POOL *pool_ptr, ULONG wait_option);
UINT nx_packet_data_extract_offset(NX_PACKET *packet_ptr, ULONG offset, VOID *buffer_start,
                                   ULONG buffer_length, ULONG *bytes_copied);
UINT nx_packet_data_retrieve(NX_PACKET *packet_ptr, VOID *buffer_start, ULONG *bytes_copied);
UINT nx_packet_length_get(NX_PACKET *packet_ptr, ULONG *length);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name, ULONG payload_size,
                             VOID *memory_ptr, ULONG memory_size, UINT pool_control_block_size);
#else
UINT _nx_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name, ULONG payload_size,
                            VOID *memory_ptr, ULONG memory_size);
#endif
UINT nx_packet_pool_delete(NX_PACKET_POOL *pool_ptr);
UINT nx_packet_pool_info_get(NX_PACKET_POOL *pool_ptr, ULONG *total_packets, ULONG *free_packets,
                             ULONG *empty_pool_requests, ULONG *empty_pool_suspensions,
                             ULONG *invalid_packet_releases);
UINT nx_packet_pool_low_watermark_set(NX_PACKET_POOL *pool_ptr, ULONG low_water_mark);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_packet_release(NX_PACKET **packet_ptr_ptr);
UINT _nxe_packet_transmit_release(NX_PACKET **packet_ptr_ptr);
#else
UINT _nx_packet_release(NX_PACKET *packet_ptr);
UINT _nx_packet_transmit_release(NX_PACKET *packet_ptr);
#endif

/* APIs for RARP. */
UINT nx_rarp_disable(NX_IP *ip_ptr);
UINT nx_rarp_enable(NX_IP *ip_ptr);
UINT nx_rarp_info_get(NX_IP *ip_ptr, ULONG *rarp_requests_sent, ULONG *rarp_responses_received,
                      ULONG *rarp_invalid_messages);

/* APIs for TCP. */
UINT nx_tcp_client_socket_bind(NX_TCP_SOCKET *socket_ptr, UINT port, ULONG wait_option);
UINT nx_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, ULONG server_ip,
                                  UINT server_port, ULONG wait_option);
UINT nx_tcp_client_socket_port_get(NX_TCP_SOCKET *socket_ptr, UINT *port_ptr);
UINT nx_tcp_client_socket_unbind(NX_TCP_SOCKET *socket_ptr);
UINT nx_tcp_enable(NX_IP *ip_ptr);
UINT nx_tcp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT nx_tcp_info_get(NX_IP *ip_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                     ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                     ULONG *tcp_invalid_packets, ULONG *tcp_receive_packets_dropped,
                     ULONG *tcp_checksum_errors, ULONG *tcp_connections,
                     ULONG *tcp_disconnections, ULONG *tcp_connections_dropped,
                     ULONG *tcp_retransmit_packets);
UINT nx_tcp_server_socket_accept(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT nx_tcp_server_socket_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr, UINT listen_queue_size,
                                 VOID (*tcp_listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port));
UINT nx_tcp_server_socket_relisten(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr);
UINT nx_tcp_server_socket_unaccept(NX_TCP_SOCKET *socket_ptr);
UINT nx_tcp_server_socket_unlisten(NX_IP *ip_ptr, UINT port);
UINT nx_tcp_socket_bytes_available(NX_TCP_SOCKET *socket_ptr, ULONG *bytes_available);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                            VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                            VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr),
                            UINT tcp_socket_size);
#else
UINT _nx_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                           ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                           VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                           VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr));
#endif
UINT nx_tcp_socket_delete(NX_TCP_SOCKET *socket_ptr);
UINT nx_tcp_socket_disconnect(NX_TCP_SOCKET *socket_ptr, ULONG wait_option);
UINT nx_tcp_socket_disconnect_complete_notify(NX_TCP_SOCKET *socket_ptr,
                                              VOID (*tcp_disconnect_complete_notify)(NX_TCP_SOCKET *));
UINT nx_tcp_socket_establish_notify(NX_TCP_SOCKET *socket_ptr,
                                    VOID (*tcp_establish_notify)(NX_TCP_SOCKET *));
UINT nx_tcp_socket_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                            ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                            ULONG *tcp_retransmit_packets, ULONG *tcp_packets_queued,
                            ULONG *tcp_checksum_errors, ULONG *tcp_socket_state,
                            ULONG *tcp_transmit_queue_depth, ULONG *tcp_transmit_window,
                            ULONG *tcp_receive_window);
UINT nx_tcp_socket_mss_get(NX_TCP_SOCKET *socket_ptr, ULONG *mss);
UINT nx_tcp_socket_mss_peer_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_mss);
UINT nx_tcp_socket_mss_set(NX_TCP_SOCKET *socket_ptr, ULONG mss);
UINT nx_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *peer_ip_address, ULONG *peer_port);
UINT nx_tcp_socket_queue_depth_notify_set(NX_TCP_SOCKET *socket_ptr,
                                          VOID (*tcp_socket_queue_depth_notify)(NX_TCP_SOCKET *));
UINT nx_tcp_socket_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT nx_tcp_socket_receive_notify(NX_TCP_SOCKET *socket_ptr,
                                  VOID (*tcp_receive_notify)(NX_TCP_SOCKET *));
UINT nx_tcp_socket_receive_queue_max_set(NX_TCP_SOCKET *socket_ptr, UINT receive_queue_maximum);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_tcp_socket_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr, ULONG wait_option);
#else
UINT _nx_tcp_socket_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, ULONG wait_option);
#endif
UINT nx_tcp_socket_state_wait(NX_TCP_SOCKET *socket_ptr, UINT desired_state, ULONG wait_option);
UINT nx_tcp_socket_timed_wait_callback(NX_TCP_SOCKET *socket_ptr,
                                       VOID (*tcp_timed_wait_callback)(NX_TCP_SOCKET *));
UINT nx_tcp_socket_transmit_configure(NX_TCP_SOCKET *socket_ptr, ULONG max_queue_depth, ULONG timeout,
                                      ULONG max_retries, ULONG timeout_shift);
UINT nx_tcp_socket_window_update_notify_set(NX_TCP_SOCKET *socket_ptr,
                                            VOID (*tcp_window_update_notify)(NX_TCP_SOCKET *));
UINT nxd_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *server_ip,
                                   UINT server_port, ULONG wait_option);
UINT nxd_tcp_socket_peer_info_get(NX_TCP_SOCKET *socket_ptr, NXD_ADDRESS *peer_ip_address, ULONG *peer_port);

/* APIs for UDP. */
UINT nx_udp_enable(NX_IP *ip_ptr);
UINT nx_udp_free_port_find(NX_IP *ip_ptr, UINT port, UINT *free_port_ptr);
UINT nx_udp_info_get(NX_IP *ip_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                     ULONG *udp_packets_received, ULONG *udp_bytes_received,
                     ULONG *udp_invalid_packets, ULONG *udp_receive_packets_dropped,
                     ULONG *udp_checksum_errors);
UINT nx_udp_packet_info_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *protocol, UINT *port, UINT *interface_index);
UINT nx_udp_socket_bind(NX_UDP_SOCKET *socket_ptr, UINT  port, ULONG wait_option);
UINT nx_udp_socket_bytes_available(NX_UDP_SOCKET *socket_ptr, ULONG *bytes_available);
UINT nx_udp_socket_checksum_disable(NX_UDP_SOCKET *socket_ptr);
UINT nx_udp_socket_checksum_enable(NX_UDP_SOCKET *socket_ptr);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live,
                            ULONG queue_maximum, UINT udp_socket_size);
#else

UINT _nx_udp_socket_create(NX_IP *ip_ptr, NX_UDP_SOCKET *socket_ptr, CHAR *name,
                           ULONG type_of_service, ULONG fragment, UINT time_to_live,
                           ULONG queue_maximum);
#endif
UINT nx_udp_socket_delete(NX_UDP_SOCKET *socket_ptr);
UINT nx_udp_socket_info_get(NX_UDP_SOCKET *socket_ptr, ULONG *udp_packets_sent, ULONG *udp_bytes_sent,
                            ULONG *udp_packets_received, ULONG *udp_bytes_received, ULONG *udp_packets_queued,
                            ULONG *udp_receive_packets_dropped, ULONG *udp_checksum_errors);
UINT nx_udp_socket_port_get(NX_UDP_SOCKET *socket_ptr, UINT *port_ptr);
UINT nx_udp_socket_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option);
UINT nx_udp_socket_receive_notify(NX_UDP_SOCKET *socket_ptr,
                                  VOID (*udp_receive_notify)(NX_UDP_SOCKET *));
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxde_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr,
                           NXD_ADDRESS *ip_address, UINT port);
UINT _nxde_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                  NXD_ADDRESS *ip_address, UINT port, UINT address_index);

#else /* NX_DISABLE_ERROR_CHECKING */
UINT _nxd_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                          NXD_ADDRESS *ip_address, UINT port);
UINT _nxd_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                 NXD_ADDRESS *ip_address, UINT port, UINT address_index);
#endif /* NX_DISABLE_ERROR_CHECKING */
UINT nx_udp_socket_unbind(NX_UDP_SOCKET *socket_ptr);
UINT nx_udp_source_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *port);
UINT nxd_udp_source_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT *port);
#ifndef NX_DISABLE_ERROR_CHECKING
UINT _nxe_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr_ptr,
                          ULONG ip_address, UINT port);
UINT _nxe_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr,
                                 ULONG ip_address, UINT port, UINT address_index);
#else
UINT _nx_udp_socket_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                         ULONG ip_address, UINT port);
UINT _nx_udp_socket_source_send(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                ULONG ip_address, UINT port, UINT address_index);

#endif
UINT nxd_udp_packet_info_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address,
                                 UINT *protocol, UINT *port, UINT *interface_index);

/* APIs for others. */
VOID nx_system_initialize(VOID);
UINT nx_http_proxy_client_enable(NX_IP *ip_ptr, NXD_ADDRESS *proxy_server_ip, UINT proxy_server_port,
                                 UCHAR *username, UINT username_length, UCHAR *password, UINT password_length);

/* Define several function prototypes for exclusive use by NetX I/O drivers.  These routines
   are used by NetX drivers to report received packets to NetX.  */

/* Define the driver deferred packet routines.  Using these routines results in the lowest
   possible ISR processing time.  However, it does require slightly more overhead than the
   other NetX receive processing routines.  The _nx_ip_driver_deferred_enable routine
   should be called from the driver's initialization routine, with the driver's deferred
   packet processing routine provided.  Each packet the driver receives should be
   delivered to NetX via the _nx_ip_driver_deferred_receive function.  This function
   queues the packet for the NetX IP thread.  The NetX IP thread will then call the driver's
   deferred packet processing routine, which can then process the packet at a thread level
   of execution.  The deferred packet processing routine should use the _nx_ip_packet_receive,
   _nx_arp_packet_deferred_receive, and _nx_rarp_packet_deferred_receive to dispatch the
   appropriate packets to NetX.  In order to use the deferred packet processing, NetX
   must be built with NX_DRIVER_DEFERRED_PROCESSING defined.  */

VOID _nx_ip_driver_deferred_enable(NX_IP *ip_ptr, VOID (*driver_deferred_packet_handler)(NX_IP *ip_ptr, NX_PACKET *packet_ptr));
VOID _nx_ip_driver_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);


/* Define the driver deferred processing notification routine. Calling this routine from
   the driver will cause the driver to be called with the NX_LINK_DEFERRED_PROCESSING
   value specified in the nx_ip_driver_command field of the NX_IP_DRIVER request
   structure. This is useful in situations where the driver wishes to process activities
   like transmit complete interrupts at the thread level rather than in the ISR. Note
   that the driver must set its own internal variables in order to know what processing
   needs to be done when subsequently called from the IP helper thread. */

VOID _nx_ip_driver_deferred_processing(NX_IP *ip_ptr);


/* Define the deferred NetX receive processing routines.  These routines depend on the
   NetX I/O drive to perform enough processing in the ISR to strip the link protocol
   header and dispatch to the appropriate NetX receive processing.  These routines
   can also be called from the previously mentioned driver deferred processing.  */

VOID _nx_ip_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_arp_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_rarp_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);


/* Define the direct IP packet receive processing.  This is the lowest overhead way
   to notify NetX of a received IP packet, however, it results in the most amount of
   processing in the driver's receive ISR.  If the driver deferred packet processing
   is used, this routine should be used to notify NetX of the newly received IP packet.  */

VOID _nx_ip_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);

/* Define the direct link status event.  This is the lowest overhead way
   to notify NetX of link status event, however, it results in the most amount of
   processing in the driver's receive ISR.  */
VOID _nx_ip_driver_link_status_event(NX_IP *ip_ptr, UINT interface_index);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
/* Define the direct TCP packet receive processing.  This is used with TCP/IP offload feature.  */
VOID _nx_tcp_socket_driver_packet_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr);

/* Define the direct TCP established processing.  This is used with TCP/IP offload feature.  */
UINT _nx_tcp_socket_driver_establish(NX_TCP_SOCKET *socket_ptr, NX_INTERFACE *interface_ptr, UINT remote_port);

/* Define the direct UDP packet receive processing.  This is used with TCP/IP offload feature.  */
VOID _nx_udp_socket_driver_packet_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                          NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip, UINT remote_port);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

#endif

#ifdef FEATURE_NX_IPV6

#ifdef NX_LITTLE_ENDIAN
#define NX_IPV6_ADDRESS_CHANGE_ENDIAN(addr) _nx_ipv6_address_change_endian(addr)
#else /* BIG ENDIAN */
#define NX_IPV6_ADDRESS_CHANGE_ENDIAN(addr)
#endif /* NX_LITTLE_ENDIAN */

#endif /* FEATURE_NX_IPV6 */


#ifdef NX_IPV6_UTIL_INLINE
/* Return 1 if IPv6 address is unspecified (::)  */
#define CHECK_UNSPECIFIED_ADDRESS(ip_addr) \
    (!((ip_addr)[0] || (ip_addr)[1] || (ip_addr)[2] || (ip_addr)[3]))


/* Set IPv6 address to unspecified (::) */
#define SET_UNSPECIFIED_ADDRESS(ip_addr) \
    do {                                 \
        (ip_addr)[0] = 0;                \
        (ip_addr)[1] = 0;                \
        (ip_addr)[2] = 0;                \
        (ip_addr)[3] = 0;                \
    } while (0)


/* Copy IPv6 address. */
#define COPY_IPV6_ADDRESS(copy_from, copy_to) \
    do {                                      \
        (copy_to)[0] = (copy_from)[0];        \
        (copy_to)[1] = (copy_from)[1];        \
        (copy_to)[2] = (copy_from)[2];        \
        (copy_to)[3] = (copy_from)[3];        \
    } while (0)

/* Copy NXD IP address structure  */
#define COPY_NXD_ADDRESS(copy_from, copy_to)                                     \
    do {                                                                         \
        (copy_to) -> nxd_ip_version       = (copy_from) -> nxd_ip_version;       \
        (copy_to) -> nxd_ip_address.v6[0] = (copy_from) -> nxd_ip_address.v6[0]; \
        (copy_to) -> nxd_ip_address.v6[1] = (copy_from) -> nxd_ip_address.v6[1]; \
        (copy_to) -> nxd_ip_address.v6[2] = (copy_from) -> nxd_ip_address.v6[2]; \
        (copy_to) -> nxd_ip_address.v6[3] = (copy_from) -> nxd_ip_address.v6[3]; \
    } while (0)


#define SET_SOLICITED_NODE_MULTICAST_ADDRESS(address, unicast) \
    do {                                                       \
        (address)[0] = 0xFF020000;                             \
        (address)[1] = 0;                                      \
        (address)[2] = 1;                                      \
        (address)[3] = 0xFF000000 | (unicast)[3];              \
    } while (0)


#define CHECK_ALL_ROUTER_MCAST_ADDRESS(addr) \
    (((addr)[0] == 0xFF020000) &&            \
     ((addr)[1] == 0)          &&            \
     ((addr)[2] == 0)          &&            \
     ((addr)[3] == 2))

/* Check whether or not two IPv6 addresses are the same. */
#define CHECK_IPV6_ADDRESSES_SAME(addr1, addr2) \
    (((addr1)[0] == (addr2)[0]) &&              \
     ((addr1)[1] == (addr2)[1]) &&              \
     ((addr1)[2] == (addr2)[2]) &&              \
     ((addr1)[3] == (addr2)[3]))


#endif /* NX_IPV6_UTIL_INLINE */

/* Utility functions.  */
UINT _nx_utility_string_length_check(CHAR *input_string, UINT *string_length, UINT max_string_length);
UINT _nx_utility_string_to_uint(CHAR *input_string, UINT string_length, UINT *number);
UINT _nx_utility_uint_to_string(UINT number, UINT base, CHAR *string_buffer, UINT string_buffer_size);
UINT _nx_utility_base64_encode(UCHAR *name, UINT name_size, UCHAR *base64name, UINT base64name_size, UINT *bytes_copied);
UINT _nx_utility_base64_decode(UCHAR *base64name, UINT base64name_size, UCHAR *name, UINT name_size, UINT *bytes_copied);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif

