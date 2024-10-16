/* 
   This is a small demo of the NetX SMTP Client on the high-performance NetX TCP/IP stack.  
   This demo relies on Thread, NetX and SMTP Client API to perform simple SMTP mail 
   transfers in an SMTP client application to an SMTP mail server.  
 */
/* Note: This demo works for IPv4 only. */

#include "nx_api.h"
#include "nx_ip.h"
#include "nxd_smtp_client.h"


#ifndef NX_DISABLE_IPV4
/* Define the host user name and mail box parameters */
#define USERNAME               "myusername"
#define PASSWORD               "mypassword"
#define FROM_ADDRESS           "my@mycompany.com"
#define RECIPIENT_ADDRESS      "your@yourcompany.com"
#define LOCAL_DOMAIN           "mycompany.com"

#define SUBJECT_LINE           "NetX SMTP Client Demo" 
#define MAIL_BODY              "NetX SMTP client is a simple SMTP client implementation  \r\n" \
                               "that allow embedded devices to send email to an SMTP server. \r\n" \
                               "This feature is intended to allow a device to send simple status\r\n " \
                               "reports using the most universal Internet application, email.\r\n"


/* See the NetX SMTP Client User Guide for how to set the authentication type.  
   The most common authentication type is PLAIN. */
#define CLIENT_AUTHENTICATION_TYPE  NX_SMTP_CLIENT_AUTH_PLAIN


#define CLIENT_IP_ADDRESS  IP_ADDRESS(1,2,3,5)
#define SERVER_IP_ADDRESS  IP_ADDRESS(1,2,3,4)
#define SERVER_PORT        25


/* Define the NetX and ThreadX structures for the SMTP client appliciation. */
NX_PACKET_POOL                  ip_packet_pool;
NX_PACKET_POOL                  client_packet_pool;
NX_IP                           client_ip;
TX_THREAD                       demo_client_thread;
static NX_SMTP_CLIENT           demo_client;


void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);
void    demo_client_thread_entry(ULONG info);

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
CHAR    *free_memory_pointer;


    /* Setup the pointer to unallocated memory.  */
    free_memory_pointer =  (CHAR *) first_unused_memory;

    /* Create IP default packet pool. This packets do not need a very large payload. */
    status =  nx_packet_pool_create(&ip_packet_pool, "Default IP Packet Pool", 
                                    512, free_memory_pointer, 2048);

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer = free_memory_pointer + 2048;    

    /* Create SMTP Client packet pool. This is only for transmitting packets to the server.
       It need not be a separate packet pool than the IP default packet pool but for more efficient
       resource use, we use two different packet pools because the CLient SMTP messages
       generally require more payload than IP control packets. 

       Packet payload depends on the SMTP Client application requirements.  Size of packet payload 
       must include IP and TCP headers. For IPv6 connections, IP and TCP header data is 60 bytes. For IPv4  
       IP and TCP header data is 40 bytes (not including TCP options). 
    */

    status |=  nx_packet_pool_create(&client_packet_pool, "SMTP Client Packet Pool", 
                                     800, free_memory_pointer, (10*800));

    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer = free_memory_pointer + (10*800);    

    /* Initialize the NetX system. */
    nx_system_initialize();

    /* Create the client thread */
    status = tx_thread_create(&demo_client_thread, "client_thread",
                              demo_client_thread_entry, 0, free_memory_pointer,
                              2048, 16, 16,
                              TX_NO_TIME_SLICE, TX_DONT_START);

    if (status != NX_SUCCESS)
    {

        printf("Error creating Client thread. Status 0x%x\r\n", status);
        return;       
    }

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer =  free_memory_pointer + 4096;


    /* Create Client IP instance. Remember to replace the generic driver
       with a real ethernet driver to actually run this demo! */
    status = nx_ip_create(&client_ip, "SMTP Client IP Instance", CLIENT_IP_ADDRESS, 0xFFFFFF00UL, 
                          &ip_packet_pool, _nx_ram_network_driver, free_memory_pointer, 
                          2048, 1);
   
    free_memory_pointer =  free_memory_pointer + 2048;

    /* Enable ARP and supply ARP cache memory. */
    status =  nx_arp_enable(&client_ip, (void **) free_memory_pointer, 1040);

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer = free_memory_pointer + 1040;
    
    /* Enable TCP for client. */
    status =  nx_tcp_enable(&client_ip);

    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Enable ICMP for client. */
    status =  nx_icmp_enable(&client_ip);

    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Start the client thread. */
    tx_thread_resume(&demo_client_thread);

    return;
}


/* Define the smtp application thread task.   */
void    demo_client_thread_entry(ULONG info)
{

UINT        status;
UINT        error_counter = 0;
NXD_ADDRESS server_ip_address;

    NX_PARAMETER_NOT_USED(info);

    tx_thread_sleep(NX_IP_PERIODIC_RATE);

    /* Set up the server IP address. */
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip_address.nxd_ip_address.v4 = SERVER_IP_ADDRESS;

    /* The demo client username and password is the authentication 
       data used when the server attempts to authentication the client. */

    status =  nxd_smtp_client_create(&demo_client, &client_ip, &client_packet_pool,
                                     USERNAME, 
                                     PASSWORD,
                                     FROM_ADDRESS,
                                     LOCAL_DOMAIN, CLIENT_AUTHENTICATION_TYPE, 
                                     &server_ip_address, SERVER_PORT);

    if (status != NX_SUCCESS)
    {
        printf("Error creating the client. Status: 0x%x.\n\r", status);
        return;
    }

    /* Create a mail instance with the above text message and recipient info. */
    status =  nx_smtp_mail_send(&demo_client, RECIPIENT_ADDRESS, NX_SMTP_MAIL_PRIORITY_NORMAL, 
                                SUBJECT_LINE, MAIL_BODY, sizeof(MAIL_BODY) - 1);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        
        /* Mail item was not sent. Note that we need not delete the client. The error status may be a failed
           authentication check or a broken connection.  We can simply call nx_smtp_mail_send
           again.  */
        
        error_counter++;
    }

    /* Release resources used by client. Note that the transmit packet
       pool must be deleted by the application if it no longer has use for it.*/
    status = nx_smtp_client_delete(&demo_client);

    /* Check for errors. */
    if (status != NX_SUCCESS)
    {
        error_counter++;
    }

    return;
}

#endif /* NX_DISABLE_IPV4 */
