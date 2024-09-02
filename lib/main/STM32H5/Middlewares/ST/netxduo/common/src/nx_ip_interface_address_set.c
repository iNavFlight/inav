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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_interface_address_set                        PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the IP address and the network mask for the      */
/*    supplied IP instance.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       IP Interface Index            */
/*    ip_address                            IP address                    */
/*    network_mask                          Network mask                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_interface_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG network_mask)
{

#ifndef NX_DISABLE_IPV4
TX_INTERRUPT_SAVE_AREA

VOID  (*address_change_notify)(NX_IP *, VOID *);
VOID *additional_info;
VOID  (*address_change_notify_internal)(NX_IP *, VOID *);
ULONG previous_ip_address;
ULONG previous_network_mask;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_ADDRESS_SET, ip_ptr, ip_address, network_mask, 0, NX_TRACE_IP_EVENTS, 0, 0);


    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Save previous IP address and network mask.  */
    previous_ip_address =    ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address;
    previous_network_mask =  ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_network_mask;

    /* Pickup the current notification callback and additional information pointers.  */
    address_change_notify =  ip_ptr -> nx_ip_address_change_notify;
    additional_info =        ip_ptr -> nx_ip_address_change_notify_additional_info;

    /* Pickup the internal notification callback.  */
    address_change_notify_internal = ip_ptr -> nx_ip_address_change_notify_internal;

    /* Setup the IP address and the network mask. */
    ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_address      =  ip_address;
    ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_network_mask =  network_mask;
    ip_ptr -> nx_ip_interface[interface_index].nx_interface_ip_network      =  ip_address & network_mask;

    /* Ensure the RARP function is disabled.  */
    ip_ptr -> nx_ip_rarp_periodic_update =  NX_NULL;
    ip_ptr -> nx_ip_rarp_queue_process =    NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Determine if the application should be notified of the IP address and/or
       network mask change.  */
    if ((address_change_notify) &&
        ((ip_address != previous_ip_address) || (network_mask != previous_network_mask)))
    {

        /* Yes, call the application's IP address change notify function.  */
        (address_change_notify)(ip_ptr, additional_info);
    }

    /* Determine if the internal application should be notified of the IP address and/or
       network mask change.  */
    if ((address_change_notify_internal) &&
        ((ip_address != previous_ip_address) || (network_mask != previous_network_mask)))
    {

        /* Yes, call the application's IP address change notify function.  */
        (address_change_notify_internal)(ip_ptr, NX_NULL);
    }

    /* Initialize the ARP defend timeout.  */
    ip_ptr -> nx_ip_interface[interface_index].nx_interface_arp_defend_timeout = 0;

    /* Return completion status.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(network_mask);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

