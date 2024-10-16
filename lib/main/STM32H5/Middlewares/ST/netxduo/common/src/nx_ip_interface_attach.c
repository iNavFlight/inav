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
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_interface_attach                             PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attaches a physical network interface to the IP       */
/*    instance, invokes the associated device driver to initialize and    */
/*    enable the device.                                                  */
/*                                                                        */
/*    If this function is called before the IP thread starts running,     */
/*    the deivce initialization is executed as part of the device         */
/*    initialization during IP thread start up phase.  If this function   */
/*    is called after the IP thread has been started, the device          */
/*    initialization is executed as part of this function call.           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr_value                          Pointer to IP control block   */
/*    interface_name                        Name of this interface        */
/*    ip_address                            Interface IP Address, in host */
/*                                            byte order                  */
/*    network_mask                          Network Mask, in host byte    */
/*                                             order                      */
/*    ip_link_driver                        User supplied link driver     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    (ip_link_driver)                      User supplied link driver     */
/*    _nx_ipv6_multicast_join               Join IPv6 multicast group     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT _nx_ip_interface_attach(NX_IP *ip_ptr, CHAR *interface_name, ULONG ip_address, ULONG network_mask, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *))
{

INT           i;
NX_INTERFACE *nx_interface = NX_NULL;
NX_IP_DRIVER  driver_request;
UINT          status = NX_SUCCESS;

#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ROUTER_SOLICITATION)
ULONG         address[4];
#endif

#ifdef NX_DISABLE_IPV4
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(network_mask);
#else
    /* Perform duplicate address detection.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if ((ip_ptr -> nx_ip_interface[i].nx_interface_ip_address == ip_address) &&
            (ip_address != 0))
        {

            /* The IPv4 address already exists.  */
            return(NX_DUPLICATED_ENTRY);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        nx_interface = &(ip_ptr -> nx_ip_interface[i]);

        if (!(nx_interface -> nx_interface_valid))
        {
            /* Find a valid entry. */
            break;
        }
    }

    if (i == NX_MAX_PHYSICAL_INTERFACES)
    {
        /* No more free entry.  return. */
        return(NX_NO_MORE_ENTRIES);
    }

    /* Obtain the IP internal mutex before calling the driver.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Mark the entry as valid. */
    nx_interface -> nx_interface_valid = NX_TRUE;

    /* Fill in the interface information. */
#ifndef NX_DISABLE_IPV4
    nx_interface -> nx_interface_ip_address        = ip_address;
    nx_interface -> nx_interface_ip_network_mask   = network_mask;
    nx_interface -> nx_interface_ip_network        = ip_address & network_mask;
#endif /* !NX_DISABLE_IPV4  */
    nx_interface -> nx_interface_link_driver_entry = ip_link_driver;
    nx_interface -> nx_interface_name              = interface_name;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_INTERFACE_ATTACH, ip_ptr, ip_address, i, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* If the IP thread is already running, this service needs to go through the rest of the initializeation process. */
    if (ip_ptr -> nx_ip_initialize_done == NX_TRUE)
    {

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        /* Clear capability flag.  */
        ip_ptr -> nx_ip_interface[i].nx_interface_capability_flag = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

        /* First attach the interface to the device. */
        driver_request.nx_ip_driver_ptr       =  ip_ptr;
        driver_request.nx_ip_driver_command   =  NX_LINK_INTERFACE_ATTACH;
        driver_request.nx_ip_driver_interface = &(ip_ptr -> nx_ip_interface[i]);
        (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
        if (ip_ptr -> nx_ip_interface[i].nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD)
        {

            /* Set checksum capability for TCP/IP offload interface.  */
            ip_ptr -> nx_ip_interface[i].nx_interface_capability_flag |= NX_INTERFACE_CAPABILITY_CHECKSUM_ALL;
        }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

        /* Call the link driver to initialize the hardware. Among other
           responsibilities, the driver is required to provide the
           Maximum Transfer Unit (MTU) for the physical layer. The MTU
           should represent the actual physical layer transfer size
           less the physical layer headers and trailers.  */
        driver_request.nx_ip_driver_ptr =      ip_ptr;
        driver_request.nx_ip_driver_command =  NX_LINK_INITIALIZE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_INITIALIZE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /*
           When an IP instance is created, the first interface (nx_ip_interface[0]) is configured using parameters
           provided in the IP create call.

           When IP thread runs, it invokes the 1st interface link driver for link initialization.
         */
        (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);



        /* Call the link driver again to enable the interface.  */
        driver_request.nx_ip_driver_ptr =      ip_ptr;
        driver_request.nx_ip_driver_command =  NX_LINK_ENABLE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_LINK_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);

        /* Also configure the all-node multicast address. */
#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ROUTER_SOLICITATION)
        /* Create the all-node multicast group address, */
        address[0] = 0xFF020000;
        address[1] = 0;
        address[2] = 0;
        address[3] = 1;


        /* Join all-node multicast group. */
        status = _nx_ipv6_multicast_join(ip_ptr, address, &ip_ptr -> nx_ip_interface[i]);

#endif
    }

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));


    /* All done.  Return. */
    return(status);
}

