/* This is a small demo of NetX FTP on the high-performance NetX TCP/IP stack.  This demo 
   relies on ThreadX, NetX, and FileX to show a simple file transfer from the client 
   and then back to the server.  */



#include    "tx_api.h"
#include    "fx_api.h" 
#include    "nx_api.h"
#include    "nxd_ftp_client.h"
#include    "nxd_ftp_server.h"

#define     DEMO_STACK_SIZE         4096
#define     DEMO_DATA               "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
                  
#ifdef FEATURE_NX_IPV6
#define USE_IPV6
#endif /* FEATURE_NX_IPV6 */

/* Uncomment the following line to enable passive transfer mode.  */
/*
#define PASSIVE_MODE
*/

/* Uncomment the following line to enable block mode.  */
/*
#define BLOCK_MODE
*/


/* Define the ThreadX, NetX, and FileX object control blocks...  */

TX_THREAD               server_thread;
TX_THREAD               client_thread;
NX_PACKET_POOL          server_pool;
NX_IP                   server_ip;
NX_PACKET_POOL          client_pool;
NX_IP                   client_ip;
FX_MEDIA                ram_disk;


/* Define the NetX FTP object control blocks.  */

NX_FTP_CLIENT           ftp_client;
NX_FTP_SERVER           ftp_server;


/* Define the counters used in the demo application...  */

ULONG                   error_counter = 0;


/* Define the memory area for the FileX RAM disk.  */

UCHAR                   ram_disk_memory[32000];
UCHAR                   ram_disk_sector_cache[512];


#define FTP_SERVER_ADDRESS  IP_ADDRESS(1,2,3,4)
#define FTP_CLIENT_ADDRESS  IP_ADDRESS(1,2,3,5)

extern UINT  _fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                        CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors, 
                        ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster, 
                        UINT heads, UINT sectors_per_track);

/* Define the FileX and NetX driver entry functions.  */
VOID    _fx_ram_driver(FX_MEDIA *media_ptr);

/* Replace the 'ram' driver with your own Ethernet driver. */
VOID    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);


void    client_thread_entry(ULONG thread_input);
void    thread_server_entry(ULONG thread_input);

          
#ifdef USE_IPV6          
/* Define NetX Duo IP address for the NetX Duo FTP Server and Client. */
NXD_ADDRESS     server_ip_address;
NXD_ADDRESS     client_ip_address;   
#endif


/* Define server login/logout functions.  These are stubs for functions that would 
   validate a client login request.   */
           
#ifdef USE_IPV6
UINT    server_login6(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
UINT    server_logout6(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
#else
UINT    server_login(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
UINT    server_logout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info);
#endif


/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
    return(0);
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

UINT    status;
UCHAR   *pointer;

    /* Initialize NetX.  */
    nx_system_initialize();

    /* Initialize FileX.  */
    fx_system_initialize();

    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    /* Create a helper thread for the server. */
    tx_thread_create(&server_thread, "FTP Server thread", thread_server_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the packet pool for the FTP Server.  */
    status = nx_packet_pool_create(&server_pool, "NetX Server Packet Pool", 512, pointer, 8192);
    pointer = pointer + 8192;
    
    /* Check for errors.  */
    if (status)
        error_counter++;

    /* Create the IP instance for the FTP Server.  */
    status = nx_ip_create(&server_ip, "NetX Server IP Instance", FTP_SERVER_ADDRESS, 0xFFFFFF00UL, 
                                        &server_pool, _nx_ram_network_driver, pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for server IP instance.  */
    nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;
#endif /* NX_DISABLE_IPV4  */

    /* Enable TCP.  */
    nx_tcp_enable(&server_ip);
               
#ifdef USE_IPV6

    /* Next set the NetX Duo FTP Server and Client addresses. */
    server_ip_address.nxd_ip_address.v6[3] = 0x105;
    server_ip_address.nxd_ip_address.v6[2] = 0x0;
    server_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    server_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    client_ip_address.nxd_ip_address.v6[3] = 0x101;
    client_ip_address.nxd_ip_address.v6[2] = 0x0;
    client_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    client_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    client_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Create the FTP server.  */
    status =  nxd_ftp_server_create(&ftp_server, "FTP Server Instance", &server_ip, &ram_disk, pointer, DEMO_STACK_SIZE, &server_pool,
                                   server_login6, server_logout6);
#else
    /* Create the FTP server.  */
    status =  nx_ftp_server_create(&ftp_server, "FTP Server Instance", &server_ip, &ram_disk, pointer, DEMO_STACK_SIZE, &server_pool,
                                   server_login, server_logout);
#endif
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Now set up the FTP Client. */

    /* Create the main FTP client thread.  */
    status = tx_thread_create(&client_thread, "FTP Client thread ", client_thread_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE ;

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Create a packet pool for the FTP client.  */
    status =  nx_packet_pool_create(&client_pool, "NetX Client Packet Pool", 512, pointer, 8192);
    pointer =  pointer + 8192;

    /* Create an IP instance for the FTP client.  */
    status = nx_ip_create(&client_ip, "NetX Client IP Instance", FTP_CLIENT_ADDRESS, 0xFFFFFF00UL, 
                                                &client_pool, _nx_ram_network_driver, pointer, 2048, 1);
    pointer = pointer + 2048;

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for the FTP Client IP.  */
    nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;
#endif /* NX_DISABLE_IPV4  */

    /* Enable TCP for client IP instance.  */
    nx_tcp_enable(&client_ip);

    return;

}

/* Define the FTP client thread.  */

void    client_thread_entry(ULONG thread_input)
{

NX_PACKET   *my_packet;
NX_PACKET   *recv_packet_ptr;
UINT        status;
ULONG       file_size;

#ifdef USE_IPV6    
UINT        iface_index, address_index;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Format the RAM disk - the memory for the RAM disk was defined above.  */
    status = _fx_media_format(&ram_disk, 
                            _fx_ram_driver,                  /* Driver entry                */
                            ram_disk_memory,                 /* RAM disk memory pointer     */
                            ram_disk_sector_cache,           /* Media buffer pointer        */
                            sizeof(ram_disk_sector_cache),   /* Media buffer size           */
                            "MY_RAM_DISK",                   /* Volume Name                 */
                            1,                               /* Number of FATs              */
                            32,                              /* Directory Entries           */
                            0,                               /* Hidden sectors              */
                            256,                             /* Total sectors               */
                            128,                             /* Sector size                 */
                            1,                               /* Sectors per cluster         */
                            1,                               /* Heads                       */
                            1);                              /* Sectors per track           */

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Open the RAM disk.  */
    status = fx_media_open(&ram_disk, "RAM DISK", _fx_ram_driver, ram_disk_memory, ram_disk_sector_cache, sizeof(ram_disk_sector_cache));

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Let the IP threads and driver initialize the system.    */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);
                
#ifdef USE_IPV6
               
    /* Here's where we make the FTP Client IPv6 enabled. */
    status = nxd_ipv6_enable(&client_ip);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    status = nxd_icmp_enable(&client_ip); 

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }
    
    /* Set the Client link local and global addresses. */
    iface_index = 0;
    
    /* This assumes we are using the primary network interface (index 0). */
    status = nxd_ipv6_address_set(&client_ip, iface_index, NX_NULL, 10, &address_index);

    /* Check for link local address set error.  */
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
     }
    
     /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&client_ip, iface_index, &client_ip_address, 64, &address_index);

    /* Check for global address set error.  */
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
     }

    /* Let NetX Duo validate the addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
    
#endif /* USE_IPV6 */

    /* Create an FTP client.  */
    status =  nx_ftp_client_create(&ftp_client, "FTP Client", &client_ip, 2000, &client_pool);

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
     }

    printf("Created the FTP Client\n");

#ifdef USE_IPV6

    do
    {
    
        /* Now connect with the NetX Duo FTP (IPv6) server. */    
        status =  nxd_ftp_client_connect(&ftp_client, &server_ip_address, "name", "password", NX_IP_PERIODIC_RATE);
    } while (status != NX_SUCCESS);

#else

    /* Now connect with the NetX FTP (IPv4) server. */
    status =  nx_ftp_client_connect(&ftp_client, FTP_SERVER_ADDRESS, "name", "password", NX_IP_PERIODIC_RATE);

#endif /* USE_IPV6 */

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
     }

    printf("Connected to the FTP Server\n");

#ifdef PASSIVE_MODE
    /* Enable passive mode. */
    status = nx_ftp_client_passive_mode_set(&ftp_client, NX_TRUE);
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
    }

    printf("Enabled FTP Client Passive Mode\n");
#endif /* PASSIVE_MODE  */

#ifdef BLOCK_MODE
    /* Set block mode. */
    status = nx_ftp_client_transfer_mode_set(&ftp_client, NX_FTP_TRANSFER_MODE_BLOCK);
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
    }

    printf("Enabled FTP Client Block Mode\n");
#endif /* BLOCK_MODE  */

    /* Open a FTP file for writing.  */
    status =  nx_ftp_client_file_open(&ftp_client, "test.txt", NX_FTP_OPEN_FOR_WRITE, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
     }

    printf("Opened the FTP client test.txt file\n");

#ifdef BLOCK_MODE
    /* Set the file size in block mode.  */
    status =  nx_ftp_client_file_size_set(&ftp_client, 28);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
    }
    else
        printf("Set the File Size in Block Mode\n");
#endif /* BLOCK_MODE  */

    /* Allocate a FTP packet.  */
    status =  nx_packet_allocate(&client_pool, &my_packet, NX_TCP_PACKET, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
     }

    /* Write ABCs into the packet payload!  */
    memcpy(my_packet -> nx_packet_prepend_ptr, DEMO_DATA, sizeof(DEMO_DATA)); /* Use case of memcpy is verified. */

    /* Adjust the write pointer.  */
    my_packet -> nx_packet_length =  sizeof(DEMO_DATA);
    my_packet -> nx_packet_append_ptr =  my_packet -> nx_packet_prepend_ptr + sizeof(DEMO_DATA);

    /* Write the packet to the file test.txt.  */
    status =  nx_ftp_client_file_write(&ftp_client, my_packet, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        nx_packet_release(my_packet);
        return;
    }
    else
        printf("Wrote to the FTP client test.txt file\n");


    /* Close the file.  */
    status =  nx_ftp_client_file_close(&ftp_client, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }
    else
        printf("Closed the FTP client test.txt file\n");
    

    /* Now open the same file for reading.  */
    status =  nx_ftp_client_file_open(&ftp_client, "test.txt", NX_FTP_OPEN_FOR_READ, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;        
        return;
    }
    else
        printf("Reopened the FTP client test.txt file\n");

    /* Loop to read the data.  */
    file_size = 0;
    do
    {

        /* Read the file.  */
        status =  nx_ftp_client_file_read(&ftp_client, &my_packet, 100);

        /* Check status.  */
        if (status == NX_SUCCESS)
        {
            file_size += my_packet -> nx_packet_length;
            nx_packet_release(my_packet);
        }
    } while(status == NX_SUCCESS);

    /* Check file size.  */
    if (file_size != 28)
    {
        error_counter++;
        return;
    }
    else
    {
        printf("Reread the FTP client test.txt file\n");
    }

    /* Close this file.  */
    status =  nx_ftp_client_file_close(&ftp_client, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }
    else
        printf("Reclosed the FTP client test.txt file\n");

    /* Get directory listing (NLST). */
    status = nx_ftp_client_directory_listing_get(&ftp_client, "", &my_packet, 200);
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
    }
    else
    {
        nx_packet_release(my_packet);
    }

    do
    {
        /* Receive the next data packet. */
        status =  nx_ftp_client_directory_listing_continue(&ftp_client, &recv_packet_ptr, 200);
        if (status == NX_SUCCESS)
        {
            nx_packet_release(recv_packet_ptr);
        }

        /* Check if this is the end of the download. */
        if (status == NX_FTP_END_OF_LISTING)
          break;

    } while (status == NX_SUCCESS);

    printf("Got the directory list test.txt\n");

    /* Disconnect from the server.  */
    status =  nx_ftp_client_disconnect(&ftp_client, NX_IP_PERIODIC_RATE);

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {
        error_counter++;
        return;
     }

    /* Delete the FTP client.  */
    status =  nx_ftp_client_delete(&ftp_client);

    /* Check status.  */
    if (status != NX_SUCCESS)   
    {
        error_counter++;
        return;
     }
}


/* Define the helper FTP server thread.  */
void    thread_server_entry(ULONG thread_input)
{

UINT            status;    
#ifdef  USE_IPV6    
UINT            iface_index, address_index;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Wait till the IP thread and driver have initialized the system. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);
        
#ifdef USE_IPV6

    /* Here's where we make the FTP server IPv6 enabled. */
    status = nxd_ipv6_enable(&server_ip);

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
     }

    status = nxd_icmp_enable(&server_ip); 

    /* Check status.  */
    if (status != NX_SUCCESS) 
    {

        error_counter++;
        return;
     }

     /* Set the link local address with the host MAC address. */
    iface_index = 0;
    
    /* This assumes we are using the primary network interface (index 0). */
    status = nxd_ipv6_address_set(&server_ip, iface_index, NX_NULL, 10, &address_index);

    /* Check for link local address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }

    /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&server_ip, iface_index, &server_ip_address, 64, &address_index);

    /* Check for global address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }
    
    /* Wait while NetX Duo validates the link local and global address. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif /* USE_IPV6 */

    /* OK to start the FTP Server.   */
    status = nx_ftp_server_start(&ftp_server);

    if (status != NX_SUCCESS)
        error_counter++;

    printf("Server started!\n");

    /* FTP server ready to take requests! */

    /* Let the IP threads execute.    */
    tx_thread_relinquish();

    return;
}

           
#ifdef USE_IPV6
UINT  server_login6(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, 
                   CHAR *name, CHAR *password, CHAR *extra_info)
{
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(client_ipduo_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(name);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(extra_info);

    printf("Logged in6!\n");

    /* Always return success.  */
    return(NX_SUCCESS);
}

UINT  server_logout6(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, NXD_ADDRESS *client_ipduo_address, UINT client_port, 
                    CHAR *name, CHAR *password, CHAR *extra_info)
{ 
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(client_ipduo_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(name);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(extra_info);

    printf("Logged out6!\n");

    /* Always return success.  */
    return(NX_SUCCESS);
}
#else 
UINT  server_login(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(client_ip_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(name);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(extra_info);

    printf("Logged in!\n");
    /* Always return success.  */
    return(NX_SUCCESS);
}

UINT  server_logout(struct NX_FTP_SERVER_STRUCT *ftp_server_ptr, ULONG client_ip_address, UINT client_port, CHAR *name, CHAR *password, CHAR *extra_info)
{
    NX_PARAMETER_NOT_USED(ftp_server_ptr);
    NX_PARAMETER_NOT_USED(client_ip_address);
    NX_PARAMETER_NOT_USED(client_port);
    NX_PARAMETER_NOT_USED(name);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(extra_info);

    printf("Logged out!\n");

    /* Always return success.  */
    return(NX_SUCCESS);
}
#endif  /* USE_IPV6 */
