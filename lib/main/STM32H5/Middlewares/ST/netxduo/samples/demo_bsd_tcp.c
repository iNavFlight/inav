/* This is a small demo of BSD Wrapper for the high-performance NetX TCP/IP stack.
   This demo used standard BSD services for TCP connection, disconnection, sending, and 
   receiving using a simulated Ethernet driver.  */


#include         "tx_api.h"
#include         "nx_api.h"
#include         "nxd_bsd.h"
#include         <string.h>
#include         <stdlib.h>

#define          DEMO_STACK_SIZE     (16*1024)
#define          SERVER_PORT          87
#define          CLIENT_PORT          77
#define          SERVER_RCV_BUFFER_SIZE 100


/* Define the ThreadX and NetX object control blocks... */

TX_THREAD        thread_server;
TX_THREAD        thread_client;
NX_PACKET_POOL   bsd_pool;
NX_IP            bsd_ip;

/* Define some global data. */
INT      maxfd;

/* Define the counters used in the demo application...  */

ULONG            error_counter;

/* Define fd_sets for the BSD server socket.  */
fd_set           master_list, read_ready;

#ifdef NX_DISABLE_IPV4
/* To send IPv6 packets, define DUO.  */
#define  DUO
#endif


/* Define thread prototypes.  */

VOID        thread_server_entry(ULONG thread_input);
VOID        thread_client_entry(ULONG thread_input);
void        _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);


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

    /* Create a server thread.  */
    tx_thread_create(&thread_server, "Server", thread_server_entry, 0,  
                          pointer, DEMO_STACK_SIZE, 8, 8, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create a Client thread.  */
    tx_thread_create(&thread_client, "Client", thread_client_entry, 0,  
                          pointer, DEMO_STACK_SIZE, 16, 16, TX_NO_TIME_SLICE, TX_AUTO_START);

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
    status = nx_ip_create(&bsd_ip, "BSD IP Instance", IP_ADDRESS(1,2,3,4), 0xFFFFFF00UL,  &bsd_pool, _nx_ram_network_driver,
                    pointer, 2048, 1);
    pointer =  pointer + 2048;

    if (status)
    {
    error_counter++;
        printf("Error creating BSD IP instance\n!");
    }
    
#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for BSD IP Instance */
    status =  nx_arp_enable(&bsd_ip, (void *) pointer, 1024);
    pointer = pointer + 1024; 

    /* Check ARP enable status.  */     
    if (status)
    {
        error_counter++;
        printf("Error in Enable ARP and supply ARP cache memory to BSD IP instance\n");
    }   
#endif /* NX_DISABLE_IPV4 */

    /* Enable TCP processing for BSD IP instances.  */

    status = nx_tcp_enable(&bsd_ip);

    /* Check TCP enable status.  */
    if (status)
    {
        error_counter++;
        printf("Error in Enable TCP \n");
    }   

    /* Now initialize BSD Scoket Wrapper */
    status = (UINT)bsd_initialize (&bsd_ip, &bsd_pool,pointer, 2048, 2);
}


/* Define the Server thread.  */
CHAR        Server_Rcv_Buffer[SERVER_RCV_BUFFER_SIZE];

VOID  thread_server_entry(ULONG thread_input)
{


INT         status,  sock, sock_tcp_server;
ULONG       actual_status;
INT         Clientlen;
INT         i;
INT         is_set = 0;
#ifdef DUO  
NXD_ADDRESS ip_address;
struct      sockaddr_in6 serverAddr;                  
struct      sockaddr_in6 ClientAddr;
UINT        iface_index, address_index;
#else
struct      sockaddr_in serverAddr;                  
struct      sockaddr_in ClientAddr;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    status =  (INT)nx_ip_status_check(&bsd_ip, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE); 
    
    /* Check status...  */ 
    if (status != NX_SUCCESS) 
    { 
        return; 
    } 

#ifdef DUO
    /* Enable IPv6 */
    status = (INT)nxd_ipv6_enable(&bsd_ip);
    if(status != NX_SUCCESS)
    {
        printf("Error with IPv6 enable 0x%x\n", status);
        return;
    }

    /* Enable ICMPv6 */
    status = (INT)nxd_icmp_enable(&bsd_ip);
    if(status)
    {
        printf("Error with ECMPv6 enable 0x%x\n", status);
        return;
    }

    /* Set the primary interface for our DNS IPv6 addresses. */
    iface_index = 0;

    /* This assumes we are using the primary network interface (index 0). */
    status = (INT)nxd_ipv6_address_set(&bsd_ip, iface_index, NX_NULL, 10, &address_index);

    if (status)
        return;


    /* Set ip_0 interface address. */
    ip_address.nxd_ip_version = NX_IP_VERSION_V6;
    ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    ip_address.nxd_ip_address.v6[2] = 0;
    ip_address.nxd_ip_address.v6[3] = 0x101;
    
    /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = (INT)nxd_ipv6_address_set(&bsd_ip, iface_index, &ip_address, 64, &address_index);

    if (status)
        return;

    /* Wait for IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif

    /* Create BSD TCP Socket */
#ifdef DUO
    sock_tcp_server = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
    sock_tcp_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

    if (sock_tcp_server == -1)
    {
        printf("\nError: BSD TCP Server socket create \n");
        return;
    }
    
    printf("\nBSD TCP Server socket created %lu \n", (ULONG)sock_tcp_server);

    /* Set the server port and IP address */
#ifdef DUO
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    serverAddr.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    serverAddr.sin6_addr._S6_un._S6_u32[2] = 0x0;
    serverAddr.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    serverAddr.sin6_port = htons(SERVER_PORT);
    serverAddr.sin6_family = AF_INET6;

#else
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
    serverAddr.sin_port = htons(SERVER_PORT);
#endif  

    /* Bind this server socket */
    status = bind (sock_tcp_server, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    if (status < 0)
    {
        printf("Error: Server Socket Bind \n");
        return;
    }   

    FD_ZERO(&master_list);
    FD_ZERO(&read_ready);
    FD_SET(sock_tcp_server,&master_list);
    maxfd = sock_tcp_server;

    /* Now listen for any client connections for this server socket */
    status = listen (sock_tcp_server, 5);
    if (status < 0)
    {
        printf("Error: Server Socket Listen\n");
        return;
    }
    else
        printf("Server Listen complete\n");

    /* All set to accept client connections */
    printf("Now accepting client connections\n");  

    /* Loop to create and establish server connections.  */
    while(1)
    {

        printf("\n");

        read_ready = master_list;

        tx_thread_sleep(20);   /* Allow some time to other threads too */

        /* Let the underlying TCP stack determine the timeout. */
        status = select(maxfd + 1, &read_ready, 0, 0, 0);

        if ( (status == ERROR) || (status == 0) )
        {

            printf("Error with select? Status 0x%x. Try again\n", status);

            continue;
        }

        /* Detected a connection request. */
        is_set = FD_ISSET(sock_tcp_server,&read_ready);

        if(is_set)
        {

            Clientlen = sizeof(ClientAddr);

            sock = accept(sock_tcp_server,(struct sockaddr*)&ClientAddr, &Clientlen);

            /* Add this new connection to our master list */
            FD_SET(sock, &master_list);   

            if ( sock > maxfd)
            {
                printf("New connection %d\n", sock);

                maxfd = sock;
            }   

            continue; 
        }

        /* Check the set of 'ready' sockets, e.g connected to remote host and waiting for
           notice of packets received. */
        for (i = 0; i < (maxfd+1); i++)
        {

            if (((i+ NX_BSD_SOCKFD_START) != sock_tcp_server) && 
                 (FD_ISSET(i + NX_BSD_SOCKFD_START, &master_list)) && 
                 (FD_ISSET(i + NX_BSD_SOCKFD_START, &read_ready)))
            {

                while(1)
                {

                    status = recv(i + NX_BSD_SOCKFD_START, (VOID *)Server_Rcv_Buffer, SERVER_RCV_BUFFER_SIZE, 0);
            
                    if (status == ERROR)
                    {
                        /* This is a blocking socket. If no data is received, but the connection is still good,
                           the EAGAIN error is set. If it was a non blocking socket, the EWOULDBLOCK socket 
                           error is set. */
                        if (errno == EAGAIN) 
                        {
                            printf("No error received. Try again later\n");
                            continue;
                        }
                        else if (errno == ENOTCONN) 
                        {
                            /* If the socket connection is terminated, the socket error will be ENOTCONN. */
                            printf("Connection is broken.  Close the socket.\n");
                            break;
                        }
                        else
                        {
                            /* Another error has occurred...probably an internal error of some kind
                               so best to terminate the connection. */
                            printf("Error on Client Socket %d receiving data: 0x%x \n", i, errno);
                            break;
                        }                    
                    }
                    /* recv returns with message.  Make sure Server_Rcv_Buffer is NULL-terminated. */
                    if(status == SERVER_RCV_BUFFER_SIZE)
                        status--;
                    Server_Rcv_Buffer[status] = 0;

                    printf("Server socket received from Client on socket %d %lu bytes: %s\n ",
                           i+ NX_BSD_SOCKFD_START, (ULONG)status, Server_Rcv_Buffer);
                    
           
                    status = send(i + NX_BSD_SOCKFD_START, "Hello\n", sizeof("Hello\n"), 0);
            
                    if (status == ERROR)
                    {
                        printf("Error on Server sending to Client on socket %d\n", i+ NX_BSD_SOCKFD_START);
                    }
                    else
                    {
                        printf("Server socket message sent to Client on socket %d: Hello\n", i+ NX_BSD_SOCKFD_START);
                    }
                }
            
                /* Close this socket */   
                status = soc_close(i+ NX_BSD_SOCKFD_START);

                if (status != ERROR)
                {
                    printf("Socket closing socket connected to Client on %d \n", i+ NX_BSD_SOCKFD_START);
                }
                else
                {
                
                    printf("Error on Server closing socket %d connected to Client \n", i+ NX_BSD_SOCKFD_START);
                }
            }    
        }   

        /* Loop back to check any next client connection */
    } 
}

CHAR        Client_Rcv_Buffer[100];

VOID  thread_client_entry(ULONG thread_input)
{


INT         status;
ULONG       actual_status;
INT         sock_tcp_client, length;
#ifdef DUO
struct      sockaddr_in6 echoServAddr6;               /* Echo server address */
struct      sockaddr_in6 localAddr6;                  /* Local address */
struct      sockaddr_in6 remoteAddr6;                 /* Remote address */
#else
struct      sockaddr_in echoServAddr;               /* Echo server address */
struct      sockaddr_in localAddr;                  /* Local address */
struct      sockaddr_in remoteAddr;                 /* Remote address */
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    status =  (INT)nx_ip_status_check(&bsd_ip, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE); 
    
    /* Check status...  */ 
    if (status != NX_SUCCESS) 
    { 
        return; 
    } 

#ifdef DUO
    /* Wait for IPv6 stack to finish DAD process. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

    /* Fill Local and Server port and IP address */
    memset(&localAddr6, 0, sizeof(localAddr6));
    localAddr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    localAddr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    localAddr6.sin6_addr._S6_un._S6_u32[2] = 0x0;
    localAddr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    localAddr6.sin6_port = htons(CLIENT_PORT);
    localAddr6.sin6_family = AF_INET6;

    memset(&echoServAddr6, 0, sizeof(echoServAddr6));
    echoServAddr6.sin6_addr._S6_un._S6_u32[0] = htonl(0x20010db8u);
    echoServAddr6.sin6_addr._S6_un._S6_u32[1] = htonl(0xf101u);
    echoServAddr6.sin6_addr._S6_un._S6_u32[2] = 0x0;
    echoServAddr6.sin6_addr._S6_un._S6_u32[3] = htonl(0x0101u);
    echoServAddr6.sin6_port = htons(SERVER_PORT);
    echoServAddr6.sin6_family = AF_INET6;

#else
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
    localAddr.sin_port = htons(CLIENT_PORT);

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(IP_ADDRESS(1,2,3,4));
    echoServAddr.sin_port = htons(SERVER_PORT);
#endif  /* DUO */

    /* Now make client connections with the server. */
    while (1)
    {

         printf("\n");
        /* Create BSD TCP Socket */
#ifdef DUO
        sock_tcp_client = socket( AF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
        sock_tcp_client = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif

        if (sock_tcp_client == -1)
        {
            printf("Error: BSD TCP Client socket create \n");
            return;
        }
    
        printf("Client socket created %lu \n", (ULONG)sock_tcp_client);
    
        /* Now connect this client to the server */
#ifdef DUO
        status = connect(sock_tcp_client, (struct sockaddr *)&echoServAddr6, sizeof(echoServAddr6));
#else
        status = connect(sock_tcp_client, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr));
#endif
    
        /* Check for error.  */
        if (status != OK)
        {
            printf("\nError: BSD TCP Client socket Connect\n");
            status = soc_close(sock_tcp_client);
            return;
    
        }
        /* Get and print source and destination information */
        printf("Client socket %d connected \n", sock_tcp_client);
    
#ifdef DUO
        length = sizeof(struct sockaddr_in6);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&localAddr6, &length);
        printf("Client port = %u , Client = 0x%x 0x%x 0x%x 0x%x,", 
                            localAddr6.sin6_port, 
                            (UINT)localAddr6.sin6_addr._S6_un._S6_u32[0],
                            (UINT)localAddr6.sin6_addr._S6_un._S6_u32[1],             
                            (UINT)localAddr6.sin6_addr._S6_un._S6_u32[2],             
                            (UINT)localAddr6.sin6_addr._S6_un._S6_u32[3]);
    
        length = sizeof(struct sockaddr_in6);
        status = getpeername( sock_tcp_client, (struct sockaddr *) &remoteAddr6, &length);
        printf("Remote port = %u, Remote IP = 0x%x 0x%x 0x%x 0x%x \n", 
                            remoteAddr6.sin6_port, 
                            (UINT)remoteAddr6.sin6_addr._S6_un._S6_u32[0],
                            (UINT)remoteAddr6.sin6_addr._S6_un._S6_u32[1],             
                            (UINT)remoteAddr6.sin6_addr._S6_un._S6_u32[2],             
                            (UINT)remoteAddr6.sin6_addr._S6_un._S6_u32[3]);
#else
        length = sizeof(struct sockaddr_in);
        status = getsockname(sock_tcp_client, (struct sockaddr *)&localAddr, &length);
        printf("Client port = %lu , Client = 0x%x,", (ULONG)localAddr.sin_port, (UINT)localAddr.sin_addr.s_addr);
        length = sizeof(struct sockaddr_in);
        status = getpeername( sock_tcp_client, (struct sockaddr *) &remoteAddr, &length);
        printf("Remote port = %lu, Remote IP = 0x%x \n", (ULONG)remoteAddr.sin_port, (UINT)remoteAddr.sin_addr.s_addr);
#endif
    
        /* Now receive the echoed packet from the server */
        while(1)
        {
    
            printf("Client sock %d sending packet to server\n", sock_tcp_client);
    
            status = send(sock_tcp_client, "Hello", (sizeof("Hello")), 0);
    
            if (status == ERROR)
            {
                printf("Error: Client Socket (%d) send \n", sock_tcp_client);
            }
            else
            {
                printf("Client socket %d sent message Hello\n", sock_tcp_client);
            }
    
            status = recv(sock_tcp_client, (VOID *)Client_Rcv_Buffer, 100, 0);
    
            if (status < 0)
            {
    
                printf("Connection terminated or error on receiving data on socket %d \n", sock_tcp_client);

                break;
            }
            else
            {
                printf("Client socket %d received %d bytes and this message: %s\n", sock_tcp_client, status, Client_Rcv_Buffer);
            }

        }
    
        /* close this client socket */   
        status = soc_close(sock_tcp_client);

        if (status != ERROR)
        {
            printf("Client Socket %d closed\n", sock_tcp_client);
        }
        else
        {
            printf("Error: Client Socket %d on close \n", sock_tcp_client);
        }

        /* Make another Client connection...*/

    }
} 


