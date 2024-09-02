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
/*    _nx_arp_info_get                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function obtains ARP information for the specified IP          */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    arp_requests_sent                     Destination for number of ARP */
/*                                            requests sent               */
/*    arp_requests_received                 Destination for number of ARP */
/*                                            requests received           */
/*    arp_responses_sent                    Destination for number of ARP */
/*                                            responses sent              */
/*    arp_responses_received                Destination for number of ARP */
/*                                            responses received          */
/*    arp_dynamic_entries                   Destination for number of ARP */
/*                                            dynamic entries             */
/*    arp_static_entries                    Destination for number of ARP */
/*                                            static entries              */
/*    arp_aged_entries                      Destination for number of ARP */
/*                                            aged entries                */
/*    arp_invalid_messages                  Destination for number of ARP */
/*                                            invalid messages            */
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
UINT  _nx_arp_info_get(NX_IP *ip_ptr, ULONG *arp_requests_sent, ULONG *arp_requests_received,
                       ULONG *arp_responses_sent, ULONG *arp_responses_received,
                       ULONG *arp_dynamic_entries, ULONG *arp_static_entries,
                       ULONG *arp_aged_entries, ULONG *arp_invalid_messages)
{

#ifndef NX_DISABLE_IPV4
    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_INFO_GET, ip_ptr, ip_ptr -> nx_ip_arp_requests_sent, ip_ptr -> nx_ip_arp_responses_received, ip_ptr -> nx_ip_arp_requests_received, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if ARP requests sent is wanted.  */
    if (arp_requests_sent)
    {

        /* Return the number of ARP requests sent by this IP instance.  */
        *arp_requests_sent =  ip_ptr -> nx_ip_arp_requests_sent;
    }

    /* Determine if ARP requests received is wanted.  */
    if (arp_requests_received)
    {

        /* Return the number of ARP requests received by this IP instance.  */
        *arp_requests_received =  ip_ptr -> nx_ip_arp_requests_received;
    }

    /* Determine if ARP responses sent is wanted.  */
    if (arp_responses_sent)
    {

        /* Return the number of ARP responses sent by this IP instance.  */
        *arp_responses_sent =  ip_ptr -> nx_ip_arp_responses_sent;
    }

    /* Determine if ARP responses received is wanted.  */
    if (arp_responses_received)
    {

        /* Return the number of ARP responses received by this IP instance.  */
        *arp_responses_received =  ip_ptr -> nx_ip_arp_responses_received;
    }

    /* Determine if ARP dynamic entries is wanted.  */
    if (arp_dynamic_entries)
    {

        /* Return the number of ARP dynamic entries in this IP instance.  */
        *arp_dynamic_entries =  ip_ptr -> nx_ip_arp_dynamic_active_count;
    }

    /* Determine if ARP static entries is wanted.  */
    if (arp_static_entries)
    {

        /* Return the number of ARP static entries in this IP instance.  */
        *arp_static_entries =  ip_ptr -> nx_ip_arp_static_entries;
    }

    /* Determine if ARP aged entries is wanted.  */
    if (arp_aged_entries)
    {

        /* Return the number of ARP aged entries by this IP instance.  */
        *arp_aged_entries =  ip_ptr -> nx_ip_arp_aged_entries;
    }

    /* Determine if ARP invalid messages is wanted.  */
    if (arp_invalid_messages)
    {

        /* Return the number of ARP invalid messages handled by this IP instance.  */
        *arp_invalid_messages =  ip_ptr -> nx_ip_arp_invalid_messages;
    }

    /* Release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return status to the caller.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(arp_requests_sent);
    NX_PARAMETER_NOT_USED(arp_requests_received);
    NX_PARAMETER_NOT_USED(arp_responses_sent);
    NX_PARAMETER_NOT_USED(arp_responses_received);
    NX_PARAMETER_NOT_USED(arp_dynamic_entries);
    NX_PARAMETER_NOT_USED(arp_static_entries);
    NX_PARAMETER_NOT_USED(arp_aged_entries);
    NX_PARAMETER_NOT_USED(arp_invalid_messages);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

