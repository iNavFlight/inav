/* This is a small demo of DHCP Client for the high-performance NetX IP stack.  */

#include   "tx_api.h"
#include   "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include   "nxd_dhcp_client.h"
#include   "nxd_dhcp_server.h"

#define     DEMO_STACK_SIZE             4096
#define     NX_PACKET_SIZE              1536
#define     NX_PACKET_POOL_SIZE         NX_PACKET_SIZE * 8

#define     NX_DHCP_SERVER_IP_ADDRESS_0 IP_ADDRESS(10,0,0,1)   
#define     START_IP_ADDRESS_LIST_0     IP_ADDRESS(10,0,0,10)
#define     END_IP_ADDRESS_LIST_0       IP_ADDRESS(10,0,0,19)

#define     NX_DHCP_SUBNET_MASK_0       IP_ADDRESS(255,255,255,0)
#define     NX_DHCP_DEFAULT_GATEWAY_0   IP_ADDRESS(10,0,0,1)
#define     NX_DHCP_DNS_SERVER_0        IP_ADDRESS(10,0,0,1)

/* Define the interface index.  */
#define     NX_DHCP_INTERFACE_INDEX     0             

/* If defined, the host requests a (previous) client IP address. */
/*
#define REQUEST_CLIENT_IP
*/

#ifdef REQUEST_CLIENT_IP       
/* Request a specific IP address using the DHCP client address option. */           
#define     NX_DHCP_CLIENT_IP_ADDRESS   IP_ADDRESS(10,0,0,18)  

/* If defined NX_TRUE, the client requests to jump to the boot state and skip the DISCOVER message.  */
#define     SKIP_DISCOVER_MESSAGE       NX_TRUE 
#endif



/* Define the ThreadX and NetX object control blocks...  */
TX_THREAD               client_thread;
NX_PACKET_POOL          client_pool;
NX_IP                   client_ip;
NX_DHCP                 dhcp_client;

TX_THREAD               server_thread;
NX_PACKET_POOL          server_pool;
NX_IP                   server_ip;
NX_DHCP_SERVER          dhcp_server;

/* Define the counters used in the demo application...  */

ULONG                   client_thread_counter;
ULONG                   state_changes;
ULONG                   error_counter;
CHAR                    *pointer;

UCHAR message[50] = "My Ping Request!" ;


/* Define thread prototypes.  */

void    server_thread_entry(ULONG thread_input);
void    client_thread_entry(ULONG thread_input);
void    dhcp_state_change(NX_DHCP *dhcp_ptr, UCHAR new_state);

/******** Optionally substitute your Ethernet driver here. ***********/
void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
    return 0;
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

UINT    status;


    /* Setup the working pointer.  */
    pointer = (CHAR *) first_unused_memory;

    /* Create the client thread.  */
    tx_thread_create(&client_thread, "thread client", client_thread_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Create the server thread.  */
    tx_thread_create(&server_thread, "thread server", server_thread_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the client packet pool.  */
    status = nx_packet_pool_create(&client_pool, "NetX Main Packet Pool", 1024, pointer, NX_PACKET_POOL_SIZE);
    pointer = pointer + NX_PACKET_POOL_SIZE;

    /* Check for pool creation error.  */
    if (status)
        return;
    
    /* Create the server packet pool.  */
    status = nx_packet_pool_create(&server_pool, "NetX Main Packet Pool", 1024, pointer, NX_PACKET_POOL_SIZE);
    pointer = pointer + NX_PACKET_POOL_SIZE;

    /* Check for pool creation error.  */
    if (status)
        return;

    /* Create an IP instance for the DHCP Client.  */
    status = nx_ip_create(&client_ip, "DHCP Client", IP_ADDRESS(0, 0, 0, 0), 0xFFFFFF00UL, &client_pool, _nx_ram_network_driver, pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        return;
    
    /* Create an IP instance for the DHCP Server.  */
    status = nx_ip_create(&server_ip, "DHCP Server", NX_DHCP_SERVER_IP_ADDRESS_0, 0xFFFFFF00UL, &server_pool, _nx_ram_network_driver, pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        return;         

    /* Enable ARP and supply ARP cache memory for DHCP Client IP.  */
    status = nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;
    
    /* Enable ARP and supply ARP cache memory for DHCP Server IP.  */
    status += nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check for ARP enable errors.  */
    if (status)
        return;

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&client_ip);
    status += nx_udp_enable(&server_ip);

    /* Check for UDP enable errors.  */
    if (status)
        return;

    /* Enable ICMP.  */
    status = nx_icmp_enable(&client_ip);
    status += nx_icmp_enable(&server_ip);

    /* Check for errors.  */
    if (status)
        return;
}

/* Define the server thread.  */

void    server_thread_entry(ULONG thread_input)
{

UINT        status;
UINT        addresses_added;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Modified the mtu size to avoid fragmenting the DHCP packet since the default mtu size is 128 in _nx_ram_network_driver.  */
    status = nx_ip_interface_mtu_set(&server_ip, NX_DHCP_INTERFACE_INDEX, 1500);    

    /* Check for MTU set errors.  */
    if (status)                         
        return;

    /* Create the DHCP Server.  */
    status = nx_dhcp_server_create(&dhcp_server, &server_ip, pointer, DEMO_STACK_SIZE, 
                                   "DHCP Server", &server_pool);
    pointer = pointer + DEMO_STACK_SIZE;
    
    /* Check for errors creating the DHCP Server. */
    if (status)
        return;

    /* Load the assignable DHCP IP addresses for the first interface.  */
    status = nx_dhcp_create_server_ip_address_list(&dhcp_server, NX_DHCP_INTERFACE_INDEX, START_IP_ADDRESS_LIST_0, 
                                                   END_IP_ADDRESS_LIST_0, &addresses_added);

    /* Check for errors creating the list. */
    if (status)
        return;

    /* Verify all the addresses were added to the list. */
    if (addresses_added != 10)
        return;

    /* Set the interface network parameters.  */
    status = nx_dhcp_set_interface_network_parameters(&dhcp_server, NX_DHCP_INTERFACE_INDEX, NX_DHCP_SUBNET_MASK_0, 
                                                      NX_DHCP_DEFAULT_GATEWAY_0, NX_DHCP_DNS_SERVER_0);

    /* Check for errors setting network parameters. */
    if (status)
        return;

    /* Start DHCP Server task.  */
    status = nx_dhcp_server_start(&dhcp_server);

    /* Check for errors starting up the DHCP server.  */
    if (status)
    return;
}


/* Define the client thread.  */

void    client_thread_entry(ULONG thread_input)
{

UINT        status;
UINT        actual_status;
UINT        length;
UINT        ping = NX_TRUE;
UINT        run_dhcp_client = NX_TRUE;
NX_PACKET   *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Modified the mtu size to avoid fragmenting the DHCP packet since the default mtu size is 128 in _nx_ram_network_driver.  */
    status = nx_ip_interface_mtu_set(&client_ip, NX_DHCP_INTERFACE_INDEX, 1500);    

    /* Check for MTU set errors.  */
    if (status)                         
        return;

    /* Create the DHCP instance.  */
    status = nx_dhcp_create(&dhcp_client, &client_ip, "DHCP-CLIENT");
    if (status)
        return;    

#ifdef REQUEST_CLIENT_IP
    /* Request a specific IP address using the DHCP client address option. */
    status = nx_dhcp_request_client_ip(&dhcp_client, NX_DHCP_CLIENT_IP_ADDRESS, SKIP_DISCOVER_MESSAGE);
    if (status)
        error_counter++;
#endif

    /* Register state change variable.  */
    status = nx_dhcp_state_change_notify(&dhcp_client, dhcp_state_change);
    if (status)
        error_counter++;

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);
    while(run_dhcp_client)
    {

        /* Wait for DHCP to assign the IP address.  */
        do
        {
    
            /* Check for address resolution.  */
            status = nx_ip_status_check(&client_ip, NX_IP_ADDRESS_RESOLVED, (ULONG *) &actual_status, NX_IP_PERIODIC_RATE);
    
            /* Check status.  */
            if (status)
            {
                /* wait a bit. */
                tx_thread_sleep(NX_IP_PERIODIC_RATE);
            }
    
        } while (status != NX_SUCCESS);
    
        length = sizeof(message);
    
        while(ping)
        {
    
            /* Send pings to another host on the network...  */
            status = nx_icmp_ping(&client_ip, NX_DHCP_SERVER_IP_ADDRESS_0, (CHAR *)message, length, &my_packet, NX_IP_PERIODIC_RATE);
            if (status)
               error_counter++;
            else
                nx_packet_release(my_packet);

            /* Increment counter.  */
            client_thread_counter++; 
          
            /* Sleep for a few ticks...  */
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
        }
    
        /* Use this API to send a message to the server, e.g. a DECLINE if the IP address is owned by another host. 
        nx_dhcp_send_request(&dhcp_client, NX_DHCP_TYPE_DHCPDECLINE);
        */

        /* Use this API to release an IP address if the host is switching networks or running the host through DHCP cycles. 
        nx_dhcp_release(&dhcp_client);
        */
    
        /* Stopping the DHCP client. */
        nx_dhcp_stop(&dhcp_client);
    
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    
        /* Use this API to clear the network parameters and restart the client in the INIT state. */
        nx_dhcp_reinitialize(&dhcp_client);
    
        /* Resume the DHCP client thread. */
        nx_dhcp_start(&dhcp_client);
    
        /* Ok to resume ping attempts. */
        ping = NX_TRUE;
    }

    /* All done. Return resources to NetX and ThreadX. */    
    nx_dhcp_delete(&dhcp_client);

    return;
}


void dhcp_state_change(NX_DHCP *dhcp_ptr, UCHAR new_state)
{

    NX_PARAMETER_NOT_USED(dhcp_ptr);
    NX_PARAMETER_NOT_USED(new_state);

    /* Increment state changes counter.  */
    state_changes++;

    return;
}
#endif /* NX_DISABLE_IPV4 */
