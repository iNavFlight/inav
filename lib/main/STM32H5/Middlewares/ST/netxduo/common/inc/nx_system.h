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


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_system.h                                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX system management component,             */
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
/*                                                                        */
/**************************************************************************/

#ifndef NX_SYS_H
#define NX_SYS_H



/* Define system management function prototypes.  */

VOID _nx_system_initialize(VOID);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

/* System management component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

/*lint -e767 suppress different definitions.  */
#ifdef NX_SYSTEM_INIT
#define SYSTEM_DECLARE
#else
#define SYSTEM_DECLARE extern
#endif
/*lint +e767 enable checking for different definitions.  */


/* Define the global NetX build options variables. These variables contain a bit
   map representing how the NetX library was built. The following are the bit
   field definitions:

    _nx_system_build_options_1:

                    Bit(s)                   Meaning

                    31                  NX_LITTLE_ENDIAN
                    30                  NX_DISABLE_ARP_AUTO_ENTRY
                    29                  NX_ENABLE_TCP_KEEPALIVE
                    28                  NX_TCP_IMMEDIATE_ACK
                    27                  NX_DRIVER_DEFERRED_PROCESSING
                    26                  NX_DISABLE_FRAGMENTATION
                    25                  NX_DISABLE_IP_RX_CHECKSUM
                    24                  NX_DISABLE_IP_TX_CHECKSUM
                    23                  NX_DISABLE_TCP_RX_CHECKSUM
                    22                  NX_DISABLE_TCP_TX_CHECKSUM
                    21                  NX_DISABLE_RESET_DISCONNECT
                    20                  NX_DISABLE_RX_SIZE_CHECKING
                    19                  NX_DISABLE_ARP_INFO
                    18                  NX_DISABLE_IP_INFO
                    17                  NX_DISABLE_ICMP_INFO
                    16                  NX_DISABLE_IGMP_INFO
                    15                  NX_DISABLE_PACKET_INFO
                    14                  NX_DISABLE_RARP_INFO
                    13                  NX_DISABLE_TCP_INFO
                    12                  NX_DISABLE_UDP_INFO
                    3-0                 NX_TCP_RETRY_SHIFT

    _nx_system_build_options_2:

                    Bit(s)                   Meaning

                    31-16               NX_IP_PERIODIC_RATE
                    15-8                NX_ARP_EXPIRATION_RATE
                    7-0                 NX_ARP_UPDATE_RATE

    _nx_system_build_options_3:

                    Bit(s)                   Meaning

                    31-24               NX_TCP_ACK_TIMER_RATE
                    23-16               NX_TCP_FAST_TIMER_RATE
                    15-8                NX_TCP_TRANSMIT_TIMER_RATE
                    7-0                 NX_TCP_KEEPALIVE_RETRY

    _nx_system_build_options_4:

                    Bit(s)                   Meaning

                    31-16               NX_TCP_KEEPALIVE_INITIAL
                    15-8                NX_ARP_MAXIMUM_RETRIES
                    7-4                 NX_ARP_MAX_QUEUE_DEPTH
                    3-0                 NX_TCP_KEEPALIVE_RETRIES

    _nx_system_build_options_5:

                    Bit(s)                   Meaning

                    31-24               NX_MAX_MULTICAST_GROUPS
                    23-16               NX_MAX_LISTEN_REQUESTS
                    15-8                NX_TCP_MAXIMUM_RETRIES
                    7-0                 NX_TCP_MAXIMUM_TX_QUEUE

   Note that values greater than the value that can be represented in the build options
   bit field are represented as all ones in the bit field. For example, if NX_TCP_ACK_TIMER_RATE
   is 256, the value in the bits 31-24 of _nx_system_build_options_3 is 0xFF, which is 255
   decimal.  */

SYSTEM_DECLARE  ULONG       _nx_system_build_options_1;
SYSTEM_DECLARE  ULONG       _nx_system_build_options_2;
SYSTEM_DECLARE  ULONG       _nx_system_build_options_3;
SYSTEM_DECLARE  ULONG       _nx_system_build_options_4;
SYSTEM_DECLARE  ULONG       _nx_system_build_options_5;


#endif

