/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX PPPoE Client stack Component                                     */
/**                                                                       */
/**   This is a small demo of the high-performance NetX PPPoE Client      */
/**   stack. This demo includes IP instance, PPPoE Client and PPP Client  */
/**   stack. Create one IP instance includes two interfaces to support    */
/**   for normal IP stack and PPPoE Client, PPPoE Client can use the      */
/**   mutex of IP instance to send PPPoE message when share one Ethernet  */
/**   driver. PPPoE Client work with normal IP instance at the same time. */
/**                                                                       */
/**   Note1: Substitute your Ethernet driver instead of                   */
/**   _nx_ram_network_driver before run this demo                         */
/**                                                                       */
/**   Note2: Prerequisite for using PPPoE.                                */
/**   Redefine NX_PHYSICAL_HEADER to 24 to ensure enough space for filling*/
/**   in physical header. Physical header:14(Ethernet header)             */
/**    + 6(PPPoE header) + 2(PPP header) + 2(four-byte aligment)          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


      /*****************************************************************/
      /*                          NetX Stack                           */
      /*****************************************************************/

                                            /***************************/
                                            /*        PPP Client       */
                                            /***************************/

                                            /***************************/
                                            /*       PPPoE Client      */
                                            /***************************/
      /***************************/         /***************************/
      /*    Normal Ethernet Type */         /*    PPPoE Ethernet Type  */
      /***************************/         /***************************/
      /***************************/         /***************************/
      /*       Interface 0       */         /*       Interface 1       */
      /***************************/         /***************************/

      /*****************************************************************/
      /*                       Ethernet Dirver                         */
      /*****************************************************************/
 
#include   "tx_api.h"
#include   "nx_api.h" 
#include   "nx_ppp.h"
#include   "nx_pppoe_client.h"

#ifndef NX_DISABLE_IPV4

/* Defined NX_PPP_PPPOE_ENABLE if using PPP, since PPP module has been modified to match PPPoE module under this definition.  */
#ifdef NX_PPP_PPPOE_ENABLE

/* If the driver is not initialized in other module, define NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE to initialize the driver in PPPoE module .  
   In this demo, the driver has been initialized in IP module.  */
#ifndef NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE

/* Define the block size.  */
#define     NX_PACKET_POOL_SIZE     ((1536 + sizeof(NX_PACKET)) * 30)
#define     DEMO_STACK_SIZE         2048
#define     PPPOE_THREAD_SIZE       2048

/* Define the ThreadX and NetX object control blocks...  */
TX_THREAD               thread_0;

/* Define the packet pool and IP instance for normal IP instnace.  */
NX_PACKET_POOL          pool_0;
NX_IP                   ip_0;
                
/* Define the PPP Client instance.  */
NX_PPP                  ppp_client;

/* Define the PPPoE Client instance.  */
NX_PPPOE_CLIENT         pppoe_client;

/* Define the counters.  */
CHAR                    *pointer;
ULONG                   error_counter; 

/* Define thread prototypes.  */
void    thread_0_entry(ULONG thread_input);

/***** Substitute your PPP driver entry function here *********/
extern void    _nx_ppp_driver(NX_IP_DRIVER *driver_req_ptr);

/***** Substitute your Ethernet driver entry function here *********/ 
extern void    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);

/* Define the porting layer function for PPP */
void    ppp_client_packet_send(NX_PACKET *packet_ptr);
void    pppoe_client_packet_receive(NX_PACKET *packet_ptr);

/* Define main entry point.  */

int main()
{
    
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}

UINT generate_login(CHAR *name, CHAR *password)
{

    /* Make a name and password, called "myname" and "mypassword".  */
    name[0] = 'm';
    name[1] = 'y';
    name[2] = 'n';
    name[3] = 'a';
    name[4] = 'm';
    name[5] = 'e';
    name[6] = (CHAR) 0;
    
    password[0] = 'm';
    password[1] = 'y';
    password[2] = 'p';
    password[3] = 'a';
    password[4] = 's';
    password[5] = 's';
    password[6] = 'w';
    password[7] = 'o';
    password[8] = 'r';
    password[9] = 'd';
    password[10] = (CHAR) 0;

    return(NX_SUCCESS);
}

/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

UINT    status;
                              
    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool for normal IP instance.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 
                                   (1536 + sizeof(NX_PACKET)), 
                                   pointer, NX_PACKET_POOL_SIZE); 
    pointer = pointer + NX_PACKET_POOL_SIZE;
                 
    /* Check for error.  */
    if (status)
        error_counter++;

    /* Create an normal IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance", IP_ADDRESS(192, 168, 100, 44), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Create the PPP instance.  */
    status = nx_ppp_create(&ppp_client, "PPP Instance", &ip_0, pointer, 2048, 1, &pool_0, NX_NULL, NX_NULL);
    pointer = pointer + 2048;

    /* Check for PPP create error.   */
    if (status)
        error_counter++;

    /* Set the PPP packet send function.  */
    status = nx_ppp_packet_send_set(&ppp_client, ppp_client_packet_send);

    /* Check for PPP packet send function set error.   */
    if (status)
        error_counter++;

    /* Define IP address. This PPP instance is effectively the client since it doesn't have any IP addresses. */
    status = nx_ppp_ip_address_assign(&ppp_client, IP_ADDRESS(0, 0, 0, 0), IP_ADDRESS(0, 0, 0, 0));
    
    /* Check for PPP IP address assign error.   */
    if (status)
        error_counter++;
        
    /* Setup PAP, this PPP instance is effectively the since it generates the name and password for the peer..  */
    status = nx_ppp_pap_enable(&ppp_client, generate_login, NX_NULL);

    /* Check for PPP PAP enable error.  */
    if (status)
        error_counter++;

    /* Attach an interface for PPP.  */
    status = nx_ip_interface_attach(&ip_0, "Second Interface For PPP", IP_ADDRESS(0, 0, 0, 0), 0, nx_ppp_driver);

    /* Check for error.  */
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for Normal IP Instance.  */
    status = nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024; 

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP */
    status = nx_icmp_enable(&ip_0);
    if(status)
        error_counter++;
    
    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&ip_0);
    if (status)
        error_counter++;

    /* Enable TCP traffic.  */
    status =  nx_tcp_enable(&ip_0);
    if (status)
        error_counter++;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

}

/* Define the test threads.  */

void    thread_0_entry(ULONG thread_input)
{
UINT    status;
ULONG   ip_status;

    /* Create the PPPoE instance.  */
    status =  nx_pppoe_client_create(&pppoe_client, (UCHAR *)"PPPoE Client",  &ip_0,  0, &pool_0, pointer, PPPOE_THREAD_SIZE, 4, _nx_ram_network_driver, pppoe_client_packet_receive);
    pointer = pointer + PPPOE_THREAD_SIZE;
    if (status)
    {
        error_counter++;
        return;
    }

    /* Establish PPPoE Client sessione.  */
    status = nx_pppoe_client_session_connect(&pppoe_client, NX_WAIT_FOREVER);
    if (status)
    {
        error_counter++;
        return;
    }

    /* Wait for the link to come up.  */
    status = nx_ip_interface_status_check(&ip_0, 1, NX_IP_ADDRESS_RESOLVED, &ip_status, NX_WAIT_FOREVER);
    if (status)
    {
        error_counter++;
        return;
    }

    /* Get the PPPoE Server physical address and Session ID after establish PPPoE Session.  */
    /*
    status = nx_pppoe_client_session_get(&pppoe_client, &server_mac_msw, &server_mac_lsw, &session_id);  
    if (status)
        error_counter++;
    */
}

/* PPPoE Client receive function.  */
void    pppoe_client_packet_receive(NX_PACKET *packet_ptr)
{

    /* Call PPP Client to receive the PPP data fame.  */
    nx_ppp_packet_receive(&ppp_client, packet_ptr);
}

/* PPP Client send function.  */
void    ppp_client_packet_send(NX_PACKET *packet_ptr)
{

    /* Directly Call PPPoE send function to send out the data through PPPoE module.  */
    nx_pppoe_client_session_packet_send(&pppoe_client, packet_ptr);
}
#endif /* NX_PPPOE_CLIENT_INITIALIZE_DRIVER_ENABLE  */

#endif /* NX_PPP_PPPOE_ENABLE  */

#endif /* NX_DISABLE_IPV4  */
