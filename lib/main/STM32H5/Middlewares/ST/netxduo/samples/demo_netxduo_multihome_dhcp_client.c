/* This is a small demo of DHCP Client for multiple interfaces for the high-performance NetX IP stack.  

   It is suggested that applications call interface specific API to do specific action on the specified
   interface if DHCP is enabled on multiple interfaces at the same time. 
   Example of a device possibly with a secondary interfaces attached, and DHCP Client having only the secondary 
   interface enabled for DHCP:
 
#if (NX_MAX_PHYSICAL_INTERFACES >= 2) && (NX_DHCP_CLIENT_MAX_RECORDS >= 2) 
 
       Client configured for DHCP enabled on more than one interface. Use the interface specific service to
       request Client IP on the secondary interface.
    
       status = nx_dhcp_interface_request_client_ip(&dhcp_client, 1, NX_DHCP_CLIENT_IP_ADDRESS_1, SKIP_DISCOVER_MESSAGE);
#else 
        Client is configured for one interface to be enabled for DHCP. Use the non-interface specific service
        to perform the request Client IP action on the interface index that the Client has set for DHCP.
      
        Note: the application must first call nx_dhcp_set_interface_index() to set the secondary interface as the interface
        to run DHCP on. Otherwise DHCP runs on the primary interface, by default.
      
        status = nx_dhcp_request_client_ip(&dhcp_client, NX_DHCP_CLIENT_IP_ADDRESS_1, SKIP_DISCOVER_MESSAGE);
#endif 
 
        if (status)
            error_counter++;
 
*/

#include   "tx_api.h"
#include   "nx_api.h"
#include   "nxd_dhcp_client.h"

#define     DEMO_STACK_SIZE             4096
#define     NX_PACKET_SIZE              1536
#define     NX_PACKET_POOL_SIZE         NX_PACKET_SIZE * 8


/* If defined, the host requests a (previous) client IP address. */
/*
#define REQUEST_CLIENT_IP
*/

#ifdef REQUEST_CLIENT_IP
/* Request a specific IP address using the DHCP client address option. */
#define     NX_DHCP_CLIENT_IP_ADDRESS_0   IP_ADDRESS(192, 168, 0, 18)
#define     NX_DHCP_CLIENT_IP_ADDRESS_1   IP_ADDRESS(10, 0, 0, 18)  

/* If defined NX_TRUE, the client requests to jump to the boot state and skip the DISCOVER message.  */
#define     SKIP_DISCOVER_MESSAGE       NX_TRUE 
#endif /* REQUEST_CLIENT_IP  */

/* Define the ThreadX and NetX object control blocks...  */
TX_THREAD               client_thread;
NX_PACKET_POOL          client_pool;
NX_IP                   client_ip;
NX_DHCP                 dhcp_client;


/* Define the counters used in the demo application...  */
ULONG                   client_thread_counter;
ULONG                   state_changes;
ULONG                   error_counter;
CHAR                    *pointer;


/* Define thread prototypes.  */
void    client_thread_entry(ULONG thread_input);
void    dhcp_interface_state_change(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state);

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

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the client packet pool.  */
    status = nx_packet_pool_create(&client_pool, "NetX Main Packet Pool", 1024, pointer, NX_PACKET_POOL_SIZE);
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

    /* Enable ARP and supply ARP cache memory for DHCP Client IP.  */
    status = nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check for ARP enable errors.  */
    if (status)
        return;

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&client_ip);

    /* Check for UDP enable errors.  */
    if (status)
        return;

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&client_ip);

    /* Check for TCP enable errors.  */
    if (status)
        return;

    /* Enable ICMP.  */
    status = nx_icmp_enable(&client_ip);

    /* Check for errors.  */
    if (status)
        return;
}


/* Define the client thread.  */
void    client_thread_entry(ULONG thread_input)
{

UINT        status;
UINT        actual_status;
ULONG       server_address;

    NX_PARAMETER_NOT_USED(thread_input);

     /*  Check if there are multiple network interfaces */

#if (NX_MAX_PHYSICAL_INTERFACES >= 2) && (NX_DHCP_CLIENT_MAX_RECORDS >= 2)
    /* There are.  Attach the second interface.  */
    status = nx_ip_interface_attach(&client_ip, "Second interface", IP_ADDRESS(0, 0, 0, 0), 0xFFFFFF00UL,_nx_ram_network_driver);

    /* Check status.  */
    if (status)
        return;
#endif

    /* Create the DHCP instance; this automatically enables DHCP on the primary interface.  */
    status = nx_dhcp_create(&dhcp_client, &client_ip, "dhcp_client");
    if (status)
        return;    
                           
    /* Check if there are multiple network interfaces and if DHCP CLient is set up to run DHCP on multiple interfaces. */
#if (NX_MAX_PHYSICAL_INTERFACES >= 2) && (NX_DHCP_CLIENT_MAX_RECORDS >= 2)
    /* Enable DHCP for specified interface(1:second interface).  */
    status = nx_dhcp_interface_enable(&dhcp_client, 1);
    if (status)
        return;
#else
    /* for NX_MAX_PHYSICAL_INTERFACES >= 2, NX_DHCP_CLIENT_MAX_RECORDS == 1*/
    status = nx_dhcp_set_interface_index(&dhcp_client, 1);
#endif

#ifdef REQUEST_CLIENT_IP

    /* nx_dhcp_request_client_ip() requests a specific IP address for DHCP client address option on the first DHCP enabled (valid) interface it finds. */
    /* Suggest using nx_dhcp_interface_request_client_ip() on a single interface if DHCP is enabled on multiple interfaces
       status = nx_dhcp_interface_request_client_ip(&dhcp_client, 1, NX_DHCP_CLIENT_IP_ADDRESS_0, SKIP_DISCOVER_MESSAGE)
           requests the IP address on the specified address (1:second interface)
    */
    status = nx_dhcp_request_client_ip(&dhcp_client, NX_DHCP_CLIENT_IP_ADDRESS_0, SKIP_DISCOVER_MESSAGE);
    if (status)
        error_counter++;

#endif /* REQUEST_CLIENT_IP  */

    /* Clear the broadcast flag.  */
    /* nx_dhcp_clear_broadcast_flag(&dhcp_client) clears the broadcast flag on all DHCP enabled interfaces.  
       Suggest using nx_dhcp_interface_clear_broadcast_flag() to clear the flag on one interface if DHCP is enabled on multiple interfaces
       status = nx_dhcp_interface_clear_broadcast_flag(&dhcp_client, 1) clears the broadcast flag on the specified interface(1:second interface)
    */
    status = nx_dhcp_clear_broadcast_flag(&dhcp_client, NX_TRUE);
    if (status)
        error_counter++;


    /* Register the interface state change callback.  */
    status = nx_dhcp_interface_state_change_notify(&dhcp_client, dhcp_interface_state_change);
    if (status)
        error_counter++;

    /* Start the DHCP Client.  */
    /* nx_dhcp_start(&dhcp_client) start DHCP for all DHCP enabled interfaces.  
       Suggest using nx_dhcp_interface_start() to start DHCP on one interface if DHCP is enabled on multiple interfaces
       status = nx_dhcp_interface_start(&dhcp_client, 1) starts DHCP on the specified interface (1:second interface)
    */
    status = nx_dhcp_start(&dhcp_client);

    /* Loop to test DHCP.  */
    while (1)
    {

        /* Check the address resolution for primary interface.  */
        nx_ip_interface_status_check(&client_ip, 0, NX_IP_ADDRESS_RESOLVED, (ULONG *)&actual_status, NX_WAIT_FOREVER);

        /* Check the address resolution for second interface.  */
        nx_ip_interface_status_check(&client_ip, 1, NX_IP_ADDRESS_RESOLVED, (ULONG *)&actual_status, NX_WAIT_FOREVER);

        /* Use this API to get the Server address.  */
        /* nx_dhcp_server_address_get(&dhcp_client, &server_address) get the server address for the first DHCP enabled ("valid") interface.
           Suggest using nx_dhcp_interface_server_address_get() to get the server address on a specific interface if DHCP is enabled on multiple interfaces
           status = nx_dhcp_interface_server_address_get(&dhcp_client, 1, &server_address) returns the server address on specified interface(1:second interface)
        */
        status = nx_dhcp_server_address_get(&dhcp_client, &server_address);
        if (status)
            error_counter++;


        /* Release the IP address the Client is bound to.
         
           Use the nx_dhcp_release() API to release an IP address if the host is switching networks or running the host through DHCP cycles.
           Note that it is not necessary to call nx_dhcp_reinitialize() or nx_dhcp_interface_reinitialize() after calling this
           function. DHCP on this interface (or interfaces) is ready to be restarted. */

        /* nx_dhcp_release(&dhcp_client) releases the DHCP generated IP address for all DHCP enabled interfaces.          
           Suggest using nx_dhcp_interface_release() to release the IP address on a specific interface if DHCP is enabled on multiple interfaces
           status = nx_dhcp_interface_release(&dhcp_client, 1) releases the IP address for the specified interface(1:second interface)
        */
        status = nx_dhcp_release(&dhcp_client);
        if (status)
            error_counter++;

        /* Stopping the DHCP client.
         
           Use this API if the Client has not reached the BOUND state. This simply stops the DHCP
           Client.  It does not clear any network parameters or reset the Client state to NOT STARTED.  To do clear network parameters,
           and reset the state (e.g. before calling nx_dhcp_start() on the stopped interface(s), call nx_dhcp_reinitialize() or
           nx_dhcp_interface_reinitialize() depending which interface(s) need to be reinitialized.         
        */

        /* nx_dhcp_stop(&dhcp_client) stops DHCP on all DHCP enabled interfaces.  
           Suggest using nx_dhcp_interface_stop() to stop DHCP on a specific interface if DHCP is enabled on multiple interfaces
           status = nx_dhcp_interface_stop(&dhcp_client, 1) stop DHCP on the specified interface(1:second interface)
           */
        status = nx_dhcp_stop(&dhcp_client);
        if (status)
            error_counter++;

        /* Sleep one second. */
        tx_thread_sleep(NX_IP_PERIODIC_RATE);

        /* Reinitialize the Client for restarting DHCP.
         
           Use this API to clear the network parameters and restart the client in the not started state. */

        /* nx_dhcp_reinitialize(&dhcp_client) clears the network parameters on all DHCP enabled interfaces.  
           Suggest using nx_dhcp_interface_reinitialize() to reinitialize DHCP on a specific interface if DHCP is enabled on multiple interfaces
           status = nx_dhcp_interface_reinitialize(&dhcp_client, 1) reinitializes DHCP on the specified interface(1:second interface)
           */
        status = nx_dhcp_reinitialize(&dhcp_client);
        if (status)
            error_counter++;

        /* Resume the DHCP client thread. */
        /* nx_dhcp_start(&dhcp_client) start DHCP for all DHCP enabled interfaces.  
           or nx_dhcp_interface_start(&dhcp_client, 1) to start DHCP for specified interface(1:second interface) */
        status = nx_dhcp_start(&dhcp_client); 
        if (status)
            error_counter++;
    }

    /* All done. Return resources to NetX and ThreadX. */    
    nx_dhcp_delete(&dhcp_client);

    return;
}


void dhcp_interface_state_change(NX_DHCP *dhcp_ptr, UINT iface_index,  UCHAR new_state)
{

    NX_PARAMETER_NOT_USED(dhcp_ptr);
    NX_PARAMETER_NOT_USED(iface_index);
    NX_PARAMETER_NOT_USED(new_state);

    /* Increment state changes counter.  */
    state_changes++;

    return;
}

