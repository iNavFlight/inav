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
/**   System Management (System)                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Locate NetX system data in this file.  */

#define NX_SYSTEM_INIT


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_system.h"
#include "nx_packet.h"
#include "nx_arp.h"
#include "nx_rarp.h"
#include "nx_ip.h"
#include "nx_udp.h"
#include "nx_icmp.h"
#include "nx_igmp.h"
#include "nx_tcp.h"

#ifdef FEATURE_NX_IPV6
#include "nx_nd_cache.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_system_initialize                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various components and system data    */
/*    structures.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_start                            Packet pool starting address  */
/*    pool_size                             Packet pool size in bytes     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_pool_initialize            Initialize Packet Pool        */
/*                                            component                   */
/*    _nx_ip_initialize                     Initialize IP component       */
/*    _nx_tcp_initialize                    Initialize TCP component      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
VOID  _nx_system_initialize(VOID)
{

    /* Check whether or not system has been initialized? */
    if (_nx_system_build_options_1 | _nx_system_build_options_2 |
        _nx_system_build_options_3 | _nx_system_build_options_4 | _nx_system_build_options_5)
    {

        /* Yes it is. Just return. */
        return;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_SYSTEM_INITIALIZE, 0, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Call the packet pool initialization component for NetX.  */
    _nx_packet_pool_initialize();

    /* Call the IP component initialization.  */
    _nx_ip_initialize();

    /* Call the TCP component initialization.  */
    /*lint -e{522} suppress lack of side-effects.  */
    _nx_tcp_initialize();

    /* Setup the build options variables.  */
    _nx_system_build_options_1 = 0
#ifdef NX_LITTLE_ENDIAN
        | (((ULONG)1) << 31)
#endif
#ifdef NX_DISABLE_ARP_AUTO_ENTRY
        | (((ULONG)1) << 30)
#endif
#ifdef NX_ENABLE_TCP_KEEPALIVE
        | (((ULONG)1) << 29)
#endif
#ifdef NX_TCP_IMMEDIATE_ACK
        | (((ULONG)1) << 28)
#endif
#ifdef NX_DRIVER_DEFERRED_PROCESSING
        | (((ULONG)1) << 27)
#endif
#ifdef NX_DISABLE_FRAGMENTATION
        | (((ULONG)1) << 26)
#endif
#ifdef NX_DISABLE_IP_RX_CHECKSUM
        | (((ULONG)1) << 25)
#endif
#ifdef NX_DISABLE_IP_TX_CHECKSUM
        | (((ULONG)1) << 24)
#endif
#ifdef NX_DISABLE_TCP_RX_CHECKSUM
        | (((ULONG)1) << 23)
#endif
#ifdef NX_DISABLE_TCP_TX_CHECKSUM
        | (((ULONG)1) << 22)
#endif
#ifdef NX_DISABLE_RESET_DISCONNECT
        | (((ULONG)1) << 21)
#endif
#ifdef NX_DISABLE_RX_SIZE_CHECKING
        | (((ULONG)1) << 20)
#endif
#ifdef NX_DISABLE_ARP_INFO
        | (((ULONG)1) << 19)
#endif
#ifdef NX_DISABLE_IP_INFO
        | (((ULONG)1) << 18)
#endif
#ifdef NX_DISABLE_ICMP_INFO
        | (((ULONG)1) << 17)
#endif
#ifdef NX_DISABLE_IGMP_INFO
        | (((ULONG)1) << 16)
#endif
#ifdef NX_DISABLE_PACKET_INFO
        | (((ULONG)1) << 15)
#endif
#ifdef NX_DISABLE_RARP_INFO
        | (((ULONG)1) << 14)
#endif
#ifdef NX_DISABLE_TCP_INFO
        | (((ULONG)1) << 13)
#endif
#ifdef NX_DISABLE_UDP_INFO
        | (((ULONG)1) << 12)
#endif
    ;


    /* Add the retry shift value to the options.  */
#if (NX_TCP_RETRY_SHIFT > 0xF)
    _nx_system_build_options_1 |=  0xF;
#else
    _nx_system_build_options_1 |=  NX_TCP_RETRY_SHIFT;
#endif

#if (NX_IP_PERIODIC_RATE > 0xFFFFUL)
    _nx_system_build_options_2 =  ((ULONG)0xFFFF0000);
#else
    _nx_system_build_options_2 =  ((ULONG)NX_IP_PERIODIC_RATE) << 16;
#endif

#if (NX_ARP_EXPIRATION_RATE > 0xFF)
    _nx_system_build_options_2 |=  ((ULONG)0xFF) << 8;
#else
    _nx_system_build_options_2 |=  ((ULONG)NX_ARP_EXPIRATION_RATE) << 8;
#endif
#if (NX_ARP_UPDATE_RATE > 0xFF)
    _nx_system_build_options_2 |=  ((ULONG)0xFF);
#else
    _nx_system_build_options_2 |=  ((ULONG)NX_ARP_UPDATE_RATE);
#endif

    /* Setup third option word.  */
#if (NX_TCP_ACK_TIMER_RATE > 0xFF)
    _nx_system_build_options_3 =  ((ULONG)0xFF000000);
#else
    _nx_system_build_options_3 =  ((ULONG)NX_TCP_ACK_TIMER_RATE) << 24;
#endif
#if (NX_TCP_FAST_TIMER_RATE > 0xFF)
    _nx_system_build_options_3 |=  ((ULONG)0xFF) << 16;
#else
    _nx_system_build_options_3 |=  ((ULONG)NX_TCP_FAST_TIMER_RATE) << 16;
#endif
#if (NX_TCP_TRANSMIT_TIMER_RATE > 0xFF)
    _nx_system_build_options_3 |=  ((ULONG)0xFF) << 8;
#else
    _nx_system_build_options_3 |=  ((ULONG)NX_TCP_TRANSMIT_TIMER_RATE) << 8;
#endif
#if (NX_TCP_KEEPALIVE_RETRY > 0xFF)
    _nx_system_build_options_3 |=  ((ULONG)0xFF);
#else
    _nx_system_build_options_3 |=  ((ULONG)NX_TCP_KEEPALIVE_RETRY);
#endif

    /* Setup the fourth option word.  */
#if (NX_TCP_KEEPALIVE_INITIAL > 0xFFFFUL)
    _nx_system_build_options_4 =  ((ULONG)0xFFFF0000);
#else
    _nx_system_build_options_4 =  ((ULONG)NX_TCP_KEEPALIVE_INITIAL) << 16;
#endif
#if (NX_ARP_MAXIMUM_RETRIES > 0xFF)
    _nx_system_build_options_4 |=  ((ULONG)0xFF) << 8;
#else
    _nx_system_build_options_4 |=  ((ULONG)NX_ARP_MAXIMUM_RETRIES) << 8;
#endif
#if (NX_ARP_MAX_QUEUE_DEPTH > 0xF)
    _nx_system_build_options_4 |=  ((ULONG)0xF) << 4;
#else
    _nx_system_build_options_4 |=  ((ULONG)NX_ARP_MAX_QUEUE_DEPTH) << 4;
#endif
#if (NX_TCP_KEEPALIVE_RETRIES > 0xF)
    _nx_system_build_options_4 |=  ((ULONG)0xF);
#else
    _nx_system_build_options_4 |=  ((ULONG)NX_TCP_KEEPALIVE_RETRIES);
#endif

    /* Setup the fifth option word.  */
#if (NX_MAX_MULTICAST_GROUPS > 0xFF)
    _nx_system_build_options_5 =  ((ULONG)0xFF000000);
#else
    _nx_system_build_options_5 =  ((ULONG)NX_MAX_MULTICAST_GROUPS) << 24;
#endif
#if (NX_MAX_LISTEN_REQUESTS > 0xFF)
    _nx_system_build_options_5 |=  ((ULONG)0xFF) << 16;
#else
    _nx_system_build_options_5 |=  ((ULONG)NX_MAX_LISTEN_REQUESTS) << 16;
#endif
#if (NX_TCP_MAXIMUM_RETRIES > 0xFF)
    _nx_system_build_options_5 |=  ((ULONG)0xFF) << 8;
#else
    _nx_system_build_options_5 |=  ((ULONG)NX_TCP_MAXIMUM_RETRIES) << 8;
#endif
#if (NX_TCP_MAXIMUM_TX_QUEUE > 0xFF)
    _nx_system_build_options_5 |=  ((ULONG)0xFF);
#else
    _nx_system_build_options_5 |=  ((ULONG)NX_TCP_MAXIMUM_TX_QUEUE);
#endif
}

