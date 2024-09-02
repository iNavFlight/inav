/* This is a small demo of the high-performance NetX TCP/IP stack.  This demo concentrates
   on UDP packet sending and receiving - with ARP - using a simulated Ethernet driver.  */
/* This demo works for IPv4 only */

#include   "tx_api.h"
#include   "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include   "nxd_bsd.h"
#include   <string.h>
#include   <stdlib.h>

#define     DEMO_STACK_SIZE         2048
#define     SERVER_PORT             721

/* Message sent to the server */
#define CLIENT_MESSAGE "Client BSD UDP Socket testing\n"


/* Define the ThreadX and NetX object control blocks...  */

TX_THREAD               thread_server;
TX_THREAD               thread_client;
NX_PACKET_POOL          bsd_pool;
NX_IP                   bsd_ip;


/* Define the counters used in the demo application...  */

ULONG                  thread_0_counter;
ULONG                  thread_2_counter;
ULONG                  error_counter;

/* Define thread prototypes.  */

VOID    thread_server_entry(ULONG thread_input);
VOID    thread_client_entry(ULONG thread_input);
VOID    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


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

    /* Create a thread for the Server  */
    tx_thread_create(&thread_server, "Server", thread_server_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create a thread for client.  */
    tx_thread_create(&thread_client, "Client", thread_client_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            16, 16, TX_NO_TIME_SLICE, TX_AUTO_START);
    
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a BSD packet pool.  */
    status =  nx_packet_pool_create(&bsd_pool, "NetX BSD Packet Pool", 128, pointer, 16384);
    
    pointer = pointer + 16384;   
    if (status)
    {
        error_counter++;
        printf("Error in creating BSD packet pool\n!");
    }
      
    /* Create an IP instance for BSD.  */
    status += nx_ip_create(&bsd_ip, "NetX IP Instance 2", IP_ADDRESS(1, 2, 3, 4), 0xFFFFFF00UL,  
                           &bsd_pool, _nx_ram_network_driver,
                           pointer, DEMO_STACK_SIZE, 1);
    
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Check for any errors */
    if (status)
        error_counter++;
    
    if (status)
        printf("Error creating BSD IP instance\n!");
    
    /* Enable ARP and supply ARP cache memory for BSD IP Instance */
    status +=  nx_arp_enable(&bsd_ip, (void *) pointer, 1024);
    
    pointer = pointer + 1024; 

    /* Check ARP enable status.  */     
    if (status)
        error_counter++;

    if (status)
        printf("Error in Enable ARP and supply ARP cache memory to BSD IP instance\n");

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&bsd_ip);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Now initialize BSD Socket Wrapper */
    status = (UINT)bsd_initialize (&bsd_ip, &bsd_pool, pointer, 2048, 2);

    if (status)
        error_counter++;

}


void    thread_server_entry(ULONG thread_input)
{

    /* Start the Server */
INT          status,addrlen1;
INT          sock_udp_2;
struct       sockaddr_in echoServAddr;               /* Echo server address */
struct       sockaddr_in fromAddr;                   /* Cleint address */
CHAR         buffer[64];

    NX_PARAMETER_NOT_USED(thread_input);

    /* Create a BSD UDP Server Socket */
    sock_udp_2 = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    /* Check for any errors */
    if (sock_udp_2 == -1)
    {
        printf("Error: BSD UDP Servert socket create\n");
        return;
    }   

     /* Fill ths socket with Server side information */
     memset(&echoServAddr, 0, sizeof(echoServAddr));
     echoServAddr.sin_family = PF_INET;
     echoServAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
     echoServAddr.sin_port = htons(SERVER_PORT);

     /* Bind the UDP socket to the IP port.  */
     status = bind (sock_udp_2, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr));
     if (status < 0)
     {
         printf("Error: BSD UDP Server Socket Bind\n");
         return;
     }   
     
     while(1)
     {  

         addrlen1 = sizeof(struct sockaddr_in);

         /* Receive a UDP packet from a client. */
         status = recvfrom(sock_udp_2,(VOID *)buffer, 64, 0,(struct sockaddr *) &fromAddr, &addrlen1);

             /* Check for any errors */
         if (status == ERROR)
            printf("Error: BSD Server Socket receive\n");
         else
         {

             /* Print client information */
             printf("Server received data from Client at IP address 0x%x at port %lu\n", 
                    (UINT)fromAddr.sin_addr.s_addr, (ULONG)fromAddr.sin_port);

            /* Print the packet received from the client */
            printf("Server received from Client: %s\n", buffer);

            /* Now echo the recieved data string to the same client */
            status = sendto(sock_udp_2, buffer, (INT)(status + 1), 0, (struct sockaddr *) &fromAddr, sizeof(fromAddr));

            /* Check for any errors */
            if (status == ERROR)
                printf("Error:BSD Server Socket echo\n");
         }   

         tx_thread_sleep(NX_IP_PERIODIC_RATE);

        /* All done , loop back to recieve next packet */
    }  
}

/* Define the Client thread.  */
void    thread_client_entry(ULONG thread_input)
{

INT          status,sock_udp_1;                                 
INT          addrlen;
struct       sockaddr_in destAddr;                       /* Destination address */
struct       sockaddr_in ClientAddr;                     /* Client address */
struct       sockaddr_in fromAddr;                       /* Cleint address */
CHAR         echomsg[64];                                /* Message echoed by the server */

    NX_PARAMETER_NOT_USED(thread_input);
    
    /* Allow Server side to set up */
    tx_thread_sleep(10);
    thread_2_counter =0;
    
    while(1)
    {
         
         /* Create a BSD UDP Client Socket */
         sock_udp_1 = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
         /* Check for any errors */
         if (sock_udp_1 == ERROR)
         {
             printf("Error: BSD UDP Client socket create\n");
             return;
         }
         
         /* Set up client side */
         memset(&ClientAddr, 0, sizeof(ClientAddr));
         ClientAddr.sin_family = PF_INET;
         ClientAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
    
         /* Assign a known Server port */
         memset(&destAddr, 0, sizeof(destAddr));
         destAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
         destAddr.sin_port = htons(SERVER_PORT);
         destAddr.sin_family = PF_INET;
    
         /* Client socket is ready now start sending packets */
         printf("BSD Client Socket (%lu) sending test message: %s\n", (ULONG)sock_udp_1, CLIENT_MESSAGE);
    
         status = sendto(sock_udp_1, CLIENT_MESSAGE, (INT)(sizeof(CLIENT_MESSAGE)), 0,
                         (struct sockaddr *) &destAddr, sizeof(destAddr));

         /* Check for any errors */
         if (status == ERROR)
         {    

             printf("Error: BSD Client Socket send\n");
         }     
         else
         {
    
             addrlen = sizeof(struct sockaddr_in);
    
             /* Try to receive an echo from the server. */
             status = recvfrom(sock_udp_1,(VOID *)echomsg,64, 0, (struct sockaddr *) &fromAddr, &addrlen);
    
             /* * Check for any errors */
             if (status == ERROR)
             {    

                 printf("Error: BSD Client Socket echo receive\n");
             }     
             else
             {
    
                 /* Print Server detials */
                 printf("Client contacted Server at IP address 0x%x and port %lu\n", 
                        (UINT)fromAddr.sin_addr.s_addr, (ULONG)fromAddr.sin_port);

                 /* Print the received echo string from the server */
                 printf("Client (%lu) received echo string: %s\n", (ULONG)sock_udp_1, echomsg);
             }
         }   
    
         /* Done with this client socket */
         status = soc_close(sock_udp_1);
    
         /* Check for any errors */
         if (status == ERROR)
         {    
             printf("Error: BSD Client Socket close\n");
         } 
    
        /* Increment client transaction counter.  */
        thread_2_counter++;

        tx_thread_sleep(NX_IP_PERIODIC_RATE);       
    }
}
#endif /* NX_DISABLE_IPV4 */
