/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "nx_api.h"
#ifndef SAMPLE_DHCP_DISABLE
#include "nxd_dhcp_client.h"
#endif /* SAMPLE_DHCP_DISABLE */
#include "nxd_dns.h"
#include "nxd_sntp_client.h"
#include "nx_secure_tls_api.h"

/* Defined, HTTP proxy is enabled.  */
/*
#define SAMPLE_HTTP_PROXY_ENABLE
*/

#ifdef SAMPLE_HTTP_PROXY_ENABLE
#ifndef NX_ENABLE_HTTP_PROXY
#error "SYMBOL NX_ENABLE_HTTP_PROXY must be defined. "
#endif /* NX_ENABLE_HTTP_PROXY */
#endif /* SAMPLE_HTTP_PROXY_ENABLE */

#if defined(SAMPLE_HTTP_PROXY_ENABLE)
#include "nx_http_proxy_client.h"
#endif /* SAMPLE_HTTP_PROXY_ENABLE */

/* Include the sample.  */
extern VOID sample_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

/* Define the helper thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef SAMPLE_HELPER_STACK_SIZE
#define SAMPLE_HELPER_STACK_SIZE        (4096)
#endif /* SAMPLE_HELPER_STACK_SIZE  */

#ifndef SAMPLE_HELPER_THREAD_PRIORITY
#define SAMPLE_HELPER_THREAD_PRIORITY   (4)
#endif /* SAMPLE_HELPER_THREAD_PRIORITY  */

/* Define user configurable symbols. */
#ifndef SAMPLE_IP_STACK_SIZE
#define SAMPLE_IP_STACK_SIZE            (2048)
#endif /* SAMPLE_IP_STACK_SIZE  */

#ifndef SAMPLE_PACKET_COUNT
#define SAMPLE_PACKET_COUNT             (32)
#endif /* SAMPLE_PACKET_COUNT  */

#ifndef SAMPLE_PACKET_SIZE
#define SAMPLE_PACKET_SIZE              (1536)
#endif /* SAMPLE_PACKET_SIZE  */

#define SAMPLE_POOL_SIZE                ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_PACKET_COUNT)

#ifndef SAMPLE_ARP_CACHE_SIZE
#define SAMPLE_ARP_CACHE_SIZE           (512)
#endif /* SAMPLE_ARP_CACHE_SIZE  */

#ifndef SAMPLE_IP_THREAD_PRIORITY
#define SAMPLE_IP_THREAD_PRIORITY       (1)
#endif /* SAMPLE_IP_THREAD_PRIORITY */

#if defined(SAMPLE_DHCP_DISABLE) && !defined(SAMPLE_NETWORK_CONFIGURE)
#ifndef SAMPLE_IPV4_ADDRESS
/*#define SAMPLE_IPV4_ADDRESS           IP_ADDRESS(192, 168, 100, 33)*/
#error "SYMBOL SAMPLE_IPV4_ADDRESS must be defined. This symbol specifies the IP address of device. "
#endif /* SAMPLE_IPV4_ADDRESS */

#ifndef SAMPLE_IPV4_MASK
/*#define SAMPLE_IPV4_MASK              0xFFFFFF00UL*/
#error "SYMBOL SAMPLE_IPV4_MASK must be defined. This symbol specifies the IP address mask of device. "
#endif /* SAMPLE_IPV4_MASK */

#ifndef SAMPLE_GATEWAY_ADDRESS
/*#define SAMPLE_GATEWAY_ADDRESS        IP_ADDRESS(192, 168, 100, 1)*/
#error "SYMBOL SAMPLE_GATEWAY_ADDRESS must be defined. This symbol specifies the gateway address for routing. "
#endif /* SAMPLE_GATEWAY_ADDRESS */

#ifndef SAMPLE_DNS_SERVER_ADDRESS
/*#define SAMPLE_DNS_SERVER_ADDRESS     IP_ADDRESS(192, 168, 100, 1)*/
#error "SYMBOL SAMPLE_DNS_SERVER_ADDRESS must be defined. This symbol specifies the dns server address for routing. "
#endif /* SAMPLE_DNS_SERVER_ADDRESS */
#else
#define SAMPLE_IPV4_ADDRESS             IP_ADDRESS(0, 0, 0, 0)
#define SAMPLE_IPV4_MASK                IP_ADDRESS(0, 0, 0, 0)

#ifndef SAMPLE_DHCP_WAIT_OPTION
#define SAMPLE_DHCP_WAIT_OPTION         (20 * NX_IP_PERIODIC_RATE)
#endif /* SAMPLE_DHCP_WAIT_OPTION */

#endif /* SAMPLE_DHCP_DISABLE */

#ifndef SAMPLE_SNTP_SYNC_MAX
#define SAMPLE_SNTP_SYNC_MAX            30
#endif /* SAMPLE_SNTP_SYNC_MAX */

#ifndef SAMPLE_SNTP_UPDATE_MAX
#define SAMPLE_SNTP_UPDATE_MAX          10
#endif /* SAMPLE_SNTP_UPDATE_MAX */

#ifndef SAMPLE_SNTP_UPDATE_INTERVAL
#define SAMPLE_SNTP_UPDATE_INTERVAL     (NX_IP_PERIODIC_RATE / 2)
#endif /* SAMPLE_SNTP_UPDATE_INTERVAL */

/* Default time. GMT: Friday, Jan 1, 2022 12:00:00 AM. Epoch timestamp: 1640995200.  */
#ifndef SAMPLE_SYSTEM_TIME 
#define SAMPLE_SYSTEM_TIME              1640995200
#endif /* SAMPLE_SYSTEM_TIME  */

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define SAMPLE_UNIX_TO_NTP_EPOCH_SECOND 0x83AA7E80

#if defined(SAMPLE_HTTP_PROXY_ENABLE)
#ifndef SAMPLE_HTTP_PROXY_SERVER
#define SAMPLE_HTTP_PROXY_SERVER        IP_ADDRESS(192, 168, 100, 6)
#endif /* SAMPLE_HTTP_PROXY_SERVER  */

#ifndef SAMPLE_HTTP_PROXY_SERVER_PORT
#define SAMPLE_HTTP_PROXY_SERVER_PORT   8888
#endif /* SAMPLE_HTTP_PROXY_SERVER_PORT  */

#ifndef SAMPLE_HTTP_PROXY_USER_NAME
#define SAMPLE_HTTP_PROXY_USER_NAME     ""
#endif /* SAMPLE_HTTP_PROXY_USER_NAME  */

#ifndef SAMPLE_HTTP_PROXY_PASSWORD
#define SAMPLE_HTTP_PROXY_PASSWORD      ""
#endif /* SAMPLE_HTTP_PROXY_PASSWORD  */
#endif /* SAMPLE_HTTP_PROXY_ENABLE  */

static TX_THREAD        sample_helper_thread;
static NX_PACKET_POOL   pool_0;
static NX_IP            ip_0;
static NX_DNS           dns_0;
#ifndef SAMPLE_DHCP_DISABLE
static NX_DHCP          dhcp_0;
#endif /* SAMPLE_DHCP_DISABLE  */
static NX_SNTP_CLIENT   sntp_client;

/* System clock time for UTC.  */
static ULONG            unix_time_base;

/* Define the stack/cache for ThreadX.  */
static ULONG sample_ip_stack[SAMPLE_IP_STACK_SIZE / sizeof(ULONG)];
#ifndef SAMPLE_POOL_STACK_USER
static ULONG sample_pool_stack[SAMPLE_POOL_SIZE / sizeof(ULONG)];
static ULONG sample_pool_stack_size = sizeof(sample_pool_stack);
#else
extern ULONG sample_pool_stack[];
extern ULONG sample_pool_stack_size;
#endif
#ifndef SAMPLE_NETWORK_CONFIGURE
static ULONG sample_arp_cache_area[SAMPLE_ARP_CACHE_SIZE / sizeof(ULONG)];
#endif
static ULONG sample_helper_thread_stack[SAMPLE_HELPER_STACK_SIZE / sizeof(ULONG)];

static const CHAR *sntp_servers[] =
{
    "0.pool.ntp.org",
    "1.pool.ntp.org",
    "2.pool.ntp.org",
    "3.pool.ntp.org",
};
static UINT sntp_server_index;

/* Define the prototypes for sample thread.  */
static void sample_helper_thread_entry(ULONG parameter);

#ifndef SAMPLE_DHCP_DISABLE
static UINT dhcp_wait();
#endif /* SAMPLE_DHCP_DISABLE */

static UINT dns_create(ULONG dns_server_address);

static UINT sntp_time_sync();
static UINT unix_time_get(ULONG *unix_time);

#ifndef SAMPLE_NETWORK_DRIVER
#define SAMPLE_NETWORK_DRIVER           _nx_ram_network_driver
#endif /* SAMPLE_NETWORK_DRIVER */

/* Include the platform IP driver. */
void SAMPLE_NETWORK_DRIVER(struct NX_IP_DRIVER_STRUCT *driver_req);

#ifdef SAMPLE_BOARD_SETUP
void SAMPLE_BOARD_SETUP();
#endif /* SAMPLE_BOARD_SETUP */

#ifdef SAMPLE_NETWORK_CONFIGURE
void SAMPLE_NETWORK_CONFIGURE(NX_IP *ip_ptr, ULONG *dns_address);
#endif

/* Define main entry point.  */
int main(void)
{

#ifdef SAMPLE_BOARD_SETUP
    SAMPLE_BOARD_SETUP();
#endif /* SAMPLE_BOARD_SETUP */

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}

/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UINT  status;

    NX_PARAMETER_NOT_USED(first_unused_memory);

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", SAMPLE_PACKET_SIZE,
                                   (UCHAR *)sample_pool_stack , sample_pool_stack_size);

    /* Check for pool creation error.  */
    if (status)
    {
        printf("nx_packet_pool_create fail: %u\r\n", status);
        return;
    }

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0",
                          SAMPLE_IPV4_ADDRESS, SAMPLE_IPV4_MASK,
                          &pool_0, SAMPLE_NETWORK_DRIVER,
                          (UCHAR*)sample_ip_stack, sizeof(sample_ip_stack),
                          SAMPLE_IP_THREAD_PRIORITY);

    /* Check for IP create errors.  */
    if (status)
    {
        printf("nx_ip_create fail: %u\r\n", status);
        return;
    }

#ifndef SAMPLE_NETWORK_CONFIGURE
    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (VOID *)sample_arp_cache_area, sizeof(sample_arp_cache_area));

    /* Check for ARP enable errors.  */
    if (status)
    {
        printf("nx_arp_enable fail: %u\r\n", status);
        return;
    }

    /* Enable ICMP traffic.  */
    status = nx_icmp_enable(&ip_0);

    /* Check for ICMP enable errors.  */
    if (status)
    {
        printf("nx_icmp_enable fail: %u\r\n", status);
        return;
    }
#endif

    /* Enable TCP traffic.  */
    status = nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
    {
        printf("nx_tcp_enable fail: %u\r\n", status);
        return;
    }

    /* Enable UDP traffic.  */
    status = nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
    {
        printf("nx_udp_enable fail: %u\r\n", status);
        return;
    }

    /* Initialize TLS.  */
    nx_secure_tls_initialize();

    /* Create sample helper thread.  */
    status = tx_thread_create(&sample_helper_thread, "Demo Thread",
                              sample_helper_thread_entry, 0,
                              sample_helper_thread_stack, SAMPLE_HELPER_STACK_SIZE,
                              SAMPLE_HELPER_THREAD_PRIORITY, SAMPLE_HELPER_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check status.  */
    if (status)
    {
        printf("Demo helper thread creation fail: %u\r\n", status);
        return;
    }
}

/* Define sample helper thread entry.  */
void sample_helper_thread_entry(ULONG parameter)
{

UINT    status;
ULONG   ip_address = 0;
ULONG   network_mask = 0;
ULONG   gateway_address = 0;
UINT    unix_time;
ULONG   dns_server_address[3];
#ifndef SAMPLE_DHCP_DISABLE
UINT    dns_server_address_size = sizeof(dns_server_address);
#endif
#if defined(SAMPLE_HTTP_PROXY_ENABLE)
NXD_ADDRESS proxy_server_address;
#endif /* SAMPLE_HTTP_PROXY_ENABLE */

    NX_PARAMETER_NOT_USED(parameter);

#if defined(SAMPLE_HTTP_PROXY_ENABLE)

    /* Enabled HTTP proxy.  */
    proxy_server_address.nxd_ip_version = NX_IP_VERSION_V4;
    proxy_server_address.nxd_ip_address.v4 = SAMPLE_HTTP_PROXY_SERVER;
    status = nx_http_proxy_client_enable(&ip_0, &proxy_server_address, SAMPLE_HTTP_PROXY_SERVER_PORT,
                                         SAMPLE_HTTP_PROXY_USER_NAME, sizeof(SAMPLE_HTTP_PROXY_USER_NAME) - 1,
                                         SAMPLE_HTTP_PROXY_PASSWORD, sizeof(SAMPLE_HTTP_PROXY_PASSWORD) - 1);
    if (status)
    {
        printf("Failed to enable HTTP proxy!\r\n");
        return;
    }
#endif /* SAMPLE_HTTP_PROXY_ENABLE  */

#ifndef SAMPLE_DHCP_DISABLE
    if (dhcp_wait())
    {
        printf("Failed to get the IP address!\r\n");
        return;
    }
#elif defined(SAMPLE_NETWORK_CONFIGURE)
    SAMPLE_NETWORK_CONFIGURE(&ip_0, &dns_server_address[0]);
#else
    nx_ip_gateway_address_set(&ip_0, SAMPLE_GATEWAY_ADDRESS);
#endif /* SAMPLE_DHCP_DISABLE  */

    /* Get IP address and gateway address.  */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gateway_address);

    /* Output IP address and gateway address.  */
    printf("IP address: %lu.%lu.%lu.%lu\r\n",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF));
    printf("Mask: %lu.%lu.%lu.%lu\r\n",
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF));
    printf("Gateway: %lu.%lu.%lu.%lu\r\n",
           (gateway_address >> 24),
           (gateway_address >> 16 & 0xFF),
           (gateway_address >> 8 & 0xFF),
           (gateway_address & 0xFF));

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_0, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address),
                                           &dns_server_address_size);
#elif !defined(SAMPLE_NETWORK_CONFIGURE)
    dns_server_address[0] = SAMPLE_DNS_SERVER_ADDRESS;
#endif /* SAMPLE_DHCP_DISABLE */

    /* Create DNS.  */
    status = dns_create(dns_server_address[0]);

    /* Check for DNS create errors.  */
    if (status)
    {
        printf("dns_create fail: %u\r\n", status);
        return;
    }

    /* Sync up time by SNTP at start up.  */
    status = sntp_time_sync();

    /* Check status.  */
    if (status)
    {
        printf("SNTP Time Sync failed.\r\n");
        printf("Set Time to default value: SAMPLE_SYSTEM_TIME.");
        unix_time_base = SAMPLE_SYSTEM_TIME;
    }
    else
    {
        printf("SNTP Time Sync successfully.\r\n");
    }

    unix_time_get((ULONG *)&unix_time);
    srand(unix_time);

    /* Start sample.  */
    sample_entry(&ip_0, &pool_0, &dns_0, unix_time_get);
}

#ifndef SAMPLE_DHCP_DISABLE
static UINT dhcp_wait()
{
UINT    status;
ULONG   actual_status;

    printf("DHCP In Progress...\r\n");

    /* Create the DHCP instance.  */
    status = nx_dhcp_create(&dhcp_0, &ip_0, "DHCP Client");

    /* Check status.  */
    if (status)
    {
        return(status);
    }

    /* Request NTP server.  */
    status = nx_dhcp_user_option_request(&dhcp_0, NX_DHCP_OPTION_NTP_SVR);

    /* Check status.  */
    if (status)
    {
        nx_dhcp_delete(&dhcp_0);
        return(status);
    }

    /* Start the DHCP Client.  */
    status = nx_dhcp_start(&dhcp_0);

    /* Check status.  */
    if (status)
    {
        nx_dhcp_delete(&dhcp_0);
        return(status);
    }

    /* Wait util address is solved.  */
    status = nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, SAMPLE_DHCP_WAIT_OPTION);

    /* Check status.  */
    if (status)
    {
        nx_dhcp_delete(&dhcp_0);
        return(status);
    }

    return(NX_SUCCESS);
}
#endif /* SAMPLE_DHCP_DISABLE  */

static UINT dns_create(ULONG dns_server_address)
{
UINT    status;

    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server.  */
    status = nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the packet pool?  */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages.  */
    status = nx_dns_packet_pool_set(&dns_0, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

    /* Add an IPv4 server address to the Client list.  */
    status = nx_dns_server_add(&dns_0, dns_server_address);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }

    /* Output DNS Server address.  */
    printf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
           (dns_server_address >> 24),
           (dns_server_address >> 16 & 0xFF),
           (dns_server_address >> 8 & 0xFF),
           (dns_server_address & 0xFF));

    return(NX_SUCCESS);
}

/* Sync up the local time.  */
static UINT sntp_time_sync_internal(ULONG sntp_server_address)
{
UINT    status;
UINT    server_status;
UINT    i;

    /* Create the SNTP Client to run in broadcast mode.. */
    status =  nx_sntp_client_create(&sntp_client, &ip_0, 0, &pool_0,  
                                    NX_NULL,
                                    NX_NULL,
                                    NX_NULL /* no random_number_generator callback */);

    /* Check status.  */
    if (status)
    {
        return(status);
    }

    /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
    status = nx_sntp_client_initialize_unicast(&sntp_client, sntp_server_address);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Set local time to 0 */
    status = nx_sntp_client_set_local_time(&sntp_client, 0, 0);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Run Unicast client */
    status = nx_sntp_client_run_unicast(&sntp_client);

    /* Check status.  */
    if (status)
    {
        nx_sntp_client_stop(&sntp_client);
        nx_sntp_client_delete(&sntp_client);
        return(status);
    }

    /* Wait till updates are received */
    for (i = 0; i < SAMPLE_SNTP_UPDATE_MAX; i++)
    {

        /* First verify we have a valid SNTP service running. */
        status = nx_sntp_client_receiving_updates(&sntp_client, &server_status);

        /* Check status.  */
        if ((status == NX_SUCCESS) && (server_status == NX_TRUE))
        {

            /* Server status is good. Now get the Client local time. */
            ULONG sntp_seconds, sntp_fraction;
            ULONG system_time_in_second;

            /* Get the local time.  */
            status = nx_sntp_client_get_local_time(&sntp_client, &sntp_seconds, &sntp_fraction, NX_NULL);

            /* Check status.  */
            if (status != NX_SUCCESS)
            {
                continue;
            }

            /* Get the system time in second.  */
            system_time_in_second = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

            /* Convert to Unix epoch and minus the current system time.  */
            unix_time_base = (sntp_seconds - (system_time_in_second + SAMPLE_UNIX_TO_NTP_EPOCH_SECOND));

            /* Time sync successfully.  */

            /* Stop and delete SNTP.  */
            nx_sntp_client_stop(&sntp_client);
            nx_sntp_client_delete(&sntp_client);

            return(NX_SUCCESS);
        }

        /* Sleep.  */
        tx_thread_sleep(SAMPLE_SNTP_UPDATE_INTERVAL);
    }

    /* Time sync failed.  */

    /* Stop and delete SNTP.  */
    nx_sntp_client_stop(&sntp_client);
    nx_sntp_client_delete(&sntp_client);

    /* Return success.  */
    return(NX_NOT_SUCCESSFUL);
}

static UINT sntp_time_sync()
{
UINT status;
ULONG gateway_address;
ULONG sntp_server_address[3];
#ifndef SAMPLE_DHCP_DISABLE
UINT  sntp_server_address_size = sizeof(sntp_server_address);
#endif

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve NTP server address.  */
    status = nx_dhcp_interface_user_option_retrieve(&dhcp_0, 0, NX_DHCP_OPTION_NTP_SVR, (UCHAR *)(sntp_server_address),
                                                    &sntp_server_address_size);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {
        for (UINT i = 0; (i * 4) < sntp_server_address_size; i++)
        {
            printf("SNTP Time Sync...%lu.%lu.%lu.%lu (DHCP)\r\n", 
                   (sntp_server_address[i] >> 24),
                   (sntp_server_address[i] >> 16 & 0xFF),
                   (sntp_server_address[i] >> 8 & 0xFF),
                   (sntp_server_address[i] & 0xFF));

            /* Start SNTP to sync the local time.  */
            status = sntp_time_sync_internal(sntp_server_address[i]);

            /* Check status.  */
            if(status == NX_SUCCESS)
            {
                return(NX_SUCCESS);
            }
        }
    }
#endif /* SAMPLE_DHCP_DISABLE */

    /* Sync time by NTP server array.  */
    for (UINT i = 0; i < SAMPLE_SNTP_SYNC_MAX; i++)
    {
        printf("SNTP Time Sync...%s\r\n", sntp_servers[sntp_server_index]);

        /* Make sure the network is still valid.  */
        while (nx_ip_gateway_address_get(&ip_0, &gateway_address))
        {
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
        }

        /* Look up SNTP Server address. */
        status = nx_dns_host_by_name_get(&dns_0, (UCHAR *)sntp_servers[sntp_server_index], &sntp_server_address[0], 5 * NX_IP_PERIODIC_RATE);

        /* Check status.  */
        if (status == NX_SUCCESS)
        {

            /* Start SNTP to sync the local time.  */
            status = sntp_time_sync_internal(sntp_server_address[0]);

            /* Check status.  */
            if(status == NX_SUCCESS)
            {
                return(NX_SUCCESS);
            }
        }

        /* Switch SNTP server every time.  */
        sntp_server_index = (sntp_server_index + 1) % (sizeof(sntp_servers) / sizeof(sntp_servers[0]));
    }

    return(NX_NOT_SUCCESSFUL);
}

static UINT unix_time_get(ULONG *unix_time)
{

    /* Return number of seconds since Unix Epoch (1/1/1970 00:00:00).  */
    *unix_time =  unix_time_base + (tx_time_get() / TX_TIMER_TICKS_PER_SECOND);

    return(NX_SUCCESS);
}
