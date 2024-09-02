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
/*    _nx_ip_gateway_address_set                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the IP gateway address that will be used for     */
/*    sending IP packets with addresses not in the local network.         */
/*                                                                        */
/*    Note 1: An input gateway address of zero is handled as an error.    */
/*    Use nx_ip_gateway_address_clear to remove an existing gateway.      */
/*                                                                        */
/*    Note 2: For a gateway address is non zero, the IP gateway address   */
/*    and gateway interface pointer must be non null, or this function    */
/*    will return an error status.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    ip_address                            Gateway IP address            */
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
UINT  _nx_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address)
{

#ifndef NX_DISABLE_IPV4
INT           i;
TX_INTERRUPT_SAVE_AREA

NX_INTERFACE *ip_interface_ptr = NX_NULL;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_GATEWAY_ADDRESS_SET, ip_ptr, ip_address, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain the IP internal mutex so the Gateway IP address can be setup.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Loop through all the interfaces to find the one for the input gateway address. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Must be a valid interface. Match the network subnet of the interface and input address. */
        if ((ip_ptr -> nx_ip_interface[i].nx_interface_valid) &&
            ((ip_address & (ip_ptr -> nx_ip_interface[i].nx_interface_ip_network_mask)) ==
             ip_ptr -> nx_ip_interface[i].nx_interface_ip_network))
        {

            /* This is the interface for the gateway.  */
            ip_interface_ptr = &(ip_ptr -> nx_ip_interface[i]);

            /* Break out of the search. */
            break;
        }
    }

    /* Check if we found an interface. */
    if (ip_interface_ptr == NX_NULL)
    {

        /* None found. Unlock the mutex, and return the error status. */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_IP_ADDRESS_ERROR);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Setup the Gateway IP address.  */
    ip_ptr -> nx_ip_gateway_address =  ip_address;

    ip_ptr -> nx_ip_gateway_interface = ip_interface_ptr;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release the protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return completion status.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

