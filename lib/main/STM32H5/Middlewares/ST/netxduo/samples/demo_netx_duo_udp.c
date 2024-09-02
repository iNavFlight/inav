/* This is a small demo of the high-performance NetX Duo TCP/IP stack.  This program demonstrates
   ICMPv6 protocols Neighbor Discovery and Stateless Address Configuration for IPv6, ARP for IPv4, and
   UDP packet sending and receiving with a simulated Ethernet driver.  */

#include   "tx_api.h"
#include   "nx_api.h"

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

ULONG thread_0_counter;
ULONG thread_1_counter;
ULONG error_counter;

/* Define thread prototypes.  */

void thread_0_entry(ULONG thread_input);
void thread_1_entry(ULONG thread_input);
void _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


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

#ifndef NX_DISABLE_IPV6
NXD_ADDRESS ip_address;
#endif
    /* Setup the working pointer.  */
    pointer =  (CHAR *)first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* .  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", PACKET_SIZE, pool_buffer, POOL_SIZE);

    /* Check for pool creation error.  */
    if (status)
    {
        error_counter++;
    }

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(1, 2, 3, 4), 0xFFFFF000UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Create another IP instance.  */
    status += nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(1, 2, 3, 5), 0xFFFFF000UL, &pool_0, _nx_ram_network_driver,
                           pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Check for IP create errors.  */
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

    /* Check for ARP enable errors.  */
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
#endif

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

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&ip_0);
    status += nx_udp_enable(&ip_1);

    /* Check for UDP enable errors.  */
    if (status)
    {
        error_counter++;
    }
#ifndef NX_DISABLE_IPV6

    /* Set ip_0 global address address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 1;
    if (status)
    {
        error_counter++;
    }

    status = nxd_ipv6_global_address_set(&ip_0, &ip_address, 64);

    /* Check for errors */
    if (status)
    {
        error_counter++;
    }

    /* Set ip_1 global address address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 2;

    status = nxd_ipv6_global_address_set(&ip_1, &ip_address, 64);

    /* Check for errors */
    if (status)
    {
        error_counter++;
    }

    /* Note we are not setting the link local address. The ram driver
       does not use physical addresses so it is unnecessary. Consult the
       NetX Duo User Guide for setting the link local address. */
#endif
}



/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

UINT       status;
NX_PACKET *my_packet;

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS ipv4_address;
#endif /* NX_DISABLE_IPV4 */
#ifndef NX_DISABLE_IPV6
NXD_ADDRESS ipv6_address;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Let the IP threads and thread 1 execute.    */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifndef NX_DISABLE_IPV6

    /* Wait for IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
#endif

#ifndef NX_DISABLE_IPV4
    ipv4_address.nxd_ip_version = NX_IP_VERSION_V4;
    ipv4_address.nxd_ip_address.v4 = IP_ADDRESS(1, 2, 3, 5);
#endif /* NX_DISABLE_IPV4 */

#ifndef NX_DISABLE_IPV6

    ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;
    ipv6_address.nxd_ip_address.v6[0] = 0x20010000;
    ipv6_address.nxd_ip_address.v6[1] = 0;
    ipv6_address.nxd_ip_address.v6[2] = 0;
    ipv6_address.nxd_ip_address.v6[3] = 2;
#endif

    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_0, &socket_0, "Socket 0", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_0, 0x88, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Disable checksum logic for this socket.  */
    nx_udp_socket_checksum_disable(&socket_0);

    /* Setup the ARP entry for the UDP send.  */
    nx_arp_dynamic_entry_set(&ip_0, IP_ADDRESS(1, 2, 3, 5), 0, 0);

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

        /* Send the UDP packet.  */
#ifndef NX_DISABLE_IPV6

#ifndef NX_DISABLE_IPV4
        /* In this demo, we alternate between IPv4 connections and IPv6 connections. */
        if (thread_0_counter & 1)
        {
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ipv4_address, 0x89);
        }
        else
#endif /* NX_DISABLE_IPV4 */
        {
            status =  nxd_udp_socket_send(&socket_0, my_packet, &ipv6_address, 0x89);
        }
#else
        status =  nxd_udp_socket_send(&socket_0, my_packet, &ipv4_address, 0x89);
#endif

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            error_counter++;
            break;
        }

        /* Increment thread 0's counter.  */
        thread_0_counter++;
    }
}


void    thread_1_entry(ULONG thread_input)
{

UINT       status;
NX_PACKET *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    tx_thread_sleep(NX_IP_PERIODIC_RATE);
#ifndef NX_DISABLE_IPV6
    /* Wait 5 seconds for IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
#endif
    /* Create a UDP socket.  */
    status = nx_udp_socket_create(&ip_1, &socket_1, "Socket 1", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&socket_1, 0x89, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

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

