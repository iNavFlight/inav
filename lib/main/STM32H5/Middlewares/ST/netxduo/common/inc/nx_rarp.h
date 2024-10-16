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
/**   Reverse Address Resolution Protocol (RARP)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_rarp.h                                           PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Reverse Address Resolution Protocol      */
/*    component, including all data types and external references.  It    */
/*    is assumed that nx_api.h and nx_port.h have already been included.  */
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

#ifndef NX_RARP_H
#define NX_RARP_H

#include "nx_api.h"


#ifndef NX_DISABLE_IPV4
/* Define RARP Message format.  This will get encapsulated by an Ethernet frame
   as well.  The Ethernet frame will typically have a 6-byte Ethernet destination
   address, a 6-byte Ethernet source address, and a 2-byte Ethernet Frame type,
   which is 0x8035.  Regular IP frames have a frame type of 0x0800.

    Byte offset     Size            Meaning

        0           2           Hardware type (1 for Ethernet)
        2           2           Protocol type (0x0800 for IP)
        4           1           Number of bytes for hardware address (6 for Ethernet)
        5           1           Number of bytes for IP address (4 for IP)
        6           2           Operation, ARP request is 1, ARP reply is 2
        8           6           Sender's Ethernet Address
        14          4           Sender's IP Address
        18          6           Target Ethernet Address
        24          4           Target IP Address
 */

#define NX_RARP_HARDWARE_TYPE   ((ULONG)0x0001)
#define NX_RARP_PROTOCOL_TYPE   ((ULONG)0x0800)
#define NX_RARP_HARDWARE_SIZE   ((ULONG)0x06)
#define NX_RARP_PROTOCOL_SIZE   ((ULONG)0x04)
#define NX_RARP_OPTION_REQUEST  ((ULONG)0x0003)
#define NX_RARP_OPTION_RESPONSE ((ULONG)0x0004)
#define NX_RARP_MESSAGE_SIZE    28

/* Define RARP internal function prototypes.  */
VOID _nx_rarp_packet_send(NX_IP *ip_ptr);
VOID _nx_rarp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_rarp_packet_deferred_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_rarp_periodic_update(NX_IP *ip_ptr);
VOID _nx_rarp_queue_process(NX_IP *ip_ptr);
#endif /* NX_DISABLE_IPV4 */

/* Define RARP function prototypes.  */
UINT _nx_rarp_enable(NX_IP *ip_ptr);
UINT _nx_rarp_disable(NX_IP *ip_ptr);
UINT _nx_rarp_info_get(NX_IP *ip_ptr, ULONG *rarp_requests_sent, ULONG *rarp_responses_received,
                       ULONG *rarp_invalid_messages);

/* Define error checking shells for RARP services.  These are only referenced by the
   application.  */

UINT _nxe_rarp_enable(NX_IP *ip_ptr);
UINT _nxe_rarp_disable(NX_IP *ip_ptr);
UINT _nxe_rarp_info_get(NX_IP *ip_ptr, ULONG *rarp_requests_sent, ULONG *rarp_responses_received,
                        ULONG *rarp_invalid_messages);
#endif

