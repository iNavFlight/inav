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
/**   Neighbor Discovery Cache Management (ND CAHCE)                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_nd_cache.h                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance NetX IPv6 Neighbor Discovery Cache implementation  */
/*    for the ThreadX real-time kernel.  All service prototypes and data  */
/*    structure definitions are defined in this file. Please note that    */
/*    basic data type definitions and other architecture-specific         */
/*    information is contained in the file nx_port.h.                     */
/*                                                                        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
/*                                                                        */
/*    This header file is for NetX Duo internal use only.  Application    */
/*    shall not include this file directly.                               */
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

#ifndef NX_ND_CACHE_H
#define NX_ND_CACHE_H


#include "nx_ip.h"
#include "nx_ipv6.h"

#ifdef FEATURE_NX_IPV6


/* Symbols used the states of ND entries. */
#define ND_CACHE_STATE_INVALID    0
#define ND_CACHE_STATE_INCOMPLETE 1
#define ND_CACHE_STATE_REACHABLE  2
#define ND_CACHE_STATE_STALE      3
#define ND_CACHE_STATE_DELAY      4
#define ND_CACHE_STATE_PROBE      5
#define ND_CACHE_STATE_CREATED    6

/*
 * Define values used for Neighbor Discovery protocol.
 * The default values are suggested by RFC2461, chapter 10.
 */


#ifndef NX_MAX_MULTICAST_SOLICIT
/* Max multicast NS messages */
#define NX_MAX_MULTICAST_SOLICIT  3
#endif

#ifndef NX_MAX_UNICAST_SOLICIT
/* Max unicast NS messages. */
#define NX_MAX_UNICAST_SOLICIT    3
#endif

#ifndef NX_REACHABLE_TIME
/* Max reachable time, in seconds. */
#define NX_REACHABLE_TIME         30
#endif

#ifndef NX_RETRANS_TIMER
/* Max retrans timer, in milliseconds. */
#define NX_RETRANS_TIMER          1000
#endif

#ifndef NX_DELAY_FIRST_PROBE_TIME
#define NX_DELAY_FIRST_PROBE_TIME 5
#endif

/* Declare internal functions */
UINT _nx_nd_cache_interface_entries_delete(NX_IP *ip_ptr, UINT index);

/* Add en entry to the cache. */
UINT _nx_nd_cache_add(NX_IP *ip_ptr, ULONG *dest_ip, NX_INTERFACE *if_index, CHAR *mac, INT IsStatic, INT status, NXD_IPV6_ADDRESS *nxd_interface, ND_CACHE_ENTRY **entry);


/* Delete an entry based on IP address. */
UINT _nx_nd_cache_delete_internal(NX_IP *ip_ptr, ND_CACHE_ENTRY *entry);

/* 100ms periodic update. */
VOID _nx_nd_cache_fast_periodic_update(NX_IP *ip_ptr);

/* Find cache entry based on neighbor IP address. */
UINT _nx_nd_cache_find_entry(NX_IP *ip_ptr, ULONG *dest_ip, ND_CACHE_ENTRY **entry);

/* Find cache entry based on neighbor IP address.  If not found, create new entry. */
UINT _nx_nd_cache_add_entry(NX_IP *ip_ptr, ULONG *dest_ip, NXD_IPV6_ADDRESS *nxd_address, ND_CACHE_ENTRY **entry);

/* Find cache entry based on LLA. */
UINT _nx_nd_cache_find_entry_by_mac_addr(NX_IP *ip_ptr, ULONG physical_msw, ULONG physical_lsw, ND_CACHE_ENTRY **entry);

/* The real function that invalidates the whole cache. */
VOID _nxd_nd_cache_invalidate_internal(NX_IP *ip_ptr);

/* One second periodic update. */
VOID _nx_nd_cache_slow_periodic_update(NX_IP *ip_ptr);

/* Invalidate a given entry from the neighbor discovery table. */
VOID _nx_invalidate_destination_entry(NX_IP *ip_ptr, ULONG *next_hop_ip);
#endif /* FEATURE_NX_IPV6 */


/* Delete an entry based on IP address. */
UINT _nxd_nd_cache_entry_delete(NX_IP *ip_ptr, ULONG *dest_ip);
UINT _nxde_nd_cache_entry_delete(NX_IP *ip_ptr, ULONG *dest_ip);

/* Create an IPv6-MAC mapping to the ND cache entry. */
UINT _nxd_nd_cache_entry_set(NX_IP *ip_ptr, ULONG *dest_ip, UINT if_index, CHAR *mac);
UINT _nxde_nd_cache_entry_set(NX_IP *ip_ptr, ULONG *dest_ip, UINT if_index, CHAR *mac);

/* Invalidate the whole cache. */
UINT _nxd_nd_cache_invalidate(NX_IP *ip_ptr);
UINT _nxde_nd_cache_invalidate(NX_IP *ip_ptr);

UINT _nxd_nd_cache_ip_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, ULONG physical_msw, ULONG physical_lsw, UINT *if_index);
UINT _nxde_nd_cache_ip_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, ULONG physical_msw, ULONG physical_lsw, UINT *if_index);

UINT _nxd_nd_cache_hardware_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, ULONG *physical_msw, ULONG *physical_lsw, UINT *if_index);
UINT _nxde_nd_cache_hardware_address_find(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, ULONG *physical_msw, ULONG *physical_lsw, UINT *if_index);

#endif /* NX_ND_CACHE_H */

