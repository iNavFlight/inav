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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX IPerf stack Component                                            */
/**                                                                       */
/**   This is a small demo of the high-performance NetX IPerf stack.      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#include   "tx_api.h"
#include   "nx_api.h"

/* Define demo stack size.   */
#define                 NX_PACKET_POOL_SIZE     ((1536 + sizeof(NX_PACKET)) * 30)
#define                 DEMO_STACK_SIZE         2048
#define                 HTTP_STACK_SIZE         2048
#define                 IPERF_STACK_SIZE        2048

/* Define the ThreadX and NetX object control blocks...  */
TX_THREAD               thread_0;
NX_PACKET_POOL          pool_0;
NX_IP                   ip_0;

UCHAR                   *pointer;
UCHAR                   *http_stack;
UCHAR                   *iperf_stack;
#ifdef FEATURE_NX_IPV6
NXD_ADDRESS             ipv6_address;
#endif

/* Define the counters used in the demo application...  */
ULONG                   error_counter;

/* Define thread prototypes.  */
VOID    thread_0_entry(ULONG thread_input);
extern  VOID nx_iperf_entry(NX_PACKET_POOL *pool_ptr, NX_IP *ip_ptr, UCHAR* http_stack, ULONG http_stack_size, UCHAR *iperf_stack, ULONG iperf_stack_size);

/***** Substitute your ethernet driver entry function here *********/
extern  VOID _nx_ram_network_driver(NX_IP_DRIVER*);


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

    /* Setup the working pointer.  */
    pointer = (UCHAR *) first_unused_memory;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool",
                                   (1536 + sizeof(NX_PACKET)),
                                   pointer, NX_PACKET_POOL_SIZE);
    pointer = pointer + NX_PACKET_POOL_SIZE;

    /* Check for packet pool create errors.  */
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(192, 168, 100, 18), 0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP */
    status = nx_icmp_enable(&ip_0);

    /* Check for ICMP enable errors.  */
    if(status)
        error_counter++;

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Enable TCP traffic.  */
    status =  nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
        error_counter++;

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,
                     pointer, DEMO_STACK_SIZE,
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

#ifdef FEATURE_NX_IPV6
    /* Set up the IPv6 address here. */
    ipv6_address.nxd_ip_address.v6[3] = 0x3;
    ipv6_address.nxd_ip_address.v6[2] = 0x0;
    ipv6_address.nxd_ip_address.v6[1] = 0x0;
    ipv6_address.nxd_ip_address.v6[0] = 0xfe800000;
    ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Enable ICMPv6 services. */
    status = nxd_icmp_enable(&ip_0);
    if (status)
        error_counter++;

    /* Enable IPv6 services. */
    status = nxd_ipv6_enable(&ip_0);
    if (status)
        error_counter++;

    status = nxd_ipv6_address_set(&ip_0, 0, &ipv6_address, 10, NX_NULL);
    if (status)
        error_counter++;
#endif
}

/* Define the test threads.  */
void    thread_0_entry(ULONG thread_input)
{
#ifdef FEATURE_NX_IPV6
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
#endif

    /* Set the HTTP stack and IPerf stack.  */
    http_stack = pointer;
    pointer += HTTP_STACK_SIZE;
    iperf_stack = pointer;
    pointer += IPERF_STACK_SIZE;

    /* Call entry function to start iperf test.  */
    nx_iperf_entry(&pool_0, &ip_0, http_stack, HTTP_STACK_SIZE, iperf_stack, IPERF_STACK_SIZE);
}