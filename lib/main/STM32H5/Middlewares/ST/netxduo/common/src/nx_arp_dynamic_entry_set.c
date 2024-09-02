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
#include "nx_ip.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_dynamic_entry_set                           PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates an ARP dynamic entry for the application    */
/*    and assigns the specified IP to hardware mapping. If the specified  */
/*    hardware address is zero, an actual ARP request will be sent out.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            IP Address to bind to         */
/*    physical_msw                          Physical address MSW          */
/*    physical_lsw                          Physical address LSW          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_route_find                     Find suitable outgoing        */
/*                                            interface                   */
/*    _nx_arp_entry_allocate                Allocate an ARP entry         */
/*    _nx_arp_packet_send                   Send ARP request              */
/*    _nx_arp_queue_send                    Send the queued packet        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
/*  04-02-2021     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the returned status,        */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_arp_dynamic_entry_set(NX_IP *ip_ptr, ULONG ip_address,
                                ULONG physical_msw, ULONG physical_lsw)
{

#ifndef NX_DISABLE_IPV4
NX_ARP       *arp_ptr;
NX_ARP       *search_ptr;
NX_ARP       *arp_list_head;
UINT          index;
UINT          status;
NX_INTERFACE *nx_interface = NX_NULL;
ULONG         next_hop_address;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_DYNAMIC_ENTRY_SET, ip_ptr, ip_address, physical_msw, physical_lsw, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Initialize next_hop_address to zero. */
    next_hop_address = 0;

    /* Make sure the destination address is directly accessible. */
    if (_nx_ip_route_find(ip_ptr, ip_address, &nx_interface, &next_hop_address) != NX_SUCCESS)
    {

        return(NX_IP_ADDRESS_ERROR);
    }

    /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized by _nx_ip_route_find. */
    if (next_hop_address != ip_address)
    {

        return(NX_IP_ADDRESS_ERROR);
    }

    /* Obtain protection on this IP instance for access into the ARP dynamic
       list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Calculate the hash index for the specified IP address.  */
    index =  (UINT)((ip_address + (ip_address >> 8)) & NX_ARP_TABLE_MASK);

    /* Pickup the head pointer of the ARP entries for this IP instance.  */
    arp_list_head =  ip_ptr -> nx_ip_arp_table[index];

    /* Search the ARP list for the same IP address.  */
    search_ptr =  arp_list_head;
    arp_ptr =     NX_NULL;
    while (search_ptr)
    {

        /* Determine if there is a duplicate IP address.  */
        if (search_ptr -> nx_arp_ip_address == ip_address)
        {

            /* Yes, the IP address matches, setup the ARP entry pointer.  */
            arp_ptr =  search_ptr;

            /* Get out of the loop.  */
            break;
        }

        /* Move to the next entry in the active list.  */
        search_ptr =  search_ptr -> nx_arp_active_next;

        /* Determine if the search pointer is back at the head of
           the list.  */
        if (search_ptr == arp_list_head)
        {

            /* End of the ARP list, end the search.  */
            break;
        }
    }

    /* Determine if an ARP entry is found.  */
    if (arp_ptr)
    {

        /* Determine if this is a static entry. */
        if (arp_ptr -> nx_arp_route_static == NX_TRUE)
        {

            /* Release the mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return the error status.  */
            return(NX_DUPLICATED_ENTRY);
        }
    }
    else
    {

        /* No matching IP address in the ARP cache and a new dynamic entry needs to be allocated.  */

        /* Allocate a dynamic ARP entry.  */
        status =  _nx_arp_entry_allocate(ip_ptr, &(ip_ptr -> nx_ip_arp_table[index]), NX_FALSE);

        /* Determine if an error occurred.  */
        if (status != NX_SUCCESS)
        {

            /* Release the mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return the error status.  */
            return(status);
        }

        /* Otherwise, setup a pointer to the new ARP entry.  The newly allocated
           ARP entry was allocated at the end of the ARP list so it should be
           referenced using the previous pointer from the list head.  */
        arp_ptr =  (ip_ptr -> nx_ip_arp_table[index]) -> nx_arp_active_previous;
    }

    /* Setup the IP address and clear the physical mapping.  */
    arp_ptr -> nx_arp_ip_address =            ip_address;
    arp_ptr -> nx_arp_physical_address_msw =  physical_msw;
    arp_ptr -> nx_arp_physical_address_lsw =  physical_lsw;
    arp_ptr -> nx_arp_retries =               0;

    /*lint -e{644} suppress variable might not be initialized, since "nx_interface" was initialized in _nx_ip_route_find. */
    arp_ptr -> nx_arp_ip_interface =          nx_interface;

    /* Determine if a physical address was supplied.  */
    if ((physical_msw | physical_lsw) == 0)
    {

        /* Since there isn't physical mapping, change the update rate
           for possible ARP retries.  */
        arp_ptr -> nx_arp_entry_next_update =     NX_ARP_UPDATE_RATE;

        /* The physical address was not specified so send an
           ARP request for the selected IP address.  */
        /*lint -e{668} suppress possibly passing a null pointer, since nx_interface is set in _nx_ip_route_find.  */
        _nx_arp_packet_send(ip_ptr, ip_address, nx_interface);
    }
    else
    {

        /* Update the next update time.  */
        arp_ptr -> nx_arp_entry_next_update = NX_ARP_EXPIRATION_RATE;

        /* Call queue send function to send the packet queued up.  */
        _nx_arp_queue_send(ip_ptr, arp_ptr);
    }

    /* Release the protection on the ARP list.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return status to the caller.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

