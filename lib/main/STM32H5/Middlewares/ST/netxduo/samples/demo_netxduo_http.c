/* This is a small demo of the NetX HTTP Client Server API running on a 
   high-performance NetX TCP/IP stack.  */

#include   "tx_api.h"
#include   "nx_api.h"
/* If not using FileX, define this option and define the file writing services
   declared in filex_stub.h.    
#define      NX_HTTP_NO_FILEX
*/
#ifndef      NX_HTTP_NO_FILEX
#include    "fx_api.h"
#else
#include    "filex_stub.h"
#endif
#include   "nxd_http_client.h"
#include   "nxd_http_server.h"

/* For NetX Duo applications, determine which IP version to use. For IPv6, 
   set IP_TYPE to 6; for IPv4 set to 4. Note that for IPv6, you must enable
   USE_DUO so the application 'knows' to enabled IPv6 services on the IP task.  */

#ifdef NX_DISABLE_IPV4
#define     IP_TYPE     6
#else
#define     IP_TYPE     4
#endif /* NX_DISABLE_IPV4 */


#define     DEMO_STACK_SIZE         4096


/* Set up FileX and file memory resources. */
UCHAR           ram_disk_memory[32000];
FX_MEDIA        ram_disk;
unsigned char   media_memory[512];


/* Define device drivers.  */
extern void _fx_ram_driver(FX_MEDIA *media_ptr);

/* Replace the 'ram' driver with your Ethernet driver. */
VOID        _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);

UINT        authentication_check(NX_HTTP_SERVER *server_ptr, UINT request_type, 
                                 CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);

/* Set up the HTTP client global variables. */

TX_THREAD       client_thread;
NX_PACKET_POOL  client_pool;
NX_HTTP_CLIENT  my_client;
NX_IP           client_ip;
#define         CLIENT_PACKET_SIZE  (NX_HTTP_SERVER_MIN_PACKET_SIZE * 2)


/* Set up the HTTP server global variables */

NX_HTTP_SERVER  my_server;
NX_PACKET_POOL  server_pool;
TX_THREAD       server_thread;
NX_IP           server_ip;
NXD_ADDRESS     server_ip_address;
#define         SERVER_PACKET_SIZE  (NX_HTTP_SERVER_MIN_PACKET_SIZE * 2)
 
void            thread_client_entry(ULONG thread_input);
void            thread_server_entry(ULONG thread_input);

#define HTTP_SERVER_ADDRESS  IP_ADDRESS(1,2,3,4)
#define HTTP_CLIENT_ADDRESS  IP_ADDRESS(1,2,3,5)


/* Define the application's authentication check.  This is called by
   the HTTP server whenever a new request is received.  */
UINT  authentication_check(NX_HTTP_SERVER *server_ptr, UINT request_type, 
            CHAR *resource, CHAR **name, CHAR **password, CHAR **realm)
{
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(request_type);
    NX_PARAMETER_NOT_USED(resource);

    /* Just use a simple name, password, and realm for all 
       requests and resources.  */
    *name =     "name";
    *password = "password";
    *realm =    "NetX Duo HTTP demo";

    /* Request basic authentication.  */
    return(NX_HTTP_BASIC_AUTHENTICATE);
}

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

    /* Create a helper thread for the server. */
    tx_thread_create(&server_thread, "HTTP Server thread", thread_server_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the server packet pool.  */
    status =  nx_packet_pool_create(&server_pool, "HTTP Server Packet Pool", SERVER_PACKET_SIZE, 
                                    pointer, SERVER_PACKET_SIZE*4);

    pointer = pointer + SERVER_PACKET_SIZE * 4;

    /* Check for pool creation error.  */
    if (status)
    {

        return;
    }

    /* Create an IP instance.  */
    status = nx_ip_create(&server_ip, "HTTP Server IP", HTTP_SERVER_ADDRESS, 
                          0xFFFFFF00UL, &server_pool, _nx_ram_network_driver,
                          pointer, 4096, 1);

    pointer =  pointer + 4096;

    /* Check for IP create errors.  */
    if (status)
    {
        return;
    }

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for the server IP instance.  */
    status = nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check for ARP enable errors.  */
    if (status)
    {
        return;
    }
#endif /* NX_DISABLE_IPV4  */

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&server_ip);

    if (status)
    {
        return;
    }
#if (IP_TYPE==6) 

    /* Set up the server's IPv6 address here. */
    server_ip_address.nxd_ip_address.v6[3] = 0x105;
    server_ip_address.nxd_ip_address.v6[2] = 0x0;
    server_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    server_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V6;
#else

    /* Set up the server's IPv4 address here. */
    server_ip_address.nxd_ip_address.v4 = HTTP_SERVER_ADDRESS;
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;

#endif /* (IP_TYPE==6) */

    /* Create the HTTP Server.  */
    status = nx_http_server_create(&my_server, "My HTTP Server", &server_ip, &ram_disk, 
                          pointer, 2048, &server_pool, authentication_check, NX_NULL);
    if (status)
    {
        return;
    }

    pointer =  pointer + 2048;

    /* Create the HTTP Client thread. */
    status = tx_thread_create(&client_thread, "HTTP Client", thread_client_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Check for thread create error.  */
    if (status)
    {

        return;
    }

    /* Create the Client packet pool.  */
    status =  nx_packet_pool_create(&client_pool, "HTTP Client Packet Pool", SERVER_PACKET_SIZE, 
                                    pointer, SERVER_PACKET_SIZE*4);

    pointer = pointer + SERVER_PACKET_SIZE * 4;

    /* Check for pool creation error.  */
    if (status)
    {

        return;
    }


    /* Create an IP instance.  */
    status = nx_ip_create(&client_ip, "HTTP Client IP", HTTP_CLIENT_ADDRESS, 
                          0xFFFFFF00UL, &client_pool, _nx_ram_network_driver,
                          pointer, 2048, 1);

    pointer =  pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
    {
        return;
    }

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for the client IP instance.  */
    status = nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer =  pointer + 2048;

    /* Check for ARP enable errors.  */
    if (status)
    {
        return;
    }
#endif /* NX_DISABLE_IPV4  */

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&client_ip);

    return;
}


VOID thread_client_entry(ULONG thread_input)
{

UINT            status;
NX_PACKET       *my_packet;
#if (IP_TYPE == 6)
UINT           iface_index;
UINT           address_index;
NXD_ADDRESS    client_ip_address;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Format the RAM disk - the memory for the RAM disk was setup in 
      tx_application_define above.  This must be set up before the client(s) start
      sending requests. */
    status = fx_media_format(&ram_disk, 
                             _fx_ram_driver,         /* Driver entry               */
                             ram_disk_memory,        /* RAM disk memory pointer    */
                             media_memory,           /* Media buffer pointer       */
                             sizeof(media_memory),   /* Media buffer size          */
                             "MY_RAM_DISK",          /* Volume Name                */
                             1,                      /* Number of FATs             */
                             32,                     /* Directory Entries          */
                             0,                      /* Hidden sectors             */
                             256,                    /* Total sectors              */
                             128,                    /* Sector size                */
                             1,                      /* Sectors per cluster        */
                             1,                      /* Heads                      */
                             1);                     /* Sectors per track          */

    /* Check the media format status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, bail out.  */
        return ;
    }

    /* Open the RAM disk.  */
    status =  fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, media_memory, sizeof(media_memory));

    /* Check the media open status.  */
    if (status != FX_SUCCESS)
    {

        /* Error, bail out.  */
        return ;
    }

    /* Give IP task and driver a chance to initialize the system. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    /* Create an HTTP client instance.  */
    status = nx_http_client_create(&my_client, "HTTP Client", &client_ip, &client_pool, 600);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }


#if (IP_TYPE == 6)

    /* Set up the client IPv6 address here. */
    client_ip_address.nxd_ip_address.v6[3] = 0x101;
    client_ip_address.nxd_ip_address.v6[2] = 0x0;
    client_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    client_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    client_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Enable ICMPv6 services for the Client. */
    status = nxd_icmp_enable(&client_ip);
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Enable IPv6 services for the Client. */
    status = nxd_ipv6_enable(&client_ip);
    if (status != NX_SUCCESS)
    {
        return;
    }


    /* Register the Client link local and global IPv6 address with NetX Duo. */

    /* This assumes we are using the Server primary interface. See the NetX Duo
       User Guide for more information on address configuration. */

    /* This assumes we are using the Client primary interface. See the NetX Duo
       User Guide for more information on address configuration. */

    iface_index = 0;
    status = nxd_ipv6_address_set(&client_ip, iface_index, NX_NULL, 10, &address_index);
    status += nxd_ipv6_address_set(&client_ip, iface_index, &client_ip_address, 64, &address_index);


    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Wait for DAD to validate Client addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif /* (IP_TYPE == 6) */


    /* Now upload an HTML file to the HTTP IP server using the 'duo' service (supports IPv4 and IPv6). */
    status =  nxd_http_client_put_start(&my_client, &server_ip_address, "/client_test.htm", 
                                            "name", "password", 112, 5 * NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }


    /* Allocate a packet.  */
    status =  nx_packet_allocate(&client_pool, &my_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Build a simple 103-byte HTML page.  */
    nx_packet_data_append(my_packet, "<HTML>\r\n", 8, 
                        &client_pool, NX_WAIT_FOREVER);
    nx_packet_data_append(my_packet, 
                 "<HEAD><TITLE>NetX HTTP Test</TITLE></HEAD>\r\n", 44,
                        &client_pool, NX_WAIT_FOREVER);
    nx_packet_data_append(my_packet, "<BODY>\r\n", 8, 
                        &client_pool, NX_WAIT_FOREVER);
    nx_packet_data_append(my_packet, "<H1>Another NetX Test Page!</H1>\r\n", 34, 
                        &client_pool, NX_WAIT_FOREVER);
    nx_packet_data_append(my_packet, "</BODY>\r\n", 9,
                        &client_pool, NX_WAIT_FOREVER);
    nx_packet_data_append(my_packet, "</HTML>\r\n", 9,
                        &client_pool, NX_WAIT_FOREVER);

    /* Complete the PUT by writing the total length.  */
    status =  nx_http_client_put_packet(&my_client, my_packet, 50);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Now GET the test file  */


    /* Use the 'duo' service to send a GET request to the server (can use IPv4 or IPv6 addresses). */
    status =  nxd_http_client_get_start(&my_client, &server_ip_address, 
                                            "/client_test.htm", NX_NULL, 0, "name", "password", 50);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    status = nx_http_client_delete(&my_client);

    return;

}


/* Define the helper HTTP server thread.  */
void    thread_server_entry(ULONG thread_input)
{

UINT            status;
#if (IP_TYPE == 6)
UINT           iface_index;
UINT           address_index;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Give NetX a chance to initialize the system. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#if (IP_TYPE == 6)

    /* Here's where we make the HTTP server IPv6 enabled. */

    /* Enable ICMPv6 services for the Server. */
    status = nxd_icmp_enable(&server_ip);
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Enable IPv6 services for the Server. */
    status = nxd_ipv6_enable(&server_ip);
    if (status != NX_SUCCESS)
    {
        return;
    }


    /* Register the Server link local and global IPv6 address with NetX Duo. */

    /* This assumes we are using the Server primary interface. See the NetX Duo
       User Guide for more information on address configuration. */



    iface_index = 0;
    status = nxd_ipv6_address_set(&server_ip, iface_index, NX_NULL, 10, &address_index);
    status += nxd_ipv6_address_set(&server_ip, iface_index, &server_ip_address, 64, &address_index);


    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Wait for DAD to validate Server address. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);


#endif  /* (IP_TYPE == 6) */

    /* OK to start the HTTP Server.   */
    status = nx_http_server_start(&my_server);

    if (status != NX_SUCCESS)
    {
        return;
    }

    /* HTTP server ready to take requests! */

    /* Let the IP thread execute.    */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    return;
}



