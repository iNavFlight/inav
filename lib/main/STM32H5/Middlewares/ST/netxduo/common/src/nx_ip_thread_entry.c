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
#include "nx_icmp.h"
#include "nx_igmp.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#include "nx_icmpv6.h"
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* FEATURE_IPSEC_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_thread_entry                                 PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the entry point for each IP's helper thread.  The  */
/*    IP helper thread is responsible for periodic ARP requests,          */
/*    reassembling fragmented IP messages, and helping with TCP           */
/*    protocol.                                                           */
/*                                                                        */
/*    Note that the priority of this function is determined by the IP     */
/*    create service.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr_value                          Pointer to IP control block   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_get                    Suspend on event flags that   */
/*                                            are used to signal this     */
/*                                            thread what to do           */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    (nx_ip_driver_deferred_packet_handler)Optional deferred packet      */
/*                                            processing routine          */
/*    _nx_ip_packet_receive                 IP receive packet processing  */
/*    _nx_ipv6_multicast_join               Join IPv6 multicast group     */
/*    (nx_arp_queue_process)                ARP receive queue processing  */
/*    (nx_ip_arp_periodic_update)           ARP periodic update processing*/
/*    (nx_ip_rarp_periodic_update)          RARP periodic processing      */
/*    (nx_ip_fragment_assembly)             IP fragment processing        */
/*    (nx_ip_fragment_timeout_check)        Fragment timeout checking     */
/*    (nx_ip_icmp_queue_process)            ICMP message queue processing */
/*    (nx_ip_igmp_queue_process)            IGMP message queue processing */
/*    (nx_ip_igmp_periodic_processing)      IGMP periodic processing      */
/*    (nx_ip_tcp_queue_process)             TCP message queue processing  */
/*    (nx_ip_tcp_periodic_processing)       TCP periodic processing       */
/*    (nx_tcp_deferred_cleanup_check)       TCP deferred cleanup check    */
/*    _nx_ipsec_sa_lifetime_tick            IPsec lifetime tick update    */
/*    (ip_link_driver)                      User supplied link driver     */
/*    _nx_ipsec_hw_packet_process           IPsec HW packet process       */
/*    _nx_icmpv6_send_ns                    Send Neighbor Solicitation    */
/*                                            message                     */
/*    (nx_nd_cache_fast_periodic_update)    ND Cache fast periodic service*/
/*                                            routine                     */
/*    (nx_nd_cache_slow_periodic_update)    ND Cache slow periodic service*/
/*                                            routine                     */
/*    _nxd_ipv6_prefix_router_timer_tick    IPv6 Preifx service routine   */
/*    _nxd_ipv6_router_solicitation_check   IPv6 RS service routine       */
/*    (nx_destination_table_periodic_update)                              */
/*                                          Destination table service     */
/*                                            routine.                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    ThreadX Scheduler                                                   */
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
VOID  _nx_ip_thread_entry(ULONG ip_ptr_value)
{

TX_INTERRUPT_SAVE_AREA

NX_IP_DRIVER      driver_request;
NX_IP            *ip_ptr;
ULONG             ip_events;
NX_PACKET        *packet_ptr;
UINT              i;
UINT              index;
ULONG             foo;
#ifdef FEATURE_NX_IPV6
NXD_IPV6_ADDRESS *interface_ipv6_address;
#endif /* FEATURE_NX_IPV6 */


    /* Setup IP pointer.  */
    NX_THREAD_EXTENSION_PTR_GET(ip_ptr, NX_IP, ip_ptr_value)

    /* Obtain the IP internal mutex before calling the driver.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Set the IP initialization done flag to true.  */
    ip_ptr -> nx_ip_initialize_done =  NX_TRUE;

    /* Loop through all physical interfaces to initialize and enable the hardware. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Is this a valid interface with a link driver associated with it? */
        if ((ip_ptr -> nx_ip_interface[i].nx_interface_valid) && (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry))
        {

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
            /* Clear capability flag first.  */
            ip_ptr -> nx_ip_interface[i].nx_interface_capability_flag = 0;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */


            ip_ptr -> nx_ip_interface[i].nx_interface_link_up = NX_TRUE;

            /* Yes; attach the interface to the device. */
            driver_request.nx_ip_driver_ptr        =  ip_ptr;
            driver_request.nx_ip_driver_command    =  NX_LINK_INTERFACE_ATTACH;
            driver_request.nx_ip_driver_interface  = &(ip_ptr -> nx_ip_interface[i]);
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

               When IP thread runs, it invokes the first interface link driver for link initialization.
             */
            (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);

            /* Call the link driver again to enable the interface.  */
            driver_request.nx_ip_driver_ptr =      ip_ptr;
            driver_request.nx_ip_driver_command =  NX_LINK_ENABLE;

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_LINK_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);

#ifdef FEATURE_NX_IPV6
            /* For ever IPv6 address on this interface, set the Solicitated Multicast address. */
            interface_ipv6_address = ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head;

            while (interface_ipv6_address)
            {
            ULONG multicast_address[4];

                SET_SOLICITED_NODE_MULTICAST_ADDRESS(multicast_address, interface_ipv6_address -> nxd_ipv6_address);
                _nx_ipv6_multicast_join(ip_ptr, multicast_address, &ip_ptr -> nx_ip_interface[i]);
                interface_ipv6_address = interface_ipv6_address -> nxd_ipv6_address_next;
            }
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
            if (ip_ptr -> nx_ipv6_packet_receive)
            {
            ULONG address[4];

                /* Create the all-node multicast group address, */
                address[0] = 0xFF020000;
                address[1] = 0;
                address[2] = 0;
                address[3] = 1;


                /* Join all-node multicast group. */
                _nx_ipv6_multicast_join(ip_ptr, address, &ip_ptr -> nx_ip_interface[i]);
            }
#endif
#endif
        }
    }

    /* Loop to process events for this IP instance.  */
    for (;;)
    {

        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Pickup IP event flags.  */
        tx_event_flags_get(&(ip_ptr -> nx_ip_events), NX_IP_ALL_EVENTS, TX_OR_CLEAR, &ip_events, TX_WAIT_FOREVER);

        /* Obtain the IP internal mutex before processing the IP event.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifdef NX_DRIVER_DEFERRED_PROCESSING
        /* Check for any packets deferred by the Driver.  */
        /*lint -e{644} suppress variable might not be initialized, since "ip_events" was initialized in tx_event_flags_get. */
        if (ip_events & NX_IP_DRIVER_PACKET_EVENT)
        {

            /* Loop to process all deferred packet requests.  */
            while (ip_ptr -> nx_ip_driver_deferred_packet_head)
            {
                /* Remove the first packet and process it!  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Pickup the first packet.  */
                packet_ptr =  ip_ptr -> nx_ip_driver_deferred_packet_head;

                /* Move the head pointer to the next packet.  */
                ip_ptr -> nx_ip_driver_deferred_packet_head =  packet_ptr -> nx_packet_queue_next;

                /* Check for end of deferred processing queue.  */
                if (ip_ptr -> nx_ip_driver_deferred_packet_head == NX_NULL)
                {

                    /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                    ip_ptr -> nx_ip_driver_deferred_packet_tail =  NX_NULL;
                }

                /* Restore interrupts.  */
                TX_RESTORE

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* make sure that there is a deferred processing function */
                if (ip_ptr ->  nx_ip_driver_deferred_packet_handler)
                {
                    /* Call the actual Deferred packet processing function.  */
                    (ip_ptr ->  nx_ip_driver_deferred_packet_handler)(ip_ptr, packet_ptr);
                }
            }

            /* Determine if there is anything else to do in the loop.  */
            ip_events =  ip_events & ~(NX_IP_DRIVER_PACKET_EVENT);
            if (!ip_events)
            {
                continue;
            }
        }
#endif

        /* Check for an IP receive packet event.  */
        /*lint -e{644} suppress variable might not be initialized, since "ip_events" was initialized by tx_event_flags_get. */
        if (ip_events & NX_IP_RECEIVE_EVENT)
        {

            /* Loop to process all deferred packet requests.  */
            while (ip_ptr -> nx_ip_deferred_received_packet_head)
            {

                /* Remove the first packet and process it!  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Pickup the first packet.  */
                packet_ptr =  ip_ptr -> nx_ip_deferred_received_packet_head;

                /* Move the head pointer to the next packet.  */
                ip_ptr -> nx_ip_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;

                /* Check for end of deferred processing queue.  */
                if (ip_ptr -> nx_ip_deferred_received_packet_head == NX_NULL)
                {

                    /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                    ip_ptr -> nx_ip_deferred_received_packet_tail =  NX_NULL;
                }

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call the actual IP packet receive function.  */
                _nx_ip_packet_receive(ip_ptr, packet_ptr);
            }

            /* Determine if there is anything else to do in the loop.  */
            ip_events =  ip_events & ~(NX_IP_RECEIVE_EVENT);
            if (!ip_events)
            {
                continue;
            }
        }

        /* Check for a TCP message event.  */
        if (ip_events & NX_IP_TCP_EVENT)
        {

            /* Process the TCP packet queue.  */
            (ip_ptr -> nx_ip_tcp_queue_process)(ip_ptr);

            /* Determine if there is anything else to do in the loop.  */
            ip_events =  ip_events & ~(NX_IP_TCP_EVENT);
            if (!ip_events)
            {
                continue;
            }
        }

        /* Check for a fast TCP event.  */
        if (ip_events & NX_IP_FAST_EVENT)
        {

            /* Start DAD for the link local address by sending off the first solicitation immediately
               while subsequent solicitations will occur on the next slow event. */
#ifdef FEATURE_NX_IPV6
#ifndef NX_DISABLE_IPV6_DAD

            if (ip_ptr -> nx_ip_icmpv6_packet_process)
            {

                /* Proceed with DAD check only if ICMPv6 is enabled. */
                for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
                {

                    interface_ipv6_address = ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head;

                    if (interface_ipv6_address &&
                        interface_ipv6_address -> nxd_ipv6_address_DupAddrDetectTransmit == NX_IPV6_DAD_TRANSMITS)
                    {


                        /* No. This address is still under DAD.  Transmit a NS */
                        /* Note that the 2nd last parameter sendUnicast is set to Zero. In this case
                           the last arg NDCacheEntry is not being used in _nx_icmpv6_send_ns. */
                        _nx_icmpv6_send_ns(ip_ptr,
                                           interface_ipv6_address -> nxd_ipv6_address,
                                           0, interface_ipv6_address, 0, &ip_ptr -> nx_ipv6_nd_cache[0]);

                        interface_ipv6_address -> nxd_ipv6_address_DupAddrDetectTransmit--;
                    }
                }
            }
#endif

            if (ip_ptr -> nx_nd_cache_fast_periodic_update)
            {
                /* Run the ND Cache update routine.  This is a 100 millisecond timer */
                ip_ptr -> nx_nd_cache_fast_periodic_update(ip_ptr);
            }

#endif /* FEATURE_NX_IPV6 */

            /* Process the fast TCP processing.  */
            if (ip_ptr -> nx_ip_tcp_fast_periodic_processing)
            {
                (ip_ptr -> nx_ip_tcp_fast_periodic_processing)(ip_ptr);
            }

            /* Determine if there is anything else to do in the loop.  */
            ip_events =  ip_events & ~(NX_IP_FAST_EVENT);
            if (!ip_events)
            {
                continue;
            }
        }

        /* Check for a periodic events.  */
        if (ip_events & NX_IP_PERIODIC_EVENT)
        {

#ifndef NX_DISABLE_IPV4
            /* Process the ARP periodic update, if ARP has been enabled.  */
            if (ip_ptr -> nx_ip_arp_periodic_update)
            {
                (ip_ptr -> nx_ip_arp_periodic_update)(ip_ptr);
            }

            /* Process the RARP periodic update, if RARP has been enabled.  */
            if (ip_ptr -> nx_ip_rarp_periodic_update)
            {
                (ip_ptr -> nx_ip_rarp_periodic_update)(ip_ptr);
            }

            /* Process IGMP periodic events, if IGMP has been enabled.  */
            if (ip_ptr -> nx_ip_igmp_periodic_processing)
            {
                (ip_ptr -> nx_ip_igmp_periodic_processing)(ip_ptr);
            }
#endif /* !NX_DISABLE_IPV4  */

            /* Process IP fragmentation timeouts, if IP fragmenting has been
               enabled.  */
            if (ip_ptr -> nx_ip_fragment_timeout_check)
            {
                (ip_ptr -> nx_ip_fragment_timeout_check)(ip_ptr);
            }

            /* Process TCP periodic events, if TCP has been enabled.  */
            if (ip_ptr -> nx_ip_tcp_periodic_processing)
            {
                (ip_ptr -> nx_ip_tcp_periodic_processing)(ip_ptr);
            }
#ifdef FEATURE_NX_IPV6
            /* Process IPv6 events, such as DAD... */
#ifndef NX_DISABLE_IPV6_DAD
            if (ip_ptr -> nx_ip_icmpv6_packet_process)
            {
                _nx_icmpv6_perform_DAD(ip_ptr);
            }
#endif /* NX_DISABLE_IPV6_DAD */
            if (ip_ptr -> nx_nd_cache_slow_periodic_update)
            {
                /* Run the ND Cache update routine.  This is a 1 second timer */
                ip_ptr -> nx_nd_cache_slow_periodic_update(ip_ptr);
            }

            _nxd_ipv6_prefix_router_timer_tick(ip_ptr);
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
            _nxd_ipv6_router_solicitation_check(ip_ptr);
#endif

#ifdef NX_IPSEC_ENABLE
            _nx_ipsec_sa_lifetime_tick(ip_ptr);
#endif /* NX_IPSEC_ENABLE */

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

            if (ip_ptr -> nx_destination_table_periodic_update)
            {

                /* Run the Destination table update routine.  This will update the timers
                   on destination table entries' MTU data, and if expired will initiate
                   MTU path discovery.  */
                ip_ptr -> nx_destination_table_periodic_update(ip_ptr);
            }
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

#endif /* FEATURE_NX_IPV6 */
            /* Determine if there is anything else to do in the loop.  */
            ip_events =  ip_events & ~(NX_IP_PERIODIC_EVENT);
            if (!ip_events)
            {
                continue;
            }
        }

#ifdef NX_IPSEC_ENABLE
        if (ip_events & NX_IP_HW_DONE_EVENT)
        {

            /* Process the hw_done_packet queue.  */
            _nx_ipsec_hw_packet_process(ip_ptr);
        }
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
        /* Check for an ARP receive packet event.  */
        if ((ip_events & NX_IP_ARP_REC_EVENT) && (ip_ptr -> nx_ip_arp_queue_process))
        {

            /* Process the ARP queue.  */
            (ip_ptr -> nx_ip_arp_queue_process)(ip_ptr);
        }

        /* Check for an RARP receive packet event.  */
        if ((ip_events & NX_IP_RARP_REC_EVENT) && (ip_ptr -> nx_ip_rarp_queue_process))
        {

            /* Process the RARP queue.  */
            (ip_ptr -> nx_ip_rarp_queue_process)(ip_ptr);
        }

        /* Check for an IGMP message event.  */
        if (ip_events & NX_IP_IGMP_EVENT)
        {

            /* Process the ICMP packet queue.  */
            (ip_ptr -> nx_ip_igmp_queue_process)(ip_ptr);
        }

        /* Check for an IGMP enable event.  */
        if (ip_events & NX_IP_IGMP_ENABLE_EVENT)
        {

            /* Call the associated driver for this IP instance to register the "all hosts"
               multicast address.  */
            for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
            {
                /* Enable the hardware for IGMP for all valid interfaces. */
                if (ip_ptr -> nx_ip_interface[i].nx_interface_valid)
                {
                    driver_request.nx_ip_driver_ptr =                    ip_ptr;
                    driver_request.nx_ip_driver_command =                NX_LINK_MULTICAST_JOIN;
                    driver_request.nx_ip_driver_physical_address_msw =   NX_IP_MULTICAST_UPPER;
                    /*lint -e{835} -e{845} suppress operating on zero. */
                    driver_request.nx_ip_driver_physical_address_lsw =   NX_IP_MULTICAST_LOWER | (NX_ALL_HOSTS_ADDRESS & NX_IP_MULTICAST_MASK);
                    driver_request.nx_ip_driver_interface            =   &(ip_ptr -> nx_ip_interface[i]);

                    /* If trace is enabled, insert this event into the trace buffer.  */
                    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_MULTICAST_JOIN, ip_ptr, 0, 0, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                    (ip_ptr -> nx_ip_interface[i].nx_interface_link_driver_entry)(&driver_request);
                }
            }
        }
#endif /* !NX_DISABLE_IPV4  */

        /* Check for an IP unfragment event.  */
        if (ip_events & NX_IP_UNFRAG_EVENT)
        {

            /* Process the IP fragment reassemble, if fragment has been enabled.  */
            if (ip_ptr -> nx_ip_fragment_assembly)
            {
                (ip_ptr -> nx_ip_fragment_assembly)(ip_ptr);
            }
        }

#ifndef NX_DISABLE_IPV4
        /* Check for an ICMP message event.  */
        if (ip_events & NX_IP_ICMP_EVENT)
        {

            /* Process the ICMP packet queue.  */
            (ip_ptr -> nx_ip_icmp_queue_process)(ip_ptr);
        }
#endif /* NX_DISABLE_IPV4 */

        /* Check for a deferred processing request from the driver.  */
        if (ip_events & NX_IP_DRIVER_DEFERRED_EVENT)
        {

            /* Go through each valid interface. */
            for (index = 0; index < NX_MAX_PHYSICAL_INTERFACES; index++)
            {
                if (ip_ptr -> nx_ip_interface[index].nx_interface_valid)
                {

                    /* Yes, there is a deferred processing event from the driver. The only valid information
                       fields are the IP pointer and the command.  */
                    driver_request.nx_ip_driver_ptr =        ip_ptr;
                    driver_request.nx_ip_driver_command =    NX_LINK_DEFERRED_PROCESSING;
                    driver_request.nx_ip_driver_interface  = &(ip_ptr -> nx_ip_interface[index]);
                    driver_request.nx_ip_driver_return_ptr = &foo;

                    (ip_ptr -> nx_ip_interface[index].nx_interface_link_driver_entry)(&driver_request);
                }
            }
        }

        /* Check for a deferred TCP cleanup processing request from the driver.  */
        if (ip_events & NX_IP_TCP_CLEANUP_DEFERRED)
        {

            /* Yes, there is a deferred cleanup processing event. Call the TCP deferred cleanup
               processing function.  */
            (ip_ptr -> nx_tcp_deferred_cleanup_check)(ip_ptr);
        }

        /* Check for a link status change request from the driver.  */
        if (ip_events & NX_IP_LINK_STATUS_EVENT)
        {

            /* Yes, there is a link status change  event. Call the deferred link status processing function. */
            _nx_ip_deferred_link_status_process(ip_ptr);
        }
    }
}

