/* This is a small demo of the NetX Duo DHCPv6 Client and Server for the high-performance 
   NetX Duo stack.  */

#include    <stdio.h>
#include   "tx_api.h"
#include   "nx_api.h"
#include   "nxd_dhcpv6_client.h"
#include   "nxd_dhcpv6_server.h"

#define     DEMO_DATA               "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

#ifdef FEATURE_NX_IPV6

#define     DEMO_STACK_SIZE                     2048
#define     NX_DHCPV6_THREAD_STACK_SIZE         2048

/* Define the ThreadX and NetX object control blocks...  */

NX_PACKET_POOL          pool_0;
TX_THREAD               thread_client;
NX_IP                   client_ip;
TX_THREAD               thread_server;
NX_IP                   server_ip;

/* Define the Client and Server instances. */

NX_DHCPV6               dhcp_client;
NX_DHCPV6_SERVER        dhcp_server;

/* Define the variables.  */

CHAR                    *pointer;
NXD_ADDRESS             server_address; 
NXD_ADDRESS             dns_ipv6_address;
NXD_ADDRESS             start_ipv6_address;
NXD_ADDRESS             end_ipv6_address;


/* Define thread prototypes.  */

void    thread_client_entry(ULONG thread_input);
void    thread_server_entry(ULONG thread_input);

/***** Substitute your ethernet driver entry function here *********/
VOID    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);


/* Define some DHCPv6 parameters. */

#define DHCPV6_IANA_ID                  0xABCDEFAB 
#define DHCPV6_T1                       NX_DHCPV6_INFINITE_LEASE 
#define DHCPV6_T2                       NX_DHCPV6_INFINITE_LEASE 
#define NX_DHCPV6_REFERRED_LIFETIME     NX_DHCPV6_INFINITE_LEASE
#define NX_DHCPV6_VALID_LIFETIME        NX_DHCPV6_INFINITE_LEASE


/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

UINT    status;

    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Initialize the NetX system.  */
    nx_system_initialize();
    
    /* Create the Client thread.  */
    status = tx_thread_create(&thread_client, "Client thread", thread_client_entry, 0,  
                              pointer, DEMO_STACK_SIZE, 
                              4, 4, TX_NO_TIME_SLICE, TX_AUTO_START); 
    pointer = pointer + DEMO_STACK_SIZE;

    /* Check for thread create errors.  */
    if (status)
        return;

    /* Create the Server thread.  */
    status = tx_thread_create(&thread_server, "Server thread", thread_server_entry, 0,  
                              pointer, DEMO_STACK_SIZE, 
                              3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Check for thread create errors.  */
    if (status)
        return;

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 1024, pointer, NX_DHCPV6_PACKET_POOL_SIZE);
    pointer = pointer + NX_DHCPV6_PACKET_POOL_SIZE;

    /* Check for pool creation error.  */
    if (status)
        return;

    /* Create a Client IP instance.  */
    status = nx_ip_create(&client_ip, "Client IP", IP_ADDRESS(0, 0, 0, 0), 
                          0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        return;

    /* Create a Server IP instance.  */
    status = nx_ip_create(&server_ip, "Server IP", IP_ADDRESS(1, 2, 3, 4), 
                          0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        return;

    /* Enable UDP traffic for sending DHCPv6 messages.  */
    status =  nx_udp_enable(&client_ip);
    status +=  nx_udp_enable(&server_ip);

    /* Check for UDP enable errors.  */
    if (status)
        return;
    
    /* Enable the IPv6 services. */
    status = nxd_ipv6_enable(&client_ip);
    status += nxd_ipv6_enable(&server_ip);
    
    /* Check for IPv6 enable errors.  */
    if (status)
        return;
    
    /* Enable the ICMPv6 services. */
    status = nxd_icmp_enable(&client_ip);
    status += nxd_icmp_enable(&server_ip);
    
    /* Check for ICMPv6 enable errors.  */
    if (status)
        return;
}

/* Define the Client host application thread. */

void    thread_client_entry(ULONG thread_input)
{

UINT        status;

#ifdef  GET_ONE_SPECIFIC_ADDRESS
NXD_ADDRESS ia_ipv6_address;
#endif

NXD_ADDRESS ipv6_address;
NXD_ADDRESS dns_address;
ULONG       T1, T2, preferred_lifetime, valid_lifetime;
UINT        address_count;
UINT        address_index;
UINT        dns_index;
NX_PACKET   *my_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Modified the mtu size to avoid fragmenting the DHCP packet since the default mtu size is 128 in _nx_ram_network_driver.  */
    status = nx_ip_interface_mtu_set(&server_ip, 0, 1500);    

    /* Check for MTU set errors.  */
    if (status)                         
        return;

    /* Establish the link local address for the host. The RAM driver creates
       a virtual MAC address of 0x1122334456. */
    status = nxd_ipv6_address_set(&client_ip, 0, NX_NULL, 10, NULL);

    /* Let NetX Duo and the network driver get initialized. Also give the server time to get set up. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);        
    
    /* Create the DHCPv6 Client. */
    status =  nx_dhcpv6_client_create(&dhcp_client, &client_ip, "DHCPv6 Client", &pool_0, pointer, NX_DHCPV6_THREAD_STACK_SIZE,
                                      NX_NULL, NX_NULL);
    pointer = pointer + NX_DHCPV6_THREAD_STACK_SIZE;

    /* Check for errors.  */
    if (status)
        return;

    /* Create a Link Layer Plus Time DUID for the DHCPv6 Client. Set time ID field 
       to NULL; the DHCPv6 Client API will supply one. */
    status = nx_dhcpv6_create_client_duid(&dhcp_client, NX_DHCPV6_DUID_TYPE_LINK_TIME, 
                                          NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET, 0);

    /* Check for errors.  */
    if (status)
        return;

    /* Create the DHCPv6 client's Identity Association (IA-NA) now. 

       Note that if this host had already been assigned in IPv6 lease, it
       would have to use the assigned T1 and T2 values in loading the DHCPv6
       client with an IANA block. 
    */
    status = nx_dhcpv6_create_client_iana(&dhcp_client, DHCPV6_IANA_ID, DHCPV6_T1,  DHCPV6_T2); 

    /* Check for errors.  */
    if (status)
        return;
    
    /* Starting up the NetX DHCPv6 Client. */          
    status =  nx_dhcpv6_start(&dhcp_client);

    /* Check for errors.  */
    if (status)
        return;

    /* Let DHCPv6 Server start. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
    
#ifdef  GET_ONE_SPECIFIC_ADDRESS

    /* Create an IA address option. 
       The client includs IA options for any IAs to which it wants the server to assign addresses. 
    */
    ia_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    ia_ipv6_address.nxd_ip_address.v6[0] = 0x20010db8;
    ia_ipv6_address.nxd_ip_address.v6[1] = 0x00000f101;
    ia_ipv6_address.nxd_ip_address.v6[2] = 0x0;
    ia_ipv6_address.nxd_ip_address.v6[3] = 0x00000115;
    status = nx_dhcpv6_create_client_ia(&dhcp_client, &ia_ipv6_address, NX_DHCPV6_REFERRED_LIFETIME, 
                                        NX_DHCPV6_VALID_LIFETIME);

    /* Check for errors.  */
    if (status != NX_SUCCESS)
        return;
#endif

    /* If the host also want to get the option message, set the list of desired options to enabled. */
    nx_dhcpv6_request_option_timezone(&dhcp_client, NX_TRUE); 
    nx_dhcpv6_request_option_DNS_server(&dhcp_client, NX_TRUE);
    nx_dhcpv6_request_option_time_server(&dhcp_client, NX_TRUE);
    nx_dhcpv6_request_option_domain_name(&dhcp_client, NX_TRUE);

    /* Now, the host send the solicit message to get the IPv6 address and other options from the DHCPv6 server. */
    status = nx_dhcpv6_request_solicit(&dhcp_client);

    /* Check status.  */
    if (status)
        return;

    /* Waiting for get the IPv6 address and do the duplicate address detection. */
    /*
        Note, if the host detect another host withe the same address, the DHCPv6 Client can automatically 
        declient the address. At time T1 for an IPv6 address, the DHCPv6 Client can automatically renew the address.
        At time T2 for an IPv6 address, the DHCPv6 Client can automatically rebind the address.
        At time valid lifetime for an IPv6 address, the DHCPv6 Client can automatically delete the IPv6 address.
    */
    tx_thread_sleep(6 * NX_IP_PERIODIC_RATE);    
    
    /* Get the T1 and T2 value of IANA option.  */
    status = nx_dhcpv6_get_iana_lease_time(&dhcp_client, &T1, &T2);

    /* Check status.  */
    if (status)
        return;

    /* Get the valid IPv6 address count which the DHCPv6 server assigned .  */
    status = nx_dhcpv6_get_valid_ip_address_count(&dhcp_client, &address_count);

    /* Check status.  */
    if (status)
        return;
    
    /* Get the IPv6 address, preferred lifetime and valid lifetime according to the address index. */
    address_index = 0;
    status = nx_dhcpv6_get_valid_ip_address_lease_time(&dhcp_client, address_index, &ipv6_address, &preferred_lifetime, &valid_lifetime);

    /* Check status.  */
    if (status)
        return;

    /* Get the IPv6 address.        
       Note, This API only applies to one IA. */
    status = nx_dhcpv6_get_IP_address(&dhcp_client, &ipv6_address);

    /* Check status.  */
    if (status)
        return;

    /* Get IP address lease time. 
       Note, This API only applies to one IA. */
    status = nx_dhcpv6_get_lease_time_data(&dhcp_client, &T1, &T2, &preferred_lifetime, &valid_lifetime);

    /* Check status.  */ 
    if (status)
        return;
    
    /* Get the DNS Server address lease time. */
    dns_index = 0;
    status = nx_dhcpv6_get_DNS_server_address(&dhcp_client, dns_index, &dns_address);

    /* Check status.  */
    if (status)
        return;

    /**************************************************/
    /* Ping the DHCPv6 Server, Test the IPv6 address. */ 
    /**************************************************/    
    
    /* Ping the IPv6 address of DHCPv6 Server.  */
    status = nxd_icmp_ping(&client_ip, &server_address, DEMO_DATA, sizeof(DEMO_DATA), &my_packet, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status)
        return;

    /* If we want to release the address, we can send release message to 
       the server we are releasing the assigned address.  */
    status = nx_dhcpv6_request_release(&dhcp_client);

    /* Check status.  */ 
    if (status)
        return;

    /* Stop the Client task. */
    status = nx_dhcpv6_stop(&dhcp_client);

    /* Check status.  */
    if (status)
        return;

    /* Now delete the DHCPv6 client. */
    status = nx_dhcpv6_client_delete(&dhcp_client);        
 
    /* Check status.  */
    if (status)
        return; 
}

/* Define the test server thread. */

void    thread_server_entry(ULONG thread_input)
{

UINT        status;
ULONG       duid_time;
UINT        addresses_added;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Modified the mtu size to avoid fragmenting the DHCP packet since the default mtu size is 128 in _nx_ram_network_driver.  */
    status = nx_ip_interface_mtu_set(&client_ip, 0, 1500);    

    /* Check for MTU set errors.  */
    if (status)                         
        return;

    /* Set the IPv6 address of DHCPv6 Server.  */
    server_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    server_address.nxd_ip_address.v6[0] = 0x20010db8;
    server_address.nxd_ip_address.v6[1] = 0xf101;
    server_address.nxd_ip_address.v6[2] = 0x00000000;
    server_address.nxd_ip_address.v6[3] = 0x00000101;    

    /* Set the link local and global addresses. */
    status = nxd_ipv6_address_set(&server_ip, 0, NX_NULL, 10, NULL);
    status += nxd_ipv6_address_set(&server_ip, 0, &server_address, 64, NULL);

    /* Check for errors. */
    if (status)
        return;
    
    /* Create the DHCPv6 Server. */
    status =  nx_dhcpv6_server_create(&dhcp_server, &server_ip, "DHCPv6 Server", &pool_0, pointer, NX_DHCPV6_SERVER_THREAD_STACK_SIZE, NX_NULL, NX_NULL);
    pointer = pointer + NX_DHCPV6_SERVER_THREAD_STACK_SIZE;

    /* Check for errors.  */
    if (status)
        return;

    /* Note this example assumes a single global IP address on the primary interface.  If otherwise
       the host should call the service to set the network interface and global IP address index. 

        UINT _nx_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT interface_index, UINT address_index)
    */

    /* Validate the link local and global addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

    /* Set up the DNS IPv6 server address. */
    dns_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    dns_ipv6_address.nxd_ip_address.v6[0] = 0x20010db8;
    dns_ipv6_address.nxd_ip_address.v6[1] = 0x0000f101;
    dns_ipv6_address.nxd_ip_address.v6[2] = 0x00000000;
    dns_ipv6_address.nxd_ip_address.v6[3] = 0x00000107; 

    status = nx_dhcpv6_create_dns_address(&dhcp_server, &dns_ipv6_address);

    /* Check for errors. */
    if (status)
        return;

     /* Note: For DUID types that do not require time, the 'duid_time' input can be left at zero. 
        The DUID_TYPE and HW_TYPE are configurable options that are user defined in nx_dhcpv6_server.h.  */

    /* Set the DUID time as the start of the millenium. */
    duid_time = SECONDS_SINCE_JAN_1_2000_MOD_32;
    status = nx_dhcpv6_set_server_duid(&dhcp_server,
                                    NX_DHCPV6_SERVER_DUID_TYPE, NX_DHCPV6_SERVER_HW_TYPE,
                                    dhcp_server.nx_dhcpv6_ip_ptr -> nx_ip_arp_physical_address_msw,
                                    dhcp_server.nx_dhcpv6_ip_ptr -> nx_ip_arp_physical_address_lsw,
                                    duid_time);

    /* Check for errors. */
    if (status)
        return;
                
    /* Set the IPv6 start address.  */
    start_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    start_ipv6_address.nxd_ip_address.v6[0] = 0x20010db8;
    start_ipv6_address.nxd_ip_address.v6[1] = 0x00000f101;
    start_ipv6_address.nxd_ip_address.v6[2] = 0x0;
    start_ipv6_address.nxd_ip_address.v6[3] = 0x00000110;    
            
    /* Set the IPv6 end address.  */
    end_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6 ;
    end_ipv6_address.nxd_ip_address.v6[0] = 0x20010db8;
    end_ipv6_address.nxd_ip_address.v6[1] = 0x0000f101;
    end_ipv6_address.nxd_ip_address.v6[2] = 0x00000000;
    end_ipv6_address.nxd_ip_address.v6[3] = 0x00000120;  

    /* Set the IPv6 address range.  */
    status = nx_dhcpv6_create_ip_address_range(&dhcp_server, &start_ipv6_address, &end_ipv6_address, &addresses_added);

    /* Check for errors. */
    if (status)
        return;

    /* Start the NetX DHCPv6 server!  */
    status =  nx_dhcpv6_server_start(&dhcp_server);

    /* Check for errors. */
    if (status) 
        return;
}
#endif /* FEATURE_NX_IPV6 */
