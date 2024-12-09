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
#include "nx_system.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_create                                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an Internet Protocol instance, including      */
/*    setting up all appropriate data structures and calling the supplied */
/*    link driver for initialization of the physical interface.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    name                                  Name of this IP instance      */
/*    ip_address                            Internet address for this IP  */
/*    network_mask                          Network mask for IP address   */
/*    default_pool                          Default packet pool           */
/*    ip_link_driver                        User supplied IP link driver  */
/*    memory_ptr                            Pointer memory area for IP    */
/*    memory_size                           Size of IP memory area        */
/*    priority                              Priority of IP helper thread  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_event_flags_create                 Create IP event flags         */
/*    tx_event_flags_delete                 Delete IP event flags         */
/*    tx_mutex_create                       Create IP protection mutex    */
/*    tx_mutex_delete                       Delete IP protection mutex    */
/*    tx_thread_create                      Create IP helper thread       */
/*    tx_timer_create                       Create IP periodic timer      */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                    NX_PACKET_POOL *default_pool, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                    VOID *memory_ptr, ULONG memory_size, UINT priority)
{

TX_INTERRUPT_SAVE_AREA

NX_IP     *tail_ptr;
UINT       i;
UINT       old_threshold = 0;
TX_THREAD *current_thread;

#ifdef NX_DISABLE_IPV4
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(network_mask);
#endif /* NX_DISABLE_IPV4 */

    /* Reference the version ID and option words to ensure they are linked in.  */
    if (((ULONG)_nx_system_build_options_1 | (ULONG)_nx_system_build_options_2 | (ULONG)_nx_system_build_options_3 |
         (ULONG)_nx_system_build_options_4 | (ULONG)_nx_system_build_options_5 | (ULONG)_nx_version_id[0]) == 0)
    {

        /* We should never get here!  */
        return(NX_NOT_IMPLEMENTED);
    }

    /* Initialize the IP control block to zero.  */
    memset((void *)ip_ptr, 0, sizeof(NX_IP));

    /* Configure the primary interface. */
    ip_ptr -> nx_ip_interface[0].nx_interface_valid = 1;

#ifndef NX_DISABLE_IPV4
    /* Save the IP address.  */
    ip_ptr -> nx_ip_interface[0].nx_interface_ip_address =   ip_address;

    /* Save the network mask.  */
    ip_ptr -> nx_ip_interface[0].nx_interface_ip_network_mask =  network_mask;

    /* Derive the network bits of this IP address.  */
    ip_ptr -> nx_ip_interface[0].nx_interface_ip_network =  ip_address & network_mask;

    /* Initialize the ARP defend timeout.  */
    ip_ptr -> nx_ip_interface[0].nx_interface_arp_defend_timeout = 0;
#endif /* !NX_DISABLE_IPV4  */

    /* Setup the link driver address.  */
    ip_ptr -> nx_ip_interface[0].nx_interface_link_driver_entry =  ip_link_driver;

    /* Set the device interface name to "PRI". */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_ptr -> nx_ip_interface[0].nx_interface_name = (CHAR *)"PRI";

    /* Set index of each interface. */
    for (i = 0; i < NX_MAX_IP_INTERFACES; i++)
    {
        ip_ptr -> nx_ip_interface[i].nx_interface_index = (UCHAR)i;
    }


#ifndef NX_DISABLE_LOOPBACK_INTERFACE

    /* Set the Loopback interface name. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_name = (CHAR *)"Internal IP Loopback";


    /* Mark the loopback interface as valid. */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_valid = 1;

#ifndef NX_DISABLE_IPV4
    /* Set the loopback interface address. */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_ip_address = 0x7F000001;
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_ip_network_mask = 0xFF000000;
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_ip_network = 0x7F000000;
#endif /* !NX_DISABLE_IPV4  */

    /* Loopback interface is a special case. Therefore no dedicated link driver needed. */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_link_driver_entry = NX_NULL;

    /* Loopback interface does not need IP/MAC address mapping. */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_address_mapping_needed = 0;

    /* There is actually no MTU limit for the loopback interface. */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_ip_mtu_size = 65535;

    /* Mark the loopback interface as LINK UP */
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_link_up = 1;

    /* Set all the link capability. */
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nx_interface_capability_flag = (NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_ICMPV4_RX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM |
                                                                                     NX_INTERFACE_CAPABILITY_IGMP_RX_CHECKSUM);
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#endif /* !NX_DISABLE_LOOPBACK_INTERFACE */

    /* Save the supplied IP name.  */
    ip_ptr -> nx_ip_name =  name;

    /* Set the initial IP packet ID.  */
    ip_ptr -> nx_ip_packet_id =  NX_INIT_PACKET_ID;

    /* Setup the default packet pool for this IP instance.  */
    ip_ptr -> nx_ip_default_packet_pool =  default_pool;

#ifdef NX_ENABLE_DUAL_PACKET_POOL
    /* Setup the auxiliary packet pool for this IP instance. By default it pointers to default pool. */
    ip_ptr -> nx_ip_auxiliary_packet_pool = default_pool;
#endif /* NX_ENABLE_DUAL_PACKET_POOL */

    /* Create the internal IP protection mutex.  */
    tx_mutex_create(&(ip_ptr -> nx_ip_protection), name, TX_NO_INHERIT);

    /* Create the internal IP event flag object.  */
    tx_event_flags_create(&(ip_ptr -> nx_ip_events), name);

    /* Pickup current thread pointer.  */
    current_thread =  tx_thread_identify();

    /* Disable preemption temporarily.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, priority, &old_threshold);
    }

    /* Create the internal IP thread for handling more processing intensive
       duties.  */
    /*lint -e{923} suppress cast of pointer to ULONG.  */
    tx_thread_create(&(ip_ptr -> nx_ip_thread), name, _nx_ip_thread_entry, (ULONG)(ALIGN_TYPE)(ip_ptr),
                     memory_ptr, memory_size, priority, priority, 1, TX_AUTO_START);

    NX_THREAD_EXTENSION_PTR_SET(&(ip_ptr -> nx_ip_thread), ip_ptr)

    /* Create the periodic timer for this IP instance.  */
    /*lint -e{923} suppress cast of pointer to ULONG.  */
    tx_timer_create(&(ip_ptr -> nx_ip_periodic_timer), name,
                    _nx_ip_periodic_timer_entry, (ULONG)(ALIGN_TYPE)ip_ptr,
                    NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_AUTO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(ip_ptr -> nx_ip_periodic_timer), ip_ptr)

    /* If trace is enabled, register this object.  */
    NX_TRACE_OBJECT_REGISTER(NX_TRACE_OBJECT_TYPE_IP, ip_ptr, name, memory_ptr, memory_size);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_CREATE, ip_ptr, ip_address, network_mask, default_pool, NX_TRACE_IP_EVENTS, 0, 0);

#ifndef NX_DISABLE_IPV4
    /* Install IPv4 packet receive processing function pointer */
    ip_ptr -> nx_ipv4_packet_receive = _nx_ipv4_packet_receive;
#endif

    /* Otherwise, the IP initialization was successful.  Place the
       IP control block on the list of created IP instances.  */
    TX_DISABLE

    /* Load the IP ID field in the IP control block.  */
    ip_ptr -> nx_ip_id =  NX_IP_ID;

    /* Place the new IP control block on the list of created IPs.  First,
       check for an empty list.  */
    if (_nx_ip_created_ptr)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  _nx_ip_created_ptr -> nx_ip_created_previous;

        /* Place the new IP control block in the list.  */
        _nx_ip_created_ptr -> nx_ip_created_previous =  ip_ptr;
        tail_ptr -> nx_ip_created_next =  ip_ptr;

        /* Setup this IP's created links.  */
        ip_ptr -> nx_ip_created_previous =  tail_ptr;
        ip_ptr -> nx_ip_created_next =      _nx_ip_created_ptr;
    }
    else
    {

        /* The created IP list is empty.  Add IP control block to empty list.  */
        _nx_ip_created_ptr =                ip_ptr;
        ip_ptr -> nx_ip_created_next =      ip_ptr;
        ip_ptr -> nx_ip_created_previous =  ip_ptr;
    }

    /* Increment the created IP counter.  */
    _nx_ip_created_count++;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Restore preemption.  */
    if (current_thread)
    {

        /*lint -e{644} suppress variable might not be initialized, since "old_threshold" was initialized in previous tx_thread_preemption_change call. */
        tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
    }

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}

