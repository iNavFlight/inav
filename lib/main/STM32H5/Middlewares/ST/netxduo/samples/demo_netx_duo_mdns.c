/* This is a small demo of NetX Duo mDNS/DNS-SD on the high-performance NetX Duo TCP/IP stack.  */


#include "tx_api.h"
#include "nx_api.h"
#include "nxd_mdns.h"


#define     DEMO_STACK_SIZE         4096
#define     NX_PACKET_SIZE          1536
#define     NX_PACKET_POOL_SIZE     ((1536 + sizeof(NX_PACKET)) * 64)


/* Define the ThreadX, NetX control blocks...  */

static TX_THREAD                    thread_0;
static NX_PACKET_POOL               pool_0;
static NX_IP                        ip_0;
static NX_MDNS                      mdns;
static UCHAR                        pool_memory[NX_PACKET_POOL_SIZE];


/* Define the counters used in the demo application...  */

static UINT                         status;
static UCHAR                       *pointer;
static ULONG                        error_counter = 0;


/* Define the services for mDNS.  */

#define MDNS_PRIORITY                   3
#define MDNS_LOCAL_SERVICE_CACHE_SIZE   4096
#define MDNS_PEER_SERVICE_CACHE_SIZE    4096

static CHAR*        mdns_host_name = "mDNS-HOST";

#define PRIMARY_INTERFACE 0
#define SERVICE1_INSTANCE_NAME  "NETXDUO_MDNS_Test1"
#define SERVICE_TYPE_HTTP "_http._tcp"
#define SERVICE_INSTANCE_NULL  NX_NULL
#define SERVICE_TYPE_NULL      NX_NULL
#define SERVICE_SUBTYPE_NULL   NX_NULL
#define SERVICE_TXT_NULL       NX_NULL
#define SERVICE_SUBTYPE_PRINTER "_printer"
#ifndef NX_MDNS_DISABLE_SERVER
static      UINT     service1_ttl = 120;
static      UINT     service1_priority = 0;
static      UINT     service1_weights = 0;
static      UINT     service1_port = 80;

#define SERVICE2_INSTANCE_NAME  "NETXDUO_MDNS_Test2"

static      UINT     service2_ttl = 120;
static      UINT     service2_priority = 0;
static      UINT     service2_weights = 0;
static      UINT     service2_port = 1026;

#define SERVICE3_INSTANCE_NAME "NETXDUO_MDNS_Test3"
#define SERVICE_TYPE_IPP  "_ipp._tcp"
#define SERVICE3_TXT_INFO  "paper=A4;version=01"
static      UINT     service3_ttl = 120;
static      UINT     service3_priority = 0;
static      UINT     service3_weights = 0;
static      UINT     service3_port = 1028;

#define SERVICE4_INSTANCE_NAME "NETXDUO_MDNS_Test4"
#define SERVICE4_TXT_INFO "paper=A3;version=02"
static      UINT     service4_ttl = 120;
static      UINT     service4_priority = 0;
static      UINT     service4_weights = 0;
static      UINT     service4_port = 1030;
#endif /* NX_MDNS_DISABLE_SERVER */
#define SERVICE_INSTANCE_NAME_NULL NX_NULL

#ifndef NX_MDNS_DISABLE_CLIENT
static      NX_MDNS_SERVICE  service_instance;
static      ULONG            query_timeout = 500;
#endif /* NX_MDNS_DISABLE_CLIENT */

#define WIN_MDNS_INSTANCE_1 "WIN_MDNS_Test1"
#define WIN_MDNS_INSTANCE_2 "WIN_MDNS_Test2"

#define SERVICE_TYPE_SMB "_smb._tcp"

static ULONG mdns_thread_stack[DEMO_STACK_SIZE / sizeof(ULONG)];
static ULONG local_service_cache[MDNS_LOCAL_SERVICE_CACHE_SIZE / sizeof(ULONG)];
static ULONG peer_service_cache[MDNS_PEER_SERVICE_CACHE_SIZE / sizeof(ULONG)];

/* Define thread prototypes.  */

static VOID thread_0_entry(ULONG thread_input);

/******** Optionally substitute your Ethernet driver here. ***********/

extern VOID _nx_ram_network_driver(NX_IP_DRIVER *);


/* Define main entry point.  */ 

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Setup the working pointer.  */
    pointer = (UCHAR *) first_unused_memory;

    /* Create a packet pool for the mDNS.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Client Packet Pool", NX_PACKET_SIZE, pool_memory, NX_PACKET_POOL_SIZE);
                
    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;     
        return;
    }
    
    /* Create an IP instance for the mDNS.  */
    status = nx_ip_create(&ip_0, "NetX Client IP Instance", IP_ADDRESS(192, 168, 100, 34), 0xFFFFFF00UL, 
                          &pool_0, _nx_ram_network_driver, pointer, 2048, 1);
    pointer =  pointer + 2048;
          
    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for the FTP Client IP.  */
    nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Enable ICMP for client IP instance.  */
    status = nx_icmp_enable(&ip_0);
    
    /* Enable ICMP for client IP instance.  */
    status += nx_igmp_enable(&ip_0);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }
#endif /* NX_DISABLE_IPV4  */
    
    /* Enable fragment */
    status = nx_ip_fragment_enable(&ip_0); 

    /* Check for fragment enable errors.  */
    if(status)
        error_counter++;

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START); 
    pointer =  pointer + DEMO_STACK_SIZE;
}


#ifndef NX_MDNS_DISABLE_SERVER
static void register_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype, UCHAR *txt, UINT ttl,
                                   UINT priority, UINT weight, UINT port, UINT is_unique)
{


    status = nx_mdns_service_add(&mdns, instance, type, subtype, txt, ttl, (USHORT)priority, (USHORT)weight, (USHORT)port, 
                                 (UCHAR)is_unique, PRIMARY_INTERFACE);

    printf("Local Service Added: %s %s ", instance, type);
    if(subtype)
        printf("%s ", subtype);
    if(status)
        printf("failed\n");
    else
        printf("successfully\n");
}

static void delete_local_service(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{

    printf("Delete local service: ");
    if(instance)
        printf("instance: %s ", instance);
    if(type)
        printf("type %s ", type);
    if(subtype)
        printf("subtype %s ", subtype);

    status = nx_mdns_service_delete(&mdns, instance, type, subtype);
    if(status)
        printf("failed\n");
    else
        printf("successfully\n");

}
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
static void clear_service_cache(void)
{

    status = nx_mdns_peer_cache_clear(&mdns);
    
    printf("Clear service cache ");

    if(status)
        printf("Failed\n\n\n");
    else
        printf("Successful\n\n\n");

}
static void perform_oneshot_query(UCHAR *instance, UCHAR *type, UCHAR *subtype, UINT timeout)
{

    /* Print the query instance.  */
    printf("Send One Shot query: ");
    if(instance)
        printf("instance: %s ", instance);
    if(type)
        printf("type %s ", type);
    if(subtype)
        printf("subtype %s ", subtype);
    printf("timeout %d ticks\n", timeout);
         
    status = nx_mdns_service_one_shot_query(&mdns, instance, type, subtype, &service_instance, timeout);
        
    if (status == NX_MDNS_SUCCESS)
    {              
         printf("Service Get: \n");
         printf("Name: %s\n",service_instance.service_name);
         printf("Type: %s\n",service_instance.service_type);
         printf("Domain: %s\n",service_instance.service_domain);
         printf("TXT Info: %s\n",service_instance.service_text);
         printf("Priority: %d\n",service_instance.service_priority);
         printf("Weight: %d\n",service_instance.service_weight);
         printf("Port: %d\n",service_instance.service_port);
         printf("Target Host: %s\n",service_instance.service_host);
         printf("IPv4 Address: %lu.%lu.%lu.%lu\n",
                 service_instance.service_ipv4 >> 24,
                 service_instance.service_ipv4 >> 16 & 0xFF,                   
                 service_instance.service_ipv4 >> 8 & 0xFF,
                 service_instance.service_ipv4 & 0xFF);
         printf("Interface: %d\n\n\n",service_instance.interface_index);
    }
    else
    {
        printf("No Service!!!\n\n\n");
    }
}

static void start_continous_query(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{

    status = nx_mdns_service_continuous_query(&mdns, instance, type, subtype);

    printf("Start continous query: ");
    if(instance)
        printf("instance %s ", instance);
    if(type)
        printf("type %s ", type);
    if(subtype)
        printf("subtype %s ", subtype);

    /* CHECK FOR ERROR.  */
    if (status) 
    {
        printf("failed\n");
        error_counter++;
    } 
    else
        printf("successfully\n");
}

static void stop_continous_query(UCHAR *instance, UCHAR *type, UCHAR *subtype)
{

    status = nx_mdns_service_query_stop(&mdns, instance, type, subtype);

    printf("Query stop: ");
    if(instance)
        printf("instance %s ", instance);
    if(type)
        printf("type %s ", type);
    if(subtype)
        printf("subtype %s ", subtype);

    if(status)
        printf("failed\n");
    else
        printf("successful\n");

}
UINT service_map = 0;
static void perform_service_lookup(UCHAR *instance, UCHAR *type, UCHAR *subtype, UINT show_all)
{

UINT service_index = 0;
UINT num_info_incomplete = 0;
ULONG   ipv4_address;
ULONG   ipv6_address[4];


    printf("Perform Service Lookup for type:%s \n", type);
                        
    /* Get the all services.  */
    while (1)
    {

        status = nx_mdns_service_lookup(&mdns, instance, type, subtype,  service_index, &service_instance);

        if (status == NX_MDNS_SUCCESS)
        {      

            if(service_instance.service_name &&
               service_instance.service_type &&
               service_instance.service_domain &&
               service_instance.service_port &&
               service_instance.service_host[0] &&
               service_instance.service_ipv4)
            {
                if(show_all || ((service_map & (UINT)(1 << service_index)) == 0))
                {
                    /* The service index may not be the same with query index. */
                    printf("Service Get: %d \n", service_index);
                    printf("  Name: %s\n",service_instance.service_name);
                    printf("  Type: %s\n",service_instance.service_type);
                    printf("  Domain: %s\n",service_instance.service_domain);
                    if(service_instance.service_text != NX_NULL)
                        printf("  TXT Info: %s\n",service_instance.service_text);
                    else
                        printf("  TXT Info: None!\n");
                    
                    printf("  Priority: %d\n",service_instance.service_priority);
                    printf("  Weight: %d\n",service_instance.service_weight);
                    printf("  Port: %d\n",service_instance.service_port);
                    printf("  Target Host: %s\n",service_instance.service_host);
                    printf("  IPv4 Address: %lu.%lu.%lu.%lu\n",
                           service_instance.service_ipv4 >> 24,
                           service_instance.service_ipv4 >> 16 & 0xFF,                   
                           service_instance.service_ipv4 >> 8 & 0xFF,
                           service_instance.service_ipv4 & 0xFF);
                    printf("  Interface: %d\n\n\n",service_instance.interface_index);
                    service_map = service_map | (UINT)(1 << service_index);
                }
            }
            else
            {

                /* Check if have all service info except address.  */
                if(service_instance.service_name &&
                   service_instance.service_type &&
                   service_instance.service_domain &&
                   service_instance.service_port &&
                   service_instance.service_host[0])
                {

                    /* Start to get the host address.  */
                    status = nx_mdns_host_address_get(&mdns, service_instance.service_host, &ipv4_address, ipv6_address, query_timeout);
                    
                    if(show_all || ((service_map & (UINT)(1 << service_index)) == 0))
                    {
                        /* The service index may not be the same with query index. */
                        printf("Service Get: %d \n", service_index);
                        printf("  Name: %s\n",service_instance.service_name);
                        printf("  Type: %s\n",service_instance.service_type);
                        printf("  Domain: %s\n",service_instance.service_domain);
                        if(service_instance.service_text != NX_NULL)
                            printf("  TXT Info: %s\n",service_instance.service_text);
                        else
                            printf("  TXT Info: None!\n");
                    
                        printf("  Priority: %d\n",service_instance.service_priority);
                        printf("  Weight: %d\n",service_instance.service_weight);
                        printf("  Port: %d\n",service_instance.service_port);
                        printf("  Target Host: %s\n",service_instance.service_host);
                        printf("  IPv4 Address: %lu.%lu.%lu.%lu\n",
                               service_instance.service_ipv4 >> 24,
                               service_instance.service_ipv4 >> 16 & 0xFF,                   
                               service_instance.service_ipv4 >> 8 & 0xFF,
                               service_instance.service_ipv4 & 0xFF);
                        printf("  Interface: %d\n\n\n",service_instance.interface_index);
                        service_map = service_map | (UINT)(1 << service_index);
                    }
                }
                else
                {
                    nx_mdns_service_continuous_query(&mdns, service_instance.service_name, service_instance.service_type, NX_NULL);
                    printf("Start continuous query:%d \n", service_index);
                    if (service_instance.service_name)
                    {
                        printf("Service Name: %s\n",  service_instance.service_name);
                    }
                    printf("Service Type: %s\n",  service_instance.service_type);
                    num_info_incomplete ++;
                }
            }
            service_index ++;
        }
        else
            return;

    }
}

static VOID  service_change_notify(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state)
{

    NX_PARAMETER_NOT_USED(mdns_ptr);

    switch(state)
    {
        case NX_MDNS_PEER_SERVICE_RECEIVED:
        {              
              printf("Received the new Service!!! \n"); 
              printf("Name: %s\n",service_ptr -> service_name);
              printf("Type: %s\n",service_ptr -> service_type);
              printf("Domain: %s\n",service_ptr -> service_domain);
              printf("TXT Info: %s\n",service_ptr -> service_text);
              printf("Priority: %d\n",service_ptr -> service_priority);
              printf("Weight: %d\n",service_ptr -> service_weight);
              printf("Port: %d\n",service_ptr -> service_port);
              printf("Target Host: %s\n",service_ptr -> service_host);
              printf("IPv4 Address: %lu.%lu.%lu.%lu\n",
                  service_ptr -> service_ipv4 >> 24,
                  service_ptr -> service_ipv4 >> 16 & 0xFF,                   
                  service_ptr -> service_ipv4 >> 8 & 0xFF,
                  service_ptr -> service_ipv4 & 0xFF);
              printf("Interface: %d\n\n\n",service_ptr -> interface_index);
              break;
        }
        case NX_MDNS_PEER_SERVICE_UPDATED:
        {
            
              printf("Update the Service address!!! \n"); 
              printf("Name: %s\n",service_ptr -> service_name);
              printf("Type: %s\n",service_ptr -> service_type);
              printf("Domain: %s\n",service_ptr -> service_domain);
              printf("TXT Info: %s\n",service_ptr -> service_text);
              printf("Priority: %d\n",service_ptr -> service_priority);
              printf("Weight: %d\n",service_ptr -> service_weight);
              printf("Port: %d\n",service_ptr -> service_port);
              printf("Target Host: %s\n",service_ptr -> service_host);
              printf("IPv4 Address: %lu.%lu.%lu.%lu\n",
                  service_ptr -> service_ipv4 >> 24,
                  service_ptr -> service_ipv4 >> 16 & 0xFF,                   
                  service_ptr -> service_ipv4 >> 8 & 0xFF,
                  service_ptr -> service_ipv4 & 0xFF);
              printf("Interface: %d\n\n\n",service_ptr -> interface_index);
              break;
        }     
        case NX_MDNS_PEER_SERVICE_DELETED:
        {              
              
              printf("Delete the old Service!!! \n"); 
              printf("Name: %s\n",service_ptr -> service_name);
              printf("Type: %s\n",service_ptr -> service_type);
              printf("Domain: %s\n",service_ptr -> service_domain);
              printf("TXT Info: %s\n",service_ptr -> service_text);
              printf("Priority: %d\n",service_ptr -> service_priority);
              printf("Weight: %d\n",service_ptr -> service_weight);
              printf("Port: %d\n",service_ptr -> service_port);
              printf("Target Host: %s\n",service_ptr -> service_host);
              printf("IPv4 Address: %lu.%lu.%lu.%lu\n",
                  service_ptr -> service_ipv4 >> 24,
                  service_ptr -> service_ipv4 >> 16 & 0xFF,                   
                  service_ptr -> service_ipv4 >> 8 & 0xFF,
                  service_ptr -> service_ipv4 & 0xFF);
              printf("Interface: %d\n\n\n",service_ptr -> interface_index);
              break;
        } 
    }
}
#endif /* NX_MDNS_DISABLE_CLIENT  */

static VOID  probing_notify(struct NX_MDNS_STRUCT *mdns_ptr, UCHAR *name, UINT state)
{

    NX_PARAMETER_NOT_USED(mdns_ptr);

    switch(state)
    {
        case NX_MDNS_LOCAL_SERVICE_REGISTERED_SUCCESS:
          {              
              
              printf("Service Name: %s\n",name);
              printf("Registered success!!! \n\n"); 
              break;
          }
        case NX_MDNS_LOCAL_SERVICE_REGISTERED_FAILURE:
          {
            
              printf("Service Name: %s\n",name);
              printf("Registered failure! \n\n"); 
              break;
          }     
        case NX_MDNS_LOCAL_HOST_REGISTERED_SUCCESS:
          {              
              
              printf("Host Name: %s\n",name);
              printf("Registered success!!! \n\n"); 
              break;
          }
        case NX_MDNS_LOCAL_HOST_REGISTERED_FAILURE:
          {
            
              printf("Host Name: %s\n",name);
              printf("Registered failure! \n\n"); 
              break;
          }   
    }
}

static VOID  cache_full_notify(NX_MDNS *mdns_ptr, UINT state, UINT cache_type)
{

    NX_PARAMETER_NOT_USED(mdns_ptr);

    /* Check cache type.  */
    if (cache_type == NX_MDNS_CACHE_TYPE_LOCAL)
    {
        printf("Local Cache: ");
    }
    else
    {
        printf("Peer Cache: ");
    }

    /* Check cache state.  */
    if (state == NX_MDNS_CACHE_STATE_FULL)
    {
        printf("Cache Full!!!\n");
    }
    else
    {
        printf("Cache Full With Fragment!!!\n");
    }
}

/* Define a test thread.  */
static void    thread_0_entry(ULONG thread_input)
{

UINT     test_case = 0;
UINT     time_count = 0;
UINT     i = 200;
#ifndef NX_MDNS_DISABLE_CLIENT
UINT     j = 0;
#endif /* NX_MDNS_DISABLE_CLIENT  */
#ifndef NX_MDNS_DISABLE_CLIENT
ULONG    service_mask = 0x00000000;     /* Set the service mask to select service types it wishes to monitor. 
                                           A list of services being monitored ar hard-coded 
                                           in the table nx_mdns_service_types in nxd_mdns.c.  */
#endif /* NX_MDNS_DISABLE_CLIENT  */


    NX_PARAMETER_NOT_USED(thread_input);

    /* Create a mDNS instance for mDNS.  */
    status =  nx_mdns_create(&mdns, &ip_0, &pool_0, MDNS_PRIORITY, mdns_thread_stack,
                             sizeof(mdns_thread_stack), (UCHAR *)mdns_host_name, 
                             (VOID *)local_service_cache, sizeof(local_service_cache), 
                             (VOID *)peer_service_cache, sizeof(peer_service_cache), probing_notify);

    /* Check for error.  */
    if (status) 
    {

        error_counter++;
        return;
    }

    /* Set the cache notify callback function.  */
    status = nx_mdns_cache_notify_set(&mdns, cache_full_notify);
    if (status) 
    {
        error_counter++;
        return;
    }
    
#ifndef NX_MDNS_DISABLE_CLIENT
    /* Set the service change callback function to listen the service.  */
    status = nx_mdns_service_notify_set(&mdns, service_mask, service_change_notify);  
    if (status) 
    {
        error_counter++;
        return;
    }
#endif /* NX_MDNS_DISABLE_CLIENT  */
    
    /* Enable the mDNS function.  */
    nx_mdns_enable(&mdns, PRIMARY_INTERFACE);

    /* Waiting for host name register. */
    tx_thread_sleep(500);

    while(1)
    {

        tx_thread_sleep(100);

        test_case = 0;
        switch(test_case)
        {
            
        case 0:

            time_count++;            
            
#ifndef NX_MDNS_DISABLE_SERVER
            if ((time_count % i) == 1)
            {
              
                /* Register local service.  */
                register_local_service((UCHAR *)SERVICE1_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL,
                                       SERVICE_TXT_NULL, service1_ttl, service1_priority, service1_weights,
                                       service1_port, NX_TRUE);
                       

                register_local_service((UCHAR *)SERVICE2_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER,
                                       SERVICE_TXT_NULL, service2_ttl, service2_priority, service2_weights,
                                       service2_port, NX_TRUE);

                register_local_service((UCHAR *)SERVICE3_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL,
                                       (UCHAR *)SERVICE3_TXT_INFO, service3_ttl, service3_priority, service3_weights, 
                                       service3_port, NX_TRUE);

                register_local_service((UCHAR *)SERVICE4_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL, 
                                       (UCHAR *)SERVICE4_TXT_INFO, service4_ttl, service4_priority, service4_weights, 
                                       service4_port,  NX_TRUE);
            }

            if((time_count % i) == 60)
                delete_local_service((UCHAR *)SERVICE1_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);
            else if((time_count % i) == 70)
                delete_local_service((UCHAR *)SERVICE2_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);
            else if((time_count % i) == 80)
                delete_local_service((UCHAR *)SERVICE3_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL);
            else if((time_count % i) == 90)
                delete_local_service((UCHAR *)SERVICE4_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL);
#else
            i = 100;
#ifndef NX_MDNS_DISABLE_CLIENT
            j = 100;
#endif /* NX_MDNS_DISABLE_CLIENT  */
#endif /*NX_MDNS_DISABLE_SERVER*/

#ifndef NX_MDNS_DISABLE_CLIENT
            if ((time_count % i) == (101 - j))
            {
                clear_service_cache();
                perform_oneshot_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL, query_timeout);
            }            
            else if ((time_count % i) == (110 - j))
            {
                perform_oneshot_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_SMB, SERVICE_SUBTYPE_NULL, query_timeout);
            }                   
            else if ((time_count % i) == (120 - j))
            {
                start_continous_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);
            }                
            else if ((time_count % i) == (130 - j))
            {
                stop_continous_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);
                perform_service_lookup(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, NX_NULL, 1);
            }
            else if ((time_count % i) == (140 - j))
            {
                perform_service_lookup(NX_NULL, (UCHAR *)SERVICE_TYPE_SMB, NX_NULL, 1);
            }
#else
            i = 100;
#endif /* NX_MDNS_DISABLE_CLIENT  */
            break;

#ifndef NX_MDNS_DISABLE_SERVER
        case 1: /* Register local service: NETXDUO_MDNS_Test1._http._tcp */
            
            register_local_service((UCHAR *)SERVICE1_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL,
                                   SERVICE_TXT_NULL, service1_ttl, service1_priority, service1_weights,
                                   service1_port, NX_TRUE);
            break;

        case 2: /* Register local service: NETXDUO_MDNS_Test2._printer._sub._http._tcp */
            register_local_service((UCHAR *)SERVICE2_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER,
                                   SERVICE_TXT_NULL, service2_ttl, service2_priority, service2_weights,
                                   service2_port, NX_TRUE);
            break;

        case 3: /* Register local service: NETXDUO_MDNS_Test3._http._tcp */
            register_local_service((UCHAR *)SERVICE3_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL,
                                   (UCHAR *)SERVICE3_TXT_INFO, service3_ttl, service3_priority, service3_weights, 
                                   service3_port, NX_TRUE);
            break;

        case 4: /* Register local service: NETXDUO_MDNS_Test4._ipp._tcp */
            register_local_service((UCHAR *)SERVICE4_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL, 
                                   (UCHAR *)SERVICE4_TXT_INFO, service4_ttl, service4_priority, service4_weights, 
                                   service4_port,  NX_TRUE);  
            break;
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
        case 5: /* One shot query for HTTP type. */
            perform_oneshot_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL, query_timeout);

            break;

        case 6: /* One shot query for _printer._sub._http._tcp */
            perform_oneshot_query((UCHAR *)SERVICE_INSTANCE_NULL, (UCHAR *)SERVICE_TYPE_HTTP, 
                                  (UCHAR *)SERVICE_SUBTYPE_PRINTER, query_timeout);
            break;

        case 7: /* Clear service cache */
            clear_service_cache();
            break;
            
        case 8: /* Continous query for all services. */
            {
            start_continous_query(NX_NULL, NX_NULL, NX_NULL);
            
            /* Waiting for Service response.  */
            tx_thread_sleep(500);
            
            /* Perform the lookup.  */
            perform_service_lookup(NX_NULL, NX_NULL, NX_NULL, 1);
            break;
            }
        case 81: /* Stop continous query.  */
            stop_continous_query(NX_NULL, NX_NULL, NX_NULL);
            break;
        case 9: /* Continous query for _http._tcp.local. */
            {
            start_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);

            /* Waiting for Service response.  */
            tx_thread_sleep(500);

            /* Perform the lookup.  */
            perform_service_lookup(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL, 1);

            break;
            }
        case 91:

            /* Stop continous query.  */
            stop_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);
            break;
        case 10: /* Continous query for _smb._tcp */
            {
            start_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_SMB, SERVICE_SUBTYPE_NULL);

            /* Waiting for Service response.  */
            tx_thread_sleep(500);
            
            /* Perform the lookup.  */
            perform_service_lookup(NX_NULL, (UCHAR *)SERVICE_TYPE_SMB, SERVICE_SUBTYPE_NULL, 1);

            break;
            }
        case 101: /* Stop continous query.  */
            stop_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_SMB, SERVICE_SUBTYPE_NULL);
            break;
            
        case 11: /* Continous_query for _printer._sub._http._tcp */
            {
            start_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);

            /* Waiting for Service response.  */
            tx_thread_sleep(500);

            /* Perform the lookup.  */
            perform_service_lookup(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER, 1);

            break;
            }       
        case 111: /* Stop continous query.  */
            stop_continous_query(NX_NULL, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);
            break;
        case 12: /* Continous_query for WIN_MDNS_Test_2._printer._sub._http._tcp */
            {
            start_continous_query((UCHAR *)WIN_MDNS_INSTANCE_2, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);

            /* Waiting for Service response.  */
            tx_thread_sleep(500);
            
            /* Perform the lookup.  */
            perform_service_lookup((UCHAR *)WIN_MDNS_INSTANCE_2, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER, 1);

            break;
            }
        case 121: /* Stop continous query.  */
            stop_continous_query((UCHAR *)WIN_MDNS_INSTANCE_2, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);
            break;
#endif /* NX_MDNS_DISABLE_CLIENT  */

            /* When the service name is changed due to conflict, we need delete the new service. */

#ifndef NX_MDNS_DISABLE_SERVER
        case 13: /* Delete local service: NETXDUO_MDNS_Test1._http._tcp */
            delete_local_service((UCHAR *)SERVICE1_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, SERVICE_SUBTYPE_NULL);
            break;

        case 14: /* Delete local service NETXDUO_MDNS_Test2._printer._sub._http._tcp */
            delete_local_service((UCHAR *)SERVICE2_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_HTTP, (UCHAR *)SERVICE_SUBTYPE_PRINTER);
            break;

        case 15: /* Delete local service: NETXDUO_MDNS_Test3._http._tcp */
            delete_local_service((UCHAR *)SERVICE3_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL);
            break;

        case 16: /* Delete local service: NETXDUO_MDNS_Test4._ipp._tcp */
            delete_local_service((UCHAR *)SERVICE4_INSTANCE_NAME, (UCHAR *)SERVICE_TYPE_IPP, SERVICE_SUBTYPE_NULL);
            break;
#endif /* NX_MDNS_DISABLE_SERVER  */

        default:
            tx_thread_sleep(100);
            break;
        }            
    }
}

