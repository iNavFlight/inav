/* This is a small demo of the high-performance BSD socket using the NetX Duo TCP/IP stack.  This 
   demo concentrates on raw packet sending and receiving using a simulated Ethernet driver.  */

#include   "tx_api.h"
#include   "nx_api.h"
#include   "nxd_bsd.h"
#include   "nx_ipv6.h"
#include   <string.h>
#include   <stdlib.h>

#define     DEMO_STACK_SIZE         2048
#define     CLIENT0_IP_ADDRESS       IP_ADDRESS(1,2,3,4)
#define     CLIENT1_IP_ADDRESS       IP_ADDRESS(1,2,3,4)
#define     MESSAGE                  "Client 0 BSD Raw Socket testing\n"

/* Define the ThreadX and NetX Duo object control blocks...  */

TX_THREAD               thread_0;
TX_THREAD               thread_1;
NX_PACKET_POOL          bsd_pool;
NX_IP                   bsd_ip;

#ifdef NX_DISABLE_IPV4
/* To send and receive raw packets over IPv6, define this macro.  */
#define USE_IPV6
#endif /* NX_DISABLE_IPV4 */

/* Define thread prototypes.  */

VOID    thread_0_entry(ULONG thread_input);
VOID    thread_1_entry(ULONG thread_input);
void    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);


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

    /* Create a thread for client 0.  */
    tx_thread_create(&thread_0, "Client 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            2, 2, TX_NO_TIME_SLICE, TX_AUTO_START);
    
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create a thread for client 1.  */
    tx_thread_create(&thread_1, "Client 1", thread_1_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            2, 2, TX_NO_TIME_SLICE, TX_AUTO_START);
    
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a BSD packet pool.  */
    status =  nx_packet_pool_create(&bsd_pool, "NetX BSD Packet Pool", 128, pointer, 16384);
    
    pointer = pointer + 16384;   
    if (status != NX_SUCCESS)
    {
        printf("Error in creating BSD packet pool : 0x%x\n", status);
        return;
    }
      
    /* Create an IP instance for BSD.  */
    status += nx_ip_create(&bsd_ip, "NetX IP Instance 2", CLIENT0_IP_ADDRESS, 0xFFFFFF00UL,  
                           &bsd_pool, _nx_ram_network_driver,
                           pointer, DEMO_STACK_SIZE, 1);
    
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Check for any errors */
    if (status != NX_SUCCESS)
    {
        return;
    }
    
#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for BSD IP Instance */
    status =  nx_arp_enable(&bsd_ip, (void *) pointer, 1024);
    
    pointer = pointer + 1024; 
#endif /* NX_DISABLE_IPV4 */

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&bsd_ip);

    /* Now initialize BSD Socket Wrapper */
    status = (UINT)bsd_initialize (&bsd_ip, &bsd_pool, pointer, DEMO_STACK_SIZE, 2);

    /* Check for any errors */
    if (status != NX_SUCCESS)
    {
        return;
    }
}
/* Define Client 0. */
void    thread_0_entry(ULONG thread_input)
{

INT          sock_raw;
INT          socket_family;
INT          bytes_sent;
#ifdef USE_IPV6
NXD_ADDRESS  client1_ip_address;
UINT         if_index, address_index;
struct       sockaddr_in6 ClientAddr;                     /* Client address */
struct       sockaddr_in6 destAddr;                       /* Destination address */
#else
struct       sockaddr_in destAddr;                       /* Destination address */
struct       sockaddr_in ClientAddr;                     /* Client address */
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow the network driver to initialize NetX Duo. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifndef USE_IPV6

    /* Set up IPv4 client and peer addressing.  */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin_family = PF_INET;
    ClientAddr.sin_addr.s_addr = htonl(CLIENT0_IP_ADDRESS);

    /* Assign a known Server port */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_addr.s_addr = htonl(CLIENT1_IP_ADDRESS);
    destAddr.sin_family = PF_INET;

    socket_family =  AF_INET;

#else

    /* Set up IPv6 client and peer addressing.  */

    /* Enable IPv6 for this IP instance. */
    nxd_ipv6_enable(&bsd_ip);

    /* Enable ICMPv6 next. */
    nxd_icmp_enable(&bsd_ip);

    /* Create a global address for the host. */
    client1_ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    client1_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    client1_ip_address.nxd_ip_address.v6[1] = 0xf101;
    client1_ip_address.nxd_ip_address.v6[2] = 0;
    client1_ip_address.nxd_ip_address.v6[3] = 0x101;

    socket_family =  AF_INET6;

    /* Set the primary interface (index 0) where we will set the global address. */
    if_index = 0;

    /* Now we are ready to set the global address. This will also return the index into
       the IP address table where NetX Duo inserted the global address. */
    nxd_ipv6_address_set(&bsd_ip, if_index, &client1_ip_address, 64, &address_index);

    /* Set up client side */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:0db8:0:f101::101", &ClientAddr.sin6_addr._S6_un._S6_u32);

    /* Set up destination address */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:0db8:0:f101::101", &destAddr.sin6_addr._S6_un._S6_u32);

    /* Allow time for NetX Duo to validate host IPv6 addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif  /* USE_IPV6 */

    while (1)
    {
    
         /* Create a BSD raw Client Socket */
         sock_raw = socket(socket_family, SOCK_RAW, 0x99);

         /* Check for any errors */
         if (sock_raw == ERROR)
         {
             printf("Error: BSD Client socket create\n");
             return;
         }

         /* Client 1 is ready to send a packet. */

         bytes_sent = sendto(sock_raw, MESSAGE, (INT)(sizeof(MESSAGE)), 0, (struct sockaddr *) &destAddr, sizeof(destAddr));

         if (bytes_sent > 0)
         {
             printf("Client 0 sent a packet\n");
         }
          
         printf("Close Client0 socket.\n");

         /* Close the Client 0 socket */
         soc_close(sock_raw);

         tx_thread_sleep(NX_IP_PERIODIC_RATE);
    }
}


/* Define Client 1. */
void    thread_1_entry(ULONG thread_input)
{

INT          addrlen;
INT          sock_raw;
CHAR         recv_buffer[64];                                
INT          socket_family;
INT          bytes_received;
#ifdef USE_IPV6
struct       sockaddr_in6 ClientAddr;                     /* Client address */
struct       sockaddr_in6 destAddr;                       /* Destination address */
struct       sockaddr_in6 fromAddr;                       /* Cleint address */
#else
struct       sockaddr_in destAddr;                       /* Destination address */
struct       sockaddr_in ClientAddr;                     /* Client address */
struct       sockaddr_in fromAddr;                       /* Client address */
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow the network driver to initialize NetX Duo. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifndef USE_IPV6

    /* Set up IPv4 client and peer addressing.  */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin_family = PF_INET;
    ClientAddr.sin_addr.s_addr = htonl(CLIENT1_IP_ADDRESS);

    /* Assign a known Server port */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_addr.s_addr = htonl(CLIENT0_IP_ADDRESS);
    destAddr.sin_family = PF_INET;

    socket_family =  AF_INET;

#else

    /* Set up IPv6 client and peer addressing.  */

    /* Enable IPv6 for this IP instance. */
    nxd_ipv6_enable(&bsd_ip);

    /* Enable ICMPv6 next. */
    nxd_icmp_enable(&bsd_ip);

    socket_family =  AF_INET6;

    /* Set up Client 1 addresses.  */
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    ClientAddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:0db8:0:f101::101", &ClientAddr.sin6_addr._S6_un._S6_u32);

    /* Set up destination address */
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:0db8:0:f101::101", &destAddr.sin6_addr._S6_un._S6_u32);

    /* Allow time for NetX Duo to validate host IPv6 addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif  /* USE_IPV6 */


    while(1)
    {
    
         /* Create Client 1's raw socket */
         sock_raw = socket(socket_family, SOCK_RAW, 0x99);

         /* Check for any errors */
         if (sock_raw == ERROR)
         {
             printf("Error: BSD Client 1 socket create\n");
             return;
         }

         /* Client 1 is ready to receive packets. */
         bytes_received = 0;

         /* Try to receive an message from the remote peer. */
         addrlen = sizeof(fromAddr);
         bytes_received = recvfrom(sock_raw, (VOID *)recv_buffer,64, 0, (struct sockaddr *) &fromAddr, &addrlen);
         if (bytes_received > 0)
         {
#ifndef USE_IPV6
             printf("Client 1 received (%d bytes); %s \n", bytes_received, (CHAR *)&recv_buffer[0] + 20);
             printf("Remote IP address 0x%x\n", (UINT)fromAddr.sin_addr.s_addr);
#else
             printf("Client 1 received (%d bytes); %s \n", bytes_received, (CHAR *)&recv_buffer[0]);
             printf("Remote IP address = 0x%x 0x%x 0x%x 0x%x\n", 
                                 (UINT)fromAddr.sin6_addr._S6_un._S6_u32[0],
                                 (UINT)fromAddr.sin6_addr._S6_un._S6_u32[1],             
                                 (UINT)fromAddr.sin6_addr._S6_un._S6_u32[2],             
                                 (UINT)fromAddr.sin6_addr._S6_un._S6_u32[3]);
#endif /* USE_IPV6 */
         }

         /* Clear the message buffer. */
         memset(&recv_buffer[0], 0, 64);

         printf("Close Client1 socket.\n");
                                           
         /* Close the raw socket */
         soc_close(sock_raw);
     } 


}



