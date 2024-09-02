/* This is a small demo of the high-performance NetX Duo TCP/IP stack.
   This program demonstrates ICMPv6 protocols Neighbor Discovery and
   Stateless Address Configuration for IPv6, ARP for IPv4, and
   TCP packet sending and receiving with a simulated Ethernet driver.  */


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
NX_TCP_SOCKET           client_socket;
NX_TCP_SOCKET           server_socket;
UCHAR                   pool_buffer[POOL_SIZE];



/* Define the counters used in the demo application...  */

ULONG thread_0_counter;
ULONG thread_1_counter;
ULONG error_counter;


/* Define thread prototypes.  */

void thread_0_entry(ULONG thread_input);
void thread_1_entry(ULONG thread_input);
void thread_1_connect_received(NX_TCP_SOCKET *server_socket, UINT port);
void thread_1_disconnect_received(NX_TCP_SOCKET *server_socket);

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


    /* Enable TCP processing for both IP instances.  */
    status =  nx_tcp_enable(&ip_0);
    status += nx_tcp_enable(&ip_1);

#ifndef NX_DISABLE_IPV6
    /* Set ip_0 interface address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 1;

    status = nxd_ipv6_global_address_set(&ip_0, &ip_address, 64);
    if (status)
    {
        error_counter++;
    }

    /* Set ip_0 interface address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010000;
    ip_address.nxd_ip_address.v6[1] = 0;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 2;

    status = nxd_ipv6_global_address_set(&ip_1, &ip_address, 64);

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
ULONG      length;

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS server_ipv4_address;
#endif /* NX_DISABLE_IPV4 */
#ifndef NX_DISABLE_IPV6
NXD_ADDRESS server_ipv6_address;
#endif
NXD_ADDRESS peer_address;
ULONG       peer_port;

    NX_PARAMETER_NOT_USED(thread_input);

#ifndef NX_DISABLE_IPV6

    /* Wait 5 seconds for the IP thread to finish its initilization and
       for the IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#else
    /* Wait 1 second for the IP thread to finish its initilization. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);
#endif

#ifndef NX_DISABLE_IPV4
    /* set the TCP server addresses. */
    server_ipv4_address.nxd_ip_version = NX_IP_VERSION_V4;
    server_ipv4_address.nxd_ip_address.v4 = IP_ADDRESS(1, 2, 3, 5);
#endif /* NX_DISABLE_IPV4 */

#ifndef NX_DISABLE_IPV6

    server_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;
    server_ipv6_address.nxd_ip_address.v6[0] = 0x20010000;
    server_ipv6_address.nxd_ip_address.v6[1] = 0;
    server_ipv6_address.nxd_ip_address.v6[2] = 0;
    server_ipv6_address.nxd_ip_address.v6[3] = 2;
#endif
    /* Loop to repeat things over and over again!  */
    while (1)
    {

        /* Create a socket.  */
        status =  nx_tcp_socket_create(&ip_0, &client_socket, "Client Socket",
                                       NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 200,
                                       NX_NULL, NX_NULL);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Bind the socket.  */
        status =  nx_tcp_client_socket_bind(&client_socket, 12, NX_WAIT_FOREVER);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Attempt to connect the socket.  */
#ifndef NX_DISABLE_IPV6
#ifndef NX_DISABLE_IPV4
        /* In this demo, we alternate between IPv4 connections and IPv6 connections. */
        if (thread_0_counter & 1)
        {
            status = nxd_tcp_client_socket_connect(&client_socket, &server_ipv4_address, 12, NX_IP_PERIODIC_RATE);
        }
        else
#endif /* NX_DISABLE_IPV4 */
        {
            status = nxd_tcp_client_socket_connect(&client_socket, &server_ipv6_address, 12, NX_IP_PERIODIC_RATE);
        }
#else
        status = nxd_tcp_client_socket_connect(&client_socket, &server_ipv4_address, 12, NX_IP_PERIODIC_RATE);
#endif

        /* Check for error.  */
        if (status)
        {
            printf("Error with socket connect: 0x%x\n", status);
            return;
        }

        status = nxd_tcp_socket_peer_info_get(&client_socket, &peer_address, &peer_port);

        /* Allocate a packet.  */
        status =  nx_packet_allocate(&pool_0, &my_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Write ABCs into the packet payload!  */
        nx_packet_data_append(my_packet, DEMO_DATA, sizeof(DEMO_DATA), &pool_0, TX_WAIT_FOREVER);

        status =  nx_packet_length_get(my_packet, &length);
        if ((status) || (length != sizeof(DEMO_DATA)))
        {
            error_counter++;
        }

        /* Send the packet out!  */
        status =  nx_tcp_socket_send(&client_socket, my_packet, NX_IP_PERIODIC_RATE);

        /* Determine if the status is valid.  */
        if (status)
        {
            error_counter++;
            nx_packet_release(my_packet);
        }

        /* Disconnect this socket.  */
        status =  nx_tcp_socket_disconnect(&client_socket, NX_IP_PERIODIC_RATE);

        /* Determine if the status is valid.  */
        if (status)
        {
            error_counter++;
        }

        /* Unbind the socket.  */
        status =  nx_tcp_client_socket_unbind(&client_socket);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Delete the socket.  */
        status =  nx_tcp_socket_delete(&client_socket);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Increment thread 0's counter.  */
        thread_0_counter++;
    }
}


void    thread_1_entry(ULONG thread_input)
{

UINT       status;
NX_PACKET *packet_ptr;
ULONG      actual_status;

    NX_PARAMETER_NOT_USED(thread_input);

#ifndef NX_DISABLE_IPV6

    /* Wait 5 seconds for the IP thread to finish its initilization and
       for the IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#else
    /* Wait 1 second for the IP thread to finish its initilization. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);
#endif


    /* Ensure the IP instance has been initialized.  */
    status =  nx_ip_status_check(&ip_1, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if (status != NX_SUCCESS)
    {

        error_counter++;
        return;
    }

    /* Create a socket.  */
    status =  nx_tcp_socket_create(&ip_1, &server_socket, "Server Socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 100,
                                   NX_NULL, thread_1_disconnect_received);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }

    /* Setup this thread to listen.  */
    status =  nx_tcp_server_socket_listen(&ip_1, 12, &server_socket, 5, thread_1_connect_received);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }

    /* Loop to create and establish server connections.  */
    while (1)
    {

        /* Increment thread 1's counter.  */
        thread_1_counter++;

        /* Accept a client socket connection.  */
        status =  nx_tcp_server_socket_accept(&server_socket, NX_WAIT_FOREVER);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Receive a TCP message from the socket.  */
        status =  nx_tcp_socket_receive(&server_socket, &packet_ptr, NX_IP_PERIODIC_RATE);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }
        else
        {
            /* Release the packet.  */
            nx_packet_release(packet_ptr);
        }

        /* Disconnect the server socket.  */
        status =  nx_tcp_socket_disconnect(&server_socket, NX_IP_PERIODIC_RATE);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Unaccept the server socket.  */
        status =  nx_tcp_server_socket_unaccept(&server_socket);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }

        /* Setup server socket for listening again.  */
        status =  nx_tcp_server_socket_relisten(&ip_1, 12, &server_socket);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
        }
    }
}


void  thread_1_connect_received(NX_TCP_SOCKET *socket_ptr, UINT port)
{

    /* Check for the proper socket and port.  */
    if ((socket_ptr != &server_socket) || (port != 12))
    {
        error_counter++;
    }
}


void  thread_1_disconnect_received(NX_TCP_SOCKET *socket)
{

    /* Check for proper disconnected socket.  */
    if (socket != &server_socket)
    {
        error_counter++;
    }
}

