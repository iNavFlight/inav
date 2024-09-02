/* This is a small demo of TELNET on the high-performance NetX TCP/IP stack.  
   This demo relies on ThreadX and NetX to show a simple TELNET connection,
   send, server echo, and then disconnection from the TELNET server.  */

#include  "tx_api.h"
#include  "nx_api.h"
#include  "nxd_telnet_client.h"
#include  "nxd_telnet_server.h"

#define     DEMO_STACK_SIZE         4096    


/* Define the ThreadX and NetX object control blocks...  */

TX_THREAD               server_thread;
TX_THREAD               client_thread;
NX_PACKET_POOL          pool_server;
NX_PACKET_POOL          pool_client;
NX_IP                   ip_server;
NX_IP                   ip_client;


/* If the Telnet connection requires IPv6 support, define USE_IPV6.  Note that
   the NetX Duo Telnet Client and Server can communicate over IPv4 regardless
   if IPv6 is enabled in NetX Duo. However to use IPv6 addressing, the 
   FEATURE_NX_IPV6 must be defined in nx_user.h.  */
#ifdef NX_DISABLE_IPV4
#define USE_IPV6
#endif /* NX_DISABLE_IPV4 */

#ifdef USE_IPV6    
NXD_ADDRESS     server_ip_address;
#endif

/* Define TELNET objects.  */

NX_TELNET_SERVER        my_server;
NX_TELNET_CLIENT        my_client;


#define         SERVER_ADDRESS          IP_ADDRESS(1,2,3,4)
#define         CLIENT_ADDRESS          IP_ADDRESS(1,2,3,5)


/* Define the counters used in the demo application...  */

ULONG                   error_counter;


/* Define timeout in ticks for connecting and sending/receiving data. */

#define                 TELNET_TIMEOUT  (2 * NX_IP_PERIODIC_RATE)

/* Define function prototypes.  */

void    thread_server_entry(ULONG thread_input);
void    thread_client_entry(ULONG thread_input);

/* Replace the 'ram' driver with your actual Ethernet driver. */
void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


/* Define the application's TELNET Server callback routines.  */

void    telnet_new_connection(NX_TELNET_SERVER *server_ptr, UINT logical_connection); 
void    telnet_receive_data(NX_TELNET_SERVER *server_ptr, UINT logical_connection, NX_PACKET *packet_ptr);
void    telnet_connection_end(NX_TELNET_SERVER *server_ptr, UINT logical_connection);


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
CHAR    *pointer;

    
    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create the server thread.  */
    tx_thread_create(&server_thread, "server thread", thread_server_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the client thread.  */
    tx_thread_create(&client_thread, "client thread", thread_client_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create packet pool.  */
    nx_packet_pool_create(&pool_server, "Server NetX Packet Pool", 600, pointer, 8192);
    pointer = pointer + 8192;

    /* Create an IP instance.  */
    nx_ip_create(&ip_server, "Server NetX IP Instance", SERVER_ADDRESS, 
                        0xFFFFFF00UL, &pool_server, _nx_ram_network_driver,
                        pointer, 4096, 1);

    pointer =  pointer + 4096;

    /* Create another packet pool. */
    nx_packet_pool_create(&pool_client, "Client NetX Packet Pool", 600, pointer, 8192);
    pointer = pointer + 8192;
    
    /* Create another IP instance.  */
    nx_ip_create(&ip_client, "Client NetX IP Instance", CLIENT_ADDRESS, 
                        0xFFFFFF00UL, &pool_client, _nx_ram_network_driver, 
                        pointer, 4096, 1);

    pointer = pointer + 4096;

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    nx_arp_enable(&ip_server, (void *) pointer, 1024);
    pointer = pointer + 1024;
  
    /* Enable ARP and supply ARP cache memory for IP Instance 1.  */
    nx_arp_enable(&ip_client, (void *) pointer, 1024);
    pointer = pointer + 1024;
#endif /* NX_DISABLE_IPV4  */

    /* Enable TCP processing for both IP instances.  */
    nx_tcp_enable(&ip_server);
    nx_tcp_enable(&ip_client);

#ifdef USE_IPV6
    /* Next set the NetX Duo Telnet Server address. */
    server_ip_address.nxd_ip_address.v6[3] = 0x106;
    server_ip_address.nxd_ip_address.v6[2] = 0x0;
    server_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    server_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V6;
#endif

    /* Create the NetX TELNET Server.  */
    status =  nx_telnet_server_create(&my_server, "Telnet Server", &ip_server, 
                    pointer, 2048, telnet_new_connection, telnet_receive_data, 
                    telnet_connection_end);

    /* Check for errors.  */
    if (status)
        error_counter++;

    return;
}

/* Define the Server thread.  */
void    thread_server_entry(ULONG thread_input)
{

UINT    status;
#ifdef USE_IPV6    
UINT   iface_index, address_index;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow IP thread task to initialize the system. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifdef USE_IPV6    
    /* Here's where we make the Telnet Server IPv6 enabled. */
    status = nxd_ipv6_enable(&ip_server);
    
    /* Check for ipv6 enable error.  */
    if (status) 
    {

        error_counter++;
        return;
     }    
    
    status = nxd_icmp_enable(&ip_server);     

    /* Check for icmp6 enable error.  */
    if (status) 
    {

        error_counter++;
        return;
     }    



     /* Set the link local address with the host MAC address. */
    /* This assumes we are using the primary network interface (index 0). */
    iface_index = 0;
    
    status = nxd_ipv6_address_set(&ip_server, iface_index, NX_NULL, 10, &address_index);

    /* Check for link local address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }

    /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&ip_server, iface_index, &server_ip_address, 64, &address_index);

    /* Check for global address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }
    
    /* Wait while NetX Duo validates the link local and global address. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
#endif


    /* Start the TELNET Server.  */
    status =  nx_telnet_server_start(&my_server);

    /* Check for errors.  */
    if (status != NX_SUCCESS)
    {
       
        return;
    }


}

/* Define the client thread.  */
void    thread_client_entry(ULONG thread_input)
{

NX_PACKET   *my_packet;
UINT        status;
#ifdef USE_IPV6    
NXD_ADDRESS     client_ip_address;
UINT   iface_index, address_index;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow IP thread task to initialize the system. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifdef USE_IPV6

    /* Here's where we make the Telnet Client IPv6 enabled. */
    status= nxd_ipv6_enable(&ip_client);
    
        
    /* Check for ipv6 enable error.  */
    if (status) 
    {

        error_counter++;
        return;
     }  
    
    nxd_icmp_enable(&ip_client);     
    
        
    /* Check for icmp6 enable error.  */
    if (status) 
    {

        error_counter++;
        return;
    }  
    
    client_ip_address.nxd_ip_address.v6[3] = 0x101;
    client_ip_address.nxd_ip_address.v6[2] = 0x0;
    client_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    client_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    client_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Set the Client link local and global addresses. */

     /* Set the link local address with the host MAC address. */
    iface_index = 0;
    
    /* This assumes we are using the primary network interface (index 0). */
    status = nxd_ipv6_address_set(&ip_client, iface_index, NX_NULL, 10, &address_index);

    /* Check for link local address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }
    
     /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&ip_client, iface_index, &client_ip_address, 64, &address_index);

    /* Check for global address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }

    /* Let NetX Duo validate the addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
    
#endif /* USE_IPV6 */

    /* Create a TELENT client instance.  */
    status =  nx_telnet_client_create(&my_client, "My TELNET Client", &ip_client, 600);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    do
    {
#ifdef USE_IPV6
    
        /* Connect the TELNET client to the TELNET Server at port 23 over IPv6. Note that this
           service accept IPv4 addresses in the NXD_ADDRESS server_ip_address input.  */
        status =  nxd_telnet_client_connect(&my_client, &server_ip_address, NX_TELNET_SERVER_PORT, TELNET_TIMEOUT);

#else
        /* Connect the TELNET client to the TELNET Server at port 23 over IPv4.  */
        status =  nx_telnet_client_connect(&my_client, SERVER_ADDRESS, NX_TELNET_SERVER_PORT, TELNET_TIMEOUT);
#endif
    } while (status != NX_SUCCESS);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Allocate a packet.  */
    status =  nx_packet_allocate(&pool_client, &my_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Build a simple 1-byte message.  */
    nx_packet_data_append(my_packet, "a", 1, &pool_client, NX_WAIT_FOREVER);

    /* Send the packet to the TELNET Server.  */
    status =  nx_telnet_client_packet_send(&my_client, my_packet, TELNET_TIMEOUT);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Pickup the Server header.  */
    status =  nx_telnet_client_packet_receive(&my_client, &my_packet, TELNET_TIMEOUT);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* At this point the packet should contain the Server's banner
       message sent by the Server callback function below.  Just
       release it for this demo.  */
    nx_packet_release(my_packet);

    /* Pickup the Server echo of the character.  */
    status =  nx_telnet_client_packet_receive(&my_client, &my_packet, TELNET_TIMEOUT);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* At this point the packet should contain the character 'a' that
       we sent earlier.  Just release the packet for now.  */
    nx_packet_release(my_packet);

    /* Now disconnect form the TELNET Server.  */
    status =  nx_telnet_client_disconnect(&my_client, TELNET_TIMEOUT);


    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Delete the TELNET Client.  */
    status =  nx_telnet_client_delete(&my_client);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }
}


/* This routine is called by the NetX Telnet Server whenever a new Telnet client 
   connection is established.  */
void  telnet_new_connection(NX_TELNET_SERVER *server_ptr, UINT logical_connection)
{

UINT        status;
NX_PACKET   *packet_ptr;



    /* Allocate a packet for client greeting. */
    status =  nx_packet_allocate(&pool_server, &packet_ptr, NX_TCP_PACKET, NX_NO_WAIT);

    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Build a banner message and a prompt.  */
    nx_packet_data_append(packet_ptr, "**** Welcome to NetX TELNET Server ****\r\n\r\n\r\n", 45, 
                          &pool_server, NX_NO_WAIT);

    nx_packet_data_append(packet_ptr, "NETX> ", 6, &pool_server, NX_NO_WAIT);
    
    /* Send the packet to the client.  */
    status =  nx_telnet_server_packet_send(server_ptr, logical_connection, packet_ptr, TELNET_TIMEOUT);

    if (status != NX_SUCCESS)
    {
        error_counter++;
        nx_packet_release(packet_ptr);
    }

    return;
}


/* This routine is called by the NetX Telnet Server whenever data is present on a Telnet client 
   connection.  */          
void  telnet_receive_data(NX_TELNET_SERVER *server_ptr, UINT logical_connection, NX_PACKET *packet_ptr)
{

UINT    status;
UCHAR   alpha;


    /* This demo just echoes the character back and on <cr,lf> sends a new prompt back to the
       client.  A real system would most likely buffer the character(s) received in a buffer 
       associated with the supplied logical connection and process according to it.  */


    /* Just throw away carriage returns.  */
    if ((packet_ptr -> nx_packet_prepend_ptr[0] == '\r') && (packet_ptr -> nx_packet_length == 1))
    {

        nx_packet_release(packet_ptr);
        return;
    }

    /* Setup new line on line feed.  */
    if ((packet_ptr -> nx_packet_prepend_ptr[0] == '\n') || 
        ((packet_ptr -> nx_packet_prepend_ptr[0] == '\r') && (packet_ptr -> nx_packet_prepend_ptr[1] == '\n')))
    {

        /* Clean up the packet.  */
        packet_ptr -> nx_packet_length =  0;
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;
        packet_ptr -> nx_packet_append_ptr =   packet_ptr -> nx_packet_data_start + NX_TCP_PACKET;

        /* Build the next prompt.  */
        nx_packet_data_append(packet_ptr, "\r\nNETX> ", 8, &pool_server, NX_NO_WAIT);

        /* Send the packet to the client.  */
        status =  nx_telnet_server_packet_send(server_ptr, logical_connection, packet_ptr, TELNET_TIMEOUT);

        if (status != NX_SUCCESS)
        {
            error_counter++;
            nx_packet_release(packet_ptr);
        }

        return;
    }

    /* Pickup first character (usually only one from client).  */
    alpha =  packet_ptr -> nx_packet_prepend_ptr[0];

    /* Echo character.  */
    status =  nx_telnet_server_packet_send(server_ptr, logical_connection, packet_ptr, TELNET_TIMEOUT);

    if (status != NX_SUCCESS)
    {
        error_counter++;
        nx_packet_release(packet_ptr);
    }

    /* Check for a disconnection.  */
    if (alpha == 'q')
    {

        /* Initiate server disconnection.  */
        nx_telnet_server_disconnect(server_ptr, logical_connection);
    }
}


/* This routine is called by the NetX Telnet Server whenever the client disconnects.  */
void  telnet_connection_end(NX_TELNET_SERVER *server_ptr, UINT logical_connection)
{
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(logical_connection);

    /* Cleanup any application specific connection or buffer information.  */
    return;
}

