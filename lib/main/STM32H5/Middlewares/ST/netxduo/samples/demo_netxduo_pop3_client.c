/* 
   This is a small demo of POP3 Client on the high-performance NetX TCP/IP stack.  
   This demo relies on Thread, NetX and POP3 Client API to conduct 
   a POP3 mail session.  
 */
/* Note: This demo works for IPv4 only. */

#include  "tx_api.h"
#include  "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include  "nxd_pop3_client.h"

#define DEMO_STACK_SIZE             4096
#define CLIENT_ADDRESS              IP_ADDRESS(192,2,2,61)
#define SERVER_ADDRESS              IP_ADDRESS(192,2,2,89)  
#define SERVER_PORT                 110


/* Replace the 'ram' driver with your own Ethernet driver. */
void    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Set up the POP3 Client.  */

TX_THREAD           demo_client_thread;
NX_POP3_CLIENT      demo_client;
NX_PACKET_POOL      client_packet_pool;
NX_IP               client_ip;

/* Use the maximum size payload to insure no packets are dropped. */
#define PAYLOAD_SIZE 1460

/* Set up Client thread entry point. */
void    demo_thread_entry(ULONG info);


  /* Shared secret is the same as password. */

#define LOCALHOST                               "recipient@domain.com" 
#define LOCALHOST_PASSWORD                      "testpwd" 


/* Define main entry point.  */
int main()
{
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UINT     status;
UCHAR    *free_memory_pointer;


    /* Setup the working pointer.  */
    free_memory_pointer =  first_unused_memory;

    /* Create a client thread.  */
    tx_thread_create(&demo_client_thread, "Client", demo_thread_entry, 0,  
                     free_memory_pointer, DEMO_STACK_SIZE, 1, 1, 
                     TX_NO_TIME_SLICE, TX_AUTO_START);

    free_memory_pointer =  free_memory_pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* The demo client username and password is the authentication 
       data used when the server attempts to authentication the client. */

    /* Create Client packet pool. */
    status =  nx_packet_pool_create(&client_packet_pool, "POP3 Client Packet Pool", 
                                    PAYLOAD_SIZE, free_memory_pointer, (PAYLOAD_SIZE * 10));
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer = free_memory_pointer + (PAYLOAD_SIZE * 10);    


    /* Create IP instance for demo Client */
    status = nx_ip_create(&client_ip, "POP3 Client IP Instance", CLIENT_ADDRESS, 0xFFFFFF00UL, 
                          &client_packet_pool, _nx_ram_network_driver, free_memory_pointer, 
                          2048, 1);

    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer =  free_memory_pointer + 2048;

    /* Enable ARP and supply ARP cache memory. */
    nx_arp_enable(&client_ip, (void *) free_memory_pointer, 1024);

    /* Update pointer to unallocated (free) memory. */
    free_memory_pointer = free_memory_pointer + 1024;
    
    /* Enable TCP and ICMP for Client IP. */
    nx_tcp_enable(&client_ip);
    nx_icmp_enable(&client_ip);

    return;
}



/* Define the application thread entry function. */

void    demo_thread_entry(ULONG info)
{

UINT        status;
UINT        mail_item, number_mail_items; 
UINT        bytes_downloaded = 0;
UINT        final_packet = NX_FALSE;
ULONG       total_size, mail_item_size, bytes_retrieved;
NX_PACKET   *packet_ptr;

    NX_PARAMETER_NOT_USED(info);

    /* Let the IP instance get initialized with driver parameters. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);


    /* Create a NetX POP3 Client instance with no byte or block memory pools. 
       Note that it uses its password for its APOP shared secret. */
    status =  nx_pop3_client_create(&demo_client,
                                    NX_TRUE /* if true, enables Client to send APOP command to authenticate */,
                                    &client_ip, &client_packet_pool, SERVER_ADDRESS, SERVER_PORT, 
                                    LOCALHOST, LOCALHOST_PASSWORD);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {

        status = nx_pop3_client_delete(&demo_client);

        /* Abort. */
        return;
    }

    /* Find out how many items are in our mailbox.  */
    status = nx_pop3_client_mail_items_get(&demo_client, &number_mail_items, &total_size);

    printf("Got %d mail items, total size%ld \n", number_mail_items, total_size);

    /* If nothing in the mailbox, disconnect. */
    if (number_mail_items == 0)
    {

        nx_pop3_client_delete(&demo_client);

        return;
    }

    /* Download all mail items.  */
    mail_item = 1; 

    while (mail_item <= number_mail_items)
    {

        /* This submits a RETR request and gets the mail message size. */
        status = nx_pop3_client_mail_item_get(&demo_client, mail_item, &mail_item_size);

        /* Loop to get the next mail message packet until the mail item is completely
           downloaded. */
        do
        {

            status = nx_pop3_client_mail_item_message_get(&demo_client, &packet_ptr, 
                                                        &bytes_retrieved, 
                                                        &final_packet); 

            if (status != NX_SUCCESS)
            {

                break;
            }

            if (bytes_retrieved != 0)
            {

                printf("Received %ld bytes of data for item %d: %s\n", packet_ptr -> nx_packet_length, mail_item, packet_ptr -> nx_packet_prepend_ptr);
            }

            nx_packet_release(packet_ptr);

            /* Determine if this is the last data packet. */
            if (final_packet)
            {
                /* It is. Let the server know it can delete this mail item. */
                status = nx_pop3_client_mail_item_delete(&demo_client, mail_item);

                if (status != NX_SUCCESS)
                {

                    break;
                }
            }

            /* Keep track of how much mail message data is left. */
            bytes_downloaded += bytes_retrieved;

        } while (final_packet == NX_FALSE);

        /* Get the next mail item. */
        mail_item++;

        tx_thread_sleep(NX_IP_PERIODIC_RATE);

    }

    /* Disconnect from the POP3 server. */
    status = nx_pop3_client_quit(&demo_client);

    /* Delete the POP3 Client.  This will not delete the packet pool used by the POP3 Client to 
       transmit messages. */
    status = nx_pop3_client_delete(&demo_client);

}
#endif /* NX_DISABLE_IPV4  */
