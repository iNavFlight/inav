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
/**   RAM Network (RAM)                                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#include "nx_api.h"


/* Define the Link MTU. Note this is not the same as the IP MTU.  The Link MTU
   includes the addition of the Physical Network header (usually Ethernet). This
   should be larger than the IP instance MTU by the size of the physical header. */
#define NX_LINK_MTU      1514


/* Define Ethernet address format.  This is prepended to the incoming IP
   and ARP/RARP messages.  The frame beginning is 14 bytes, but for speed
   purposes, we are going to assume there are 16 bytes free in front of the
   prepend pointer and that the prepend pointer is 32-bit aligned.

    Byte Offset     Size            Meaning

        0           6           Destination Ethernet Address
        6           6           Source Ethernet Address
        12          2           Ethernet Frame Type, where:

                                        0x0800 -> IP Datagram
                                        0x0806 -> ARP Request/Reply
                                        0x0835 -> RARP request reply

        42          18          Padding on ARP and RARP messages only.  */

#define NX_ETHERNET_IP   0x0800
#define NX_ETHERNET_ARP  0x0806
#define NX_ETHERNET_RARP 0x8035
#define NX_ETHERNET_IPV6 0x86DD
#define NX_ETHERNET_SIZE 14

/* For the simulated ethernet driver, physical addresses are allocated starting
   at the preset value and then incremented before the next allocation.  */

ULONG   simulated_address_msw =  0x0011;
ULONG   simulated_address_lsw =  0x22334456;


/* Define driver prototypes.  */

VOID    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);
void    _nx_ram_network_driver_output(NX_PACKET *packet_ptr, UINT interface_instance_id);
void    _nx_ram_network_driver_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT interface_instance_id);

#define NX_MAX_RAM_INTERFACES             4
#define NX_RAM_DRIVER_MAX_MCAST_ADDRESSES 3
typedef struct MAC_ADDRESS_STRUCT
{
    ULONG nx_mac_address_msw;
    ULONG nx_mac_address_lsw;
} MAC_ADDRESS;


/* Define an application-specific data structure that holds internal
   data (such as the state information) of a device driver.

   The example below applies to the simulated RAM driver.
   User shall replace its content with information related to
   the actual driver being used. */
typedef struct _nx_ram_network_driver_instance_type
{
    UINT          nx_ram_network_driver_in_use;

    UINT          nx_ram_network_driver_id;

    NX_INTERFACE *nx_ram_driver_interface_ptr;

    NX_IP        *nx_ram_driver_ip_ptr;

    MAC_ADDRESS   nx_ram_driver_mac_address;

    MAC_ADDRESS   nx_ram_driver_mcast_address[NX_RAM_DRIVER_MAX_MCAST_ADDRESSES];
} _nx_ram_network_driver_instance_type;


/* In this example, there are four instances of the simulated RAM driver.
   Therefore an array of four driver instances are created to keep track of
   the interface information of each driver. */
static _nx_ram_network_driver_instance_type nx_ram_driver[NX_MAX_RAM_INTERFACES];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ram_network_driver                              PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function acts as a virtual network for testing the NetX source */
/*    and driver concepts.   User application may use this routine as     */
/*    a template for the actual network driver.  Note that this driver    */
/*    simulates Ethernet operation.  Some of the parameters don't apply   */
/*    for non-Ethernet interfaces.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP protocol block  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ram_network_driver_output         Send physical packet out      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX IP processing                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            added sample of returning   */
/*                                            link's interface type,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr)
{
UINT          i = 0;
NX_IP        *ip_ptr;
NX_PACKET    *packet_ptr;
ULONG        *ethernet_frame_ptr;
NX_INTERFACE *interface_ptr;
UINT          interface_index;

    /* Setup the IP pointer from the driver request.  */
    ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;

    /* Default to successful return.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;

    /* Setup interface pointer.  */
    interface_ptr = driver_req_ptr -> nx_ip_driver_interface;

    /* Obtain the index number of the network interface. */
    interface_index = interface_ptr -> nx_interface_index;

    /* Find out the driver interface if the driver command is not ATTACH. */
    if (driver_req_ptr -> nx_ip_driver_command != NX_LINK_INTERFACE_ATTACH)
    {
        for (i = 0; i < NX_MAX_RAM_INTERFACES; i++)
        {
            if (nx_ram_driver[i].nx_ram_network_driver_in_use == 0)
            {
                continue;
            }

            if (nx_ram_driver[i].nx_ram_driver_ip_ptr != ip_ptr)
            {
                continue;
            }

            if (nx_ram_driver[i].nx_ram_driver_interface_ptr == driver_req_ptr -> nx_ip_driver_interface)
            {
                break;
            }
        }

        if (i == NX_MAX_RAM_INTERFACES)
        {
            driver_req_ptr -> nx_ip_driver_status =  NX_INVALID_INTERFACE;
            return;
        }
    }


    /* Process according to the driver request type in the IP control
       block.  */
    switch (driver_req_ptr -> nx_ip_driver_command)
    {

    case NX_LINK_INTERFACE_ATTACH:
    {

        /* Find an available driver instance to attach the interface. */
        for (i = 0; i < NX_MAX_RAM_INTERFACES; i++)
        {
            if (nx_ram_driver[i].nx_ram_network_driver_in_use == 0)
            {
                break;
            }
        }
        /* An available entry is found. */
        if (i < NX_MAX_RAM_INTERFACES)
        {
            /* Set the IN USE flag.*/
            nx_ram_driver[i].nx_ram_network_driver_in_use  = 1;

            nx_ram_driver[i].nx_ram_network_driver_id = i;

            /* Record the interface attached to the IP instance. */
            nx_ram_driver[i].nx_ram_driver_interface_ptr = driver_req_ptr -> nx_ip_driver_interface;

            /* Record the IP instance. */
            nx_ram_driver[i].nx_ram_driver_ip_ptr = ip_ptr;

            nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_msw = simulated_address_msw;
            nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_lsw = simulated_address_lsw + i;
        }
        else
        {
            driver_req_ptr -> nx_ip_driver_status =  NX_INVALID_INTERFACE;
        }

        break;
    }

    case NX_LINK_INTERFACE_DETACH:
    {

        /* Zero out the driver instance. */
        memset(&(nx_ram_driver[i]), 0, sizeof(_nx_ram_network_driver_instance_type));

        break;
    }

    case NX_LINK_INITIALIZE:
    {

        /* Device driver shall initialize the Ethernet Controller here. */

#ifdef NX_DEBUG
        printf("NetX RAM Driver Initialization - %s\n", ip_ptr -> nx_ip_name);
        printf("  IP Address =%08X\n", ip_ptr -> nx_ip_address);
#endif

        /* Once the Ethernet controller is initialized, the driver needs to
           configure the NetX Interface Control block, as outlined below. */

        /* The nx_interface_ip_mtu_size should be the MTU for the IP payload.
           For regular Ethernet, the IP MTU is 1500. */
        nx_ip_interface_mtu_set(ip_ptr, interface_index, (NX_LINK_MTU - NX_ETHERNET_SIZE));

        /* Set the physical address (MAC address) of this IP instance.  */
        /* For this simulated RAM driver, the MAC address is constructed by
           incrementing a base lsw value, to simulate multiple nodes on the
           ethernet.  */
        nx_ip_interface_physical_address_set(ip_ptr, interface_index,
                                             nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_msw,
                                             nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_lsw, NX_FALSE);

        /* Indicate to the IP software that IP to physical mapping is required.  */
        nx_ip_interface_address_mapping_configure(ip_ptr, interface_index, NX_TRUE);

        break;
    }

    case NX_LINK_UNINITIALIZE:
    {

        /* Zero out the driver instance. */
        memset(&(nx_ram_driver[i]), 0, sizeof(_nx_ram_network_driver_instance_type));

        break;
    }

    case NX_LINK_ENABLE:
    {

        /* Process driver link enable.  An Ethernet driver shall enable the
           transmit and reception logic.  Once the IP stack issues the
           LINK_ENABLE command, the stack may start transmitting IP packets. */


        /* In the RAM driver, just set the enabled flag.  */
        interface_ptr -> nx_interface_link_up =  NX_TRUE;

#ifdef NX_DEBUG
        printf("NetX RAM Driver Link Enabled - %s\n", ip_ptr -> nx_ip_name);
#endif
        break;
    }

    case NX_LINK_DISABLE:
    {

        /* Process driver link disable.  This command indicates the IP layer
           is not going to transmit any IP datagrams, nor does it expect any
           IP datagrams from the interface.  Therefore after processing this command,
           the device driver shall not send any incoming packets to the IP
           layer.  Optionally the device driver may turn off the interface. */

        /* In the RAM driver, just clear the enabled flag.  */
        interface_ptr -> nx_interface_link_up =  NX_FALSE;

#ifdef NX_DEBUG
        printf("NetX RAM Driver Link Disabled - %s\n", ip_ptr -> nx_ip_name);
#endif
        break;
    }

    case NX_LINK_PACKET_SEND:
    case NX_LINK_PACKET_BROADCAST:
    case NX_LINK_ARP_SEND:
    case NX_LINK_ARP_RESPONSE_SEND:
    case NX_LINK_RARP_SEND:
    {

        /*
           The IP stack sends down a data packet for transmission.
           The device driver needs to prepend a MAC header, and fill in the
           Ethernet frame type (assuming Ethernet protocol for network transmission)
           based on the type of packet being transmitted.

           The following sequence illustrates this process.
         */


        /* Place the ethernet frame at the front of the packet.  */
        packet_ptr =  driver_req_ptr -> nx_ip_driver_packet;

        /* Adjust the prepend pointer.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - NX_ETHERNET_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + NX_ETHERNET_SIZE;

        /* Setup the ethernet frame pointer to build the ethernet frame.  Backup another 2
           bytes to get 32-bit word alignment.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ethernet_frame_ptr =  (ULONG *)(packet_ptr -> nx_packet_prepend_ptr - 2);

        /* Build the ethernet frame.  */
        *ethernet_frame_ptr     =  driver_req_ptr -> nx_ip_driver_physical_address_msw;
        *(ethernet_frame_ptr + 1) =  driver_req_ptr -> nx_ip_driver_physical_address_lsw;
        *(ethernet_frame_ptr + 2) =  (interface_ptr -> nx_interface_physical_address_msw << 16) |
            (interface_ptr -> nx_interface_physical_address_lsw >> 16);
        *(ethernet_frame_ptr + 3) =  (interface_ptr -> nx_interface_physical_address_lsw << 16);

        if (driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_SEND)
        {
            *(ethernet_frame_ptr + 3) |= NX_ETHERNET_ARP;
        }
        else if (driver_req_ptr -> nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND)
        {
            *(ethernet_frame_ptr + 3) |= NX_ETHERNET_ARP;
        }
        else if (driver_req_ptr -> nx_ip_driver_command == NX_LINK_RARP_SEND)
        {
            *(ethernet_frame_ptr + 3) |= NX_ETHERNET_RARP;
        }
        else if (packet_ptr -> nx_packet_ip_version == 4)
        {
            *(ethernet_frame_ptr + 3) |= NX_ETHERNET_IP;
        }
        else
        {
            *(ethernet_frame_ptr + 3) |= NX_ETHERNET_IPV6;
        }


        /* Endian swapping if NX_LITTLE_ENDIAN is defined.  */
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
        NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));
#ifdef NX_DEBUG_PACKET
        printf("NetX RAM Driver Packet Send - %s\n", ip_ptr -> nx_ip_name);
#endif

        /* At this point, the packet is a complete Ethernet frame, ready to be transmitted.
           The driver shall call the actual Ethernet transmit routine and put the packet
           on the wire.

           In this example, the simulated RAM network transmit routine is called. */
        _nx_ram_network_driver_output(packet_ptr, i);
        break;
    }


    case NX_LINK_MULTICAST_JOIN:
    {
    UINT mcast_index;

        /* The IP layer issues this command to join a multicast group.  Note that
           multicast operation is required for IPv6.

           On a typically Ethernet controller, the driver computes a hash value based
           on MAC address, and programs the hash table.

           It is likely the driver also needs to maintain an internal MAC address table.
           Later if a multicast address is removed, the driver needs
           to reprogram the hash table based on the remaining multicast MAC addresses. */


        /* The following procedure only applies to our simulated RAM network driver, which manages
           multicast MAC addresses by a simple look up table. */
        for (mcast_index = 0; mcast_index < NX_RAM_DRIVER_MAX_MCAST_ADDRESSES; mcast_index++)
        {
            if (nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_msw == 0 &&
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_lsw == 0)
            {
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_msw = driver_req_ptr -> nx_ip_driver_physical_address_msw;
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_lsw = driver_req_ptr -> nx_ip_driver_physical_address_lsw;
                break;
            }
        }
        if (mcast_index == NX_RAM_DRIVER_MAX_MCAST_ADDRESSES)
        {
            driver_req_ptr -> nx_ip_driver_status =  NX_NO_MORE_ENTRIES;
        }

        break;
    }


    case NX_LINK_MULTICAST_LEAVE:
    {

    UINT mcast_index;

        /* The IP layer issues this command to remove a multicast MAC address from the
           receiving list.  A device driver shall properly remove the multicast address
           from the hash table, so the hardware does not receive such traffic.  Note that
           in order to reprogram the hash table, the device driver may have to keep track of
           current active multicast MAC addresses. */

        /* The following procedure only applies to our simulated RAM network driver, which manages
           multicast MAC addresses by a simple look up table. */
        for (mcast_index = 0; mcast_index < NX_RAM_DRIVER_MAX_MCAST_ADDRESSES; mcast_index++)
        {
            if (nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_msw == driver_req_ptr -> nx_ip_driver_physical_address_msw &&
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_lsw == driver_req_ptr -> nx_ip_driver_physical_address_lsw)
            {
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_msw = 0;
                nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_lsw = 0;
                break;
            }
        }
        if (mcast_index == NX_RAM_DRIVER_MAX_MCAST_ADDRESSES)
        {
            driver_req_ptr -> nx_ip_driver_status =  NX_ENTRY_NOT_FOUND;
        }

        break;
    }

    case NX_LINK_GET_STATUS:
    {

        /* Return the link status in the supplied return pointer.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) =  ip_ptr -> nx_ip_interface[0].nx_interface_link_up;
        break;
    }

    case NX_LINK_GET_SPEED:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_DUPLEX_TYPE:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_ERROR_COUNT:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_RX_COUNT:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_TX_COUNT:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_ALLOC_ERRORS:
    {

        /* Return the link's line speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_LINK_GET_INTERFACE_TYPE:
    {

        /* Return the link's interface type in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = NX_INTERFACE_TYPE_UNKNOWN;
        break;
    }

    case NX_LINK_DEFERRED_PROCESSING:
    {

        /* Driver defined deferred processing. This is typically used to defer interrupt
           processing to the thread level.

           A typical use case of this command is:
           On receiving an Ethernet frame, the RX ISR does not process the received frame,
           but instead records such an event in its internal data structure, and issues
           a notification to the IP stack (the driver sends the notification to the IP
           helping thread by calling "_nx_ip_driver_deferred_processing()".  When the IP stack
           gets a notification of a pending driver deferred process, it calls the
           driver with the NX_LINK_DEFERRED_PROCESSING command.  The driver shall complete
           the pending receive process.
         */

        /* The simulated RAM driver doesn't require a deferred process so it breaks out of
           the switch case. */


        break;
    }

    case NX_LINK_SET_PHYSICAL_ADDRESS:
    {

        /* Find an driver instance to attach the interface. */
        for (i = 0; i < NX_MAX_RAM_INTERFACES; i++)
        {
            if (nx_ram_driver[i].nx_ram_driver_interface_ptr == driver_req_ptr -> nx_ip_driver_interface)
            {
                break;
            }
        }

        /* An available entry is found. */
        if (i < NX_MAX_RAM_INTERFACES)
        {

            /* Set the physical address.  */
            nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_msw = driver_req_ptr -> nx_ip_driver_physical_address_msw;
            nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_lsw = driver_req_ptr -> nx_ip_driver_physical_address_lsw;
        }
        else
        {
            driver_req_ptr -> nx_ip_driver_status =  NX_INVALID_INTERFACE;
        }

        break;
    }

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    case NX_INTERFACE_CAPABILITY_GET:
    {

        /* Return the capability of the Ethernet controller speed in the supplied return pointer. Unsupported feature.  */
        *(driver_req_ptr -> nx_ip_driver_return_ptr) = 0;
        break;
    }

    case NX_INTERFACE_CAPABILITY_SET:
    {

        /* Set the capability of the Ethernet controller. Unsupported feature.  */
        break;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY  */

    default:

        /* Invalid driver request.  */

        /* Return the unhandled command status.  */
        driver_req_ptr -> nx_ip_driver_status =  NX_UNHANDLED_COMMAND;

#ifdef NX_DEBUG
        printf("NetX RAM Driver Received invalid request - %s\n", ip_ptr -> nx_ip_name);
#endif
        break;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ram_network_driver_output                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function simply sends the packet to the IP instance on the     */
/*    created IP list that matches the physical destination specified in  */
/*    the Ethernet packet.  In a real hardware setting, this routine      */
/*    would simply put the packet out on the wire.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Packet pointer                */
/*    interface_instance_id                 ID of driver instance         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_copy                        Copy a packet                 */
/*    nx_packet_transmit_release            Release a packet              */
/*    _nx_ram_network_driver_receive        RAM driver receive processing */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX IP processing                                                  */
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
void  _nx_ram_network_driver_output(NX_PACKET *packet_ptr, UINT interface_instance_id)
{

NX_IP     *next_ip;
NX_PACKET *packet_copy;
ULONG      destination_address_msw;
ULONG      destination_address_lsw;
UINT       old_threshold = 0;
UINT       i;
UINT       mcast_index;

#ifdef NX_DEBUG_PACKET
UCHAR *ptr;
UINT   j;

    ptr =  packet_ptr -> nx_packet_prepend_ptr;
    printf("Ethernet Packet: ");
    for (j = 0; j < 6; j++)
    {
        printf("%02X", *ptr++);
    }
    printf(" ");
    for (j = 0; j < 6; j++)
    {
        printf("%02X", *ptr++);
    }
    printf(" %02X", *ptr++);
    printf("%02X ", *ptr++);

    i = 0;
    for (j = 0; j < (packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE); j++)
    {
        printf("%02X", *ptr++);
        i++;
        if (i > 3)
        {
            i = 0;
            printf(" ");
        }
    }
    printf("\n");


#endif

    /* Pickup the destination IP address from the packet_ptr.  */
    destination_address_msw =  (ULONG)*(packet_ptr -> nx_packet_prepend_ptr);
    destination_address_msw =  (destination_address_msw << 8) | (ULONG)*(packet_ptr -> nx_packet_prepend_ptr + 1);
    destination_address_lsw =  (ULONG)*(packet_ptr -> nx_packet_prepend_ptr + 2);
    destination_address_lsw =  (destination_address_lsw << 8) | (ULONG)*(packet_ptr -> nx_packet_prepend_ptr + 3);
    destination_address_lsw =  (destination_address_lsw << 8) | (ULONG)*(packet_ptr -> nx_packet_prepend_ptr + 4);
    destination_address_lsw =  (destination_address_lsw << 8) | (ULONG)*(packet_ptr -> nx_packet_prepend_ptr + 5);


    /* Disable preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &old_threshold);

    for (i = 0; i < NX_MAX_RAM_INTERFACES; i++)
    {

        /* Skip the interface from which the packet was sent. */
        if (i == interface_instance_id)
        {
            continue;
        }

        /* Skip the instance that has not been initialized. */
        if (nx_ram_driver[i].nx_ram_network_driver_in_use == 0)
        {
            continue;
        }

        /* Set the next IP instance.  */
        next_ip = nx_ram_driver[i].nx_ram_driver_ip_ptr;

        /* If the destination MAC address is broadcast or the destination matches the interface MAC,
           accept the packet. */
        if (((destination_address_msw == ((ULONG)0x0000FFFF)) && (destination_address_lsw == ((ULONG)0xFFFFFFFF))) ||   /* Broadcast match */
            ((destination_address_msw == nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_msw) &&
             (destination_address_lsw == nx_ram_driver[i].nx_ram_driver_mac_address.nx_mac_address_lsw)) ||
            (destination_address_msw == ((ULONG)0x00003333)) ||
            ((destination_address_msw == 0) && (destination_address_lsw == 0)))
        {

            /* Make a copy of packet for the forwarding.  */
            if (nx_packet_copy(packet_ptr, &packet_copy, next_ip -> nx_ip_default_packet_pool, NX_NO_WAIT))
            {

                /* Remove the Ethernet header.  */
                packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

                /* Adjust the packet length.  */
                packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

                /* Error, no point in continuing, just release the packet.  */
                nx_packet_transmit_release(packet_ptr);
                return;
            }

            /*lint -e{644} suppress variable might not be initialized, since "packet_copy" was initialized in nx_packet_copy. */
            _nx_ram_network_driver_receive(next_ip, packet_copy, i);
        }
        else
        {
            for (mcast_index = 0; mcast_index < NX_RAM_DRIVER_MAX_MCAST_ADDRESSES; mcast_index++)
            {

                if (destination_address_msw == nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_msw &&
                    destination_address_lsw == nx_ram_driver[i].nx_ram_driver_mcast_address[mcast_index].nx_mac_address_lsw)
                {

                    /* Make a copy of packet for the forwarding.  */
                    if (nx_packet_copy(packet_ptr, &packet_copy, next_ip -> nx_ip_default_packet_pool, NX_NO_WAIT))
                    {

                        /* Remove the Ethernet header.  */
                        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

                        /* Adjust the packet length.  */
                        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

                        /* Error, no point in continuing, just release the packet.  */
                        nx_packet_transmit_release(packet_ptr);
                        return;
                    }

                    _nx_ram_network_driver_receive(next_ip, packet_copy, i);
                }
            }
        }
    }

    /* Remove the Ethernet header.  In real hardware environments, this is typically
       done after a transmit complete interrupt.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

    /* Now that the Ethernet frame has been removed, release the packet.  */
    nx_packet_transmit_release(packet_ptr);

    /* Restore preemption.  */
    /*lint -e{644} suppress variable might not be initialized, since "old_threshold" was initialized in previous tx_thread_preemption_change. */
    tx_thread_preemption_change(tx_thread_identify(), old_threshold, &old_threshold);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ram_network_driver_receive                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processing incoming packets.  In the RAM network      */
/*    driver, the incoming packets are coming from the RAM driver output  */
/*    routine.  In real hardware settings, this routine would be called   */
/*    from the receive packet ISR.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP protocol block  */
/*    packet_ptr                            Packet pointer                */
/*    interface_instance_id                 The interface ID the packet is*/
/*                                            destined for                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_receive                 IP receive packet processing  */
/*    _nx_ip_packet_deferred_receive        IP deferred receive packet    */
/*                                            processing                  */
/*    _nx_arp_packet_deferred_receive       ARP receive processing        */
/*    _nx_rarp_packet_deferred_receive      RARP receive processing       */
/*    nx_packet_release                     Packet release                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX IP processing                                                  */
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
void  _nx_ram_network_driver_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT interface_instance_id)
{

UINT packet_type;

    /* Pickup the packet header to determine where the packet needs to be
       sent.  */
    packet_type =  (((UINT)(*(packet_ptr -> nx_packet_prepend_ptr + 12))) << 8) |
        ((UINT)(*(packet_ptr -> nx_packet_prepend_ptr + 13)));


    /* Setup interface pointer.  */
    packet_ptr -> nx_packet_address.nx_packet_interface_ptr = nx_ram_driver[interface_instance_id].nx_ram_driver_interface_ptr;


    /* Route the incoming packet according to its ethernet type.  */
    /* The RAM driver accepts both IPv4 and IPv6 frames. */
    if ((packet_type == NX_ETHERNET_IP) || (packet_type == NX_ETHERNET_IPV6))
    {

        /* Note:  The length reported by some Ethernet hardware includes bytes after the packet
           as well as the Ethernet header.  In some cases, the actual packet length after the
           Ethernet header should be derived from the length in the IP header (lower 16 bits of
           the first 32-bit word).  */

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

        /* Route to the ip receive function.  */
#ifdef NX_DEBUG_PACKET
        printf("NetX RAM Driver IP Packet Receive - %s\n", ip_ptr -> nx_ip_name);
#endif

#ifdef NX_DIRECT_ISR_CALL
        _nx_ip_packet_receive(ip_ptr, packet_ptr);
#else
        _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
#endif
    }
#ifndef NX_DISABLE_IPV4
    else if (packet_type == NX_ETHERNET_ARP)
    {

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

        /* Route to the ARP receive function.  */
#ifdef NX_DEBUG
        printf("NetX RAM Driver ARP Receive - %s\n", ip_ptr -> nx_ip_name);
#endif
        _nx_arp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
    else if (packet_type == NX_ETHERNET_RARP)
    {

        /* Clean off the Ethernet header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + NX_ETHERNET_SIZE;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - NX_ETHERNET_SIZE;

        /* Route to the RARP receive function.  */
#ifdef NX_DEBUG
        printf("NetX RAM Driver RARP Receive - %s\n", ip_ptr -> nx_ip_name);
#endif
        _nx_rarp_packet_deferred_receive(ip_ptr, packet_ptr);
    }
#endif /* !NX_DISABLE_IPV4  */
    else
    {

        /* Invalid ethernet header... release the packet.  */
        nx_packet_release(packet_ptr);
    }
}

