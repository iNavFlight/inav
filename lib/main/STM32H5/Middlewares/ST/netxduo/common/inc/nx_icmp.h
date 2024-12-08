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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_icmp.h                                           PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Internet Control Message Protocol (ICMP) */
/*    component, including all data types and external references.  It is */
/*    assumed that nx_api.h and nx_port.h have already been included.     */
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

#ifndef NX_ICMP_H
#define NX_ICMP_H

#include "nx_api.h"

VOID _nx_icmp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID _nx_icmp_cleanup(TX_THREAD *thread_ptr NX_CLEANUP_PARAMETER);

UINT _nxd_icmp_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                    CHAR *data_ptr, ULONG data_size,
                    NX_PACKET **response_ptr, ULONG wait_option);
UINT _nxd_icmp_source_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                           UINT address_index, CHAR *data_ptr, ULONG data_size,
                           NX_PACKET **response_ptr, ULONG wait_option);

UINT _nxd_icmp_enable(NX_IP *ip_ptr);

UINT _nxde_icmp_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                     CHAR *data_ptr, ULONG data_size,
                     NX_PACKET **response_ptr, ULONG wait_option);
UINT _nxde_icmp_source_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address,
                            UINT address_index, CHAR *data_ptr, ULONG data_size,
                            NX_PACKET **response_ptr, ULONG wait_option);
UINT _nxde_icmp_enable(NX_IP *ip_ptr);

#include "nx_icmpv4.h"
#include "nx_icmpv6.h"

#endif

