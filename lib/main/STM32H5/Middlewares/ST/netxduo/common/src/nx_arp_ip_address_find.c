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
/*    _nx_arp_ip_address_find                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for the specified hardware address in the    */
/*    ARP lists.  If found, the associated IP address is returned.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            IP Address return pointer     */
/*    physical_msw                          Physical address MSW          */
/*    physical_lsw                          Physical address LSW          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_arp_ip_address_find(NX_IP *ip_ptr, ULONG *ip_address,
                              ULONG physical_msw, ULONG physical_lsw)
{

#ifndef NX_DISABLE_IPV4
NX_ARP *arp_entry;
ULONG   count;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_IP_ADDRESS_FIND, ip_ptr, 0, physical_msw, physical_lsw, NX_TRACE_ARP_EVENTS, &trace_event, &trace_timestamp);

    /* Obtain protection on this IP instance for access into the ARP static
       list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Search the static list for matching hardware mapping.  */
    arp_entry =  ip_ptr -> nx_ip_arp_static_list;
    while (arp_entry)
    {

        /* Determine if we have a hardware address match.  */
        if ((arp_entry -> nx_arp_physical_address_msw == physical_msw) &&
            (arp_entry -> nx_arp_physical_address_lsw == physical_lsw))
        {

            /* Yes, we have found the ARP entry we are looking for.  */
            break;
        }
        else
        {

            /* Determine if we are at the end of the list.  */
            if (arp_entry -> nx_arp_pool_next == ip_ptr -> nx_ip_arp_static_list)
            {

                /* Set the arp_entry to NULL to signify nothing was found and get
                   out of the search loop.  */
                arp_entry =  NX_NULL;
                break;
            }
            else
            {

                /* Just move to the next ARP entry on the static list.  */
                arp_entry =  arp_entry -> nx_arp_pool_next;
            }
        }
    }

    /* Determine if an entry has been found.  If so, we are finished and it needs to be
       returned to the caller.  */
    if (arp_entry)
    {

        /* Store the IP address in the return field.  */
        *ip_address =  arp_entry -> nx_arp_ip_address;

        /* Update the trace event with the status.  */
        NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_ARP_IP_ADDRESS_FIND, 0, *ip_address, 0, 0);

        /* Release the protection on the ARP list.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return status to the caller.  */
        return(NX_SUCCESS);
    }

    /* Otherwise, we need to search the ARP dynamic list for a match.  */
    arp_entry =  ip_ptr -> nx_ip_arp_dynamic_list;
    count =      1;
    while (arp_entry)
    {

        /* Determine if we have a hardware address match.  */
        if ((arp_entry -> nx_arp_physical_address_msw == physical_msw) &&
            (arp_entry -> nx_arp_physical_address_lsw == physical_lsw))
        {

            /* Yes, we have found the ARP entry we are looking for.  */
            break;
        }
        else
        {

            /* Determine if we are at the end of the list of active ARP entries.  */
            if (count >=  ip_ptr -> nx_ip_arp_dynamic_active_count)
            {

                /* Set the arp_entry to NULL to signify nothing was found and get
                   out of the search loop.  */
                arp_entry =  NX_NULL;
                break;
            }
            else
            {

                /* Just move to the next ARP entry on the dynamic list.  */
                arp_entry =  arp_entry -> nx_arp_pool_next;

                /* Increment the active count.  */
                count++;
            }
        }
    }

    /* Determine if an entry has been found.  If so, we are finished and it needs to be
       returned to the caller.  */
    if (arp_entry)
    {

        /* Store the IP address in the return fields.  */
        *ip_address =  arp_entry -> nx_arp_ip_address;

        /* Update the trace event with the status.  */
        NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_ARP_IP_ADDRESS_FIND, 0, *ip_address, 0, 0);

        /* Release the protection on the ARP list.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return status to the caller.  */
        return(NX_SUCCESS);
    }
    else
    {

        /* Release the protection on the ARP list.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return status to the caller.  */
        return(NX_ENTRY_NOT_FOUND);
    }
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

