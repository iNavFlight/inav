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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_enable                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the ARP management component for the          */
/*    specified IP instance.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    arp_cache_memory                      Start of ARP cache memory     */
/*    arp_cache_size                        Size in bytes of cache memory */
/*    memset                                Set the memory                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_arp_enable(NX_IP *ip_ptr, VOID *arp_cache_memory, ULONG arp_cache_size)
{

#ifndef NX_DISABLE_IPV4
ULONG   i;
ULONG   arp_entries;
NX_ARP *entry_ptr;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_ENABLE, ip_ptr, arp_cache_memory, arp_cache_size, 0, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Clear the entire ARP cache.  */
    memset((void *)arp_cache_memory, 0, arp_cache_size);

    /* Pickup starting address of ARP entry array.  */
    entry_ptr =  (NX_ARP *)arp_cache_memory;

    /* Determine how many ARP entries will fit in this cache area.  */
    arp_entries =  arp_cache_size / sizeof(NX_ARP);

    /* Initialize the forward pointers of available ARP entries.  */
    for (i = 0; i < (arp_entries - 1); i++)
    {
        /* Setup each entry to point to the next entry.  */
        entry_ptr -> nx_arp_pool_next =  entry_ptr + 1;
        entry_ptr++;
    }

    /* The entry now points to the last entry in the ARP array.  Set its
       next pointer to the first entry.  */
    entry_ptr -> nx_arp_pool_next =  (NX_ARP *)arp_cache_memory;

    /* Initialize the backward pointers of available ARP entries.  */
    for (i = 0; i < (arp_entries - 1); i++)
    {
        /* Setup each entry to point to the previous entry.  */
        entry_ptr -> nx_arp_pool_previous =  entry_ptr - 1;
        entry_ptr--;
    }

    /* The entry now points to the first entry, set the previous pointer
       to the last entry.  */
    entry_ptr -> nx_arp_pool_previous =  (entry_ptr + (arp_entries - 1));

    /* At this point, everything is okay in the ARP enable call.. populate the
       information in the IP structure.  */

    /* Setup the list head pointers in the IP instance.  At first all ARP
       entries are associated with the dynamic ARP list.  The static ARP list
       is NULL until static ARP entry calls are made.  */
    ip_ptr -> nx_ip_arp_static_list =   NX_NULL;
    ip_ptr -> nx_ip_arp_dynamic_list =  (NX_ARP *)arp_cache_memory;

    /* Store the initial ARP cache information in the IP control block.  */
    ip_ptr -> nx_ip_arp_cache_memory  =  arp_cache_memory;
    ip_ptr -> nx_ip_arp_total_entries =  arp_entries;

    /* Setup the ARP periodic update routine.  */
    ip_ptr -> nx_ip_arp_periodic_update =  _nx_arp_periodic_update;

    /* Setup the ARP queue process routine.  */
    ip_ptr -> nx_ip_arp_queue_process =  _nx_arp_queue_process;

    /* Setup the ARP send packet routine.  */
    ip_ptr -> nx_ip_arp_packet_send =  _nx_arp_packet_send;

    /* Setup the ARP allocate service request pointer.  */
    ip_ptr -> nx_ip_arp_allocate =  _nx_arp_entry_allocate;

    /* Return successful completion.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(arp_cache_memory);
    NX_PARAMETER_NOT_USED(arp_cache_size);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

