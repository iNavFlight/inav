/* This is a small demo of the NetX TCP/IP stack using the AUTO IP module.  */

#include "tx_api.h"
#include "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include "nx_auto_ip.h"


#define     DEMO_STACK_SIZE         4096
#define     DEMO_DATA               "ABCDEFGHIJKLMNOPQRSTUVWXYZ "


/* Define the ThreadX and NetX object control blocks...  */

TX_THREAD               thread_0;
TX_THREAD               thread_1;
NX_PACKET_POOL          pool_0;
NX_IP                   ip_0;
NX_IP                   ip_1;
NX_UDP_SOCKET           socket_0;
NX_UDP_SOCKET           socket_1;


/* Define the AUTO IP structures for each IP instance.   */

NX_AUTO_IP              auto_ip_0;
NX_AUTO_IP              auto_ip_1;


/* Define the counters used in the demo application...  */

ULONG                   thread_0_counter;
ULONG                   thread_1_counter;
ULONG                   address_changes;
ULONG                   error_counter;


/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);
void    thread_1_entry(ULONG thread_input);
void    ip_address_changed(NX_IP *ip_ptr, VOID *auto_ip_address);

void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

CHAR    *pointer;
UINT    status;

    
    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            16, 16, 1, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the main thread.  */
    tx_thread_create(&thread_1, "thread 1", thread_1_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            16, 16, 1, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 256, pointer, 4096);
    pointer = pointer + 4096;

    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(0, 0, 0, 0), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                    pointer, 4096, 1);
    pointer =  pointer + 4096;

    /* Create another IP instance.  */
    status += nx_ip_create(&ip_1, "NetX IP Instance 1", IP_ADDRESS(0, 0, 0, 0), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                    pointer, 4096, 1);
    pointer =  pointer + 4096;

    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    status +=  nx_arp_enable(&ip_1, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check ARP enable status.  */
    if (status)
        error_counter++;

    /* Enable UDP processing for both IP instances.  */
    status =  nx_udp_enable(&ip_0);
    status += nx_udp_enable(&ip_1);

    /* Check UDP enable status.  */
    if (status)
        error_counter++;

    /* Create the AutoIP instance for each IP instance.   */
    status =  nx_auto_ip_create(&auto_ip_0, "AutoIP 0", &ip_0, pointer, 4096, 1);
    pointer = pointer + 4096;
    status += nx_auto_ip_create(&auto_ip_1, "AutoIP 1", &ip_1, pointer, 4096, 1);
    pointer = pointer + 4096;

    /* Check AutoIP create status.  */
    if (status)
        error_counter++;

    /* Start both AutoIP instances.  */
    status =  nx_auto_ip_start(&auto_ip_0, 0 /*IP_ADDRESS(169,254,254,255)*/);
    status += nx_auto_ip_start(&auto_ip_1, 0 /*IP_ADDRESS(169,254,254,255)*/);

    /* Check AutoIP start status.  */
    if (status)
        error_counter++;

    /* Register an IP address change function for each IP instance.  */
    status =  nx_ip_address_change_notify(&ip_0, ip_address_changed, (void *) &auto_ip_0);
    status += nx_ip_address_change_notify(&ip_1, ip_address_changed, (void *) &auto_ip_1);

    /* Check IP address change notify status.  */
    if (status)
        error_counter++;
}



/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{

UINT        status;
ULONG       actual_status;
NX_PACKET   *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Wait for IP address to be resolved.   */
    do
    {

        /* Call IP status check routine.   */
        status =  nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, 10 * NX_IP_PERIODIC_RATE);
    
    } while (status != NX_SUCCESS);

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

    /* Let other threads run again.  */
    tx_thread_relinquish();

    while(1)
    {


        /* Allocate a packet.  */
        status =  nx_packet_allocate(&pool_0, &my_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
            break;

        /* Write ABCs into the packet payload!  */
        memcpy(my_packet -> nx_packet_prepend_ptr, DEMO_DATA, sizeof(DEMO_DATA)); /* Use case of memcpy is verified. */

        /* Adjust the write pointer.  */
        my_packet -> nx_packet_length =  sizeof(DEMO_DATA);
        my_packet -> nx_packet_append_ptr =  my_packet -> nx_packet_prepend_ptr + sizeof(DEMO_DATA);

        /* Send the UDP packet.  */
        status =  nx_udp_socket_send(&socket_0, my_packet, ip_1.nx_ip_address, 0x89);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            error_counter++;
            break;
        }

        /* Increment thread 0's counter.  */
        thread_0_counter++;

        /* Relinquish to thread 1.  */
        tx_thread_relinquish();
    }
}
    

void    thread_1_entry(ULONG thread_input)
{

UINT        status;
ULONG       actual_status;
NX_PACKET   *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Wait for IP address to be resolved.   */
    do
    {

        /* Call IP status check routine.   */
        status =  nx_ip_status_check(&ip_1, NX_IP_ADDRESS_RESOLVED, &actual_status, 10 * NX_IP_PERIODIC_RATE);
    
    } while (status != NX_SUCCESS);

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

    while(1)
    {


        /* Receive a UDP packet.  */
        status =  nx_udp_socket_receive(&socket_1, &my_packet, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
            break;

        /* Release the packet.  */
        status =  nx_packet_release(my_packet);

        /* Check status.  */
        if (status != NX_SUCCESS)
            break;

        /* Increment thread 1's counter.  */
        thread_1_counter++;
    }
}

void  ip_address_changed(NX_IP *ip_ptr, VOID *auto_ip_address)
{

ULONG           ip_address;
ULONG           network_mask;
NX_AUTO_IP      *auto_ip_ptr;


    /* Setup pointer to auto IP instance.  */
    auto_ip_ptr =  (NX_AUTO_IP *) auto_ip_address;

    /* Pickup the current IP address.  */
    nx_ip_address_get(ip_ptr, &ip_address, &network_mask);

    /* Determine if the IP address has changed back to zero. If so, make sure the
       AutoIP instance is started.  */
    if (ip_address == 0)
    {

        /* Get the last AutoIP address for this node.  */
        nx_auto_ip_get_address(auto_ip_ptr, &ip_address);

        /* Start this AutoIP instance.  */
        nx_auto_ip_start(auto_ip_ptr, ip_address);
    }

    /* Determine if the IP address has transitioned to a non local IP address.  */
    else if ((ip_address & 0xFFFF0000UL) != IP_ADDRESS(169, 254, 0, 0))
    {

        /* Stop the AutoIP processing.  */
        nx_auto_ip_stop(auto_ip_ptr);
    }

    /* Increment a counter.  */
    address_changes++;
}
#endif /* NX_DISABLE_IPV4 */
