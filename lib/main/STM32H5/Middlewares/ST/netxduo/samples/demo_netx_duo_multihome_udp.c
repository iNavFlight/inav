

/* This is a small demo of the high-performance NetX Duo TCP/IP
   IPv4/IPv6 dual stack.  This demo concentrates on UDP datagram
   send/receive in IPv4 and IPv6 network in a simulated Ethernet
   driver in a multihome environment.  */


/*
   IP_0 has two simulated phyiscal interfaces.
   The primary interface is 1.2.3.4/255.255.255.0
   the secondary interface is 2.2.3.4/255.255.255.0

   IP_1 has two simulated physical interface.
   The primary interface is 1.2.3.5/255.255.255.0
   the secondary interface is 2.2.3.5/255.255.255.0

   Each interface has an IPv6 link local address and
   an IPv6 global address.  The following table lists the IPv6
   configuration:

   IP_0 primary interface Link Local Address:  FE80::
   IP_0 primary interface Global Address:

   IP_0 secondary interface Link Local Address:  FE80::
   IP_0 secondary interface Global Address:

   IP_1 primary interface Link Local Address:  FE80::
   IP_1 primary interface Global Address:

   IP_1 secondary interface Link Local Address:  FE80::
   IP_1 secondary interface Global Address:





   These four simulated interfaces are connected to the same channel.



       ---------  Primary                         Primary   ---------
       |       |  Interface                       Interface |       |
       | IP_0  |----------------------     |----------------| IP_1  |
       |       | 1.2.3.4             |     |       1.2.3.5  |       |
       |       | 2001::4             |     |       2001::5  |       |
       |       |                     |     |                |       |
       |       |                     |     |                |       |
       |       |-------------------  |     |  --------------|       |
       |       | Secondary        |  |     |  |   Secondary |       |
       --------- Interface        |  |     |  |   Interface ---------
                 2.2.3.4          |  |     |  |     2.2.3.5
                 2002::4          |  |     |  |     2002::5
                               ------------------
                               |                |
                               |   Switch Box   |
                               |                |
                               ------------------


*/

#include   "tx_api.h"
#include   "nx_api.h"

#if (NX_MAX_PHYSICAL_INTERFACES > 1)

#define     DEMO_STACK_SIZE 2048
#define     DEMO_DATA       "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
#define     PACKET_SIZE     1536
#define     POOL_SIZE       ((sizeof(NX_PACKET) + PACKET_SIZE) * 16)

/* Define the ThreadX and NetX object control blocks...  */

TX_THREAD               thread_0;
TX_THREAD               thread_1;

NX_PACKET_POOL          pool_0;
NX_IP                   ip_0;
NX_IP                   ip_1;

NX_UDP_SOCKET           socket_0;
NX_UDP_SOCKET           socket_1;
UCHAR                   pool_buffer[POOL_SIZE];



/* Define the counters used in the demo application...  */

ULONG                   thread_0_counter;
ULONG                   thread_1_counter;
ULONG                   error_counter;


/* Define thread prototypes.  */

void thread_0_entry(ULONG thread_input);
void thread_1_entry(ULONG thread_input);

void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


#define PRIMARY_INTERFACE   0
#define SECONDARY_INTERFACE 1

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS ip1_primary_ipv4_address;
NXD_ADDRESS ip1_secondary_ipv4_address;
#endif /* NX_DISABLE_IPV4 */

#ifndef NX_DISABLE_IPV6

UINT ip0_primary_linklocal_address_index;
UINT ip0_primary_global_address_index;
UINT ip0_secondary_linklocal_address_index;
UINT ip0_secondary_global_address_index;

UINT ip1_primary_linklocal_address_index;
UINT ip1_primary_global_address_index;
UINT ip1_secondary_linklocal_address_index;
UINT ip1_secondary_global_address_index;


NXD_ADDRESS ip1_primary_linklocal_address;
NXD_ADDRESS ip1_primary_global_address;
NXD_ADDRESS ip1_secondary_linklocal_address;
NXD_ADDRESS ip1_secondary_global_address;
#endif /* NX_DISABLE_IPV6 */

#ifndef NX_DISABLE_IPV4

#ifndef NX_DISABLE_IPV6
/*
   Define the number of UDP tests.  If IPv6 is enabled, there are 6 tests:
   (1) UDP packet tranmisssion to the IPv4 address of the primary interface on ip_1;
   (2) UDP packet transmission to the IPv4 address of the secondary interface on ip_1;
   (3) UDP packet transmission to the IPv6 link local address of the primary interface on ip_1;
   (4) UDP packet transmission to the IPv6 link local address of the secondary interface on ip_1;
   (5) UDP packet transmission to the IPv6 global address of the primary interface on ip_1;
   (6) UDP packet transmission to the IPv6 global address of the secondary interface on ip_1;
*/
#define NUMBER_OF_TESTS 8


#else  /* !NX_DISABLE_IPV6 */
/*
   Define the number of UDP tests.  If IPv6 is enabled, there are 6 tests:
   (1) UDP packet tranmisssion to the IPv4 address of the primary interface on ip_1;
   (2) UDP packet transmission to the IPv4 address of the secondary interface on ip_1;

*/
#define NUMBER_OF_TESTS 2
#endif /* !NX_DISABLE_IPV6 */

#else  /* !NX_DISABLE_IPV4 */
/*
   Define the number of UDP tests.  If IPv6 is enabled, there are 6 tests:
   (1) UDP packet tranmisssion to the IPv4 address of the primary interface on ip_1;
   (2) UDP packet transmission to the IPv4 address of the secondary interface on ip_1;
   (3) UDP packet transmission to the IPv6 link local address of the primary interface on ip_1;
   (4) UDP packet transmission to the IPv6 link local address of the secondary interface on ip_1;
   (5) UDP packet transmission to the IPv6 global address of the primary interface on ip_1;
   (6) UDP packet transmission to the IPv6 global address of the secondary interface on ip_1;
*/
#define NUMBER_OF_TESTS 6
#endif /* !NX_DISABLE_IPV4 */

/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

CHAR *pointer;
UINT  status;


    /* Setup the working pointer.  */
    pointer =  (CHAR *)first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the main thread.  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;


    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", PACKET_SIZE, pool_buffer, POOL_SIZE);

    if (status)
    {
        error_counter++;
    }

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Create another IP instance.  */
    status += nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                           pointer, 2048, 1);
    pointer =  pointer + 2048;

    if (status)
    {
        error_counter++;
    }

#ifndef NX_DISABLE_IPV4
    /* Set the server IP address to ip_1 primary IPv4 address. */
    ip1_primary_ipv4_address.nxd_ip_version = NX_IP_VERSION_V4;
    ip1_primary_ipv4_address.nxd_ip_address.v4 = IP_ADDRESS(1, 2, 3, 5);
#endif /* NX_DISABLE_IPV4 */


    /* Attach the second interface to IP_0. Note that this interface is attached during initialization time.
       Alternatively the second interface may also be attached in thread context, as illustrated below
       in thead_0_entry function. */
    status = nx_ip_interface_attach(&ip_0, "IP_0 Secondary Interface", IP_ADDRESS(2, 2, 3, 4), 0xFFFFFF00, _nx_ram_network_driver);

    if (status)
    {
        error_counter++;
    }

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&ip_0, (void *)pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status +=  nx_arp_enable(&ip_1, (void *)pointer, 1024);
    pointer = pointer + 1024;

    /* Check ARP enable status.  */
    if (status)
    {
        error_counter++;
    }
#endif /* NX_DISABLE_IPV4 */
#ifndef NX_DISABLE_IPV6

    /* Enable IPv6 */
    status = nxd_ipv6_enable(&ip_0);
    if (status)
    {
        error_counter++;
    }

    status = nxd_ipv6_enable(&ip_1);
    if (status)
    {
        error_counter++;
    }
#endif /* !NX_DISABLE_IPV6 */

    /* Enable ICMP */
    status = nxd_icmp_enable(&ip_0);
    if (status)
    {
        error_counter++;
    }

    status = nxd_icmp_enable(&ip_1);
    if (status)
    {
        error_counter++;
    }


    /* Enable UDP processing for both IP instances.  */
    status =  nx_udp_enable(&ip_0);
    status += nx_udp_enable(&ip_1);

    if (status)
    {
        error_counter++;
    }
}



/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

UINT       status;
NX_PACKET *my_packet;


#ifndef NX_DISABLE_IPV6
NXD_ADDRESS       ip_address;
NXD_IPV6_ADDRESS *server_ipv6_ptr;
#endif /* !NX_DISABLE_IPV6 */

    NX_PARAMETER_NOT_USED(thread_input);

    /*
       At this point, IP_0 already has two interfaces attached during the system initialization phase.
       Here is a demonstration of attaching a secondary interface to an IP instance (ip_1 in this case)
       after the IP instance has been created and already started.
     */

    /*
       Attch the second interface to IP_1.  Note that this interface is attached in thread context.
       Alternatively the second interface may also be attached during system initilization, as illustrated
       above in tx_application_define.
     */

    /* Attach the 2nd interface to IP_1 */
    status = nx_ip_interface_attach(&ip_1, "IP_1 Secondary Interface", IP_ADDRESS(2, 2, 3, 5), 0xFFFFFF00, _nx_ram_network_driver);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }


#ifndef NX_DISABLE_IPV4
    /* Set the server IP address to ip_1 primary IPv4 address. */
    ip1_secondary_ipv4_address.nxd_ip_version = NX_IP_VERSION_V4;
    ip1_secondary_ipv4_address.nxd_ip_address.v4 = IP_ADDRESS(2, 2, 3, 5);
#endif /* NX_DISABLE_IPV4 */

#ifndef NX_DISABLE_IPV6


    /* Set ip_0 primary link local address. */
    status = nxd_ipv6_address_set(&ip_0, PRIMARY_INTERFACE, NX_NULL, 10, &ip0_primary_linklocal_address_index);
    if (status)
    {
        error_counter++;
    }

    /* Set ip_1 primary link local address. */
    status = nxd_ipv6_address_set(&ip_1, PRIMARY_INTERFACE, NX_NULL, 10, &ip1_primary_linklocal_address_index);
    if (status)
    {
        error_counter++;
    }



    /* Set ip_0 primary interface global address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 4;

    status = nxd_ipv6_address_set(&ip_0, PRIMARY_INTERFACE, &ip_address, 64, &ip0_primary_global_address_index);

    if (status)
    {
        error_counter++;
    }

    /* Set ip_1 primary interface global address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 5;

    status = nxd_ipv6_address_set(&ip_1, PRIMARY_INTERFACE, &ip_address, 64, &ip1_primary_global_address_index);

    if (status)
    {
        error_counter++;
    }



    /* Set ip_0 secondary link local address. */
    status = nxd_ipv6_address_set(&ip_0, SECONDARY_INTERFACE, NX_NULL, 10, &ip0_secondary_linklocal_address_index);
    if (status)
    {
        error_counter++;
    }

    /* Set ip_1 secondary link local address. */
    status = nxd_ipv6_address_set(&ip_1, SECONDARY_INTERFACE, NX_NULL, 10, &ip1_secondary_linklocal_address_index);
    if (status)
    {
        error_counter++;
    }



    /* Set ip_0 secondary interface global address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20020000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 4;

    status = nxd_ipv6_address_set(&ip_0, SECONDARY_INTERFACE, &ip_address, 64, &ip0_secondary_global_address_index);

    if (status)
    {
        error_counter++;
    }

    /* Set ip_1 primary interface global address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20020000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 5;

    status = nxd_ipv6_address_set(&ip_1, SECONDARY_INTERFACE, &ip_address, 64, &ip1_secondary_global_address_index);

    if (status)
    {
        error_counter++;
    }

    /* Set the server IP address to ip_1 primary IPv6 link local address. */
    server_ipv6_ptr = &ip_1.nx_ipv6_address[ip1_primary_linklocal_address_index];
    ip1_primary_linklocal_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip1_primary_linklocal_address.nxd_ip_address.v6[0] = server_ipv6_ptr -> nxd_ipv6_address[0];
    ip1_primary_linklocal_address.nxd_ip_address.v6[1] = server_ipv6_ptr -> nxd_ipv6_address[1];
    ip1_primary_linklocal_address.nxd_ip_address.v6[2] = server_ipv6_ptr -> nxd_ipv6_address[2];
    ip1_primary_linklocal_address.nxd_ip_address.v6[3] = server_ipv6_ptr -> nxd_ipv6_address[3];

    /* Set the server IP address to ip_1 primary IPv6 global address. */
    server_ipv6_ptr = &ip_1.nx_ipv6_address[ip1_primary_global_address_index];
    ip1_primary_global_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip1_primary_global_address.nxd_ip_address.v6[0] = server_ipv6_ptr -> nxd_ipv6_address[0];
    ip1_primary_global_address.nxd_ip_address.v6[1] = server_ipv6_ptr -> nxd_ipv6_address[1];
    ip1_primary_global_address.nxd_ip_address.v6[2] = server_ipv6_ptr -> nxd_ipv6_address[2];
    ip1_primary_global_address.nxd_ip_address.v6[3] = server_ipv6_ptr -> nxd_ipv6_address[3];

    /* Set the server IP address to ip_1 secondary IPv6 link local address. */
    server_ipv6_ptr = &ip_1.nx_ipv6_address[ip1_secondary_linklocal_address_index];
    ip1_secondary_linklocal_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip1_secondary_linklocal_address.nxd_ip_address.v6[0] = server_ipv6_ptr -> nxd_ipv6_address[0];
    ip1_secondary_linklocal_address.nxd_ip_address.v6[1] = server_ipv6_ptr -> nxd_ipv6_address[1];
    ip1_secondary_linklocal_address.nxd_ip_address.v6[2] = server_ipv6_ptr -> nxd_ipv6_address[2];
    ip1_secondary_linklocal_address.nxd_ip_address.v6[3] = server_ipv6_ptr -> nxd_ipv6_address[3];

    /* Set the server IP address to ip_1 secondary IPv6 global address. */
    server_ipv6_ptr = &ip_1.nx_ipv6_address[ip1_secondary_global_address_index];
    ip1_secondary_global_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip1_secondary_global_address.nxd_ip_address.v6[0] = server_ipv6_ptr -> nxd_ipv6_address[0];
    ip1_secondary_global_address.nxd_ip_address.v6[1] = server_ipv6_ptr -> nxd_ipv6_address[1];
    ip1_secondary_global_address.nxd_ip_address.v6[2] = server_ipv6_ptr -> nxd_ipv6_address[2];
    ip1_secondary_global_address.nxd_ip_address.v6[3] = server_ipv6_ptr -> nxd_ipv6_address[3];

#endif /* !NX_DISABLE_IPV6 */


    /* Wait 5 seconds for the IP thread to finish its initilization and
       for the IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_0, &socket_0, "Socket 0", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_0, 0x88, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }


    /* Loop to repeat things over and over again!  */
    while (1)
    {


        /* Allocate a packet.  */
        status =  nx_packet_allocate(&pool_0, &my_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Write ABCs into the packet payload!  */
        nx_packet_data_append(my_packet, DEMO_DATA, sizeof(DEMO_DATA), &pool_0, TX_WAIT_FOREVER);

        /* Attempt to send the data. */
        /* In this demo, we alternate between IPv4 connections and IPv6 connections. */
        switch (thread_0_counter & (NUMBER_OF_TESTS - 1))
        {
#ifndef NX_DISABLE_IPV4
        case 0:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_primary_ipv4_address, 0x89, PRIMARY_INTERFACE);
            break;
        case 1:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_secondary_ipv4_address, 0x89, SECONDARY_INTERFACE);
            break;
#ifndef NX_DISABLE_IPV6
        case 2:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_primary_linklocal_address, 0x89, ip0_primary_linklocal_address_index);
            break;
        case 3:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_secondary_linklocal_address, 0x89, ip0_secondary_linklocal_address_index);
            break;
        case 4:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_primary_global_address, 0x89, ip0_primary_global_address_index);
            break;
        case 5:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_secondary_global_address, 0x89, ip0_secondary_global_address_index);
            break;
        case 6:
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ip1_primary_global_address, 0x89);
            break;
        case 7:
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ip1_secondary_global_address, 0x89);
            break;
#endif /* !NX_DISABLE_IPV6 */
#else /* !NX_DISABLE_IPV4  */
#ifndef NX_DISABLE_IPV6
        case 0:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_primary_linklocal_address, 0x89, ip0_primary_linklocal_address_index);
            break;
        case 1:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_secondary_linklocal_address, 0x89, ip0_secondary_linklocal_address_index);
            break;
        case 2:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_primary_global_address, 0x89, ip0_primary_global_address_index);
            break;
        case 3:
            status =  nxd_udp_socket_source_send(&socket_0, my_packet, &ip1_secondary_global_address, 0x89, ip0_secondary_global_address_index);
            break;
        case 4:
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ip1_primary_global_address, 0x89);
            break;
        case 5:
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ip1_secondary_global_address, 0x89);
            break;
#endif /* !NX_DISABLE_IPV6 */
#endif /* NX_DISABLE_IPV4 */
        default:
            break;
        }
        /* Check for error.  */
        if (status)
        {
            nx_packet_release(my_packet);
            error_counter++;
        }

        /* Increment thread 0's counter.  */
        thread_0_counter++;



        /* Relinquish to thread 1.  */
        tx_thread_relinquish();
    }
}


void    thread_1_entry(ULONG thread_input)
{

UINT       status;
NX_PACKET *my_packet;
ULONG      actual_status;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Wait 5 seconds for the IP thread to finish its initilization and
       for the IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);


    /* Ensure the IP instance has been initialized.  */
    status =  nx_ip_status_check(&ip_1, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if (status != NX_SUCCESS)
    {

        error_counter++;
    }



    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_1, &socket_1, "Socket 1", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_1, 0x89, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Loop to create and establish server connections.  */
    while (1)
    {


        /* Receive a UDP packet.  */
        status =  nx_udp_socket_receive(&socket_1, &my_packet, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Release the packet.  */
        status =  nx_packet_release(my_packet);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Increment thread 1's counter.  */
        thread_1_counter++;
    }
}
#endif
